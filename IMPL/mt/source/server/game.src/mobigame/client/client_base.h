#pragma once
#if __MOBICORE__
#include <vector>
#include <cstdint>

#include <Network/common.h>

#include <raw_client.h>

namespace mobi_game {
	class GameClientBase final : public network::NetworkClientImpl {
	private:
		void OnDisconnect() override;
		void OnEnabledEnc() override;
		bool OnDataReceived(network::THEADER header, const std::vector<uint8_t>& data) override;
		bool IsValidHeader(network::THEADER header) const override;
	public:
		~GameClientBase() noexcept override;
	};
}
#endif

