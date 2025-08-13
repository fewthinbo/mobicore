#pragma once
#include <memory>
#include <string>
#include <cstdint>

#include <Utility/flag_wrapper.h>
#include "constants/packets.h"

struct TJsonFile;

//her core kendi authority'sine gore tasklari gerceklestirebilir.
namespace mobi_game {
	class CNotificationManager;
	class CConfigManager;
	class GameNetworkClient;
	class GameClientBase;

	struct TDBInfo {
		const std::string& host;
		const std::string& user;
		const std::string& password;
		TDBInfo(const std::string& _host, const std::string& _user, const std::string& _pw)
			: host(_host), user(_user), password(_pw) {
		}
	};

	struct TBridgeInfo {
		const std::string& host;
		uint16_t port;
		TBridgeInfo(const std::string& _host, uint16_t _port)
			: host(_host), port(_port) {
		}
	};

	class CAdminDataManager final {
		friend class GameClientBase;
		GameNetworkClient* client_ = nullptr;
		NUtility::CFlagWrapper authority_ = 0;

		std::unique_ptr<CNotificationManager> notification_manager_;
		std::unique_ptr<CConfigManager> config_manager_;

		std::unique_ptr<TJsonFile> f_info_;
		std::unique_ptr<TDBInfo> db_info_;
		std::unique_ptr<TBridgeInfo> bridge_info_;
	public:
		CAdminDataManager(GameNetworkClient* client);
		~CAdminDataManager() noexcept;
	private:
		bool CheckInfoFields() const;

		const char* GetBridgeHost() const;
		uint16_t GetBridgePort() const;
	public:
		void LoadInfoFile();
		bool WriteDBSettings(MSDBInfo& packet) const;
	public:
		void SetAuthority(uint16_t authority_type) noexcept;
		bool HasAuthority(uint16_t flag) { return authority_.HasFlag(flag); }

		//yetkiler guncellendiginde veya tekrar baglanildiginda calistir.
		void Refresh();
	public:
		void Process();
	};
}

