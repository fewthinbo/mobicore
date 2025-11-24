#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif

#include "mobi_client.h"

#include <cstring>

#if __BUILD_FOR_GAME__
#include "char.h"
#include "char_manager.h"
#include "p2p.h"
#include "desc.h"
#include "messenger_manager.h"
#include "desc_manager.h"
#include "config.h"
#include "buffer_manager.h"
#include "packet.h"
#include "../../../common/tables.h"
#include "../../../common/length.h"
#include "db.h"
#if __OFFSHOP__ == 1
#include "ikarus_shop_manager.h"
#endif
#endif

#include <Singletons/log_manager.h>
#include <Network/buffer.h>

#include "constants/packets.h"
#include "constants/custom_packets.h"

#include "client/client_base.h"
#include "admin/queries.h"
#include "admin/admin_data_manager.h"
#include "unprocessed/message_queue.h"
#include "unprocessed/unprocessed.h"
#include "character/ch_manager.h"


using namespace network;

namespace mobi_game {
	using namespace consts;
	using namespace custom_packets;

	bool MobiClient::HandleDbInfo(TDataRef data) {
		if (!admin_data_manager_->authority_.HasFlag(EAuthorityType::DB_MANAGER)) {
			LOG_TRACE("The core hot has authority to handle DB info request.");
			return true;
		}
		LOG_TRACE("Request received.");
		std::vector<uint8_t> db_cache = admin_data_manager_->GetDBCache();
		return SendPacket(db_cache);
	}

	bool MobiClient::HandleKeyExchange(TDataRef data) {
		auto* sm = reinterpret_cast<const TKeyExchange*>(data.data());
		std::vector<uint8_t> key(data.data() + sizeof(TKeyExchange), data.data() + sizeof(TKeyExchange) + sm->size);
		LOG_TRACE("Key exchange packet received, size(?), key(?)", sm->size, reinterpret_cast<const char*>(key.data()), key.size());

#if !__MOBI_PACKET_ENCRYPTION__
		return false;
#endif

		TKeyExchange key_packet{};
		key_packet.header = HEADER_MS_KEY_EXCHANGE;
#if __BUILD_FOR_GAME__
		key_packet.is_auth_core = g_bAuthServer;
#else
		key_packet.is_auth_core = false;
#endif
		const std::vector<uint8_t>& public_key = client_impl_->session_get_public_key();
		if (public_key.empty()) {
			LOG_ERR("Generated public key is empty");
			return false;
		}

		LOG_INFO("Public key is(?)", std::string(reinterpret_cast<const char*>(public_key.data()), public_key.size()));

		key_packet.size = public_key.size();

		TMP_BUFFER buffer(sizeof(TKeyExchange) + key_packet.size);
		buffer.write(&key_packet, sizeof(TKeyExchange));
		buffer.write(public_key.data(), key_packet.size);

		//bu paket sifrelenmeden gonderilmelidir.
		if (!SendPacket(buffer.get(), false)) {
			LOG_INFO("Public key couldn't sent");
			return false;
		}

		//IMPORTANT: session enable encryption fonksiyonunda afterenc calisiyor, bu admin notifications settings vb bilgileri sifreli gonderecektir.
		//burada oldugu gibi: public key'i gonderme islemi once schedule edilmelidir bu sayede ara sunucu sifreleme islemini tamamlayip sifreli verileri dogru okuyabilir.

		if (!client_impl_->session_enable_encryption(key)) {
			LOG_ERR("an error occured while enable encryption");
			return false;
		}
		return true;
	}

