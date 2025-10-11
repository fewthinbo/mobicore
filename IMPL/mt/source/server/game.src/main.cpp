#if __MOBICORE__
#include "mobi_client.h"
#endif

void heartbeat(LPHEART ht, int pulse) 
{
	...
	CPVPManager::instance().Process();
#if __MOBICORE__
	mobileInstance.Process();
#endif
	...
}

int main(int argc, char **argv)
{
	...

#if __MOBICORE__
	mobi_game::GameClient mobile_client; // for mobile
#endif
	if (!start(argc, argv)) {
		CleanUpForEarlyExit();
		return 0;
	}

	...

#if __MOBICORE__
	mobile_client.Disconnect(false);
#endif
	sys_log(0, "<shutdown> Destroying CArenaManager...");
	arena_manager.Destroy();

	...
}

int start(int argc, char **argv)
{
	...

	if (!g_bAuthServer) {
		db_clientdesc->UpdateChannelStatus(0, true);
	}

#if __MOBICORE__
	mobileInstance.Connect();
#endif

	...
}