#pragma once
#include <string>
#include <chrono>
#include <sstream>
#include <string_view>

enum class LogLevel {
	TRACE, // Debug logs
	INFO,
	WARN,
	ERR,
	FATAL,
};

namespace NSingletons {
	class CAbstractLogger {
	protected:
		const char* GetLogLevelStr(const LogLevel& level) const {
			switch (level) {
			case LogLevel::TRACE: return "TRACE";
			case LogLevel::INFO: return "INFO";
			case LogLevel::WARN: return "WARN";
			case LogLevel::ERR: return "ERROR";
			case LogLevel::FATAL: return "FATAL";
			default: return "";
			}
		}
#ifdef PLATFORM_FREEBSD
	protected:
		virtual void DoFileStuff(const LogLevel& level, const std::string& formatted_msg) = 0;
		//virtual ~CAbstractLogger() = default; //avoiding silence breaks.
		virtual ~CAbstractLogger();
#endif
	public:
		std::string GetCurrentTimestamp(bool w_milli_seconds = false) const;
		std::string TimeToStr(const std::chrono::system_clock::time_point& _tm) const;

		template<typename ... Args>
		std::string WriteBuf(std::string_view message, Args&& ... args) {
			if (message.empty()) return "";
			std::string converted = message.data();
			constexpr size_t argCount = sizeof...(args);
			if constexpr (argCount > 0) {
				(([&](auto&& arg) {
					size_t pos = converted.find('?');
					if (pos != std::string::npos) {
						using cleanType = std::decay_t<decltype(arg)>;
						std::ostringstream ss;
						if constexpr (std::is_enum_v<cleanType>) {
							ss << static_cast<long>(arg);
						}
						else if constexpr (std::is_same_v<uint8_t, cleanType>) /*sstream unsigned char fix*/ {
							ss << static_cast<int>(arg);
						}
						else if constexpr (std::is_same_v<std::chrono::milliseconds, cleanType>
							|| std::is_same_v<std::chrono::seconds, cleanType>
							|| std::is_same_v<std::chrono::minutes, cleanType>
							) {
							ss << static_cast<long long>(arg.count());
						}
						else {
							ss << arg;
						}
						converted.replace(pos, 1, ss.str());
					}
					}(args)), ...);
			}

			return converted;
		}

		template<typename ... Args>
		void log(const LogLevel& level, std::string_view message, Args&&... args) {
			if (message.empty()) return;

			std::string copiedMsg = WriteBuf(message, std::forward<Args>(args)...);

			std::string timeStamp = GetCurrentTimestamp(true); //rvo, doesn't need to use move
			std::string levelStr = GetLogLevelStr(level);

			// Format the log message for file
			std::string formatted_message;
			formatted_message.reserve(timeStamp.size() + levelStr.size() + copiedMsg.size() + 4); //tahmin

			formatted_message.append(timeStamp);
			formatted_message.append(" [");
			formatted_message.append(levelStr);
			formatted_message.append("] ");
			formatted_message.append(copiedMsg);
			formatted_message.push_back('\n');

			formatted_message.shrink_to_fit();

#ifdef PLATFORM_FREEBSD
			DoFileStuff(level, formatted_message);
#endif
#ifdef PLATFORM_WINDOWS
			printf("%s\n", formatted_message.c_str());
#endif
		}

		template<typename ... Args>
		void trace(std::string_view message, Args&&... args) {
			log(LogLevel::TRACE, message, std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void info(std::string_view message, Args&&... args) {
			log(LogLevel::INFO, message, std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void warn(std::string_view message, Args&&... args) {
			log(LogLevel::WARN, message, std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void error(std::string_view message, Args&&... args) {
			log(LogLevel::ERR, message, std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void fatal(std::string_view message, Args&&... args) {
			log(LogLevel::FATAL, message, std::forward<Args>(args)...);
		}
	};
}