#pragma once

#include <spdlog/logger.h>

namespace Base {
	class debugoutput_sink : public spdlog::sinks::base_sink<std::mutex>
	{
	protected:
		void _sink_it(const spdlog::details::log_msg& msg) override;
		void _flush() override;
	};
}