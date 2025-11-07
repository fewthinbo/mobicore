#if __MOBICORE__
bool SECTREE_MANAGER::GetCoordsOfSafeArea(long map_idx, DWORD related_x, DWORD related_y, PIXEL_POSITION& result) const {
	int i = 0;

	do
	{
		long dx = related_x + aArroundCoords[i].x;
		long dy = related_y + aArroundCoords[i].y;

		LPSECTREE tree = SECTREE_MANAGER::instance().Get(map_idx, dx, dy);

		if (!tree)
			continue;

		if (tree->IsAttr(dx, dy, ATTR_BANPK))
		{
			result.x = dx;
			result.y = dy;
			return true;
		}
	} while (++i < ARROUND_COORD_MAX_NUM);

	return false;
}
#endif