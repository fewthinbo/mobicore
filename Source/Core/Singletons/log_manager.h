#pragma once

#include "Abstract/abstract_logger.h"
#include <singleton.h>


#include <string>
#include <map>
#ifdef PLATFORM_FREEBSD
#include <fstream>
#endif

namespace NSingletons {
    class CLogManager : public CSingleton<CLogManager>, public CAbstractLogger {
#ifdef PLATFORM_FREEBSD
        struct LogFile {
            std::ofstream file;
            size_t line_count;
            int current_file_number;

            LogFile() : line_count(0), current_file_number(0) {}
        };
        void InitializeLogDirectory();
        void CleanupOldLogs() noexcept;
        LogFile& OpenLogFile(LogLevel level, bool bInit = false);
        std::string GetLogFilePath(LogLevel level, int file_number) const;
    protected:
        void DoFileStuff(const LogLevel& level, const std::string& formatted_msg) override;
    public:
        CLogManager();
        ~CLogManager();
    private:
        static constexpr size_t max_lines_per_file = 2000;
        static constexpr const char* log_base_dir = "mobile_logs";
        static constexpr const char* log_file_extension = ".txt";

        static constexpr const char* log_backup_base = "mobile_log_backups";
        static constexpr const char* log_backup_prefix = "logs_";
        static constexpr const char* log_backup_extension = ".zip";

        static constexpr uint8_t log_retention_days = 30;
        static constexpr uint8_t log_rotation_size = 10;
        const std::map<LogLevel, std::string> level_dirs = {
            {LogLevel::TRACE, "trace"},
            {LogLevel::INFO, "info"},
            {LogLevel::WARN, "warnings"},
            {LogLevel::ERR, "errors"},
            {LogLevel::FATAL, "fatal"},
        };
        std::map<LogLevel, LogFile> log_files;
#endif
    };
}

#define loggerInstance NSingletons::CLogManager::getInstance()

#define LOG_FATAL(message, ...) NSingletons::CLogManager::getInstance().fatal("? : ?", __FUNCTION__, message, ##__VA_ARGS__)
#define LOG_ERR(message, ...) NSingletons::CLogManager::getInstance().error("? : ?", __FUNCTION__, message, ##__VA_ARGS__)

#ifdef _DEBUG
#define LOG_INFO(message, ...) NSingletons::CLogManager::getInstance().info("? : ?", __FUNCTION__, message, ##__VA_ARGS__)
#define LOG_TRACE(message, ...) NSingletons::CLogManager::getInstance().trace("? : ?", __FUNCTION__, message, ##__VA_ARGS__)
#define LOG_WARN(message, ...) NSingletons::CLogManager::getInstance().warn("? : ?", __FUNCTION__, message, ##__VA_ARGS__)
#else
#define DO_NOTHING do {} while(0) //not void to supress warnings
#define LOG_INFO(message, ...) DO_NOTHING
#define LOG_TRACE(message, ...) DO_NOTHING
#define LOG_WARN(message, ...) DO_NOTHING
#endif
