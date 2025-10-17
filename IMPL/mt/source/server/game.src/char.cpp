#if __MOBICORE__
#include "mobi_client.h"
#endif

void CHARACTER::Destroy(){
#if __MOBICORE__
	mobileInstance.sendLogout(GetPlayerID());
#endif
	...
}

void CHARACTER::PointChange(BYTE type ...)
{
	...
		case POINT_LEVEL:
			if ((GetLevel() + amount) > gPlayerMaxLevel)
				return;

			SetLevel(GetLevel() + amount);
			val = GetLevel();
#if __MOBICORE__
			mobileInstance.sendLevelPacket(GetPlayerID(), GetLevel());
#endif
		...
}

bool CHARACTER::ChangeSex()
{
	...
#if __MOBICORE__
	mobileInstance.sendChangeSex(GetPlayerID(), m_points.job);
#endif
	return true;

}

void CHARACTER::SetRace(BYTE race)
{
	...

	m_points.job = race;
#if __MOBICORE__
	//mobileInstance.sendChangeRace(GetPlayerID(), race);
#endif
}