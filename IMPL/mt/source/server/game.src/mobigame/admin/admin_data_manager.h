#pragma once
#if __MOBICORE__
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

#include <Utility/flag_wrapper.h>
#include "constants/packets.h"

struct TJsonFile;

//her core kendi authority'sine gore tasklari gerceklestirebilir.
namespace mobi_game {
	class CNotificationManager;
	class CConfigManager;
	class MobiClient;
	class GameClientBase;

#if __MT_DB_INFO__
	struct TDBInfo {
		const std::string& host;
		const std::string& user;
		const std::string& password;
		TDBInfo(const std::string& _host, const std::string& _user, const std::string& _pw)
			: host(_host), user(_user), password(_pw) {
		}
	};
#endif

	struct TBridgeInfo {
		const std::string& host;
		uint16_t port;
		TBridgeInfo(const std::string& _host, uint16_t _port)
			: host(_host), port(_port) {
		}
	};

	class CAdminDataManager final {
		friend class MobiClient;
		GameClientBase* client_ = nullptr;
		NUtility::CFlagWrapper authority_ = 0;

		std::unique_ptr<CNotificationManager> notification_manager_;
		std::unique_ptr<CConfigManager> config_manager_;

		std::unique_ptr<TJsonFile> f_info_;
#if __MT_DB_INFO__
		std::unique_ptr<TDBInfo> db_info_;
#endif
		std::unique_ptr<TBridgeInfo> bridge_info_;
	public:
		CAdminDataManager(GameClientBase* client);
		~CAdminDataManager() noexcept;
	private:
		bool CheckInfoFields() const;

		const char* GetBridgeHost() const;
		uint16_t GetBridgePort() const;
	public:
		void LoadInfoFile();
		std::vector<uint8_t> GetDBCache();
	public:
		void SetAuthority(uint16_t authority_type) noexcept;
		bool HasAuthority(uint16_t flag) { return authority_.HasFlag(flag); }

		//yetkiler guncellendiginde veya tekrar baglanildiginda calistir.
		void Refresh();
	public:
		void Process();
	};
}

#endif