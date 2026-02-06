// SoundDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sound.h"
#include "SoundDlg.h"
#include "..\FModSound\FMSound.h"
#include "..\FileIO\Streams.h"

using namespace NFMSound;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

CSoundDlg::CSoundDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSoundDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSoundDlg, CDialog)
	//{{AFX_MSG_MAP(CSoundDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_2D, OnButtonPlay2d)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSoundDlg message handlers

CObj<CSample3D> s1,s2,s4;
CObj<CSample2D> s3;
const int INTERFACE_UPDATETIME = 30;
CObj<CSound3D> pSound;
SListener listener;

static void LoadFile( CMemoryStream *pDst, const char *pszFileName )
{
	CFileStream f;
	f.OpenRead( pszFileName );
	f.ReadTo( *pDst, f.GetSize() );
}

BOOL CSoundDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	NFMSound::SearchDevices();
	NFMSound::SStartInfo info;
	//info.pHWnd = &m_hWnd;
	info.nMaxChannels = 1;
	NFMSound::Init( info );
	CMemoryStream m;
	LoadFile( &m, "drumloop.wav" );
	s1 = LoadSample3D( m.GetBuffer(), m.GetSize(), 4, 1000 );
	LoadFile( &m, "jungle.wav" );
	s2 = LoadSample3D( m.GetBuffer(), m.GetSize(), 4, 1000 );
	//s2 = LoadSample( "jungle.wav", FSOUND_HW3D | FSOUND_LOOP_NORMAL );
	LoadFile( &m, "hit.wav" );
	s3 = LoadSample2D(  m.GetBuffer(), m.GetSize() );
	LoadFile( &m, "fireblast.wav" );
	s4 = LoadSample3D( m.GetBuffer(), m.GetSize() );
	pSound = Play3DSound( s1, CVec3(-10,0,0) );
//	Play3DSound(s2);
	SetTimer( 33, INTERFACE_UPDATETIME, 0 );
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSoundDlg::OnClose()
{
	CDialog::OnClose();
}

void CSoundDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSoundDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CSoundDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSoundDlg::OnButtonPlay()
{
	Play3DSound(s2);
}

void CSoundDlg::OnButtonPlay2d()
{
	Play3DSound(s4);
}

float t = 0;

void CSoundDlg::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 33 )
	{
		CVec3 position( ((float)sin(t*0.05f) * 33.0f), 0, 0 );
		CDynamicCast<ISound3D> p3D( pSound );
		p3D->SetPosition( position );
		NFMSound::Update( listener );
		t += (30 * (1.0f / (float)INTERFACE_UPDATETIME));
	}
	CDialog::OnTimer(nIDEvent);
}

BOOL CSoundDlg::DestroyWindow() 
{
	NFMSound::Done();
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::DestroyWindow();
}
