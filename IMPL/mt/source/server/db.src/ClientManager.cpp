void CClientManager::ProcessPackets(CPeer* peer)
{
	...
	while (...)
	{
		switch (header)
		{
			...

#if __MOBICORE__
		case HEADER_GD_MOBI_LOAD:
			MobiHandle(data);
			break;
#endif
		}
	}

}

#if __MOBICORE__
bool CClientManager::IsLoadingForMobi(DWORD handle_id) const {
	auto found = std::find_if(mobi_handlers_.begin(), mobi_handlers_.end(), [handle_id](const THandlerMobi& handler) {
		return handler.handle_id = handle_id;
	});
	return found != mobi_handlers_.end();
}

void CClientManager::MobiHandle(const char* data)
{
	auto now = std::chrono::steady_clock::now();

	auto it = mobi_handlers_.begin();
	while (it != mobi_handlers_.end()) {
		if (it->IsLoadTimeout(now)) {
			it = mobi_handlers_.erase(it);
			continue;
		}
		++it;
	}

	auto* pack = reinterpret_cast<const TGDHandleData*>(data);
	if (pack->is_remove) {
		mobi_handlers_.erase(pack->handle_id);
		return;
	}
	mobi_handlers_.emplace(pack->handle_id);
}
#endif
