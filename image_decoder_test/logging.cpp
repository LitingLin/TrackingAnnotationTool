#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/spdlog.h>

#include <base/debugoutput_logger_sink.h>
#include <base/message_box.h>
#include <base/utils.h>

std::shared_ptr<spdlog::logger> logger;

class LoggerInitializer
{
public:
	LoggerInitializer()
	{
		std::string loggingName = "image_viewer_domo logger";
		try {
			logger = std::make_shared<spdlog::logger>(loggingName, std::make_shared<Base::debugoutput_sink>());
		}
		catch (spdlog::spdlog_ex &exp)
		{
			Base::showMessageBox_Error(Base::UTF8ToUTF16(exp.what()).c_str(), L"Logger initialization failed");
			exit(1);
		}
	}
	~LoggerInitializer()
	{
		logger.reset();
	}
}g_logger_initializer;