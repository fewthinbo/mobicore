#ifdef MOBICORE
#include "mobi_client.h"
#endif

void DESC_MANAGER::UpdateLocalUserCount(){
	...
#ifdef MOBICORE
	mobileInstance.sendOnlineCount(m_aiEmpireUserCount);
#endif
}