	bool MobiClient::HandleMobilePm(TDataRef data) const {
		auto* sm = reinterpret_cast<const SMMessage*>(data.data());
		const char* message = reinterpret_cast<const char*>(data.data() + sizeof(SMMessage));

		LOG_TRACE("Pm from mobile: message(?), sender_name(?), receiver_acc_id(?)", message, sm->name, sm->receiver_acc_id);
#if __BUILD_FOR_GAME__
		LPDESC desc = DESC_MANAGER::instance().FindByAccountID(sm->receiver_acc_id);

		if (!desc){
			LOG_TRACE("? desc not found for receiver_acc_id(?)", sm->receiver_acc_id);
			return false;
		}

		TSIZE msg_len = sm->size - sizeof(SMMessage);

		TPacketGCWhisper pack{};
		pack.bHeader = HEADER_GC_WHISPER;
		pack.wSize = sizeof(TPacketGCWhisper) + msg_len;
		pack.bType = WHISPER_TYPE_MOBILE;
		strlcpy(pack.szNameFrom, sm->name, sizeof(pack.szNameFrom));

		TEMP_BUFFER tmpbuf{};
		tmpbuf.write(&pack, sizeof(pack));
		tmpbuf.write(message, msg_len);

		desc->Packet(tmpbuf.read_peek(), tmpbuf.size());
		LOG_TRACE("Pm sent to client(accID: ?)", sm->receiver_acc_id);
		return true;
#endif

		return true;
	}

	bool MobiClient::HandleUserCheck(TDataRef data) const {
		auto* ms = reinterpret_cast<const SMUserCheck*>(data.data());
		auto response = static_cast<EUserCheckResponse>(ms->response);

		switch (response)
		{
		case EUserCheckResponse::VALID: {
			return message_helper_->message_process(ms->container_id); // Mesaj mobile gonderilir ve silinir.
		}

		case EUserCheckResponse::OFFLINE:
		case EUserCheckResponse::BLOCKED:
		case EUserCheckResponse::NOT_EXIST:
		{
#if __BUILD_FOR_GAME__
			auto container = message_helper_->message_get_container(ms->container_id);
			if (!container) return false;

			auto sender_pid = container->get_sender_pid();
			if (auto ch = CHARACTER_MANAGER::instance().FindByPID(sender_pid)) {
				if (auto desc = ch->GetDesc()) {
					TPacketGCWhisper pack{};

					pack.bHeader = HEADER_GC_WHISPER;
					pack.bType = WHISPER_TYPE_NOT_EXIST;
					pack.wSize = sizeof(TPacketGCWhisper);
					strlcpy(pack.szNameFrom, container->get_receiver_name(), sizeof(pack.szNameFrom));
					desc->Packet(&pack, sizeof(TPacketGCWhisper));
				}
			}
#endif
			break;
		}
		default:
			break;
		}

		message_helper_->message_remove(ms->container_id); // Mesaj memory'den silinir.
		return true;
	}

	bool MobiClient::HandleMobileLogin(TDataRef data) const {
		auto* packet = reinterpret_cast<const SMLogin*>(data.data());
		LOG_TRACE("Mobile login packet received, name:?", packet->name);
#if __BUILD_FOR_GAME__
		MessengerManager::instance().MobileLogin(packet->name);
#endif
		return true;
	}
	bool MobiClient::HandleMobileLogout(TDataRef data) const {
		auto* packet = reinterpret_cast<const SMLogout*>(data.data());
		LOG_TRACE("Mobile logout packet received, name:?", packet->name);
#if __BUILD_FOR_GAME__
		MessengerManager::instance().MobileLogout(packet->name);
#endif
		return true;
	}

	bool MobiClient::HandleCacheStatus(TDataRef data) {
		auto* packet = reinterpret_cast<const SMCacheStatus*>(data.data());
		SetBridgeCacheStatus(packet->is_ready);
		return true;
	}

	//For server developers/admins: You can write your own custom packets
	//Those data comes from mobile devices of admins and dynamic sizes of the packets are validated from bridgeServer. 
	//use memcpy in this function for more safety to UB

