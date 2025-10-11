#include "raw_client.h"

#include <Singletons/log_manager.h>

#include "Network/crypto_helper.h"
#include "Network/key_exchange.h"
#include "Network/error_handler.h"

#include "constants.h"

namespace network {
	using namespace consts;
	bool NetworkClientImpl::session_start() {
		if (IsConnected()) {
			session_handle_error("Session already active");
			return false;
		}

		connect_state_ = EConnectState::CONNECTED; // Don't touch this order
		ClearTimers();

		LOG_INFO("Session started");

		PostTask([this]() {
#if __MOBI_PACKET_ENCRYPTION__
			if (crypto_ && is_encrypted_) {
				session_read_encrypted();
				return;
			}
#endif
			session_read();
		});

		return true;
	}

	void NetworkClientImpl::session_stop() noexcept {
		if (!IsConnected()) return;
		LOG_INFO("Session stopping");

		try
		{
#if __MOBI_PACKET_ENCRYPTION__
			is_encrypted_ = false;
			crypto_.reset();
#endif
			write_queue_.clear();

			//Close socket begin
			if (socket_.native_handle() != INVALID_SOCKET) {
				LOG_INFO("Closing client socket...");

				if (!socket_.is_open()) {
					LOG_INFO("Socket is not open to close.");
					return;
				}

				error_code socket_ec;
				socket_.shutdown(tcp::socket::shutdown_both, socket_ec);
				if (socket_ec) {
					LOG_WARN("Socket shutdown failed: ?", socket_ec.message());
				}
				socket_ec.clear();

				socket_.close(socket_ec);
				if (socket_ec) {
					LOG_WARN("Socket close failed: ?", socket_ec.message());
				}
				else {
					LOG_INFO("Client socket closed successfully");
				}
			}
			else {
				LOG_INFO("Socket is already invalid.");
			}

			//end of close socket progress

			LOG_INFO("Session stopped successfully.");
		}
		catch (const std::exception& e)
		{
			LOG_ERR("Exception caught: ?", e.what());
		}
	}

	void NetworkClientImpl::session_read() {
		if (!IsConnected()) return;

		// Once paket registry'den alinacak bilgilere gore kac byte okunacagini belirle
		auto read_buffer = std::make_shared<std::vector<uint8_t>>(HEADER_SIZE);

		// Ilk olarak header'i oku
		boost::asio::async_read(socket_,
			boost::asio::buffer(*read_buffer, HEADER_SIZE),
			[this, read_buffer](const error_code& ec, size_t bytes_read) {
				if (ec) {
					// Tek bir fonksiyon cagrisi ile hem hata isleniyor hem de gerekirse okuma devam ediyor
					session_handle_error("read header", EActivityState::READ, ec);
					return;
				}

				if (!IsConnected()) {
					LOG_TRACE("Session is not connected, skipping.");
					return;
				}

				// Header'i isle
				THEADER header = (*read_buffer)[0];

				packet_process(header);
			});
	}

#if __MOBI_PACKET_ENCRYPTION__
	void NetworkClientImpl::session_read_encrypted() {
		if (!IsConnected()) return;

		auto size_buffer = std::make_shared<std::vector<uint8_t>>(SIZE_SIZE);

		boost::asio::async_read(
			socket_,
			boost::asio::buffer(*size_buffer, SIZE_SIZE),
			[this, size_buffer](const error_code& ec, size_t bytes_read) {
				if (ec) {
					// Tek bir fonksiyon cagrisi ile hem hata isleniyor hem de gerekirse okuma devam ediyor
					session_handle_error("read size", EActivityState::READ, ec);
					return;
				}
				TSIZE packet_size = 0;
				std::memcpy(&packet_size, size_buffer->data(), SIZE_SIZE);

				LOG_TRACE("size_info(?) of encrypted packet received", packet_size);

				if (!IsConnected()) {
					LOG_TRACE("Session is not connected, skipping.");
					return;
				}

				// Veri boyutu kontrolü yap
				if (packet_size <= 0 || packet_size >= MAX_SIZE_PACKET) {
					session_handle_error("invalid packet size", EActivityState::READ);
					return;
				}

				// Veriyi oku
				auto data_buffer = std::make_shared<std::vector<uint8_t>>(packet_size);
				boost::asio::async_read(
					socket_,
					boost::asio::buffer(*data_buffer, packet_size),
					[this, data_buffer](const error_code& ec, size_t bytes_read) {
						if (ec) {
							// Tek bir fonksiyon çağrısı ile hem hata işleniyor hem de gerekirse okuma devam ediyor
							session_handle_error("read encrypted data", EActivityState::READ, ec);
							return;
						}

#if _DEBUG
						LOG_TRACE("Encrypted data(?) received.", std::string(reinterpret_cast<const char*>(data_buffer->data()), data_buffer->size()));
#endif
						if (!IsConnected()) {
							LOG_TRACE("Session is not connected, skipping.");
							return;
						}

						// Sifrelenmis veriyi coz
						if (!crypto_->decrypt(*data_buffer)) {
							session_handle_error("decryption failed", EActivityState::READ);
							return;
						}

#if _DEBUG
						THEADER header = (*data_buffer)[0];
						LOG_TRACE("Decrypted successfully, header (?)", header);
#endif
						// Paketi isle
						packet_process_decrypted(data_buffer);
					});
			});
	}
#endif

