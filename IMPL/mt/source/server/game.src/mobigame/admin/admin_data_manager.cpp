#include "admin_data_manager.h"

#include <Singletons/json_loader.h>


#include "constants/consts.h"
#include "client/client_core.h"

#include "notification.h"
#include "config_manager.h"

using namespace network;

namespace mobi_game {
	using namespace consts;

	CAdminDataManager::CAdminDataManager(GameNetworkClient* client)
		: client_(client) {
		notification_manager_ = std::make_unique<CNotificationManager>(client);
		config_manager_ = std::make_unique<CConfigManager>(client);

		LoadInfoFile();
	}

	CAdminDataManager::~CAdminDataManager() noexcept = default;

	void CAdminDataManager::Process(){
		notification_manager_->doWork();
		config_manager_->doWork();
	}

	void CAdminDataManager::SetAuthority(uint16_t authority_type) noexcept {
		if (authority_.HasFlag(authority_type)) return;
		authority_ = authority_type;
		notification_manager_->authority_ = authority_type;
		config_manager_->authority_ = authority_type;
		Refresh();
	}

	void CAdminDataManager::Refresh() {
		config_manager_->LoadConfigFile(false);
		notification_manager_->LoadNotifications(false);
	}
	
	bool CAdminDataManager::CheckInfoFields() const {
		if (!f_info_) return false;

		const auto& db = f_info_->json_converted->operator[](JFields::DB);
		if (!(
			db.contains(JFields::HOST) ||
			db.contains(JFields::PASSWORD) ||
			db.contains(JFields::USER)
			)) {
			LOG_WARN("? has missing fields", JFields::DB);
			return false;
		}

		const auto& bridge = f_info_->json_converted->operator[](JFields::SERVER_BRIDGE);

		if (!(
			bridge.contains(JFields::HOST) ||
			bridge.contains(JFields::PORT_TCP)
			)) {
			LOG_WARN("? has missing fields in settings", JFields::SERVER_BRIDGE);
			return false;
		}

		return true;
	}

	void CAdminDataManager::LoadInfoFile() {
		if (!f_info_) /*late init*/ {
			f_info_ = std::make_unique<TJsonFile>(
				JFileNames::BASE_FOLDER + std::string(JFileNames::F_INFO),
				std::vector<std::string>{ consts::JFields::DB}
			);
		}

		//IMPORTANT: adres tuttuklari icin member json icerigini yenilemeden once invalid et.
		if (db_info_) {
			db_info_.reset();
		}
		if (bridge_info_) {
			bridge_info_.reset();
		}
		
		if (!jsonLoaderInstance.LoadFile(*f_info_)) {
			LOG_FATAL("? could not be loaded", JFileNames::F_INFO);
			return;
		}

		if (!CheckInfoFields()) return;

		const type_json& dbInfo = f_info_->json_converted->operator[](JFields::DB);

		db_info_ = std::make_unique<TDBInfo>(
			dbInfo[JFields::HOST].get_ref<const std::string&>(),
			dbInfo[JFields::USER].get_ref<const std::string&>(),
			dbInfo[JFields::PASSWORD].get_ref<const std::string&>()
		);

		const type_json& bridgeInfo = f_info_->json_converted->operator[](JFields::SERVER_BRIDGE);

		bridge_info_ = std::make_unique<TBridgeInfo>(
			bridgeInfo[JFields::HOST].get_ref<const std::string&>(),
			bridgeInfo[JFields::PORT_TCP].get<uint16_t>()
		);
	}

	bool CAdminDataManager::WriteDBSettings(MSDBInfo& packet) const {
		LOG_TRACE("Writing.");
		if (!db_info_) return false;

		network::TMP_BUFFER::str_copy(packet.host, sizeof(packet.host), db_info_->host.c_str());
		network::TMP_BUFFER::str_copy(packet.user, sizeof(packet.user), db_info_->user.c_str());
		network::TMP_BUFFER::str_copy(packet.password, sizeof(packet.password), db_info_->password.c_str());

		LOG_TRACE("Successfully wrote.");


		return true;
	}

	const char* CAdminDataManager::GetBridgeHost() const {
		if (!bridge_info_) return "";
		return bridge_info_->host.c_str();
	}

	uint16_t CAdminDataManager::GetBridgePort() const {
		if (!bridge_info_) return 0;
		return bridge_info_->port;
	}
}
