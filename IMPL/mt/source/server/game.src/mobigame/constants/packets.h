#pragma once
#include <Network/common.h>
#include "packet_constants.h"

using namespace network;

namespace mobi_game {
	// SM: Server(Bridge) to mt
	// MS : mt to Server

	enum Header {
		HEADER_MS_LOGIN = 1, //game login
		HEADER_MS_LOGOUT, //game logout

		HEADER_SM_MESSAGE, //mobile to game message
		HEADER_MS_MESSAGE, //game to mobile message

		HEADER_MS_MESSAGE_NAME, // because of mt whisper packet, game to mobile message

		HEADER_MS_SHOUT, //game to mobile shout
		HEADER_SM_LOGIN, //mobile login
		HEADER_SM_LOGOUT, //mobil logout

		HEADER_MS_ONLINE, //online counter

		HEADER_MS_DATA_UPDATE, //cache invalidation etc.

		HEADER_MS_USER_CHECK, // @deprecated oyundan mobile mesaj atilmak isteniyor, mesaj paketi dogrudan gonderilmeden once mobilde aktif mi kontrol edelim.
		HEADER_SM_USER_CHECK, // bridge sunucunun cevabi

		HEADER_MS_GUILD_JOIN, //guild join
		HEADER_MS_GUILD_LEAVE, //guild leave

		HEADER_MS_GUILD_POINT, //ladder point
		HEADER_MS_GUILD_STATS, //guild stats: win, loss etc.

		HEADER_MS_MESSENGER_ADD, //messenger add
		HEADER_MS_MESSENGER_REMOVE, //messenger remove

		HEADER_MS_LEVEL_UP, //level up

		HEADER_MS_KEY_EXCHANGE,
		HEADER_SM_KEY_EXCHANGE,

		HEADER_SM_DB_INFO, //db info get request
		HEADER_MS_DB_INFO, //db info response

		HEADER_MS_GUILD_WAR,

		HEADER_MS_ADMIN_NOTIFICATION, //admin notification
		HEADER_SM_CORE_AUTHORITY, //core authority, admin data manager etc.

		HEADER_MS_KILL, //kill, friends notifications etc.

		HEADER_MS_CONFIG, //data_config.json settings field'i bildirilir

		HEADER_MS_CHARACTER,

		HEADER_MS_SYNC, //baglanti kopmalari durumunda bazi verilerin senkronize olmasi icin

		HEADER_SM_CACHE_STATUS, //bridge'teki cache durumu

		HEADER_SM_FORWARD, //custom admin panel packets with parameters

		HEADER_MS_MOBILE_NOTIFY, //mt -> mobile notification
		HEADER_SM_VALIDATE_LOGIN, //validate: mobile login credentials
		HEADER_MS_VALIDATE_LOGIN, //mt server response

#ifndef ENABLE_MT_DB_INFO
		HEADER_SM_GET_CACHE, //sadece yeni acc kayitlari getirmek ve cache senkronizasyonu icin kullanilir.
#endif

		HEADER_MAX
	};

	enum class ESubGuildWar : uint8_t {
		NONE,
		WAR_START,
		WAR_END,
		PLAYER_KILL,
		PLAYER_JOIN,
		PLAYER_LEAVE,
		PLAYER_POSITION_UPDATE,
		NOTIFICATION,
	};

	enum class ESubCharacter : uint8_t {
		CHANGE_RACE,
		CHANGE_EMPIRE,
		CHANGE_SEX,
		CHANGE_NAME,
	};

#ifndef ENABLE_MT_DB_INFO
	enum class ECacheType : uint8_t {
		ACCOUNT,
		PLAYER,
		GUILD,
		GUILD_MEMBER,
		MESSENGER,
		CACHE_TYPE_MAX,
	};
#endif

#pragma pack(push, 1)
	struct MSDBInfo {
		THEADER header = HEADER_MS_DB_INFO;
#ifndef ENABLE_MT_DB_INFO
		TSIZE size{};
		TSIZE cache_sizes[static_cast<uint8_t>(ECacheType::CACHE_TYPE_MAX)]{};
#else
		char host[consts::HOSTNAME_MAX_LENGTH + 1]{};
		char user[consts::USERNAME_MAX_LENGTH + 1]{};
		char password[consts::PASSWORD_MAX_LENGTH + 1]{};
#endif
	};

	struct SMValidateMobileLogin {
		THEADER header = HEADER_SM_VALIDATE_LOGIN;
		TSIZE size{}; //id(char), pw(char)
	};

