#include "stdafx.h"
#include "constants.h"
#include "gm.h"
#include "messenger_manager.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "log.h"
#include "config.h"
#include "p2p.h"
#include "crc32.h"
#include "char.h"
#include "char_manager.h"
#include "questmanager.h"
#if __MOBICORE__
#include "db.h"
#include "mobi_client.h"
MessengerManagerEnhanced::~MessengerManagerEnhanced() noexcept = default;

void MessengerManagerEnhanced::HandleConnectionLost() {
	//Clear mobile status of all users
	for (const auto& acc : m_set_mobileAccount) {
		auto newStatus = IsAccountInGame(acc) ? EUserStatus::IN_GAME : EUserStatus::OFFLINE;
		NotifyStatusChange(acc, newStatus);

		// Clean up relations only if completely offline
		if (newStatus == EUserStatus::OFFLINE)
		{
			for (auto& relationPair : m_Relation)
			{
				relationPair.second.erase(acc);
			}
			m_Relation.erase(acc);
		}
	}

	m_set_mobileAccount.clear();
}

uint16_t MessengerManagerEnhanced::GetUserStatus(keyA account) const
{
	bool inGame = IsAccountInGame(account);
	bool inMobile = IsAccountInMobile(account);
	
	if (inGame && inMobile)
		return EUserStatus::IN_GAME | EUserStatus::IN_MOBILE;
	else if (inGame)
		return EUserStatus::IN_GAME;
	else if (inMobile)
		return EUserStatus::IN_MOBILE;
	else
		return EUserStatus::OFFLINE;
}

bool MessengerManagerEnhanced::IsAccountInGame(keyA account) const
{
	return m_set_loginAccount.find(account) != m_set_loginAccount.end();
}

bool MessengerManagerEnhanced::IsAccountInMobile(keyA account) const
{
	return m_set_mobileAccount.find(account) != m_set_mobileAccount.end();
}

void MessengerManagerEnhanced::SetAccountGameStatus(keyA account, bool inGame)
{
	if (inGame)
		m_set_loginAccount.insert(account);
	else
		m_set_loginAccount.erase(account);
}

void MessengerManagerEnhanced::SetAccountMobileStatus(keyA account, bool inMobile)
{
	if (inMobile)
		m_set_mobileAccount.insert(account);
	else
		m_set_mobileAccount.erase(account);
}

void MessengerManagerEnhanced::NotifyStatusChange(keyA account, uint16_t newStatus)
{
	ForEachFriend(account, [this, &account, newStatus](keyA friendAccount) {
		// No need to update stored status - we calculate it real-time
		// Just notify if friend is in game
		if (!IsAccountInGame(friendAccount))
			return;
		
		SendStatusUpdate(friendAccount, account, newStatus);
	});
}

void MessengerManagerEnhanced::SendStatusUpdate(keyA account, keyA companion, uint16_t newStatus)
{
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : NULL;

	if (!d || !d->GetCharacter())
		return;

	if (ch->GetGMLevel() == GM_PLAYER && gm_get_level(companion.c_str()) != GM_PLAYER)
		return;

	// Use optimized packet format for all status updates
	TPacketGCMessenger pack{};
	TPacketGCMessengerUserStatus userInfo{};

	userInfo.status = newStatus;
	userInfo.name_len = companion.size();

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_UPDATE_STATUS;
	pack.size = sizeof(TPacketGCMessenger) + sizeof(TPacketGCMessengerUserStatus) + userInfo.name_len;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&userInfo, sizeof(TPacketGCMessengerUserStatus));
	d->Packet(companion.c_str(), companion.size());
}

void MessengerManagerEnhanced::MobileLogin(MessengerManager::keyA account)
{
	if (IsAccountInMobile(account))
		return;

	SetAccountMobileStatus(account, true);
	
	// Get new status after mobile login
	auto newStatus = GetUserStatus(account);
	
	// Notify friends about status change
	NotifyStatusChange(account, newStatus);
}

