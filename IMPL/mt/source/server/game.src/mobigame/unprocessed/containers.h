#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <memory>

#include <Network/buffer.h>

#include "constants/consts.h"

namespace mobi_game {
	class GameNetworkClient;

	struct TMSMessageContainer final {
		network::TMP_BUFFER buf;
		uint32_t container_id = 0;
		uint32_t sender_pid = 0;
		std::string receiver_name = "";
		std::chrono::steady_clock::time_point created_time;

		TMSMessageContainer(uint32_t _sender_pid,
			const std::string& _receiver_name, const std::string& _message, uint32_t _containerID, uint16_t code_page = consts::DEFAULT_CODEPAGE);
		~TMSMessageContainer() noexcept;
		bool process(GameNetworkClient* client);
		bool is_expired(const std::chrono::seconds& timeout) const noexcept;
		uint32_t get_sender_pid() const noexcept;
		const char* get_receiver_name() const noexcept;
	};

	struct TUnprocessedContainer final {
		uint32_t container_id = 0;
		std::vector<uint8_t> data;
		bool bUsed = false;
		std::chrono::steady_clock::time_point created_time;
		bool need_encrypt = true;

		explicit TUnprocessedContainer(const std::vector<uint8_t>& _data, uint32_t _containerID, bool _encrypt = true);
		~TUnprocessedContainer() noexcept;

		bool is_expired(const std::chrono::seconds& timeout) const noexcept;
	};
}
