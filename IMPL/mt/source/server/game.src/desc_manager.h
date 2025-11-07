#if __MOBICORE__
namespace mobi_game {
	class CMobiCharManager;
}
#endif

class DESC_MANAGER : public singleton<DESC_MANAGER>
{
	...
	LPDESC			FindByCharacterName(const char* name);
#if __MOBICORE__
public:
	friend class mobi_game::CMobiCharManager;
	LPDESC			FindByAccountID(uint32_t id);
#endif
}