#if __MOBICORE__
#include "mobi_client.h"
#endif


int CInputMain::Chat(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	...

	if (pinfo->type == CHAT_TYPE_SHOUT)
	{
		...

		ch->SetLastShoutPulse(thecore_heart->pulse);
#if __MOBICORE__
		mobileInstance.sendShout(ch->GetPlayerID(), buf, pinfo->code_page);
#endif

		...
	}
	...
}


int CInputMain::Whisper(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	...

#if __MOBICORE__
	if (pinfo && ch && MessengerManager::instance().IsMessageToMobile(pinfo->szNameTo)) {
		char buf[CHAT_MAX_LEN + 1];
		strlcpy(buf, data + sizeof(TPacketCGWhisper), MIN(iExtraLen + 1, sizeof(buf)));
		const size_t buflen = strlen(buf);
		if (buflen > 0) {
			mobileInstance.sendUserCheck(pinfo->szNameTo, buf, ch->GetPlayerID(), pinfo->code_page);
		}
		return iExtraLen;
	}
#endif

	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().FindPC(pinfo->szNameTo);

	...
}

int CInputMain::Messenger(LPCHARACTER ch, const char* c_pData, size_t uiBytes)
{
	...
	case MESSENGER_SUBHEADER_CG_REMOVE:
	{
		...
#if __MOBICORE__
		if (ch) {
			mobileInstance.sendMessengerRemove(ch->GetName(), char_name);
		}
#endif
	}
	...
}