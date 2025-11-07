#pragma once
#if __MOBICORE__
#include <string>
#include <cstdint>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#if __BUILD_FOR_GAME__
#include <utility>
#endif
#include <singleton.h>
#include <Network/buffer.h>
#include <Utility/flag_wrapper.h>

#include "constants/consts.h"
#include "constants/packet_constants.h"
#include "constants/packets.h"

#if __BUILD_FOR_GAME__
	#if !__MT_DB_INFO__
		class CGuild;
		struct TSimplePlayer;
	#endif
	#if __OFFSHOP__
		namespace ikashop {
			struct TPriceInfo;
			struct TShopInfo;
			struct TShopItem;
		}
	#endif
#endif

namespace mobi_game {
	class GameClientBase;
	class CAdminDataManager;
	class CMessageHelper;
	class CUnprocessedHelper;

	class MobiClient final :
		public CSingleton<MobiClient> {
	public:
		friend class GameClientBase;
		MobiClient();
		~MobiClient() noexcept;

//==================================== BASE API
	public:
		void ConnectToBridge();
		void Disconnect(bool need_reconnect = true) noexcept;
		void Process();

//==================================== SHITS
	private:
		bool IsCoreP2PManager() const noexcept;
		void GetSyncData(std::vector<uint8_t>& data) const noexcept;
		void SendSync();
		void SetBridgeCacheStatus(bool uptodate);
		void RegisterPackets() noexcept;
		bool HandlePacket(network::THEADER header, const std::vector<uint8_t>& data);
	public:
		void setOnlineRefreshInterval(std::chrono::seconds sec) { refresh_interval_ = sec; };
#if __BUILD_FOR_GAME__
	private:
		std::pair<network::TSIZE, uint32_t> WritePids(network::TMP_BUFFER& buf) const noexcept;
		std::pair<network::TSIZE, uint32_t> WriteWars(network::TMP_BUFFER& buf) const noexcept;
		std::pair<network::TSIZE, uint32_t> WriteMobiCh(network::TMP_BUFFER& buf) const noexcept;
#endif

//==================================== SEND PACKETS
	protected:
		using TDataRef = const std::vector<uint8_t>&;
		bool SendPacket(std::vector<uint8_t>& data, bool encrypt = true); //non-const because of encryption processes etc.
	public:
		bool sendLogin(uint32_t pid, uint32_t map_idx, bool is_mobile_request);
		bool sendLogout(uint32_t pid);
		bool sendMessage(uint32_t sender_pid, uint32_t receiver_pid, const std::string& message, uint16_t code_page = consts::DEFAULT_CODEPAGE);
		bool sendShout(uint32_t pid, const std::string& message, uint16_t code_page = consts::DEFAULT_CODEPAGE);
		bool sendUserCheck(const std::string& nameTo, const std::string& message, uint32_t sender_pid, uint16_t code_page = consts::DEFAULT_CODEPAGE);

		bool sendLevelPacket(uint32_t pid, uint32_t level);

		bool sendGuildJoin(uint32_t guild_id, uint32_t pid);
		bool sendGuildLeave(uint32_t pid);
		bool sendMessengerAdd(const std::string& name, const std::string& target);
		bool sendMessengerRemove(const std::string& name, const std::string& target);
		bool sendOnlineCount(const int(&empires)[4]);

		bool sendGuildWarStart(uint32_t guild_id1, uint32_t guild_id2, uint32_t scoreLimit);
		bool sendGuildWarEnd(uint32_t guild_id1, uint32_t guild_id2);
		bool sendGuildWarPlayerKill(uint32_t guild_id1, uint32_t guild_id2, uint32_t killer_pid, uint32_t victim_pid);
		bool sendGuildWarPlayerJoin(uint32_t guild_id, uint32_t pid);
		bool sendGuildWarPlayerLeave(uint32_t guild_id, uint32_t pid);
		bool sendGuildWarPlayerPositionUpdate(uint32_t guild_id, uint32_t pid, uint32_t pos[2]);
		bool sendGuildWarMapNotification(uint32_t guild_id1, uint32_t guild_id2, const std::string& message);

#if __MT_DB_INFO__
		bool sendCharacterCreate(uint32_t pid);
#elif __BUILD_FOR_GAME__
		bool sendCharacterCreate(const TSimplePlayer& player, uint32_t acc_id);
#elif PLATFORM_WINDOWS
		bool sendCharacterCreate(const MSCache::Player& player);
#endif
		bool sendCharacterDelete(uint32_t pid);

#if __MT_DB_INFO__
		bool sendGuildCreate(uint32_t guild_id);
#elif __BUILD_FOR_GAME__
		bool sendGuildCreate(const CGuild& guild);
#elif PLATFORM_WINDOWS
		bool sendGuildCreate(const MSCache::Guild& guild);
#endif
		bool sendGuildDelete(uint32_t guild_id);

		bool sendLadderPoint(uint32_t guild_id, uint32_t point);
		bool sendGuildStats(uint32_t guild_id, uint32_t win, uint32_t draw, uint32_t loss);

		bool sendKill(uint32_t killerID, uint32_t victimID);

		bool sendChangeRace(uint32_t pid, uint8_t race);
		bool sendChangeSex(uint32_t pid, uint8_t sex);
		bool sendChangeEmpire(uint32_t pid, uint8_t empire);
		bool sendChangeName(uint32_t pid, const std::string& name);
		bool sendChLoadStatus(uint32_t pid, EMobiLoad code);

		//use wherever you want for send mobile notification to a player.
		//Example: send when sold player's item in offshop
		bool sendMobileNotification(uint32_t pid, const std::string& message, ENotificationChannels channel = ENotificationChannels::SYSTEM);

#if __OFFSHOP__
	public: //offshop
		bool sendShopItemUpdatePos(uint32_t owner_pid, uint32_t vid_item, uint32_t uptodate);
#if __BUILD_FOR_GAME__
		bool sendShopItemUpdatePrice(uint32_t owner_pid, uint32_t vid_item, const ikashop::TPriceInfo& price);
		bool sendShopCreate(const ikashop::TShopInfo& info);
#endif
		bool sendShopClose(uint32_t owner_pid);
		bool sendShopUpdateSlotCount(uint32_t owner_pid, uint32_t uptodate);
#if __BUILD_FOR_GAME__
		bool sendShopItemAdd(uint32_t owner_pid, const ikashop::TShopItem& item);
#endif
		bool sendShopItemRemove(uint32_t owner_pid, uint32_t vid_item);
		bool sendShopItemBuy(uint32_t owner_pid, uint32_t buyer_id, uint32_t vid_item);

		bool sendShopOpResponse(uint32_t to_pid, EResponseShopOperation response);
#endif
		bool SendLoginResponse(uint32_t acc_id, bool is_valid);
//==================================== EOF SEND PACKETS


//==================================== TEST
	public:
		//100 intensity'de 1 saniye boyunca yazma islemi yapar.
		bool spamTest(uint8_t intensity);
//==================================== EOF TEST

//==================================== HANDLERS
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
#if __OFFSHOP__
		bool HandleOffshop(TDataRef data);
#endif
		bool HandleModifyCharacter(TDataRef data);
//==================================== EOF HANDLERS
	private:
		std::unique_ptr<CAdminDataManager> admin_data_manager_;
		std::unique_ptr<CMessageHelper> message_helper_;
		std::unique_ptr<CUnprocessedHelper> unprocessed_helper_;
		std::unique_ptr<GameClientBase> client_impl_; //important: must be the last member to be destructed last.

		std::chrono::steady_clock::time_point last_sent_{};
		std::chrono::seconds refresh_interval_ = std::chrono::seconds(15);

		bool bridge_cache_ready_{false}; //use SetBridgeCacheStatus instead of directly set.
		bool sync_packet_sent_{ false };
	};
}

#define mobileInstance mobi_game::MobiClient::getInstance()
#endif