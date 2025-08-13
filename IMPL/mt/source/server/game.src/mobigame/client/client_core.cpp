#include "client_core.h"

#include <Singletons/log_manager.h>

#include "constants/packets.h"
#include "mobi_client.h"
#include "unprocessed/unprocessed.h"
#include "admin/admin_data_manager.h"

using namespace network;

namespace mobi_game {
	GameNetworkClient::~GameNetworkClient() noexcept{
		Disconnect(false);
	}

	void GameNetworkClient::OnDisconnect() {
		mobileInstance.SetBridgeCacheStatus(false);
#ifdef MOBICORE
		//messenger'da herkes offline isaretlenir.
		MessengerManager::instance().HandleConnectionLost();
#endif
	}

	bool GameNetworkClient::IsValidHeader(THEADER header) const {
		return header <= HEADER_MAX;
	}

	bool GameNetworkClient::OnDataReceived(THEADER header, const std::vector<uint8_t>& data) {
		return mobileInstance.HandlePacket(header, data);
	}

	void GameNetworkClient::OnEnabledEnc() {
		mobileInstance.unprocessed_helper_->unprocessed_process();
		mobileInstance.admin_data_manager_->Refresh();
	}

}