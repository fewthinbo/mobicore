#pragma once

#include <string>
#include <cstdint>

#include <singleton.h>

#include "constants/consts.h"
#include "client/mobi_base.h"

namespace mobi_game {
	class GameNetworkClient;

	//should be synced with bridgeServer/Common/tables.h
	enum class ENotificationChannels {
		SYSTEM,
		CHAT,
		FRIENDS,
		GUILD,
		EVENTS,
		MAX,
	};

	class GameClient final :
		public GameClientBase,
		public CSingleton<GameClient> {
		friend class GameNetworkClient;
	public:
		~GameClient() noexcept;
	public:
		bool sendLogin(uint32_t pid, uint32_t map_idx);
		bool sendLogout(uint32_t pid);
		bool sendMessage(uint32_t sender_pid, uint32_t receiver_pid, const std::string& message, uint16_t code_page = consts::DEFAULT_CODEPAGE);
		bool sendShout(uint32_t pid, const std::string& message, uint16_t code_page = consts::DEFAULT_CODEPAGE);
		bool sendUserCheck(const std::string& nameTo, const std::string& message, uint32_t sender_pid, uint16_t code_page = consts::DEFAULT_CODEPAGE);

		bool sendLevelPacket(uint32_t pid, uint32_t level);

		bool sendGuildJoin(uint32_t guild_id, uint32_t pid);
		bool sendGuildLeave(uint32_t pid);
		bool sendMessengerAdd(const std::string& name, const std::string& target);
		bool sendMessengerRemove(const std::string& name, const std::string& target);
		bool sendOnlineCount(const int(&empires)[4]);

		bool sendGuildWarStart(uint32_t guild_id1, uint32_t guild_id2, uint32_t scoreLimit);
		bool sendGuildWarEnd(uint32_t guild_id1, uint32_t guild_id2);
		bool sendGuildWarPlayerKill(uint32_t guild_id1, uint32_t guild_id2, uint32_t killer_pid, uint32_t victim_pid);
		bool sendGuildWarPlayerJoin(uint32_t guild_id, uint32_t pid);
		bool sendGuildWarPlayerLeave(uint32_t guild_id, uint32_t pid);
		bool sendGuildWarPlayerPositionUpdate(uint32_t guild_id, uint32_t pid, uint32_t pos[2]);
		bool sendGuildWarMapNotification(uint32_t guild_id1, uint32_t guild_id2, const std::string& message);

		bool sendCharacterCreate(uint32_t pid);
		bool sendCharacterDelete(uint32_t pid);

		bool sendGuildCreate(uint32_t guild_id);
		bool sendGuildDelete(uint32_t guild_id);

		bool sendLadderPoint(uint32_t guild_id, uint32_t point);
		bool sendGuildStats(uint32_t guild_id, uint32_t win, uint32_t draw, uint32_t loss);

		bool sendKill(uint32_t killerID, uint32_t victimID);

		bool sendChangeRace(uint32_t pid, uint8_t race);
		bool sendChangeSex(uint32_t pid, uint8_t sex);
		bool sendChangeEmpire(uint32_t pid, uint8_t empire);
		bool sendChangeName(uint32_t pid, const std::string& name);

		//use wherever you want for send mobile notification to a player.
		//Example: send when sold player's item in offshop
		bool sendMobileNotification(uint32_t pid, const std::string& message, ENotificationChannels channel = ENotificationChannels::SYSTEM);
	public:
		//100 intensity'de 1 saniye boyunca yazma islemi yapar.
		bool spamTest(uint8_t intensity);
	};
}

#define mobileInstance mobi_game::GameClient::getInstance()
