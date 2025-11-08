class CClientManager : public CNetBase, public singleton<CClientManager>
{
	...
	void		BlockChat(TPacketBlockChat * p);
#if __MOBICORE__
public:
	CPeer* GetMostAvailCore() const;
	void MobiLogin(CPeer* pr, const char* data);
	void MobiWarp(CPeer* pr, const char* data);
#endif
}