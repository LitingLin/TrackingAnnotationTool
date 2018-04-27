#include <base/mutex.h>

#include <base/logging.h>
#include <base/security_attributes.h>

namespace Base
{
	Mutex::Mutex()
	{
		InitializeCriticalSection(&_handle);
	}

	Mutex::~Mutex()
	{
		DeleteCriticalSection(&_handle);
	}

	void Mutex::lock()
	{
		EnterCriticalSection(&_handle);
	}

	bool Mutex::try_lock()
	{
		return TryEnterCriticalSection(&_handle);
	}

	void Mutex::unlock()
	{
		LeaveCriticalSection(&_handle);
	}

	NamedMutex::NamedMutex(const wchar_t* name, bool acl)
	{
		if (acl)
			_handle = CreateMutex(nullptr, FALSE, name);
		else
		{
			AllowAllSecurityATTRIBUTES security_attributes;
			_handle = CreateMutex(security_attributes.get(), FALSE, name);
		}

		ENSURE_WIN32API(_handle);
	}

	NamedMutex::~NamedMutex()
	{
		CloseHandle(_handle);
	}

	void NamedMutex::lock()
	{
		ENSURE_EQ_WIN32API(WaitForSingleObject(_handle, INFINITE), WAIT_OBJECT_0);
	}

	bool NamedMutex::try_lock()
	{
		const DWORD rc = WaitForSingleObject(_handle, 0);
		if (rc == WAIT_OBJECT_0)
			return true;
		else if (rc == WAIT_TIMEOUT)
			return false;
		else
			UNREACHABLE_ERROR;
		return false;
	}

	bool NamedMutex::unlock()
	{
		return ReleaseMutex(_handle);
	}

	HANDLE NamedMutex::getHandle() const
	{
		return _handle;
	}
}