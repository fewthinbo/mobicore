#include "admin_data_manager.h"

#include <Singletons/json_loader.h>

#include "constants/consts.h"
#include "client/client_core.h"

#include "notification.h"
#include "config_manager.h"
#include "queries.h"

#ifdef PLATFORM_WINDOWS
#include "../Test/Console/DatabaseManager.h"
#endif
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
#ifdef ENABLE_MT_DB_INFO
		if (db_info_) {
			db_info_.reset();
		}
#endif
		if (bridge_info_) {
			bridge_info_.reset();
		}
		
		if (!jsonLoaderInstance.LoadFile(*f_info_)) {
			LOG_FATAL("? could not be loaded", JFileNames::F_INFO);
			return;
		}

		if (!CheckInfoFields()) return;

#ifdef ENABLE_MT_DB_INFO
		const type_json& dbInfo = f_info_->json_converted->operator[](JFields::DB);

		db_info_ = std::make_unique<TDBInfo>(
			dbInfo[JFields::HOST].get_ref<const std::string&>(),
			dbInfo[JFields::USER].get_ref<const std::string&>(),
			dbInfo[JFields::PASSWORD].get_ref<const std::string&>()
		);
#endif

		const type_json& bridgeInfo = f_info_->json_converted->operator[](JFields::SERVER_BRIDGE);

		bridge_info_ = std::make_unique<TBridgeInfo>(
			bridgeInfo[JFields::HOST].get_ref<const std::string&>(),
			bridgeInfo[JFields::PORT_TCP].get<uint16_t>()
		);
	}

	std::vector<uint8_t> CAdminDataManager::GetDBCache() {
		LOG_TRACE("Writing.");
		TMP_BUFFER buf(sizeof(MSDBInfo));
#ifdef ENABLE_MT_DB_INFO
		MSDBInfo packet{};
		network::TMP_BUFFER::str_copy(packet.host, sizeof(packet.host), db_info_->host.c_str());
		network::TMP_BUFFER::str_copy(packet.user, sizeof(packet.user), db_info_->user.c_str());
		network::TMP_BUFFER::str_copy(packet.password, sizeof(packet.password), db_info_->password.c_str());
		buf.write(&packet, sizeof(MSDBInfo));
		LOG_TRACE("Successfully wrote.");
#elif defined(MOBICORE)
		MSDBInfo packet{};
		packet.size = sizeof(MSDBInfo);

		using namespace utils;
		auto* ret_acc = GetResultOfQuery(query::ACCOUNT_WITH_EMPIRE);
		auto* ret_player = GetResultOfQuery(query::PLAYER);
		auto* ret_gld = GetResultOfQuery(query::GUILD);
		auto* ret_gld_member = GetResultOfQuery(query::GUILD_MEMBER);
		auto* ret_messenger = GetResultOfQuery(query::MESSENGER_LIST);

		auto b_acc = ret_acc && ret_acc->Get();
		auto b_player = ret_player && ret_player->Get()
		auto b_guild = ret_gld && ret_gld->Get();
		auto b_gld_member = ret_gld_member && ret_gld_member->Get();
		auto b_messenger = ret_messenger && ret_messenger->Get();

		//statik kisma ait size bilgilerini doldur.
		auto acc_size = sizeof(MSCache::Account);
		auto player_size = sizeof(MSCache::Player);
		auto gld_size = sizeof(MSCache::Guild);
		auto gld_member_size = sizeof(MSCache::GuildMember);
		auto messenger_size = sizeof(MSCache::MessengerList);
		if (b_acc) {
			auto cache_size = acc_size * ret_acc->Get()->uiNumRows;
			packet.size += cache_size;
			packet.cache_sizes[static_cast<uint8_t>(ECacheType::ACCOUNT)] = cache_size;
		}
		if (b_player) {
			auto cache_size = player_size * ret_player->Get()->uiNumRows;
			packet.size += cache_size;
			packet.cache_sizes[static_cast<uint8_t>(ECacheType::PLAYER)] = cache_size;
		}
		if (b_guild) {
			auto cache_size = gld_size * ret_gld->Get()->uiNumRows;
			packet.size += cache_size;
			packet.cache_sizes[static_cast<uint8_t>(ECacheType::GUILD)] = cache_size;
		}
		if (b_gld_member) {
			auto cache_size = gld_member_size * ret_gld_member->Get()->uiNumRows;
			packet.size += cache_size;
			packet.cache_sizes[static_cast<uint8_t>(ECacheType::GUILD_MEMBER)] = cache_size;
		}
		if (b_messenger) {
			auto cache_size = messenger_size * ret_messenger->Get()->uiNumRows;
			packet.size += cache_size;
			packet.cache_sizes[static_cast<uint8_t>(ECacheType::MESSENGER_LIST)] = cache_size;
		}

		//dinamik bolumleri yaz: enum sirasina bagli 
		if (b_acc) {
			if (SQLResult* sql_res = ret_acc->Get()) {
				for (auto i = 0; i < sql_res->uiNumRows; ++i) {
					MYSQL_ROW row = mysql_fetch_row(sql_res->pSQLResult);
					MSCache::Account data{
						//login, id, empire, email, authority
						row[1], std::stoi(row[0]), std::stoi(row[2]), row[3], std::stoi(row[4])
					};
					buf.write(&data, acc_size);
				}
			}
		}
		if (b_player) {
			if (SQLResult* sql_res = ret_player->Get()) {
				for (auto i = 0; i < sql_res->uiNumRows; ++i) {
					MYSQL_ROW row = mysql_fetch_row(sql_res->pSQLResult);
					MSCache::Player data{
						std::stoi(row[0]), std::stoi(row[1]), row[2], std::stoi(row[3]), std::stoi(row[4]), //mapidx
						std::stoi(row[5]), std::stoi(row[6]), row[7]/*lastplay*/, std::stoi(row[8]), std::stoi(row[9])
					};
					buf.write(&data, player_size);
				}
			}
		}
		if (b_guild) {
			if (SQLResult* sql_res = ret_gld->Get()) {
				for (auto i = 0; i < sql_res->uiNumRows; ++i) {
					MYSQL_ROW row = mysql_fetch_row(sql_res->pSQLResult);
					MSCache::Guild data{
						row[1], std::stoi(row[0]), std::stoi(row[1]), std::stoi(row[2]),
						std::stoi(row[3]), std::stoi(row[4]), std::stoi(row[5]),
						std::stoi(row[6])
					};
					buf.write(&data, gld_size);
				}
			}
		}
		if (b_gld_member) {
			if (SQLResult* sql_res = ret_gld_member->Get()) {
				for (auto i = 0; i < sql_res->uiNumRows; ++i) {
					MYSQL_ROW row = mysql_fetch_row(sql_res->pSQLResult);
					MSCache::GuildMember data{
						std::stoi(row[0]), std::stoi(row[1])
					};
					buf.write(&data, gld_member_size);
				}
			}
		}
		if (b_messenger) {
			if (SQLResult* sql_res = ret_messenger->Get()) {
				for (auto i = 0; i < sql_res->uiNumRows; ++i) {
					MYSQL_ROW row = mysql_fetch_row(sql_res->pSQLResult);
					MSCache::MessengerList data{
						row[0], row[1]
					};
					buf.write(&data, messenger_size);
				}
			}
		}
#elif (PLATFORM_WINDOWS) //ifndef ENABLE_MT_DB_INFO and ifndef mobicore: send random test db data
		DatabaseManager dbManager;
		dbManager.initialize();
		return dbManager.GetDbBuffer();
#endif
		LOG_TRACE("DBCache packet ready. Total size(bytes: ?)", buf.get().size());
		return buf.get();
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
