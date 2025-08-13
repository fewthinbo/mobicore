#ifndef MOBICORE
void CPythonMessenger::OnFriendLogin(const char * c_szKey/*, const char * c_szName*/)
{
	...
}

void CPythonMessenger::OnFriendLogout(const char * c_szKey)
{
	...
}
#else
void CPythonMessenger::UpdateFriendStatus(const char * name, uint16_t status)
{
	m_FriendNameMap.insert(name);

	if (!m_poMessengerHandler) return;

	PyCallClassMemberFunc(m_poMessengerHandler, "OnStatusUpdate", Py_BuildValue("(isi)", MESSENGER_GRUOP_INDEX_FRIEND, name, status));
}
#endif



...
{

	...

#ifndef MOBICORE
	Py_InitModule("messenger", s_methods);
#else
	auto* poModule = Py_InitModule("messenger", s_methods);
	PyModule_AddIntConstant(poModule, "STATE_OFFLINE",		static_cast<int>(CPythonMessenger::EUserStatus::OFFLINE));
	PyModule_AddIntConstant(poModule, "STATE_IN_GAME",		static_cast<int>(CPythonMessenger::EUserStatus::IN_GAME));
	PyModule_AddIntConstant(poModule, "STATE_IN_MOBILE",		static_cast<int>(CPythonMessenger::EUserStatus::IN_MOBILE));
	PyModule_AddIntConstant(poModule, "STATE_IN_GAME_AND_MOBILE",		static_cast<int>(CPythonMessenger::EUserStatus::IN_GAME | CPythonMessenger::EUserStatus::IN_MOBILE));
#endif
}
