void CInstanceBase::__AttachEmpireEffect(DWORD eEmpire)
{
	...
	if (IsResource())
		return;
#if __MOBICORE__
	if (IsMobiDesc()) return;
#endif
	...
}