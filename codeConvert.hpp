#pragma once

#include <string>
#include <codecvt>

using namespace std;

// utf8转unicode
wstring utf8_to_wstring(const string &str)
{
	wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt;
	return utf8_cvt.from_bytes(str);
}

// unicode转utf8
string wstring_to_utf8(const wstring &str)
{
	wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt;
	return utf8_cvt.to_bytes(str);
}

// gbk转unicode
wstring  gbk_to_wstring(const string &str)
{
	wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs"));
	return gbk_cvt.from_bytes(str);
}

// unicode转gbk
string  wstring_to_gbk(const wstring &str)
{
	wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs"));
	return gbk_cvt.to_bytes(str);
}

// utf8转gbk
string utf8_to_gbk(const string &str)
{
	wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt; // utf8-》unicode转换器
	wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs")); // unicode-》gbk转换器
	wstring t = utf8_cvt.from_bytes(str);
	return gbk_cvt.to_bytes(t);
}

// gbk转utf8
string gbk_to_utf8(const string &str)
{
	wstring_convert<codecvt_utf8<wchar_t>> utf8_cvt; // utf8-》unicode转换器
	wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs")); // unicode-》gbk转换器
	wstring t = gbk_cvt.from_bytes(str);
	return utf8_cvt.to_bytes(t);
}