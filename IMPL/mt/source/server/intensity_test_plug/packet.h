#if __MOBICORE__
HEADER_GG_MOBITEST = *****,
#endif

...

#pragma pack(1)

#if __MOBICORE__
struct TMobiTest {
	uint8_t header = HEADER_GG_MOBITEST;
	uint8_t intensity{};
};
#endif
...
