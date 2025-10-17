#if __MOBICORE__
#include "mobi_client.h"
#endif

void CGuildManager::StartWar(DWORD guild_id1, DWORD guild_id2)
{
	...
#if __MOBICORE__
	mobileInstance.sendGuildWarStart(guild_id1, guild_id2, 100);
#endif
}


bool CGuildManager::EndWar(DWORD guild_id1, DWORD guild_id2){
	...
/*#if __MOBICORE__
	mobileInstance.sendGuildWarEnd(guild_id1, guild_id2);
#endif*/
	return true;
}

DWORD CGuildManager::CreateGuild(TGuildCreateParameter& gcp)
{
	...

#if __MOBICORE__
	if (pg) {
#if __MT_DB_INFO__
		mobileInstance.sendGuildCreate(pg->GetID());
#else
		mobileInstance.sendGuildCreate(*pg);
#endif
	}
#endif
	return pg->GetID();
}

void CGuildManager::DisbandGuild(DWORD guild_id)
{
	...
#if __MOBICORE__
	mobileInstance.sendGuildDelete(guild_id);
#endif
}
