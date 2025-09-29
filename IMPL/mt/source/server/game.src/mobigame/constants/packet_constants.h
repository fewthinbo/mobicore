#pragma once
#include <cstdint>

namespace mobi_game {

	namespace consts {
		enum ELenghts {
			GUILD_NAME_MAX_LENGTH = 12,
			CHARACTER_NAME_MAX_LENGTH = 24,
			HOSTNAME_MAX_LENGTH = 21, //:$port eklenebilir
			USERNAME_MAX_LENGTH = 32,
			PASSWORD_MAX_LENGTH = 32,
			LASTPLAY_MAX_LENGTH = 32,
			EMAIL_MAX_LENGTH = 64,
		};

		enum EGMAuthorityLevel {
			GM_PLAYER,
			GM_LOW_WIZARD,
			GM_WIZARD,
			GM_HIGH_WIZARD,
			GM_GOD,
			GM_IMPLEMENTOR
		};

		enum class ENotificationSubHeader : uint8_t {
			ADD,
			REMOVE,
			LIST,
		};

		enum EAuthorityType : uint16_t {
			NONE = (0 << 0), //0
			NOTIFICATION_MANAGER = (1 << 1),
			CONFIG_MANAGER = (1 << 2),
			ONLINE_COUNTER = (1 << 3),
			DB_MANAGER = (1 << 4), //2^4
		};

#ifdef ENABLE_MT_DB_INFO
		enum class EDataUpdateTypes/*created for MSDataUpdate::type*/ : uint8_t {
			CREATE_PLAYER,
			DELETE_PLAYER,
			CREATE_GUILD,
			DELETE_GUILD,
		};
#endif

		enum class EUserCheckResponse : uint8_t {
			VALID,
			OFFLINE,
			BLOCKED,
			NOT_EXIST,
		};
	}
}