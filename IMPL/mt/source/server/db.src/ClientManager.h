class CClientManager : public CNetBase, public singleton<CClientManager>
{
	...
	void		BlockChat(TPacketBlockChat * p);
#if __MOBICORE__
public:
	CPeer* GetMostAvailCore() const;
	void MobiLogin(const char* data);
	void MobiWarp(const char* data);
#endif
}