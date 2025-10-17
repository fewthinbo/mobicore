class CWarMap
{
	...

#if __MOBICORE__
public:
#endif
	typedef struct STeamData
	{
		...
	} TeamData;
#if __MOBICORE__
	private:
#endif
	...

	CHARACTER_SET m_set_pkChr;
#if __MOBICORE__
	public:
		const TeamData& GetTeamData(BYTE bIdx) const;
#if __FIGHTER_SCORE_SYNC__
		auto begin() const { return map_MemberStats.begin(); }
		auto end() const { return map_MemberStats.end(); }
		auto begin() { return map_MemberStats.begin(); }
		auto end() { return map_MemberStats.end(); }
#endif
#endif
	...
}