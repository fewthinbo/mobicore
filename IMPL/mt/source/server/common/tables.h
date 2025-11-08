#if __MOBICORE__ 
#include <cstdint>
#endif

{

#if __MOBICORE__ 
	HEADER_GD_MOBI_LOGIN = 249, //adjust this number: should be unique
	HEADER_GD_MOBI_WARP = 250, //adjust this number: should be unique
#endif
	HEADER_GD_SETUP			= 0xff,
	
	...
	
#if __MOBICORE__ 
	HEADER_DG_MOBI_LOGIN = 249, //adjust this number: should be unique
	HEADER_DG_MOBI_LOGOUT = 250, //adjust this number: should be unique
#endif
	HEADER_DG_MAP_LOCATIONS			= 0xfe,
}


#if __MOBICORE__ 
struct TMobiDGLogin {
	uint32_t pid{};
	uint32_t login_key{};
	char login[LOGIN_MAX_LEN + 1]{};
};
struct TMobiDGLogout {
	uint32_t pid{};
};
struct TMobiGD {
	uint32_t pid{};
	uint32_t login_key{};
};
struct TMobiGDWarp { //isinlanirken db'ye gonderilir.
	TMobiGD info{};
	int32_t	addr; //inet_addr
	uint16_t port{};
};
#endif

typedef struct SAccountTable
{
	...
	TSimplePlayer	players[PLAYER_PER_ACCOUNT];
#if __MOBICORE__
	int GetIndexByPID(DWORD pid) const {
		for (auto i = 0; i < sizeof(players) / sizeof(*players); ++i) {
			if (players[i].dwID == pid) {
				return i;
			}
		}
		return -1;
	}
#endif
} TAccountTable;

