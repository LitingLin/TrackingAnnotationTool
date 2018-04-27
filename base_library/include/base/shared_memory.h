#pragma once

#include "security_attributes.h"

typedef void * HANDLE;

namespace Base
{
	class SharedMemory
	{
	public:
		SharedMemory();
		SharedMemory(const SharedMemory &) = delete;
		SharedMemory(SharedMemory &&) noexcept;
		~SharedMemory();
		void create(const wchar_t *name, size_t size);
		void open(const wchar_t *name);
		void close();
		char *get();
	private:
		HANDLE _hMapFile;
		AllowAllSecurityATTRIBUTES _securityAttributes;
		char *_ptr;
	};
}