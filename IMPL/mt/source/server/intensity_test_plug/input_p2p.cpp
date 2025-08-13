#ifdef MOBICORE
#include "mobi_client.h"
#endif

int CInputP2P::Analyze(LPDESC d, BYTE bHeader, const char* c_pData)
{
	...
	switch (bHeader)
	{
		...
#ifdef MOBICORE
	case HEADER_GG_MOBITEST:
		if (auto spam_packet = reinterpret_cast<const TMobiTest*>(c_pData)) {
			mobileInstance.spamTest(spam_packet->intensity);
		}
		break;
#endif
	}
}