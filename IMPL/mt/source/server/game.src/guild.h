class CGuild
{
	...
#if !__MOBICORE__
		void		SetWarData(int iWin, int iDraw, int iLoss) { m_data.win = iWin, m_data.draw = iDraw, m_data.loss = iLoss; }
#else
		void		SetWarData(int iWin, int iDraw, int iLoss);
#endif
}