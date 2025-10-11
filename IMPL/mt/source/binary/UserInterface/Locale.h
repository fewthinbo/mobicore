#if __MOBICORE__
#include <string>
#endif

...

#if __MOBICORE__
extern std::string ConvertUtf8ToCodePage(const std::string& utf8Input, UINT codePage = LocaleService_GetCodePage());
#endif
