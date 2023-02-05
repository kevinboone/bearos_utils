/*=========================================================================

  bearcalc

  defs.h

  General handy definitions

  Copyright (c)2022 Kevin Boone, GPL v3.0 

=========================================================================*/

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

/* Note that the BearOS API has its own printf replacement. This _must_
   be used in this application, because the ARM Newlib-nano version of 
   printf does not process floating-point numbers. Unfortunately, the 
   ARM math library forces all the Newlib printf stuff to be included as
   well, which is a waste of memory. I do not know if there is any way
   to prevent this. */
#ifdef BEAROS
#define printf printf_
#define sprintf sprintf_
#define putchar _putchar
#endif


