// AIPathTestDoc.cpp : implementation of the CAIPathTestDoc class
//

#include "stdafx.h"
#include "AIPathTest.h"

#include "AIPathTestDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestDoc

IMPLEMENT_DYNCREATE(CAIPathTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CAIPathTestDoc, CDocument)
	//{{AFX_MSG_MAP(CAIPathTestDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestDoc construction/destruction

CAIPathTestDoc::CAIPathTestDoc()
{
	// TODO: add one-time construction code here

}

CAIPathTestDoc::~CAIPathTestDoc()
{
}

static float GetRandom( float fMax ) { return rand() * fMax / RAND_MAX; }

BOOL CAIPathTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// add reinitialization code here
	// (SDI documents will reuse this document)
	world.polys.clear();
	for ( int i = 0; i < 120; i++ )
	{
		CPolygon p;
		switch ( rand() % 3 )
		{
			case 0:
				p.Add(0,0).Add(100,0).Add(100,100).Add(0,100);
				break;
			case 1:
				p.Add(0,0).Add(50,99).Add(100,0).Add(100,100).Add(0,100);
				break;
			default:
				p.Add(0,0).Add(50,70).Add(0,100);
				break;
		}
		p.Rotate( GetRandom( FP_2PI ) );
		p.Scale( 0.5f + GetRandom( 0.7f ) );
		p.Move( CVec2( 100, 100 ) + CVec2( GetRandom( 1000 ), GetRandom( 1000 ) ) );
		// add random polys
		world.AddPolygon( p );
		//p.Add(0,0).Add(100,0).Add(100,100).Add(0,100);
		//p.Move( CVec2( 100 + i * 5 + GetRandom( 2 ), 100 ) );
	}
	world.BuildGraph( CVec2( 1200, 1200 ), 200 );
	src = CVec2(0,0);
	dst = CVec2(0,0);
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CAIPathTestDoc serialization

void CAIPathTestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestDoc diagnostics

#ifdef _DEBUG
void CAIPathTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAIPathTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestDoc commands
void CAIPathTestDoc::UpdatePath()
{
	path.clear();
	if ( fabs2(dst) != 0 )
	{
		bool bRes = world.SearchPath( &path, src, dst );
		if ( !bRes )
			MessageBeep( MB_ICONEXCLAMATION );
	}
	UpdateAllViews(0);
}