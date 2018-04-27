#pragma once
#include <cstdint>

typedef void * HANDLE;

namespace Base
{
	class MemoryMappedIO
	{
	public:
		MemoryMappedIO(const wchar_t *filename);
		MemoryMappedIO(const MemoryMappedIO&) = delete;
		~MemoryMappedIO() noexcept(false);
		const unsigned char *getPtr() const;
		uint64_t getSize() const;
	private:
		HANDLE hFile;
		HANDLE hFileMapping;
		void *ptr;
	};
}