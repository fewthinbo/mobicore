#ifdef MOBICORE
#include <string>
#endif

...

#ifdef MOBICORE
extern std::string ConvertUtf8ToCodePage(const std::string& utf8Input, UINT codePage = LocaleService_GetCodePage());
#endif
