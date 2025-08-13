#include "error_handler.h"

#include <boost/asio.hpp>

namespace network {
    ErrorContext ErrorHandler::categorizeError(const boost::system::error_code& ec, const char* operation) noexcept {
        ErrorContext context;
        context.message = std::string(operation);
        context.details = ec.message();
        context.ec = ec;
        adjustErrorCategory(ec, context);
        adjustShouldCloseSession(ec, context);

        return context;
    }

    ErrorHandler::~ErrorHandler() noexcept = default;

    void ErrorHandler::handleError(const char* operation, const boost::system::error_code& ec) {
        ErrorContext context = categorizeError(ec, operation);
        if (m_errorCallback) {
            m_errorCallback(context);
        }
    }

    void ErrorHandler::adjustErrorCategory(const boost::system::error_code& ec, ErrorContext& context) noexcept {
        if (ec.category() == boost::asio::error::get_system_category()) {
            context.category = ErrorCategory::NETWORK;
        }
        else if (ec.category() == boost::asio::error::get_misc_category()) {
            context.category = ErrorCategory::PROTOCOL;
        }
        else {
            context.category = ErrorCategory::SYSTEM;
        }
    }

    void ErrorHandler::adjustShouldCloseSession(const boost::system::error_code& ec, ErrorContext& context) noexcept {

        // Sonra session'in kapatilip kapatilmayacaginin karar verilmesi
        if (ec == boost::asio::error::connection_reset ||      // ECONNRESET: Ani kapatma (RST)
            ec == boost::asio::error::connection_aborted ||    // ECONNABORTED: Yazilim kapatmasi
            ec == boost::asio::error::network_down ||          // ENETDOWN: Ağ kapali 
            ec == boost::asio::error::broken_pipe ||           // EPIPE: 
            ec == boost::asio::error::operation_aborted ||     // islem iptal edildi
            ec == boost::asio::error::shut_down) {
            context.should_close_session = CloseType::SYSTEM;
        }
        else if (context.details.find("connection reset") != std::string::npos ||
            context.details.find("connection aborted") != std::string::npos ||
            context.details.find("broken pipe") != std::string::npos ||
            context.details.find("operation aborted") != std::string::npos ||
            context.details.find("connection was closed") != std::string::npos) {
            context.should_close_session = CloseType::SYSTEM;
        }
        else if (context.message.find("critical:") != std::string::npos) {
            context.should_close_session = CloseType::SYSTEM;
        }
        else if (ec == boost::asio::error::eof ||
            context.details.find("end of file") != std::string::npos) {
            context.should_close_session = CloseType::USER;
        }
        // Kurtarilabilir hatalar icin session'i kapatma
        else if (ec == boost::asio::error::connection_refused ||
            ec == boost::asio::error::timed_out ||
            ec == boost::asio::error::host_unreachable ||
            ec == boost::asio::error::network_unreachable) {
            context.should_close_session = CloseType::NONE;
        }
        else {
            context.should_close_session = CloseType::NONE;
        }
    }

    void ErrorHandler::setErrorCallback(ErrorCallback callback) noexcept {
        m_errorCallback = std::move(callback);
    }



}

