#if __MOBICORE__
#include <string>
#endif

...
unsigned	LocaleService_GetCodePage();
#if __MOBICORE__
extern std::string ConvertUtf8ToCodePage(const std::string& in, UINT codePage = LocaleService_GetCodePage());
#endif

...