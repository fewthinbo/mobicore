#if __MOBICORE__
#include "character/ch_manager.h"
#endif

void P2P_MANAGER::Boot(LPDESC d)
{
	
	...
	while (it != map.end())
	{
		...
#if __MOBICORE__
		if (auto* ch_desc = ch->GetDesc()) {
			p.is_mobile_request = ch_desc->is_mobile_request;
		}
		else {
			p.is_mobile_request = false;
		}
#endif
		d->Packet(&p, sizeof(p));
	}
}


void P2P_MANAGER::Login(LPDESC d, const TPacketGGLogin * p)
{
	...
	pkCCI->pkDesc = d;
#if __MOBICORE__
	pkCCI->is_mobile_request = p->is_mobile_request;
	if (d) {
		++d->p2p_online_tracker;
	}
#endif
	...
}

void P2P_MANAGER::Logout(CCI * pkCCI)
{
	if (pkCCI->bChannel == g_bChannel)
	{
		if (...)
		{
			--m_aiEmpireUserCount[pkCCI->bEmpire];
#if __MOBICORE__
			if (auto* d = pkCCI->pkDesc) {
				--d->p2p_online_tracker;
			}
#endif
			if (... < 0)
			{
				...
			}
		}
		else
		{
			...
		}
	}
#if __MOBICORE__
	/*if (pkCCI->is_mobile_request) {
		mobileChInstance.HandleLogout(pkCCI->dwPID);
	}*/
#endif
	...
}

