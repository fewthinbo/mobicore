class CHARACTER : public ...
{
	...
#if __MOBICORE__
	private:
		bool m_destroyed{ false };
		bool can_mobi_warp{ false };
	public:
		void GoToSafePosition();
#endif
};