#if __MOBICORE__
#include "mobi_client.h"
#endif

void CInputLogin::Entergame(LPDESC d, const char * data)
{
	...

#if __MOBICORE__
	if (ch) {
		mobileInstance.sendLogin(ch->GetPlayerID(), ch->GetMapIndex());
	}
#endif

	TPacketGCTime p;
	...
}