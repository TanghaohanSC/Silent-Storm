#include "StdAfx.h"
#include "GGeometry.h"
#include "Bound.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void FilterTrinagles( vector<STriangle> *pRes, const vector<WORD> &filter )
{
	int nTarget = 0;
	for ( int k = 0; k < pRes->size(); ++k )
	{
		const STriangle &src = (*pRes)[k];
		STriangle &res = (*pRes)[nTarget];
		res.i1 = filter[ src.i1 ];
		res.i2 = filter[ src.i2 ];
		res.i3 = filter[ src.i3 ];
		nTarget += (res.i1 != res.i2) & (res.i1 != res.i3) & (res.i2 != res.i3);
	}
	pRes->resize( nTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MergePositions( vector<WORD> *pMatches, vector<CVec3> *pPositions )
{
	vector<CVec3> mergedPositions;
	vector<CVec3> &positions = *pPositions;
	vector<WORD> &posIndices = *pMatches;
	posIndices.resize( positions.size() );
	mergedPositions.reserve( pPositions->size() );
	typedef hash_map<CVec3,int,SVec3Hash> CPosHash;
	CPosHash posHash;
	for ( int k = 0; k < positions.size(); ++k )
	{
		int nRes;
		CPosHash::iterator i = posHash.find( positions[k] );
		if ( i == posHash.end() )
		{
			nRes = mergedPositions.size();
			mergedPositions.push_back( positions[k] );
			posHash[ positions[k] ] = nRes;
		}
		else
			nRes = i->second;
		posIndices[k] = nRes;
	}
	positions = mergedPositions;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SEdgeInfo
{
	int nU, nV, nXSize, nYSize;
};
inline bool operator==( const SEdgeInfo &a, const SEdgeInfo &b ) { return memcmp( &a, &b, sizeof(a) ) == 0; }
struct SEdgeInfoHash
{
	int operator()( const SEdgeInfo &e ) const { return e.nU ^ e.nV ^ e.nXSize ^ e.nYSize; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_VX_CACHE_SIZE = 16;
const int N_VX_CACHE_SIZE_EFF = N_VX_CACHE_SIZE - 4;
class CVertexCacheOptimizer
{
public:
	typedef CObjectInfo::SLightmapInfo CQuad;
private:
	template<int N_SIZE>
	struct SCache
	{
		WORD nData[N_SIZE];
		int nPos;

		SCache() { memset( nData, 0, sizeof(nData) ); nPos = 0; }
		void Push( WORD n ) { nData[(nPos++)%N_SIZE] = n; }
		bool IsIn( WORD n ) const { for ( int k = 0; k < N_SIZE; ++k ) if ( nData[k] == n ) return true; return false; }
	};
	vector<vector<int> > quadPerVertex; // list of incident quad for each vertex
	vector<int> freeLinks; // number of free (not used) quads incident to this vertex
	vector<bool> isUsed; // true if quad already out
	SCache<N_VX_CACHE_SIZE_EFF> vxCache;

	void AddVertex( int nVertex, int nQuad )
	{
		if ( nVertex >= quadPerVertex.size() )
			quadPerVertex.resize( nVertex + 1 );
		quadPerVertex[nVertex].push_back( nQuad );
	}
	void MeasureEfficiency( const vector<CQuad> &quads );
	int SearchBest( const vector<CQuad> &quads );
public:
	void Optimize( vector<CQuad> *pQuads );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexCacheOptimizer::MeasureEfficiency( const vector<CQuad> &quads )
{
	SCache<N_VX_CACHE_SIZE> cache;
	int nMisses = 0, nTotal = 0;
	for ( int k = 0; k < quads.size(); ++k )
	{
		const CQuad &q = quads[k];
		if ( !cache.IsIn( q.n1 ) )
			++nMisses;
		cache.Push( q.n1 );
		if ( !cache.IsIn( q.n2 ) )
			++nMisses;
		cache.Push( q.n2 );
		if ( !cache.IsIn( q.n3 ) )
			++nMisses;
		cache.Push( q.n3 );
		nTotal += 3;
		if ( q.n4 != 0xffff )
		{
			if ( !cache.IsIn( q.n4 ) )
				++nMisses;
			cache.Push( q.n4 );
			++nTotal;
		}
	}
	char szBuf[1024];
	sprintf( szBuf, "vcache efficiency %g\n", 1.0f - nMisses / (float)(nTotal) );
	OutputDebugString( szBuf );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexCacheOptimizer::SearchBest( const vector<CQuad> &quads )
{
	int nBestIdx = -1, nBestCached = -1, nBestFreeLinks;
	for ( int k = 0; k < quads.size(); ++k )
	{
		if ( isUsed[k] )
			continue;
		int nCached = 0;
		const CQuad &q = quads[k];
		if ( vxCache.IsIn( q.n1 ) )
			++nCached;
		if ( vxCache.IsIn( q.n2 ) )
			++nCached;
		if ( vxCache.IsIn( q.n3 ) )
			++nCached;
		if ( q.n4 != 0xffff && vxCache.IsIn( q.n4 ) )
			++nCached;
		if ( nCached >= nBestCached )
		{
			if ( nCached > nBestCached )
				nBestFreeLinks = 0x7fffffff;
			nBestCached = nCached;
			int nFreeLinks = Max( Max( freeLinks[q.n1], freeLinks[q.n2] ), freeLinks[q.n3] );
			if ( q.n4 != 0xffff )
				nFreeLinks = Max( nFreeLinks, freeLinks[q.n4] );
			if ( nFreeLinks < nBestFreeLinks )
			{
				nBestFreeLinks = nFreeLinks;
				nBestIdx = k;
			}
		}
	}
	ASSERT( nBestIdx >= 0 );
	return nBestIdx;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexCacheOptimizer::Optimize( vector<CQuad> *pQuads )
{
	//MeasureEfficiency( *pQuads );
	// prepare some data structures
	for ( int k = 0; k < pQuads->size(); ++k )
	{
		const CQuad &q = (*pQuads)[k];
		AddVertex( q.n1, k );
		AddVertex( q.n2, k );
		AddVertex( q.n3, k );
		if ( q.n4 != 0xffff )
			AddVertex( q.n4, k );
	}
	freeLinks.resize( quadPerVertex.size() );
	for ( int k = 0; k < freeLinks.size(); ++k )
		freeLinks[k] = quadPerVertex[k].size();
	// form result
	vector<CQuad> res;
	isUsed.resize( pQuads->size(), false );
	for ( int k = 0; k < pQuads->size(); ++k )
	{
		// search for a best candidate
		int nBest = SearchBest( *pQuads );
		ASSERT( !isUsed[nBest] );
		const CQuad &q = (*pQuads)[nBest];
		--freeLinks[q.n1];
		--freeLinks[q.n2];
		--freeLinks[q.n3];
		if ( q.nOrder1 & 4 )
		{
			vxCache.Push( q.n1 );
			vxCache.Push( q.n3 );
			vxCache.Push( q.n2 );
		}
		else
		{
			vxCache.Push( q.n1 );
			vxCache.Push( q.n2 );
			vxCache.Push( q.n3 );
		}
		if ( q.n4 != 0xffff )
		{
			--freeLinks[q.n4];
			if ( q.nOrder2 & 4 )
			{
				vxCache.Push( q.n4 );
				vxCache.Push( q.n3 );
				vxCache.Push( q.n2 );
			}
			else
			{
				vxCache.Push( q.n4 );
				vxCache.Push( q.n2 );
				vxCache.Push( q.n3 );
			}
		}
		isUsed[nBest] = true;
		res.push_back( q );
	}
	//MeasureEfficiency( res );
	*pQuads = res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSquarePacker
{
	struct STriInfo
	{
		int n1, n2, n3, n4;
		bool bInverse, bInverse1;
		char nFirst, nFirst1;
		//CTPoint<int> vStart, vSize;
		int nSquare; // number of scheduled square inside SLevel
		int nShift;  // shift in texels inside square
		STriInfo() {}
		STriInfo( int _n1, int _n2, int _n3, bool _bInverse, char _nFirst )
			: n1(_n1), n2(_n2), n3(_n3), bInverse(_bInverse), nFirst(_nFirst), nFirst1(0), bInverse1(false) {}
	};
	struct SSquarePack
	{
		vector<STriInfo> squares;
	};
	struct SSquarePlace
	{
		int nSquare;
		int nShiftX, nShiftY;
	};
	struct SLevel
	{
		int nSquares; // total number of squares used on this level
		int nLastSquareFill;
		vector<SSquarePack> ysizes;
		vector<SSquarePlace> lowerSquares;
	};
	vector<SLevel> xsizes;
	CObjectInfo &info;
	float fTexelsPerMeter;
	typedef hash_map<SEdgeInfo, int, SEdgeInfoHash> CEdgesHash;
	CEdgesHash edgesInfo;

	void PushTriangle( int n1, int n2, int n3, char nFirst );
	void MapResult( int nLevel, const vector<SSquarePlace> &places );
	void CalcSquareOffsets();
	int GetTopXSize();
public:
	CSquarePacker( int _nMaxSize, CObjectInfo &_info, float _fTexelPerMeter )
		: info(_info), fTexelsPerMeter(_fTexelPerMeter)
	{
		int nMaxSize = GetMSB(_nMaxSize);
		xsizes.resize( nMaxSize + 1 );
		for ( int k = 0; k < xsizes.size(); ++k )
			xsizes[k].ysizes.resize( k + 1 );
	}
	void AddTriangle( int n1, int n2, int n3 );
	void Build( int nLMSize );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSquarePacker
////////////////////////////////////////////////////////////////////////////////////////////////////
// n1 - vertex with largest angle
void CSquarePacker::PushTriangle( int n1, int n2, int n3, char nFirst )
{
	const CVec3 &v1 = info.GetPositions()[ info.GetPositionIndices()[n1] ];
	const CVec3 &v2 = info.GetPositions()[ info.GetPositionIndices()[n2] ];
	const CVec3 &v3 = info.GetPositions()[ info.GetPositionIndices()[n3] ];
	// calc sizes
	float fSize21 = fabs( v2 - v1 );
	float fSize31 = fabs( v3 - v1 );
	// select largest size (&swap indices if needed)
	bool bInverse = false;
	if ( fSize31 > fSize21 )
	{
		swap( fSize21, fSize31 );
		swap( n2, n3 );
		bInverse = true;
	}
	// add to storage
	int nXSize = GetMSB( Max( 0, Float2Int( fSize21 * fTexelsPerMeter ) - 1 ) ) + 1;
	int nYSize = GetMSB( Max( 0, Float2Int( fSize31 * fTexelsPerMeter ) - 1 ) ) + 1;
	ASSERT( nYSize <= nXSize );
	nXSize = Min( nXSize, (int)( xsizes.size() - 1 ) );
	nYSize = Min( nYSize, nXSize );
	if ( nXSize == nYSize && n2 > n3 )
	{
		swap( n2, n3 );
		bInverse = !bInverse;
	}
	SEdgeInfo edge;
	edge.nU = n2;
	edge.nV = n3;
	edge.nXSize = nXSize;
	edge.nYSize = nYSize;
	CEdgesHash::iterator i = edgesInfo.find( edge );
	SSquarePack &dst = xsizes[nXSize].ysizes[nYSize];
	if ( i != edgesInfo.end() )
	{
		STriInfo &tri = dst.squares[i->second];
		if ( tri.n2 != n2 )
		{
			swap( n2, n3 );
			bInverse = !bInverse;
		}
		ASSERT( tri.n2 == n2 && tri.n3 == n3 );
		tri.n4 = n1;
		tri.nFirst1 = nFirst;
		tri.bInverse1 = bInverse;
		edgesInfo.erase( i );
		return;
	}
	if ( nXSize != nYSize )
		swap( edge.nU, edge.nV );
	edgesInfo[edge] = dst.squares.size();
	dst.squares.push_back( STriInfo( n1, n2, n3, bInverse, nFirst ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSquarePacker::AddTriangle( int n1, int n2, int n3 )
{
	// select largest angle
	const CVec3 &v1 = info.GetPositions()[ info.GetPositionIndices()[n1] ];
	const CVec3 &v2 = info.GetPositions()[ info.GetPositionIndices()[n2] ];
	const CVec3 &v3 = info.GetPositions()[ info.GetPositionIndices()[n3] ];
	float f1 = fabs2( v2 - v3 );
	float f2 = fabs2( v1 - v3 );
	float f3 = fabs2( v1 - v2 );
	if ( f2 > f1 )
	{
		if ( f2 > f3 )
			PushTriangle( n2, n3, n1, 2 );
		else
			PushTriangle( n3, n1, n2, 3 );
	}
	else
	{
		if ( f3 > f1 )
			PushTriangle( n3, n1, n2, 3 );
		else
			PushTriangle( n1, n2, n3, 1 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSquareFiller
{
	int nSquare, nSquareSize;
	int nShift;
	void Advance() { ++nSquare; nShift = 0; }
public:
	CSquareFiller( int _nSize ): nSquare(0), nSquareSize(_nSize), nShift(0) {}
	void Add( int n ) 
	{ 
		Alloc( n );
		nShift += n; 
		if ( nShift == nSquareSize )
			Advance();
	}
	void Alloc( int n )
	{
		if ( nShift + n > nSquareSize ) 
			Advance(); 
		ASSERT( n <= nSquareSize ); 
	}
	int GetSquare() const { return nSquare; }
	int GetShift() const { return nShift; }
};
void CSquarePacker::CalcSquareOffsets()
{
	// for every xsize calc number of full squares
	for ( int k = 0; k < xsizes.size(); ++k )
	{
		SLevel &level = xsizes[k];
		CSquareFiller filler( 1 << k );
		// map previous size squares
		if ( k > 0 )
		{
			SLevel &pl = xsizes[k - 1];
			level.lowerSquares.resize( pl.nSquares + 1 );
			int nLowerCubeSize = 1 << ( k -1 );
			for ( int i = 0; i < pl.nSquares; ++i )
			{
				int nShiftX = i & 1;
				SSquarePlace &s = level.lowerSquares[i];
				s.nSquare = filler.GetSquare();
				s.nShiftY = filler.GetShift();
				s.nShiftX = nShiftX * nLowerCubeSize;
				if ( nShiftX )
					filler.Add( nLowerCubeSize ); // working without Alloc, dangerous but suitable here
			}
			// map last square
			SSquarePlace &sLast = level.lowerSquares[ pl.nSquares ];
			if ( pl.nSquares & 1 )
			{
				sLast.nSquare = filler.GetSquare();
				sLast.nShiftY = filler.GetShift();
				sLast.nShiftX = nLowerCubeSize;
				filler.Add( nLowerCubeSize );
			}
			else
			{
				sLast.nSquare = filler.GetSquare();
				sLast.nShiftY = filler.GetShift();
				sLast.nShiftX = 0;
				filler.Add( pl.nLastSquareFill );
			}
		}
		// map own width squares
		for ( int i = 0; i < level.ysizes.size(); ++i )
		{
			SSquarePack &p = level.ysizes[i];
			int nDelta = 1 << i;
			for ( int z = 0; z < p.squares.size(); ++z )
			{
				filler.Alloc( nDelta );
				STriInfo &tri = p.squares[z];
				tri.nSquare = filler.GetSquare();
				tri.nShift = filler.GetShift();
				filler.Add( nDelta );
			}
		}
		level.nSquares = filler.GetSquare();
		level.nLastSquareFill = filler.GetShift();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CSquarePacker::GetTopXSize()
{
	for ( int k = xsizes.size() - 1; k >= 0; --k )
	{
		SLevel &level = xsizes[k];
		for ( int i = 0; i < level.ysizes.size(); ++i )
		{
			if ( !level.ysizes[i].squares.empty() )
				return k;
		}
	}
//	ASSERT( 0 ); // no output data exist
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSquarePacker::MapResult( int nLevel, const vector<SSquarePlace> &places )
{
	if ( nLevel < 0 )
		return;
	SLevel &l = xsizes[nLevel];
	// output triangles
	for ( int k = 0; k < l.ysizes.size(); ++k )
	{
		const SSquarePack &p = l.ysizes[k];
		for ( int i = 0; i < p.squares.size(); ++i )
		{
			const STriInfo &tri = p.squares[i];
			CObjectInfo::SLightmapInfo &res = *info.lmaps.insert( info.lmaps.end() );
			res.n1 = tri.n1;
			res.n2 = tri.n2;
			res.n3 = tri.n3;
			res.nOrder1 = tri.nFirst;
			if ( tri.bInverse )
				res.nOrder1 |= 4;
			++info.nTris;
			if ( tri.nFirst1 )
			{
				res.n4 = tri.n4;
				res.nOrder2 = tri.nFirst1;
				if ( tri.bInverse1 )
					res.nOrder2 |= 4;
				++info.nTris;
			}
			else
			{
				res.n4 = 0xffff;
				res.nOrder2 = 0;
			}
			res.nShiftX = places[tri.nSquare].nShiftX;
			res.nShiftY = places[tri.nSquare].nShiftY + tri.nShift;
			res.nXSize = nLevel;
			res.nYSize = k;
		}
	}
	// map lower squares
	vector<SSquarePlace> squarePlaces( l.lowerSquares.size() );
	for ( int k = 0; k < squarePlaces.size(); ++k )
	{
		SSquarePlace &d = squarePlaces[k];
		SSquarePlace &src = l.lowerSquares[k];
		const SSquarePlace &top = places[ src.nSquare ];
		d.nShiftX = src.nShiftX + top.nShiftX;
		d.nShiftY = src.nShiftY + top.nShiftY;
	}

	MapResult( nLevel - 1, squarePlaces );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int GetLog2( int n )
{
	if ( n <= 1 )
		return 0;
	return GetMSB( n - 1 ) + 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSquarePacker::Build( int nLMSize )
{
	int nMaxSize = xsizes.size();
	CalcSquareOffsets();

	// calc dimensions of required texture
	int nXSize = 1, nYSize = 1;
	int nTopXSize = GetTopXSize();
	int nSquaresNumber = xsizes[ nTopXSize ].nSquares;
	if ( nSquaresNumber == 0 )
	{
		if ( xsizes[ nTopXSize ].nLastSquareFill )
		{
			++nSquaresNumber;
			nXSize = 1 << nTopXSize;
			nYSize = 1 << GetLog2( xsizes[ nTopXSize ].nLastSquareFill );
			nYSize = nXSize;
		}
	}
	else
	{
		if ( xsizes[ nTopXSize ].nLastSquareFill )
			++nSquaresNumber;
		int nP2 = GetLog2( nSquaresNumber );
		nXSize = 1 << ( nTopXSize + ( nP2 - nP2 / 2 ) );
		nYSize = 1 << ( nTopXSize + nP2 / 2 );
	}
	int nLargestSize = 1 << ( nMaxSize - 1 );
	ASSERT( nLMSize >= nLargestSize );
	ASSERT( (nLMSize * nLMSize) >= nXSize * nYSize );
/*
	NStr::DebugTrace( "LM mapping stats\n" );
	static int nGlobalTrack = 0;
	int nTotal = 0;
	for ( int k = 0; k < xsizes.size(); ++k )
	{
		for ( int i = 0; i <= k; ++i )
		{
			NStr::DebugTrace( "%d ", xsizes[k].ysizes[i].squares.size() );
			nTotal += ( 1 << ( k + i ) ) * xsizes[k].ysizes[i].squares.size();
		}
		NStr::DebugTrace( "\n" );
	}
	nGlobalTrack += nXSize * nYSize;
	NStr::DebugTrace( "%d texels, %d buffer, %g used\n", 
		nTotal, nXSize * nYSize, (float)( ( nTotal * 100.0f ) / ( nXSize * nYSize ) )  );
	NStr::DebugTrace( "%d texels total\n", nGlobalTrack );
*/
	// place top level squares in it
	vector<SSquarePlace> squarePlaces( nSquaresNumber );
	int nShiftX = 0, nShiftY = 0;
	int nSquareSide = 1 << nTopXSize;
	for ( int i = 0; i < nSquaresNumber; ++i )
	{
		ASSERT( nShiftY < nYSize );
		SSquarePlace &s = squarePlaces[i];
		s.nShiftX = nShiftX;// + placeInfo.region.x1;
		s.nShiftY = nShiftY;// + placeInfo.region.y1;
		nShiftX += nSquareSide;
		if ( nShiftX == nXSize )
		{
			nShiftX = 0;
			nShiftY += nSquareSide;
			ASSERT( nShiftY < nYSize || i == nSquaresNumber - 1 );
		}
	}
	// map triangles to lightmap
	MapResult( nTopXSize, squarePlaces );
	info.lmSize.x = nXSize;
	info.lmSize.y = nYSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SPolygonIndices
////////////////////////////////////////////////////////////////////////////////////////////////////
void SPolygonIndices::GetTriangles( vector<STriangle> *pRes ) const
{
	pRes->resize( GetTrianglesCount() );
	if ( pRes->empty() )
		return;
	STriangle *pTri = &(*pRes)[0];

	int i1 = 0, i2 = 2;
	int nPoly = 1;
	int nPolys = polys.size();
	while ( nPoly < nPolys )
	{
		ASSERT( polys[nPoly] != polys[nPoly-1] );
		pTri->i1 = indices[ i1 ];
		pTri->i2 = indices[ i2 - 1 ];
		pTri->i3 = indices[ i2 ];
		++pTri;
		if ( ++i2 == polys[nPoly] )
		{
			i1 = i2;
			i2 = i1 + 2;
			++nPoly;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SPolygonIndices::SetTriangles( const vector<STriangle> &tris )
{
	int nTris = tris.size();

	indices.resize( nTris * 3 );
	polys.resize( nTris + 1 );
	polys[0] = 0;
	if ( nTris == 0 )
		return;
	int ind = -1;
	const STriangle *pTri = &tris[0];
	for ( int i = 0; i < nTris; ++i, ++pTri )
	{
		polys[i+1] = (i+1) * 3;
		indices[ ++ind ] = pTri->i1;
		indices[ ++ind ] = pTri->i2;
		indices[ ++ind ] = pTri->i3;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SPolygonIndices::AddTriangles( const vector<STriangle> &tris, int nBaseIndex )
{
	ASSERT( !polys.empty() );
	int nTris = tris.size();
	if ( nTris == 0 )
		return;
	int nBasePoly = polys.size();
	int ind = indices.size() - 1;
	polys.resize( nBasePoly + nTris );
	indices.resize( ind + 1 + nTris * 3 );
	const STriangle *pTri = &tris[0];
	for ( int i = 0; i < nTris; ++i, ++pTri )
	{
		indices[ ++ind ] = pTri->i1 + nBaseIndex;
		indices[ ++ind ] = pTri->i2 + nBaseIndex;
		indices[ ++ind ] = pTri->i3 + nBaseIndex;
		polys[ i + nBasePoly ] = ind + 1;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SPolygonIndices::Filter( const vector<WORD> &posIndices )
{
	vector<WORD> findices;
	vector<WORD> fpolys;
	fpolys.push_back(0);
	for ( int k = 1; k < polys.size(); ++k )
	{
		WORD nPrev = posIndices[ indices[ polys[k] - 1 ] ];
		for ( int i = polys[k-1]; i < polys[k]; ++i )
		{
			WORD nCurPos = posIndices[ indices[i] ];
			if ( nCurPos != nPrev )
				findices.push_back( indices[i] );
			nPrev = nCurPos;
		}
		if ( findices.size() - fpolys.back() < 3 )
			findices.resize( fpolys.back() );
		else
			fpolys.push_back( findices.size() );
	}
	indices = findices;
	polys = fpolys;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CObjectInfo 
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRefTracker
{
	vector<int> temp;

	void SetSize( int n )
	{
		temp.resize( n );
		for ( int k = 0; k < temp.size(); ++k )
			temp[k] = -1;
	}
	int GetRef( int nIndex, int nPos )
	{
		int &nRef = temp[ nIndex ];
		if ( nRef == -1 )
		{
			nRef = nPos;
			return nPos;
		}
		return nRef;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::EstablishRefs()
{
	SRefTracker rt, rtPos;
	rt.SetSize( positions.size() );
	vertRefPositions.resize( verts.size() );
	for ( int k = 0; k < vertRefPositions.size(); ++k )
		vertRefPositions[k] = rt.GetRef( posIndices[k], k );

	rt.SetSize( verts.size() );
	rtPos.SetSize( positions.size() );
	lmRefVertices.resize( lmaps.size() * 4 );
	lmRefPositions.resize( lmaps.size() * 4 );
	for ( int k = 0; k < lmaps.size(); ++k )
	{
		const SLightmapInfo &lm = lmaps[k];
		lmRefVertices[k*4 + 0] = rt.GetRef( lm.n1, k*4 + 0 );
		lmRefPositions[k*4 + 0] = rtPos.GetRef( posIndices[ lm.n1 ], k*4 + 0 );
		lmRefVertices[k*4 + 1] = rt.GetRef( lm.n2, k*4 + 1 );
		lmRefPositions[k*4 + 1] = rtPos.GetRef( posIndices[ lm.n2 ], k*4 + 1 );
		lmRefVertices[k*4 + 2] = rt.GetRef( lm.n3, k*4 + 2 );
		lmRefPositions[k*4 + 2] = rtPos.GetRef( posIndices[ lm.n3 ], k*4 + 2 );
		if ( lm.n4 != 0xffff )
		{
			lmRefVertices[k*4 + 3] = rt.GetRef( lm.n4, k*4 + 3 );
			lmRefPositions[k*4 + 3] = rtPos.GetRef( posIndices[ lm.n4 ], k*4 + 3 );
		}
		else
		{
			lmRefVertices[k*4 + 3] = 0;
			lmRefPositions[k*4 + 3] = 0;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SCompoundPosKey
{
	CVec3 v;
	SRealVertexWeight w;

	SCompoundPosKey() {}
	SCompoundPosKey( const CVec3 &_v, const SRealVertexWeight &_w ) : v(_v), w(_w) {}
	bool operator== ( const SCompoundPosKey &k ) const { return v == k.v && w == k.w; }
};
struct CalcCompoundKeyHash
{
	int operator()( const SCompoundPosKey &k ) const { SVec3Hash v; return v(k.v); }
};
void CObjectInfo::MergePositions()
{
	// merge positions
	if ( weights.size() != positions.size() )
	{
		NGScene::MergePositions( &posIndices, &positions );
		return;
	}
	vector<CVec3> mergedPositions;
	vector<SRealVertexWeight> mergedWeights;
	mergedPositions.reserve( positions.size() );
	mergedWeights.reserve( weights.size() );
	posIndices.resize( positions.size() );
	typedef hash_map<SCompoundPosKey,int,CalcCompoundKeyHash> CPosHash;
	CPosHash posHash;
	for ( int k = 0; k < positions.size(); ++k )
	{
		int nRes;
		SCompoundPosKey key( positions[k], weights[k] );
		CPosHash::iterator i = posHash.find( key );
		if ( i == posHash.end() )
		{
			nRes = mergedPositions.size();
			mergedPositions.push_back( positions[k] );
			mergedWeights.push_back( weights[k] );
			posHash[ key ] = nRes;
		}
		else
			nRes = i->second;
		posIndices[k] = nRes;
	}
	positions = mergedPositions;
	weights = mergedWeights;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::AssignGeometry( const SData &data )
{
	// initialize stuff
	positions.resize( data.verts.size() );
	verts.resize( data.verts.size() );
	weights.resize( data.weights.size() );
	for ( int k = 0; k < data.weights.size(); ++k )
	{
		SRealVertexWeight &res = weights[k];
		const SVertexWeight &src = data.weights[k];
		for ( int i = 0; i < 4; ++i )
		{
			res.fWeights[i] = src.fWeights[i];
			res.cBoneIndices[i] = src.cBoneIndices[i];
			ASSERT( src.fWeights[i] >= 0 && src.fWeights[i] <= 1 );
			res.nWeights[i] = Clamp( Float2Int( src.fWeights[i] * 256 ), 0, 255 );
		}
		// sort them, we are not in hurry :)
		for ( int i = 1; i < 4; ++i )
		{
			for ( int k = i - 1; k >= 0; --k )
			{
				if ( res.fWeights[k] < res.fWeights[k+1] )
				{
					swap( res.fWeights[k], res.fWeights[k+1] );
					swap( res.nWeights[k], res.nWeights[k+1] );
					swap( res.cBoneIndices[k], res.cBoneIndices[k+1] );
				}
				else
					break;
			}
		}
		ASSERT( res.fWeights[0] >= res.fWeights[1] && res.fWeights[1] >= res.fWeights[2] && res.fWeights[2] >= res.fWeights[3] );
	}
	for ( int k = 0; k < data.verts.size(); ++k )
	{
		const SVertex &v = data.verts[k];
		SUVInfo &res = verts[k];
		positions[k] = v.pos;
		res.tex.nU = Float2Int( v.tex.u * N_VERTEX_TEX_SIZE );
		res.tex.nV = Float2Int( v.tex.v * N_VERTEX_TEX_SIZE );
		NGfx::CalcCompactVector( &res.normal, v.normal );
		NGfx::CalcCompactVector( &res.texU, v.texU );
		NGfx::CalcCompactVector( &res.texV, v.texV );
	}
	geometry = data.geometry;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::Assign( const SData &data )
{
	AssignGeometry( data );

	MergePositions();
	//if ( geometry.polys.empty() )
	//	geometry.polys.push_back(0);
	geometry.Filter( posIndices );
	// calc lightmaps
	vector<STriangle> tris;
	/*data.*/geometry.GetTriangles( &tris );
	float fTexelPerMeter = NGlobal::GetVar( "gfx_lm_resolution", 5 ).GetFloat();
	CSquarePacker packer( N_MAX_LIGHTMAP_SIDE, *this, fTexelPerMeter);
	for ( int k = 0; k < tris.size(); ++k )
		packer.AddTriangle( tris[k].i1, tris[k].i2, tris[k].i3 );
	nTris = 0;
	packer.Build( N_LM_SIZE );

	CVertexCacheOptimizer vxOptimize;
	vxOptimize.Optimize( &lmaps );
	//
	EstablishRefs();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::AssignFast( const SData &data )
{
	AssignGeometry( data );
	lmSize = CTPoint<int>(1,1);
	nTris = geometry.GetTrianglesCount();
	posIndices.resize( verts.size() );
	for ( int k = 0; k < posIndices.size(); ++k )
		posIndices[k] = k;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::GetLMTriangles( vector<STriangle> *pRes ) const
{
	pRes->resize(0);
	for ( int k = 0; k < lmaps.size(); ++k )
	{
		int nOffset = k * 4;
		const SLightmapInfo &lm = lmaps[k];
		if ( lm.nOrder1 & 4 )
			pRes->push_back( STriangle( nOffset + 0, nOffset + 2, nOffset + 1 ) );
		else
			pRes->push_back( STriangle( nOffset + 0, nOffset + 1, nOffset + 2 ) );
		if ( lm.nOrder2 == 0 )
			continue;
		if ( lm.nOrder2 & 4 )
			pRes->push_back( STriangle( nOffset + 3, nOffset + 2, nOffset + 1 ) );
		else
			pRes->push_back( STriangle( nOffset + 3, nOffset + 1, nOffset + 2 ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::GetLMVerticesTriangles( vector<STriangle> *pRes ) const
{
	GetLMTriangles( pRes );
	FilterTrinagles( pRes, lmRefVertices );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::GetLMPositionTriangles( vector<STriangle> *pRes ) const
{
	GetLMTriangles( pRes );
	FilterTrinagles( pRes, lmRefPositions );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::GetVxVerticesTriangles( vector<STriangle> *pRes ) const
{
	if ( lmaps.empty() )
	{
		geometry.GetTriangles( pRes );
		return;
	}
	pRes->resize(0);
	for ( int k = 0; k < lmaps.size(); ++k )
	{
		const SLightmapInfo &lm = lmaps[k];
		if ( lm.nOrder1 & 4 )
			pRes->push_back( STriangle( lm.n1, lm.n3, lm.n2 ) );
		else
			pRes->push_back( STriangle( lm.n1, lm.n2, lm.n3 ) );
		if ( lm.nOrder2 == 0 )
			continue;
		if ( lm.nOrder2 & 4 )
			pRes->push_back( STriangle( lm.n4, lm.n3, lm.n2 ) );
		else
			pRes->push_back( STriangle( lm.n4, lm.n2, lm.n3 ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::GetVxPositionTriangles( vector<STriangle> *pRes ) const
{
	GetVxVerticesTriangles( pRes );
	if ( !vertRefPositions.empty() )
		FilterTrinagles( pRes, vertRefPositions );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::GetPosTriangles( vector<STriangle> *pRes ) const
{
	GetVxVerticesTriangles( pRes );
	if ( !posIndices.empty() )
		FilterTrinagles( pRes, posIndices );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::CalcBound( SBound *pRes )
{
	if ( verts.empty() )
		pRes->BoxInit( VNULL3, VNULL3 );
	else
		::CalcBound( pRes, positions, SGetSelf<CVec3>() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfo::CalcBound( SSphere *pRes )
{
	::CalcBound( pRes, positions, SGetSelf<CVec3>() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
START_REGISTER(GGeometry)
	REGISTER_VAR( "gfx_lm_resolution", 0, 5, true )
FINISH_REGISTER
