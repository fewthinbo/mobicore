#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif
#include "config_manager.h"

#include <vector>

#include <Singletons/json_loader.h>
#include <Singletons/log_manager.h>
#include <Network/buffer.h>

#include "mobi_client.h"

#include "constants/packets.h"
#include "constants/consts.h"
#include "client/client_base.h"

using namespace network;
namespace mobi_game {
	using namespace consts;

	CConfigManager::CConfigManager(GameClientBase* client)
		: client_(client) {}

	CConfigManager::~CConfigManager() noexcept = default;

	bool CConfigManager::CheckSettingsFields() const {
		if (!f_settings_) return false;

		const auto& settings = f_settings_->json_converted->operator[](JFields::SETTINGS);
		if (!(
			settings.contains(JFields::MOBILE) ||
			settings.contains(JFields::DISABLED_PACKETS)
			)) {
			LOG_WARN("? has missing fields", JFields::SETTINGS);
			return false;
		}

		const auto& mobile = settings[JFields::MOBILE];
		const auto& dp_field = settings[JFields::DISABLED_PACKETS];

		if (!(
			mobile.contains(JFields::SECURITY) ||
			mobile.contains(JFields::ONLINE_COUNTER) ||
			mobile.contains(JFields::MAINTENANCE_MODE)
			)) {
			LOG_WARN("? has missing fields in settings", JFields::MOBILE);
			return false;
		}

		if (!(
			dp_field.contains(JFields::TO_MOBILE) ||
			dp_field.contains(JFields::TO_MT)
			)) {
			LOG_WARN("? has missing fields in settings", JFields::DISABLED_PACKETS);
			return false;
		}
		return true;
	}

	void CConfigManager::LoadConfigFile(bool bNeedTimeCheck) {
		if (!authority_.HasFlag(EAuthorityType::CONFIG_MANAGER)) return;
		if (!client_) return;

		auto now = std::chrono::system_clock::now();
		if (bNeedTimeCheck && std::chrono::duration_cast<std::chrono::seconds>(now - last_checktime_).count() < READ_FILE_INTERVAL) return;
		last_checktime_ = now; //yine de zamani kaydet

		if (!f_settings_) {
			f_settings_ = std::make_unique<TJsonFile>(
				JFileNames::BASE_FOLDER + std::string(JFileNames::F_SETTINGS),
				std::vector<std::string>{ consts::JFields::SETTINGS }
			);
		}

		if (!jsonLoaderInstance.LoadFile(*f_settings_)) {
			LOG_FATAL("? could not be loaded", JFileNames::F_SETTINGS);
			return;
		}

		if (!CheckSettingsFields()) return;
		
		const auto& settings = f_settings_->json_converted->operator[](JFields::SETTINGS);

		const auto& dp_to_mt = settings[JFields::DISABLED_PACKETS][JFields::TO_MT];
		const auto& dp_to_mobile = settings[JFields::DISABLED_PACKETS][JFields::TO_MOBILE];

		const auto& mobile_security = settings[JFields::MOBILE][JFields::SECURITY];
		const auto& mobile_online_counter = settings[JFields::MOBILE][JFields::ONLINE_COUNTER];
		const auto& mobile_maintenance_mode = settings[JFields::MOBILE][JFields::MAINTENANCE_MODE];

		//Performans bilgileri sunucuya gonderilir. Sunucu gerekirse kendini yeniden ayarlar. Rate limits vb.
		MSConfig config_packet{};
		config_packet.header = HEADER_MS_CONFIG;

		//adjusting size variables
		const auto& ip_blocklist = mobile_security[JFields::IP_BLOCKLIST];
		size_t total_str_len = 0;
		for (const auto& ip : ip_blocklist) {
			total_str_len += ip.get_ref<const std::string&>().size() + 1 /*null terminator*/;
		}
		config_packet.security_size = total_str_len;

		config_packet.dp_to_mt_size = dp_to_mt.size() * kPacketIdSize;
		config_packet.dp_to_mobile_size = dp_to_mobile.size() * kPacketIdSize;

		const auto& maintenance_msg = mobile_maintenance_mode[JFields::MESSAGE];

		config_packet.maintenance_notification_size = maintenance_msg.size() + 1;

		config_packet.size = sizeof(MSConfig) +
			sizeof(MSConfigStatic) +
			config_packet.security_size +
			config_packet.dp_to_mt_size +
			config_packet.dp_to_mobile_size +
			config_packet.maintenance_notification_size;
		//end of size variables

		//adjust static part of the security packet
		MSConfigStatic static_part{};
		static_part.max_active_sessions = mobile_security[JFields::MAX_ACTIVE_SESSIONS].get<uint32_t>();

		static_part.max_missed_heartbeats = mobile_security[JFields::MAX_MISSED_HEARTBEATS].get<uint8_t>();

		const auto& login_policy = mobile_security[JFields::LOGIN_POLICY];
		static_part.max_login_attempts = login_policy[JFields::MAX_ATTEMPTS].get<uint32_t>();
		static_part.login_block_duration_minutes = login_policy[JFields::BLOCK_DURATION_MINUTES].get<uint32_t>();

		const auto& rate_limit = mobile_security[JFields::RATE_LIMIT];
		static_part.rate_limit_per_second = rate_limit[JFields::PER_SECOND].get<uint32_t>();
		static_part.rate_limit_max_overflows = rate_limit[JFields::MAX_OVERFLOWS].get<uint32_t>();
		static_part.rate_limit_block_duration_minutes = rate_limit[JFields::BLOCK_DURATION_MINUTES].get<uint32_t>();

		static_part.online_counter_enabled = mobile_online_counter[JFields::ENABLED].get<bool>();
#if __BUILD_FOR_GAME__
		auto counter_refresh_interval = mobile_online_counter[JFields::REFRESH_INTERVAL_SECONDS].get<uint16_t>();
		mobileInstance.setOnlineRefreshInterval(std::chrono::seconds(counter_refresh_interval));
#endif

		static_part.maintenance_mode_enabled = mobile_maintenance_mode[JFields::ENABLED].get<bool>();
		static_part.maintenance_est_duration = mobile_maintenance_mode[JFields::DURATION].get<uint16_t>();
		//end of static part of the security packet

		TMP_BUFFER buf(config_packet.size);
		buf.write(&config_packet, sizeof(MSConfig));
		buf.write(&static_part, sizeof(MSConfigStatic));

		for (const auto& ip : ip_blocklist) {
			const auto& ip_str = ip.get_ref<const std::string&>();
			buf.write(ip_str.data(), ip_str.size() + 1);
		}

		for (const auto& js : dp_to_mt) {
			const auto packet_id = js.get<int>();
			buf.write(&packet_id, kPacketIdSize);
		}
		for (const auto& js : dp_to_mobile) {
			const auto packet_id = js.get<int>();
			buf.write(&packet_id, kPacketIdSize);
		}

		const auto& maintenance_message = maintenance_msg.get_ref<const std::string&>();
		buf.write(maintenance_message.data(), maintenance_message.size() + 1);

		LOG_INFO("Config packet sent to client(size: ? bytes)", config_packet.size);

		client_->Send(buf.get());
	}

	void CConfigManager::doWork() {
		LoadConfigFile();
	}
}
#endif