class CPythonMessenger : ...
{
		...
		

#if !__MOBICORE__
		void OnFriendLogin(const char * c_szKey);
		void OnFriendLogout(const char * c_szKey);
#else
		enum EUserStatus : uint16_t {
			OFFLINE = (1 << 0),
			IN_GAME = (1 << 1),
			IN_MOBILE = (1 << 2)
		};	
		void UpdateFriendStatus(const char * name, uint16_t status);
#endif
		...

}