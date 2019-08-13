#pragma once

#include <string>
#include <codecvt>

using namespace std;

// utf8תunicode
wstring utf8_to_wstring(const string &str);

// unicodeתutf8
string wstring_to_utf8(const wstring &str);

// gbkתunicode
wstring  gbk_to_wstring(const string &str);

// unicodeתgbk
string  wstring_to_gbk(const wstring &str);

// utf8תgbk
string utf8_to_gbk(const string &str);
// gbkתutf8
string gbk_to_utf8(const string &str);