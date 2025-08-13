#ifdef MOBICORE
namespace mobi_game {
	class GameClientBase;
}
#endif

class CWarMap
{
	...
	friend class CGuild;
#ifdef MOBICORE
	friend class GameClientBase;
#endif
	...
}