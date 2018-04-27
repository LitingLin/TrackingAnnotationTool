#include <base/debugoutput_logger_sink.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <base/utils.h>

namespace Base
{
	void debugoutput_sink::_sink_it(const spdlog::details::log_msg& msg)
	{
		OutputDebugString(UTF8ToUTF16(msg.formatted.str()).c_str());
	}

	void debugoutput_sink::_flush()
	{
	}
}