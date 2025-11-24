#if __MOBICORE__
#include "mobi_client.h"
#endif

#if __MOBICORE__
CWarMap::TMemberStats* CWarMap::RegisterMemberStats(CHARACTER* ch)
{
	if (!ch) return nullptr;
	CGuild* pGuild = ch->GetGuild();
	if (!pGuild) return nullptr;

	auto pair = map_MemberStats.try_emplace(ch->GetPlayerID(), std::make_unique<TMemberStats>(pGuild->GetID(), 0, 0));
	return pair.second ? pair.first->second.get() : nullptr;
}

CWarMap::TMemberStats* CWarMap::GetMemberStats(CHARACTER* ch)
{
	if (!ch) return nullptr;
	auto found = map_MemberStats.find(ch->GetPlayerID());
	return found != map_MemberStats.end() ? found->second.get() : nullptr;
}

const CWarMap::TeamData& CWarMap::GetTeamData(BYTE bIdx) const {
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
	if (isWarMember)
	{
		...
		event_cancel(&m_pkTimeoutEvent);
#if __MOBICORE__
		if (ch) {
			mobileInstance.sendGuildWarPlayerJoin(gid, ch->GetPlayerID());
		}
#endif
		...
	}
	...
}

void CWarMap::DecMember(LPCHARACTER ch)
{
	...
	if (!ch->IsObserverMode())
	{
		ch->SetQuestFlag("war.is_war_member", 0);
#if __MOBICORE__
		if (ch) {
			mobileInstance.sendGuildWarPlayerLeave(gid, ch->GetPlayerID());
		}
#endif
	}
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
	if (!GetTeamIndex(dwDeadGuild, idx))
		return;

#if __MOBICORE__
	if (ch && killer) {
		mobileInstance.sendGuildWarPlayerKill(dwKillerGuild, dwDeadGuild, killer->GetPlayerID(), ch->GetPlayerID());
		if (TMemberStats* stats = GetMemberStats(killer)) {
			stats->dwKills++;
		}
		if (TMemberStats* stats = GetMemberStats(ch)) {
			stats->dwDeaths++;
		}
	}
#endif
	...
}