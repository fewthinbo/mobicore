#include "raw_client.h"

#include <Singletons/log_manager.h>

#include "Network/crypto_helper.h"
#include "Network/key_exchange.h"
#include "Network/error_handler.h"

namespace network {
    using namespace consts;

    NetworkClientImpl::NetworkClientImpl()
        : socket_(io_context_)
        , reconnect_timer_(io_context_)
        , connect_timeout_timer_(io_context_) {
        error_handler_ = std::make_unique<ErrorHandler>();

        error_handler_->setErrorCallback([this](const ErrorContext& context) {
            LOG_TRACE("Error in client -> ec_message(?), ec_code(?), message(?)", context.details, context.ec.value(), context.message);
            if (context.should_close_session == CloseType::NONE) return;
            Disconnect(); //context.should_close_session == CloseType::USER
        });

        // IO baslat(Client yok olana kadar durdurulmaz)
        StartIO(true);
    }

    NetworkClientImpl::~NetworkClientImpl() noexcept {
        //Disconnect(false); //movedd to derived class
        StopIO();
    }

    void NetworkClientImpl::StartIO(bool restart) noexcept {
        if (restart) {
            LOG_INFO("Restarting io context.");
            StopIO();
        }

        if (io_running_ && !io_context_.stopped()) return;

        try
        {
            // Work guard olustur
            if (!work_guard_) {
                work_guard_ = std::make_unique<boost::asio::executor_work_guard<
                    boost::asio::io_context::executor_type>>(io_context_.get_executor());
            }
            if (!resolver_) {
                resolver_ = std::make_unique<tcp::resolver>(io_context_);
            }
            // IO context'i resetle
            io_context_.restart();

            io_running_ = !io_context_.stopped(); //true;
        }
        catch (const std::exception& e)
        {
            LOG_ERR("Exception caught: ?", e.what());
        }
    }

    void NetworkClientImpl::StopIO() noexcept {
        try
        {
            if (!io_running_ && io_context_.stopped()) {
                LOG_INFO("IO Thread already stopped, skipping");
                return;
            }

            // Work guard'i temizle
            if (work_guard_) {
                work_guard_->reset();
                work_guard_.reset();
            }

            //DNS resolver'i temizle
            if (resolver_) {
                resolver_->cancel();
                resolver_.reset();
                LOG_INFO("Resolver clear.");
            }

            // IO context'i durdur
            io_context_.stop();


            io_running_ = io_context_.stopped(); //false;
            //io_context_.restart();
        }
        catch (const std::exception& e)
        {
            LOG_ERR("Exception caught: ?", e.what());
        }
    }

    void NetworkClientImpl::ClearTimers() {
        LOG_INFO("Timers resetted.");
        connect_timeout_timer_.cancel();
        reconnect_timer_.cancel();
    }

