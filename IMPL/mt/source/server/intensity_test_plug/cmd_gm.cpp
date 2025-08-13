#ifdef MOBICORE
#include "mobi_client.h"
#endif

...


#ifdef MOBICORE
ACMD(do_mobitest) {
	int intensity = 0; //0 kapatir
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1 != '\0') {
		str_to_number(intensity, arg1);
	}
	else if (ch) {
		ch->ChatPacket(1, "You can use with intensity, otherwise test will be closed. Usage: /mobitest <intensity> (0-100)");
	}

	intensity = MINMAX(0, intensity, 100);

	mobileInstance.spamTest(intensity);

	TMobiTest spam_pack{};
	spam_pack.intensity = intensity;
	P2P_MANAGER::instance().Send(&spam_pack, sizeof(TMobiTest));
}
#endif