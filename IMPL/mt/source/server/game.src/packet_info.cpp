CPacketInfoGG::CPacketInfoGG()
{
	...
	Set(...);
#if __MOBICORE__
	Set(HEADER_GG_MOBITEST, sizeof(TMobiTest), "MobiIntensityTest", false);
#endif
}