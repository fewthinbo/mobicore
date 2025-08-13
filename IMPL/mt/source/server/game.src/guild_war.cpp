#ifdef MOBICORE
#include "mobi_client.h"
#endif

void CGuild::SetLadderPoint(int point)
{
	...
	m_data.ladder_point = point;
#ifdef MOBICORE
	mobileInstance.sendLadderPoint(m_data.guild_id, m_data.ladder_point);
#endif
}