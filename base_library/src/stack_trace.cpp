#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <Dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#include <string>
#include <limits>
#include <mutex>

namespace Base
{
	std::mutex stack_trace_locker;
	HANDLE hProcess = GetCurrentProcess();

	std::string getStackTrace()
	{
		std::string message;
		SYMBOL_INFO *symbol = static_cast<SYMBOL_INFO*>(malloc(sizeof(SYMBOL_INFO) + (MAX_SYM_NAME - 1) * sizeof(char)));
		memset(symbol, 0, sizeof(SYMBOL_INFO));

		symbol->MaxNameLen = MAX_SYM_NAME;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		const unsigned short max_frames = std::numeric_limits<unsigned short>::max();
		void **stacks = (void**)malloc(sizeof(void*) * max_frames);

		stack_trace_locker.lock();
		static bool stack_trace_initialized = false;
		if (!stack_trace_initialized)
		{
			if (!SymInitialize(hProcess, nullptr, TRUE))
			{
				message += ("Warning! SymInitialize() failed with "
					"Win32 Error Code: " + std::to_string(GetLastError()) + "\n");
			}
			stack_trace_initialized = true;
		}

		unsigned short frames = CaptureStackBackTrace(1, max_frames, stacks, nullptr);
		const unsigned address_buffer_length = sizeof(ptrdiff_t) * 2 + 1;
		char buffer[address_buffer_length];

		for (unsigned short i = 0; i < frames; ++i)
		{
			if (!SymFromAddr(hProcess, (DWORD64)stacks[i], 0, symbol))
			{
				message += "Warning! SymFromAddr() failed.\n"
					"Win32 Error Code: " + std::to_string(GetLastError()) + "\n";
			}
			snprintf(buffer, address_buffer_length, "%llx", symbol->Address);
			message += std::to_string(i) + "\t0x" + buffer + "\t" + (symbol->Name[0] == '\0' ? "(unknown)" : symbol->Name) + "\n";
		}
		stack_trace_locker.unlock();

		free(symbol);
		free(stacks);

		return message;
	}
}