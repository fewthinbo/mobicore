#ifdef MOBICORE
#include "mobi_client.h"
#endif

void CHARACTER::Destroy(){
#ifdef MOBICORE
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
#ifdef MOBICORE
			mobileInstance.sendLevelPacket(GetPlayerID(), GetLevel());
#endif
		...
}

bool CHARACTER::ChangeSex()
{
	...
#ifdef MOBICORE
	mobileInstance.sendChangeSex(GetPlayerID(), m_points.job);
#endif
	return true;

}


void CHARACTER::SetRace(BYTE race)
{
	...

	m_points.job = race;
#ifdef MOBICORE
	//mobileInstance.sendChangeRace(GetPlayerID(), race);
#endif
}