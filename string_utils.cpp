#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <codecvt>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include "string_utils.h"


std::string wstringToString(std::wstring str) {
	std::string strLocale;
	const wchar_t* wchSrc = str.c_str();
	size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
	char* chDest = new char[nDestSize];
	memset(chDest, 0, nDestSize);
	wcstombs(chDest, wchSrc, nDestSize);
	std::string strResult = chDest;
	delete[] chDest;
	setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}

std::wstring stringToWstring(std::string str) {
	std::string strLocale;
	const char* chSrc = str.c_str();
	size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
	wchar_t* wchDest = new wchar_t[nDestSize];
	wmemset(wchDest, 0, nDestSize);
	mbstowcs(wchDest, chSrc, nDestSize);
	std::wstring wstrResult = wchDest;
	delete[] wchDest;
	setlocale(LC_ALL, strLocale.c_str());
	return wstrResult;
}