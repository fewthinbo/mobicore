#if __MOBICORE__ && __OFFSHOP__
#include "mobi_client.h"
#endif

bool CShop::ModifyItemPrice(DWORD itemid, const TPriceInfo& price)
{
	...
	if (!shopItem)
		return false;
#if __MOBICORE__ && __OFFSHOP__
	mobileInstance.sendShopItemUpdatePrice(this->GetOwnerPID(), shopItem->GetInfo().pos, price);
#endif
	...
}


void CShop::MoveItem(DWORD itemid, int destCell)
{
	...
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopItemUpdatePos(this->GetOwnerPID(), item->GetInfo().pos, destCell);
#endif
		item->SetCell(destCell);
	...
}
