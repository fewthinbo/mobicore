#ifdef MOBICORE
#include <windows.h>
std::string ConvertUtf8ToCodePage(const std::string& utf8Input, UINT codePage) {
	int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Input.c_str(), -1, NULL, 0);
	if (wideLen <= 0) {
		return "";
	}

	std::wstring wideStr(wideLen, L'\0');
	if (MultiByteToWideChar(CP_UTF8, 0, utf8Input.c_str(), -1, &wideStr[0], wideLen) <= 0) {
		return "";
	}

	int ansiLen = WideCharToMultiByte(codePage, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
	if (ansiLen <= 0) {
		return "";
	}

	std::string ansiStr(ansiLen, '\0');
	if (WideCharToMultiByte(codePage, 0, wideStr.c_str(), -1, &ansiStr[0], ansiLen, NULL, NULL) <= 0) {
		return "";
	}

	if (!ansiStr.empty() && ansiStr.back() == '\0') {
		ansiStr.pop_back();
	}

	return ansiStr;
}
#endif
