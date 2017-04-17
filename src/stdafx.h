// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include "mytypes.h"

#include <sys/types.h>
#include <sys/stat.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _MSC_VER
#include <windows.h>
#define strdup _strdup
#endif

#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"

#include <MMSystem.h>

//#define GLEW_STATIC
#include <gl/glew.h>
#include <GL/wglew.h>
#include <gl/GL.h>
//#include <gl/GLU.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeImage.h>
#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <ctype.h>

#include <Commctrl.h>
#include <memory.h>
#include <time.h>
#include <crtdbg.h>

#include <tchar.h>
#include <random>

#define USE_NEWTON_PHYSICS
#define USE_NEWTON_PHYSICS_SDK

////////////////////////////////////////////
#ifdef USE_NEWTON_PHYSICS_SDK

typedef char dInt8;
typedef unsigned char dUnsigned8;

typedef short dInt16;
typedef unsigned short dUnsigned16;

typedef int dInt32;
typedef unsigned dUnsigned32;
typedef unsigned int dUnsigned32;

typedef long long unsigned64;

/* The guts header contains all the multiplication and addition macros that are defined for
 fixed or floating point complex numbers.  It also delares the kf_ internal functions.
 */

#include "fsaa.h"
#include "dirent.h"
#include "aopengl.h"
#include "colorshader.h"
#include "alist.h"
#include "parse.h"
#include "input.h"
#include "main.h"
#include "aimage.h"
#include "file.h"
#include "init.h"
#include "timers.h"
#include "game.h"
#include "quat.h"
#include "menu.h"
#include "draw.h"
#include "afont.h"
#include "console.h"

#undef M_PI
extern F32 const M_PI;

// BEGIN STUFF TO MAKE DIRECTX AUDIO API HAPPY
// The rationalization to do these exceptions is that we compile much faster
// with precompiled headers .. and it's only a few changes
// this is basically all that was used from DXUT.h so we just put it here
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

// All of this is a big headache for error checking in the sound routines
// so it's all just disabled for now
#undef DXTRACE_MSG
#undef DXTRACE_ERR
#undef DXTRACE_ERR_MSGBOX
#undef DXUT_ERR
#define DXUT_ERR(str,hr) (hr)
#define DXTRACE_MSG(str)              (0L)
#define DXTRACE_ERR(str,hr)           (hr)
#define DXTRACE_ERR_MSGBOX(str,hr)    (hr)

// The new SDK audio stuff insists on wide, and we compile non-wide, so we let them
// use their routines the way they want and continue to use ours without UNICODE
#undef mmioOpen
#define mmioOpen mmioOpenW
#undef FindResource
#define FindResource FindResourceW
// END STUFF TO MAKE DIRECTX AUDIO API HAPPY

#define USE_AHIGH_DEBUG_MALLOC 0

#if USE_AHIGH_DEBUG_MALLOC
#undef strdup
#define malloc myMalloc
#define free   myFree
#define calloc myCalloc
#define _strdup myStrdup
#define strdup myStrdup

void *myMalloc( size_t size );
void myFree( const void *ptr );
void *myCalloc( int size, int count );
char *myStrdup( const char *string );
#endif

#endif /* !USE_NEWTON_PHYSICS_SDK */

//#define glPushMatrix myPushMatrix
//#define glPopMatrix myPopMatrix
