#ifdef MOBICORE
ACMD(do_mobitest);
#endif

...

#ifdef MOBICORE
{ "mobitest", do_mobitest, 0, 0, GM_IMPLEMENTOR },
#endif