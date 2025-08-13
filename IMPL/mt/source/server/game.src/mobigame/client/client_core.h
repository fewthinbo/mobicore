#pragma once

#include <Network/common.h>
#include <raw_client.h>

namespace mobi_game{
	class GameNetworkClient final : public network::NetworkClientImpl {
		public:
			~GameNetworkClient() noexcept;
		protected:
			void OnDisconnect() override;
			void OnEnabledEnc() override;
			bool OnDataReceived(network::THEADER header, const std::vector<uint8_t>& data) override;
			bool IsValidHeader(network::THEADER header) const override;
	};
}