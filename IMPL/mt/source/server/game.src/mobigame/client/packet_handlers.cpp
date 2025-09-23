#include "mobi_base.h"

#include <vector>
#include <cstring>

#include <Singletons/log_manager.h>
#include <Network/buffer.h>

#include "constants/packets.h"
#include "constants/custom_packets.h"

#include "admin/admin_data_manager.h"
#include "unprocessed/message_queue.h"
#include "unprocessed/unprocessed.h"
#include "client_core.h"

using namespace network;

namespace mobi_game {
	using namespace consts;
	using namespace custom_packets;

	bool GameClientBase::HandleDbInfo(TDataRef data) {
		LOG_TRACE("Request received.");
		MSDBInfo packet{};
		packet.header = HEADER_MS_DB_INFO;

		if (!admin_data_manager_->WriteDBSettings(packet)) return false;

		TMP_BUFFER buf(sizeof(packet));
		buf.write(&packet, sizeof(packet));
		return SendPacket(buf.get());
	}

	bool GameClientBase::HandleKeyExchange(TDataRef data) {
		auto* sm = reinterpret_cast<const TKeyExchange*>(data.data());
		std::vector<uint8_t> key(data.data() + sizeof(TKeyExchange), data.data() + sizeof(TKeyExchange) + sm->size);
		LOG_TRACE("Key exchange packet received, size(?), key(?)", sm->size, reinterpret_cast<const char*>(key.data()), key.size());

#ifndef _MOBI_PACKET_ENCRYPTION
		return false;
#endif

		TKeyExchange key_packet{};
		key_packet.header = HEADER_MS_KEY_EXCHANGE;

		const std::vector<uint8_t>& public_key = client_->session_get_public_key();
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

		if (!client_->session_enable_encryption(key)) {
			LOG_ERR("an error occured while enable encryption");
			return false;
		}
		return true;
	}

	bool GameClientBase::HandleMobilePm(TDataRef data) const {
		auto* sm = reinterpret_cast<const SMMessage*>(data.data());
		const char* message = reinterpret_cast<const char*>(data.data() + sizeof(SMMessage));

		LOG_TRACE("Pm from mobile: message(?), sender_name(?), receiver_pid(?)", message, sm->name, sm->receiver_pid);
#ifdef MOBICORE
		LPDESC desc = nullptr;

		//desc bul
		if (auto ch = CHARACTER_MANAGER::instance().FindByPID(sm->receiver_pid)) {
			desc = ch->GetDesc();
		}
		else if (auto peer = P2P_MANAGER::instance().FindByPID(sm->receiver_pid)) {
			desc = peer->pkDesc;
		}

		if (!desc){
			LOG_TRACE("? desc not found for receiver_pid(?)", sm->receiver_pid);
			return false;
		}

		TPacketGCWhisper pack{};
		pack.bHeader = HEADER_GC_WHISPER;
		pack.wSize = sizeof(TPacketGCWhisper) + sm->size;
		pack.bType = WHISPER_TYPE_MOBILE;
		strlcpy(pack.szNameFrom, sm->name, sizeof(pack.szNameFrom));

		TEMP_BUFFER tmpbuf{};
		tmpbuf.write(&pack, sizeof(pack));
		tmpbuf.write(message, sm->size);

		desc->Packet(tmpbuf.read_peek(), tmpbuf.size());
		desc->SetRelay("");
		return true;
#endif

		return true;
	}

	bool GameClientBase::HandleUserCheck(TDataRef data) const {
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
#ifdef MOBICORE
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

	bool GameClientBase::HandleMobileLogin(TDataRef data) const {
		auto* packet = reinterpret_cast<const SMLogin*>(data.data());
		LOG_TRACE("Mobile login packet received, name:?", packet->name);
#ifdef MOBICORE
		MessengerManager::instance().MobileLogin(packet->name);
#endif
		return true;
	}
	bool GameClientBase::HandleMobileLogout(TDataRef data) const {
		auto* packet = reinterpret_cast<const SMLogout*>(data.data());
		LOG_TRACE("Mobile logout packet received, name:?", packet->name);
#ifdef MOBICORE
		MessengerManager::instance().MobileLogout(packet->name);
#endif
		return true;
	}

	bool GameClientBase::HandleCacheStatus(TDataRef data) {
		auto* packet = reinterpret_cast<const SMCacheStatus*>(data.data());
		SetBridgeCacheStatus(packet->is_ready);
		return true;
	}

	//For server developers/admins: You can write your own custom packets
	//Those data comes from mobile devices of admins and dynamic sizes of the packets are validated from bridgeServer. 
	//use memcpy in this function for more safety to UB

	bool GameClientBase::HandleForwardPacket(TDataRef data) {
		auto* packet = reinterpret_cast<const SMForward*>(data.data());
		auto sub_id = static_cast<ECustomPackets>(packet->sub_header);
		const uint8_t* dynamic_data = reinterpret_cast<const uint8_t*>(data.data() + sizeof(SMForward)); //dynamic part, if you have
		const uint8_t* data_end = reinterpret_cast<const uint8_t*>(data.data() + packet->size);

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
		}
		default:
			break;
		}

		return false;
	}
}