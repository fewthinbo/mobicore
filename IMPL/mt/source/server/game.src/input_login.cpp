#ifdef MOBICORE
#include "mobi_client.h"
#endif


void CInputLogin::Entergame(LPDESC d, const char * data)
{
	...

#ifdef MOBICORE
	if (ch) {
		mobileInstance.sendLogin(ch->GetPlayerID(), ch->GetMapIndex());
	}
#endif

	TPacketGCTime p;
	...
}