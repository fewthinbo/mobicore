#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <chrono>

#include <Utility/flag_wrapper.h>
#include <Network/common.h>

namespace mobi_game {
	class CAdminDataManager;
	class CMessageHelper;
	class CUnprocessedHelper;
	class GameNetworkClient;
	
	class GameClientBase {
	public:
		GameClientBase();
		virtual ~GameClientBase() noexcept;
		void Connect();
		void Disconnect(bool need_reconnect = true) noexcept;
		void Process();
		bool IsConnected() const noexcept;
	protected:
		using TDataRef = const std::vector<uint8_t>&;
		bool SendPacket(std::vector<uint8_t>& data, bool encrypt = true); //non-const because of encryption processes etc.
	private:
		void RegisterPackets() noexcept;
	public:
		bool HandlePacket(network::THEADER header, const std::vector<uint8_t>& data);

	private: //packet handlers
		bool HandleDbInfo(TDataRef data);
		bool HandleMobilePm(TDataRef data) const;
		bool HandleUserCheck(TDataRef data) const;
		bool HandleMobileLogin(TDataRef data) const;
		bool HandleMobileLogout(TDataRef data) const;
		bool HandleCacheStatus(TDataRef data);
		bool HandleKeyExchange(TDataRef data);
		bool HandleForwardPacket(TDataRef data) const;
		bool HandleValidateLogin(TDataRef data);
#if !__MT_DB_INFO__
		bool HandleGetCache(TDataRef data); //yeni acc kaydini senkronize et
#endif
	public: //util
		void GetSyncData(std::vector<uint8_t>& data) const noexcept;
		void SendSync();
		void SetBridgeCacheStatus(bool uptodate);
	public:
		void setOnlineRefreshInterval(std::chrono::seconds sec) { refresh_interval_ = sec; };
	protected:
		std::chrono::steady_clock::time_point last_sent_{};
		std::chrono::seconds refresh_interval_ = std::chrono::seconds(15);
	protected:
		std::unique_ptr<GameNetworkClient> client_;
		std::unique_ptr<CAdminDataManager> admin_data_manager_;
		std::unique_ptr<CMessageHelper> message_helper_;
		std::unique_ptr<CUnprocessedHelper> unprocessed_helper_;
        bool bridge_cache_ready_ = false; //use SetBridgeCacheStatus instead of directly set.
        bool sync_packet_sent_ = false;
	};
}

