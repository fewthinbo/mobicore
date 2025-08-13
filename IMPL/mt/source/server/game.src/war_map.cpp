#ifdef MOBICORE
#include "mobi_client.h"
#endif

bool CWarMap::SetEnded()
{
	if (m_pkEndEvent)
		return false;
#ifdef MOBICORE
	mobileInstance.sendGuildWarEnd(m_TeamData[0].dwID, m_TeamData[1].dwID);
#endif
	...
}


void CWarMap::IncMember(LPCHARACTER ch)
{
	...
#ifdef MOBICORE
	if (ch) {
		mobileInstance.sendGuildWarPlayerJoin(gid, ch->GetPlayerID());
	}
#endif
	m_set_pkChr.insert(ch);
	...
}

void CWarMap::DecMember(LPCHARACTER ch)
{
	...
#ifdef MOBICORE
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
#ifdef MOBICORE
	mobileInstance.sendGuildWarMapNotification(m_TeamData[0].dwID, m_TeamData[1].dwID, psz);
#endif
}

void CWarMap::OnKill(LPCHARACTER killer, LPCHARACTER ch)
{
	...
#ifdef MOBICORE
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