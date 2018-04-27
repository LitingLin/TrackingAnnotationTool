#include <base/process.h>

#include <Shellapi.h>
#include <Psapi.h>

#include <base/logging.h>
#include <base/sync.h>
#include <base/file.h>

namespace Base
{
	OpenProcessGuard::OpenProcessGuard(DWORD dwDesiredAccess, DWORD dwProcessId)
	{
		handle = OpenProcess(dwDesiredAccess, FALSE, dwProcessId);
	}

	OpenProcessGuard::~OpenProcessGuard()
	{
		if (handle)
			CloseHandle(handle);
	}

	HANDLE OpenProcessGuard::getHANDLE() const
	{
		return handle;
	}

	Process::Process(const wchar_t* application, const wchar_t* command)
	{
		SHELLEXECUTEINFO shellexceptioninfo;
		memset(&shellexceptioninfo, 0, sizeof(SHELLEXECUTEINFO));
		shellexceptioninfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shellexceptioninfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI | SEE_MASK_UNICODE | SEE_MASK_NOASYNC;
		shellexceptioninfo.lpVerb = L"open";
		shellexceptioninfo.lpFile = application;
		shellexceptioninfo.lpParameters = command;
		shellexceptioninfo.nShow = SW_SHOWNORMAL;
		CHECK_WIN32API(ShellExecuteEx(&shellexceptioninfo));
		CHECK_GE_WIN32API(std::ptrdiff_t(shellexceptioninfo.hInstApp), 32);
		_processHandle = shellexceptioninfo.hProcess;
	}

	Process::Process(Process&& other) noexcept
	{
		_processHandle = other._processHandle;
		other._processHandle = nullptr;
	}

	Process::~Process()
	{
		if (_processHandle)
			CloseHandle(_processHandle);
	}

	HANDLE Process::getHandle() const
	{
		return _processHandle;
	}

	unsigned long Process::getId() const
	{
		return GetProcessId(_processHandle);
	}

	bool Process::isActive() const
	{
		DWORD rc = WaitForSingleObject(_processHandle, 0);
		if (rc == WAIT_TIMEOUT)
			return true;
		else if (rc == WAIT_OBJECT_0)
			return false;
		else
			NOT_IMPLEMENTED_ERROR;
		return false;
	}

	struct MonitorWorkerContext
	{
		ProcessMonitor *ptr;
		HANDLE process;
	};

	unsigned long Process::getExitCode() const
	{
		DWORD exit_code;
		ENSURE_WIN32API(GetExitCodeProcess(_processHandle, &exit_code));
		return exit_code;
	}

	ProcessMonitor::ProcessMonitor()
	{
		_finalize_signal = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		ENSURE_WIN32API(_finalize_signal);
	}

	ProcessMonitor::~ProcessMonitor()
	{
		std::map<DWORD, HANDLE> running_threads;
		{
			std::lock_guard<std::mutex> lock_guard(_mutex);
			running_threads = _running_threads;
			ENSURE_WIN32API(SetEvent(_finalize_signal));
		}

		for (auto thread_context : running_threads) {
			WaitForSingleObject(thread_context.second, INFINITE);
			CloseHandle(thread_context.second);
		}

		LOG_IF_FAILED_WIN32API(CloseHandle(_finalize_signal));
	}

	void ProcessMonitor::add(HANDLE process_handle)
	{
		MonitorWorkerContext *context = new MonitorWorkerContext{ this, process_handle };
		HANDLE thread_handle = (HANDLE)_beginthreadex(nullptr, 0, monitor_worker, context, 0, nullptr);
		ENSURE_NE_CRTAPI(std::ptrdiff_t(thread_handle), -1L);
		const DWORD tid = GetThreadId(thread_handle);
		ENSURE_WIN32API(tid);
		_running_threads.insert(std::make_pair(tid, thread_handle));
	}

	HANDLE ProcessMonitor::getEventHandle() const
	{
		return _event.getHandle();
	}

	HANDLE ProcessMonitor::pickLastExitProcessHandle()
	{
		HANDLE handle;
		if (!_exitedProcessHandle.try_pop(handle))
			return nullptr;

		return handle;
	}

	unsigned ProcessMonitor::monitor_worker(void* para)
	{
		MonitorWorkerContext context = *static_cast<MonitorWorkerContext*>(para);
		delete static_cast<MonitorWorkerContext*>(para);

		ProcessMonitor *this_ptr = context.ptr;
		HANDLE waitProcess[2] = { context.process, this_ptr->_finalize_signal };
		unsigned signaledIndex;
		waitForMultipleObjects(waitProcess, 2, &signaledIndex);
		if (signaledIndex == 1)
			return 1;

		this_ptr->_exitedProcessHandle.push(context.process);
		this_ptr->_event.set();

		const DWORD tid = GetCurrentThreadId();

		{
			std::lock_guard<std::mutex> lock_guard(this_ptr->_mutex);
			const auto current_thread_handle_iter = this_ptr->_running_threads.find(tid);			
			if (current_thread_handle_iter == this_ptr->_running_threads.end()) return 0;
			HANDLE current_thread_handle = current_thread_handle_iter->second;
			ENSURE(this_ptr->_running_threads.erase(tid)) << "_running_threads should have the handle";
			CloseHandle(current_thread_handle);
		}
		return 0;
	}

