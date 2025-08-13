#ifdef MOBICORE
HEADER_GG_MOBITEST = *****,
#endif

...

#pragma pack(1)

#ifdef MOBICORE
struct TMobiTest {
	uint8_t header = HEADER_GG_MOBITEST;
	uint8_t intensity{};
};
#endif
...
