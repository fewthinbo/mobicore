#if __MOBICORE__
#include "mobi_client.h"
#endif

void DESC_MANAGER::UpdateLocalUserCount(){
	...
#if __MOBICORE__
	mobileInstance.sendOnlineCount(m_aiEmpireUserCount);
#endif
}

#if __MOBICORE__
LPDESC DESC_MANAGER::FindByAccountID(uint32_t id) {
	for (const auto& d : m_set_pkDesc) {
		if (!d) continue;
		if (d->GetAccountTable().id == id) return d;
	}
	return nullptr;
}
#endif