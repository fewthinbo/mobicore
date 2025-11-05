#pragma once
//#include "log_manager.h"
#include <singleton.h>

#include <nlohmann/json.hpp>

#include <type_traits>
#include <typeindex>
#include <string>

//#define _JSON_LOG


using type_json = nlohmann::json;

namespace JSONTables {
	struct TResponse {
		long code = -1;
		type_json json_data = type_json::object(); // main json
		TResponse(const std::string& _msg = "", long _code = -1);
		bool HasDataExceptID() const;
		TResponse(uint32_t packetId);
		~TResponse() noexcept;
	};
};

namespace NSingletons {
	class JsonManager : public CSingleton<JsonManager> {
	public:
		template<typename T>
		void set(type_json& j, const std::string& _field, T&& _value) const {
			using clearType = std::decay_t<T>;
			try {
				if constexpr (std::is_convertible_v<clearType, const char*>
					|| std::is_same_v<clearType, std::string>
					|| std::is_integral_v<clearType>
					|| std::is_floating_point_v<clearType>
					|| std::is_same_v<clearType, unsigned char>
					|| std::is_same_v<clearType, type_json>
					) {
					j[_field] = _value;
				}
				//timepoint için
				else if constexpr (std::is_same_v<clearType, std::chrono::system_clock::time_point>) {
					j[_field] = timePointToStringWithMS(_value);
				}
				else if constexpr (std::is_enum_v<clearType>) {
					j[_field] = static_cast<int>(_value);
				}
				else if constexpr (std::is_same_v<clearType, std::atomic<bool>>) {
					j[_field] = _value.load(std::memory_order_acquire);
				}
				// std::vector<uint8_t> için
				else if constexpr (std::is_same_v<clearType, std::vector<uint8_t>>) {
					j[_field] = std::string(_value.begin(), _value.end());
				}
			}
			catch (const std::exception& e) {
				//LOG_ERR("Error setting JSON field ?: ?", _field, e.what());
			}
		}

		template<typename T>
		void getFieldValue(const type_json& j, const std::string& field, T& value) const {
			using clearType = std::decay_t<T>;
			try {
				if (!j.contains(field)) {
					return;
				}

				if constexpr (std::is_convertible_v<clearType, const char*> || std::is_same_v<clearType, std::string>) {
					if (!j[field].is_string()) return;
					value = j[field].get_ref<const std::string&>();
				}
				// integers, floating points, json, boo lean
				else if constexpr (std::is_integral_v<clearType> || std::is_floating_point_v<clearType> || std::is_same_v<clearType, type_json>) {
					if (!(j[field].is_number() || j[field].is_boolean() || j[field].is_structured())) return;
					value = j[field].get<clearType>();
				}
				else if constexpr (std::is_enum_v<clearType>) {
					if (!j[field].is_number()) return;
					value = static_cast<clearType>(j[field].get<int>());
				}
				// Time point için
				else if constexpr (std::is_same_v<clearType, std::chrono::system_clock::time_point>) {
					const auto& jsonStr = j[field].get_ref<const std::string&>();
					value = stringToTimePoint(jsonStr);
				}
			}
			catch (const std::exception& e) {
				//LOG_ERR("Error getting JSON field ?: ?", field, e.what());
			}
		}

		template<typename ... Args>
		bool checkField(const type_json& j, Args&& ... args) const {
			if (j.is_null()) {
				//LOG_WARN("Json is null, argSize(?)", sizeof...(args));
				return false;
			}
			return (j.contains(args) && ...);
		}

		template<typename T>
		bool valcmp(const type_json& j, const std::string& _field, T&& _value) const {
			using clearType = std::decay_t<T>;
			if (j.is_null()) {
				//LOG_WARN("Json is null");
				return false;
			}
			if (!j.contains(_field)) {
				//LOG_WARN("Field ? not found", _field);
				return false;
			}

			// Time point için karşılaştırma
			if constexpr (std::is_same_v<clearType, std::chrono::system_clock::time_point>) {
				try {
					std::string jsonStr = j[_field].get<std::string>();
					auto jsonTime = stringToTimePoint(jsonStr);
					return jsonTime == _value;
				}
				catch (const std::exception& e) {
					//LOG_ERR("Error comparing time point: ?", e.what());
					return false;
				}
			}

			// Debug için tip bilgisi
#if defined(DEBUG) && defined(_JSON_LOG)
			//LOG_TRACE("valcmp type info - Raw type: ?, Is string literal/const char*: ?, Is std::string: ?, Is decayed std::string: ?",
				typeid(T).name(),
				std::is_convertible_v<T, const char*>,
				std::is_same_v<T, std::string>,
				std::is_same_v<clearType, std::string>);
			//LOG_TRACE("valcmp comparing - Field: ?, Expected Value: ?", _field, _value);
#endif

			// String literal veya const char* için özel kontrol
			if constexpr (std::is_convertible_v<T, const char*> || std::is_same_v<clearType, std::string>) {
				try {
					if (!j[_field].is_string()) {
#if defined(DEBUG) && defined(_JSON_LOG)
						//LOG_WARN("Field ? is not a string", _field);
#endif
						return false;
					}
					const auto& jsonStr = j[_field].get_ref<const std::string&>();
					return _value == jsonStr;
				}
				catch (const std::exception& e) {
					//LOG_ERR("Error comparing string literal values: ?", e.what());
					return false;
				}
			}
			// integral types & floating points & boo lean
			else if constexpr (std::is_integral_v<clearType> || std::is_floating_point_v<clearType>) {
				try {
					if (!(j[_field].is_number() || j[_field].is_boolean())) {
						return false;
					}
					auto comp = j[_field].get<clearType>();
#if defined(DEBUG) && defined(_JSON_LOG)
					//LOG_TRACE("int/uint: valcmp comparing - Field: ?, Value in JSON: ?, Expected Value: ?", _field, comp, _value);
#endif
					return comp == _value;
				}
				catch (const std::exception& e) {
					//LOG_ERR("Error comparing integer values: ?", e.what());
					return false;
				}
			}

			// Desteklenmeyen tip
			//LOG_WARN("Unsupported type for comparison");
			return false;
		}


		std::string toString(const type_json& j) const;

		type_json toJson(const std::string& data) const;

		bool fieldcmp(const type_json& j, const std::string& _field, const std::string& _field2) const;

		// Timepoint helpers
		std::string timePointToStringWithMS(const std::chrono::system_clock::time_point& tp) const;

		std::chrono::system_clock::time_point stringToTimePoint(const std::string& timeStr) const;

		// Windows encoding'den UTF-8'e dönüşüm (Türkçe karakterler için)
		std::string toUtf8(const std::string& input, uint16_t code_page = 857/*console*/) const;

		// UTF-8 validation
		bool isValidUtf8(const std::string& str) const;

		// Convenience function for common use case
		const std::string& getStringField(const type_json& j, const std::string& field) const;

		const type_json::array_t& getArrayField(const type_json& j, const std::string& field) const;
	};
}


#define jsonInstance NSingletons::JsonManager::getInstance()
