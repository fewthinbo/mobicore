#if __MOBICORE__
#include <array>
#include <string>
#include <cstdint>
#include <chrono>

#include <Singletons/log_manager.h>

#include "mobi_client.h"

namespace mobi_game {
	using namespace consts;

	namespace test_constants {
		static const std::string one_hundred_bytes = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
		constexpr uint16_t test_code_page = 65001; //utf8
		constexpr auto intensity_part_budget = std::chrono::milliseconds(10);

		struct STimer {
			std::chrono::steady_clock::time_point time_start;
			const char* name;
			STimer(const char* task_name) : time_start(std::chrono::steady_clock::now()), name(task_name) {
				LOG_WARN("(?) has been started", name);
			}
			~STimer() {
				LOG_WARN("(?) complated in ms(?)", name,
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_start)
				);
			}
		};
	}

	//Sunucudan cevap beklenilen hicbir paket gonderilmez, amac: gonderim yogunlugunu test etmektir.
	bool MobiClient::spamTest(uint8_t intensity) {
		using namespace test_constants;

		constexpr uint32_t receiver_pid = 116;
		constexpr std::array<uint32_t, 4> pid_arr{ 1, 114, 115, 116 };

		auto time_start = std::chrono::steady_clock::now();
		auto total_budget = intensity_part_budget * intensity;
		size_t write_counter = 0;

		constexpr auto arr_size = pid_arr.size();
		LOG_WARN("Waiting other tasks to finish");
		{
			STimer timer("Write Part");

			while (std::chrono::steady_clock::now() - time_start < total_budget) {
				for (size_t i = 0; i < arr_size; ++i) {
					auto pid = pid_arr[i];
					sendShout(pid, one_hundred_bytes, test_code_page);
					sendMessage(pid, receiver_pid, one_hundred_bytes, test_code_page);
				}
				write_counter += arr_size*2;
			}
		}
		LOG_WARN("Total bytes written(~?)", write_counter * 100);
		return true;
	}
}
#endif