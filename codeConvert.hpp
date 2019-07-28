#pragma once

#include <string>
#include <codecvt>

using namespace std;

// utf8תunicode
wstring utf8_to_wstring(const string &str)
{
	wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt;
	return utf8_cvt.from_bytes(str);
}

// unicodeתutf8
string wstring_to_utf8(const wstring &str)
{
	wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt;
	return utf8_cvt.to_bytes(str);
}

// gbkתunicode
wstring  gbk_to_wstring(const string &str)
{
	wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs"));
	return gbk_cvt.from_bytes(str);
}

// unicodeתgbk
string  wstring_to_gbk(const wstring &str)
{
	wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs"));
	return gbk_cvt.to_bytes(str);
}

// utf8תgbk
string utf8_to_gbk(const string &str)
{
	wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt; // utf8-��unicodeת����
	wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs")); // unicode-��gbkת����
	wstring t = utf8_cvt.from_bytes(str);
	return gbk_cvt.to_bytes(t);
}

// gbkתutf8
string gbk_to_utf8(const string &str)
{
	wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt; // utf8-��unicodeת����
	wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs")); // unicode-��gbkת����
	wstring t = gbk_cvt.from_bytes(str);
	return utf8_cvt.to_bytes(t);
}