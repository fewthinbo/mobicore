#ifdef MOBICORE
#include "Locale.h"
#endif

bool CPythonNetworkStream::SendChatPacket(const char* c_szChat, BYTE byType)
{
	...
	ChatPacket.type = byType;
#ifdef MOBICORE
	ChatPacket.code_page = LocaleService_GetCodePage();
#endif
}

bool CPythonNetworkStream::SendWhisperPacket(const char* name, const char* c_szChat)
{
	...
	WhisperPacket.wSize = sizeof(WhisperPacket) + iTextLen;
#ifdef MOBICORE
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
#ifdef MOBICORE
	if (CPythonChat::WHISPER_TYPE_MOBILE == whisperPacket.bType)
	{
		std::string converted = ConvertUtf8ToCodePage(buf);
		if (converted.empty()) {
			TraceError("Utf8 conversion failed to code_page(%d)\n", LocaleService_GetCodePage());
			_snprintf(line, sizeof(line), "[M] %s : %s", whisperPacket.szNameFrom, buf);
		}
		else {
			_snprintf(line, sizeof(line), "[M] %s : %s", whisperPacket.szNameFrom, converted.c_str());
		}
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvMobileWhisper", Py_BuildValue("(iss)", whisperPacket.bType, whisperPacket.szNameFrom, line));
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
#ifndef MOBICORE
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
#ifndef MOBICORE
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