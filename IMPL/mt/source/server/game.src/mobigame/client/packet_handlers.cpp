#include "mobi_base.h"

#include <vector>

#include <Singletons/log_manager.h>
#include <Network/buffer.h>

#include "constants/packets.h"

#include "admin/admin_data_manager.h"
#include "unprocessed/message_queue.h"
#include "unprocessed/unprocessed.h"
#include "client_core.h"

using namespace network;

namespace mobi_game {
	using namespace consts;

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
}