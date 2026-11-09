/*! @file
	@brief Onigmo 的 config.h（macOS / Linux 用）

	Onigmo 在 Windows 上用 win32/config.h，在其他平台上本来由 configure 生成 config.h。
	Mac 版用 CMake 直接编译 Onigmo 的源码，所以在这里给出 configure 会生成的内容中用得到的部分。
	只支持 64 位（arm64 / x86_64），与 Mac 版的目标一致。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

#define STDC_HEADERS 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STRINGS_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_UNISTD_H 1
#define HAVE_ALLOCA_H 1
#define HAVE_ALLOCA 1
#define HAVE_PROTOTYPES 1
#define HAVE_STDARG_PROTOTYPES 1

#define SIZEOF_INT 4
#define SIZEOF_SHORT 2
#define SIZEOF_LONG 8
#define SIZEOF_LONG_LONG 8
#define SIZEOF_VOIDP 8

#if !defined(__LP64__)
#error "Sakura Editor for Mac supports 64-bit targets only"
#endif
