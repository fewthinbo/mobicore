#include "notification.h"

#include <vector>

#include <Singletons/json_loader.h>
#include <Network/buffer.h>

#include "constants/packets.h"
#include "constants/consts.h"
#include "client/client_core.h"

using namespace network;

namespace mobi_game {
	using namespace consts;

	CNotificationManager::~CNotificationManager() noexcept = default;

	CNotificationManager::CNotificationManager(GameNetworkClient* client)
		: client_(client) {}

	//game ve db'nin bulundugu dizinde olan admin_data.json dosyasini oku ve m_notifications'a ekle.
	void CNotificationManager::LoadNotifications(bool bNeedTimeCheck) {
		if (!authority_.HasFlag(EAuthorityType::NOTIFICATION_MANAGER)) return;
		if (!client_) return;

		auto now = std::chrono::system_clock::now();
		if (bNeedTimeCheck && std::chrono::duration_cast<std::chrono::seconds>(now - last_checktime_).count() < READ_FILE_INTERVAL) return;
		last_checktime_ = now;

		notifications_.clear(); //IMPORTANT: once verileri invalid et.

		if (!f_notification_) {
			f_notification_ = std::make_unique<TJsonFile>(
				JFileNames::BASE_FOLDER + std::string(JFileNames::F_NOTIFICATIONS),
				std::vector<std::string>{ JFields::NOTIFICATIONS /*required files*/ }
			);
		}

		if (!jsonLoaderInstance.LoadFile(*f_notification_)) {
			LOG_FATAL("? could not be loaded", JFileNames::F_NOTIFICATIONS);
			return;
		}

		const type_json& notifications = f_notification_->json_converted->operator[](JFields::NOTIFICATIONS);

		for (const auto& notification : notifications) {
			const std::string& title = jsonInstance.getStringField(notification, JFields::TITLE);
			const std::string& subtitle = jsonInstance.getStringField(notification, JFields::SUBTITLE);
			const std::string& details = jsonInstance.getStringField(notification, JFields::DETAILS);
			const std::string& admin_name = jsonInstance.getStringField(notification, JFields::ADMIN_NAME);

			notifications_.emplace_back(std::make_unique<TNotification>(title, subtitle, details, admin_name));
		}

		//Bridge tarafindan herhangi bir session'a -core'a- gonderme gorevini yapmasi icin yetki verilir.
		for (const auto& notification : notifications_) {
			if (!notification) continue;

			auto& notif = *notification;

			//her string için bir null terminator
			size_t titleSize = notif.title.size() + 1;
			size_t subtitleSize = notif.subtitle.size() + 1;
			size_t detailsSize = notif.details.size() + 1;
			size_t adminNameSize = notif.admin_name.size() + 1;

			TMP_BUFFER buf(sizeof(MSNotification) + adminNameSize + titleSize + subtitleSize + detailsSize);

			MSNotification packet{};
			packet.header = HEADER_MS_ADMIN_NOTIFICATION;
			packet.subheader = static_cast<uint8_t>(ENotificationSubHeader::ADD);

			packet.size = adminNameSize + titleSize + subtitleSize + detailsSize;

			buf.write(&packet, sizeof(MSNotification));

			buf.write(notif.admin_name.c_str(), adminNameSize);
			buf.write(notif.title.c_str(), titleSize);
			buf.write(notif.subtitle.c_str(), subtitleSize);
			buf.write(notif.details.c_str(), detailsSize);

			client_->Send(buf.get());
		}
	}

	void CNotificationManager::doWork() {
		LoadNotifications();
	}

}