	const std::vector<uint8_t>& NetworkClientImpl::session_get_public_key() {
		if (!key_exchange_) {
			key_exchange_ = std::make_unique<KeyExchange>();
		}
		return key_exchange_->getPublicKey();
	}

	void NetworkClientImpl::session_write() {
		if (write_queue_.empty()) {
#if _DEBUG
			LOG_INFO("Write queue is empty, skipping write");
#endif
			return;
		}

		// Veriyi shared_ptr icinde olustur
		auto current_packet = std::make_shared<std::vector<uint8_t>>(
			std::move(write_queue_.front()));
		write_queue_.pop_front();

		if (current_packet->empty()) {
			if (!write_queue_.empty()) {
				PostTask([this]() {
					session_write();
				});
				return;
			}
			return;
		}

#if _DEBUG
		THEADER header = (*current_packet)[0];
		LOG_TRACE("Writing packet(header:?) with size(?)", header, current_packet->size());
#endif

		boost::asio::async_write(socket_,
			boost::asio::buffer(current_packet->data(), current_packet->size()),
			[this, current_packet/*life time garantisi*/, header](
				error_code ec, std::size_t length) {
					if (ec) {
						session_handle_error("write", EActivityState::WRITE, ec);
						return;
					}

					if (!IsConnected()) {
						LOG_TRACE("Session is not connected, skipping.");
						return;
					}
#if _DEBUG
					LOG_TRACE("Successfully wrote packet(header:?), size(?)", header, length);
#endif
					if (!write_queue_.empty()) {
						PostTask([this]() {
							session_write(); 
						});
					}
			});
	}

	void NetworkClientImpl::session_handle_error(const char* operation, const error_code& ec) {
		if (!error_handler_) return;
		error_handler_->handleError(operation, ec);
	}

	void NetworkClientImpl::session_handle_error(const char* operation, EActivityState activity_state, const error_code& ec) {
		if (!error_handler_) return;
		ErrorContext context = error_handler_->categorizeError(ec, operation);

		if (context.should_close_session == CloseType::NONE) {
			if (IsConnected()) {
				if (activity_state == EActivityState::READ) {
					PostTask([this]() {
#if __MOBI_PACKET_ENCRYPTION__
						if (crypto_ && is_encrypted_) {
							session_read_encrypted();
							return;
						}
#endif
						session_read();
						});
				}
				else if (activity_state == EActivityState::WRITE) {
					PostTask([this]() {
						if (!write_queue_.empty()) {
							session_write();
							return;
						}
					});
				}
				else if (activity_state == EActivityState::BOTH) {
					PostTask([this]() {
						session_write();
#if __MOBI_PACKET_ENCRYPTION__
						if (crypto_ && is_encrypted_) {
							session_read_encrypted();
							return;
						}
#endif
						session_read();
						});
				}
			}
		}
		error_handler_->handleError(operation, ec);//session'un kapatma kararini client'e birak.
	}

	bool NetworkClientImpl::session_enable_encryption(const std::vector<uint8_t>& key) {
#if __MOBI_PACKET_ENCRYPTION__
		if (!key_exchange_) {
			key_exchange_ = std::make_unique<KeyExchange>();
		}
		if (is_encrypted_) {
			LOG_INFO("Encryption already enabled");
			return true;
		}

		std::vector<uint8_t> computed_key{};
		if (!key_exchange_->computeSharedKey(key, computed_key)) {
			LOG_INFO("An error occured while compute shared key.");
			return false;
		}
		LOG_INFO("Computed key, size:?, key(?)", computed_key.size(), std::string(reinterpret_cast<const char*>(computed_key.data()), computed_key.size()));

		if (!crypto_) {
			crypto_ = std::make_unique<CryptoHelper>();
		}
		if (!crypto_->initialize(computed_key)) {
			crypto_.reset();
			return false;
		}
		LOG_INFO("Encryption initializing progress: finished successfully.");
		is_encrypted_ = true;

		OnEnabledEnc();
		return true;
#else
		return true;
#endif
	}
}