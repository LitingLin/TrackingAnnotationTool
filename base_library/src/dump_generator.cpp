#include <base/dump_generator.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>
#include <vector>

#include <base/logging.h>
#include <base/file.h>
#include <base/process.h>
#include <base/thread.h>

namespace Base
{
	std::vector<uint32_t> getProductProcessIds(const std::wstring &productPathPrefix)
	{
		CHECK_LE(productPathPrefix.size(), std::numeric_limits<uint32_t>::max());
		std::wstring lowerCaseProductPathPrefix = toLowerCase(productPathPrefix);
		std::vector<uint32_t> processIds = getSystemProcessesIds();

		std::vector<uint32_t> productProcessIds;
		for (uint32_t processId : processIds)
		{
			OpenProcessGuard processHandle(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, processId);
			if (processHandle.getHANDLE() == NULL)
				continue;

			std::wstring processFullPath = toLowerCase(getProcessFullPath(processHandle.getHANDLE()));
			if (processFullPath.size() < lowerCaseProductPathPrefix.size())
				continue;

			if (memcmp(processFullPath.data(), lowerCaseProductPathPrefix.data(), lowerCaseProductPathPrefix.size() * sizeof(wchar_t)) == 0)
			{
				productProcessIds.push_back(processId);
			}
			else
			{
				std::vector<HMODULE> processModules = getProcessModules(processHandle.getHANDLE());
				if (processModules.empty())
					continue;

				for (HMODULE hmodule : processModules)
				{
					std::wstring moduleFullPath = toLowerCase(getModuleFullPath(processHandle.getHANDLE(), hmodule));
					if (moduleFullPath.size() > lowerCaseProductPathPrefix.size())
					{
						if (memcmp(moduleFullPath.data(), lowerCaseProductPathPrefix.data(), lowerCaseProductPathPrefix.size()) == 0)
						{
							productProcessIds.push_back(processId);
							break;
						}
					}
				}
			}
		}

		return productProcessIds;
	}

	class DumpWritingWorker : public Runnable
	{
	public:
		DumpWritingWorker(uint32_t processId, const std::wstring &dumpPath, bool excludeSelf)
			: _processId(processId), _dumpPath(dumpPath), _excludeSelf(excludeSelf)
		{
		}
		int job_entry() override
		{
			uint32_t currentProcessId = GetCurrentProcessId();
			if (_excludeSelf && _processId == currentProcessId)
				return -1;
			OpenProcessGuard processHandle(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_DUP_HANDLE, _processId);
			if (processHandle.getHANDLE() == NULL)
				return -1;
			const std::wstring processName = getProcessName(processHandle.getHANDLE());
			if (processName.empty())
				return -1;

			const std::wstring dumpPath = fmt::format(L"{}{}-{}.dmp", _dumpPath, processName, _processId);

			BOOL isOK;

			{
				File dumpFile(dumpPath, File::Mode::write | File::Mode::create_always);
				isOK = MiniDumpWriteDump(processHandle.getHANDLE(), _processId, dumpFile.getHANDLE(), MiniDumpWithFullMemory,
					NULL, NULL, NULL);
			}
			if (!isOK) {
				logger->warn(fmt::format("Minidump generation failed while dumping process: {}, Win32 error code: {}, message: {}\n",
					UTF16ToUTF8(processName), GetLastError(), Win32ErrorCodeToString(GetLastError()).getString()));
				return 1;
			}
			return 0;
		}
		bool job_cancel() override
		{
			return false;
		}
	private:
		uint32_t _processId;
		std::wstring _dumpPath;
		bool _excludeSelf;
	};

	void generateDump(const std::wstring &productPathPrefix, const std::wstring &path, bool excludeSelf)
	{
		std::vector<uint32_t> productProcessIds = getProductProcessIds(productPathPrefix);
		std::vector<std::unique_ptr<WorkerWrapper>> workers;
		workers.reserve(productProcessIds.size());

		for (uint32_t processId : productProcessIds)
		{
			workers.emplace_back(std::make_unique<WorkerWrapper>(std::make_unique<DumpWritingWorker>(processId, path, excludeSelf), false, false));
		}
		for (std::unique_ptr<WorkerWrapper> &worker : workers)
		{
			worker->run();
		}
	}
}