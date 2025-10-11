class CPythonChat : public ...
{
	public:
		enum EWhisperType
		{
			...
			WHISPER_TYPE_GM                 = 5,
#if __MOBICORE__
			WHISPER_TYPE_MOBILE             = 6,
#endif
			WHISPER_TYPE_SYSTEM             = 0xFF
		};

		...
};