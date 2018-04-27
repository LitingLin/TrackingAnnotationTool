#include <base/thread.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <utility>

#include <base/logging.h>
#include <base/utils.h>

namespace Base
{
	Thread::Thread()
		: _runnable(nullptr), _crashOnUnexpectedExit(false), _is_canceled_by_user(false), _return_code(0), _exp_thrown(false), _handle(nullptr)
	{
	}

	Thread::~Thread()
	{
		if (_runnable) {
			try
			{
				cancel();
			}
			catch (...) {}
			join();
			LOG_IF_FAILED_WIN32API(CloseHandle(_handle));
		}
	}

	void Thread::enableCrashNotify()
	{
		_crashNofityContext = std::make_unique<CrashNofityContext>();
	}

	void Thread::crashOnUnexpectedExit(bool enable)
	{
		_crashOnUnexpectedExit = enable;
	}

	void Thread::initialize(Runnable* runnable, bool runImmediately)
	{
		_runnable = runnable;
		if (runImmediately)
			_handle = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, start_routine, this, 0, NULL));
		else
			_handle = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, start_routine, this, CREATE_SUSPENDED, NULL));

		ENSURE_NE_CRTAPI(reinterpret_cast<uintptr_t>(_handle), uintptr_t(-1));
	}

	void Thread::run()
	{
		ENSURE_NE_WIN32API(ResumeThread(_handle), (DWORD)-1);
	}

	void Thread::pause()
	{
		ENSURE_NE_WIN32API(SuspendThread(_handle), (DWORD)-1);
	}

	void Thread::join() const
	{
		ENSURE_EQ_WIN32API(WaitForSingleObjectEx(_handle, INFINITE, FALSE), WAIT_OBJECT_0);
	}

	bool Thread::join(uint32_t timeout) const
	{
		uint32_t rc = WaitForSingleObject(_handle, timeout);
		if (rc == WAIT_TIMEOUT)
			return false;
		ENSURE_EQ_WIN32API(rc, WAIT_OBJECT_0);
		return true;
	}

	bool Thread::cancel()
	{
		_is_canceled_by_user = true;
		return _runnable->job_cancel();
	}

	HANDLE Thread::getHandle() const
	{
		return _handle;
	}

	int Thread::getReturnCode() const
	{
		return _return_code;
	}

	bool Thread::isRunning() const
	{
		DWORD rc = WaitForSingleObject(_handle, 0);
		if (rc == WAIT_OBJECT_0)
			return false;
		else if (rc == WAIT_TIMEOUT)
			return true;
		else {
			UNREACHABLE_ERROR;
			return false;
		}
	}

	bool Thread::isExceptionThrown() const
	{
		return _exp_thrown;
	}

	std::string Thread::getExceptionMessage() const
	{
		return _exp_message;
	}

	std::string Thread::getExceptionType() const
	{
		return _exp_type;
	}

	Runnable* Thread::getObject() const
	{
		return _runnable;
	}

	HANDLE Thread::getCrashEventHandle() const
	{
		return _crashNofityContext->event.getHandle();
	}

	unsigned Thread::start_routine(void* para)
	{
		Thread *this_class = static_cast<Thread*>(para);
		try {
			this_class->_return_code = this_class->_runnable->job_entry();
		}
		catch (const std::exception &exp)
		{
			this_class->_exp_thrown = true;
			this_class->_exp_type = typeid(exp).name();
			this_class->_exp_message = exp.what();
		}
		if (this_class->_crashOnUnexpectedExit)
		{
			if (!this_class->_is_canceled_by_user)
			{
				throw FatalError("Thread terminated unexpected", -1L, ErrorCodeType::USERDEFINED);
			}
		}
		if (this_class->_crashNofityContext)
		{
			std::lock_guard<std::mutex> lock_guard(this_class->_crashNofityContext->lock);
			if (!this_class->_is_canceled_by_user)
			{
				this_class->_crashNofityContext->event.set();
			}
		}

		return 0;
	}

	WorkerWrapper::WorkerWrapper(std::unique_ptr<Runnable> runnable, bool crashOnUnexpectedExit, bool runImmediately, bool enableCrashNotify)
		: _runnable(std::move(runnable))
	{
		if (crashOnUnexpectedExit)
			_thread.crashOnUnexpectedExit(true);
		if (enableCrashNotify)
			_thread.enableCrashNotify();
		_thread.initialize(runnable.get(), runImmediately);
	}

	HANDLE WorkerWrapper::getCrashEventHandle() const
	{
		return _thread.getCrashEventHandle();
	}

	void WorkerWrapper::run()
	{
		_thread.run();
	}

	void WorkerWrapper::pause()
	{
		_thread.pause();
	}

	void WorkerWrapper::join() const
	{
		_thread.join();
	}

	bool WorkerWrapper::join(uint32_t timeout) const
	{
		return _thread.join(timeout);
	}

	bool WorkerWrapper::cancel()
	{
		return _thread.cancel();
	}

	bool WorkerWrapper::isRunning() const
	{
		return _thread.isRunning();
	}

	HANDLE WorkerWrapper::getHandle() const
	{
		return _thread.getHandle();
	}

	int WorkerWrapper::getReturnCode() const
	{
		return _thread.getReturnCode();
	}

	bool WorkerWrapper::isExceptionThrown() const
	{
		return _thread.isExceptionThrown();
	}

	std::string WorkerWrapper::getExceptionMessage() const
	{
		return _thread.getExceptionMessage();
	}

	std::string WorkerWrapper::getExceptionType() const
	{
		return _thread.getExceptionType();
	}

	Runnable* WorkerWrapper::getObject() const
	{
		return _thread.getObject();
	}

	void ThreadSynchronization::join(Thread* threads, unsigned n, unsigned* i)
	{
		std::vector<HANDLE> handles = getHandles(threads, n);
		uint32_t rc = WaitForMultipleObjects(n, &handles[0], FALSE, INFINITE);
		ENSURE_GE_WIN32API(rc, WAIT_OBJECT_0);
		ENSURE_LE_WIN32API(rc, WAIT_OBJECT_0 + n - 1);
		*i = rc - WAIT_OBJECT_0;
	}

	bool ThreadSynchronization::join(Thread* threads, unsigned n, uint32_t timeout, unsigned* i)
	{
		std::vector<HANDLE> handles = getHandles(threads, n);
		uint32_t rc = WaitForMultipleObjects(n, &handles[0], FALSE, timeout);
		if (rc == WAIT_TIMEOUT)
			return false;
		ENSURE_GE_WIN32API(rc, WAIT_OBJECT_0);
		ENSURE_LE_WIN32API(rc, WAIT_OBJECT_0 + n - 1);
		*i = rc - WAIT_OBJECT_0;
		return true;
	}

	void ThreadSynchronization::joinAll(Thread* threads, unsigned n)
	{
		std::vector<HANDLE> handles = getHandles(threads, n);
		uint32_t rc = WaitForMultipleObjects(n, &handles[0], TRUE, INFINITE);
		ENSURE_GE_WIN32API(rc, WAIT_OBJECT_0);
		ENSURE_LE_WIN32API(rc, WAIT_OBJECT_0 + n - 1);
	}

	bool ThreadSynchronization::joinAll(Thread* threads, unsigned n, uint32_t timeout)
	{
		std::vector<HANDLE> handles = getHandles(threads, n);
		uint32_t rc = WaitForMultipleObjects(n, &handles[0], TRUE, timeout);
		if (rc == WAIT_TIMEOUT)
			return false;
		ENSURE_GE_WIN32API(rc, WAIT_OBJECT_0);
		ENSURE_LE_WIN32API(rc, WAIT_OBJECT_0 + n - 1);
		return true;
	}

	std::vector<HANDLE> ThreadSynchronization::getHandles(Thread* threads, unsigned n)
	{
		ENSURE_NE(n, 0U);
		ENSURE_LE(n, unsigned(MAXIMUM_WAIT_OBJECTS));
		std::vector<HANDLE> handles;
		handles.reserve(n);
		for (unsigned i = 0; i < n; ++i)
			handles.push_back(threads[i].getHandle());
		return handles;
	}
}