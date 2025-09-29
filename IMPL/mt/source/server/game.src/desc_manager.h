class DESC_MANAGER : public singleton<DESC_MANAGER>
{
	...
	LPDESC			FindByCharacterName(const char* name);
#ifdef MOBICORE
	LPDESC			FindByAccountID(uint32_t id);
#endif
}