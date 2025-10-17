#if __MOBICORE__
ACMD(do_mobitest);
#endif

...

#if __MOBICORE__
	{ "mobitest", do_mobitest, 0, 0, GM_IMPLEMENTOR },
#endif