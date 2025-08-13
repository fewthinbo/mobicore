#pragma once
#include <cstdint>

namespace mobi_game {
	
	namespace consts {
		enum ELenghts {
			CHARACTER_NAME_MAX_LENGTH = 24,
			HOSTNAME_MAX_LENGTH = 21, //:$port eklenebilir
			USERNAME_MAX_LENGTH = 32,
			PASSWORD_MAX_LENGTH = 32,
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
		};
	
		enum class EDataUpdateTypes/*created for MSDataUpdate::type*/ : uint8_t {
			CREATE_PLAYER,
			DELETE_PLAYER,
			CREATE_GUILD,
			DELETE_GUILD,
		};
	
		enum class EUserCheckResponse : uint8_t {
			VALID,
			OFFLINE,
			BLOCKED,
			NOT_EXIST,
		};
	}
}