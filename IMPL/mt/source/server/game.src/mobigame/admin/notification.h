#pragma once
#if __MOBICORE__
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <memory>

#include <Utility/flag_wrapper.h>

struct TJsonFile;

namespace mobi_game {
	class GameClientBase;
	class CAdminDataManager;

	struct TNotification{
		const std::string& title;
		const std::string& subtitle;
		const std::string& details;
		const std::string& admin_name;

		TNotification() = delete;
		TNotification(const TNotification& other) = delete;
		TNotification operator=(const TNotification& other) = delete;

		//only move.

		explicit TNotification(
			const std::string& _title, 
			const std::string& _subtitle,
			const std::string& _details,
			const std::string& _admin_name
		) : title(_title), subtitle(_subtitle), details(_details), admin_name(_admin_name) {}
	};

	class CNotificationManager final {
			friend class CAdminDataManager;
			GameClientBase* client_ = nullptr;
			NUtility::CFlagWrapper authority_ = 0;
			std::unique_ptr<TJsonFile> f_notification_;
			std::chrono::system_clock::time_point last_checktime_;
			std::vector<std::unique_ptr<TNotification>> notifications_;
		public:
			CNotificationManager(GameClientBase* client);
			~CNotificationManager() noexcept;
		public:
			void LoadNotifications(bool bNeedTimeCheck = true);
			void doWork();
	};

}
#endif