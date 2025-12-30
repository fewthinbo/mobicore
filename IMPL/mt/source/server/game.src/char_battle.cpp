#if __MOBICORE__
#include "mobi_client.h"
#endif

void CHARACTER::Dead(LPCHARACTER pkKiller, bool bImmediateDead)
{
	bool isForked = false;

#if __MOBICORE__
	auto* ch_desc = this->GetDesc();
	if (ch_desc && ch_desc->is_mobile_request) {
		DWORD victim_pid = GetPlayerID();
		if (pkKiller) {
			sys_log(0, "Player(%d) could kill to mobiDesc(pid: %d).", pkKiller->GetPlayerID(), victim_pid);
		}
		sys_log(0, "Mobidesc(pid: ?) killed, reviving immediately in safe position", victim_pid);
		this->can_mobi_warp = true;
		this->GoToSafePosition();
		this->can_mobi_warp = false;
	}
#endif

	...

#if __MOBICORE__
	if (pkKiller && pkKiller->IsPC() && this->IsMonster()) {
		mobileInstance.sendKill(pkKiller->GetPlayerID(), this->GetRaceNum());
	}
#endif
	
	SetPosition(POS_DEAD);
	...
}


bool CHARACTER::Damage(LPCHARACTER pAttacker, int dam, EDamageType type) // returns true if dead
{
#if __MOBICORE__
	auto ch_desc = GetDesc();
	if (ch_desc && ch_desc->is_mobile_request) {
		if (pAttacker) {
			SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
		}
		return false;
	}
#endif
	...
}