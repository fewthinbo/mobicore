#ifdef MOBICORE
#include "stdafx.h"
#endif
#include "mobi_client.h"

#include <Singletons/log_manager.h>
#include "constants/packets.h"
#include <Network/buffer.h>

#include "admin/queries.h"
#include "admin/admin_data_manager.h"
#include "unprocessed/message_queue.h"
#include "client/client_core.h"

using namespace network;

namespace mobi_game {
	using namespace consts;
	namespace test_constants {
		constexpr auto one_hundred_bytes = u8"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
		constexpr uint16_t test_code_page = 65001; //utf8
		constexpr auto intensity_part_budget = std::chrono::milliseconds(10);

		struct STimer {
			const char* name;
			std::chrono::steady_clock::time_point time_start;
			STimer(const char* task_name) : time_start(std::chrono::steady_clock::now()), name(task_name) {
				LOG_WARN("(?) has been started", name);
			}
			~STimer() {
				LOG_WARN("(?) complated in ms(?)", name,
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_start)
				);
			}
		};
	}

	//Sunucudan cevap beklenilen hicbir paket gonderilmez, amac: gonderim yogunlugunu test etmektir.
	bool GameClient::spamTest(uint8_t intensity) {
		using namespace test_constants;

		constexpr uint32_t receiver_pid = 116;
		constexpr std::array<uint32_t, 4> pid_arr{ 1, 114, 115, 116 };

		auto time_start = std::chrono::steady_clock::now();
		auto total_budget = intensity_part_budget * intensity;
		size_t write_counter = 0;

		LOG_WARN("Waiting other tasks to finish");
		{
			STimer timer("Write Part");

			while (std::chrono::steady_clock::now() - time_start < total_budget) {
				for (size_t i = 0; i < pid_arr.size(); ++i) {
					const auto& pid = pid_arr[i];
					sendShout(pid, one_hundred_bytes, test_code_page);
					sendMessage(pid, receiver_pid, one_hundred_bytes, test_code_page);
				}
				write_counter += 8;
			}
		}
		LOG_WARN("Total bytes written(~?)", write_counter * 100);
		return true;
	}



	GameClient::~GameClient() noexcept = default;

	bool GameClient::sendLogin(uint32_t pid, uint32_t map_idx) {
		if (pid == 0) return false;

		MSLogin packet{};
		packet.header = HEADER_MS_LOGIN;
		packet.pid = pid;
		packet.map_idx = map_idx;
		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendLogout(uint32_t pid) {
		if (pid == 0) return false;

		MSLogout packet{};
		packet.header = HEADER_MS_LOGOUT;
		packet.pid = pid;
		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));

		return SendPacket(buf.get());
	}

	bool GameClient::sendMessage(uint32_t sender_pid, uint32_t receiver_pid, const std::string& message, uint16_t code_page) {
		if (sender_pid == 0 || receiver_pid == 0 || message.empty()) return false;

		MSMessage packet{};
		packet.header = HEADER_MS_MESSAGE;
		packet.sender_pid = sender_pid;
		packet.receiver_pid = receiver_pid;
		packet.code_page = code_page;
		packet.size = message.size() + 1;

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(message.data(), packet.size);
		return SendPacket(buf.get());
	}

	bool GameClient::sendShout(uint32_t pid, const std::string& message, uint16_t code_page) {
		if (pid == 0 || message.empty()) return false;

		MSShout packet{};
		packet.header = HEADER_MS_SHOUT;
		packet.pid = pid;
		packet.size = message.size() + 1;
		packet.code_page = code_page;

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(message.data(), packet.size);

		return SendPacket(buf.get());
	}

	bool GameClient::sendLevelPacket(uint32_t pid, uint32_t level) {
		if (pid == 0) return false;

		MSLevelUp packet{};
		packet.header = HEADER_MS_LEVEL_UP;
		packet.level = level;
		packet.pid = pid;
		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendUserCheck(const std::string& nameTo, const std::string& message, uint32_t sender_pid, uint16_t code_page) {
		if (nameTo.empty() || message.empty() || sender_pid == 0) return false;

		MSUserCheck packet{};
		packet.header = HEADER_MS_USER_CHECK;
		network::TMP_BUFFER::str_copy(packet.name, sizeof(packet.name), nameTo.c_str());
		packet.sender_pid = sender_pid;
		packet.container_id = message_helper_->message_get_container_id();

		message_helper_->message_add(sender_pid, nameTo, message, code_page);

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));

		/*
		Server'a kullanici mobilde mi diye basit bir paket gider. Geri donus paketi olumlu alindiginda memory'e eklenmis olan mesaj, dogru container id ile bulunr
		ve o mesaj mobile daha kapsamli bir paket ile gonderilir.
		Olumsuz gelirse ise memory'deki mesaj silinir.
		*/

		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildJoin(uint32_t guild_id, uint32_t pid) {
		if (guild_id == 0 || pid == 0) return false;

		MSGuildJoin packet{};
		packet.header = HEADER_MS_GUILD_JOIN;
		packet.guild_id = guild_id;
		packet.pid = pid;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildLeave(uint32_t pid) {
		if (pid == 0) return false;

		MSGuildLeave packet{};
		packet.header = HEADER_MS_GUILD_LEAVE;
		packet.pid = pid;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendMessengerAdd(const std::string& name, const std::string& target) {
		if (name.empty() || target.empty()) return false;

		MSMessengerAdd packet{};
		packet.header = HEADER_MS_MESSENGER_ADD;
		network::TMP_BUFFER::str_copy(packet.name, sizeof(packet.name), name.c_str());
		network::TMP_BUFFER::str_copy(packet.target, sizeof(packet.target), target.c_str());

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendMessengerRemove(const std::string& name, const std::string& target) {
		if (name.empty() || target.empty()) return false;

		MSMessengerRemove packet{};
		packet.header = HEADER_MS_MESSENGER_REMOVE;
		network::TMP_BUFFER::str_copy(packet.name, sizeof(packet.name), name.c_str());
		network::TMP_BUFFER::str_copy(packet.target, sizeof(packet.target), target.c_str());

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendOnlineCount(const int(&empires)[4]) {
		if (!admin_data_manager_->HasAuthority(EAuthorityType::ONLINE_COUNTER)) return false;
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now - last_sent_) < refresh_interval_) return false;

		MSOnline packet{};
		packet.header = HEADER_MS_ONLINE;
		packet.empires[0] = empires[1];
		packet.empires[1] = empires[2];
		packet.empires[2] = empires[3];

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	//guild war packets
	bool GameClient::sendGuildWarStart(uint32_t guild_id1, uint32_t guild_id2, uint32_t scoreLimit) {
		if (guild_id1 == 0 || guild_id2 == 0) return false;

		MSGuildWar packet{};
		packet.header = HEADER_MS_GUILD_WAR;
		packet.subheader = static_cast<uint8_t>(ESubGuildWar::WAR_START);
		packet.size = sizeof(SGuildWarStart);

		SGuildWarStart sPacket{};
		sPacket.guild_id[0] = guild_id1;
		sPacket.guild_id[1] = guild_id2;
		sPacket.scoreLimit = scoreLimit;

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(&sPacket, sizeof(sPacket));
		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildWarEnd(uint32_t guild_id1, uint32_t guild_id2) {
		if (guild_id1 == 0 || guild_id2 == 0) return false;

		MSGuildWar packet{};
		packet.header = HEADER_MS_GUILD_WAR;
		packet.subheader = static_cast<uint8_t>(ESubGuildWar::WAR_END);
		packet.size = sizeof(SGuildWarEnd);

		SGuildWarEnd sPacket{};
		sPacket.guild_id[0] = guild_id1;
		sPacket.guild_id[1] = guild_id2;

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(&sPacket, sizeof(sPacket));
		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildWarPlayerKill(uint32_t guild_id1, uint32_t guild_id2, uint32_t killer_pid, uint32_t victim_pid) {
		if (guild_id1 == 0 || guild_id2 == 0 || killer_pid == 0 || victim_pid == 0) return false;
		MSGuildWar packet{};
		packet.header = HEADER_MS_GUILD_WAR;
		packet.subheader = static_cast<uint8_t>(ESubGuildWar::PLAYER_KILL);
		packet.size = sizeof(SGuildWarPlayerKill);

		SGuildWarPlayerKill sPacket{};
		sPacket.guild_id[0] = guild_id1;
		sPacket.guild_id[1] = guild_id2;
		sPacket.killer_pid = killer_pid;
		sPacket.victim_pid = victim_pid;

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(&sPacket, sizeof(sPacket));
		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildWarPlayerJoin(uint32_t guild_id, uint32_t pid) {
		if (guild_id == 0 || pid == 0) return false;

		MSGuildWar packet{};
		packet.header = HEADER_MS_GUILD_WAR;
		packet.subheader = static_cast<uint8_t>(ESubGuildWar::PLAYER_JOIN);
		packet.size = sizeof(SGuildWarPlayerJoin);

		SGuildWarPlayerJoin sPacket{};
		sPacket.guild_id = guild_id;
		sPacket.pid = pid;

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(&sPacket, sizeof(sPacket));
		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildWarPlayerLeave(uint32_t guild_id, uint32_t pid) {
		if (guild_id == 0 || pid == 0) return false;

		MSGuildWar packet{};
		packet.header = HEADER_MS_GUILD_WAR;
		packet.subheader = static_cast<uint8_t>(ESubGuildWar::PLAYER_LEAVE);
		packet.size = sizeof(SGuildWarPlayerLeave);

		SGuildWarPlayerLeave sPacket{};
		sPacket.guild_id = guild_id;
		sPacket.pid = pid;

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(&sPacket, sizeof(sPacket));
		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildWarPlayerPositionUpdate(uint32_t guild_id, uint32_t pid, uint32_t pos[2]) {
		if (guild_id == 0 || pid == 0) return false;

		MSGuildWar packet{};
		packet.header = HEADER_MS_GUILD_WAR;
		packet.subheader = static_cast<uint8_t>(ESubGuildWar::PLAYER_POSITION_UPDATE);
		packet.size = sizeof(SGuildWarPlayerPositionUpdate);

		SGuildWarPlayerPositionUpdate sPacket{};
		sPacket.guild_id = guild_id;
		sPacket.pid = pid;
		sPacket.pos[0] = pos[0];
		sPacket.pos[1] = pos[1];

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(&sPacket, sizeof(sPacket));
		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildWarMapNotification(uint32_t guild_id1, uint32_t guild_id2, const std::string& message) {
		if (guild_id1 == 0 || guild_id2 == 0 || message.empty()) return false;
		auto msgSize = message.size() + 1;

		MSGuildWar packet{};
		packet.header = HEADER_MS_GUILD_WAR;
		packet.subheader = static_cast<uint8_t>(ESubGuildWar::NOTIFICATION);
		packet.size = sizeof(SGuildWarNotification) + msgSize;

		SGuildWarNotification sPacket{};
		sPacket.guild_id[0] = guild_id1;
		sPacket.guild_id[1] = guild_id2;

		TMP_BUFFER buf(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(&sPacket, sizeof(sPacket));
		buf.write(message.data(), msgSize);
		return SendPacket(buf.get());
	}
	//end of guild war packets


#ifdef ENABLE_MT_DB_INFO
	bool GameClient::sendCharacterCreate(uint32_t pid) {
		if (pid == 0) return false;
		MSDataUpdate packet{};
		packet.type = static_cast<uint8_t>(EDataUpdateTypes::CREATE_PLAYER);
		packet.id = pid;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}
#elif defined(MOBICORE)
	bool GameClient::sendCharacterCreate(const TSimplePlayer& player, uint32_t acc_id) {
		if (!d) return false;
		//TODO: sendAccountCreate de ekle. db bilgilerini aliyorken buna gerek yoktu fakat simdi gerekli.
		//tODO: Bunlari windows icin test konsola ekle ve paketleri test et.
		MSCache::Player data{
			player.dwID,
			acc_id,
			player.szName,
			player.byJob,
			1, //mapidx
			player.byLevel,
			player.dwPlayMinutes,
			"", //lastplay
			0, //gld_id
			false //is guild leader
		};

		MSDataUpdate packet{};
		packet.cache_type = static_cast<uint8_t>(ECacheType::PLAYER);
		packet.is_invalidate = false;
		packet.size = sizeof(MSDataUpdate) + sizeof(MSCache::Player);

		TMP_BUFFER buf(packet.size);
		buf.write(&packet, sizeof(MSDataUpdate));
		buf.write(&data, sizeof(MSCache::Player));
		return SendPacket(buf.get());
	}
#elif defined(PLATFORM_WINDOWS)
	bool GameClient::sendCharacterCreate(const MSCache::Player& player) {
		MSDataUpdate packet{};
		packet.cache_type = static_cast<uint8_t>(ECacheType::PLAYER);
		packet.is_invalidate = false;
		packet.size = sizeof(MSDataUpdate) + sizeof(MSCache::Player);

		TMP_BUFFER buf(packet.size);
		buf.write(&packet, sizeof(MSDataUpdate));
		buf.write(&player, sizeof(MSCache::Player));
		return SendPacket(buf.get());
	}
#endif

	bool GameClient::sendCharacterDelete(uint32_t pid) {
		if (pid == 0) return false;

		MSDataUpdate packet{};
		packet.header = HEADER_MS_DATA_UPDATE;
#ifdef ENABLE_MT_DB_INFO
		packet.type = static_cast<uint8_t>(EDataUpdateTypes::DELETE_PLAYER);
		packet.id = pid;
		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
#else
		packet.cache_type = static_cast<uint8_t>(ECacheType::PLAYER);
		packet.is_invalidate = true;
		packet.size = sizeof(MSDataUpdate) + sizeof(uint32_t);
		TMP_BUFFER buf(sizeof(packet) + sizeof(uint32_t));
		buf.write(&packet, sizeof(packet));
		buf.write(&pid, sizeof(uint32_t));
#endif
		return SendPacket(buf.get());
	}

#ifdef ENABLE_MT_DB_INFO
	bool GameClient::sendGuildCreate(uint32_t guild_id) {
		if (guild_id == 0) return false;

		MSDataUpdate packet{};
		packet.header = HEADER_MS_DATA_UPDATE;
		packet.type = static_cast<uint8_t>(EDataUpdateTypes::CREATE_GUILD);
		packet.id = guild_id;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}
#elif defined(MOBICORE)
	bool GameClient::sendGuildCreate(const CGuild& gld) {
		MSCache::Guild data{
			gld.GetName(),
			gld.GetID(),
			gld.GetMasterPID(),
			gld.GetLevel(),
			gld.GetGuildWarWinCount(),
			gld.GetGuildWarDrawCount(),
			gld.GetGuildWarLossCount(),
			gld.GetLadderPoint()
		};

		MSDataUpdate packet{};
		packet.cache_type = static_cast<uint8_t>(ECacheType::GUILD);
		packet.is_invalidate = false;
		packet.size = sizeof(MSDataUpdate) + sizeof(MSCache::Guild);

		TMP_BUFFER buf(packet.size);
		buf.write(&packet, sizeof(MSDataUpdate));
		buf.write(&data, sizeof(MSCache::Guild));
		return SendPacket(buf.get());
	}
#elif defined(PLATFORM_WINDOWS)
	bool GameClient::sendGuildCreate(const MSCache::Guild& gld) {
		MSDataUpdate packet{};
		packet.cache_type = static_cast<uint8_t>(ECacheType::GUILD);
		packet.is_invalidate = false;
		packet.size = sizeof(MSDataUpdate) + sizeof(MSCache::Guild);

		TMP_BUFFER buf(packet.size);
		buf.write(&packet, sizeof(MSDataUpdate));
		buf.write(&gld, sizeof(MSCache::Guild));
		return SendPacket(buf.get());
	}
#endif

	bool GameClient::sendGuildDelete(uint32_t guild_id) {
		if (guild_id == 0) return false;

		MSDataUpdate packet{};
		packet.header = HEADER_MS_DATA_UPDATE;
#ifdef ENABLE_MT_DB_INFO
		packet.type = static_cast<uint8_t>(EDataUpdateTypes::DELETE_GUILD);
		packet.id = guild_id;
		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
#else
		packet.cache_type = static_cast<uint8_t>(ECacheType::GUILD);
		packet.is_invalidate = true;
		packet.size = sizeof(MSDataUpdate) + sizeof(uint32_t);
		TMP_BUFFER buf(sizeof(packet) + sizeof(uint32_t));
		buf.write(&packet, sizeof(packet));
		buf.write(&guild_id, sizeof(uint32_t));
#endif
		return SendPacket(buf.get());
	}

	bool GameClient::sendLadderPoint(uint32_t guild_id, uint32_t point) {
		if (guild_id == 0) return false;

		MSLadderPoint packet{};
		packet.header = HEADER_MS_GUILD_POINT;
		packet.guild_id = guild_id;
		packet.point = point;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendGuildStats(uint32_t guild_id, uint32_t win, uint32_t draw, uint32_t loss) {
		if (guild_id == 0) return false;

		MSGuildStats packet{};
		packet.header = HEADER_MS_GUILD_STATS;
		packet.guild_id = guild_id;
		packet.win = win;
		packet.draw = draw;
		packet.loss = loss;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendKill(uint32_t killerID, uint32_t victimID) {
		if (killerID == 0 || victimID == 0) return false;
		if (!IsBoss(victimID)) return true;

		MSKill packet{};
		packet.header = HEADER_MS_KILL;
		packet.killer_pid = killerID;
		packet.victim_pid = victimID;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClient::sendChangeRace(uint32_t pid, uint8_t race) {
		if (pid == 0) return false;

		MSCharacter packet{};
		packet.header = HEADER_MS_CHARACTER;
		packet.sub_header = static_cast<uint8_t>(ESubCharacter::CHANGE_RACE);
		
		MSChangeRace s_packet{};
		s_packet.pid = pid;
		s_packet.race = race;

		TMP_BUFFER buf(sizeof(MSCharacter) + sizeof(MSChangeRace));
		buf.write(&packet, sizeof(packet));
		buf.write(&s_packet, sizeof(s_packet));

		return SendPacket(buf.get());
	}

	bool GameClient::sendChangeSex(uint32_t pid, uint8_t sex) {
		if (pid == 0) return false;

		MSCharacter packet{};
		packet.header = HEADER_MS_CHARACTER;
		packet.sub_header = static_cast<uint8_t>(ESubCharacter::CHANGE_SEX);

		MSChangeSex s_packet{};
		s_packet.pid = pid;
		s_packet.sex = sex;

		TMP_BUFFER buf(sizeof(MSCharacter) + sizeof(MSChangeSex));
		buf.write(&packet, sizeof(packet));
		buf.write(&s_packet, sizeof(s_packet));

		return SendPacket(buf.get());
	}

	bool GameClient::sendChangeEmpire(uint32_t pid, uint8_t empire) {
		if (pid == 0) return false;

		MSCharacter packet{};
		packet.header = HEADER_MS_CHARACTER;
		packet.sub_header = static_cast<uint8_t>(ESubCharacter::CHANGE_EMPIRE);

		MSChangeEmpire s_packet{};
		s_packet.pid = pid;
		s_packet.empire = empire;

		TMP_BUFFER buf(sizeof(MSCharacter) + sizeof(MSChangeEmpire));
		buf.write(&packet, sizeof(packet));
		buf.write(&s_packet, sizeof(s_packet));

		return SendPacket(buf.get());
	}

	bool GameClient::sendChangeName(uint32_t pid, const std::string& name) {
		if (pid == 0 || name.empty()) return false;

		MSCharacter packet{};
		packet.header = HEADER_MS_CHARACTER;
		packet.sub_header = static_cast<uint8_t>(ESubCharacter::CHANGE_NAME);

		MSChangeName s_packet{};
		s_packet.pid = pid;
		network::TMP_BUFFER::str_copy(s_packet.name, sizeof(s_packet.name), name.c_str());

		TMP_BUFFER buf(sizeof(MSCharacter) + sizeof(MSChangeName));
		buf.write(&packet, sizeof(packet));
		buf.write(&s_packet, sizeof(s_packet));

		return SendPacket(buf.get());
	}

	bool GameClient::sendMobileNotification(uint32_t pid, const std::string& message, ENotificationChannels channel) {
		if (pid == 0 || message.empty()) return false;

		MSMobileNotification packet{};
		packet.header = HEADER_MS_MOBILE_NOTIFY;
		packet.size = message.size() + 1;
		packet.pid = pid;
		packet.type = static_cast<uint8_t>(channel);

		TMP_BUFFER buf(sizeof(MSMobileNotification) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(message.data(), packet.size);

		return SendPacket(buf.get());
	}
}