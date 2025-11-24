#if __MOBICORE__
#include "character/ch_manager.h"
#endif


void CInputAuth::Login(LPDESC d, const char * c_pData)
{
	...
	if (false == ...)
	{
		...
		LoginFailure(d, "NOID");
#if __MOBICORE__
		if (d && d->is_mobile_request) {
			mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::ID_NOT_EXISTS);
		}
#endif
		return;
	}

	if (g_bNoMoreClient)
	{
#if __MOBICORE__
		if (d && d->is_mobile_request) {
			mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::SHUTDOWN);
			return;
		}
#endif
		...
	}

	if (...)
	{
		LoginFailure(d, "ALREADY");
#if __MOBICORE__
		if (d && d->is_mobile_request){
			mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::INGAME_REAL);
		}
#endif
		return;
	}


}