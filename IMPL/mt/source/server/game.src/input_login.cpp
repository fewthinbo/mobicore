#if __MOBICORE__
#include "mobi_client.h"
#include "character/ch_manager.h"
#endif


void CInputLogin::LoginByKey(LPDESC d, const char * data)
{
	...

	if (g_bNoMoreClient)
	{
#if __MOBICORE__
		if (d->is_mobile_request) {
			mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::SHUTDOWN);
			return;
		}
#endif
		...
	}

	if (g_iUserLimit > 0)
	{
		...

		if (g_iUserLimit <= iTotal)
		{
#if __MOBICORE__
			if (d->is_mobile_request) {
				mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::USER_LIMIT);
				return;
			}
#endif
			...
		}
	}
	...
}


void CInputLogin::CharacterSelect(LPDESC d, const char * data)
{
	...

	if (!c_r.id)
	{
		...
#if __MOBICORE__
		mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::NO_ACCOUNT_TABLE);
#endif
		return;
	}

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		...
#if __MOBICORE__
		mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::INDEX_OVERFLOW);
#endif
		return;
	}

	if (!c_r.players[pinfo->index].dwID) // fixme190
	{
		...
		d->SetPhase(PHASE_CLOSE);
#if __MOBICORE__
		mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::INVALID_PID);
#endif
		return;
	}

	if (c_r.players[pinfo->index].bChangeName)
	{
		...
#if __MOBICORE__
		mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::NAME_CHANGED);
#endif
		return;
	}

	...
}


void CInputLogin::Entergame(LPDESC d, const char * data)
{
	...
#if __MOBICORE__
	if (!d->is_mobile_request) {
#endif

	CGuildManager::instance().LoginMember(ch);

#if __MOBICORE__
	}
#endif

	...

#if __MOBICORE__
	if (ch)/*her player icin gonderilir*/ {
		mobileInstance.sendLogin(ch->GetPlayerID(), ch->GetMapIndex(), d->is_mobile_request);
	}
	if (!d->is_mobile_request) {
#endif

	CPVPManager::instance().Connect(ch);

	...

	d->Packet(&p2, sizeof(p2));

#if __MOBICORE__
	}
	else if (ch) {
		//TODO: put your all EBlockAction elements like below.
		ch->SetBlockMode(
			BLOCK_EXCHANGE |
			BLOCK_PARTY_INVITE |
			BLOCK_GUILD_INVITE |
			BLOCK_WHISPER |
			BLOCK_MESSENGER_INVITE |
			BLOCK_PARTY_REQUEST
		);
	}
#endif

	...

#if __MOBICORE__
	if (g_bCheckClientVersion && !d->is_mobile_request)
#else
	if (g_bCheckClientVersion)
#endif
	{
		
		...
	}
	
	...

#ifdef __MOBICORE__
	if (!d->is_mobile_request) {
#endif
	if (ch->GetMapIndex() >= ...)
	{
		...
	}
	else if (...)
	{
		...
	}
	else if (ch->GetMapIndex() == ...)
	{
		...
	}
	else
	{
		...
	}
#ifdef __MOBICORE__
	}
	else {
		ch->GoToSafePosition();
	}
#endif
}