// AIPathTestView.cpp : implementation of the CAIPathTestView class
//

#include "stdafx.h"
#include "AIPathTest.h"

#include "AIPathTestDoc.h"
#include "AIPathTestView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestView

IMPLEMENT_DYNCREATE(CAIPathTestView, CScrollView)

BEGIN_MESSAGE_MAP(CAIPathTestView, CScrollView)
	//{{AFX_MSG_MAP(CAIPathTestView)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestView construction/destruction

CAIPathTestView::CAIPathTestView()
{
	// TODO: add construction code here

}

CAIPathTestView::~CAIPathTestView()
{
}

BOOL CAIPathTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestView drawing
CPoint P( const CVec2 &p ) { return CPoint( p.x, p.y ); }
void CAIPathTestView::OnDraw(CDC* pDC)
{
	CAIPathTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CWorld &w = pDoc->world;
	{
		CPen p( PS_SOLID, 2, RGB(64,64,64) ), *pOld;
		pOld = pDC->SelectObject( &p );
		for ( vector<CPolygon>::iterator i = w.polys.begin(); i != w.polys.end(); ++i )
		{
			pDC->MoveTo( P( i->points.back() ) );
			for ( int k = 0; k < i->points.size(); ++k )
			{
				pDC->LineTo( P( i->points[k] ) );
			}
		}
		pDC->SelectObject( pOld );
	}
	/*{
		CPen p( PS_DASH, 1, RGB(0,0,0) ), *pOld;
		pOld = pDC->SelectObject( &p );
		for ( list<CWorld::SPoint>::iterator i = w.points.begin(); i != w.points.end(); ++i )
		{
			for ( vector<CWorld::SLink>::iterator k = i->links.begin(); k != i->links.end(); ++k )
			{
				pDC->MoveTo( P( i->ptPlace ) );
				pDC->LineTo( P( k->p->ptPlace ) );
			}
		}
	}*/
	if ( fabs2( pDoc->src ) > 0 )
	{
		CPen p( PS_SOLID, 3, RGB(0,128,0) ), *pOld;
		pOld = pDC->SelectObject( &p );
		pDC->Ellipse( pDoc->src.x - 3, pDoc->src.y - 3, pDoc->src.x + 3, pDoc->src.y + 3 );
		pDC->SelectObject( pOld );
	}
	if ( fabs2( pDoc->dst ) > 0 )
	{
		CPen p( PS_SOLID, 3, RGB(255,0,0) ), *pOld;
		pOld = pDC->SelectObject( &p );
		pDC->MoveTo( pDoc->dst.x - 5, pDoc->dst.y - 5 );
		pDC->LineTo( pDoc->dst.x + 5, pDoc->dst.y + 5 );
		pDC->MoveTo( pDoc->dst.x - 5, pDoc->dst.y + 5 );
		pDC->LineTo( pDoc->dst.x + 5, pDoc->dst.y - 5 );
		pDC->SelectObject( pOld );
	}
	if ( !pDoc->path.empty() )
	{
		CPen p( PS_SOLID, 3, RGB(0,0,128) ), *pOld;
		pOld = pDC->SelectObject( &p );
		pDC->MoveTo( P( pDoc->path.front() ) );
		for ( int i = 1; i < pDoc->path.size(); ++i )
			pDC->LineTo( P( pDoc->path[i] ) );
		pDC->SelectObject( pOld );
	}
}

void CAIPathTestView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 1200;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestView diagnostics

#ifdef _DEBUG
void CAIPathTestView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CAIPathTestView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CAIPathTestDoc* CAIPathTestView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAIPathTestDoc)));
	return (CAIPathTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAIPathTestView message handlers

void CAIPathTestView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CAIPathTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	//
	point += GetScrollPosition();
	if ( fabs2( pDoc->src ) == 0 || (nFlags & MK_CONTROL) )
	{
		pDoc->src = CVec2( point.x, point.y );
		pDoc->dst = CVec2(0,0);
	}
	else
		pDoc->dst = CVec2( point.x, point.y );
	//
	pDoc->UpdatePath();
		
	CScrollView::OnLButtonDown(nFlags, point);
}

void CAIPathTestView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CAIPathTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	//
	point += GetScrollPosition();
	pDoc->src = CVec2( point.x, point.y );
	pDoc->dst = CVec2(0,0);
	//
	pDoc->UpdatePath();

	CScrollView::OnRButtonDown(nFlags, point);
}
