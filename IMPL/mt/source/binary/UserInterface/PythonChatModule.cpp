	...
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_GM",		CPythonChat::WHISPER_TYPE_GM);
#if __MOBICORE__
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_MOBILE",	CPythonChat::WHISPER_TYPE_MOBILE);
#endif

	...