#pragma once
#if __MOBICORE__
#include <cstdint>

namespace mobi_game {
	//should be synced with bridgeServer/Common/tables.h
	enum class ENotificationChannels {
		SYSTEM,
		CHAT,
		FRIENDS,
		GUILD,
		EVENTS,
		MAX,
	};

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

		enum class ItemWindows {
			RESERVED_WINDOW,
			INVENTORY,
			EQUIPMENT,
			SAFEBOX,
			MALL,
			DRAGON_SOUL_INVENTORY,
			BELT_INVENTORY,
			GROUND,
#if __OFFSHOP__
			IKASHOP_OFFLINESHOP,
			IKASHOP_SAFEBOX,
			IKASHOP_AUCTION,
#endif
			WINDOW_TYPE_MAX,
		};

		enum ITEM_MAXS {
			MAX_SOCKETS_COUNT = 3, // max tas veya cevher slotu
			MAX_ATTR_COUNT = 5 + 2, // max efsun sayisi
			MAX_ITEM_COUNT = 200, // max item sayisi
			MAX_INVENTORY_PAGE_SLOT = 45,
			MAX_INVENTORY_PAGE_COUNT = 5,
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
			P2P_MANAGER = (1 << 3),
			DB_MANAGER = (1 << 4),
		};

#if __MT_DB_INFO__
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
#endif