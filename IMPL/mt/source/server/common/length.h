
enum EMisc
{
	...
#if !__MOBICORE__
	QUERY_MAX_LEN			= 8192,
#endif
	FILE_MAX_LEN			= 128,
}
enum EWhisperType
{
	...
	WHISPER_TYPE_GM			= 5,
#if __MOBICORE__
	WHISPER_TYPE_MOBILE		= 6,
#endif
	WHISPER_TYPE_SYSTEM		= 0xFF
};