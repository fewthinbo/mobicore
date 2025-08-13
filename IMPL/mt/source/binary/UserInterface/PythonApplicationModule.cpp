void initapp()
{
	...

#ifdef MOBICORE
	PyModule_AddIntConstant(poModule, "MOBICORE",	1);
#else	
	PyModule_AddIntConstant(poModule, "MOBICORE",	0);
#endif
	...
}