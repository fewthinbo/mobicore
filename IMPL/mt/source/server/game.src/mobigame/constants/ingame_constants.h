#pragma once
#include <cstdint>
namespace mobi_game {
	namespace consts {
		/*How many characters can be created per account in your game?*/
		static constexpr uint8_t MAX_CHARACTER_COUNT = 4;

		/*What's the minimum core count except auth, db and game99 core in your server?
		* For example: If you've; auth, ch1_core1, ch1_core2, ch2_core1, game99, db cores then you've minimum 3 cores.*/
		static constexpr uint8_t MIN_CHANNEL_COUNT = 5;

		/*What's your account, player and common database names in sql?*/
		static constexpr const char* SCHEMA_ACCOUNT = "account";
		static constexpr const char* SCHEMA_PLAYER = "player";
		static constexpr const char* SCHEMA_COMMON = "common";

		enum ELengths {
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
			GM_IMPLEMENTOR,
			GM_DISABLE,
			MAX, //don't touch this
		};

		enum class ItemWindows {
			RESERVED_WINDOW,
			INVENTORY,
			EQUIPMENT,
			SAFEBOX,
			MALL,
			DRAGON_SOUL_INVENTORY,
			//#ifdef ENABLE_SPECIAL_STORAGE
			UPGRADE_INVENTORY,
			BOOK_INVENTORY,
			STONE_INVENTORY,
			CHANGE_INVENTORY,
			COSTUME_INVENTORY,
			CHEST_INVENTORY,
			//#endif
			//#ifdef ENABLE_SWITCHBOT
			SWITCHBOT,
			//#endif
			//#ifdef CHANGE_EQUIP_WORLDARD
			CHANGE_EQUIP,
			//#endif
			BELT_INVENTORY,
			GROUND,
			//#ifdef ENABLE_IKASHOP_RENEWAL
			IKASHOP_OFFLINESHOP,
			IKASHOP_SAFEBOX,
			IKASHOP_AUCTION,
			//#endif
			//#ifdef __AURA_SYSTEM__
			AURA_REFINE,
			//#endif
			WINDOW_TYPE_MAX,
		};

		/*What's the count of x,y grids in your offshop window?*/
		enum EOffshopMaxs {
			GRID_WIDTH = 10,
			GRID_HEIGHT = 8,
			DEFAULT_GRID_COUNT = GRID_WIDTH * GRID_HEIGHT * 2, //2 pages
		};

		enum ITEM_MAXS {
			MAX_SOCKETS_COUNT = 8,
			MAX_ATTR_COUNT = 15,
		};
	}

}