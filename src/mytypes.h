#ifndef __MYTYPES_H__
#define __MYTYPES_H__

// MACROS FOR CONSTANTS
#define C(x,y,z) const x y = z
#define CI(x,y) const int x = y
#define CU(x,y) const unsigned x = y
#define CF(x,y) const float x = y
#define CD(x,y) const double x = y

typedef unsigned int    U32;
typedef unsigned short  U16;
typedef int             S32;
typedef short           S16;
typedef float           F32;
typedef double          F64;
typedef unsigned char   U08;
typedef unsigned char   U8;
typedef signed   char   S08;
typedef signed   char   S8;

#if _MSC_VER < 1400
typedef unsigned __int64 U64;
typedef __int64          S64;
#else
typedef unsigned long long U64;
typedef signed long long   S64;
#endif

#endif
