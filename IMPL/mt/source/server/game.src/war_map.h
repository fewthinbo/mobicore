#if __MOBICORE__
namespace mobi_game {
	class GameClientBase;
}
#endif

class CWarMap
{
	...
	friend class CGuild;
#if __MOBICORE__
	friend class GameClientBase;
#endif
	...
}