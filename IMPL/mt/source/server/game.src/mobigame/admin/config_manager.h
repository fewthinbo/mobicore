#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <chrono>

#include <Utility/flag_wrapper.h>

struct TJsonFile;
namespace mobi_game {
	class GameNetworkClient;
	class CAdminDataManager;

	class CConfigManager final {
		friend class CAdminDataManager;
		GameNetworkClient* client_ = nullptr;
		NUtility::CFlagWrapper authority_ = 0;
		static constexpr auto kPacketIdSize = sizeof(uint8_t);
	public:
		CConfigManager(GameNetworkClient* client);
		~CConfigManager() noexcept;
	private:
		std::unique_ptr<TJsonFile> f_settings_;
	private:
		std::chrono::system_clock::time_point last_checktime_;
		bool CheckSettingsFields() const;
	public:
		void LoadConfigFile(bool bNeedTimeCheck = true);
		void doWork();
	};
}