	bool MobiClient::HandleForwardPacket(TDataRef data) const {
		auto* packet = reinterpret_cast<const SMForward*>(data.data());
		auto sub_id = static_cast<ECustomPackets>(packet->sub_header);
		const uint8_t* dynamic_data = reinterpret_cast<const uint8_t*>(data.data() + sizeof(SMForward)); //dynamic part, if you have
		const uint8_t* data_end = reinterpret_cast<const uint8_t*>(data.data() + packet->size);
		LOG_TRACE("Custom forward packet received");
		switch (sub_id)
		{
		case ECustomPackets::EXAMPLE_EVENT: {
			uint16_t event_id{};
			std::memcpy(&event_id, dynamic_data, sizeof(uint16_t));

			if (!advance_cursor(dynamic_data, sizeof(uint16_t), data_end)) {
				LOG_WARN("Buffer overflow sub_id(?), line(?)", sub_id, __LINE__);
				return false;
			}

			bool status{};
			std::memcpy(&status, dynamic_data + sizeof(uint16_t), sizeof(bool));

			//now you've event_id and status in MT
			return true;
		}
		case ECustomPackets::EXAMPLE_NOTIFICATION: {
			uint32_t pid{};
			std::memcpy(&pid, dynamic_data, sizeof(uint32_t));
			if (!advance_cursor(dynamic_data, sizeof(uint32_t), data_end)) {
				LOG_WARN("Buffer overflow sub_id(?), line(?)", sub_id, __LINE__);
				return false;
			}

			char title[32 + 1] = "";
			std::memcpy(title, dynamic_data, sizeof(char) * 33);
			if (!advance_cursor(dynamic_data, sizeof(char) * 33, data_end)) {
				LOG_WARN("Buffer overflow sub_id(?), line(?)", sub_id, __LINE__);
				return false;
			}

			const char* unlimited_msg = reinterpret_cast<const char*>(dynamic_data);

			// now you've player id, title and message with ~unlimited character. (total size should be <= TSIZE)
			break;
		}
		case ECustomPackets::EXAMPLE_COMPLEX: {
			//example: you've tournament sub headers
			enum class ESubTour {
				STATUS, //open close etc.
				ADD_REWARD, // adjust reward of war
				MANAGE_PLAYER, //disconnect, ban a player from tournament
			};

			uint32_t ex_sub_id{};
			std::memcpy(&ex_sub_id, dynamic_data, sizeof(uint32_t));
			if (!advance_cursor(dynamic_data, sizeof(uint32_t), data_end)) {
				LOG_WARN("Buffer overflow sub_id(?), line(?)", sub_id, __LINE__);
				return false;
			}

			auto extra_sub_id = static_cast<ESubTour>(ex_sub_id);

			switch (extra_sub_id)
			{
			case ESubTour::STATUS: {
				bool status{};
				std::memcpy(&status, dynamic_data, sizeof(bool));
				if (!advance_cursor(dynamic_data, sizeof(bool), data_end)) {
					LOG_WARN("Buffer overflow sub_id(?), line(?)", sub_id, __LINE__);
					return false;
				}

				//now you've war status from mobile device, you can update in game war status
				break;
			}

			case ESubTour::ADD_REWARD: {
				uint32_t item_id{};
				std::memcpy(&item_id, dynamic_data, sizeof(uint32_t));
				if (!advance_cursor(dynamic_data, sizeof(uint32_t), data_end)) {
					LOG_WARN("Buffer overflow sub_id(?), line(?)", sub_id, __LINE__);
					return false;
				}

				uint32_t count{};
				std::memcpy(&count, dynamic_data, sizeof(uint32_t));
				if (!advance_cursor(dynamic_data, sizeof(uint32_t), data_end)) {
					LOG_WARN("Buffer overflow sub_id(?), line(?)", sub_id, __LINE__);
					return false;
				}

				//now you've item_id and count to add as reward.

				break;
			}

			case ESubTour::MANAGE_PLAYER: {
				uint32_t pid{};
				std::memcpy(&pid, dynamic_data, sizeof(uint32_t));
				if (!advance_cursor(dynamic_data, sizeof(uint32_t), data_end)) {
					LOG_WARN("Buffer overflow sub_id(?), line(?)", sub_id, __LINE__);
					return false;
				}

				const char* cmd = reinterpret_cast<const char*>(dynamic_data);

				if (!strcmp(cmd, "disconnect")) {
					//dc player
				}
				else if (!strcmp(cmd, "ban")) {
					//ban player
				}
				//...
				break;
			}
			default:
				break;

			}
			//eof EXAMPLE_COMPLEX
			break;
		}
		default:
			break;
		}

		return false;
	}

