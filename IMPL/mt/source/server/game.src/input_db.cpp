#if __MOBICORE__
#include "mobi_client.h"
#include "character/ch_manager.h"
#endif


void CInputDB::LoginSuccess(DWORD dwHandle, const char *data)
{
	...
	if (strcmp(pTab->status, "OK"))
	{
		...
		LoginFailure(d, pTab->status);
#if __MOBICORE__
		mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::INVALID_STATUS);
#endif
		return;
	}

	...

	d->BindAccountTable(pTab);
#if __MOBICORE__
	if (!d->is_mobile_request) {
#endif

	...
	d->SendLoginSuccessPacket();

#if __MOBICORE__
	}
	else {
		mobileChInstance.CharacterSelect(d);
	}
#endif

	...
}


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
	TPlayerItem t;
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

void CInputDB::ChangeName(LPDESC d, const char * data)
{
	...

	for (...)
		if (...)
		{
			...
			strlcpy(pgc.name, p->name, sizeof(pgc.name));
#if __MOBICORE__
			mobileInstance.sendChangeName(p->pid, p->name);
#endif
			...
		}
}

void CInputDB::PlayerLoad(LPDESC d, const char * data)
{
	...

	if (!SECTREE_MANAGER::instance().GetValidLocation(pTab->lMapIndex, pTab->x, pTab->y, lMapIndex, pos, d->GetEmpire()))
	{
		...
		d->SetPhase(PHASE_CLOSE);
#if __MOBICORE__
		mobileChInstance.NotifyStatus(d, EMobiLoad::INVALID_LOCATION);
#endif
		...
	}
	
	...

	{
		...
#if __MOBICORE__
		p.is_mobile_request = d->is_mobile_request;
#endif

		P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGLogin));

		...
	}

	...


#if __MOBICORE__
	if (d->is_mobile_request) {
		ch->QuerySafeboxSize();
		mobileChInstance.Entergame(d);
		return;
	}
#endif

	ch->MainCharacterPacket();
}


void CInputDB::LoginAlready(LPDESC d, const char * c_pData)
{
	if (!d)
		return;

#if __MOBICORE__
	if (!d->is_mobile_request)
#endif
	{
		TPacketDGLoginAlready ...
		...
	}

	LoginFailure(d, "ALREADY");
#if __MOBICORE__
	mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::INGAME_REAL);
#endif
}

void CInputDB::AuthLogin(LPDESC d, const char * c_pData)
{
	if (!d)
		return;
	...


#if __MOBICORE__
	if (!d->is_mobile_request)
	{
#endif
	TPacketGCAuthSuccess ptoc;

	...

	d->Packet(&ptoc, sizeof(TPacketGCAuthSuccess));
#if __MOBICORE__
	}
	else {
		mobileChInstance.HandleLoginResult(d, bResult);
	}
#endif
	...
}


int CInputDB::Analyze(LPDESC d, BYTE bHeader, const char * c_pData)
{
	switch (bHeader)
	{
		...
	case HEADER_DG_LOGIN_NOT_EXIST:
		LoginFailure(...);
#if __MOBICORE__
		mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::ID_NOT_EXISTS);
#endif
		break;

	case HEADER_DG_LOGIN_WRONG_PASSWD:
		LoginFailure(...);
#if __MOBICORE__
		mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::WRONG_PWD);
#endif
		break;
		...
		
	...
	case HEADER_DG_PLAYER_LOAD_FAILED:
#if __MOBICORE__
		mobileChInstance.NotifyStatus(DESC_MANAGER::instance().FindByHandle(m_dwHandle), mobi_game::EMobiLoad::PLAYER_LOAD_FAILED);
#endif
		...
		break;

#if __MOBICORE__
	case HEADER_DG_MOBI_LOGIN:
		mobileChInstance.SendLoginRequest(c_pData);
		break;
#endif
	}
}