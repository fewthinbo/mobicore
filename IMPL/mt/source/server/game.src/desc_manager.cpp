#ifdef MOBICORE
#include "mobi_client.h"
#endif

void DESC_MANAGER::UpdateLocalUserCount(){
	...
#ifdef MOBICORE
	mobileInstance.sendOnlineCount(m_aiEmpireUserCount);
#endif
}

#ifdef MOBICORE
LPDESC DESC_MANAGER::FindByAccountID(uint32_t id) {
	for (const auto& d : m_set_pkDesc) {
		if (!d) continue;
		if (d->GetAccountTable().id == id) return d;
	}
	return nullptr;
}
#endif