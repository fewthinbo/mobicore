#include "abstract_logger.h"

#include <sstream>
#include <iomanip>

namespace NSingletons {
	std::string CAbstractLogger::TimeToStr(const std::chrono::system_clock::time_point& _tm) const {
		time_t nTime = std::chrono::system_clock::to_time_t(_tm);
		struct tm timeinfo {};
#if PLATFORM_WINDOWS
		localtime_s(&timeinfo, &nTime);
#else
		localtime_r(&nTime, &timeinfo);
#endif
		std::stringstream ss;
		ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
		return ss.str();
	}

	std::string CAbstractLogger::GetCurrentTimestamp(bool w_milli_seconds) const {
		auto now = std::chrono::system_clock::now();
		time_t now_time = std::chrono::system_clock::to_time_t(now);
		struct tm timeinfo;
#if PLATFORM_WINDOWS
		localtime_s(&timeinfo, &now_time);
#else
		localtime_r(&now_time, &timeinfo);
#endif
		//2025.03.25 21:27:40
		std::stringstream ss;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			now.time_since_epoch()) % 1000;

		if (w_milli_seconds) {
			ss << std::put_time(&timeinfo, "%Y.%m.%d %H:%M:%S");
			ss << '.' << std::setw(3) << std::setfill('0') << ms.count();
		}
		else/*mobile DateTime compatible*/ {
			ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
		}


		return ss.str();
	}
}