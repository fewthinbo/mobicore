#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif

#include "mobi_client.h"

#include <Singletons/log_manager.h>
#include <Network/common.h>

#include "client/client_base.h"
#include "admin/admin_data_manager.h"
#include "unprocessed/message_queue.h"
#include "unprocessed/unprocessed.h"

#include "constants/consts.h"


using namespace network;
namespace mobi_game {
	using namespace consts;

	MobiClient::~MobiClient() noexcept = default;
	MobiClient::MobiClient()
		: client_impl_(std::make_unique<GameClientBase>()) {
		if (client_impl_) {
			LOG_TRACE("member variables initialized successfully");
			auto ptr = client_impl_.get();
			message_helper_ = std::make_unique<CMessageHelper>(ptr);
			unprocessed_helper_ = std::make_unique<CUnprocessedHelper>(ptr);
			admin_data_manager_ = std::make_unique<CAdminDataManager>(ptr);
		}
		RegisterPackets();
	}

	//for event-driven arch
	void MobiClient::Process() {
		SendSync();

		client_impl_->DoWork();

		if (message_helper_) {
			message_helper_->message_cleanup_expired();
		}
		if (unprocessed_helper_) {
			unprocessed_helper_->unprocessed_cleanup_expired();
		}
		if (admin_data_manager_) {
			admin_data_manager_->Process();
		}
	}

	void MobiClient::Disconnect(bool need_reconnect) noexcept {
		client_impl_->Disconnect(need_reconnect);
	}

	void MobiClient::ConnectToBridge() {
		if (client_impl_->IsConnected() || !admin_data_manager_) return;

		client_impl_->Connect(admin_data_manager_->GetBridgeHost(), admin_data_manager_->GetBridgePort());
	}

	bool MobiClient::SendPacket(std::vector<uint8_t>& data, bool encrypt) {
		if (data.empty()) return false;

		THEADER header = data[0];

		/*if (!IsConnected()) {
			LOG_TRACE("header(?) failed: not connected to server", header);
			return false;
		}*/

		ESendResult res = client_impl_->Send(data, encrypt);
		switch (res)
		{
		case ESendResult::SUCCESS:
			LOG_TRACE("header(?) sent successfully", header);
			return true;
		case ESendResult::NOT_CONNECTED:
			LOG_TRACE("header(?) couldn't sent, adding to unprocessed queue", header);
			unprocessed_helper_->unprocessed_add(data, encrypt);
			return false;
		default:
			return false;
		}
	}

	void MobiClient::SetBridgeCacheStatus(bool uptodate) {
		if (bridge_cache_ready_ == uptodate) return;
		bridge_cache_ready_ = uptodate;

		if (!bridge_cache_ready_) {
			sync_packet_sent_ = false; //cache hazir oldugunda sync paketi tekrar gonderilecek.
		}
	}

	bool MobiClient::IsCoreP2PManager() const noexcept {
		if (!admin_data_manager_) return false;
		return admin_data_manager_->HasAuthority(EAuthorityType::P2P_MANAGER);
	}
}
#endif