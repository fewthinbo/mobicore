#include "unprocessed.h"

#include <Singletons/log_manager.h>

#include <Network/common.h>

#include "constants/packets.h"
#include "constants/consts.h"
#include "client/client_core.h"

using namespace network;

namespace mobi_game {
	using namespace consts;
	static inline uint32_t s_unprocessed_container_id = 0;

	//mt -> bridge baglantisi tekrar kuruldugunda tum bilgilerinin alinmasi gereken paketler:
	static inline bool IsSyncPacket(THEADER header) {
		switch (header)
		{
		case HEADER_MS_DATA_UPDATE:
		case HEADER_MS_GUILD_JOIN:
		case HEADER_MS_GUILD_LEAVE:
		case HEADER_MS_GUILD_WAR:
			return true;
		default:
			break;
		}
		return false;
	}

	void CUnprocessedHelper::unprocessed_process() {
		if (!client_) return;
		for (auto& unprocessed : unp_packets_) {
#ifdef _DEBUG
			if (!unprocessed->data.empty()) {
				LOG_TRACE("sync, header: ?", unprocessed->data[0]);
			}
#endif
			if (client_->Send(unprocessed->data, unprocessed->need_encrypt) == ESendResult::SUCCESS) {
				unprocessed->bUsed = true;
			}
		}
		unprocessed_cleanup_expired();
	}

	bool CUnprocessedHelper::unprocessed_add(const std::vector<uint8_t>& data, bool encrypt) noexcept {
		if (data.empty()) return false;
		THEADER header = data[0];

		if (!IsSyncPacket(header)) return false;

		if (unp_packets_.size() >= MAX_UNPROCESSED_COUNT) {
			unp_packets_.erase(unp_packets_.begin());
		}
		unp_packets_.emplace_back(std::make_unique<TUnprocessedContainer>(data, s_unprocessed_container_id++, encrypt));
		LOG_TRACE("Added sync unprocessed, header: ?", header);
		return true;
	}

	void CUnprocessedHelper::unprocessed_cleanup_expired() {
		auto now = std::chrono::steady_clock::now();
		if (now - last_cleanup_time_ < std::chrono::seconds(CLEANUP_INTERVAL)) return;
		
		last_cleanup_time_ = now;
		
		size_t removed_count = 0;
		auto it = std::remove_if(unp_packets_.begin(), unp_packets_.end(),
			[this](auto& msg) { return msg->is_expired(std::chrono::seconds(DATA_TIMEOUT)) || msg->bUsed; });
		removed_count = std::distance(it, unp_packets_.end());
		unp_packets_.erase(it, unp_packets_.end());
		LOG_TRACE("Removed ? expired unprocessed packets", removed_count);
	}
}
