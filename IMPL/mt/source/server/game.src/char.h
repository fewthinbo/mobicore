class CHARACTER : public ...
{
	...
#if __MOBICORE__
	private:
		bool m_destroyed{ false };
	public:
		bool can_mobi_warp{ false };
		void GoToSafePosition();
#endif
};