void MessengerManagerEnhanced::MobileLogout(MessengerManager::keyA account)
{
	if (!IsAccountInMobile(account))
		return;

	SetAccountMobileStatus(account, false);
	
	auto newStatus = GetUserStatus(account);
	
	NotifyStatusChange(account, newStatus);
	
	// Clean up relations only if completely offline
	if (newStatus == EUserStatus::OFFLINE)
	{
		for (auto& relationPair : m_Relation)
		{
			relationPair.second.erase(account);
		}
		m_Relation.erase(account);
	}
}

bool MessengerManagerEnhanced::IsMessageToMobile(keyA companion) const
{
	return IsAccountInMobile(companion) && !IsAccountInGame(companion);
}
#endif

...

void MessengerManager::Login(MessengerManager::keyA account)
{
#if __MOBICORE__
	if (IsAccountInGame(account))
		return;

	DBManager::instance().FuncQuery(std::bind(&MessengerManager::LoadList, this, std::placeholders::_1),
			"SELECT account, companion FROM messenger_list%s WHERE account='%s'", get_table_postfix(), account.c_str());

	SetAccountGameStatus(account, true);
#else
	
	if (m_set_loginAccount.find(account) != m_set_loginAccount.end())
		return;

	DBManager::instance().FuncQuery(std::bind(&MessengerManager::LoadList, this, std::placeholders::_1),
			"SELECT account, companion FROM messenger_list%s WHERE account='%s'", get_table_postfix(), account.c_str());

	m_set_loginAccount.insert(account);
#endif
}

void MessengerManager::LoadList(SQLMsg * msg)
{
	...
#if __MOBICORE__
	auto currentStatus = GetUserStatus(account);
	NotifyStatusChange(account, currentStatus);
#else
	
	std::set<MessengerManager::keyT>::iterator it;

	for (it = m_InverseRelation[account].begin(); it != m_InverseRelation[account].end(); ++it)
		SendLogin(*it, account);
#endif
}

void MessengerManager::SendLogin(MessengerManager::keyA account, MessengerManager::keyA companion)
{
#if __MOBICORE__
	if(!companion.size()) return;
	auto companionStatus = GetUserStatus(companion);
	SendStatusUpdate(account, companion, companionStatus);
#else
	...
#endif
}

void MessengerManager::Logout(MessengerManager::keyA account)
{
#if __MOBICORE__
	if (!IsAccountInGame(account))
		return;

	SetAccountGameStatus(account, false);
	
	// Get new status after game logout (might still be mobile)
	auto newStatus = GetUserStatus(account);

	NotifyStatusChange(account, newStatus);

	// Clean up relations only if completely offline
	if (newStatus == EUserStatus::OFFLINE)
	{
		for (auto& relationPair : m_Relation)
		{
			relationPair.second.erase(account);
		}
		m_Relation.erase(account);
	}
#else
	...
	m_Relation.erase(account);
#endif
}

void MessengerManager::SendLogout(MessengerManager::keyA account, MessengerManager::keyA companion)
{
#if __MOBICORE__
	if (!companion.size()) return;

	auto companionStatus = GetUserStatus(companion);
	SendStatusUpdate(account, companion, companionStatus);
#else
	...
#endif
}

void MessengerManager::__AddToList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
#if __MOBICORE__
	m_Relation[account].insert(companion);
	m_InverseRelation[companion].insert(account);

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : NULL;

	if (d)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<메신저> %s 님을 친구로 추가하였습니다."), companion.c_str());
	}

	// Send status notification - status calculated on-demand
	auto companionStatus = GetUserStatus(companion);
	if (companionStatus & EUserStatus::IN_GAME || companionStatus & EUserStatus::IN_MOBILE)
		SendLogin(account, companion);
	else
		SendLogout(account, companion);
#else
	...
#endif
}

