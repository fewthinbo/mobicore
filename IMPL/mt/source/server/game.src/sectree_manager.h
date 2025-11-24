class SECTREE_MANAGER : public ...
{
		bool SaveAttributeToImage(...);
#if __MOBICORE__
		bool GetCoordsOfSafeArea(long map_idx, DWORD related_x, DWORD related_y, PIXEL_POSITION& result) const;
		bool IsAttackablePosition(long lMapIndex, long x, long y) const;
#endif
	...
}