class CInstanceBase
{
	...
	enum{
		
		...
#if __MOBICORE__
			AFFECT_MOBILE,
#endif

			AFFECT_NUM = 64,

			...
	};


	...
		BOOL					IsGuildWall();
#if __MOBICORE__
		bool IsMobiDesc() const { return m_kAffectFlagContainer.IsSet(AFFECT_MOBILE); };
#endif
}