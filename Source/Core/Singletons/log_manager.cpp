#include "log_manager.h"


#if PLATFORM_FREEBSD
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#endif

namespace NSingletons {

#if PLATFORM_FREEBSD
	CLogManager::CLogManager() {
		InitializeLogDirectory();
		CleanupOldLogs();
	}

	CLogManager::~CLogManager() {
		for (auto& pair : log_files) {
			auto& log_file = pair.second;
			if (log_file.file.is_open()) {
				log_file.file.close();
			}
		}
	}

	void CLogManager::CleanupOldLogs() noexcept {
		namespace fs = std::filesystem;
		auto now = std::chrono::system_clock::now();

		try {
			// Iterate through all log directories
			for (const auto& pair : level_dirs) {
				auto& dir_name = pair.second;
				fs::path dir_path = fs::path(log_base_dir) / dir_name;

				if (!fs::exists(dir_path)) continue;

				// Check each file in the directory
				for (const auto& entry : fs::directory_iterator(dir_path)) {
					if (!entry.is_regular_file()) continue;

					auto file_time = fs::last_write_time(entry.path());
					auto file_time_sys = std::chrono::system_clock::now() +
						(file_time - fs::file_time_type::clock::now());
					auto age = std::chrono::duration_cast<std::chrono::hours>(
						now - file_time_sys).count();

					// Delete files older than retention period
					if (age > log_retention_days * 24) {
						fs::remove(entry.path());
						info("Deleted old log file: " + entry.path().string());
					}
				}
			}
		}
		catch (const std::exception& e) {
			error("Error cleaning up old logs: " + std::string(e.what()));
		}
	}

	CAbstractLogger::~CAbstractLogger() = default;

	void CLogManager::DoFileStuff(const LogLevel& level, const std::string& formatted_msg) {
		auto& log_file = OpenLogFile(level);

		log_file.file << formatted_msg;
		log_file.file.flush();
		log_file.line_count++;
	}

	std::string CLogManager::GetLogFilePath(LogLevel level, int file_number) const {
		std::stringstream ss;
		ss << log_base_dir << "/";
		if (level_dirs.find(level) != level_dirs.end()) {
			ss << level_dirs.at(level) << "/"
				<< level_dirs.at(level) << "_";
		}
		ss << std::setw(5) << std::setfill('0') << file_number << log_file_extension;
		return ss.str();
	}

	CLogManager::LogFile& CLogManager::OpenLogFile(LogLevel level, bool bInit) {
		namespace fs = std::filesystem;

		auto& log_file = log_files[level];

		std::stringstream ss{};
		ss << log_base_dir << "/";
		
		if (level_dirs.find(level) != level_dirs.end()){
			ss << level_dirs.at(level);
		}
		if (!fs::exists(ss.str())) {
			fs::create_directory(ss.str());
		}

		std::string filepath = GetLogFilePath(level, log_file.current_file_number);
		if (fs::exists(filepath)) {
			// Get file size in MB 
			auto file_size = fs::file_size(filepath) / (1024 * 1024);
			if (file_size >= static_cast<uintmax_t>(log_rotation_size)/*rotate*/
				|| log_file.line_count >= max_lines_per_file) {
				log_file.current_file_number++;
				if (log_file.file.is_open())
					log_file.file.close();
				log_file.line_count = 0;
			}
		}
		filepath = GetLogFilePath(level, log_file.current_file_number); //if there is any update

		if (log_file.file.is_open()) {
			return log_file;
		}
		else {
			log_file.file.open(filepath, std::ios::app);
			if (bInit) {
				log_file.file << "Log file created at " << GetCurrentTimestamp() << "\n";
				log_file.file.close();
			}
			else {
				if (!log_file.file.is_open()) {
					printf("CRITICAL: Failed to open log file: %s", filepath.c_str());
				}
			}
			return log_file;
		}
	}

	void CLogManager::InitializeLogDirectory() {
		namespace fs = std::filesystem;

		// Check base log directory
		if (!fs::exists(log_base_dir)) {
			fs::create_directory(log_base_dir);
			//there is no log files
			// Initialize log files without archiving
			for (auto& pair : log_files) {
				auto& level = pair.first;
				OpenLogFile(level, true);
			}
			return;
		}
		else/*if there is log folder then check if there any files to backup etc.*/ {
			bool bBackup = false;
			//initialize zip archive
			auto now = std::chrono::system_clock::now();
			auto time_t_now = std::chrono::system_clock::to_time_t(now);
			struct tm timeinfo;

			if (localtime_r(&time_t_now, &timeinfo) == nullptr) {
				return;
			}
			std::stringstream ss;
			ss << std::put_time(&timeinfo, "%Y%m%d_%H%M%S");

			// Create backup directory
			fs::path backup_dir = log_backup_base;
			if (!fs::exists(backup_dir)) {
				fs::create_directory(backup_dir);
			}

			std::string timeStamp = ss.str();

			fs::path archive_dir = fs::path(log_backup_base) / (log_backup_prefix + timeStamp);
			fs::create_directory(archive_dir);

			for (const auto& pair : level_dirs) {
				auto& level = pair.first;
				auto& dir_name = pair.second;
				fs::path log_dir = fs::path(log_base_dir) / dir_name;
				if (!fs::exists(log_dir)) {
					OpenLogFile(level, true);
					continue;
				}

				fs::path target_dir = archive_dir / dir_name;
				fs::create_directory(target_dir);

				for (const auto& entry : fs::directory_iterator(log_dir)) {
					if (entry.path().extension() != log_file_extension) {
						continue;
					}

					auto& log_file = log_files[level];
					if (log_file.file.is_open()) {
						log_file.file.close();
					}

					fs::path target_file = target_dir / entry.path().filename();
					fs::copy_file(entry.path(), target_file);
					fs::remove(entry.path());
					bBackup = true;
				}
			}
			if (!bBackup) {
				ss.clear();
				// Create archive filename
				ss << log_backup_base << "/" << log_backup_prefix << timeStamp << log_backup_extension;

				fs::remove(ss.str());
			}
		} // end of else
	}
#endif
}


