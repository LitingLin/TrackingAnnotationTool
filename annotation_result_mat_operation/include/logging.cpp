#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>

auto logger = std::make_shared<spdlog::logger>("null_logger", std::make_shared<spdlog::sinks::null_sink_st>());