#if __MOBICORE__
#include "mobi_client.h"
#include "character/ch_manager.h"
#endif

int CInputP2P::Analyze(LPDESC d, BYTE bHeader, const char * c_pData)
{
	...
	switch (bHeader)
	{
		...
#if __MOBICORE__
		case HEADER_GG_MOBITEST: {
			if (auto spam_packet = reinterpret_cast<const TMobiTest*>(c_pData)) {
				mobileInstance.spamTest(spam_packet->intensity);
			}
			break;
		}
#endif
	}
}