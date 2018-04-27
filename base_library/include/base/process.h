#pragma once
#include "event.h"
#include <mutex>
#include <vector>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <concurrent_queue.h>
#include <map>

namespace Base
{
	class OpenProcessGuard
	{
	public:
		OpenProcessGuard(DWORD dwDesiredAccess, DWORD dwProcessId);

		~OpenProcessGuard();

		HANDLE getHANDLE() const;
	private:
		HANDLE handle;
	};

	class Process
	{
	public:
		Process(const wchar_t *application, const wchar_t *command = nullptr);
		Process(const Process &) = delete;
		Process(Process &&other) noexcept;
		~Process();
		HANDLE getHandle() const;
		unsigned long getId() const;
		bool isActive() const;
		unsigned long getExitCode() const;
	private:
		HANDLE _processHandle;
	};

	class ProcessMonitor
	{
	public:
		ProcessMonitor();
		ProcessMonitor(const ProcessMonitor&) = delete;
		ProcessMonitor(ProcessMonitor&&) = delete;
		~ProcessMonitor();
		void add(HANDLE process_handle);
		HANDLE getEventHandle() const;
		HANDLE pickLastExitProcessHandle();
	private:
		Concurrency::concurrent_queue<HANDLE> _exitedProcessHandle;
		Event _event;
		HANDLE _finalize_signal;
		std::mutex _mutex;
		std::map<DWORD,HANDLE> _running_threads;
		unsigned static __stdcall monitor_worker(void *);
	};

	std::vector<uint32_t> getSystemProcessesIds();
	std::vector<HMODULE> getProcessModules(HANDLE hProcess);
	std::wstring getModuleFullPath(HANDLE hProcess, HMODULE hModule);
	std::wstring getProcessFullPath(HANDLE hProcess);
	std::wstring getProcessName(HANDLE hProcess);

	std::wstring enumerateAllProcessInfomation();
}
