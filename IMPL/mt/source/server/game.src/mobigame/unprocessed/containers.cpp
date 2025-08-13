#include "containers.h"


#include <Network/buffer.h>
#include "constants/packets.h"

#include "client/client_core.h"

using namespace network;

namespace mobi_game {
	TMSMessageContainer::~TMSMessageContainer() noexcept = default;
	TMSMessageContainer::TMSMessageContainer(uint32_t _sender_pid,
		const std::string& _receiver_name, const std::string& _message, uint32_t _containerID, uint16_t code_page)
		: container_id(_containerID),
		sender_pid(_sender_pid),
		receiver_name(_receiver_name),
		created_time(std::chrono::steady_clock::now()) {

		MSMessageName packet{};
		packet.header = HEADER_MS_MESSAGE_NAME;
		network::TMP_BUFFER::str_copy(packet.recevier_name, sizeof(packet.recevier_name), _receiver_name.c_str());
		packet.sender_pid = _sender_pid;
		packet.code_page = code_page;
		packet.size = _message.size() + 1;

		buf.clear_and_resize(sizeof(packet) + packet.size);
		buf.write(&packet, sizeof(packet));
		buf.write(_message.data(), packet.size);
	}

	bool TMSMessageContainer::process(GameNetworkClient* client) {
		if (!client) return false;
		return client->Send(buf.get()) == ESendResult::SUCCESS;
	}

	bool TMSMessageContainer::is_expired(const std::chrono::seconds& timeout) const noexcept {
		auto now = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::seconds>(now - created_time) > timeout;
	}

	uint32_t TMSMessageContainer::get_sender_pid() const noexcept { return sender_pid; }
	const char* TMSMessageContainer::get_receiver_name() const noexcept { return receiver_name.c_str(); }

	TUnprocessedContainer::~TUnprocessedContainer() noexcept = default;
	TUnprocessedContainer::TUnprocessedContainer(const std::vector<uint8_t>& _data, uint32_t _containerID, bool _encrypt)
		: container_id(_containerID),
		data(_data), bUsed(false),
		created_time(std::chrono::steady_clock::now()), need_encrypt(_encrypt) {
	}

	bool TUnprocessedContainer::is_expired(const std::chrono::seconds& timeout) const noexcept {
		auto now = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::seconds>(now - created_time) > timeout;
	}
}