#include <base/file.h>

#include <base/logging.h>
#include <random>

namespace Base
{
	bool isPathExists(const std::wstring& path)
	{
		return GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	unsigned char UTF16LE_BOM[2] = { 0xFF,0xFE };

	std::wstring getModuleInstanceFullPath(HINSTANCE instance)
	{
		std::vector<wchar_t> pathBuf;
		DWORD copied = 0;
		do {
			pathBuf.resize(pathBuf.size() + MAX_PATH);
			copied = GetModuleFileName(instance, &pathBuf.at(0), DWORD(pathBuf.size()));
			CHECK_NE_WIN32API(copied, 0U);
		} while (copied >= pathBuf.size());

		pathBuf.resize(copied);

		std::wstring path(pathBuf.begin(), pathBuf.end());
		return path;
	}

	std::wstring getWorkDirectory()
	{
		DWORD current_directory_size = GetCurrentDirectory(0, nullptr);
		std::wstring work_directory;
		work_directory.resize(current_directory_size - 1);
		ENSURE_WIN32API(GetCurrentDirectory(current_directory_size, &work_directory[0]));
		return work_directory;
	}

	std::wstring getApplicationPath()
	{
		return getModuleInstanceFullPath(nullptr);
	}

	std::wstring getTempPath()
	{
		wchar_t buffer[MAX_PATH + 1];
		DWORD tempPathSize = GetTempPath(MAX_PATH + 1, buffer);
		ENSURE_NE(tempPathSize, DWORD(0));
		return std::wstring(buffer);
	}

	std::wstring getFileName(const std::wstring& path)
	{
		size_t backslashpos = path.find_last_of(L'\\');
		size_t slashpos = path.find_last_of(L'/');
		size_t pos;
		if (backslashpos > slashpos)
		{
			if (backslashpos == path.npos)
				pos = slashpos;
			else
				pos = backslashpos;
		}
		else
		{
			if (slashpos == path.npos)
				pos = backslashpos;
			else
				pos = slashpos;
		}
		if (pos == path.npos)
			pos = 0;
		else
			pos++;
		return path.substr(pos, path.size() - pos);

	}

	bool isURL(const std::wstring& string)
	{
		size_t slash_pos = string.find_first_of(L":");
		if (slash_pos != string.npos && slash_pos < string.size() - 2 && string[slash_pos + 1] == L'/' && string[slash_pos + 2] == L'/')
			return true;
		else
			return false;
	}

	bool isDirectory(const std::wstring& string)
	{
		return GetFileAttributes(string.c_str()) & FILE_ATTRIBUTE_DIRECTORY;
	}

	std::wstring getFullPath(const std::wstring& path)
	{
		if (isURL(path))
			return path;
		std::wstring buffer;
		std::wstring res;
		buffer.resize(MAX_PATH);
		while (true)
		{
			DWORD copied_size = GetFullPathName(path.c_str(), (DWORD)buffer.size() + 1, &buffer[0], nullptr);
			CHECK_NE_WIN32API(copied_size, 0U);
			if (copied_size == buffer.size())
				buffer.resize(buffer.size() * 2);
			else
			{
				res.resize(copied_size);
				memcpy(&res[0], buffer.c_str(), copied_size * sizeof(wchar_t));
				break;
			}
		}
		return res;
	}

	std::wstring appendPath(const std::wstring& path, const std::wstring &fileName)
	{
		if (path[path.size() - 1] == L'/' || path[path.size() - 1] == L'\\')
			return path + fileName;
		else
			return path + L'\\' + fileName;
	}

	std::wstring getFileExtension(const std::wstring& path)
	{
		size_t dot_pos = path.find_last_of(L'.');
		if (dot_pos == path.npos || path.find(L'/', dot_pos + 1) != path.npos || path.find(L'\\', dot_pos + 1) != path.npos)
			return std::wstring();
		return path.substr(dot_pos + 1);
	}

	std::wstring getCanonicalPath(const std::wstring& path)
	{
		std::wstring buffer;
		buffer.resize(path.size());

		size_t buffer_ind = 0;

		for (size_t i = 0; i < path.size(); ++i)
		{
			if (path[i] == L'\\')
			{
				if (buffer_ind != 0 && buffer[buffer_ind - 1] == L'\\')
					continue;
				else {
					buffer[buffer_ind] = L'\\';
					buffer_ind++;
				}
			}
			else if (path[i] == L'/')
			{
				if (buffer_ind != 0 && buffer[buffer_ind - 1] == L'\\')
					continue;
				else {
					buffer[buffer_ind] = L'\\';
					buffer_ind++;
				}
			}
			else if (path[i] == L'.')
			{
				if (i + 1 < path.size() && path[i + 1] == L'.' && (i + 2 == path.size() || path[i + 2] == L'\\' || path[i + 2] == L'/') && i >= 1 && (path[i - 1] == L'\\' || path[i - 1] == L'/'))
				{
					i++;
					if (buffer_ind < 2)
						continue;
					size_t slash_pos = buffer.find_last_of(L'\\', buffer_ind - 2);
					if (slash_pos == buffer.npos)
						continue;
					buffer_ind = slash_pos;
				}
				else if (i >= 1 && (path[i - 1] == L'\\' || path[i - 1] == L'/') && (i + 1 == path.size() || path[i + 1] == L'\\' || path[i + 1] == L'/'))
				{
					continue;
				}
				else
				{
					buffer[buffer_ind] = path[i];
					buffer_ind++;
				}
			}
			else
			{
				buffer[buffer_ind] = path[i];
				buffer_ind++;
			}
		}
		return buffer.substr(0, buffer_ind);
	}

	DirectoryIterator::DirectoryIterator(const std::wstring& path)
		: _handle(nullptr), _path(path)
	{
	}

	DirectoryIterator::~DirectoryIterator()
	{
		if (_handle)
			LOG_IF_FAILED_WIN32API(FindClose(_handle));
	}

	bool DirectoryIterator::next(std::wstring& fileName, bool &isDirectory, uint64_t &lastFileWriteTime)
	{
		WIN32_FIND_DATA fileData;
		if (!_handle) {
			_handle = FindFirstFileEx((_path + L"\\*").c_str(), FindExInfoBasic, &fileData, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
			if (_handle == INVALID_HANDLE_VALUE) {
				_handle = nullptr;
				return false;
			}
		}
		else
			if (FindNextFile(_handle, &fileData) == FALSE)
				return false;

		fileName = fileData.cFileName;
		isDirectory = fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		lastFileWriteTime = *((uint64_t*)&fileData.ftLastWriteTime);
		return true;
	}

	void DirectoryIterator::reset()
	{
		if (_handle) {
			LOG_IF_FAILED_WIN32API(FindClose(_handle));
			_handle = nullptr;
		}
	}

	SequentialDirectoryFileListGetter::SequentialDirectoryFileListGetter(const std::wstring& path)
		: _path(path)
	{
	}

	bool SequentialDirectoryFileListGetter::getFileList(std::vector<std::wstring> &fileNames, std::vector<uint64_t> &lastWriteTimes)
	{
		return getDirectoryFileLists(_path, fileNames, lastWriteTimes);
	}

	RandomDirectoryFileListGetter::RandomDirectoryFileListGetter(const std::wstring& path)
		: _path(path)
	{
	}

	bool RandomDirectoryFileListGetter::getFileList(std::vector<std::wstring>& fileNames,
		std::vector<uint64_t>& lastWriteTimes)
	{
		return getRandomShuffledDirectoryFileLists(_path, fileNames, lastWriteTimes);
	}

	bool getDirectoryFileLists(const std::wstring &path, std::vector<std::wstring> &fileNames, std::vector<uint64_t> &lastWriteTimes)
	{
		DirectoryIterator directoryIterator(path);
		std::wstring tempFileName;
		bool isDirectory;
		uint64_t tempLastWriteTime;
		while (directoryIterator.next(tempFileName, isDirectory, tempLastWriteTime))
		{
			if (isDirectory)
				continue;
			fileNames.push_back(tempFileName);
			lastWriteTimes.push_back(tempLastWriteTime);
		}
		return !fileNames.empty();
	}

	bool getRandomShuffledDirectoryFileLists(const std::wstring &path, std::vector<std::wstring> &fileNames, std::vector<uint64_t> &lastWriteTimes)
	{
		std::vector<std::wstring> sequentialFileNames;
		std::vector<uint64_t> sequentialLastWriteTimes;
		if (getDirectoryFileLists(path, sequentialFileNames, sequentialLastWriteTimes))
		{
			std::vector<size_t> index(sequentialFileNames.size());
			for (size_t i = 0; i < index.size(); ++i)
			{
				index[i] = i;
			}
			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(index.begin(), index.end(), g);
			fileNames.resize(sequentialFileNames.size());
			lastWriteTimes.resize(sequentialLastWriteTimes.size());
			for (size_t i = 0; i < index.size(); ++i)
			{
				fileNames[index[i]] = sequentialFileNames[i];
				lastWriteTimes[index[i]] = sequentialLastWriteTimes[i];
			}
			return true;
		}
		return false;
	}

	File::Mode operator&(File::Mode left, File::Mode right)
	{
		return File::Mode((uint32_t)left & (uint32_t)right);
	}

	File::Mode operator|(File::Mode left, File::Mode right)
	{
		return File::Mode((uint32_t)left | (uint32_t)right);
	}

	bool randomPickFile(DirectoryIterator& direcotryIterator, std::wstring& fileName, uint64_t *lastWriteTimePtr,
		std::vector<std::wstring> *fileNamesPtr, std::vector<uint64_t> *lastWriteTimesPtr, size_t *indexPtr)
	{
		direcotryIterator.reset();
		std::vector<std::wstring> fileNames;
		std::vector<uint64_t> lastWriteTimes;
		std::wstring tempFileName;
		bool isDirectory;
		uint64_t tempLastWriteTime;
		while (direcotryIterator.next(tempFileName, isDirectory, tempLastWriteTime))
		{
			if (isDirectory)
				continue;
			fileNames.push_back(tempFileName);
			lastWriteTimes.push_back(tempLastWriteTime);
		}

		if (fileNames.empty())
			return false;

		std::random_device rd;  //Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<size_t> dis(0, fileNames.size() - 1);
		size_t fileIndex = dis(gen);
		fileName = fileNames[fileIndex];

		if (lastWriteTimePtr)
			*lastWriteTimePtr = lastWriteTimes[fileIndex];
		if (fileNamesPtr)
			*fileNamesPtr = fileNames;
		if (lastWriteTimesPtr)
			*lastWriteTimesPtr = lastWriteTimes;
		if (indexPtr)
			*indexPtr = fileIndex;
		return true;
	}

	bool pickNextFile(DirectoryIterator& direcotryIterator, std::wstring& fileName, uint64_t *lastWriteTimePtr)
	{
		bool isDirectory;
		std::wstring tempFileName;
		uint64_t lastWriteTime;
		while (direcotryIterator.next(tempFileName, isDirectory, lastWriteTime))
		{
			if (!isDirectory)
				break;
		}
		if (tempFileName.empty())
		{
			direcotryIterator.reset();
			while (direcotryIterator.next(tempFileName, isDirectory, lastWriteTime))
			{
				if (!isDirectory)
					break;
			}
		}
		if (tempFileName.empty())
			return false;
		else
		{
			fileName = tempFileName;
			if (lastWriteTimePtr)
				*lastWriteTimePtr = lastWriteTime;
			return true;
		}
	}

	File::File(const std::wstring& path, Mode mode)
	{
		uint32_t desiredAccess = 0;
		if (uint32_t(mode & Mode::read))
			desiredAccess |= GENERIC_READ;
		else if (uint32_t(mode & Mode::write))
			desiredAccess |= GENERIC_WRITE;
		uint32_t creationDisposition = OPEN_EXISTING;
		if (uint32_t(mode & Mode::create_always))
			creationDisposition = CREATE_ALWAYS;
		else if (uint32_t(mode & Mode::create_new))
			creationDisposition |= CREATE_NEW;
		else if (uint32_t(mode & Mode::open_always))
			creationDisposition = OPEN_ALWAYS;
		else if (uint32_t(mode & Mode::truncate_existing))
			creationDisposition = TRUNCATE_EXISTING;
		_fileHandle = CreateFile(path.c_str(), desiredAccess, FILE_SHARE_READ, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		CHECK_NE_WIN32API(_fileHandle, INVALID_HANDLE_VALUE);
	}

	File::File(File&& other) noexcept
		: _fileHandle(other._fileHandle)
	{
		other._fileHandle = nullptr;
	}

	File::~File()
	{
		if (_fileHandle)
			LOG_IF_FAILED_WIN32API(CloseHandle(_fileHandle));
	}

	uint64_t File::getSize() const
	{
		LARGE_INTEGER large_integer;
		CHECK_WIN32API(GetFileSizeEx(_fileHandle, &large_integer));
		return large_integer.QuadPart;
	}

	uint64_t File::read(unsigned char* buffer, uint64_t offset, uint64_t size) const
	{
		LARGE_INTEGER large_integer;
		large_integer.QuadPart = offset;
		CHECK_WIN32API(SetFilePointerEx(_fileHandle, large_integer, nullptr, FILE_BEGIN));
		uint64_t totalReadFileSize = 0;
		while (size)
		{
			DWORD this_read_size;
			if (size > std::numeric_limits<DWORD>::max()) {
				this_read_size = std::numeric_limits<DWORD>::max();
				size -= this_read_size;
			}
			else
			{
				this_read_size = (DWORD)size;
				size = 0;
			}

			DWORD sizeRead;
			CHECK_WIN32API(ReadFile(_fileHandle, buffer, this_read_size, &sizeRead, nullptr));
			totalReadFileSize += sizeRead;
			buffer += this_read_size;
		}
		return totalReadFileSize;
	}

	uint64_t File::write(const unsigned char* buffer, uint64_t offset, uint64_t size)
	{
		LARGE_INTEGER large_integer;
		large_integer.QuadPart = offset;
		CHECK_WIN32API(SetFilePointerEx(_fileHandle, large_integer, nullptr, FILE_BEGIN));
		uint64_t totalWriteFileSize = 0;
		while (size)
		{
			DWORD currentWriteSize;
			if (size > std::numeric_limits<DWORD>::max()) {
				currentWriteSize = std::numeric_limits<DWORD>::max();
				size -= currentWriteSize;
			}
			else
			{
				currentWriteSize = (DWORD)size;
				size = 0;
			}

			DWORD sizeWritten;
			CHECK_WIN32API(WriteFile(_fileHandle, buffer, currentWriteSize, &sizeWritten, nullptr));
			totalWriteFileSize += sizeWritten;
			buffer += currentWriteSize;
		}
		return totalWriteFileSize;
	}

	uint64_t File::getLastWriteTime() const
	{
		FILETIME lastWriteTime;
		CHECK_WIN32API(GetFileTime(_fileHandle, nullptr, nullptr, &lastWriteTime));
		static_assert(sizeof(uint64_t) == sizeof(FILETIME), "");
		return *((uint64_t*)(&lastWriteTime));
	}

	HANDLE File::getHANDLE()
	{
		return _fileHandle;
	}

	std::wstring getParentPath(const std::wstring& path)
	{
		size_t end_pos = path.size();

		while (true)
		{
			if (!end_pos)
				return std::wstring();
			if (path[end_pos - 1] == L'/' || path[end_pos - 1] == L'\\')
				--end_pos;
			else
				break;
		}

		auto last_slash_pos = path.find_last_of(L'/', end_pos - 1);
		auto last_back_slash_pos = path.find_last_of(L'\\', end_pos - 1);
		if (last_slash_pos == path.npos)
			last_slash_pos = last_back_slash_pos;
		else if (last_back_slash_pos != path.npos && last_back_slash_pos > last_slash_pos)
			last_slash_pos = last_back_slash_pos;
		if (end_pos > last_slash_pos)
			end_pos = last_slash_pos;

		while (true)
		{
			if (!end_pos)
				return std::wstring();
			if (path[end_pos - 1] == L'/' || path[end_pos - 1] == L'\\')
				--end_pos;
			else
				break;
		}

		return path.substr(0, end_pos);
	}
}