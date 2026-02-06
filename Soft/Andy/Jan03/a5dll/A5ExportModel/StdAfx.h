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
#include "stl_user_config.h"
#include <stl/_config.h>

#include <windows.h>
//#include <objbase.h>
//#include <assert.h>
#ifdef _DEBUG
#  ifdef FAST_DEBUG
#    define ASSERT( a ) if ( !(a) ) __debugbreak();
#  else
#    define ASSERT( aParam ) if ( !(aParam) ) { char szBuf[1024]; sprintf( szBuf, "%s(%d) assertion %s failed", __FILE__, __LINE__, #aParam ); MessageBox( 0, szBuf, "Error", MB_OK ); __debugbreak(); }
#  endif
#else
#  define ASSERT( a ) ((void)0)
#endif
//
#include <algorithm>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <crtdbg.h>
#include <hash_map>
#include "basic2.h"
//
using std::vector;
using std::hash_map;
using std::string;
using std::list;
//
#define dbgnew new


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
