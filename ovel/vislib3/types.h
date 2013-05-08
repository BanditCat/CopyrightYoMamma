#ifndef VISLIB_TYPES_H
#define VISLIB_TYPES_H

#include <limits.h>
#include <float.h>

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;
typedef float f32;
typedef double f64;

#if CHAR_BIT != 8 
#error Alien build envioronment, check types.h for correct typedefs.
#endif 
#if INT_MAX != 2147483647
#error Alien build envioronment, check types.h for correct typedefs.
#endif 
#if UINT_MAX != 4294967295 
#error Alien build envioronment, check types.h for correct typedefs.
#endif 
#if LLONG_MAX != 9223372036854775807
#error Alien build envioronment, check types.h for correct typedefs.
#endif 
#if ULLONG_MAX != 18446744073709551615
#error Alien build envioronment, check types.h for correct typedefs.
#endif 
#if SHRT_MAX != 32767 
#error Alien build envioronment, check types.h for correct typedefs.
#endif 
#if USHRT_MAX != 65535 
#error Alien build envioronment, check types.h for correct typedefs.
#endif 
#if SCHAR_MAX != 127 
#error Alien build envioronment, check types.h for correct typedefs.
#endif 
#if UCHAR_MAX != 255 
#error Alien build envioronment, check types.h for correct typedefs.
#endif 



#endif VISLIB_TYPES_H