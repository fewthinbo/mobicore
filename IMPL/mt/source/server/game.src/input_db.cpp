#if __MOBICORE__
#include "mobi_client.h"
#endif

void CInputDB::PlayerCreateSuccess(LPDESC d, const char * data)
{
	...

	d->Packet(&pack, sizeof(TPacketGCPlayerCreateSuccess));
#if __MOBICORE__
#if __MT_DB_INFO__
	if (pPacketDB) {
		mobileInstance.sendCharacterCreate(pPacketDB->player.dwID);
	}
#else
	if (pPacketDB) {
		mobileInstance.sendCharacterCreate(r_Tab.players[pPacketDB->bAccountCharacterIndex], r_Tab.id);
	}
#endif
#endif
	...
}

void CInputDB::PlayerDeleteSuccess(LPDESC d, const char * data)
{
	...
#if __MOBICORE__
	if (d) {
		auto& player = d->GetAccountTable().players[account_index];
		mobileInstance.sendCharacterDelete(player.dwID);
		player.dwID = 0;
	}
#else
	d->GetAccountTable().players[account_index].dwID = 0;
#endif
}


void CInputDB::ChangeName(LPDESC d, const char* data)
{
	...

	if (r.players[i].dwID == p->pid)
	{
		...
#if __MOBICORE__
		mobileInstance.sendChangeName(p->pid, p->name);
#endif
		d->Packet(&pgc, sizeof(TPacketGCChangeName));
	}
}
