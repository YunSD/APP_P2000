#pragma once

#include <string>
#include <codecvt>
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

namespace StrUtil {
	const string toString(const wstring str);

	const wstring toWstring(const string str);

	const vector<string> explode(const string& s, const char& c);
}