	struct MSValidateMobileLogin {
		THEADER header = HEADER_MS_VALIDATE_LOGIN;
		bool is_valid{};
		uint32_t acc_id{};
	};

#ifndef ENABLE_MT_DB_INFO
	struct MSDataUpdate {
		THEADER header = HEADER_MS_DATA_UPDATE;
		TSIZE size{};
		uint8_t cache_type{};
		bool is_invalidate{};
	};

	struct SMGetCache {
		THEADER header = HEADER_SM_GET_CACHE;
		uint32_t acc_id{};
	};

	//mt db'sinden ara sunucuya aktarilacak veri tipleri
	namespace MSCache {
		struct Account {
			char login[consts::USERNAME_MAX_LENGTH + 1]{};
			uint32_t acc_id{};
			uint8_t empire{};
			char email[consts::EMAIL_MAX_LENGTH + 1]{};
			uint8_t authority{};
		};

		struct Player {
			uint32_t pid{};
			uint32_t acc_id{};
			char name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
			uint8_t race{};
			uint32_t map_idx{};
			uint8_t level{};
			uint32_t play_time{};
			char last_play[consts::LASTPLAY_MAX_LENGTH + 1]{};
			uint32_t guild_id{};
			bool is_guild_leader{};
		};

		struct Guild {
			char name[consts::GUILD_NAME_MAX_LENGTH + 1]{};
			int32_t id{};
			uint32_t master{};
			uint16_t level{};
			uint32_t win{}, draw{}, loss{};
			int32_t ladder_point{};
		};

		struct GuildMember {
			uint32_t pid{};
			int32_t guild_id{};
		};

		struct MessengerList {
			char account[consts::USERNAME_MAX_LENGTH + 1]{};
			char companion[consts::USERNAME_MAX_LENGTH + 1]{};
		};
	};
#else //enable db info
	//cache update & invalidate
	struct MSDataUpdate {
		THEADER header = HEADER_MS_DATA_UPDATE;
		uint32_t id{};
		uint8_t type{};
	};
#endif

	struct SMMessage {
		THEADER header = HEADER_SM_MESSAGE;
		TSIZE size{}; // message character count : 40
		char name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
		uint32_t receiver_pid{};
	};

	struct TKeyExchange {
		THEADER header{};
		TSIZE size{};
	};

	struct SMLogin {
		THEADER header = HEADER_SM_LOGIN;
		char name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{}; //mobile giriş yapan kullanıcının ismi
	};

	struct SMLogout {
		THEADER header = HEADER_SM_LOGOUT;
		char name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
	};

	struct MSLadderPoint {
		THEADER header = HEADER_MS_GUILD_POINT;
		uint32_t guild_id{};
		uint32_t point{};
	};

	struct MSGuildStats {
		THEADER header = HEADER_MS_GUILD_STATS;
		uint32_t guild_id{};
		uint32_t win{};
		uint32_t draw{};
		uint32_t loss{};
	};

	struct MSKill {
		THEADER header = HEADER_MS_KILL;
		uint32_t killer_pid{};
		uint32_t victim_pid{};
	};

	struct MSMessage {
		THEADER header = HEADER_MS_MESSAGE;
		TSIZE size{}; // message character count
		uint32_t sender_pid{};
		uint32_t receiver_pid{};
		uint16_t code_page{};
	};

	struct MSLogin {
		THEADER header = HEADER_MS_LOGIN;
		uint32_t pid{};
		uint32_t map_idx{};
	};

	struct MSLogout {
		THEADER header = HEADER_MS_LOGOUT;
		uint32_t pid{};
	};

	struct MSOnline {
		THEADER header = HEADER_MS_ONLINE;
		uint32_t empires[3]{};
	};

	struct MSShout {
		THEADER header = HEADER_MS_SHOUT;
		TSIZE size{};
		uint32_t pid{};
		uint16_t code_page{};
	};

	struct MSMessageName {
		THEADER header = HEADER_MS_MESSAGE_NAME;
		TSIZE size{};
		uint32_t sender_pid{};
		char recevier_name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
		uint16_t code_page{};
	};

	struct MSUserCheck {
		THEADER header = HEADER_MS_USER_CHECK;
		char name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
		uint32_t sender_pid{};
		uint32_t container_id{};
	};

	struct SMUserCheck {
		THEADER header = HEADER_SM_USER_CHECK;
		uint32_t container_id{};
		uint8_t response{}; //EUserCheckResponse
	};

	struct MSLevelUp {
		THEADER header = HEADER_MS_LEVEL_UP;
		uint32_t pid{};
		uint32_t level{};
	};

	struct MSGuildLeave {
		THEADER header = HEADER_MS_GUILD_LEAVE;
		uint32_t pid{};
	};

