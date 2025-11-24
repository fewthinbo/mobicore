#if __MOBICORE__
#include "mobi_client.h"
#include "character/ch_manager.h"
#endif

void CHARACTER::Destroy()
{
#if __MOBICORE__
	if (m_destroyed) return;
	m_destroyed = true;
	if (IsPC()){
		mobileInstance.sendLogout(GetPlayerID());
	}
	/*if (auto* d = GetDesc()) {
		if (d->is_mobile_request) {
			mobileChInstance.HandleLogout(GetPlayerID());
		}
	}*/
#endif

	CloseMyShop();
	...
}


void CHARACTER::Disconnect(const char * c_pszReason)
{
	...
	if (GetShop())
	{
		...
	}

#if __MOBICORE__
	auto* ch_desc = GetDesc();
	if (ch_desc && !ch_desc->is_mobile_request) {
#endif

	if (GetArena() != NULL)
	{
		...
	}

	if (GetParty() != NULL)
	{
		...
	}

	marriage::CManager::instance().Logout(this);

#if __MOBICORE__
	}
#endif


	...

	if (m_pWeddingMap)
	{
		...
	}

#if __MOBICORE__
	if (ch_desc && !ch_desc->is_mobile_request) {
#endif

	if (GetGuild())
		GetGuild()->LogoutMember(this);

#if __MOBICORE__
	}
#endif

	quest::CQuestManager::instance().LogoutPC(this);

#if __MOBICORE__
	if (ch_desc && !ch_desc->is_mobile_request) {
#endif

	if (GetParty())
		GetParty()->Unlink(this);

#if __MOBICORE__
	}
#endif

	if (IsStun() || IsDead())
	{
		...
	}



	...
	

	CloseMall();

#if __MOBICORE__
	if (ch_desc && !ch_desc->is_mobile_request) {
#endif

	CPVPManager::instance().Disconnect(this);

#if __MOBICORE__
	}
#endif

	CTargetManager::instance().Logout(...);

#if __MOBICORE__
	if (ch_desc && !ch_desc->is_mobile_request) {
#endif

	MessengerManager::instance().Logout(GetName());

#if __MOBICORE__
	}
#endif

	...
}



bool CHARACTER::ChangeSex()
{
	...
#if __MOBICORE__
	if (this->IsPC()) {
		mobileInstance.sendChangeSex(GetPlayerID(), m_points.job);
	}
#endif
	sys_log(0, ...);
	return true;
}


void CHARACTER::SetRace(BYTE race)
{
	...

#if __MOBICORE__
	if (this->IsPC()) {
		//mobileInstance.sendChangeRace(GetPlayerID(), race);
	}
#endif
	m_points.job = race;
}

void CHARACTER::SetEmpire(BYTE bEmpire)
{
	m_bEmpire = bEmpire;
#if __MOBICORE__
	if (this->IsPC()) {
		mobileInstance.sendChangeEmpire(GetPlayerID(), bEmpire);
	}
#endif
}


void CHARACTER::SetPlayerProto(const TPlayerTable * t)
{
	...
#if __MOBICORE__
	auto* d = GetDesc();
	if (d && d->is_mobile_request) {
		m_afAffectFlag.Set(AFF_MOBILE);
	}
#endif

	if (GetLevel() < PK_PROTECT_LEVEL)
		m_bPKMode = PK_MODE_PROTECT;

	...
}


void CHARACTER::PointChange(BYTE type, long long amount, bool bAmount, bool bBroadcast)
{
	...
	switch (type)
	{
		...
		
		case POINT_LEVEL:
			...

			val = GetLevel();
#if __MOBICORE__
			mobileInstance.sendLevelPacket(GetPlayerID(), GetLevel());
#endif

		...
	}
}

bool CHARACTER::WarpSet(long x, long y, long lPrivateMapIndex)
{
	if (!IsPC())
		return false;
#if __MOBICORE__
	auto* desc = GetDesc();
	if (desc && desc->is_mobile_request && !this->can_mobi_warp) {
		sys_log(0, "MobiPlayer(%d) cannot warp itself", this->GetPlayerID());
		return false;
	}
#endif

	...
#if __MOBICORE__
	if (desc && desc->is_mobile_request) {
		mobileChInstance.SendMobiWarp(desc, lAddr, wPort);
	}
	else {
#endif

	TPacketGCWarp p;

	...

	GetDesc()->Packet(p);

#if __MOBICORE__
	}
#endif

}


void CHARACTER::WarpEnd()
{
	...
	{
		// P2P Login
		...
		
#if __MOBICORE__
		if (auto* ch_desc = GetDesc()) {
			p.is_mobile_request = ch_desc->is_mobile_request;
		}
		else {
			p.is_mobile_request = false;
		}
#endif
		P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGLogin));
	}
}

void CHARACTER::SetBlockModeForce(BYTE bFlag)
{
#if __MOBICORE__
	auto* d = GetDesc();
	if (d->is_mobile_request) {
		sys_log(0, "SetBlockModeForce: Tried to change block modes of mobiDesc, skipping");
		return;
	}
#endif
	m_pointsInstant.bBlockMode = bFlag;
	...
}


#if __MOBICORE__
void CHARACTER::GoToSafePosition() {
	auto* d = GetDesc();
	if (!d) return;

	if (!d->is_mobile_request) {
		sys_log(0, "Player(%d) trying to use mobiDesc funcs", GetPlayerID());
		return;
	}

	if (!this->can_mobi_warp) {
		sys_log(0, "MobiDesc(%d) tried to use GoToSafePosition but is not allowed", GetPlayerID());
		return;
	}

	DWORD pid = GetPlayerID();
	long map_idx = GetMapIndex();
	const PIXEL_POSITION& pos = GetXYZ();
	auto& sectree_manager = SECTREE_MANAGER::instance();

	//already in safe position
	if (!sectree_manager.IsAttackablePosition(map_idx, pos.x, pos.y)) return;

	//sys_err("Searching safe area for mobiDesc(%d), coord(%d, %d), map(%d)", pid, pos.x, pos.y, map_idx);
	SetHP(GetMaxHP());
	SetPosition(POS_STANDING); //cmd komutlarinin duzgun calismasi icin yeterli pos 

	//warp to nearest safe area if exists in that map
	PIXEL_POSITION result_s{};
	if (sectree_manager.GetCoordsOfSafeArea(map_idx, pos.x, pos.y, result_s)) {
		//sys_err("Safe area found: (%d, %d) warping", result_s.x, result_s.y);
		WarpSet(result_s.x, result_s.y);
	}
	else/*home is probably safe*/ {
		//sys_err("Warping to home directly");
		GoHome();
	}
}
#endif