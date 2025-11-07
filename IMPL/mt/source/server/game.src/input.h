#if __MOBICORE__
namespace mobi_game {
	class CMobiCharManager;
}
#endif

class CInputLogin : public CInputProcessor
{
	public:
		virtual BYTE	GetType() { return INPROC_LOGIN; }
#if __MOBICORE__
		friend class mobi_game::CMobiCharManager;
#endif
	...
}