	struct MSGuildJoin {
		THEADER header = HEADER_MS_GUILD_JOIN;
		uint32_t guild_id{};
		uint32_t pid{};
		bool b_leader{};
	};

	struct MSMessengerAdd {
		THEADER header = HEADER_MS_MESSENGER_ADD;
		char name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
		char target[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
	};

	struct MSMessengerRemove {
		THEADER header = HEADER_MS_MESSENGER_REMOVE;
		char name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
		char target[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
	};

	//guild war subheader packets
	struct MSGuildWar {
		THEADER header = HEADER_MS_GUILD_WAR;
		TSIZE size{};
		uint16_t subheader{};
	};

	struct SGuildWarStart {
		uint32_t guild_id[2]{};
		uint32_t scoreLimit{};
	};

	struct SGuildWarEnd {
		uint32_t guild_id[2]{};
	};

	struct SGuildWarPlayerKill {
		uint32_t guild_id[2]{};
		uint32_t killer_pid{};
		uint32_t victim_pid{};
	};

	struct SGuildWarPlayerJoin {
		uint32_t guild_id{};
		uint32_t pid{};
	};

	struct SGuildWarPlayerLeave {
		uint32_t guild_id{};
		uint32_t pid{};
	};

	struct SGuildWarPlayerPositionUpdate {
		uint32_t guild_id{};
		uint32_t pid{};
		uint32_t pos[2]{};
	};

	struct SGuildWarNotification {
		uint32_t guild_id[2]{};
	};
	//end of guild war subheader packets

	struct MSNotification {
		THEADER header = HEADER_MS_ADMIN_NOTIFICATION;
		TSIZE size{}; // sub paketin -dinamik- boyutunu belirtir, varliginin sebebi doRead'de verinin okunmasidir.
		uint8_t subheader{};
	};

	struct SMCoreAuthority {
		THEADER header = HEADER_SM_CORE_AUTHORITY;
		uint16_t authority_type{};
	};

	struct MSConfig {
		THEADER header = HEADER_MS_CONFIG;
		TSIZE size{}; //size of the whole config
		TSIZE security_size{}; //size of the ipblock list
		TSIZE dp_to_mt_size{}; //size of the disabled_packets of mt
		TSIZE dp_to_mobile_size{}; //size of the disabled_packets of mobile
		TSIZE maintenance_notification_size{}; //size of the maintenance_notification
	};

	struct MSConfigStatic {
		uint32_t max_active_sessions{};

		uint8_t max_missed_heartbeats{};

		uint32_t max_login_attempts{};
		uint32_t login_block_duration_minutes{};

		uint32_t rate_limit_per_second{};
		uint32_t rate_limit_max_overflows{};
		uint32_t rate_limit_block_duration_minutes{};

		bool online_counter_enabled{};

		bool maintenance_mode_enabled{};
		uint16_t maintenance_est_duration{};
	};

	struct MSCharacter {
		THEADER header = HEADER_MS_CHARACTER;
		uint8_t sub_header{}; //ESubCharacter
	};

	struct MSChangeRace {
		uint32_t pid{};
		uint8_t race{};
	};

	struct MSChangeSex {
		uint32_t pid{};
		uint8_t sex{};
	};

	struct MSChangeEmpire {
		uint32_t pid{};
		uint8_t empire{};
	};

	struct MSChangeName {
		uint32_t pid{};
		char name[consts::CHARACTER_NAME_MAX_LENGTH + 1]{};
	};

	struct MSReSync {
		THEADER header = HEADER_MS_SYNC;
		TSIZE size{};
		uint32_t count_sync{}; //port reinitialize icin pid bilgileri
		uint32_t count_war{}; //savas sayisi
	};

	//tek bir savas
	struct TWarElem {
		uint32_t gids[2]{};
		uint32_t scores[2]{};
#ifdef FIGHTER_SCORE_SYNC
		TSIZE team_size[2]{}; //TWar Fighter list
#endif
	};

#ifdef FIGHTER_SCORE_SYNC
	struct TWarFighter {
		uint32_t pid{};
		uint32_t kills{};
		uint32_t deaths{};
	};
#endif

	struct SMCacheStatus {
		THEADER header = HEADER_SM_CACHE_STATUS;
		bool is_ready{};
	};

	struct SMForward {
		THEADER header = HEADER_SM_FORWARD;
		TSIZE size{};
		uint16_t sub_header{};
	};

	struct MSMobileNotification {
		THEADER header = HEADER_MS_MOBILE_NOTIFY;
		TSIZE size{};
		uint8_t type{};
		uint32_t pid{};
	};

#pragma pack(pop) 

}

