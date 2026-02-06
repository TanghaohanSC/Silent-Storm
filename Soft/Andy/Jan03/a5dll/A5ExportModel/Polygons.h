#include "StdAfx.h"
#include "Data.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::pair<WORD, WORD> SEdge;
////////////////////////////////////////////////////////////////////////////////////////////////////
const CVec3& GetVertex( const CVec3 &v )
{
	return v;
}
const CVec3& GetVertex( const SVertex &v )
{
	return v.gv;
}
const CVec2& GetUV( const CVec3 &v )
{
	return VNULL2;
}
const CVec2& GetUV( const SVertex &v )
{
	return v.tv;
}
struct SDUDV
{
	CVec3 dU;
	CVec3 dV;
	bool operator==( const SDUDV &v ) const { return ( fabs2(dU - v.dU) < 1e-4f && fabs2(dV - v.dV) < 1e-4f ); }
	bool operator!=( const SDUDV &v ) const { return !operator==(v); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void SolveDuDv( const CVec3 &side1, const CVec3 &side2, float dU1, float dU2, float dV1, float dV2, SDUDV *pRes )
{
	float fA = dU1;
	float fB = dV1;
	float fC = dU2;
	float fD = dV2;
	float det = fA * fD - fB * fC;
	if ( fabs(det) < 1e-6f )
	{
		pRes->dU = VNULL3;
		pRes->dV = VNULL3;
		return;
	}
	float fAr = fD / det;
	float fBr = - fB / det;
	float fCr = - fC / det;
	float fDr = fA / det;
	pRes->dU = fAr * side1 + fBr * side2;
	pRes->dV = fCr * side1 + fDr * side2;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TVertex>
inline bool IsConvex( const vector<TVertex> &verts, const CVec3 &norm, SEdge edge, const list<WORD> &poly )
{
	// лежит ли ребро в той же плоскости, что и полигон ?
	CVec3 localN = ( GetVertex(verts[edge.first]) - GetVertex(verts[poly.front()]) ) ^
		( GetVertex(verts[edge.second]) - GetVertex(verts[poly.front()]) );
	Normalize( &localN );
	if ( fabs( fabs( localN * norm ) - 1.0f ) > 0.01f )
		return false;
	//
	SPlane plane;
	plane.Set( GetVertex(verts[edge.first]), GetVertex(verts[edge.second]), GetVertex(verts[edge.first]) + norm );
	int nPositive = 0;
	int nNegative = 0;
	
	for ( std::list<WORD>::const_iterator it = poly.begin(); it != poly.end(); ++it )
	{
		WORD ind = *it;
		if ( ind == edge.first || ind == edge.second )
			continue;
		if ( plane.GetDistanceToPoint( GetVertex(verts[ind]) ) >= 0 )
			++nPositive;
		else
			++nNegative;
	}
	return nPositive > 0 && nNegative > 0 ? false : true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TVertex>
bool ComposeNextPoly( const vector<TVertex> &verts, const vector<SFace> &srcTris,
	list<WORD> *pPoly, vector<bool> *pVisited, vector<SDUDV> *pDUDVs )
{
	SDUDV polyDUDV;
	pPoly->clear();
	const int srcSz = srcTris.size();
	int i;
	CVec3 norm;
	bool bInserted = false;
	
	// Находим затравочный треугольник для полигона
	for ( i = 0; i < srcSz; ++i )
	{
		if ( (*pVisited)[i] )
			continue;
		const SFace &tri = srcTris[i];
		pPoly->push_back( tri.i1 );
		pPoly->push_back( tri.i2 );
		pPoly->push_back( tri.i3 );
		if ( pDUDVs )
			polyDUDV = (*pDUDVs)[i];
		(*pVisited)[i] = true;
		norm = ( GetVertex(verts[tri.i2]) - GetVertex(verts[tri.i1]) ) ^
			( GetVertex(verts[tri.i3]) - GetVertex(verts[tri.i1]) );
		bInserted = true;
		break;
	}
	if ( !bInserted )
		return false; // нет необработанных треугольников
	Normalize( &norm );
	//
	while ( bInserted )
	{
		bInserted = false;
		// проходим все треугольники в списке и ищем смежные с нашим объединенным полигоном
		for ( i = 0; i < srcSz; ++i )
		{
			if ( (*pVisited)[i] )
				continue;
			if ( pDUDVs && (*pDUDVs)[i] != polyDUDV )
				continue;
			// проверяем все ребра
			const SFace &tri = srcTris[i];
			int nDUDV = 0;
			for ( std::list<WORD>::iterator it = pPoly->begin(); it != pPoly->end(); ++it, ++nDUDV )
			{
				std::list<WORD>::iterator it1 = it;
				++it1;
				CTPoint<WORD> edge( *it, pPoly->end() == it1 ? pPoly->front() : *it1 );
				// Ребро i1 - i3
				if ( tri.i1 == edge.x && tri.i3 == edge.y )
				{
					if ( !IsConvex( verts, norm, SEdge( tri.i1, tri.i2 ), *pPoly ) ||
					     !IsConvex( verts, norm, SEdge( tri.i2, tri.i3 ), *pPoly ) )
						continue;
					pPoly->insert( it1, tri.i2 );
					bInserted = true;
					break;
				}
				// Ребро i2 - i1
				if ( tri.i2 == edge.x && tri.i1 == edge.y )
				{
					if ( !IsConvex( verts, norm, SEdge( tri.i2, tri.i3 ), *pPoly ) ||
					     !IsConvex( verts, norm, SEdge( tri.i3, tri.i1 ), *pPoly ) )
						continue;
					pPoly->insert( it1, tri.i3 );
					bInserted = true;
					break;
				}
				// Ребро i3 - i2
				if ( tri.i3 == edge.x && tri.i2 == edge.y )
				{
					if ( !IsConvex( verts, norm, SEdge( tri.i1, tri.i2 ), *pPoly ) ||
					     !IsConvex( verts, norm, SEdge( tri.i3, tri.i1 ), *pPoly ) )
						continue;
					pPoly->insert( it1, tri.i1 );
					bInserted = true;
					break;
				}
			}
			if ( bInserted )
			{
				(*pVisited)[i] = true;
				break;  // удалось вставить новый треугольник, переходим на новую итерацию
			}
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TVertex>
void ConvertPolygons( const vector<TVertex> &verts, const vector<SFace> &srcTris,
	vector<WORD> *pIndices, vector<WORD> *pPolys, bool bCheckDuDv = false )
{
	vector<SDUDV> DUDVs;
	vector<SDUDV> *pDUDVs = 0;
	if ( bCheckDuDv )
	{
		pDUDVs = &DUDVs;
		DUDVs.resize( srcTris.size() );
		for ( int i = 0; i < srcTris.size(); ++i )
		{
			const SFace &tri = srcTris[i];
			SDUDV &dUdV = DUDVs[i];
			CVec3 side1 = GetVertex( verts[tri.i2] ) - GetVertex( verts[tri.i1] );
			CVec3 side2 = GetVertex( verts[tri.i3] ) - GetVertex( verts[tri.i1] );
			CVec2 uv1 = GetUV( verts[tri.i2] ) - GetUV( verts[tri.i1] );
			CVec2 uv2 = GetUV( verts[tri.i3] ) - GetUV( verts[tri.i1] );
			SolveDuDv( side1, side2, uv1.u, uv2.u, uv1.v, uv2.v, &dUdV );
		}
	}
	vector<bool> visited;
	visited.resize( srcTris.size() );
	for ( int i = 0; i < visited.size(); ++i )
		visited[i] = false;
	list<WORD> curPoly;
	WORD nIndex = 0;
	pPolys->push_back(0);
	while ( ComposeNextPoly( verts, srcTris, &curPoly, &visited, pDUDVs ) )
	{
		for ( std::list<WORD>::const_iterator i = curPoly.begin(); i != curPoly.end(); ++i )
		{
			pIndices->push_back( *i );
			++nIndex;
		}
		pPolys->push_back( nIndex );
		curPoly.clear();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TVertex>
void ConvertTriangles( const vector<TVertex> &verts, const vector<SFace> &srcTris,
	vector<WORD> *pIndices, vector<WORD> *pPolys )
{
	pIndices->resize( srcTris.size() * 3 );
	memcpy( &(*pIndices)[0], &srcTris[0], sizeof(SFace) * srcTris.size() );
	pPolys->resize( srcTris.size() + 1 );
	for ( int i = 0; i < srcTris.size() + 1; ++i )
		(*pPolys)[i] = i * 3;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
