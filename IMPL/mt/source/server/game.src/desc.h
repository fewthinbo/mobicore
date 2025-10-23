#if __MOBICORE__
#include <cstdint>

namespace mobi_game {
	class CMobiCharManager;
}

struct TMobiDescInfo {
	uint32_t pid{};
};
#endif


class DESC
{
	...
#if __MOBICORE__
public:
	friend class mobi_game::CMobiCharManager;
private:
	bool is_mobile_request{false};//is loading/loaded for mobile
#endif
}