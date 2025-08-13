#include "raw_client.h"

#include <Singletons/log_manager.h>

#include "packet_descriptor.h"
#include "Network/common.h"
#include "Network/buffer.h"

#include "constants.h"
namespace network {
    using namespace consts;

    bool NetworkClientImpl::packet_register_fixed(THEADER header, size_t total_size) noexcept {
        if (packets_.find(header) != packets_.end()) return false;
    
        auto pair = packets_.try_emplace(header, TPacketDescriptor(header, total_size));
        return pair.second;
    }
    
    bool NetworkClientImpl::packet_register_dynamic(THEADER header, size_t header_size, size_t size_offset) noexcept {
        if (packets_.find(header) != packets_.end()) return false;
        
        auto pair = packets_.try_emplace(header, TPacketDescriptor(header, header_size, size_offset));
        return pair.second;
    }
    
    const TPacketDescriptor* NetworkClientImpl::packet_get_descriptor(THEADER header) const noexcept {
        auto it = packets_.find(header);
        return it != packets_.end() ? &it->second : nullptr;
    }

    void NetworkClientImpl::packet_process_decrypted(std::shared_ptr<std::vector<uint8_t>> data) {
        if (!data) {
            LOG_WARN("data is nullptr.");
            return;
        }
    
        if (data->empty()) {
            LOG_WARN("data is empty.");
            return;
        }
    
        THEADER header = (*data)[0];
    
        if (!IsValidHeader(header)) {
            LOG_TRACE("Header(?) is not valid to process");
            return;
        }
    
        const auto* desc = packet_get_descriptor(header);
        if (!desc) {
            session_handle_error("Descriptor not exist.");
            return;
        }
    
        packet_handle(header, data);
    }
    
    void NetworkClientImpl::packet_process(THEADER header) {
        if (!IsValidHeader(header)) {
            LOG_TRACE("Header(?) is not valid to process");
            return;
        }
    
        const TPacketDescriptor* desc = packet_get_descriptor(header);
        if (!desc) {
            LOG_TRACE("Descriptor of header(?) not exist.", header);
            return;
        }
    
        std::shared_ptr<std::vector<uint8_t>> v_packet = std::make_shared<std::vector<uint8_t>>();
        v_packet->resize(desc->fixed_size); // sabit kismi almak uzere boyutu arttir.
        (*v_packet)[0] = header;
    
        //struct boyutu - header kadar veri okunur. Header'i okumak session'un gorevidir. Gerisini packet manager halleder.
        //Okunan tum veriler v_packet icerisinde otomatik siralanir.
        boost::asio::async_read(session_get_socket(),
            boost::asio::buffer(v_packet->data() + HEADER_SIZE, desc->fixed_size - HEADER_SIZE),
            [this, v_packet, desc, header](error_code ec, std::size_t length) {
                if (!IsConnected()) {
                    LOG_TRACE("Session is not active to read header body(?).", header);
                    return;
                }
    
                LOG_TRACE("body of header(?) reading", header);

                if (ec) {
                    session_handle_error("read fixed data", EActivityState::READ, ec);
                    return;
                }
    
                if (!desc) /*islem asenkron old icin desc'in varligini kontrol etmek zorundayiz veya make_shared yapilabilirdi.*/ {
                    session_handle_error("descriptor is missing", EActivityState::READ, ec);
                    return;
                }
    
                if (!desc->is_dynamic) /*if packet is not dynamic */ {
                    packet_handle(header, v_packet);
                    return;
                }
    
                TSIZE dynamic_size = 0;
                std::memcpy(&dynamic_size, v_packet->data() + desc->size_offset, SIZE_SIZE);
    
                v_packet->resize(desc->fixed_size + dynamic_size); // Dinamik icerigi de almak uzere boyutu arttir.
    
                boost::asio::async_read(session_get_socket(),
                    boost::asio::buffer(v_packet->data() + desc->fixed_size, dynamic_size),
                    [this, v_packet, desc, header](error_code ec, std::size_t length) {
                        if (!IsConnected()) {
                            session_handle_error("Session is not active to read.");
                            return;
                        }
    
                        if (ec) {
                            session_handle_error("read dynamic data", EActivityState::READ, ec);
                            return;
                        }
    
                        packet_handle(header, v_packet);
                });
            });
    }
    
    void NetworkClientImpl::packet_handle(uint32_t header, std::shared_ptr<std::vector<uint8_t>> v_data) {
        if (!v_data) return;

        const auto& data = *v_data;

        //derived method
        if (!OnDataReceived(header, data)){
            session_handle_error(loggerInstance.WriteBuf("packet handle(h:?) failed", header).c_str(), EActivityState::READ);
            return;
        }
    
        PostTask([this]() {
    #ifdef _MOBI_PACKET_ENCRYPTION
            if (session_is_crypted()) {
    #ifdef _DEBUG
                LOG_TRACE("Passing to encrypted reading manually.");
    #endif
                session_read_encrypted();
                return;
            }
    #endif
    #ifdef _DEBUG
            LOG_TRACE("Passing to reading manually.");
    #endif
            session_read();
        });
    }
}