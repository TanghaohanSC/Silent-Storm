// ParticleDynamics.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ParticleDynamics.h"
#include "MainFrm.h"

#include "ParticleDynamicsDoc.h"
#include "ParticleDynamicsView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CParticleDynamicsApp

BEGIN_MESSAGE_MAP(CParticleDynamicsApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CParticleDynamicsApp construction

CParticleDynamicsApp::CParticleDynamicsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CParticleDynamicsApp object

CParticleDynamicsApp theApp;

// CParticleDynamicsApp initialization

BOOL CParticleDynamicsApp::InitInstance()
{
	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CParticleDynamicsDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CParticleDynamicsView));
	AddDocTemplate(pDocTemplate);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}

BOOL CParticleDynamicsApp::OnIdle(LONG lCount)
{
	for ( POSITION pos = GetFirstDocTemplatePosition(); pos; )
	{
		CDocTemplate *pT = GetNextDocTemplate( pos );
		for ( POSITION p = pT->GetFirstDocPosition(); p; )
		{
			CDocument *pDoc = pT->GetNextDoc( p );
			ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CParticleDynamicsDoc)));
			CParticleDynamicsDoc *pD = (CParticleDynamicsDoc*)pDoc;
			pD->Step();
		}
	}
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CParticleDynamicsApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CParticleDynamicsApp message handlers

