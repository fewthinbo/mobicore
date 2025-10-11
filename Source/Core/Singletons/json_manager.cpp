#include "json_manager.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#endif
#if PLATFORM_FREEBSD
#include <iconv.h>
#include <errno.h>
#endif
#include <locale>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <atomic>

namespace JSONTables {
	TResponse::TResponse(const std::string& _msg, long _code)
		: code(_code) {
		if (code != -1) {
			json_data["a"] = code;
		}
		if (!_msg.empty()) {
			json_data["4"] = _msg;
		}
	}
	bool TResponse::HasDataExceptID() const {
		if (json_data.contains("L") && json_data.size() != 1) return true;

		//has not id and not empty
		return !json_data.contains("L") && !json_data.empty();
	}
	TResponse::TResponse(uint32_t packetId) {
		json_data["L"] = packetId;
	}
	TResponse::~TResponse() noexcept = default;
};


namespace NSingletons {
#if PLATFORM_FREEBSD
	// RAII wrapper for iconv to prevent resource leaks
	class IconvWrapper {
	private:
		iconv_t cd_;
	public:
		IconvWrapper(const char* tocode, const char* fromcode)
			: cd_(iconv_open(tocode, fromcode)) {
		}

		~IconvWrapper() {
			if (cd_ != (iconv_t)-1) {
				iconv_close(cd_);
			}
		}

		bool isValid() const { return cd_ != (iconv_t)-1; }
		iconv_t get() const { return cd_; }

		// Prevent copying
		IconvWrapper(const IconvWrapper&) = delete;
		IconvWrapper& operator=(const IconvWrapper&) = delete;
	};

	static inline const char* CharsetFromCodePage(uint16_t code_page) {
		switch (code_page) {
		case 65001: return "UTF-8";
		case 1254:  return "Windows-1254";
		case 1252:  return "Windows-1252";
		case 1251:  return "Windows-1251";
		case 949:   return "CP949";
		case 936:   return "GB2312";
		case 932:   return "Shift_JIS";
		case 28599: return "ISO-8859-9";
		case 28591: return "ISO-8859-1";
		default:    return nullptr;
		}
	}
#endif

	std::string JsonManager::toString(const type_json& j) const {
		try {
			// According to nlohmann::json FAQ recommendation: use error_handler_t::replace
			// This is recommended for Turkish characters and encoding issues
			return j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
		}
		catch (const std::exception&) {
			// In case of error, try with ignore handler
			try {
				return j.dump(-1, ' ', false, nlohmann::json::error_handler_t::ignore);
			}
			catch (const std::exception&) {
				return ""; // Safe fallback
			}
		}
	}

	type_json JsonManager::toJson(const std::string& data) const {
		try {
			return type_json::parse(data, nullptr, true, true); // allow_exceptions=true, ignore_comments=true
		}
		catch (const std::exception& e) {
			LOG_ERR("Exception caught: ?", e.what());
			return type_json{ nullptr }; // Safe fallback
		}
	}

	bool JsonManager::fieldcmp(const type_json& j, const std::string& _field, const std::string& _field2) const {
		if (j.is_null()) {
			LOG_WARN("Json is null");
			return false;
		}
		if (!j.contains(_field)) {
			LOG_WARN("Field ? not found", _field);
			return false;
		}
		if (!j.contains(_field2)) {
			LOG_WARN("Field ? not found", _field2);
			return false;
		}

		// Zero-copy: Direct references to JSON strings
		const auto& jsonStr1 = j[_field].get_ref<const std::string&>();
		const auto& jsonStr2 = j[_field2].get_ref<const std::string&>();
		return jsonStr1 == jsonStr2;
	}


	std::string JsonManager::timePointToStringWithMS(const std::chrono::system_clock::time_point& tp) const {
		auto microsec = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count() % 1000000;
		std::time_t t = std::chrono::system_clock::to_time_t(tp);
		std::tm tm = *std::localtime(&t);
		std::stringstream ss;
		ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(6) << microsec;
		return ss.str();
	}

	std::chrono::system_clock::time_point JsonManager::stringToTimePoint(const std::string& timeStr) const {
		std::tm tm = {};
		long microseconds = 0;
		std::istringstream ss(timeStr);
		ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

		auto dotPos = timeStr.find('.');
		if (dotPos != std::string::npos) {
			ss.ignore(1); // Ignore the dot
			ss >> microseconds;

			// Micro second field can be less than 6 digits, fix it
			// Avoid substr copy by calculating length directly
			size_t microLen = timeStr.length() - dotPos - 1;
			if (microLen < 6) {
				microseconds *= static_cast<int>(pow(10, 6 - microLen));
			}
		}

		auto timePoint = std::chrono::system_clock::from_time_t(std::mktime(&tm));
		timePoint += std::chrono::microseconds(microseconds);
		return timePoint;
	}

