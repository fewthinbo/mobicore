void CClientManager::ProcessPackets(CPeer * peer)
{
	...
	while (...)
	{
		...

		switch (header)
		{
			case HEADER_GD_GUILD_CHANGE_MEMBER_DATA:
				...
				break;
#if __MOBICORE__
			case HEADER_GD_MOBI_LOGIN: {
				MobiLogin(data);
				break;
			}
			case HEADER_GD_MOBI_WARP: {
				MobiWarp(data);
				break;
			}
#endif
		}
	}
}

#if __MOBICORE__
static void SendMobiDGLogin(CPeer& target, CLoginData& login_data, const TMobiGD& info) {
	TMobiDGLogin res{};
	res.login_key = info.login_key;
	res.pid = info.pid;
	trim_and_lower(login_data.GetAccountRef().login, res.login, sizeof(res.login));

	sys_err("Data sent to channel(%d): login(%s), login_key(%d), pid(%d)",
		target.GetChannel(), res.login, res.login_key, res.pid);

	//Send directly to game
	target.EncodeHeader(HEADER_DG_MOBI_LOGIN, 0, sizeof(TMobiDGLogin));
	target.Encode(res);
}

void CClientManager::MobiLogin(const char* data) {
	auto* pack = (TMobiGD*)data;

	auto* login_data = GetLoginData(pack->login_key);
	if (!login_data) {
		sys_err("login data of pid(?) is nullptr", pack->pid);
		return;
	}

	auto found_peer = GetMostAvailCore();
	if (!found_peer) {
		sys_err("Most avail peer not found, login_key(%d)", pack->login_key);
		return;
	}

	SendMobiDGLogin(*found_peer, *login_data, *pack);
}

void CClientManager::MobiWarp(const char* data) {
	auto* pack = (TMobiGDWarp*)data;
	const auto& info = pack->info;

	auto* login_data = GetLoginData(info.login_key);
	if (!login_data) {
		sys_err("login data of pid(?) is nullptr", info.pid);
		return;
	}

	CPeer* target = nullptr;
	for (const auto& peer : m_peerList) {
		if (!peer) continue;
		if (peer->GetInetAddr() == pack->addr && 
			peer->GetListenPort() == pack->port) {
			target = peer;
			break;
		}
	}

	if (!target) {
		sys_err("Target peer doesn't exists: host(%s:%d)", pack->addr, pack->port);
		return;
	}
	SendMobiDGLogin(*target, *login_data, info);
}

using TPeerUserCount = decltype(std::declval<CPeer>().GetUserCount());
CPeer* CClientManager::GetMostAvailCore() const {
	if (m_peerList.empty()) {
		sys_log(0, "GetMostAvailCore: no peers available");
		return nullptr;
	}

	auto min_online = std::numeric_limits<TPeerUserCount>::max();

	CPeer* found = nullptr;
	for (const auto& peer : m_peerList) {
		if (!peer) continue;
		if (m_pkAuthPeer == peer) continue;

		BYTE channel = peer->GetChannel();
		if (channel == 0 || channel == 99) continue;

		TPeerUserCount current = peer->GetUserCount();
		if (current <= min_online) {
			min_online = current;
			found = peer;
		}
	}
	if (found) {
		sys_log(0, "GetMostAvailCore: Found channel(%d)", found->GetChannel());
	}
	return found;
}
#endif
