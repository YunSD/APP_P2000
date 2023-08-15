#define _CRT_SECURE_NO_WARNINGS

#include "StrUtil.h"



const string StrUtil::toString(wstring str) {
	string strLocale;
	const wchar_t* wchSrc = str.c_str();
	size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
	char* chDest = new char[nDestSize];
	memset(chDest, 0, nDestSize);
	wcstombs(chDest, wchSrc, nDestSize);
	string strResult = chDest;
	delete[] chDest;
	setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}

const wstring StrUtil::toWstring(string str) {
	string strLocale;
	const char* chSrc = str.c_str();
	size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
	wchar_t* wchDest = new wchar_t[nDestSize];
	wmemset(wchDest, 0, nDestSize);
	mbstowcs(wchDest, chSrc, nDestSize);
	wstring wstrResult = wchDest;
	delete[] wchDest;
	setlocale(LC_ALL, strLocale.c_str());
	return wstrResult;
}

const vector<string> StrUtil::explode(const string& s, const char& c)
{
	string buff{ "" };
	vector<string> v;

	for (auto n : s)
	{
		if (n != c) { buff += n; }
		else if (n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if (buff != "") v.push_back(buff);

	return v;
}