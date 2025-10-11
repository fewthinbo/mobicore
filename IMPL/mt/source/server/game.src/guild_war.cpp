#if __MOBICORE__
#include "mobi_client.h"
#endif

void CGuild::SetLadderPoint(int point)
{
	...
	m_data.ladder_point = point;
#if __MOBICORE__
	mobileInstance.sendLadderPoint(m_data.guild_id, m_data.ladder_point);
#endif
}