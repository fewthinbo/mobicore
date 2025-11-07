#if __MOBICORE__
#include "character/ch_manager.h"
#endif


void DBManager::AnalyzeReturnQuery(SQLMsg * pMsg)
{
	...

	switch (...)
	{
		case QID_AUTH_LOGIN:
			{
				...
				if (pMsg->Get()->uiNumRows == 0)
				{
					...
					LoginFailure(d, "NOID");
#if __MOBICORE__
					mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::ID_NOT_EXISTS);
#endif
					...
				}
				else
				{
					...
					
					if (nPasswordDiff)
					{
						LoginFailure(d, "WRONGPWD");
#if __MOBICORE__ //Note: you can use mobi_game::EMobiLoad::OTHERS for other additional LoginFailures
						mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::WRONG_PWD);
#endif
						...
					}
					else if (bNotAvail)
					{
						LoginFailure(d, "NOTAVAIL");
#if __MOBICORE__
						mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::NOT_AVAIL);
#endif
						...
					}
					else if (DESC_MANAGER::instance().FindByLoginName(pinfo->login))
					{
						LoginFailure(d, "ALREADY");
#if __MOBICORE__
						mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::INGAME_REAL);
#endif
						...
					}
					else if(!CShutdownManager::Instance().CheckCorrectSocialID(szSocialID) && !test_server)
					{
						LoginFailure(d, "BADSCLID");
#if __MOBICORE__
						mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::BAD_SOCIAL_ID);
#endif
						...
					}
					else if(CShutdownManager::Instance().CheckShutdownAge(szSocialID) && CShutdownManager::Instance().CheckShutdownTime())
					{
						LoginFailure(d, "AGELIMIT");
#if __MOBICORE__
						mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::AGE_LIMIT);
#endif
						...
					}
					else if (strcmp(szStatus, "OK"))
					{
						LoginFailure(d, szStatus);
#if __MOBICORE__
						mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::INVALID_STATUS);
#endif
						...
					}
					else
					{
						{
							if (...)
							{
								LoginFailure(d, "BLKLOGIN");
#if __MOBICORE__
								mobileChInstance.NotifyStatus(d, mobi_game::EMobiLoad::BLOCKED);
#endif
								...
							}
							...
						}
						...
					}
				}
			}
			break;
	}
	
}