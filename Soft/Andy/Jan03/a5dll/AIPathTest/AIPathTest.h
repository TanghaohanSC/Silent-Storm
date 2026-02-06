// AIPathTest.h : main header file for the AIPATHTEST application
//

#if !defined(AFX_AIPATHTEST_H__0237400C_FF13_4AAF_8938_B63B830C1A77__INCLUDED_)
#define AFX_AIPATHTEST_H__0237400C_FF13_4AAF_8938_B63B830C1A77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestApp:
// See AIPathTest.cpp for the implementation of this class
//

class CAIPathTestApp : public CWinApp
{
public:
	CAIPathTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAIPathTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CAIPathTestApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AIPATHTEST_H__0237400C_FF13_4AAF_8938_B63B830C1A77__INCLUDED_)
