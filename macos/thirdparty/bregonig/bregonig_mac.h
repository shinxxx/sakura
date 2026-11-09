/*! @file
	@brief bregonig 在 macOS 上编译用的兼容头文件（编译 bregonig 时用 -include 强制包含）

	bregonig（externals/bregonig）是 Windows 的 DLL，用 TCHAR（UNICODE 版是 16 位的 wchar_t）写成。
	Mac 版不改它的源码，而是在这里把 TCHAR 定义成 char16_t（与 Sakura 的 WCHAR 相同），
	并补上它用到的少量 Windows / CRT 函数。编译结果作为静态库链接进核心库，
	由 macos/platform/src/static_modules.cpp 以“bregonig.dll”的名字提供给 CDllImp。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#ifndef __cplusplus
#error "bregonig_mac.h is for the C++ sources of bregonig"
#endif

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// bregonig 的每个源文件要编译两次：UNICODE 版（导出 BMatchW 等 W 函数）和多字节版。
// DllMain、OnigSyntaxPerl_NG_EX、dec2oct、get_right_most_captured_num 只在多字节版里定义，
// 所以多字节版也要编译链接（定义 BREGONIG_MAC_ANSI 时为多字节版）。
#ifndef BREGONIG_MAC_ANSI
#define UNICODE 1
#define _UNICODE 1
#endif

// mem_vc6.h 是给 VC6 用的 new 处理设置。没有 _MSC_VER 时会被当成 VC6，所以不让它生效
#define MEM_VC6_H_
#define set_new_throw_bad_alloc()	/**/

typedef char16_t       WCHAR;
#ifdef UNICODE
typedef char16_t       TCHAR;
typedef char16_t       TBYTE;      // Windows 的 UNICODE 版中 TBYTE 就是 WCHAR
#else
typedef char           TCHAR;
typedef unsigned char  TBYTE;
#endif
typedef TCHAR*         LPTSTR;
typedef const TCHAR*   LPCTSTR;
typedef int            BOOL;
typedef std::uint32_t  DWORD;
typedef std::uint16_t  WORD;
typedef unsigned char  BYTE;
typedef unsigned int   UINT;
typedef std::intptr_t  INT_PTR;
typedef std::uintptr_t UINT_PTR;
typedef void*          LPVOID;
typedef void*          HINSTANCE;

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif
#define WINAPI
#define __cdecl
#define __declspec(x)

#define DLL_PROCESS_DETACH 0
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH  2
#define DLL_THREAD_DETACH  3

#ifdef UNICODE
#define _T(x) u##x
#define _TEXT(x) u##x
#else
#define _T(x) x
#define _TEXT(x) x
#endif

inline std::size_t _tcslen(const TCHAR* s)
{
	return std::char_traits<TCHAR>::length(s);
}

inline TCHAR* _tcschr(const TCHAR* s, int c)
{
	for (;; ++s) {
		if (*s == static_cast<TCHAR>(c)) return const_cast<TCHAR*>(s);
		if (!*s) return nullptr;
	}
}

namespace bregonig_mac {

//! 相当于 strtoul / strtol（bregonig 只用来读十进制和十六进制数字）
template<typename T>
T tcstoint(const TCHAR* s, TCHAR** end, int base)
{
	std::string narrow;
	const TCHAR* p = s;
	while (*p && *p < 0x80) narrow.push_back(static_cast<char>(*p++));
	char* e = nullptr;
	T value;
	if constexpr (std::is_signed_v<T>) value = static_cast<T>(std::strtol(narrow.c_str(), &e, base));
	else value = static_cast<T>(std::strtoul(narrow.c_str(), &e, base));
	if (end) *end = const_cast<TCHAR*>(s + (e - narrow.c_str()));
	return value;
}

//! 把格式字符串转成 char，用 vsnprintf 格式化，再把结果（ASCII）扩展成 TCHAR。
//! bregonig 只用 %hs（char 字符串）和数字，所以只需把 %hs 换成 %s
template<typename F>
int format(TCHAR* dst, std::size_t cch, const F* fmt, std::va_list args)
{
	std::string f;
	for (const F* p = fmt; *p; ++p) {
		if (p[0] == '%' && p[1] == 'h' && p[2] == 's') { f += "%s"; p += 2; continue; }
		f.push_back(static_cast<char>(*p));
	}
	char buf[512];
	const int n = std::vsnprintf(buf, sizeof(buf), f.c_str(), args);
	if (cch == 0) return n;
	std::size_t i = 0;
	for (; i + 1 < cch && buf[i]; ++i) dst[i] = static_cast<unsigned char>(buf[i]);
	dst[i] = 0;
	return n < 0 || static_cast<std::size_t>(n) >= cch ? -1 : n;
}

} // namespace bregonig_mac

inline unsigned long _tcstoul(const TCHAR* s, TCHAR** end, int base) { return bregonig_mac::tcstoint<unsigned long>(s, end, base); }
inline long _tcstol(const TCHAR* s, TCHAR** end, int base) { return bregonig_mac::tcstoint<long>(s, end, base); }

inline int _sntprintf(TCHAR* dst, std::size_t cch, const TCHAR* fmt, ...)
{
	std::va_list args;
	va_start(args, fmt);
	const int n = bregonig_mac::format(dst, cch, fmt, args);
	va_end(args);
	return n;
}

//! bregonig.h 的 asc2tcs 传入的格式是 L"%hs"（wchar_t）
inline int _snwprintf(TCHAR* dst, std::size_t cch, const wchar_t* fmt, ...)
{
	std::va_list args;
	va_start(args, fmt);
	const int n = bregonig_mac::format(dst, cch, fmt, args);
	va_end(args);
	return n;
}

//! 多字节版的 asc2tcs 用（复制并保证以 NUL 结尾）
inline TCHAR* lstrcpyn(TCHAR* dst, const TCHAR* src, int cch)
{
	if (cch <= 0) return dst;
	int i = 0;
	for (; i + 1 < cch && src[i]; ++i) dst[i] = src[i];
	dst[i] = 0;
	return dst;
}

//! dbgtrace.h 的调试输出（Mac 版丢弃。bregonig 不定义 _DEBUG 编译，所以 TRACE 宏为空）
inline int wvsprintf(TCHAR* dst, const TCHAR* fmt, std::va_list args) { return bregonig_mac::format(dst, 1024, fmt, args); }
inline void OutputDebugString(const TCHAR*) {}

//! UNICODE 版不使用（多字节版的双字节字符判断）
inline int _ismbblead(unsigned int) { return 0; }

//! btrans.cpp 把字符串字面量 _T("") 传给 newSVpv(TCHAR*, ...)。MSVC 允许，clang 不允许，所以补一个 const 版本
#ifdef UNICODE
#define BREGONIG_MAC_NS unicode
#else
#define BREGONIG_MAC_NS ansi
#endif
namespace BREGONIG_MAC_NS {
struct sv;
sv* newSVpv(TCHAR* s, std::ptrdiff_t len);
inline sv* newSVpv(const TCHAR* s, std::ptrdiff_t len) { return newSVpv(const_cast<TCHAR*>(s), len); }
} // namespace BREGONIG_MAC_NS

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
