#pragma once

#include <stdint.h>

#ifndef false 
#define false 0
#endif
#ifndef true 
#define true 1
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE 
#define FALSE 0
#endif

#ifdef __cplusplus
#define BEGIN_DECLS extern "C" { 
#define END_DECLS }
#else
#define BEGIN_DECLS 
#define END_DECLS 
#endif

#ifndef BOOL
typedef uint8_t BOOL;
#endif

#ifndef bool 
typedef uint8_t bool;
#endif

#ifndef UTF8
typedef uint8_t UTF8;
#endif

#ifndef UTF32
typedef int32_t UTF32;
#endif

#ifndef BYTE 
typedef uint8_t BYTE;
#endif


