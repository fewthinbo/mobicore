#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif
#include "mobi_client.h"

#include <Singletons/log_manager.h>
#include "constants/packets.h"
#include <Network/buffer.h>

#if __BUILD_FOR_GAME__
#include "../../../common/tables.h"
#endif

#include "client/client_base.h"
#include "admin/admin_data_manager.h"
#include "unprocessed/unprocessed.h"
#include "unprocessed/message_queue.h"

using namespace network;

namespace mobi_game {
	using namespace consts;
	bool MobiClient::sendLogin(uint32_t pid, uint32_t map_idx) {
		if (pid == 0) return false;

		MSLogin packet{};
		packet.header = HEADER_MS_LOGIN;
		packet.pid = pid;
		packet.map_idx = map_idx;
		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool MobiClient::sendLogout(uint32_t pid) {
		if (pid == 0) return false;

		MSLogout packet{};
		packet.header = HEADER_MS_LOGOUT;
		packet.pid = pid;
		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));

		return SendPacket(buf.get());
	}

	bool MobiClient::sendMessage(uint32_t sender_pid, uint32_t receiver_pid, const std::string& message, uint16_t code_page) {
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

	bool MobiClient::sendShout(uint32_t pid, const std::string& message, uint16_t code_page) {
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

	bool MobiClient::sendLevelPacket(uint32_t pid, uint32_t level) {
		if (pid == 0) return false;

		MSLevelUp packet{};
		packet.header = HEADER_MS_LEVEL_UP;
		packet.level = level;
		packet.pid = pid;
		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool MobiClient::sendUserCheck(const std::string& nameTo, const std::string& message, uint32_t sender_pid, uint16_t code_page) {
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

	bool MobiClient::sendGuildJoin(uint32_t guild_id, uint32_t pid) {
		if (!IsCoreP2PManager()) return true;

		if (guild_id == 0 || pid == 0) return false;

		MSGuildJoin packet{};
		packet.header = HEADER_MS_GUILD_JOIN;
		packet.guild_id = guild_id;
		packet.pid = pid;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool MobiClient::sendGuildLeave(uint32_t pid) {
		if (!IsCoreP2PManager()) return true;
		if (pid == 0) return false;

		MSGuildLeave packet{};
		packet.header = HEADER_MS_GUILD_LEAVE;
		packet.pid = pid;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool MobiClient::sendMessengerAdd(const std::string& name, const std::string& target) {
		if (name.empty() || target.empty()) return false;

		MSMessengerAdd packet{};
		packet.header = HEADER_MS_MESSENGER_ADD;
		network::TMP_BUFFER::str_copy(packet.name, sizeof(packet.name), name.c_str());
		network::TMP_BUFFER::str_copy(packet.target, sizeof(packet.target), target.c_str());

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool MobiClient::sendMessengerRemove(const std::string& name, const std::string& target) {
		if (name.empty() || target.empty()) return false;

		MSMessengerRemove packet{};
		packet.header = HEADER_MS_MESSENGER_REMOVE;
		network::TMP_BUFFER::str_copy(packet.name, sizeof(packet.name), name.c_str());
		network::TMP_BUFFER::str_copy(packet.target, sizeof(packet.target), target.c_str());

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool MobiClient::sendOnlineCount(const int(&empires)[4]) {
		if (!IsCoreP2PManager()) return true;
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
	bool MobiClient::sendGuildWarStart(uint32_t guild_id1, uint32_t guild_id2, uint32_t scoreLimit) {
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

	bool MobiClient::sendGuildWarEnd(uint32_t guild_id1, uint32_t guild_id2) {
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

	bool MobiClient::sendGuildWarPlayerKill(uint32_t guild_id1, uint32_t guild_id2, uint32_t killer_pid, uint32_t victim_pid) {
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

	bool MobiClient::sendGuildWarPlayerJoin(uint32_t guild_id, uint32_t pid) {
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

	bool MobiClient::sendGuildWarPlayerLeave(uint32_t guild_id, uint32_t pid) {
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

	bool MobiClient::sendGuildWarPlayerPositionUpdate(uint32_t guild_id, uint32_t pid, uint32_t pos[2]) {
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

	bool MobiClient::sendGuildWarMapNotification(uint32_t guild_id1, uint32_t guild_id2, const std::string& message) {
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


#if __MT_DB_INFO__
	bool MobiClient::sendCharacterCreate(uint32_t pid) {
		if (!IsCoreP2PManager()) return true;
		if (pid == 0) return false;
		MSDataUpdate packet{};
		packet.type = static_cast<uint8_t>(EDataUpdateTypes::CREATE_PLAYER);
		packet.id = pid;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}
#elif __BUILD_FOR_GAME__
	bool MobiClient::sendCharacterCreate(const TSimplePlayer& player, uint32_t acc_id) {
		if (!IsCoreP2PManager()) return true;
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
#elif PLATFORM_WINDOWS
	bool MobiClient::sendCharacterCreate(const MSCache::Player& player) {
		if (!IsCoreP2PManager()) return true;
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

	bool MobiClient::sendCharacterDelete(uint32_t pid) {
		if (!IsCoreP2PManager()) return true;
		if (pid == 0) return false;


		MSDataUpdate packet{};
		packet.header = HEADER_MS_DATA_UPDATE;
#if __MT_DB_INFO__
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

#if __MT_DB_INFO__
	bool MobiClient::sendGuildCreate(uint32_t guild_id) {
		if (guild_id == 0) return false;

		MSDataUpdate packet{};
		packet.header = HEADER_MS_DATA_UPDATE;
		packet.type = static_cast<uint8_t>(EDataUpdateTypes::CREATE_GUILD);
		packet.id = guild_id;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}
#elif __BUILD_FOR_GAME__
	bool MobiClient::sendGuildCreate(const CGuild& gld) {
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
#elif PLATFORM_WINDOWS
	bool MobiClient::sendGuildCreate(const MSCache::Guild& gld) {
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

	bool MobiClient::sendGuildDelete(uint32_t guild_id) {
		if (!IsCoreP2PManager()) return true;
		if (guild_id == 0) return false;

		MSDataUpdate packet{};
		packet.header = HEADER_MS_DATA_UPDATE;
#if __MT_DB_INFO__
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

	bool MobiClient::sendLadderPoint(uint32_t guild_id, uint32_t point) {
		if (guild_id == 0) return false;

		MSLadderPoint packet{};
		packet.header = HEADER_MS_GUILD_POINT;
		packet.guild_id = guild_id;
		packet.point = point;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool MobiClient::sendGuildStats(uint32_t guild_id, uint32_t win, uint32_t draw, uint32_t loss) {
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

	bool MobiClient::sendKill(uint32_t killerID, uint32_t victimID) {
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

	bool MobiClient::sendChangeRace(uint32_t pid, uint8_t race) {
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

	bool MobiClient::sendChangeSex(uint32_t pid, uint8_t sex) {
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

	bool MobiClient::sendChangeEmpire(uint32_t pid, uint8_t empire) {
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

	bool MobiClient::sendChangeName(uint32_t pid, const std::string& name) {
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

	bool MobiClient::sendMobileNotification(uint32_t pid, const std::string& message, ENotificationChannels channel) {
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
#if __OFFSHOP__
#if __BUILD_FOR_GAME__
	bool MobiClient::sendShopCreate(const ikashop::TShopInfo& info){
		if (!IsCoreP2PManager()) return true;

		offshop::TShopCreate pack_sec{};
		pack_sec.slot_count = info.lock_index;

		MSOffshop pack{};
		pack.size = sizeof(MSOffshop) + sizeof(pack_sec);
		pack.sub_id = static_cast<uint8_t>(ESubOffshop::SHOP_OPEN);
		pack.owner_pid = info.ownerid;

		TMP_BUFFER buf(pack.size);
		buf.write(&pack, sizeof(pack));
		buf.write(&pack_sec, sizeof(pack_sec));
		return SendPacket(buf.get());
	}
#endif
	bool MobiClient::sendShopClose(uint32_t owner_pid) {
		if (!IsCoreP2PManager()) return true;
		MSOffshop pack{};
		pack.size = sizeof(MSOffshop);
		pack.sub_id = static_cast<uint8_t>(ESubOffshop::SHOP_CLOSE);
		pack.owner_pid = owner_pid;

		TMP_BUFFER buf(pack.size);
		buf.write(&pack, sizeof(pack));
		return SendPacket(buf.get());
	}
	bool MobiClient::sendShopUpdateSlotCount(uint32_t owner_pid, uint32_t uptodate){
		if (!IsCoreP2PManager()) return true;
		uint32_t slot_count = uptodate;

		MSOffshop pack{};
		pack.size = sizeof(MSOffshop) + sizeof(uint32_t);
		pack.sub_id = static_cast<uint8_t>(ESubOffshop::SHOP_UPDATE_SLOT_COUNT);
		pack.owner_pid = owner_pid;

		TMP_BUFFER buf(pack.size);
		buf.write(&pack, sizeof(pack));
		buf.write(&slot_count, sizeof(uint32_t));
		return SendPacket(buf.get());
	}
#if __BUILD_FOR_GAME__
	bool MobiClient::sendShopItemAdd(uint32_t owner_pid, const ikashop::TShopItem& item){
		if (!IsCoreP2PManager()) return true;
		offshop::TItemAdd pack_sec{};
		for (int i = 0; i < MAX_ATTR_COUNT; ++i) {
			pack_sec.attrs[i].type = item.aAttr[i].bType;
			pack_sec.attrs[i].value = item.aAttr[i].sValue;
		}
		std::memcpy(pack_sec.sockets, item.alSockets, MAX_SOCKETS_COUNT);
		pack_sec.virtual_id = item.id;
		pack_sec.count = item.count;
		pack_sec.pos = item.pos;
		pack_sec.price.cheque = item.price.cheque;
		pack_sec.price.yang = item.price.yang;
		pack_sec.vnum = item.vnum;

		MSOffshop pack{};
		pack.size = sizeof(MSOffshop) + sizeof(pack_sec);
		pack.sub_id = static_cast<uint8_t>(ESubOffshop::ITEM_ADD);
		pack.owner_pid = owner_pid;

		TMP_BUFFER buf(pack.size);
		buf.write(&pack, sizeof(pack));
		buf.write(&pack_sec, sizeof(pack_sec));
		return SendPacket(buf.get());
	}
#endif
	bool MobiClient::sendShopItemRemove(uint32_t owner_pid, uint32_t pos){
		if (!IsCoreP2PManager()) return true;
		offshop::TItemRemove pack_sec{};
		pack_sec.pos = pos;

		MSOffshop pack{};
		pack.size = sizeof(MSOffshop) + sizeof(pack_sec);
		pack.sub_id = static_cast<uint8_t>(ESubOffshop::ITEM_REMOVE);
		pack.owner_pid = owner_pid;

		TMP_BUFFER buf(pack.size);
		buf.write(&pack, sizeof(pack));
		buf.write(&pack_sec, sizeof(pack_sec));
		return SendPacket(buf.get());
	}
#if __BUILD_FOR_GAME__
	bool MobiClient::sendShopItemUpdatePrice(uint32_t owner_pid, uint32_t pos, const ikashop::TPriceInfo& price) {
		if (!IsCoreP2PManager()) return true;
		offshop::TItemUpdatePrice pack_sec{};
		pack_sec.pos = pos;
		pack_sec.price.yang = price.yang;
		pack_sec.price.cheque = price.cheque;

		MSOffshop pack{};
		pack.size = sizeof(MSOffshop) + sizeof(pack_sec);
		pack.sub_id = static_cast<uint8_t>(ESubOffshop::ITEM_UPDATE_PRICE);
		pack.owner_pid = owner_pid;

		TMP_BUFFER buf(pack.size);
		buf.write(&pack, sizeof(pack));
		buf.write(&pack_sec, sizeof(pack_sec));
		return SendPacket(buf.get());
	}
#endif
	bool MobiClient::sendShopItemUpdatePos(uint32_t owner_pid, uint32_t pos, uint32_t uptodate) {
		if (!IsCoreP2PManager()) return true;
		offshop::TItemUpdatePos pack_sec{};
		pack_sec.pos = pos;
		pack_sec.pos_uptodate = uptodate;

		MSOffshop pack{};
		pack.size = sizeof(MSOffshop) + sizeof(pack_sec);
		pack.sub_id = static_cast<uint8_t>(ESubOffshop::ITEM_UPDATE_POS);
		pack.owner_pid = owner_pid;

		TMP_BUFFER buf(pack.size);
		buf.write(&pack, sizeof(pack));
		buf.write(&pack_sec, sizeof(pack_sec));
		return SendPacket(buf.get());
	}
	bool MobiClient::sendShopItemBuy(uint32_t owner_pid, uint32_t buyer_id, uint32_t pos) {
		if (!IsCoreP2PManager()) return true;
		offshop::TItemBuy pack_sec{};
		pack_sec.buyer_pid = buyer_id;
		pack_sec.pos = pos;

		MSOffshop pack{};
		pack.size = sizeof(MSOffshop) + sizeof(pack_sec);
		pack.sub_id = static_cast<uint8_t>(ESubOffshop::ITEM_BUY);
		pack.owner_pid = owner_pid;

		TMP_BUFFER buf(pack.size);
		buf.write(&pack, sizeof(pack));
		buf.write(&pack_sec, sizeof(pack_sec));
		return SendPacket(buf.get());
	}

#endif
}

#endif