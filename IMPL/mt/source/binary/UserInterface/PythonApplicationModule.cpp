void initapp()
{
	...

#if __MOBICORE__
	PyModule_AddIntConstant(poModule, "MOBICORE",	1);
#else	
	PyModule_AddIntConstant(poModule, "MOBICORE",	0);
#endif
	...
}