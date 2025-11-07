#if __MOBICORE__
#include "Locale.h"
#endif

bool CPythonNetworkStream::SendChatPacket(const char* c_szChat, BYTE byType)
{
	...
	ChatPacket.type = byType;
#if __MOBICORE__
	ChatPacket.code_page = LocaleService_GetCodePage();
#endif
}

bool CPythonNetworkStream::SendWhisperPacket(const char* name, const char* c_szChat)
{
	...
	WhisperPacket.wSize = sizeof(WhisperPacket) + iTextLen;
#if __MOBICORE__
	WhisperPacket.code_page = LocaleService_GetCodePage();
#endif
	...
}

...

bool CPythonNetworkStream::RecvWhisperPacket()
{
	...
	buf[whisperPacket.wSize - sizeof(whisperPacket)] = '\0';

	static char line[256];
#if __MOBICORE__
	if (CPythonChat::WHISPER_TYPE_MOBILE == whisperPacket.bType)
	{
		std::string converted = ConvertUtf8ToCodePage(buf);
		_snprintf(line, sizeof(line), "[M] %s : %s", whisperPacket.szNameFrom, converted.empty() ? buf : converted.c_str());
#if defined(__BL_MULTI_LANGUAGE_PREMIUM__)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper", Py_BuildValue("(issis)", (int)whisperPacket.bType, whisperPacket.szNameFrom, line, whisperPacket.bEmpire, whisperPacket.szCountry));
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper", Py_BuildValue("(iss)", whisperPacket.bType, whisperPacket.szNameFrom, line));
#endif
		return true;
	}
#endif

	...
}

...

bool CPythonNetworkStream::RecvMessenger()
{
	...
	char char_name[24+1];

	switch (p.subheader)
	{
		case MESSENGER_SUBHEADER_GC_LIST:
		{
#if !__MOBICORE__
			TPacketGCMessengerListOnline on;
			while(iSize)
			{
				....
			}
#else
			TPacketGCMessengerUserStatus userInfo{};
			auto& messenger = CPythonMessenger::Instance();
			while(iSize)
			{
				if (!Recv(sizeof(userInfo), &userInfo))
					return false;

				if (!Recv(userInfo.name_len, char_name))
					return false;

				char_name[userInfo.name_len] = '\0';

				messenger.UpdateFriendStatus(char_name, userInfo.status);
				/*
					Friend'in mevcut durumuna gore gorsel guncelleyecektir, ve uzerine tiklanip whisper gonderilecegi zaman,
					mevcut durumuna gore mobil ayarlari yapilir.
				*/

				iSize -= sizeof(TPacketGCMessengerUserStatus);
				iSize -= userInfo.name_len;
			}
#endif
			break;
		}

	 	...
#if !__MOBICORE__
		case MESSENGER_SUBHEADER_GC_LOGIN:
		{
			...
			break;
		}

		case MESSENGER_SUBHEADER_GC_LOGOUT:
		{
			...
			break;
		}
#else
		case MESSENGER_SUBHEADER_GC_UPDATE_STATUS:
		{
			TPacketGCMessengerUserStatus userInfo{};
			if (!Recv(sizeof(userInfo), &userInfo))
				return false;
			if (!Recv(userInfo.name_len, char_name))
				return false;
			char_name[userInfo.name_len] = '\0';
			CPythonMessenger::Instance().UpdateFriendStatus(char_name, userInfo.status);
			break;
		}
#endif
		...
	}
}

...