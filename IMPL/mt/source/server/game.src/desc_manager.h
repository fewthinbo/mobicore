class DESC_MANAGER : public singleton<DESC_MANAGER>
{
	...
	LPDESC			FindByCharacterName(const char* name);
#if __MOBICORE__
	LPDESC			FindByAccountID(uint32_t id);
#endif
}