	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_RING",				CInstanceBase::AFFECT_RAMADAN_RING);
	...
#if __MOBICORE__
	PyModule_AddIntConstant(poModule, "AFFECT_MOBILE",						CInstanceBase::AFFECT_MOBILE);
#endif