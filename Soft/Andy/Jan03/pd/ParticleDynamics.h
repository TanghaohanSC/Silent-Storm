// ParticleDynamics.h : main header file for the ParticleDynamics application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CParticleDynamicsApp:
// See ParticleDynamics.cpp for the implementation of this class
//

class CParticleDynamicsApp : public CWinApp
{
public:
	CParticleDynamicsApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CParticleDynamicsApp theApp;