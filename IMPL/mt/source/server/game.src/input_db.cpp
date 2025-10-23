#if __MOBICORE__
#include "mobi_client.h"
#include "character/ch_manager.h"
#endif


void CInputDB::AuthLogin(LPDESC d, const char* c_pData)
{
	...

#if __MOBICORE__
	if (!mobileChInstance.IsLoadingForMobi(d))
	{
#endif
	TPacketGCAuthSuccess ptoc;

	...

	d->Packet(&ptoc, sizeof(TPacketGCAuthSuccess));
	sys_log(...);

#if __MOBICORE__
	}
	else {
		mobileChInstance.CharacterSetPhase(PHASE_LOGIN);
	}
#endif

}

void CInputDB::LoginSuccess(DWORD dwHandle, const char* data)
{
	...

	d->BindAccountTable(pTab);
#if __MOBICORE__
	if (!mobileChInstance.IsLoadingForMobi(d))
	{
#endif

	if (!bFound) {
			...
	}

	...
	d->SendLoginSuccessPacket();

#if __MOBICORE__
	}
	else {
		mobileChInstance.CharacterSetPhase(PHASE_SELECT);
	}
#endif

}

void CInputDB::PlayerLoad(LPDESC d, const char* data)
{
#if __MOBICORE__
	bool is_mobile_request = mobileChInstance.IsLoadingForMobi(d);
	if (!is_mobile_request) {
#endif
	if (!d)
		return;
#if __MOBICORE__
	}
#endif

	TPlayerTable* pTab = (TPlayerTable*)data;

	...

#if __MOBICORE__
	if (!is_mobile_request)
	{
#endif

	long lMapIndex = pTab->lMapIndex;
	PIXEL_POSITION pos;

	...


	if (d->GetCharacter() || d->IsPhase(PHASE_GAME))
	{
		LPCHARACTER p = d->GetCharacter();
		sys_err("login state already has main state (character %s %p)", p->GetName(), get_pointer(p));
		return;
	}
#if __MOBICORE__
	}
#endif

	...

	LPCHARACTER ch = CHARACTER_MANAGER::instance().CreateCharacter(pTab->name, pTab->id);
#if __MOBICORE__
	if (is_mobile_request) {
		mobileChInstance.CharacterAdd(ch);
	}
#endif

	...

#if __MOBICORE__
	if (!is_mobile_request)
#endif
	{
		// P2P Login
	}

	...

#if __MOBICORE__
	if (!is_mobile_request)
	{
#endif
		d->SetPhase(PHASE_LOADING);

	...

#if __MOBICORE__
	}
	else {
		mobileChInstance.CharacterSetPhase(PHASE_LOADING);
	}
#endif
	ch->QuerySafeboxSize();
}

//EOF CH SYSTEM

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
