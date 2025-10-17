#if __MOBICORE__
#include "mobi_client.h"
#endif

#if __MOBICORE__
const TeamData& CWarMap::GetTeamData(BYTE bIdx) const {
	if (bIdx >= 2) {
		bIdx = 0;
	}
	return m_TeamData[bIdx];
}
#endif

bool CWarMap::SetEnded()
{
	if (m_pkEndEvent)
		return false;
#if __MOBICORE__
	mobileInstance.sendGuildWarEnd(m_TeamData[0].dwID, m_TeamData[1].dwID);
#endif
	...
}


void CWarMap::IncMember(LPCHARACTER ch)
{
	...
#if __MOBICORE__
	if (ch) {
		mobileInstance.sendGuildWarPlayerJoin(gid, ch->GetPlayerID());
	}
#endif
	SendWarPacket(d);
	...
}

void CWarMap::DecMember(LPCHARACTER ch)
{
	...
#if __MOBICORE__
	if (ch) {
		mobileInstance.sendGuildWarPlayerLeave(gid, ch->GetPlayerID());
	}
#endif
	m_set_pkChr.erase(ch);
	...
}

void CWarMap::Notice(const char * psz)
{
	...
#if __MOBICORE__
	mobileInstance.sendGuildWarMapNotification(m_TeamData[0].dwID, m_TeamData[1].dwID, psz);
#endif
}

void CWarMap::OnKill(LPCHARACTER killer, LPCHARACTER ch)
{
	...
#if __MOBICORE__
	if (ch && killer) {
		mobileInstance.sendGuildWarPlayerKill(dwKillerGuild, dwDeadGuild, killer->GetPlayerID(), ch->GetPlayerID());
	}
#endif

	switch (m_kMapInfo.bType)
	{
		case WAR_MAP_TYPE_NORMAL:
			SendGuildWarScore(dwKillerGuild, dwDeadGuild, 1, ch->GetLevel());
			break;
			...
	}
	...
}