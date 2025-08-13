#ifdef MOBICORE
#include "mobi_client.h"
#endif

int CHARACTER::ChangeEmpire(BYTE empire)
{
	if (GetEmpire() == empire)
		return 1;
#ifdef MOBICORE
	mobileInstance.sendChangeEmpire(GetPlayerID(), empire);
#endif
	...
}