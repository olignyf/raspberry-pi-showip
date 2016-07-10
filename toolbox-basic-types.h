#ifndef __C_TOOLBOX_BASIC_TYPES_H__
#define __C_TOOLBOX_BASIC_TYPES_H__

#if defined(_MSC_VER)

#ifndef int64_t
typedef __int64 int64_t;
#endif

#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif

#ifndef mtime_t
typedef int64_t mtime_t;
#endif

#ifndef int32_t
typedef signed int int32_t;
#endif

#ifndef int32
typedef signed int int32;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef int16_t
typedef signed short int16_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef int8_t
typedef signed char int8_t;
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef fourcc_t
typedef uint32_t fourcc_t;
#endif

#define snprintf _snprintf
#define snwprintf _snwprintf
#define strncasecmp _strnicmp
#define wcsncasecmp _wcsnicmp
#define strcasecmp _stricmp
#define wcscasecmp _wcsicmp
// end visual studio

#else

// begin linux
#define DWORD unsigned long
#define LARGE_INTEGER int
#define MAX_PATH 260 // from windef.h

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#if defined (__alpha__)
#	if !defined(__GLIBC__)
typedef unsigned long long uint64_t;
typedef long long int64_t;
#	endif
#endif

typedef int64_t mtime_t;

#endif // end linux 

// if-else free section

#ifdef BIG_ENDIAN
#define toolbox_FOURCC( a, b, c, d ) \
        ( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )
#define toolbox_TWOCC( a, b ) \
        ( (uint16_t)(b) | ( (uint16_t)(a) << 8 ) )
#else
#define toolbox_FOURCC( a, b, c, d ) \
        ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#define toolbox_TWOCC( a, b ) \
        ( (uint16_t)(a) | ( (uint16_t)(b) << 8 ) )
#endif

// snprintf macro definitions BEGINS
#ifndef llx
#	if defined(_MSC_VER)
#		define I64d "%I64d"
#		define I64u "%I64u"
#		define I64x "%I64x"
#		define I64X "%I64X"
#		define lld "%I64d"
#		define llu "%I64u"
#		define llx "%I64x"
#		define llX "%I64X"
#		define I64dW L"%I64d"
#		define I64uW L"%I64u"
#		define I64xW L"%I64x"
#		define I64XW L"%I64X"
#		define lldW L"%I64d"
#		define lluW L"%I64u"
#		define llxW L"%I64x"
#		define llXW L"%I64X"
#	else
#		define I64d "%lld"
#		define I64u "%llu"
#		define I64x "%llx"
#		define I64X "%llX"
#		define lld "%lld"
#		define llu "%llu"
#		define llx "%llx"
#		define llX "%llX"
#	endif
#endif
// snprintf macro definitions ENDS

#endif // __C_TOOLBOX_BASIC_TYPES_H__
