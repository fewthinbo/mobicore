#if __MOBICORE__ && __OFFSHOP__
#include "mobi_client.h"
#endif

bool CShopManager::RecvShopCreateNewDBPacket(const TShopInfo& info)
{
	...
#if __MOBICORE__ && __OFFSHOP__
	mobileInstance.sendShopCreate(info);
#endif
	return true;
}

bool CShopManager::RecvShopForceCloseDBPacket(DWORD pid) {
	...
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopClose(pid);
#endif
}

bool CShopManager::RecvShopRestoreDurationDBPacket(DWORD owner)
{
	...
#if __MOBICORE__ && __OFFSHOP__
	//TODO: shop duration'u OFFLINESHOP_DURATION_MAX_MINUTES ile guncelle bunu direkt ara sunuccuda aypabilirim.
	mobileInstance.sendShopUpdateDuration(owner);
#endif
	return true;
}

void CShopManager::RecvShopUnlockCellDBPacket(DWORD owner, int lockIndex)
{
	...
	shop->ChangeLockIndex(lockIndex);

#if __MOBICORE__ && __OFFSHOP__
	mobileInstance.sendShopUpdateSlotCount(owner, lockIndex);
#endif
}

bool CShopManager::RecvShopAddItemDBPacket(DWORD ownerid, const TShopItem& iteminfo) {

	...
#if __MOBICORE__ && __OFFSHOP__
	mobileInstance.sendShopItemAdd(ownerid, iteminfo);
#endif
	return shop->AddItem(iteminfo);
}


bool CShopManager::RecvShopRemoveItemDBPacket(DWORD ownerid, DWORD itemid, bool requester)
{
	...
#if __MOBICORE__ && __OFFSHOP__
	if (shopItem) {
		mobileInstance.sendShopItemRemove(ownerid, shopItem->GetInfo().pos);
	}
#endif
	shop->RemoveItem(itemid);
	...
}

bool CShopManager::RecvShopBuyDBPacket(DWORD buyerid, DWORD ownerid, DWORD itemid)
{
	...

		if (auto ch = CHARACTER_MANAGER::instance().FindByPID(buyerid))
		{
			auto itemInstance = shopItem->CreateItem();
			if (!itemInstance) {
				...
			}
#if __MOBICORE__ && __OFFSHOP__
			if (shopItem) {
				mobileInstance.sendShopItemBuy(ownerid, buyerid, shopItem->GetInfo().pos);
			}
#endif		
		}

	return true;
}