	bool MobiClient::SendLoginResponse(const SMValidateMobileLogin& pack, bool is_valid) {
		TMP_BUFFER buf(sizeof(MSValidateMobileLogin));
		MSValidateMobileLogin res{};
		res.is_valid = is_valid;
		res.acc_id = pack.acc_id;
		res.request_id = pack.request_id;
		buf.write(&res, sizeof(MSValidateMobileLogin));
		return SendPacket(buf.get());
	}

	bool MobiClient::HandleValidateLogin(TDataRef data) {
		auto* pack = reinterpret_cast<const SMValidateMobileLogin*>(data.data());
		auto* login = reinterpret_cast<const char*>(data.data() + sizeof(SMValidateMobileLogin));
		auto* pw = reinterpret_cast<const char*>(login + strlen(login) + 1);
#if __BUILD_FOR_GAME__
		if (test_server) /*no check*/ {
			LOG_TRACE("Test server: Login(?) successful", login);
			return SendLoginResponse(*pack, true);
		}

		std::string query = loggerInstance.WriteBuf(
			"SELECT id FROM ?.account WHERE login='?' AND password = PASSWORD(?) LIMIT 1", mobi_game::consts::SCHEMA_ACCOUNT, login, pw);

		std::unique_ptr<SQLMsg> ret(DBManager::instance().DirectQuery(query.c_str()));
		if (!ret) {
			LOG_TRACE("Sql response is nullptr");
			return SendLoginResponse(*pack, false);
		}
		
		SQLResult* sql_res = ret->Get();
		if (!sql_res) {
			LOG_TRACE("Weird stuff line(?)", __LINE__);
			return SendLoginResponse(*pack, false);
		}
		else if (sql_res->uiNumRows == 0)
		{
			LOG_TRACE("Login(?): username or pw wrong.", login);
			return SendLoginResponse(*pack, false);
		}

		uint32_t sql_accID{};
		MYSQL_ROW row = mysql_fetch_row(sql_res->pSQLResult);
		str_to_number(sql_accID, row[0]);
		if (sql_accID == 0) {
			LOG_TRACE("AccountID in sql is not valid: ?", sql_accID);
			return SendLoginResponse(*pack, false);
		}

		LOG_TRACE("Login(?) successful", login);
		return SendLoginResponse(*pack, true);
#else
		return true;
#endif
	}

#if !__MT_DB_INFO__
	bool MobiClient::HandleGetCache(TDataRef data) {
		auto* pkt = reinterpret_cast<const SMGetCache*>(data.data());
#if __BUILD_FOR_GAME__
		auto& db_inst = DBManager::instance();
		std::string query = loggerInstance.WriteBuf(
			query::QUERY_ACCOUNT_WITH_EMPIRE + " WHERE a.id = ? LIMIT 1", pkt->acc_id);
		//SELECT Query calistir ve set'i guncelle. : Email ve authority field'leri gerekli ve bunlari TAccount'tan dogrudan alamiyoruz.

		std::unique_ptr<SQLMsg> ret(db_inst.DirectQuery(query.c_str()));
		if (!ret) {
			LOG_TRACE("Sql response is nullptr");
			return false;
		}
		SQLResult* sql_res = ret->Get();
		if (!sql_res) {
			LOG_TRACE("Weird stuff line(?)", __LINE__);
			return false;
		}
		if (sql_res->uiNumRows == 0){
			LOG_TRACE("There is not account with id(?).", pkt->acc_id);
			return false;
		}
		MYSQL_ROW row = mysql_fetch_row(sql_res->pSQLResult);

		MSCache::Account data{
			//login, id, empire, email, authority
			row[1], std::stoi(row[0]), std::stoi(row[2]), row[3], std::stoi(row[4])
		};
#endif

		MSDataUpdate packet{};
		packet.cache_type = static_cast<uint8_t>(ECacheType::ACCOUNT);
		packet.is_invalidate = false;
		packet.size = sizeof(MSDataUpdate) + sizeof(MSCache::Account);

		TMP_BUFFER buf(packet.size);
		buf.write(&packet, sizeof(MSDataUpdate));
		buf.write(&data, sizeof(MSCache::Account));

		LOG_TRACE("Account(id: ?) cache sent", pkt->acc_id);
		return SendPacket(buf.get());
	}
#endif

#if __OFFSHOP__ == 1 && __BUILD_FOR_GAME__
	bool MobiClient::HandleOffshop(TDataRef data) {
		/*bu paket dogrudan islemi yapan karakterin bulundugu porta gonderilir, karakter bulunmalidir.*/
		auto* pkt = reinterpret_cast<const SMOffshop*>(data.data());
		auto sub_header = static_cast<ESubOffshop>(pkt->sub_id);
		LOG_TRACE("Offshop modify packet(sub_id: ?) received: shop_pid(?), sender_pid(?)", sub_header, pkt->shop_pid, pkt->sender_pid);

		if (sub_header != ESubOffshop::ITEM_BUY && pkt->shop_pid != pkt->sender_pid) /*impossible case*/ {
			LOG_TRACE("Pid(?) trying to modify another character's(?) offshop.", pkt->sender_pid, pkt->shop_pid);
			return false;
		}

		LPCHARACTER sender_ch = CHARACTER_MANAGER::instance().FindByPID(pkt->sender_pid);
		if (!sender_ch) {
			LOG_TRACE("Sender(?) is not online for mobile doing things to shop(?) from mobile.", pkt->sender_pid, pkt->shop_pid);
			sendShopOpResponse(pkt->sender_pid, EResponseShopOperation::NOT_EXISTS_CH);
			return false;
		}

		LPDESC sender_desc = sender_ch->GetDesc();
		if (!sender_desc || !sender_desc->is_mobile_request) {
			LOG_TRACE("Desc of sender(pid: ?) is not created for mobile.", pkt->sender_pid);
			sendShopOpResponse(pkt->sender_pid, EResponseShopOperation::INGAME_REAL);
			return false;
		}

		sender_desc->last_activity = std::chrono::steady_clock::now();

		auto& ikaInstance = ikashop::GetManager();
		
		LOG_TRACE("Sub header(?) processing", sub_header);

		switch (sub_header)
		{
		case ESubOffshop::ITEM_REMOVE: {
			auto* pkt_sec = reinterpret_cast<const offshop::TItemRemove*>(data.data() + sizeof(SMOffshop));
			return ikaInstance.RecvShopRemoveItemClientPacket(sender_ch, pkt_sec->vid);
		}
		case ESubOffshop::ITEM_BUY:  {
			auto* pkt_sec = reinterpret_cast<const offshop::SMItemBuy*>(data.data() + sizeof(SMOffshop));
			long long seenprice = pkt_sec->seen_price.GetTotalAsYang(EMisc::YANG_PER_CHEQUE);
			LOG_TRACE("Item buy will work now: ch(?), shop_pid(?), item_vid(?), seen_price(?), channel(?)", sender_ch, pkt->shop_pid, pkt_sec->vid, seenprice, g_bChannel);
			return ikaInstance.RecvShopBuyItemClientPacket(sender_ch, pkt->shop_pid, pkt_sec->vid, false, seenprice);
		}
		case ESubOffshop::ITEM_UPDATE_POS: {
			auto* pkt_sec = reinterpret_cast<const offshop::TItemUpdatePos*>(data.data() + sizeof(SMOffshop));
			std::shared_ptr<ikashop::CShop> shop = ikaInstance.GetShopByOwnerID(pkt->shop_pid);
			if (!shop) {
				LOG_TRACE("Shop(pid: ?) not exist, sender_pid(?)", pkt->shop_pid, pkt->sender_pid);
				sendShopOpResponse(pkt->sender_pid, EResponseShopOperation::NOT_EXISTS_SHOP);
				return false;
			}

#ifdef EXTEND_IKASHOP_PRO
			if (shop->GetDuration() == 0) {
				LOG_TRACE("Shop(?) has no duration", pkt->shop_pid);
				sendShopOpResponse(pkt->sender_pid, EResponseShopOperation::NOT_EXISTS_SHOP);
				return false;
			}
#endif
			auto shopItem = shop->GetItem(pkt_sec->vid);
			if (!shopItem) {
				LOG_TRACE("Item(vid:?) not exists in shop(?)", pkt_sec->vid, pkt->shop_pid);
				sendShopOpResponse(pkt->sender_pid, EResponseShopOperation::NOT_EXISTS_ITEM);
				return false;
			}
			ikaInstance.RecvShopMoveItemClientPacket(sender_ch, shopItem->GetInfo().pos, pkt_sec->pos_uptodate);
			return true;
		}
		case ESubOffshop::ITEM_UPDATE_PRICE: {
			auto* pkt_sec = reinterpret_cast<const offshop::TItemUpdatePrice*>(data.data() + sizeof(SMOffshop));
			return ikaInstance.RecvShopEditItemClientPacket(sender_ch, pkt_sec->vid, 
				ikashop::TPriceInfo{ pkt_sec->price.yang, static_cast<int>(pkt_sec->price.cheque)});
		}
		case ESubOffshop::DURATION_RESTORE: {
			return ikaInstance.RecvShopCreateNewClientPacket(sender_ch);
		}
		default:
			break;
		}

		return false;
	}

