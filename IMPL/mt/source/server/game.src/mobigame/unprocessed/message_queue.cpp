#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif
#include "message_queue.h"

#include <chrono>

#include <Singletons/log_manager.h>

#include <Network/buffer.h>
#include "constants/packets.h"

#include "constants/consts.h"
#include "client/client_base.h"

using namespace network;

namespace mobi_game {
    using namespace consts;
    static inline uint32_t s_message_container_id = 0;

    uint32_t CMessageHelper::message_get_container_id() const noexcept {
        return s_message_container_id;
    }

    void CMessageHelper::message_add(uint32_t sender_pid, const std::string& nameTo, const std::string& message, uint16_t code_page) noexcept {
        if(message_queue_.size() >= MAX_MESSAGE_COUNT){
            message_queue_.pop_front();
        }
        
        message_queue_.emplace_back(std::make_unique<TMSMessageContainer>(sender_pid, nameTo, message, s_message_container_id++, code_page));
    }

    bool CMessageHelper::message_process(uint32_t container_id) {
        bool ret = false;
        auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
            [container_id](auto& msg) { return msg->container_id == container_id; });
        
        if (it != message_queue_.end()) {
            ret = (*it)->process(client_);
            if (it != message_queue_.end() - 1) {
                *it = std::move(message_queue_.back());
            }
            message_queue_.pop_back();
        }
        return ret;
    }

    void CMessageHelper::message_remove(uint32_t container_id) {
        auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
            [container_id](auto& msg) { return msg->container_id == container_id; });
        
        if (it != message_queue_.end()) {
            if (it != message_queue_.end() - 1) {
                *it = std::move(message_queue_.back());
            }
            message_queue_.pop_back();
        }
    }

    void CMessageHelper::message_cleanup_expired() noexcept {
        auto now = std::chrono::steady_clock::now();
        if (now - last_cleanup_time_ < std::chrono::seconds(CLEANUP_INTERVAL)) return;
        
        last_cleanup_time_ = now;
        size_t removed_count = 0;
        auto it = std::remove_if(message_queue_.begin(), message_queue_.end(),
            [this](auto& msg) { return msg->is_expired(std::chrono::seconds(MESSAGE_TIMEOUT)); });
        
        removed_count = std::distance(it, message_queue_.end());
        message_queue_.erase(it, message_queue_.end());
        LOG_TRACE("Removed ? expired message packets", removed_count);
    }

    uint32_t CMessageHelper::message_get_sender_pid(uint32_t container_id) const noexcept {
        auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
            [container_id](auto& msg) { return msg->container_id == container_id; });
        if (it != message_queue_.end()) {
            return (*it)->get_sender_pid();
        }
        return 0;
    }

    const char* CMessageHelper::message_get_receiver_name(uint32_t container_id) const noexcept {
        auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
            [container_id](auto& msg) { return msg->container_id == container_id; });
        return it != message_queue_.end() ? (*it)->get_receiver_name() : "";
    }

    const TMSMessageContainer* CMessageHelper::message_get_container(uint32_t container_id) const noexcept {
        auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
            [container_id](auto& msg) { return msg->container_id == container_id; });
        return it != message_queue_.end() ? it->get() : nullptr;
    }
}
#endif