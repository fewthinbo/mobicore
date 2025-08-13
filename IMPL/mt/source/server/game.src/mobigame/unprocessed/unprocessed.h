#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <chrono>

#include "containers.h"

namespace mobi_game {
	class GameNetworkClient;

	class CUnprocessedHelper final {
	private:		
		std::vector<std::unique_ptr<TUnprocessedContainer>> unp_packets_;
		std::chrono::steady_clock::time_point last_cleanup_time_{};
	public:
		explicit CUnprocessedHelper(GameNetworkClient* client) : client_(client) {}
	public:
		//eklendiyse true doner -> daha sonra(bridge server available oldugunda) islenmek uzere kaydet. 
		bool unprocessed_add(const std::vector<uint8_t>& data, bool encrypt = true) noexcept;
		void unprocessed_process();
		void unprocessed_cleanup_expired();
	private:
		GameNetworkClient* client_ = nullptr;
	};
}