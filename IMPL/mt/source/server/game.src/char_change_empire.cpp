#if __MOBICORE__
#include "mobi_client.h"
#endif

int CHARACTER::ChangeEmpire(BYTE empire)
{
	if (GetEmpire() == empire)
		return 1;
#if __MOBICORE__
	mobileInstance.sendChangeEmpire(GetPlayerID(), empire);
#endif
	...
}