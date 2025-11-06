void DESC::Packet(const void * c_pvData, int iSize)
{
#if __MOBICORE__
	if (is_mobile_request) return;
#endif
	...
}