bool MessengerManager::AuthToAdd(MessengerManager::keyA account, MessengerManager::keyA companion, bool bDeny)
{
	...
	if (!bDeny)
	{
		...
#if __MOBICORE__
		mobileInstance.sendMessengerAdd(account, companion);
#endif
	}
	...
}

void MessengerManager::AddToList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
	...

#if __MOBICORE__
	auto relationIt = m_Relation.find(account);
	if (relationIt != m_Relation.end() && relationIt->second.find(companion) != relationIt->second.end())
		return;
#else
	if (m_Relation[account].find(companion) != m_Relation[account].end())
		return;
#endif

	...
}

#if ENABLE_PLAYER_BLOCK_SYSTEM
void MessengerManager::__RemoveFromList(MessengerManager::keyA account, MessengerManager::keyA companion, bool isComp)
{
#if __MOBICORE__
	auto relationIt = m_Relation.find(account);
	if (relationIt != m_Relation.end())
	{
		relationIt->second.erase(companion);
	}
	
	auto inverseIt = m_InverseRelation.find(companion);
	if (inverseIt != m_InverseRelation.end())
	{
		inverseIt->second.erase(account);
	}
#else
	m_Relation[account].erase(companion);
	m_InverseRelation[companion].erase(account);
#endif

	...
}
#else
void MessengerManager::__RemoveFromList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
#if __MOBICORE__
	auto relationIt = m_Relation.find(account);
	if (relationIt != m_Relation.end()){
		relationIt->second.erase(companion);
	}
	
	auto inverseIt = m_InverseRelation.find(companion);
	if (inverseIt != m_InverseRelation.end()){
		inverseIt->second.erase(account);
	}
#else
	m_Relation[account].erase(companion);
	m_InverseRelation[companion].erase(account);
#endif

	...
}
#endif

bool MessengerManager::IsInList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
#if __MOBICORE__
	auto relationIt = m_Relation.find(account);
	if (relationIt == m_Relation.end())
		return false;

	if (relationIt->second.empty())
		return false;

	return relationIt->second.find(companion) != relationIt->second.end();
#else
	...
#endif
}


void MessengerManager::RemoveAllList(keyA account)
{
#if __MOBICORE__
	auto relationIt = m_Relation.find(account);
	if (relationIt == m_Relation.end())
		return;

	DBManager::instance().Query("DELETE FROM messenger_list%s WHERE account='%s' OR companion='%s'",
			get_table_postfix(), account.c_str(), account.c_str());

	for (const auto& friendName : relationIt->second)
	{
		this->RemoveFromList(account, friendName);
	}
#else
	...
#endif
}

void MessengerManager::SendList(MessengerManager::keyA account)
{
	...

	if (!d)
		return;

#if __MOBICORE__
	auto relationIt = m_Relation.find(account);
	if (relationIt == m_Relation.end())
		return;

	if (relationIt->second.empty())
		return;

	TPacketGCMessenger pack;
	pack.header		= HEADER_GC_MESSENGER;
	pack.subheader	= MESSENGER_SUBHEADER_GC_LIST;
	pack.size		= sizeof(TPacketGCMessenger);

	TEMP_BUFFER buf(128 * 1024); // 128k

	for (const auto& friendName : relationIt->second) {
		// Calculate status real-time instead of using stored status
		auto friendStatus = GetUserStatus(friendName);

		// Send detailed status information for each friend
		TPacketGCMessengerUserStatus userInfo{};
		
		userInfo.status = static_cast<uint8_t>(friendStatus);
		userInfo.name_len = friendName.size();

		buf.write(&userInfo, sizeof(TPacketGCMessengerUserStatus));
		buf.write(friendName.c_str(), friendName.size());
	}
#else
	if (m_Relation.find(account) == m_Relation.end())
		return;

	...

	while (it != eit)
	{
		...
	}
#endif
	pack.size += buf.size();

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->Packet(buf.read_peek(), buf.size());
}