	// Convert from Windows encoding to UTF-8 (for Turkish characters)
	std::string JsonManager::toUtf8(const std::string& input, uint16_t code_page) const {
		if (input.empty()) {
			return "";
		}

		// Fast path: if already valid UTF-8, return as-is
		if (isValidUtf8(input)) {
			return ""; // Already valid UTF-8
		}

#if PLATFORM_WINDOWS
		int len = MultiByteToWideChar(code_page, 0, input.c_str(), -1, NULL, 0);
		if (len <= 0) {
			return "";
		}

		std::wstring wideStr(len, L'\0');
		if (MultiByteToWideChar(code_page, 0, input.c_str(), -1, &wideStr[0], len) <= 0) {
			return "";
		}

		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
		if (utf8Len <= 0) {
			return "";
		}

		std::string utf8Str(utf8Len, '\0');
		if (WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &utf8Str[0], utf8Len, NULL, NULL) <= 0) {
			return "";
		}

		if (!utf8Str.empty() && utf8Str.back() == '\0') {
			utf8Str.pop_back();
		}
		return utf8Str;
#endif
#if PLATFORM_FREEBSD
		const char* charset = CharsetFromCodePage(code_page);
		if (charset == nullptr) {
			LOG_ERR("Charset of code_page(?) doesn't exists.", code_page);
			return "";
		}

		IconvWrapper conv(charset, "UTF-8");
		if (!conv.isValid()) return "";

		size_t inbytesleft = input.size();
		size_t outbytesleft = input.size() * 4;
		std::vector<char> output(outbytesleft);

		char* inbuf = const_cast<char*>(input.data());
		char* outbuf = output.data();

		size_t result = iconv(conv.get(), &inbuf, &inbytesleft, &outbuf, &outbytesleft);

		if (result != (size_t)-1) {
			std::string fixed_str(output.data(), output.size() - outbytesleft);
			if (!fixed_str.empty() && isValidUtf8(fixed_str)) {
				LOG_TRACE("Utf8 convertion successful, result(?)", fixed_str);
				LOG_TRACE("InSize: ?, OutSize: ?", input.size(), fixed_str.size());
				return fixed_str;
			}
		}

		// All conversions failed, return empty
		LOG_TRACE("Utf8 convertion failed, str(?)", input);
		return "";
#endif
	}

	// UTF-8 validation - optimized manual validation
	bool JsonManager::isValidUtf8(const std::string& str) const {
		if (str.empty()) {
			return true;
		}

		const size_t len = str.size();
		for (size_t i = 0; i < len; ) {
			unsigned char c = static_cast<unsigned char>(str[i]);

			if (c < 0x80) {
				// ASCII
				++i;
			}
			else if ((c & 0xE0) == 0xC0) {
				// 2-byte
				if (i + 1 >= len) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				}
				unsigned char c1 = static_cast<unsigned char>(str[i + 1]);

				uint32_t codepoint = ((c & 0x1F) << 6) | (c1 & 0x3F);
				if ((c1 & 0xC0) != 0x80 || codepoint < 0x80) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				}

				i += 2;
			}
			else if ((c & 0xF0) == 0xE0) {
				// 3-byte
				if (i + 2 >= len) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				}
				unsigned char c1 = static_cast<unsigned char>(str[i + 1]);
				unsigned char c2 = static_cast<unsigned char>(str[i + 2]);

				uint32_t codepoint = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
				if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				}
				if (codepoint < 0x800) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				}
				if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				} // UTF-16 surrogate

				i += 3;
			}
			else if ((c & 0xF8) == 0xF0) {
				// 4-byte
				if (i + 3 >= len) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				}
				unsigned char c1 = static_cast<unsigned char>(str[i + 1]);
				unsigned char c2 = static_cast<unsigned char>(str[i + 2]);
				unsigned char c3 = static_cast<unsigned char>(str[i + 3]);

				uint32_t codepoint = ((c & 0x07) << 18) |
					((c1 & 0x3F) << 12) |
					((c2 & 0x3F) << 6) |
					(c3 & 0x3F);

				if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				}
				if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
					LOG_TRACE("? is invalid utf8", str);
					return false;
				}

				i += 4;
			}
			else {
				// Invalid start byte
				LOG_TRACE("? is invalid utf8", str);
				return false;
			}
		}

		return true;
	}


	const std::string& JsonManager::getStringField(const type_json& j, const std::string& field) const {
		if (!j.contains(field) || !j[field].is_string()) {
			static const std::string empty_str = "";
			return empty_str;
		}

		const auto& jsonStr = j[field].get_ref<const std::string&>();

		return jsonStr;
	}

	const type_json::array_t& JsonManager::getArrayField(const type_json& j, const std::string& field) const {
		if (!j.contains(field) || !j[field].is_array()) {
			static const type_json::array_t empty_arr = {};
			return empty_arr;
		}

		return j[field].get_ref<const type_json::array_t&>();
	}
}