#pragma once
#if __MOBICORE__

#include <memory>
#include <cstdint>
#include <string>
#include <deque>
#include <chrono>

#include "containers.h"
#include "constants/consts.h"

namespace mobi_game {
	class GameClientBase;

	class CMessageHelper final {
		std::deque<std::unique_ptr<TMSMessageContainer>> message_queue_;
		std::chrono::steady_clock::time_point last_cleanup_time_{};
	public:
		explicit CMessageHelper(GameClientBase* client) : client_(client) {}
	public:
		// Interface implementations
		uint32_t message_get_container_id() const noexcept;
		void message_add(uint32_t sender_pid, const std::string& nameTo, const std::string& message, uint16_t code_page = consts::DEFAULT_CODEPAGE) noexcept;
		bool message_process(uint32_t container_id);
		void message_remove(uint32_t container_id);
		uint32_t message_get_sender_pid(uint32_t container_id) const noexcept;
		const char* message_get_receiver_name(uint32_t container_id) const noexcept;
		const TMSMessageContainer* message_get_container(uint32_t container_id) const noexcept;
		void message_cleanup_expired() noexcept;
	private:
		GameClientBase* client_ = nullptr;
	};
}
#endif