	bool MobiClient::sendShopOpResponse(uint32_t to_pid, EResponseShopOperation response) {
		offshop::MSResponseOperation pack{};
		pack.to_pid = to_pid;
		pack.response = static_cast<std::underlying_type_t<EResponseShopOperation>>(response);

		constexpr auto res_pack_size = sizeof(offshop::MSResponseOperation);

		TMP_BUFFER buf(res_pack_size);
		buf.write(&pack, res_pack_size);
		return SendPacket(buf.get());
	}

	bool MobiClient::sendShopOpResponse(CHARACTER* ch, EResponseShopOperation response) {
		if (!ch) return false;
		auto* desc = ch->GetDesc();
		if (!desc || !desc->is_mobile_request) return false;
		return sendShopOpResponse(ch->GetPlayerID(), response);
	}
#endif

	bool MobiClient::HandleModifyCharacter(TDataRef data) {
		auto* pkt = reinterpret_cast<const SMModifyCharacter*>(data.data());
		auto sub_header = static_cast<ESubModifyCharacter>(pkt->sub_id);
		switch (sub_header)
		{
		case ESubModifyCharacter::DISCONNECT: {
			auto ch = CHARACTER_MANAGER::instance().FindByPID(pkt->pid);
			if (!ch) {
				LOG_TRACE("Character(?) not found.", pkt->pid);
				sendModifyResponse(pkt->pid, EResponseModify::NOT_EXISTS_CH);
				return false;
			}
			auto desc = ch->GetDesc();
			if (!desc) {
				LOG_TRACE("Desc of ch(?) not found.", pkt->pid);
				sendModifyResponse(pkt->pid, EResponseModify::NOT_EXISTS_CH);
				return false;
			}
			DESC_MANAGER::instance().DestroyLoginKey(desc);
			DESC_MANAGER::instance().DestroyDesc(desc);
			sendModifyResponse(pkt->pid, EResponseModify::SUCCESS);
			return true;
		}
		case ESubModifyCharacter::LOAD_CH: {
			//This packet received by auth mt core directly! So we are in the auth core.
			auto* pkt_sec = reinterpret_cast<const modify::TLoadCharacter*>(data.data() + sizeof(SMModifyCharacter));
			return mobileChInstance.CharacterLoad(
				TMobiLoginInfo{ pkt->pid, pkt_sec->login }, pkt_sec->pw) == ELoadChResult::LOADING;
		}
		default:
			break;
		}
		return false;
	}
}
#endif