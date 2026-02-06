// ParticleDynamicsView.cpp : implementation of the CParticleDynamicsView class
//

#include "stdafx.h"
#include "ParticleDynamics.h"

#include "ParticleDynamicsDoc.h"
#include "ParticleDynamicsView.h"
#include "Transform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CParticleDynamicsView

IMPLEMENT_DYNCREATE(CParticleDynamicsView, CView)

BEGIN_MESSAGE_MAP(CParticleDynamicsView, CView)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CParticleDynamicsView construction/destruction

CParticleDynamicsView::CParticleDynamicsView()
{
	fRot = 0;
}

CParticleDynamicsView::~CParticleDynamicsView()
{
}

BOOL CParticleDynamicsView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CParticleDynamicsView drawing
//static int Scale( int n ) { return n / 1024; }

static void Project( int *px, int *py, int *pRad, const CPos &p, const SHMatrix &m, const CSize &sz )
{
	CVec4 v;
	m.RotateHVector( &v, CVec4( p.x / 1024.0f, p.y / 1024.0f, p.z / 1024.0f, 1 ) );
	v.x /= v.w; v.y /= v.w;
	v.x = sz.cx * 0.5f + v.x * sz.cx * 0.5f;
	v.y = sz.cy * 0.5f - v.y * sz.cy * 0.5f;
	*px = Float2Int( v.x );
	*py = Float2Int( v.y );
	if ( pRad )
		*pRad = 10 * 300 / v.w;
}

void CParticleDynamicsView::OnDraw( CDC* pDC )
{
	CParticleDynamicsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CSize sz;
	RECT clr;
	pDC->GetWindow()->GetClientRect( &clr );
	sz.cx = clr.right - clr.left; sz.cy = clr.bottom - clr.top;
	CDC hdc;
	hdc.CreateCompatibleDC(pDC);
	CBitmap bmp, *pOldBMP;
	bmp.CreateCompatibleBitmap( pDC, sz.cx, sz.cy );
	pOldBMP = hdc.SelectObject( &bmp );
	CBrush b( RGB(0,120,0) );
	RECT r; r.left = 0; r.right = sz.cx; r.top = 0; r.bottom = sz.cy;
	hdc.FillRect( &r, &b );

	CTransformStack ts;
	SHMatrix cameraPos;
	CVec3 vDir(0,0,0);//-sqrt(2)/2);
	vDir.x = cos( fRot ) - sin( fRot );
	vDir.y = sin( fRot ) + cos( fRot );
	Normalize( &vDir );
	MakeMatrix( &cameraPos, CVec3 ( 0, 0, 300 ) - vDir * 400, vDir );
	ts.MakeProjective( 1 );
	ts.SetCamera( cameraPos );
	SHMatrix m = ts.Get().forward;
	for ( int i = 0; i < pDoc->cur.pos.size(); ++i )
	{
		RECT r;
		int x, y;
		int rad = 10;
		Project( &x, &y, &rad, pDoc->cur.pos[i], m, sz );
		r.left = x - rad; r.right = x + rad;
		r.bottom = y - rad; r.top = y + rad;
		hdc.Ellipse( &r );
	}
	CPen black( PS_SOLID, 1, RGB( 0,0,0 ) );
	CPen *pOldPen = hdc.SelectObject( &black );
	for ( int i = 0; i < pDoc->links.size(); ++i )
	{
		const SLink &l = pDoc->links[i];
		int n1 = l.n1, n2 = l.n2;
		int x, y;
		Project( &x, &y, 0, pDoc->cur.pos[n1], m, sz );
		hdc.MoveTo( x, y );
		Project( &x, &y, 0, pDoc->cur.pos[n2], m, sz );
		hdc.LineTo( x, y );
	}

	// draw center mass
	{
		RECT r;
		int x, y;
		int rad = 10;
		CVec3 v = pDoc->CalcMassCenter();
		CPos pos( v.x, v.y, v.z );
		Project( &x, &y, &rad, pos, m, sz );
		r.left = x - rad; r.right = x + rad;
		r.bottom = y - rad; r.top = y + rad;
		hdc.Ellipse( &r );
	}

	hdc.SelectObject( pOldPen );
	CString str;
	str.Format( "Venergy = %g", pDoc->CalcEnergy() / 1e3f );
	hdc.SetBkMode( TRANSPARENT );
	hdc.TextOut( 10, 10, str );
//	str.Format( "ShiftEnergy = %g", pDoc->nEnergy / 1e3f );
//	hdc.TextOut( 10, 25, str );
	if ( !pDC->BitBlt( 0, 0, sz.cx, sz.cy, &hdc, 0, 0, SRCCOPY ) )
		ASSERT(0);
	hdc.SelectObject( pOldBMP );
	//InvalidateRect( NULL );
}

void CParticleDynamicsView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ( nChar == VK_LEFT )
		fRot -= 0.1f;
	else if ( nChar == VK_RIGHT )
		fRot += 0.1f;
}

// CParticleDynamicsView diagnostics

#ifdef _DEBUG
void CParticleDynamicsView::AssertValid() const
{
	CView::AssertValid();
}

void CParticleDynamicsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CParticleDynamicsDoc* CParticleDynamicsView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CParticleDynamicsDoc)));
	return (CParticleDynamicsDoc*)m_pDocument;
}
#endif //_DEBUG


// CParticleDynamicsView message handlers
