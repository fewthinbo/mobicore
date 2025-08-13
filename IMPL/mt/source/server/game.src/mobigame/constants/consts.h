#pragma once
#include <cstdint>

namespace mobi_game {
	extern bool IsBoss(uint32_t id);

	namespace consts {
#ifdef DEBUG
		static constexpr uint8_t MESSAGE_TIMEOUT{ 15 };
#else
		static constexpr uint8_t MESSAGE_TIMEOUT{ 120 };
#endif

		static constexpr uint16_t MAX_MESSAGE_COUNT{ 5000 };

		static constexpr uint16_t CLEANUP_INTERVAL{ 60 };

		static constexpr uint8_t DATA_TIMEOUT{ 180 };
		static constexpr uint16_t MAX_UNPROCESSED_COUNT{ 30000 };

		static constexpr uint16_t READ_FILE_INTERVAL = 300; // 5 dk

#ifdef PLATFORM_WINDOWS
		static constexpr uint16_t DEFAULT_CODEPAGE = 857; //console
#else
		static constexpr uint16_t DEFAULT_CODEPAGE = 1254; //windows-1254
#endif

		struct JFileNames {
#ifdef PLATFORM_FREEBSD
			static constexpr const char* BASE_FOLDER = "/usr/mobile/";
#else
			static constexpr const char* BASE_FOLDER = "";
#endif

			static constexpr const char* F_SETTINGS = "settings.json";
			static constexpr const char* F_INFO = "info.json";
			static constexpr const char* F_NOTIFICATIONS ="notifications.json";
		};

		struct JFields {
			//settings
			static constexpr const char* SETTINGS = "settings";
			static constexpr const char* MOBILE = "mobile";
			static constexpr const char* MT = "mt";
			static constexpr const char* SECURITY = "security";
			static constexpr const char* MAX_ACTIVE_SESSIONS = "max_active_sessions";
			static constexpr const char* IP_BLOCKLIST = "ip_blocklist";
			static constexpr const char* HEARTBEAT = "heartbeat";
			static constexpr const char* INTERVAL_SECONDS = "interval_seconds";
			static constexpr const char* MAX_MISSED_HEARTBEATS = "max_missed_heartbeats";
			static constexpr const char* LOGIN_POLICY = "login_policy";
			static constexpr const char* MAX_ATTEMPTS = "max_attempts";
			static constexpr const char* BLOCK_DURATION_MINUTES = "block_duration_minutes";
			static constexpr const char* RATE_LIMIT = "rate_limit";
			static constexpr const char* PER_SECOND = "per_second";
			static constexpr const char* MAX_OVERFLOWS = "max_overflows";
			static constexpr const char* DISABLED_PACKETS = "disabled_packets";
			static constexpr const char* TO_MT = "to_mt";
			static constexpr const char* TO_MOBILE = "to_mobile";
			static constexpr const char* ONLINE_COUNTER = "online_counter";
			static constexpr const char* ENABLED = "enabled";
			static constexpr const char* DURATION = "duration";
			static constexpr const char* MAINTENANCE_MODE = "maintenance_mode";
			static constexpr const char* MESSAGE = "message";
			static constexpr const char* REFRESH_INTERVAL_SECONDS = "refresh_interval_seconds";

			//info
			static constexpr const char* HOST = "host";
			static constexpr const char* PORT = "port";
			static constexpr const char* USER = "user";
			static constexpr const char* PASSWORD = "password";
			static constexpr const char* DB = "db";
			static constexpr const char* SERVER_BRIDGE = "server_bridge";
			static constexpr const char* PORT_TCP = "port_tcp";

			
			//notifications
			static constexpr const char* NOTIFICATIONS = "notifications";
			static constexpr const char* TITLE = "title";
			static constexpr const char* SUBTITLE = "subtitle";
			static constexpr const char* DETAILS = "details";
			static constexpr const char* ADMIN_NAME = "admin_name";
		};
	}
}