// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2916862C_C371_48A0_B719_2F0DAF12155C__INCLUDED_)
#define AFX_STDAFX_H__2916862C_C371_48A0_B719_2F0DAF12155C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

//#define __STL_NO_SGI_IOSTREAMS
#define _NOTHREADS
#include "stl_user_config.h"
#include <stl/_config.h>

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define dbgnew new

#pragma warning(disable: 4786) // identifier was truncated in the debug information

#include <vector>
#include <hash_map>
#include <list>
#include <map>
#include <string>
using namespace std;

#include "..\misc\Tools.h"
#include "..\misc\basic1.h"
#include "..\misc\Geom.h"
#include "..\misc\2DArray.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2916862C_C371_48A0_B719_2F0DAF12155C__INCLUDED_)
