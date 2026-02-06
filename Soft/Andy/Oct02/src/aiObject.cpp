#include "StdAfx.h"
#include "Bound.h"
#include "aiObject.h"
#include "BSPTree.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CEdgesInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CountEdge( vector<int> *pRes, int n )
{
	if ( n & 0x8000 )
		(*pRes)[n&0x7fff]--;
	else
		(*pRes)[n&0x7fff]++;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CEdgesInfo::IsClosed() const
{
	vector<int> count;
	count.resize( edges.size(), 0 );
	for ( int i = 0; i < mesh.size(); ++i )
	{
		const STriangle &t = mesh[i];
		CountEdge( &count, t.i1 );
		CountEdge( &count, t.i2 );
		CountEdge( &count, t.i3 );
	}
	bool bRes = true;
	for ( int k = 0; k < count.size(); ++k )
		bRes &= ( count[k] == 0 );
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CEdgesInfo::BuildTriangleList( vector<STriangle> *pRes ) const
{
	ASSERT( pRes != 0 );
	pRes->resize( mesh.size() );
	for ( int i = 0; i < mesh.size(); ++i )
	{
		WORD i1, i2, i3;
		const STriangle &t = mesh[i];
		if ( t.i1 & 0x8000 )
		{
			i1 = edges[ t.i1 & 0x7fff ].wFinish; 
			i2 = edges[ t.i1 & 0x7fff ].wStart;
		}
		else
		{
			i1 = edges[ t.i1 & 0x7fff ].wStart; 
			i2 = edges[ t.i1 & 0x7fff ].wFinish;
		}
		if ( t.i2 & 0x8000 )
			i3 = edges[ t.i2 & 0x7fff ].wStart;
		else
			i3 = edges[ t.i2 & 0x7fff ].wFinish;
		//
		(*pRes)[i] = STriangle( i1, i2, i3 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CEdgesInfo::InsertEdge( WORD i1, WORD i2, const vector<CVec3> &pts )
{
	for ( int i = 0; i < edges.size(); i++ )
	{
		if ( edges[i].wStart == i1 && edges[i].wFinish == i2 )
			return i;
		if ( edges[i].wStart == i2 && edges[i].wFinish == i1 )
			return i | 0x8000;
	}
	WORD f = 0;
	SEdge edge( i1, i2 );
	const CVec3 &p1 = pts[i1];
	const CVec3 &p2 = pts[i2];
	if ( p1.x > p2.x )
	{
		edge.wStart = i2;
		edge.wFinish = i1;
		f = 0x8000;
	}
	else if ( p1.x == p2.x )
	{
		if ( p1.y > p2.y )
		{
			edge.wStart = i2;
			edge.wFinish = i1;
			f = 0x8000;
		}
		else if ( p1.y == p2.y )
		{
			if ( p1.z > p2.z )
			{
				edge.wStart = i2;
				edge.wFinish = i1;
				f = 0x8000;
			}
//			else if ( p1.z == p2.z )
//			ASSERT(0);
		}
	}
	edges.push_back( edge );
	return (edges.size() - 1) | f;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CEdgesInfo::GenerateEdgeList( const vector<STriangle> &tris, const vector<CVec3> &pts )
{
	for ( int i = 0; i < tris.size(); ++i )
	{
		const STriangle &t = tris[i];
		if ( t.i1 == t.i2 || t.i1 == t.i3 || t.i2 == t.i3 )
			continue;
		mesh.push_back( STriangle( 
			InsertEdge( t.i1, t.i2, pts ), 
			InsertEdge( t.i2, t.i3, pts ), 
			InsertEdge( t.i3, t.i1, pts ) ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGeometryInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
CGeometryInfo::SPiece* CGeometryInfo::GetPiece( int nPieceID ) 
{ 
	CPieceMap::iterator i = pieces.find( nPieceID );
	if ( i == pieces.end() )
		return 0;
	return &i->second;
}
void CGeometryInfo::AddPiece( int nPieceID, const vector<CVec3> &_points, 
	const vector<STriangle> &_tris, float fVolume, vector<SJunction> juncs, bool _bClosed, CBSPTree* _pBSPTree )
{
	ASSERT( pieces.find( nPieceID ) == pieces.end() );
	if ( _tris.empty() )
		return;
	CGeometryInfo::SPiece &p = pieces[nPieceID];
	p.points = _points;
	p.edges.GenerateEdgeList( _tris, _points );
	p.edges.bClosed = _bClosed;
	p.fVolume = fVolume;
	p.juncs = juncs;
	p.pBSPTree = _pBSPTree;
	ASSERT( !_bClosed || p.edges.IsClosed() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGeometryInfo::CalcBound()
{
	SBoundCalcer b;
	for ( CPieceMap::const_iterator i = pieces.begin(); i != pieces.end(); ++i )
		b.LookSet( i->second.points, SGetSelf<CVec3>() );
	
	b.Make( &bound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGeometryInfo::CalcBSPTrees( bool bTerrain )
{
	CPieceMap::iterator i;
	for ( i = pieces.begin(); i != pieces.end(); ++i )
	{
		CGeometryInfo::SPiece &p = i->second;
		vector<STriangle> tris;
		p.edges.BuildTriangleList( &tris );
		if ( !bTerrain )
			p.pBSPTree = CreateBSPTree( p.points, tris );
		else
			p.pBSPTree = CreateTerrainBSPTree( p.points, tris );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*void MakeCube( CConvexHull *pRes, const CVec3 &base, const CVec3 &size )
{
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
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NAI;
