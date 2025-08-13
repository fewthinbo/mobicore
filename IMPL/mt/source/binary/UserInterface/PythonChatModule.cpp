	...
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_GM",		CPythonChat::WHISPER_TYPE_GM);
#ifdef MOBICORE
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_MOBILE",	CPythonChat::WHISPER_TYPE_MOBILE);
#endif

	...