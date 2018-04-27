#define NOMINMAX
#define WIN32_MEAN_AND_LEAN

#include <base/memory_mapped_io.h>
#include <base/logging.h>

#include <windows.h>

namespace Base
{
	MemoryMappedIO::MemoryMappedIO(const wchar_t* filename)
		: hFile(nullptr), hFileMapping(nullptr)
	{
		try
		{
			hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			CHECK_NE_WIN32API(hFile, INVALID_HANDLE_VALUE);
			hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
			CHECK_WIN32API(hFileMapping);
			ptr = MapViewOfFileEx(hFileMapping, FILE_MAP_READ, 0, 0, 0, NULL);
			CHECK_WIN32API(ptr);
		}
		catch (...)
		{
			if (hFileMapping)
				CloseHandle(hFileMapping);
			if (hFile)
				CloseHandle(hFile);
			throw;
		}
	}

	MemoryMappedIO::~MemoryMappedIO() noexcept(false)
	{
		LOG_IF_FAILED_WIN32API(UnmapViewOfFile(ptr));
		LOG_IF_FAILED_WIN32API(CloseHandle(hFileMapping));
		LOG_IF_FAILED_WIN32API(CloseHandle(hFile));
	}

	const unsigned char* MemoryMappedIO::getPtr() const
	{
		return (const unsigned char *)ptr;
	}

	uint64_t MemoryMappedIO::getSize() const
	{
		LARGE_INTEGER large_integer;
		CHECK_WIN32API(GetFileSizeEx(hFile, &large_integer));
		return large_integer.QuadPart;
	}
}