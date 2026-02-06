#include "StdAfx.h"
#include "FlatTest.h"
#include "geom.h"
/////////////////////////////////////////////////////////////////////////////////////
// CPolygon
/////////////////////////////////////////////////////////////////////////////////////
void CPolygon::Scale( float f )
{
	for ( vector<CVec2>::iterator i = points.begin(); i != points.end(); ++i )
		*i *= f;
}
/////////////////////////////////////////////////////////////////////////////////////
void CPolygon::Move( const CVec2 &move )
{
	for ( vector<CVec2>::iterator i = points.begin(); i != points.end(); ++i )
		*i += move;
}
/////////////////////////////////////////////////////////////////////////////////////
void CPolygon::Rotate( float f )
{
	float f11 = cos(f), f12 = sin(f);
	float f21 = -sin(f),f22 = cos(f);
	for ( vector<CVec2>::iterator i = points.begin(); i != points.end(); ++i )
	{
		float fx = i->x * f11 + i->y * f12;
		float fy = i->x * f21 + i->y * f22;
		i->x = fx;
		i->y = fy;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// trace single segment, problem - src on edge - additional tests are required
// speed is not optimal when working with bunch of points
/*
void TraceRay( const CWorld &w, vector<CEdge> *pRes, const CVec2 &src, const CVec2 &dst )
{
	pRes->clear();
	CVec2 ds( dst - src );
	float f11, f12, f21, f22;
	float fDet = 1 / ( sqr( ds.x ) + sqr( ds.y ) );
	f11 = ds.x * fDet; f12 = ds.y * fDet;
	f21 = -ds.y * fDet; f22 = ds.x * fDet;
	for ( vector<CPolygon>::const_iterator i = w.polys.begin(); i != w.polys.end(); ++i )
	{
		int nInside = 0;
		const CPolygon &p = *i;
		CVec2 eSrc, eDst = p.points.back();
		for ( int i = 0; i < p.points.size(); i++ )
		{
			eSrc = eDst;
			eDst = p.points[i];
			float a1x, a2x, a1y, a2y, fx, fy;
			fx = eSrc.x - src.x;
			fy = eSrc.y - src.y;
			a1x = f11 * fx + f12 * fy;
			a1y = f21 * fx + f22 * fy;
			fx = eDst.x - src.x;
			fy = eDst.y - src.y;
			a2x = f11 * fx + f12 * fy;
			a2y = f21 * fx + f22 * fy;
			float z1 = a2x * a1y - a1x * a2y;
			float z2 = a1y - a2y;
			float z3 = z1 - z2;
			bool T0 = ( ( FP_BITS( a1y ) ^ FP_BITS( a2y ) ) & 0x80000000 ) != 0;
			bool T1 = ( ( FP_BITS( z1 ) ^ FP_BITS( z2 ) ) & 0x80000000 ) == 0;
			bool T2 = ( ( FP_BITS( z3 ) ^ FP_BITS( z2 ) ) & 0x80000000 ) != 0;
			nInside += T0 & T1;
			if ( T0 & T1 & T2 )
			{
				// intersection exists
				pRes->push_back( CEdge( eSrc, eDst ) );
			}
		}
//		if ( nInside & 1 )
//			cout << "inside" << endl;
//		else
//			cout << "outside" << endl;
	}
}*/
/////////////////////////////////////////////////////////////////////////////////////
// CPointHorizon
/////////////////////////////////////////////////////////////////////////////////////
void CPointHorizon::AddTestPoint( const CVec2 &ptTest, void *pUserData )
{
	points.push_back();
	STestPoint &t = points.back();
	CVec2 ptDif = ptTest - ptCenter;
	t.fAngle = atan2( ptDif.y, ptDif.x );
	t.fDist = fabs( ptDif );
	t.ptTest = ptTest;
	t.pUserData = pUserData;
}
/////////////////////////////////////////////////////////////////////////////////////
void CPointHorizon::DoTrace( const CWorld &w )
{
	for ( vector<CPolygon>::const_iterator i = w.polys.begin(); i != w.polys.end(); ++i )
	{
		float fTotal = 0;           // atan2( f(t) - center ), f(t) - point on polygon border, t scalar [0,1]
		const CPolygon &p = *i;
		CVec2 eSrc, eDst = p.points.back();
		int i = 0;
		if ( eDst == ptCenter )
		{
			eDst = p.points[0];
			i = 1;
		}
		for ( ; i < p.points.size(); ++i )
		{
			eSrc = eDst;
			eDst = p.points[i];
			CVec2 dif1 = eSrc - ptCenter, dif2 = eDst - ptCenter;
			CVec2 ptEdgeDir = eDst - eSrc;
			// special case - test point is polygon vertex
			if ( dif2.x == 0 && dif2.y == 0 )
			{
				++i;
				eDst = p.points[ i % p.points.size() ];
				dif2 = eDst - ptCenter;
				//
				float fAngle1 = atan2( dif1.y, dif1.x );
				float fAngle2 = atan2( dif2.y, dif2.x );
				// it lies outside and covers everything in range, but comparisons are not strict
				//
				if ( fAngle2 > fAngle1 )
					fAngle1 += FP_2PI;
				//
				fTotal -= fabs( fAngle1 - fAngle2 );
				//
				for ( vector<STestPoint>::iterator k = points.begin(); k != points.end(); ++k )
				{
					STestPoint &t = *k;
					if ( ( t.fAngle > fAngle2 && t.fAngle < fAngle1 ) ||
						( t.fAngle + FP_2PI > fAngle2 && t.fAngle + FP_2PI < fAngle1 ) )
					{
						k->res.push_back( CEdge( eSrc, eDst ) );
					}
				}
				continue;
			}
			//
			float fAngle1 = atan2( dif1.y, dif1.x );
			float fAngle2 = atan2( dif2.y, dif2.x );
			// determine edge direction
			float fArea = dif2.x * dif1.y - dif2.y * dif1.x;
			if ( fArea > -0.001f * fabs( ptEdgeDir ) ) // 0.001f is small constant to handle point on edge cases
			{
				if ( fAngle2 > fAngle1 )
					fAngle1 += FP_2PI;
				//
				fTotal -= fabs( fAngle1 - fAngle2 );
				// positive edge direction -> it covers points
				// points in range [Angle2;Angle1] are covered
				for ( vector<STestPoint>::iterator k = points.begin(); k != points.end(); ++k )
				{
					STestPoint &t = *k;
					if ( ( t.fAngle > fAngle2 && t.fAngle <= fAngle1 ) ||
						( t.fAngle + FP_2PI > fAngle2 && t.fAngle + FP_2PI <= fAngle1 ) )
					{
						CVec2 ptTest = t.ptTest - eSrc;
						float fSide = ptEdgeDir.x * ptTest.y - ptEdgeDir.y * ptTest.x;
						if ( fSide > 0 )
							k->res.push_back( CEdge( eSrc, eDst ) );
					}
				}
			}
			else
			{
				if ( fAngle1 > fAngle2 )
					fAngle2 += FP_2PI;
				fTotal += fabs( fAngle1 - fAngle2 );
			}
		}
		if ( fabs( fTotal ) > 0.1f )
			nInsideCounter++;
		/*
		cout << "total = " << fTotal << endl;//fmod( fabs(fTotal) / FP_2PI, 1 ) << endl;
		if ( fabs( fTotal ) < 0.1f )
			cout << "outside" <<endl;
		else
			cout << "inside" <<endl;
			*/
	}
}
/////////////////////////////////////////////////////////////////////////////////////
/*std::ostream& operator<<( std::ostream &str, const CVec2 &s )
{
	str << "(" << s.x << "," << s.y << ")";
	return str;
}*/
/////////////////////////////////////////////////////////////////////////////////////
// CWorld
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::TestSymmetry()
{
	for ( list<SPoint>::iterator k = points.begin(); k != points.end(); ++k )
	{
		SPoint &p = *k;
		for ( vector<SLink>::iterator z = p.links.begin(); z != p.links.end(); ++z )
		{
			bool bFound = false;
			SPoint &sym = *z->p;
			for ( vector<SLink>::iterator s = sym.links.begin(); s != sym.links.end(); ++s )
			{
				if ( s->p == &p )
				{
					ASSERT( !bFound );
					bFound = true;
					ASSERT( s->fLeng == z->fLeng );
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::PerformTrace( CPointHorizon *pRes )
{
	for ( list<SPoint>::iterator z = points.begin(); z != points.end(); ++z )
	{
		if ( fabs2( pRes->ptCenter - z->ptPlace ) < fSearchRadius2 )
			pRes->AddTestPoint( z->ptPlace, &(*z) );
	}
	pRes->DoTrace( *this );
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::BuildGraph( const CVec2 &size, float fGridStep )
{
	points.clear();
	// fill in points
	for ( vector<CPolygon>::const_iterator i = polys.begin(); i != polys.end(); ++i )
	{
		const CPolygon &p = *i;
		for ( int k = 0; k < p.points.size(); ++k )
			points.push_back( p.points[k] );
	}
	// add up square grid points
	fSearchRadius2 = sqr( fGridStep ) * 2.001f;
	for ( int x = 0; x < size.x / fGridStep; x++ )
	{
		for ( int y = 0; y < size.y / fGridStep; y++ )
		{
			points.push_back( CVec2( x * fGridStep, y * fGridStep ) );
		}
	}
	// calc links (every pair right now)
	float fLinks = 0;
	for ( list<SPoint>::iterator i = points.begin(); i != points.end(); )
	{
		CPointHorizon h;
		h.SetCenter( i->ptPlace );
		PerformTrace( &h );
		if ( h.nInsideCounter )
			i = points.erase( i ); // point is inside polygons, not interesting
		else
		{
			for ( int c = 0; c < h.points.size(); ++c )
			{
				CPointHorizon::STestPoint &r = h.points[c];
				if ( r.res.empty() && r.pUserData != &(*i) )
					i->links.push_back( SLink( fabs( r.ptTest - h.ptCenter ), (SPoint*)r.pUserData ) );
			}
			fLinks += i->links.size(); // to be removed, counting average links count
			if ( i->links.empty() )
				i = points.erase( i );
			else
				++i;
			//ASSERT( !i->links.empty() );
			//l++k;
		}
	}
	fLinks /= points.size();
	TestSymmetry();
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::CheckTotal(SPoint *pNode )
{
	if ( pNode->fDest < 1e38f )
	{
		float fTotal = pNode->fDest + pNode->fEstimate;
		if ( fTotal < fBest )
		{
			fBest = fTotal;
			pBest = pNode;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
bool CWorld::SearchPath( vector<CVec2> *pRes, const CVec2 &src, const CVec2 &dst )
{
	int c;
	bool bOk;
	CPointHorizon h;
	list<SPoint>::iterator i;
	//
	pRes->clear();
	//
	for ( i = points.begin(); i != points.end(); ++i )
	{
		i->fEstimate = 1e38f;
		i->fDest = 1e38f;
		i->pPrev = 0;
	}
	// src nodes
	bOk = false;
	h.SetCenter( src );
	if ( fabs2( dst - src ) < fSearchRadius2 )
		h.AddTestPoint( dst, 0 );
	PerformTrace( &h );
	for ( c = 0; c < h.points.size(); ++c )
	{
		CPointHorizon::STestPoint &r = h.points[c];
		if ( r.res.empty() )
		{
			if ( r.pUserData == 0 )
			{
				// direct path exists
				pRes->push_back( src );
				pRes->push_back( dst );
				return true;
			}
			bOk = true;
			((SPoint*)r.pUserData)->fEstimate = fabs( r.ptTest - h.ptCenter );
		}
	}
	if ( !bOk )
		return false;
	//
	// reset best path info
	fBest = 1e38f;
	pBest = 0;
	//
	// dest nodes
	bOk = false;
	h.SetCenter( dst );
	PerformTrace( &h );
	for ( c = 0; c < h.points.size(); ++c )
	{
		CPointHorizon::STestPoint &r = h.points[c];
		if ( r.res.empty() )
		{
			bOk = true;
			SPoint *pNode = (SPoint*)r.pUserData;
			pNode->fDest = fabs( r.ptTest - h.ptCenter );
			CheckTotal( pNode );
		}
	}
	if ( !bOk )
		return false;
	// perform search (dumb)
	for(;;)
	{
		bool bCont = false;
		for ( i = points.begin(); i != points.end(); ++i )
		{
			if ( i->fEstimate < 1e38f )
			{
				for ( vector<SLink>::iterator z = i->links.begin(); z != i->links.end(); ++z )
				{
					float f = i->fEstimate + z->fLeng;
					if ( f < z->p->fEstimate )
					{
						bCont = true;
						z->p->fEstimate = f;
						z->p->pPrev = &(*i);
						CheckTotal( z->p );
					}
				}
			}
		}
		if ( !bCont )
			break;
	}
	// extract path
	if ( pBest )
	{
		while ( pBest )
		{
			pRes->insert( pRes->begin(), pBest->ptPlace );
			pBest = pBest->pPrev;
		}
		pRes->insert( pRes->begin(), src );
		pRes->push_back( dst );
		OptimizePath( pRes );
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::OptimizePath( vector<CVec2> *pRes )
{
	for ( int i = 0; i < pRes->size() - 2; )
	{
		CPointHorizon h;
		h.SetCenter( (*pRes)[i] );
		h.AddTestPoint( (*pRes)[i+2] );
		h.DoTrace( *this );
		if ( h.points[0].res.empty() )
		{
			pRes->erase( pRes->begin() + i + 1 );
		}
		else
			i++;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
/*
static void TestFlat()
{
	CWorld w;
	
	w.AddPolygon( CPolygon().Add(0,0).Add(50,99).Add(100,0).Add(100,100).Add(0,100) );

//	vector<CEdge> res;
//	TraceRay( w, &res, CVec2(1,1), CVec2(0,1) );
//	cout << "intersect edges:" << endl;
//	for ( int i = 0; i < res.size(); i++ )
//		cout << " src = " << res[i].src << ";  dest = " << res[i].dst << endl;

	CPointHorizon hor;
	hor.ptCenter = CVec2(-1000.1f, -1000.1f);
	hor.AddTestPoint( CVec2(101,101) );
	hor.AddTestPoint( CVec2(0.1f,0.1f) );
	hor.AddTestPoint( CVec2(1.5f, 0.5f) );
	hor.AddTestPoint( CVec2(0,0) );
	hor.AddTestPoint( CVec2(1,0) );
	hor.DoTrace( w );
	
//	cout << "Src point " << hor.ptCenter << endl;
//	for ( int i = 0; i < hor.points.size(); i++ )
//	{
//		CPointHorizon::STestPoint &t = hor.points[i];
//		cout << "Segment to point " << t.ptTest << " has ";
//		if ( t.res.empty() )
//			cout << "no intersections" << endl;
//		else
//		{
//			cout << "intersections:" << endl;
//			for ( int k = 0; k < t.res.size(); ++k )
//				cout << " src = " << t.res[k].src << ";  dest = " << t.res[k].dst << endl;
//		}
//	}
}*/
/////////////////////////////////////////////////////////////////////////////////////
