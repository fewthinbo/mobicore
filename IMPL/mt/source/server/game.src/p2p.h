typedef struct _CCI
{
	...
#if __MOBICORE__
	bool is_mobile_request{ false };
#endif
	LPDESC	pkDesc;
} CCI;