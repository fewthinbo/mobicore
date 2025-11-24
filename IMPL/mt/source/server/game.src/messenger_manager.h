#if !__INC_MESSENGER_MANAGER_H
#define __INC_MESSENGER_MANAGER_H
#include "db.h"

#if __MOBICORE__
#include <string>
//#include <functional>
#include <set>
#include <map>
#include <cstdint>
#include <type_traits>

enum EUserStatus : uint16_t {
	OFFLINE = (1 << 0),
	IN_GAME = (1 << 1),
	IN_MOBILE = (1 << 2)
};

class MessengerManagerEnhanced
{
public:
	using keyT = std::string;
	using keyA = const std::string&;
	virtual ~MessengerManagerEnhanced() noexcept;
private:
	using TAccountSet = std::set<keyT>;
	using TFriendMap = std::map<keyT, std::set<keyT>>;
protected:
	uint16_t GetUserStatus(keyA account) const;
	bool IsAccountInGame(keyA account) const;
	bool IsAccountInMobile(keyA account) const;
	void SetAccountGameStatus(keyA account, bool inGame);
	void SetAccountMobileStatus(keyA account, bool inMobile);
	void NotifyStatusChange(keyA account, uint16_t newStatus);

	// handles all transitions efficiently
	void SendStatusUpdate(keyA account, keyA companion, uint16_t newStatus);

	// on bool function : if returns false, stops iteration
	template<typename Func, typename = std::enable_if_t<std::is_invocable_v<Func, keyA>>>
	void ForEachFriend(keyA account, Func&& callback) const{
		auto inverseIt = m_InverseRelation.find(account);
		if (inverseIt == m_InverseRelation.end())
			return;

		for (const auto& friendAccount : inverseIt->second){
			if constexpr (std::is_convertible_v<std::invoke_result_t<Func, keyA>, bool>) {
				if (!callback(friendAccount))
					break;  // Early exit if callback returns false
			}
			else{
				callback(friendAccount);
			}
		}
	}
public:
	bool IsMessageToMobile(keyA companion) const; //hem oyun hem mobildeyse oyuna gider, aksi takdirde mobile gider.
	void MobileLogin(keyA account);
	void MobileLogout(keyA account);
	void HandleConnectionLost();
protected:
	TAccountSet				m_set_mobileAccount;   // Mobile login tracking
	TAccountSet				m_set_loginAccount;
	TFriendMap				m_Relation;
	TFriendMap				m_InverseRelation;
	std::set<DWORD>			m_set_requestToAdd;
};
#endif

class MessengerManager : public singleton<MessengerManager>
#if __MOBICORE__
	, public MessengerManagerEnhanced
#endif
{
	...

#if !__MOBICORE__
		std::set<keyT>						m_set_loginAccount;
		std::map<keyT, std::set<keyT> >		m_Relation;
		std::map<keyT, std::set<keyT> >		m_InverseRelation;
		std::set<DWORD>						m_set_requestToAdd;
#endif
	...
};