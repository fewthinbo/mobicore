#include "consts.h"

namespace mobi_game {
	bool IsBoss(uint32_t id) {
		switch (id)
		{
			case 691: case 692: case 693: case 791: case 991: case 992: case 993: case 1091: 
			case 1092: case 1093: case 1094: case 1095: case 2191: case 1191: case 1192: 
			case 1304: case 1306: case 1307: case 1901: case 1902: case 1903: case 2206: 
			case 2207: case 2291: case 2306: case 2492: case 2493: case 2494: case 2598: 
			case 3090: case 3091: case 3191: case 3290: case 3291: case 3390: case 3391: 
			case 3490: case 3491: case 3691: case 3791: case 3790: case 3890: case 3891: 
			case 5001: case 5002: case 5004: case 5161: case 5162: case 5163: case 6091: 
			case 6191:
			return true;
		default:
			return false;
		}
	}
}