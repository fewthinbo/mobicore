#ifdef MOBICORE
#include "mobi_client.h"
#endif

void heartbeat(LPHEART ht, int pulse) 
{
	...
	CPVPManager::instance().Process();
#ifdef MOBICORE
	mobileInstance.Process();
#endif
	...
}

int main(int argc, char **argv)
{
	...

#ifdef MOBICORE
	mobi_game::GameClient mobile_client; // for mobile
#endif
	if (!start(argc, argv)) {
		CleanUpForEarlyExit();
		return 0;
	}

	...

#ifdef MOBICORE
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

#ifdef MOBICORE
	mobileInstance.Connect();
#endif

	...
}