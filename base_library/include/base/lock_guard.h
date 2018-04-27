#pragma once
#include <shared_mutex>

namespace Base
{
	class SharedLockGuard_LockShared
	{
	public:
		SharedLockGuard_LockShared(std::shared_mutex &lock);
		~SharedLockGuard_LockShared();
		void unlock();
	private:
		std::shared_mutex *_lock;
	};

	class SharedLockGuard
	{
	public:
		SharedLockGuard(std::shared_mutex &lock);
		~SharedLockGuard();
		void unlock();
	private:
		std::shared_mutex *_lock;
	};
}