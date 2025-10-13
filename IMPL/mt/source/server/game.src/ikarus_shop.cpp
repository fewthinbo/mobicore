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

		item->SetCell(destCell);
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopItemUpdatePos(this->GetOwnerPID(), shopItem->GetInfo().pos, destCell);
#endif
	...
}

bool CShop::AcceptOffer(OFFER_HANDLE offer)
{
	...
	if (auto item = GetItem(offer->itemid)) {
		...
		BuyItem(item->GetID());
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopItemBuy(this->GetOwnerPID(), offer->buyerid, item->GetInfo().pos);
#endif
		...
	}
}