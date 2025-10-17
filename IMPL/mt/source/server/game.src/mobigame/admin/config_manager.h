#pragma once
#if __MOBICORE__
#include <string>
#include <cstdint>
#include <memory>
#include <chrono>

#include <Utility/flag_wrapper.h>

struct TJsonFile;
namespace mobi_game {
	class GameClientBase;
	class CAdminDataManager;

	class CConfigManager final {
		friend class CAdminDataManager;
		GameClientBase* client_ = nullptr;
		NUtility::CFlagWrapper authority_ = 0;
		static constexpr auto kPacketIdSize = sizeof(uint8_t);
	public:
		CConfigManager(GameClientBase* client);
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
#endif