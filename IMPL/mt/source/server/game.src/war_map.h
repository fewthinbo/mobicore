#if __MOBICORE__
#include <memory>
#endif

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
		struct TMemberStats {
			DWORD dwGuildId;
			DWORD dwKills;
			DWORD dwDeaths;
			TMemberStats(DWORD gid, DWORD k, DWORD d) : dwGuildId(gid), dwKills(k), dwDeaths(d) {}
		};
		TMemberStats* RegisterMemberStats(CHARACTER* ch);
		TMemberStats* GetMemberStats(CHARACTER* ch);
	private:
		std::map<DWORD, std::unique_ptr<TMemberStats>> map_MemberStats;
	public:
		auto begin() const { return map_MemberStats.begin(); }
		auto end() const { return map_MemberStats.end(); }
		auto begin() { return map_MemberStats.begin(); }
		auto end() { return map_MemberStats.end(); }
		const TeamData& GetTeamData(BYTE bIdx) const;
#endif
	...
}