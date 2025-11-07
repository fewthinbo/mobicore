class CHARACTER : public ...
{
	...
#if __MOBICORE__
	private:
		bool m_destroyed{ false };
	public:
		void GoToSafePosition();
#endif
};