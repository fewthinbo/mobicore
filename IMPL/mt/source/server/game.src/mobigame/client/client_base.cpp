#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif
#include "client_base.h"

#if __BUILD_FOR_GAME__
#include "messenger_manager.h"
#endif

#include "mobi_client.h"

#include <Singletons/log_manager.h>

#include "admin/admin_data_manager.h"
#include "unprocessed/unprocessed.h"

using namespace network;

namespace mobi_game {
	GameClientBase::~GameClientBase() noexcept = default;

	void GameClientBase::OnDisconnect() {
		mobileInstance.SetBridgeCacheStatus(false);
#if __BUILD_FOR_GAME__
		//messenger'da herkes offline isaretlenir.
		MessengerManager::instance().HandleConnectionLost();
#endif
	}

	bool GameClientBase::IsValidHeader(THEADER header) const {
		return header > 0 && header <= HEADER_MAX;
	}

	bool GameClientBase::OnDataReceived(THEADER header, const std::vector<uint8_t>& data) {
		return mobileInstance.HandlePacket(header, data);
	}

	void GameClientBase::OnEnabledEnc() {
		if (mobileInstance.unprocessed_helper_) {
			mobileInstance.unprocessed_helper_->unprocessed_process();
		}
		if (mobileInstance.admin_data_manager_) {
			mobileInstance.admin_data_manager_->Refresh();
		}
	}
}
#endif