// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__99C19B29_7D10_46A8_9B06_01AEC226210E__INCLUDED_)
#define AFX_STDAFX_H__99C19B29_7D10_46A8_9B06_01AEC226210E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _STLP_NO_THREADS

// fake new.h
#include "FastPow2Alloc.h"

// normal stdafx.h
#include "stl_user_config.h"
#include <stl/_config.h>

#include <windows.h>
#include <objbase.h>
#include <assert.h>


#include <stdio.h>
#ifdef _DEBUG
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


#include <hash_map>
#endif // !defined(AFX_STDAFX_H__99C19B29_7D10_46A8_9B06_01AEC226210E__INCLUDED_)
