#if __MOBICORE__
#include "mobi_client.h"
#endif

void CHARACTER::Dead(LPCHARACTER pkKiller, bool bImmediateDead)
{
	...

#if __MOBICORE__
	if (pkKiller && pkKiller->IsPC() && this->IsMonster()) {
		mobileInstance.sendKill(pkKiller->GetPlayerID(), this->GetRaceNum());
	}
#endif
	
	SetPosition(POS_DEAD);
	...
}