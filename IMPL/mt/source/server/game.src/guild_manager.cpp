#ifdef MOBICORE
#include "mobi_client.h"
#endif

void CGuildManager::StartWar(DWORD guild_id1, DWORD guild_id2)
{
	...
#ifdef MOBICORE
	mobileInstance.sendGuildWarStart(guild_id1, guild_id2, 100);
#endif
}


bool CGuildManager::EndWar(DWORD guild_id1, DWORD guild_id2){
	...
/*#ifdef MOBICORE
	mobileInstance.sendGuildWarEnd(guild_id1, guild_id2);
#endif*/
	return true;
}

DWORD CGuildManager::CreateGuild(TGuildCreateParameter& gcp)
{
	...

#ifdef MOBICORE //Bottom of function
	if (pg) {
		mobileInstance.sendGuildCreate(*pg);
	}
#endif
	return pg->GetID();
}

void CGuildManager::DisbandGuild(DWORD guild_id)
{
	...
#ifdef MOBICORE
	mobileInstance.sendGuildDelete(guild_id);
#endif
}
