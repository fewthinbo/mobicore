#ifdef MOBICORE
#include "mobi_client.h"
#endif

void CGuild::AddMember(TPacketDGGuildMember * p)
{
	...
#ifdef MOBICORE
	mobileInstance.sendGuildJoin(p->dwGuild, p->dwPID);
#endif
	if (ch)
		LoginMember(ch);
	...
}

bool CGuild::RemoveMember(DWORD pid)
{
	...
#ifdef MOBICORE
	mobileInstance.sendGuildLeave(pid);
#endif
	return true;
}

#ifdef MOBICORE
void CGuild::SetWarData(int iWin, int iDraw, int iLoss) {
	m_data.win = iWin; 
	m_data.draw = iDraw; 
	m_data.loss = iLoss;

	mobileInstance.sendGuildStats(m_data.guild_id, iWin, iDraw, iLoss);
}
#endif