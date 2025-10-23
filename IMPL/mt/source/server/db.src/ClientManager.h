#if __MOBICORE__ 
#include <unordered_set>
#include <chrono>

struct THandlerMobi {
	const DWORD handle_id{};
	std::steady_clock::time_point load_begin{};
	THandlerMobi(DWORD hid) : handle_id(hid) {}
	THandlerMobi() = delete;

	static constexpr auto TIMEOUT_MIN = std::chrono::minutes(5);
	bool IsLoadTimeout(const std::chrono::steady_clock::time_point& now) const {
		return now - load_begin > TIMEOUT_MIN;
	}

	bool operator==(const THandlerMobi& other) const {
		return this->handle_id == other.handle_id;
	}
};
namespace std {
	template<>
	struct hash<THandlerMobi> {
		std::size_t operator()(const THandlerMobi& obj) const {
			return std::hash<DWORD>{}(obj.handle_id);
		}
	};
}
#endif

class CClientManager : ...
{
	...

	void		BlockChat(TPacketBlockChat* p);
#if __MOBICORE__ 
private:
	std::unordered_set<THandlerMobi> mobi_handlers_;
public:
	bool IsLoadingForMobi(DWORD handle_id) const;
	void MobiHandle(const char* data);
#endif

}



