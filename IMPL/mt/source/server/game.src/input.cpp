void LoginFailure(LPDESC d, const char * c_pszStatus)
{
	if (!d)
		return;

#if __MOBICORE__
	if (d->is_mobile_request) return;
#endif

	...
}