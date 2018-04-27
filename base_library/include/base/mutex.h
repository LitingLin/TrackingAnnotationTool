#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Base
{
	class Mutex
	{
	public:
		Mutex();
		~Mutex();
		void lock();
		bool try_lock();
		void unlock();
	private:
		CRITICAL_SECTION _handle;
	};

	class NamedMutex
	{
	public:
		NamedMutex(const wchar_t *name, bool acl = false);
		~NamedMutex();
		void lock();
		bool try_lock();
		bool unlock();
		HANDLE getHandle() const;
	private:
		HANDLE _handle;
	};
}