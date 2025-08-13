#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <deque>
#include <vector>
#include <cstdint>
#include <atomic>

#include "external_constants.h"
#include "constants.h"

#include "Network/common.h"
#include "Network/buffer.h"

#include "packet_descriptor.h"

namespace network {
	class ErrorHandler;
	class CryptoHelper;
	class KeyExchange;
	
	// Task Counter - tracks boost::asio internal queue

	enum class EConnectState {
		CONNECTED,
		DISCONNECTED,
		CONNECTING,
	};

	//asenkron veri okuma operasyonlarinda yok olmamasi gerekiyor: lifetime'indan emin olun.
	//enable_shared_from_this veya raw ptr + manuel kontrol uygun.
	class NetworkClientImpl {
	public:
		NetworkClientImpl();
		virtual ~NetworkClientImpl() noexcept;

	protected:
		virtual void OnDisconnect() = 0;
		virtual bool OnDataReceived(THEADER header, const std::vector<uint8_t>& data) = 0;

		//sifreleme islemi aktif edildikten sonra calisir, dogrudan onConnect kullanilmaz cunku bridge server initialize olmak icin farkli veri bekler.
		virtual void OnEnabledEnc() = 0;
		virtual bool IsValidHeader(THEADER header) const { return true; };
	protected:
		std::unordered_map<THEADER, TPacketDescriptor> packets_;
		EConnectState connect_state_ = EConnectState::DISCONNECTED;
		bool io_running_ = false;
		bool is_encrypted_ = false;

		boost::asio::io_context io_context_{};
		std::unique_ptr<boost::asio::executor_work_guard<
			boost::asio::io_context::executor_type>> work_guard_;

		// Network ve baglanti
		tcp::socket socket_;
		std::unique_ptr<tcp::resolver> resolver_;

		// Timer ve yeniden baglama
		boost::asio::steady_timer reconnect_timer_;
		boost::asio::steady_timer connect_timeout_timer_;


		std::unique_ptr<ErrorHandler> error_handler_;
		std::unique_ptr<CryptoHelper> crypto_;
		std::unique_ptr<KeyExchange> key_exchange_;        
		std::deque<std::vector<uint8_t>> write_queue_;
		
		// Task Rate Limiting
		std::atomic<size_t> pending_tasks_count_{0};

		std::string last_host_ = "";
		uint16_t last_port_ = 0;
	public:
		void StartIO(bool restart = false) noexcept;
		void StopIO() noexcept;
		void DoWork();
	public:
		void Connect(const std::string& host, uint16_t port);
		void Disconnect(bool need_reconnect = true) noexcept;
		bool IsConnected() const noexcept { return connect_state_ == EConnectState::CONNECTED; }
		ESendResult Send(std::vector<uint8_t>& data, bool encrypt = true);
		
		// Debug: Get pending task count
		size_t GetPendingTaskCount() const noexcept { 
			return pending_tasks_count_.load(std::memory_order_relaxed); 
		}
	protected:
		void ClearTimers();
		void StartReconnectTimer();

		template<typename F>
		void PostTask(F&& task) {
			// Task rate limiting - prevent boost queue overflow
			if (pending_tasks_count_.load(std::memory_order_relaxed) >= consts::MAX_PENDING_TASKS) 
				return; // DROP TASK to prevent unlimited queue growth while bottleneck
			
			pending_tasks_count_.fetch_add(1, std::memory_order_relaxed);

			boost::asio::post(io_context_, [this, task = std::forward<F>(task)]() {
				// decrease counter when task is executed
				pending_tasks_count_.fetch_sub(1, std::memory_order_relaxed);
				task();
			});
		}
		
	protected:
		bool session_start();
		void session_stop() noexcept;
		void session_handle_error(const char* operation, const error_code& ec = boost::system::errc::make_error_code(
			boost::system::errc::protocol_error));
		void session_handle_error(const char* operation, EActivityState activity_state, const error_code& ec = boost::system::errc::make_error_code(
			boost::system::errc::protocol_error));
		void session_read_encrypted();
		void session_read();
		tcp::socket& session_get_socket() { return socket_; }
		void session_write();
	public:
		bool session_is_crypted() const { return crypto_ && is_encrypted_; }
		bool session_enable_encryption(const std::vector<uint8_t>& key);
		const std::vector<uint8_t>& session_get_public_key();
	protected:
		//Header'a gore descriptor kontrol eder ve gereken tum islemleri(dinamik/statik okumalarini) yapar.
		void packet_process(THEADER header);
		void packet_process_decrypted(std::shared_ptr<std::vector<uint8_t>> data);	
		void packet_handle(uint32_t header, std::shared_ptr<std::vector<uint8_t>> v_data);
		const TPacketDescriptor* packet_get_descriptor(THEADER header) const noexcept;
	public:
		bool packet_register_fixed(THEADER header, size_t total_size) noexcept;
		bool packet_register_dynamic(THEADER header, size_t header_size, size_t size_offset) noexcept;
	};

} // namespace network 