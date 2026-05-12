// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _STLP_NO_THREADS
#include "stl_user_config.h"
#include <stl/_config.h>

/* // simplified windows.h :) 
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned char byte;
typedef const char *LPCSTR;
typedef int BOOL;
struct HWND__; typedef struct HWND__ *HWND;
typedef char CHAR;
typedef wchar_t WCHAR;
#define OutputDebugString OutputDebugStringA
extern "C" __declspec(dllimport) void __stdcall  OutputDebugStringA( const char * );
extern "C" __declspec(dllimport) DWORD __stdcall  GetTickCount();
*/
#define _WIN32_WINNT 0x400
#include <windows.h>
#include <stdio.h>
//#include <objbase.h>
//#include <assert.h>
#ifdef _DEBUG
// silent-storm-port: log ASSERTs instead of MessageBox + debugbreak (Phase 1.5).
#  define SS_ASSERT_LOG_AND_CONTINUE 1
#  ifdef FAST_DEBUG
#    define ASSERT( a ) if ( !(a) ) __debugbreak();
#  else
#    ifdef SS_ASSERT_LOG_AND_CONTINUE
#      define ASSERT( aParam ) do { if ( !(aParam) ) { FILE* _af = 0; fopen_s(&_af, "silent_storm_assert.log", "a"); if (_af) { fprintf(_af, "%s(%d) assertion %s failed\n", __FILE__, __LINE__, #aParam); fclose(_af); } } } while (0)
#    else
#      define ASSERT( aParam ) if ( !(aParam) ) { char szBuf[1024]; sprintf( szBuf, "%s(%d) assertion %s failed", __FILE__, __LINE__, #aParam ); MessageBox( 0, szBuf, "Error", MB_OK ); __debugbreak(); }
#    endif
#  endif
#else
#  define ASSERT( a ) ((void)0)
#endif
//_ASSERT( a )
//
#include <algorithm>
#include <list>
#include <string>
#include <vector>
#include <crtdbg.h>
#include <hash_map>
#include "..\Misc\basic2.h"
#include "..\Misc\tools.h"

using namespace std;
#include "Specific.h"
//
#define for if(false); else for
#define dbgnew new

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
