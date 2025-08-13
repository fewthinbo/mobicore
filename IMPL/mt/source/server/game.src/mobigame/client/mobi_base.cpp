#ifdef MOBICORE
#include "stdafx.h"
#endif
#include "mobi_base.h"

#ifdef MOBICORE
#include "char.h"
#include "char_manager.h"
#include "p2p.h"
#include "desc.h"
#include "messenger_manager.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#endif

#include <Singletons/log_manager.h>

#include "constants/packets.h"
#include <Network/buffer.h>

#include <admin/admin_data_manager.h>
#include <admin/config_manager.h>
#include <unprocessed/message_queue.h>
#include <unprocessed/unprocessed.h>

#include "client_core.h"

using namespace network;

namespace mobi_game {
	GameClientBase::~GameClientBase() noexcept = default;

	GameClientBase::GameClientBase(){
		client_ = std::make_unique<GameNetworkClient>();
		auto client_ptr = client_.get();

		//asagidaki siniflar destructorlarinda ptr'i kullanmamalidir. 
		message_helper_ = std::make_unique<CMessageHelper>(client_ptr);
		unprocessed_helper_ = std::make_unique<CUnprocessedHelper>(client_ptr);
		admin_data_manager_ = std::make_unique<CAdminDataManager>(client_ptr);

		RegisterPackets();
	}

	bool GameClientBase::IsConnected() const noexcept {
		return client_->IsConnected();
	}

	//for event-driven arch
	void GameClientBase::Process() {
		SendSync();

		client_->DoWork();

		message_helper_->message_cleanup_expired();
		unprocessed_helper_->unprocessed_cleanup_expired();
		admin_data_manager_->Process();
	}

	void GameClientBase::Connect() {
		if (client_->IsConnected()) return;

		client_->Connect(admin_data_manager_->GetBridgeHost(), admin_data_manager_->GetBridgePort());
	}

	void GameClientBase::Disconnect(bool need_reconnect) noexcept {
		client_->Disconnect(need_reconnect);
	}

	bool GameClientBase::SendPacket(std::vector<uint8_t>& data, bool encrypt) {
		if (data.empty()) return false;

		THEADER header = data[0];

		if (!client_->IsConnected()) {
			LOG_TRACE("header(?) failed: not connected to server", header);
			return false;
		}

		ESendResult res = client_->Send(data, encrypt);
		switch (res)
		{
		case ESendResult::SUCCESS:
			return true;
		case ESendResult::NOT_CONNECTED:
			LOG_TRACE("header(?) couldn't sent, adding to unprocessed queue", header);
			unprocessed_helper_->unprocessed_add(data, encrypt);
			return false;
		default:
			return false;
		}
	}

	void GameClientBase::SetBridgeCacheStatus(bool uptodate) {
		if (bridge_cache_ready_ == uptodate) return;
		bridge_cache_ready_ = uptodate;

		if (!bridge_cache_ready_) {
			sync_packet_sent_ = false; //cache hazir oldugunda sync paketi tekrar gonderilecek.
		}
	}
}