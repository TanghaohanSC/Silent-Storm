#include "stdafx.h"
#include "geom.h"
#include "tools.h"
#include <iostream>
/////////////////////////////////////////////////////////////////////////////////////
struct STriangle
{
	WORD i1, i2, i3;
	//
	STriangle() {}
	STriangle( WORD _i1, WORD _i2, WORD _i3 ): i1(_i1), i2(_i2), i3(_i3) {}
};
/////////////////////////////////////////////////////////////////////////////////////
class CRay
{
public:
	CVec3 ptOrig, ptDir;
};
/////////////////////////////////////////////////////////////////////////////////////
struct SEdge
{
	WORD wStart, wFinish;
	//
	SEdge() {}
	SEdge( WORD _wS, WORD _wF ): wStart(_wS), wFinish(_wF) {}
};
/////////////////////////////////////////////////////////////////////////////////////
class CEntity
{
public:
	vector<CVec3> points;
	vector<SEdge> edges;
	vector<STriangle> mesh;
};
/////////////////////////////////////////////////////////////////////////////////////
class CScene
{
public:
	// some volume data structure
	list< CEntity > entities;
};
/////////////////////////////////////////////////////////////////////////////////////
class CTracer
{
public:
	struct SCrossPoint
	{
		float fT;
		CVec3 ptNormal;
		//
		SCrossPoint( float _fT, const CVec3 &_ptNormal ): fT(_fT), ptNormal(_ptNormal) {}
	};
	struct SInterval
	{
		const CEntity &entity;
		SCrossPoint enter, exit;
		//
		SInterval( const CEntity &e, const SCrossPoint &_enter, const SCrossPoint &_exit )
			: entity(e), enter(_enter), exit(_exit) {}
	};
	vector<SInterval> intersections;
	//
	void Trace( const CScene &, const CRay& );
protected:
	struct SRefTriangle
	{
		const CVec3 &a1, &a2, &a3;
		//
		SRefTriangle( const CVec3 &_a1, const CVec3 &_a2, const CVec3 &_a3 ): a1(_a1), a2(_a2), a3(_a3) {}
	};
	CVec3 ptOrig, ptAxis1, ptAxis2, ptDir;
	//
	SRefTriangle GetTriangle( const CEntity &e, const STriangle &t );
	SCrossPoint CalcCross( const SRefTriangle &t );
	void InitProjection( const CRay &r );
	void TraceEntity( const CEntity &e );
};
/////////////////////////////////////////////////////////////////////////////////////
// CTracer
/////////////////////////////////////////////////////////////////////////////////////
void CTracer::InitProjection( const CRay &r )
{
	CVec3 v = r.ptDir;
	ASSERT( fabs2( v ) > 0 );
	if ( fabs(v.x) > fabs(v.y) && fabs(v.x) > fabs( v.z ) )
		ptAxis1 = v ^ CVec3(0,1,0); 
	else
		ptAxis1 = v ^ CVec3(1,0,0); 
	Normalize( &ptAxis1 );
	ptAxis2 = v ^ ptAxis1;
	Normalize( &ptAxis2 );
	ptOrig = r.ptOrig;
	ptDir = r.ptDir;
}
/////////////////////////////////////////////////////////////////////////////////////
void CTracer::Trace( const CScene &s, const CRay &r ) 
{
	InitProjection( r );
	for ( list<CEntity>::const_iterator i = s.entities.begin(); i != s.entities.end(); ++i )
	{
		TraceEntity( *i );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
static float CrossProduct( const CVec2 &a, const CVec2 &b )
{
	return a.x * b.y - a.y * b.x;
}
/////////////////////////////////////////////////////////////////////////////////////
CTracer::SRefTriangle CTracer::GetTriangle( const CEntity &e, const STriangle &t )
{
	int i1, i2, i3;
	if ( t.i1 & 0x8000 )
	{
		i1 = e.edges[ t.i1 & 0x7fff ].wFinish; 
		i2 = e.edges[ t.i1 & 0x7fff ].wStart;
	}
	else
	{
		i1 = e.edges[ t.i1 & 0x7fff ].wStart; 
		i2 = e.edges[ t.i1 & 0x7fff ].wFinish;
	}
	if ( t.i2 & 0x8000 )
		i3 = e.edges[ t.i2 & 0x7fff ].wStart;
	else
		i3 = e.edges[ t.i2 & 0x7fff ].wFinish;
	return SRefTriangle( e.points[i1], e.points[i2], e.points[i3] );
}
/////////////////////////////////////////////////////////////////////////////////////
static CVec3 GetPlanePoint( const CVec3 &a1, float f1, const CVec3 &a2, float f2 )
{
	return ( a2 * f1 - a1 * f2 ) / (f1 - f2);
}
CTracer::SCrossPoint CTracer::CalcCross( const SRefTriangle &t )
{
	const CVec3 &ptOnPlane = t.a1;
	CVec3 ptNormal = (t.a2-t.a1)^(t.a3-t.a1);
	Normalize( &ptNormal );
	float fDenominator = ( ptDir * ptNormal );
	if ( fDenominator == 0 )
	{
		// shit happened, calc distance to triangle in different way
		CVec3 ptSeparate = ptNormal ^ ptDir;
		float f0 = ptOrig * ptSeparate;
		float f1 = t.a1 * ptSeparate - f0;
		float f2 = t.a2 * ptSeparate - f0;
		float f3 = t.a3 * ptSeparate - f0;
		float fMinDistance = 1e30f, fD, fDirLeng = fabs( ptDir );
		ASSERT( fDirLeng > 0 );
		if ( f1 * f2 < 0 )
		{
			fD = GetPlanePoint( t.a1, f1, t.a2, f2 ) * ptDir;
			fMinDistance = Min( fD, fMinDistance );
		}
		if ( f2 * f3 < 0 )
		{
			fD = GetPlanePoint( t.a2, f2, t.a3, f3 ) * ptDir;
			fMinDistance = Min( fD, fMinDistance );
		}
		if ( f3 * f1 < 0 )
		{
			fD = GetPlanePoint( t.a3, f3, t.a1, f1 ) * ptDir;
			fMinDistance = Min( fD, fMinDistance );
		}
		if ( fMinDistance == 1e30f )
		{
			// мало того, что параллельный треугольник подсунули, 
			// так он еще и не пересекается
			ASSERT(0); 
			fMinDistance = t.a1 * ptDir;
		}
		fMinDistance = ( fMinDistance - ptOrig * ptDir ) / fDirLeng;
		return SCrossPoint( fMinDistance, ptNormal );
	}
	return SCrossPoint( ( (ptOnPlane - ptOrig) * ptNormal ) / fDenominator, ptNormal );
}
/////////////////////////////////////////////////////////////////////////////////////
static bool CmpCrosses( const CTracer::SCrossPoint &a, const CTracer::SCrossPoint &b )
{
	return a.fT < b.fT;
}
/////////////////////////////////////////////////////////////////////////////////////
static float fTraceSign[] = {1,-1};
void CTracer::TraceEntity( const CEntity &e )
{
	// transform vertices
	vector<CVec2> transformed;
	vector<float> edgeSide;
	//vector<char> ignore;
	vector<SCrossPoint> enter;
	vector<SCrossPoint> exit;
	//
	transformed.resize( e.points.size() );
	//ignore.resize( e.points.size() );
	//
	for ( int i = 0; i < e.points.size(); ++i )
	{
		const CVec3 src = e.points[i] - ptOrig;
		CVec2 &dst = transformed[i];
		dst.x = src * ptAxis1;
		dst.y = src * ptAxis2;
		if ( dst.x == 0 && dst.y == 0 )
			dst.x = -1e-6f; // handle degenerate case via converting it to non degenerate :)
	}
	// calc cross products for edges
	edgeSide.resize( e.edges.size() );
	for ( int i = 0; i < e.edges.size(); i++ )
	{
		edgeSide[i] = CrossProduct( transformed[ e.edges[i].wStart ], transformed[ e.edges[i].wFinish ] );
		if ( edgeSide[i] == 0 )
			edgeSide[i] = 1e-6f; // same as with points, somebody has to win ;)
	}
	// check tris
	for ( vector<STriangle>::const_iterator i = e.mesh.begin(); i != e.mesh.end(); ++i )
	{
		float f1 = edgeSide[ i->i1 & 0x7fff ] * fTraceSign[ i->i1>>15 ];
		float f2 = edgeSide[ i->i2 & 0x7fff ] * fTraceSign[ i->i2>>15 ];
		float f3 = edgeSide[ i->i3 & 0x7fff ] * fTraceSign[ i->i3>>15 ];
		if ( f1 > 0 && f2 > 0 && f3 > 0 )
			exit.push_back( CalcCross( GetTriangle( e, *i ) ) );
		if ( f1 < 0 && f2 < 0 && f3 < 0 )
			enter.push_back( CalcCross( GetTriangle( e, *i ) ) );
	}
	// fill intervals structure
	ASSERT( enter.size() == exit.size() );
	sort( enter.begin(), enter.end(), CmpCrosses );
	sort( exit.begin(), exit.end(), CmpCrosses );
	for ( int i = 0; i < Min( enter.size(), exit.size() ); ++i )
	{
		// due to cheating with degenerate cases and computation errors this might happen
		ASSERT( enter[i].fT <= exit[i].fT );
		intersections.push_back( SInterval( e, enter[i], exit[i] ) );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
static WORD InsertEdge( CEntity *pRes, WORD i1, WORD i2 )
{
	vector<SEdge> &edges = pRes->edges;
	for ( int i = 0; i < edges.size(); i++ )
	{
		if ( edges[i].wStart == i1 && edges[i].wFinish == i2 )
			return i;
		if ( edges[i].wStart == i2 && edges[i].wFinish == i1 )
			return i | 0x8000;
	}
	edges.push_back( SEdge( i1, i2 ) );
	return edges.size() - 1;
}
static void GenerateEdgeList( CEntity *pRes, const vector<STriangle> &tris )
{
	vector<STriangle> &rtris = pRes->mesh;
	for ( int i = 0; i < tris.size(); ++i )
	{
		const STriangle &t = tris[i];
		if ( t.i1 == t.i2 || t.i1 == t.i3 || t.i2 == t.i3 )
			continue;
		rtris.push_back( STriangle( 
			InsertEdge( pRes, t.i1, t.i2 ), 
			InsertEdge( pRes, t.i2, t.i3 ), 
			InsertEdge( pRes, t.i3, t.i1 ) ) );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
static void MakeCube( CEntity *pRes )
{
	CVec3 base(-1,-1,-1), size(2,2,2);
	vector<CVec3> &gpos = pRes->points;
	gpos.resize( 8 );
	gpos[0] = CVec3( base.x,          base.y,          base.z );
	gpos[1] = CVec3( base.x,          base.y + size.y, base.z );
	gpos[2] = CVec3( base.x + size.x, base.y + size.y, base.z );
	gpos[3] = CVec3( base.x + size.x, base.y         , base.z );
	gpos[4] = CVec3( base.x,          base.y,          base.z + size.z);
	gpos[5] = CVec3( base.x,          base.y + size.y, base.z + size.z );
	gpos[6] = CVec3( base.x + size.x, base.y + size.y, base.z + size.z );
	gpos[7] = CVec3( base.x + size.x, base.y         , base.z + size.z );
	//
	vector<STriangle> tris;
	tris.resize( 12 );
	tris[0] = STriangle( 0, 1, 2 );
	tris[1] = STriangle( 0, 2, 3 );
	tris[2] = STriangle( 3, 6, 7 );
	tris[3] = STriangle( 3, 2, 6 );
	tris[4] = STriangle( 0, 7, 4 );
	tris[5] = STriangle( 0, 3, 7 );
	tris[6] = STriangle( 5, 0, 4 );
	tris[7] = STriangle( 5, 1, 0 );
	tris[8] = STriangle( 6, 1, 5 );
	tris[9] = STriangle( 6, 2, 1 );
	tris[10] = STriangle( 4, 6, 5 );
	tris[11] = STriangle( 4, 7, 6 );
	//
	GenerateEdgeList( pRes, tris );
}
/////////////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<( std::ostream &str, const CVec3 &s )
{
	str << "(" << s.x << "," << s.y << "," << s.z << ")";
	return str;
}
/////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	CEntity e;
	CTracer trace;
	CScene scene;
	CRay ray;
	//
	MakeCube( &e );
	ray.ptOrig = CVec3( 1.00f, 1.00f, 0.98f );
	ray.ptDir = CVec3( 1, 0.0f, 0.0f );
	scene.entities.push_back( e );
	trace.Trace( scene, ray );
	//
	for ( int i = 0; i < trace.intersections.size(); ++i )
	{
		CTracer::SInterval &interval = trace.intersections[i];
		cout << "enter = " << interval.enter.fT << " normal = " << interval.enter.ptNormal << endl;
		cout << "exit = " << interval.exit.fT << " normal = " << interval.exit.ptNormal << endl;
	}
	return 0;
}