    void NetworkClientImpl::StartReconnectTimer() {
        //timer zaten baslatilmis.
        if (connect_state_ == EConnectState::CONNECTING 
            || connect_state_ == EConnectState::CONNECTED) return;

        LOG_INFO("Will attempt to reconnect...");

        LOG_INFO("Starting reconnect timer (? seconds interval)", RECONNECT_INTERVAL);

        connect_state_ = EConnectState::CONNECTING;

        // Timer'i 2 dakikaya ayarla
        reconnect_timer_.expires_after(RECONNECT_INTERVAL);

        reconnect_timer_.async_wait([this](const error_code& ec) {
            /*timer iptal edilmis: normal*/
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }
            else if (ec.value() != 0) {
                session_handle_error("critical: Reconnect timer error", ec);
            }

            Connect(last_host_, last_port_);
        });
    }

    void NetworkClientImpl::Connect(const std::string& host, uint16_t port) {
        if (IsConnected()) return;

        // ip cozumlemeyi schedule et
        LOG_INFO("Attempting to connect...");
        if (connect_state_ == EConnectState::CONNECTED) {
            LOG_INFO("Already connected, skipping.");
            return;
        }

        connect_state_ = EConnectState::CONNECTING;
        LOG_INFO("Connecting to ?:?", host, port);
        last_host_ = host;
        last_port_ = port;

        //session'a tasinmis ise veya bir sekilde invalid olduysa yeni bir socket olustur
        if (socket_.native_handle() == INVALID_SOCKET) {
            error_code socket_ec;
            socket_ = tcp::socket(io_context_);
            if (socket_.is_open()) {
                socket_.close(socket_ec);
            }
            LOG_INFO("new Socket created");
        }

        // Start the timeout timer
        connect_timeout_timer_.expires_after(CONNECT_TIMEOUT);
        connect_timeout_timer_.async_wait([this](const error_code& ec) {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }
            else if (ec.value() == 0) {
                session_handle_error("critical: connect timeout timer error", ec);
            }

            if (connect_state_ != EConnectState::CONNECTED) {
                connect_state_ = EConnectState::DISCONNECTED;
                session_handle_error("critical: connection timeout", ec);
                return;
            }
        });

        if (!resolver_) {
            LOG_ERR("Resolver doesn't exists. Creating new one");
            resolver_ = std::make_unique<tcp::resolver>(io_context_);
        }

        auto resolve_handler = [this](const error_code& ec, tcp::resolver::results_type endpoints) {
            if (ec) {
                session_handle_error("critical: resolution failed", ec);
                return;
            }

            // IPv4 endpoint bul
            auto it = std::find_if(endpoints.begin(), endpoints.end(), [](const tcp::endpoint& ep) {
                return ep.address().is_v4();
                });

            if (it == endpoints.end()) {
                //tekrar baglanmayi deneme.
                LOG_FATAL("IPV4 ENDPOINT NOT EXISTS");
                session_handle_error("no IPv4 endpoint found", boost::asio::error::address_family_not_supported);
                return;
            }

            LOG_INFO("Host successfully resolved to ?", it->endpoint().address().to_string());

            // Sadece IPv4 endpoint ile bağlantı başlat
            boost::asio::async_connect(socket_, std::vector<tcp::endpoint>{ it->endpoint() },
                [this](const error_code& ec, const tcp::endpoint& endpoint) {
                    if (ec) {
                        session_handle_error("critical: connection failed", ec);
                        return;
                    }
            
                    if (!socket_.is_open()) {
                        session_handle_error("critical: socket is not open");
                        return;
                    }
            
                    // Session'ı başlat
                    if (!session_start()) {
                        session_handle_error("critical: session start");
                        return;
                    }
            
                    LOG_INFO("Successfully connected to raw server.");
            });
        };

        resolver_->async_resolve(host, std::to_string(port), resolve_handler);
    }
    void NetworkClientImpl::DoWork() {
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time < TIME_BUDGET) {
            if (!io_context_.poll_one()) break;
        }

        if (connect_state_ == EConnectState::DISCONNECTED) {
            StartReconnectTimer();
        }
    }

    //TODO: ek parametre ile kontrol edilerek post task kullanimina gecilebilir.
    void NetworkClientImpl::Disconnect(bool need_reconnect) noexcept {
        // Eger zaten bagli deilse, erken cik
        if (connect_state_ == EConnectState::DISCONNECTED) {
            LOG_INFO("Already disconnected, skipping clear progress.");
            StartReconnectTimer();
            return;
        }

        LOG_INFO("Connection lost.");

        try {
            ClearTimers();
            session_stop();

            connect_state_ = EConnectState::DISCONNECTED; //that line should be placed after session _ stop func
            OnDisconnect();
            // Timer'i baslat
            if (need_reconnect) {
                StartReconnectTimer();
            }
        }
        catch (const std::exception& e) {
            LOG_FATAL("Exception in disconnect: ?", e.what());
        }
    }

    ESendResult NetworkClientImpl::Send(std::vector<uint8_t>& data, bool encrypt) {
        if (data.empty()) {
            session_handle_error("Empty packet");
            return ESendResult::EMPTY_PACKET;
        }

        if (!IsConnected()) {
            session_handle_error("Cannot send: not connected");
            return ESendResult::NOT_CONNECTED;
        }

        THEADER header = data[0];
#if _DEBUG
        LOG_TRACE("Writing packet: header(?)", header);
#endif

#if __MOBI_PACKET_ENCRYPTION__
        if (encrypt && is_encrypted_ && crypto_) {
            if (!crypto_->encrypt(data)) {
                session_handle_error("Encryption failed");
                return ESendResult::ENCRYPTION_FAILED;
            }
#if _DEBUG
            else {
                LOG_TRACE("Data encrypted successfully for header(?)", header);
            }
#endif
        }
#endif

        // Prevent unbounded queue growth
        #if _DEBUG   
        static constexpr size_t MAX_WRITE_QUEUE_SIZE = 100;
        #else
        static constexpr size_t MAX_WRITE_QUEUE_SIZE = 100000;
        #endif

        if (write_queue_.size() >= MAX_WRITE_QUEUE_SIZE) {
            LOG_WARN("Write queue full (size: ?), dropping oldest packet", write_queue_.size());
            write_queue_.pop_front(); // Drop oldest packet
        }

        write_queue_.emplace_back(std::move(data));

#if _DEBUG
        if (write_queue_.size() % 100 == 0) {
            LOG_WARN("Write queue size: ?", write_queue_.size());
        }
#endif

        PostTask([this]() {
            session_write();
        });

        return ESendResult::SUCCESS;
    }
} // namespace network 