#pragma once
#include <cstdint>
#include <chrono>

#if !PLATFORM_WINDOWS
#define INVALID_SOCKET -1
#endif

namespace network {
	namespace consts {
#if !_DEBUG
		static constexpr auto RECONNECT_INTERVAL = std::chrono::seconds(60);
		static constexpr auto CONNECT_TIMEOUT = std::chrono::seconds(30);
#else
		static constexpr auto RECONNECT_INTERVAL = std::chrono::seconds(10);
		static constexpr auto CONNECT_TIMEOUT = std::chrono::seconds(5);
#endif

		static constexpr auto TIME_BUDGET = std::chrono::milliseconds(4);

		static constexpr size_t MAX_PENDING_TASKS = 100000;
		//IMPORTANT: yazma hizinin, gonderme hizindan 100 kat fazla olmasi kabul edilebilir.
		//daha fazlasi (bottleneck) oldugunda private bytes 
		//artimini sinirlamak adina bazi tasklar isleme alinmaz.
	}
}