	std::vector<uint32_t> getSystemProcessesIds()
	{
		const uint32_t initialSize = 1024;
		uint32_t size = initialSize;
		std::vector<uint32_t> ids(size);
		while (true)
		{
			uint32_t gotSize;
			ENSURE_WIN32API(EnumProcesses((DWORD*)ids.data(), size * sizeof(uint32_t), (DWORD*)&gotSize));
			gotSize /= sizeof(uint32_t);
			if (size == gotSize) {
				if (size > std::numeric_limits<uint32_t>::max() / sizeof(uint32_t) / 2)
					size = std::numeric_limits<uint32_t>::max() / sizeof(uint32_t);
				else
					size *= 2;
				ids.resize(size);
			}
			else
			{
				std::vector<uint32_t> return_ids(gotSize);
				memcpy(return_ids.data(), ids.data(), gotSize * sizeof(uint32_t));
				return return_ids;
			}
		}
	}

	std::vector<HMODULE> getProcessModules(HANDLE hProcess)
	{
		const uint32_t initialSize = 1024;
		uint32_t size = initialSize;
		std::vector<HMODULE> modules(size);
		while (true)
		{
			uint32_t gotSize;
			if (!EnumProcessModulesEx(hProcess, modules.data(), size * sizeof(HMODULE), (DWORD*)&gotSize, LIST_MODULES_ALL))
				return std::vector<HMODULE>();
			gotSize /= sizeof(HMODULE);
			if (size < gotSize) {
				size = gotSize;
				modules.resize(size);
			}
			else
			{
				std::vector<HMODULE> return_modules(gotSize);
				memcpy(return_modules.data(), modules.data(), gotSize * sizeof(HMODULE));
				return return_modules;
			}
		}
	}

	std::wstring getModuleFullPath(HANDLE hProcess, HMODULE hModule)
	{
		const uint32_t initialSize = 1024;
		std::wstring buffer;
		buffer.resize(initialSize);

		while (true)
		{
			DWORD pathSize = GetModuleFileNameEx(hProcess, hModule, (wchar_t*)buffer.data(), buffer.size() + 1);
			if (pathSize == 0)
				return std::wstring();
			if (pathSize == buffer.size())
			{
				buffer.resize(buffer.size() * 2);
			}
			else
			{
				std::wstring moduleFullPath;
				moduleFullPath.resize(pathSize);
				memcpy((wchar_t*)moduleFullPath.data(), buffer.data(), pathSize * sizeof(wchar_t));
				return moduleFullPath;
			}
		}
	}

	std::wstring getProcessFullPath(HANDLE hProcess)
	{
		const uint32_t initialSize = 1024;

		DWORD exePathBufferSize = initialSize;
		std::wstring exePathBuffer;
		exePathBuffer.resize(exePathBufferSize - 1);
		while (true)
		{
			if (!QueryFullProcessImageName(hProcess, 0, (wchar_t*)exePathBuffer.data(), &exePathBufferSize))
				return std::wstring();

			if (exePathBufferSize == exePathBuffer.size())
			{
				exePathBuffer.resize(exePathBufferSize * 2 - 1);
				exePathBufferSize *= 2;
			}
			else
			{
				std::wstring processFullPath;
				processFullPath.resize(exePathBufferSize);
				memcpy((wchar_t*)processFullPath.data(), exePathBuffer.data(), exePathBufferSize * sizeof(wchar_t));
				return processFullPath;
			}
		}
	}

	std::wstring getProcessName(HANDLE hProcess)
	{
		return getFileName(getProcessFullPath(hProcess));
	}

	SIZE_T getProcessWorkingSetSize(HANDLE hProcess)
	{
		PROCESS_MEMORY_COUNTERS process_memory;
		if (!GetProcessMemoryInfo(hProcess, &process_memory, sizeof(PROCESS_MEMORY_COUNTERS)))
			return 0;
		return process_memory.WorkingSetSize;
	}

	std::wstring SIZE_TToStringInKB(SIZE_T size)
	{
		std::wstringstream ss;
		ss << (size / 1024);
		return ss.str() + L"KB";
	}

	std::wstring enumerateAllProcessInfomation()
	{
		std::vector<uint32_t> processIds = getSystemProcessesIds();
		std::wstring processInfomation = L"Name\tID\tMemory\tPath\n";

		for (uint32_t processId : processIds)
		{
			OpenProcessGuard processHandle(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, processId);
			if (processHandle.getHANDLE() == NULL)
				continue;

			std::wstring processFullPath = getProcessFullPath(processHandle.getHANDLE());
			std::wstring processName = getFileName(processFullPath);
			std::wstring workingSetSize = SIZE_TToStringInKB(getProcessWorkingSetSize(processHandle.getHANDLE()));

			processInfomation += fmt::format(L"{}\t{}\t{}\t{}\n", processName, processId, workingSetSize, processFullPath);
		}
		return processInfomation;
	}
}