#if __MOBICORE__ && __OFFSHOP__
#include "mobi_client.h"
#endif

bool CShopManager::RecvShopBuyDBPacket(DWORD buyerid, DWORD ownerid, DWORD itemid)
{
	...

		if (auto ch = ...)
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
		else
		{
			...
#if __MOBICORE__ && __OFFSHOP__
			if (shopItem) {
				mobileInstance.sendShopItemBuy(ownerid, buyerid, shopItem->GetID());
			}
#endif		
			shop->BuyItem(shopItem->GetID());
		}

	return true;
}

bool CShopManager::RecvShopEditItemClientPacket(LPCHARACTER ch, DWORD itemid, const TPriceInfo& price)
{
#if __MOBICORE__ && __OFFSHOP__
	if (!ch) return false;
	if (!ch->GetIkarusShop()) {
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_SHOP);
		return false;
	}
#else
	if (!ch || !ch->GetIkarusShop()) return false;
#endif

	if(!CheckGMLevel(ch))
		return false;

	if(!ch->IkarusShopFloodCheck(SHOP_ACTION_WEIGHT_EDIT_ITEM)) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::FLOOD);
#endif
		return false;
	}

	if (!IsGoodSalePrice(price)) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::INVALID_SALE_PRICE);
#endif
		return false;
	}

	auto shop = ch->GetIkarusShop();
	auto shopItem = shop->GetItem(itemid);
	if(!shopItem) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_ITEM);
#endif
		return false;
	}

	...
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

bool CShopManager::RecvShopAddItemDBPacket(DWORD ownerid, const TShopItem& iteminfo) {

	...
#if __MOBICORE__ && __OFFSHOP__
	mobileInstance.sendShopItemAdd(ownerid, iteminfo);
#endif
	return shop->AddItem(iteminfo);
}


bool CShopManager::RecvShopForceCloseDBPacket(DWORD pid) {
	...
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopClose(pid);
#endif
}

bool CShopManager::RecvShopCreateNewDBPacket(const TShopInfo& info)
{
	...
#if __MOBICORE__ && __OFFSHOP__
	mobileInstance.sendShopCreate(info);
#endif
	return true;
}

bool CShopManager::RecvShopOfferAcceptDBPacket(DWORD offerid, DWORD ownerid)
{
	...

	// copying handle & removing from map
	auto offer = it->second;
	m_mapOffer.erase(it);

#if __MOBICORE__ && __OFFSHOP__
	if (offer && shop) {
		mobileInstance.sendShopItemBuy(shop->GetOwnerPID(), offer->buyerid, offer->itemid);
	}
#endif

	...
}

void CShopManager::RecvShopUnlockCellDBPacket(DWORD owner, int lockIndex)
{
	...
	shop->ChangeLockIndex(lockIndex);

#if __MOBICORE__ && __OFFSHOP__
	mobileInstance.sendShopUpdateSlotCount(owner, lockIndex);
#endif
}


bool CShopManager::RecvShopBuyItemClientPacket(LPCHARACTER ch, DWORD ownerid, DWORD itemid, bool searching, long long seenprice)  //patch seen price check
{
	if (!ch) {
		return false;
	}
	
	if (!ch->IkarusShopFloodCheck(SHOP_ACTION_WEIGHT_BUY_ITEM)) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::FLOOD);
#endif
		return false;
	}


	if (!CheckCharacterActions(ch))
	{
		...
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::CHARACTER_ACTIONS);
#endif
		return true;
	}

	if (!CheckGMLevel(ch)) {
		return false;
	}

	// searching shop
	SHOP_HANDLE shop = GetShopByOwnerID(ownerid);
	if(!shop){
		...
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_SHOP);
#endif
		return false;
	}

#ifdef EXTEND_IKASHOP_PRO
	if(shop->GetDuration() == 0) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_SHOP);
#endif
		return false;
	}
#endif

	// searching item
	auto pitem = shop->GetItem(itemid);
	if(!pitem){
		...
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_ITEM);
#endif
		return false;
	}

	if(!pitem->CanBuy(ch))
	{
		...
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_ENOUGH_MONEY);
#endif
		return false;
	}

	//patch seen price check
	if (pitem->GetPrice().GetTotalYangAmount() != seenprice)
	{
		...
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::PRICE_MISMATCH);
#endif
		return false;
	}

	...
}


bool CShopManager::RecvShopRemoveItemClientPacket(LPCHARACTER ch, DWORD itemid)
{
	if (!ch || !ch->GetIkarusShop())
		return false;

	if(!CheckGMLevel(ch))
		return false;

	if (!ch->IkarusShopFloodCheck(SHOP_ACTION_WEIGHT_REMOVE_ITEM)) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::FLOOD);
#endif
		return false;
	}

	if (!CheckSafeboxSize(ch)) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_ENOUGH_SAFEBOX_SPACE);
#endif
		return false;
	}

	...

	if(!shop->GetItem(itemid)) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_ITEM);
#endif
		return false;
	}

	...
}


void CShopManager::RecvShopMoveItemClientPacket(LPCHARACTER ch, int srcPos, int destPos)
{
	// getting the shop
	auto shop = ch->GetIkarusShop();
	if (!shop) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_SHOP);
#endif
		return;
	}

	if (!ch->IkarusShopFloodCheck(SHOP_ACTION_WEIGHT_SHOP_MOVE_ITEM)) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::FLOOD);
#endif
		return;
	}

	...
	if (iter == shop->GetItems().end()) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_ITEM);
#endif
		return;
	}

	...

	if(!table) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_EXISTS_ITEM);
#endif
		return;
	}

	if (!shop->ReserveSpace(destPos, table->bSize)) {
#if __MOBICORE__ && __OFFSHOP__
		mobileInstance.sendShopOpResponse(ch->GetPlayerID(), mobi_game::EResponseShopOperation::NOT_ENOUGH_SAFEBOX_SPACE);
#endif
		return;
	}

	...
}