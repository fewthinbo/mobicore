#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif
#include "mobi_client.h"

#include <Singletons/log_manager.h>
#include <Network/buffer.h>

#include "constants/packets.h"

#include "admin/admin_data_manager.h"
#include "client/client_base.h"

using namespace network;

namespace mobi_game {
	void MobiClient::RegisterPackets() noexcept {
		//dynamic packets
#if __MOBI_PACKET_ENCRYPTION__
        client_impl_->packet_register_dynamic(HEADER_SM_KEY_EXCHANGE, sizeof(TKeyExchange), offsetof(TKeyExchange, size));  
#endif
		client_impl_->packet_register_dynamic(HEADER_SM_MESSAGE, sizeof(SMMessage), offsetof(SMMessage, size));
		client_impl_->packet_register_dynamic(HEADER_SM_FORWARD, sizeof(SMForward), offsetof(SMForward, size));
		client_impl_->packet_register_dynamic(HEADER_SM_VALIDATE_LOGIN, sizeof(SMValidateMobileLogin), offsetof(SMValidateMobileLogin, size));
#if __OFFSHOP__
		client_impl_->packet_register_dynamic(HEADER_SM_OFFSHOP, sizeof(SMOffshop), offsetof(SMOffshop, size));
#endif

		//static packets
		client_impl_->packet_register_fixed(HEADER_SM_CORE_AUTHORITY, sizeof(SMCoreAuthority));
		client_impl_->packet_register_fixed(HEADER_SM_DB_INFO, sizeof(uint8_t));
		client_impl_->packet_register_fixed(HEADER_SM_USER_CHECK, sizeof(SMUserCheck));
		client_impl_->packet_register_fixed(HEADER_SM_LOGIN, sizeof(SMLogin));
		client_impl_->packet_register_fixed(HEADER_SM_LOGOUT, sizeof(SMLogout));
		client_impl_->packet_register_fixed(HEADER_SM_CACHE_STATUS, sizeof(SMCacheStatus));
#if !__MT_DB_INFO__
		client_impl_->packet_register_fixed(HEADER_SM_GET_CACHE, sizeof(SMGetCache));
#endif
		client_impl_->packet_register_fixed(HEADER_SM_CHARACTER, sizeof(SMModifyCharacter));
	}

	bool MobiClient::HandlePacket(THEADER header, const std::vector<uint8_t>& data) {
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
				LOG_TRACE("Login validate packet handled result(?)", result);
				break;
			}
#if !__MT_DB_INFO__
			case HEADER_SM_GET_CACHE: {
				result = HandleGetCache(data);
				break;
			}
#endif
			case HEADER_SM_CHARACTER: {
				result = HandleModifyCharacter(data);
				break;
			}
#if __OFFSHOP__
			case HEADER_SM_OFFSHOP: {
				result = HandleOffshop(data);
				break;
			}
#endif
            default:
                any_match = false;
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
#endif