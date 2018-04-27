#pragma once

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

#include "event.h"

namespace Base
{
	class Runnable
	{
	public:
		virtual ~Runnable() = default;
		virtual int job_entry() = 0;
		virtual bool job_cancel() = 0; // true: exit signal posted; false: not acceptable or not support
	};

	class Thread
	{
	public:
		Thread();
		Thread(const Thread&) = delete;
		~Thread();
		void enableCrashNotify(); // thread crash before 
		void crashOnUnexpectedExit(bool enable); // need call cancel before thread exit
		void initialize(Runnable *runnable, bool runImmediately = true);
		void run();
		void pause();
		void join() const;
		bool join(uint32_t timeout) const;
		bool cancel();
		HANDLE getHandle() const;
		int getReturnCode() const;
		bool isRunning() const;
		bool isExceptionThrown() const;
		std::string getExceptionMessage() const;
		std::string getExceptionType() const;
		Runnable *getObject() const;
		HANDLE getCrashEventHandle() const;
	private:
		static unsigned __stdcall start_routine(void *para);
		Runnable *_runnable;
		struct CrashNofityContext
		{
			std::mutex lock;
			Event event;
		};
		bool _crashOnUnexpectedExit;
		std::atomic<bool> _is_canceled_by_user;
		std::unique_ptr<CrashNofityContext> _crashNofityContext;
		std::string _exp_type;
		std::string _exp_message;
		int _return_code;
		bool _exp_thrown;
		HANDLE _handle;
	};

	class WorkerWrapper
	{
	public:
		WorkerWrapper(std::unique_ptr<Runnable> runnable, bool crashOnUnexpectedExit = false, bool runImmediately = true, bool enableCrashNotify = false);
		WorkerWrapper(const WorkerWrapper&) = delete;
		HANDLE getCrashEventHandle() const;
		void run();
		void pause();
		void join() const;
		bool join(uint32_t timeout) const;
		bool cancel();
		bool isRunning() const;
		HANDLE getHandle() const;
		int getReturnCode() const;
		bool isExceptionThrown() const;
		std::string getExceptionMessage() const;
		std::string getExceptionType() const;
		Runnable *getObject() const;
	private:
		std::unique_ptr<Runnable> _runnable;
		Thread _thread;
	};

	class ThreadSynchronization
	{
	public:
		static void join(Thread *threads, unsigned n, unsigned *i);
		static bool join(Thread *threads, unsigned n, uint32_t timeout, unsigned *i);
		static void joinAll(Thread *threads, unsigned n);
		static bool joinAll(Thread *threads, unsigned n, uint32_t timeout);
	private:
		static std::vector<HANDLE> getHandles(Thread* threads, unsigned n);
	};
}