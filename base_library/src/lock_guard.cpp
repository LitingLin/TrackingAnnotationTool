#include <base/lock_guard.h>

namespace Base {
	SharedLockGuard_LockShared::SharedLockGuard_LockShared(std::shared_mutex& lock)
	{
		_lock = &lock;
		lock.lock_shared();
	}

	SharedLockGuard_LockShared::~SharedLockGuard_LockShared()
	{
		if (_lock)
			_lock->unlock_shared();
	}

	void SharedLockGuard_LockShared::unlock()
	{
		_lock->unlock_shared();
		_lock = nullptr;
	}

	SharedLockGuard::SharedLockGuard(std::shared_mutex& lock)
	{
		lock.lock();
		_lock = &lock;
	}

	SharedLockGuard::~SharedLockGuard()
	{
		if (_lock)
			_lock->unlock();
	}

	void SharedLockGuard::unlock()
	{
		_lock->unlock();
		_lock = nullptr;
	}
}