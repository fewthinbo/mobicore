#pragma once
#include <singleton.h>
#include <random>
#include <mutex>

namespace NUtility {
	class CRandomGenerator : public CSingleton<CRandomGenerator> {
	private:
		static constexpr const char* m_hex_digits = "0123456789abcdef";
		static constexpr const char* m_name_digits = "abcdefghiklmnoprstuvyzx";
		std::random_device m_random_device{};
		mutable std::mt19937 m_random_generator;
	public:
		CRandomGenerator(): m_random_generator(m_random_device()){}
		std::string generate_client_uuid() const {
			static std::uniform_int_distribution<int> hex_distribution(0, 15);
			std::string client_uuid;
			client_uuid.reserve(36);

			for (int i = 0; i < 36; i++) {
				if (i == 8 || i == 13 || i == 18 || i == 23) {
					client_uuid += '-';
				}
				else {
					client_uuid += m_hex_digits[hex_distribution(m_random_generator)];
				}
			}
			return client_uuid;
		}

		std::string generate_name() const {
			static std::uniform_int_distribution<int> name_distribution(0, 22);
			std::string name;
			int nameLen = generate_number(5, 15);
			name.reserve(nameLen);

			for (int i = 0; i < nameLen; i++) {
				name += m_name_digits[name_distribution(m_random_generator)];
			}
			return name;
		}

		int generate_number(int minVal = std::numeric_limits<int>::min(), int maxVal = std::numeric_limits<int>::max()) const {
			std::uniform_int_distribution<int> numDistribution(minVal, maxVal);
			return numDistribution(m_random_generator);
		}
	};
}

#define randomInstance NUtility::CRandomGenerator::getInstance()
