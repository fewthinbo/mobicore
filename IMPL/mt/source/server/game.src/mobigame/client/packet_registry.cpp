#include "mobi_base.h"

#include <vector>

#include <Singletons/log_manager.h>
#include <Network/buffer.h>

#include "constants/packets.h"
#include "client_core.h"
#include "admin/admin_data_manager.h"

using namespace network;

namespace mobi_game {
	void GameClientBase::RegisterPackets() noexcept {
		//dynamic packets
#if __MOBI_PACKET_ENCRYPTION__
        client_->packet_register_dynamic(HEADER_SM_KEY_EXCHANGE, sizeof(TKeyExchange), offsetof(TKeyExchange, size));  
#endif
		client_->packet_register_dynamic(HEADER_SM_MESSAGE, sizeof(SMMessage), offsetof(SMMessage, size));
		client_->packet_register_dynamic(HEADER_SM_FORWARD, sizeof(SMForward), offsetof(SMForward, size));
		client_->packet_register_dynamic(HEADER_SM_VALIDATE_LOGIN, sizeof(SMValidateMobileLogin), offsetof(SMValidateMobileLogin, size));

		//static packets
		client_->packet_register_fixed(HEADER_SM_CORE_AUTHORITY, sizeof(SMCoreAuthority));
		client_->packet_register_fixed(HEADER_SM_DB_INFO, sizeof(uint8_t));
		client_->packet_register_fixed(HEADER_SM_USER_CHECK, sizeof(SMUserCheck));
		client_->packet_register_fixed(HEADER_SM_LOGIN, sizeof(SMLogin));
		client_->packet_register_fixed(HEADER_SM_LOGOUT, sizeof(SMLogout));
		client_->packet_register_fixed(HEADER_SM_CACHE_STATUS, sizeof(SMCacheStatus));
#if !__MT_DB_INFO__
		client_->packet_register_fixed(HEADER_SM_GET_CACHE, sizeof(SMGetCache));
#endif
	}

	bool GameClientBase::HandlePacket(THEADER header, const std::vector<uint8_t>& data) {
		bool result = false;
        bool any_match = true;
		
        switch (header){
            case HEADER_SM_KEY_EXCHANGE:{
				result = HandleKeyExchange(data);
                break;
            }
            case HEADER_SM_CORE_AUTHORITY:{
                auto* sm = reinterpret_cast<const SMCoreAuthority*>(data.data());
                admin_data_manager_->SetAuthority(sm->authority_type);
                result = true;
                break;
            }
			case HEADER_SM_MESSAGE: {
				result = HandleMobilePm(data);
				break;
			}
			case HEADER_SM_DB_INFO: {
				result = HandleDbInfo(data);
				break;
			}
			case HEADER_SM_USER_CHECK: {
				result = HandleUserCheck(data);
				break;
			}
			case HEADER_SM_LOGIN: {
				result = HandleMobileLogin(data);
				break;
			}
			case HEADER_SM_LOGOUT: {
				result = HandleMobileLogout(data);
				break;
			}
			case HEADER_SM_CACHE_STATUS: {
				result = HandleCacheStatus(data);
				break;
			}
			case HEADER_SM_FORWARD: {
				result = HandleForwardPacket(data);
				break;
			}
			case HEADER_SM_VALIDATE_LOGIN: {
				result = HandleValidateLogin(data);
				break;
			}
#if !__MT_DB_INFO__
			case HEADER_SM_GET_CACHE: {
				result = HandleGetCache(data);
				break;
			}
#endif
            default:
                any_match = false;
                result = false;
                break;
        }

        if (!any_match) {
			LOG_ERR("Handler(h:?) not exists", header);
			return false;
        }

		LOG_TRACE("Handler(h:?) result(?)", header, result ? "success" : "failed");

		return result;
	}
}