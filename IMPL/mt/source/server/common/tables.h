{

#if __MOBICORE__ 
	HEADER_GD_MOBI_LOAD = 253 //adjust this number: should be unique
#endif

	HEADER_GD_SETUP = 0xff,
	...
}

#if __MOBICORE__ 
struct TGDHandleData {
	uint32_t handle_id{};
	bool is_remove{ false };
};
#endif


typedef struct SAccountTable
{
	...
	TSimplePlayer	players[PLAYER_PER_ACCOUNT];

#if __MOBICORE__
	int GetIndexByPID(DWORD pid) const {
		for (auto i = 0; i < PLAYER_PER_ACCOUNT; ++i) {
			if (players[i].dwID == pid) {
				return i;
			}
		}
		return -1;
	}
#endif
} TAccountTable;

