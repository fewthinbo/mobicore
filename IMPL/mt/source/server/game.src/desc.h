...

#if __MOBICORE__
#include <cstdint>
#include <chrono>

namespace mobi_game {
	class CMobiCharManager;
}
#endif

class CInputProcessor;


class DESC
{
	...
#if __MOBICORE__
	public:
		friend class mobi_game::CMobiCharManager;
		bool is_mobile_request{ false };//is loading/loaded for mobile
		unsigned long p2p_online_tracker{ 0 };
		std::chrono::steady_clock::time_point last_activity{};
#endif
}