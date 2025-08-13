#ifdef MOBICORE
#include "mobi_client.h"
#endif

void CInputDB::PlayerCreateSuccess(LPDESC d, const char * data)
{
	if (!d)
		return;

	TPacketDGCreateSuccess * pPacketDB = (TPacketDGCreateSuccess *) data;

	if (pPacketDB->bAccountCharacterIndex >= PLAYER_PER_ACCOUNT)
	{
		d->Packet(encode_byte(HEADER_GC_CHARACTER_CREATE_FAILURE), 1);
		return;
	}
#ifdef MOBICORE
	if (pPacketDB) {
		mobileInstance.sendCharacterCreate(pPacketDB->player.dwID);
	}
#endif
	...
}

void CInputDB::PlayerDeleteSuccess(LPDESC d, const char * data)
{
	...
#ifdef MOBICORE
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
#ifdef MOBICORE
		mobileInstance.sendChangeName(p->pid, p->name);
#endif
	}
}
