#pragma once
#include <string>
#include <functional>

#include <boost/system/error_code.hpp>

#include "common.h"

namespace network {
	enum class CloseType {
		NONE,
		USER,
		SYSTEM
	};
	
	struct ErrorContext {
		ErrorCategory category;
		std::string message;
		std::string details;
		CloseType should_close_session;
		boost::system::error_code ec;
		ErrorContext() : category(ErrorCategory::SYSTEM), should_close_session(CloseType::NONE) {}
	};

	// Hata geri çağırma fonksiyon tipi
	using ErrorCallback = std::function<void(const ErrorContext&)>;

	class ErrorHandler final {
	public:
		ErrorHandler() = default;
		~ErrorHandler() noexcept;

		// IErrorHandler implementation
		void handleError(const char* operation, const boost::system::error_code& ec = boost::system::error_code());

		// Hata kategorilendirme fonksiyonları
		ErrorContext categorizeError(const boost::system::error_code& ec, const char* operation) noexcept;

		void adjustShouldCloseSession(const boost::system::error_code& ec, ErrorContext& context) noexcept;
		void adjustErrorCategory(const boost::system::error_code& ec, ErrorContext& context) noexcept;
		// Hata geri çağırma fonksiyonu
		void setErrorCallback(ErrorCallback callback) noexcept;

	private:
		ErrorCallback m_errorCallback;
	};

}

