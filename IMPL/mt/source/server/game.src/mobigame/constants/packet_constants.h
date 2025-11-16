#pragma once
#if __MOBICORE__
#include <cstdint>

namespace mobi_game {
	//should be synced with bridgeServer/Common/tables.h
	enum class ENotificationChannels : uint8_t {
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
#if __OFFSHOP__ == 1
			IKASHOP_OFFLINESHOP,
			IKASHOP_SAFEBOX,
			IKASHOP_AUCTION,
#endif
			WINDOW_TYPE_MAX,
		};

		enum ITEM_MAXS {
			MAX_SOCKETS_COUNT = 3, // max tas veya cevher slotu
			MAX_ATTR_COUNT = 5 + 2, // max efsun sayisi
			/*MAX_INVENTORY_PAGE_SLOT = 45,
			MAX_INVENTORY_PAGE_COUNT = 5,*/
		};

		static constexpr const uint8_t MAX_CHARACTER_COUNT = 4;

		enum class ENotificationSubHeader : uint8_t {
			ADD,
			REMOVE,
			LIST,
		};

		//Mt core authorities
		enum EAuthorityType : uint16_t {
			NONE = (0 << 0), //0
			NOTIFICATION_MANAGER = (1 << 1), //admin duyurularini gonderme yetkisi
			CONFIG_MANAGER = (1 << 2), //ayarlar dosyasini okur ve gonderir.
			P2P_MANAGER = (1 << 3), // send p2p related packets from single core.
			DB_MANAGER = (1 << 4), // read mobicore db info or update cache
			SM_MANAGER = (1 << 5), // receive and handle spesific bridge server packets 
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
	
	//TODO select it for your offshop and update macro in Makefile or CMakeLists.txt
	enum class EOffshopType : uint8_t {
		NONE = 0,
		IKARUS = 1,
	};

	enum class EMobiLoad : uint8_t {
		SUCCESS,
		LOADING,
		WRONG_PWD,
		INGAME_REAL,
		INVALID_LOCATION,
		PLAYER_LOAD_FAILED,
		NO_ACCOUNT_TABLE,
		INDEX_OVERFLOW,
		INVALID_PID,
		NAME_CHANGED,
		SHUTDOWN,
		USER_LIMIT,
		ID_NOT_EXISTS,
		NOT_AVAIL,
		BAD_SOCIAL_ID,
		AGE_LIMIT,
		BLOCKED,
		NOBILL,
		INVALID_STATUS,
		LOGOUT,
		NO_ACTIVITY,
		OTHERS, //you can use this one for extra failure types
	};

	enum class EResponseModify : uint8_t {
		SUCCESS,
		NOT_EXISTS_CH,
		OTHERS,
	};

#if __OFFSHOP__ == 1
	enum class EResponseShopOperation : uint8_t {
		SUCCESS,
		INGAME_REAL,
		NOT_EXISTS_CH,
		NOT_EXISTS_SHOP,
		NOT_EXISTS_ITEM,
		FLOOD,
		CHARACTER_ACTIONS,
		NOT_ENOUGH_SAFEBOX_SPACE, //remove
		NOT_ENOUGH_MONEY,//buy
		PRICE_MISMATCH, //buy
		INVALID_CELL, //move
		INVALID_SALE_PRICE, //edit price
		OTHERS,
	};
#endif

}
#endif