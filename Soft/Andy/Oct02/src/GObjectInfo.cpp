#include "StdAfx.h"
#include "DG.h"
#include "GGeometry.h"
#include "..\Misc\BasicShare.h"
#include "Grid.h"
#include "GScene.h"
#include "GObjectInfo.h"
#include "GPixelFormat.h"
#include "GFileSkin.h"
#include "aiObject.h"
#include "aiObjectLoader.h"

namespace NAI
{
	extern CBasicShare<int, CLoadGeometryInfo> shareAIModel;
	extern CBasicShare<int, CFileSkinPointsLoad> shareSkinPoints;
}
using namespace NBuilding;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
CBasicShare<SPartKey, CObjectInfoPiecesLoader, SPartHash> shareObjInfoPieces(122);
static vector<STriangle> trisBuf;
static SVertex zeroVertex;
////////////////////////////////////////////////////////////////////////////////////////////////////
// STriSort
////////////////////////////////////////////////////////////////////////////////////////////////////
STriSort::STriSort()
{
	pTris  = 0;
	pVerts = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void STriSort::CheckPlanes( int testTri, int coord, vector<WORD> &minPlane, vector<WORD> &maxPlane )
{
	ASSERT( pVerts && pTris );

	if ( minPlane.empty() )
	{
		minPlane.push_back( testTri );
		maxPlane.push_back( testTri );
		return;
	}
	//
	const vector<SLoadVertex> &verts  = *pVerts;
	const vector<STriangle> &tris = *pTris;
	//
	const float fTest  = verts[ tris[testTri].i1 ].pos[coord];
	const float curMax = verts[ tris[maxPlane.front()].i1 ].pos[coord];
	if ( fabs( fTest - curMax ) < FP_EPSILON )
		maxPlane.push_back( testTri );
	else if ( fTest > curMax )
	{
		maxPlane.clear();
		maxPlane.push_back( testTri );
		return;
	}
	//
	const float curMin = verts[ tris[minPlane.front()].i1 ].pos[coord];
	if ( fabs( fTest - curMin ) < FP_EPSILON )
		minPlane.push_back( testTri );
	else if ( fTest < curMin )
	{
		minPlane.clear();
		minPlane.push_back( testTri );
		return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PtMinMax( const CVec3 &pt, CVec3 *pMin, CVec3 *pMax )
{
	pMin->x = Min( pMin->x, pt.x );
	pMin->y = Min( pMin->y, pt.y );
	pMin->z = Min( pMin->z, pt.z );
	pMax->x = Max( pMax->x, pt.x );
	pMax->y = Max( pMax->y, pt.y );
	pMax->z = Max( pMax->z, pt.z );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void STriSort::Sort( const vector<SLoadVertex> &verts, const vector<STriangle> &tris )
{
	pVerts = &verts; 	// инициализаци€ переменных класса
	pTris = &tris;

	if ( verts.empty() )
		return;
	CVec3 ptMin = verts[0].pos;
	CVec3 ptMax = verts[0].pos;
	int i;
	//
	for ( i = 1; i < verts.size(); ++i )
	{
		PtMinMax( verts[i].pos, &ptMin, &ptMax );
	}
	//
	for ( i = 0; i < tris.size(); ++i )
	{
		EAlig al = GetAlignment( tris[i], verts );
		switch ( al )
		{
			case AYX:
				CheckPlanes( i, 2, tlist[BOTTOM], tlist[TOP] );
				break;
			case AYZ:
				CheckPlanes( i, 0, tlist[LEFT], tlist[RIGHT] );
				break;
			case AZX:
				CheckPlanes( i, 1, tlist[FRONT], tlist[BACK] );
				break;
		}
	}
	//
	const float F_MY_EPSILON = 1e-3f;
	if ( !tlist[LEFT].empty() && fabs( verts[tris[tlist[LEFT].front()].i1].pos.x - ptMin.x ) > F_MY_EPSILON )
		tlist[LEFT].clear();
	if ( !tlist[RIGHT].empty() && fabs( verts[tris[tlist[RIGHT].front()].i1].pos.x - ptMax.x ) > F_MY_EPSILON )
		tlist[RIGHT].clear();
	if ( !tlist[BOTTOM].empty() && fabs( verts[tris[tlist[BOTTOM].front()].i1].pos.z - ptMin.z ) > F_MY_EPSILON )
		tlist[BOTTOM].clear();
	if ( !tlist[TOP].empty() && fabs( verts[tris[tlist[TOP].front()].i1].pos.z - ptMax.z ) > F_MY_EPSILON )
		tlist[TOP].clear();
	if ( !tlist[FRONT].empty() && fabs( verts[tris[tlist[FRONT].front()].i1].pos.y - ptMin.y ) > F_MY_EPSILON )
		tlist[FRONT].clear();
	if ( !tlist[BACK].empty() && fabs( verts[tris[tlist[BACK].front()].i1].pos.y - ptMax.y ) > F_MY_EPSILON )
		tlist[BACK].clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline STriSort::EAlig STriSort::GetAlignment( const STriangle &tri, const vector<SLoadVertex> &verts )
{
	if ( fabs( verts[tri.i1].pos.x - verts[tri.i2].pos.x ) < FP_EPSILON && 
			 fabs( verts[tri.i2].pos.x - verts[tri.i3].pos.x ) < FP_EPSILON )
			 return AYZ;
	if ( fabs( verts[tri.i1].pos.y - verts[tri.i2].pos.y ) < FP_EPSILON && 
			 fabs( verts[tri.i2].pos.y - verts[tri.i3].pos.y ) < FP_EPSILON )
			 return AZX;
	if ( fabs( verts[tri.i1].pos.z - verts[tri.i2].pos.z ) < FP_EPSILON && 
			 fabs( verts[tri.i2].pos.z - verts[tri.i3].pos.z ) < FP_EPSILON )
			 return AYX;	
	return ANONE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void ConvertVertices( vector<SVertex> *pRes, const vector<SLoadVertex> &_src )
{
	pRes->resize( _src.size() );
	for ( int k = 0; k < _src.size(); ++k )
	{
		NGScene::SVertex &dst = (*pRes)[k];
		const SLoadVertex &src = _src[k];
		dst.pos = src.pos;
		dst.normal = src.normal;
		dst.tex = src.tex;
		dst.texU = src.texU;
		dst.texV = src.texV;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void Assign( CObjectInfo *pRes, const SFacesVector &src )
{
	CObjectInfo::SData objData;
	objData.geometry.polys = src.polys;
	objData.geometry.indices = src.indices;
	ConvertVertices( &objData.verts, src.verts );
	pRes->Assign( objData );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CObjectInfoLoader
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfoLoader::Recalc()
{
	vector<SLoadVertexWeight> wghts;
	vector<SLoadVertex> vertices;
	try
	{
		CResourceOpener file( "Geometries", GetKey() );
		
		pValue = new CObjectInfo;
		NGScene::CObjectInfo::SData objData;
		file->Add( 1, &vertices );
		file->Add( 2, &objData.geometry.indices );
		file->Add( 3, &objData.geometry.polys );
		file->Add( 5, &wghts );
		if ( !vertices.empty() )
			ConvertWeights( &objData.weights, wghts, vertices.size() );
		ConvertVertices( &objData.verts, vertices );
		pValue->Assign( objData );
	}
	catch(...)
	{
		OutputDebugString( "Exception: CObjectInfoLoader::Recalc()" );
		return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIGeometryConverter
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AddPiece( NGScene::CObjectInfo::SData *pObj, const NAI::CGeometryInfo::SPiece &p )
{
	NGScene::CObjectInfo::SData &objData = *pObj;
	p.edges.BuildTriangleList( &trisBuf );
	int nVerts = p.points.size(), nBase = objData.verts.size();
	objData.geometry.AddTriangles( trisBuf, nBase );
	objData.verts.resize( nBase + nVerts );
	for ( int k = 0; k < nVerts; ++k )
	{
		SVertex &dst = objData.verts[ k + nBase ];
		dst = zeroVertex;
		dst.pos = p.points[k];
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIGeometryConverter::Recalc()
{
	const NAI::CGeometryInfo *pAIGData = pAIGeom->GetValue();
	pValue = new CObjectInfo;
	if ( pAIGData == 0 )
		return;
	const NAI::CGeometryInfo &g = *pAIGData;
	NGScene::CObjectInfo::SData objData;

	vector<STriangle> tris;
	objData.geometry.SetTriangles( tris );
	if ( parts.empty() )
	{
		for ( NAI::CGeometryInfo::CPieceMap::const_iterator i = g.pieces.begin(); i != g.pieces.end(); ++i )
		{
			const NAI::CGeometryInfo::SPiece &p = i->second;
			AddPiece( &objData, p );
		}
	}
	else
	{
		for ( int k = 0; k < parts.size(); ++k )
		{
			NAI::CGeometryInfo::CPieceMap::const_iterator i = g.pieces.find( parts[k] );
			if ( i != g.pieces.end() )
				AddPiece( &objData, i->second );
		}
	}
	pValue->AssignFast( objData );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIGeometryConverter::SetKey( int _nID ) 
{
	pAIGeom = NAI::shareAIModel.Get( _nID );
	parts.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIGeometryConverter::CAIGeometryConverter( int _nID, const hash_map<int, bool> &_parts )
{
	pAIGeom = NAI::shareAIModel.Get( _nID );
	for ( hash_map<int, bool>::const_iterator i = _parts.begin(); i != _parts.end(); ++i )
	{
		if ( i->second )
			parts.push_back( i->first );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAISkinObjectConverter
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAISkinObjectConverter::Recalc()
{
	const NAI::CFileSkinPoints *pAIGData = pAIGeom->GetValue();
	pValue = new CObjectInfo;
	if ( pAIGData == 0 )
		return;
	const NAI::CFileSkinPoints &g = *pAIGData;
	NGScene::CObjectInfo::SData objData;

	vector<STriangle> tris;
	objData.geometry.SetTriangles( tris );
	for ( NAI::CFileSkinPoints::CBodypartsHash::const_iterator i = g.parts.begin(); i != g.parts.end(); ++i )
	{
		const NAI::CFileSkinPoints::SBodypart &p = i->second;
		ASSERT( p.weights.size() == p.points.size() );
		p.edges.BuildTriangleList( &trisBuf );
		int nVerts = p.points.size(), nBase = objData.verts.size();
		objData.geometry.AddTriangles( trisBuf, nBase );
		objData.verts.resize( nBase + nVerts );
		objData.weights.resize( nBase + nVerts );
		for ( int k = 0; k < nVerts; ++k )
		{
			SVertex &dst = objData.verts[ k + nBase ];
			dst = zeroVertex;
			dst.pos = p.points[k];
			objData.weights[ k + nBase ] = p.weights[k];
		}
	}
	pValue->AssignFast( objData );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAISkinObjectConverter::SetKey( int _nID )
{
	pAIGeom = NAI::shareSkinPoints.Get( _nID );
	Updated();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CObjectInfoPiecesLoader
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInfoPiecesLoader::Recalc()
{
	pValue = new CObjectInfoPieces;
	//
	try
	{
		SPartKey key = GetKey();
		CResourceOpener file( "Geometries", key );
		
		file->Add( 4, &pValue->faces );
	}
	catch(...)
	{
		return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CObjectInfoClipper
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWallObjectInfoClipper::SetKey( const SClipShare &key ) 
{
	clipInfo = key; 
	pSrc = shareObjInfoPieces.Get( clipInfo.src );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWallObjectInfoClipper::Recalc()
{
	pSrc.Refresh();
	if ( !IsValid( pSrc->GetValue() ) || pSrc->GetValue()->faces.empty() )
	{
		ASSERT( 0 );
		return;
	}
	pValue = new CObjectInfo;
	ClipWall();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void Clip( const SPlane &plane, SFacesVector *pDst, const SFacesVector &src );
////////////////////////////////////////////////////////////////////////////////////////////////////
static void ClipToPlane( const vector<SLoadVertex> &srcVerts, 
								vector<SLoadVertex> *pDstVerts, 
								CIntMap &old2new, 
								const SPlane &plane, 
								const list<WORD> &polyIn, 
								list<WORD> *pPolyOut );
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AddOldTris( SFacesVector *pDst, const SFacesVector &src );
////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void GetHideWalls( short nClip, vector<bool> *pHideSides )
{
	ASSERT( ESIZE == pHideSides->size() );
	const int iLEFT   = 0; // индексы сторон в nClip
	const int iRIGHT  = 1;
	const int iTOP    = 2;
	const int iBOTTOM = 3;

	nClip >>= 4;

	if ( nClip & 0x1 )
		(*pHideSides)[LEFT] = true;
	if ( (nClip >> iRIGHT) & 0x1 )
		(*pHideSides)[RIGHT] = true;
	if ( (nClip >> iTOP) & 0x1 )
		(*pHideSides)[TOP] = true;
	if ( (nClip >> iBOTTOM) & 0x1 )
		(*pHideSides)[BOTTOM] = true;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWallObjectInfoClipper::ClipWall()
{
	SPlane clipLeft, clipRight;
	bool bClipLeft, bClipRight;
	CPieceMap &faces = pSrc->GetValue()->faces;
	short nClipL = (clipInfo.nClip >> 8) & 0xff;
	short nClipR = clipInfo.nClip & 0xff;

	bClipLeft  = GetClipPlane( &clipLeft, nClipL, true );
	bClipRight = GetClipPlane( &clipRight, nClipR, false );
	hash_map<int, bool> visibleParts;
	UnpackParts( &visibleParts, clipInfo.dwParts, clipInfo.nSubBlockID );
	//
	if ( !bClipLeft && !bClipRight )
	{
		// не клипаем
		SFacesVector res;
		for ( CPieceMap::const_iterator it = faces.begin(); it != faces.end(); ++it )
		{
			CIntMap old2new;
			if ( !visibleParts[it->first] )
				continue;
			AddOldTris( &res, it->second );
		}
		Assign( pValue, res );
		return;
	}
	//
	SFacesVector dst;
	if ( bClipRight && bClipLeft )
	{
		SFacesVector res;
		vector<ESide> flags;
		for ( CPieceMap::const_iterator it = faces.begin(); it != faces.end(); ++it )
		{
			if ( !visibleParts[it->first] )
				continue;
			const SFacesVector &srcTris = it->second;
			Clip( clipLeft, &res, srcTris );
		}
		NGScene::Clip( clipRight, &dst, res );
	}
	else
	{
		SPlane plane = bClipLeft ? clipLeft : clipRight;
		for ( CPieceMap::const_iterator it = faces.begin(); it != faces.end(); ++it )
		{
			if ( !visibleParts[it->first] )
				continue;
			NGScene::Clip( plane, &dst, it->second );
		}
	}
	Assign( pValue, dst );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void GetHideFloors( short nClip, vector<bool> *pHideSides )
{
	ASSERT( ESIZE == pHideSides->size() );
	const int iLEFT  = 0; // индексы сторон в nClip
	const int iRIGHT = 1;
	const int iFRONT = 2;
	const int iBACK  = 3;
	
	if ( nClip & 0x1 )
		(*pHideSides)[LEFT] = true;
	if ( (nClip >> iRIGHT) & 0x1 )
		(*pHideSides)[RIGHT] = true;
	if ( (nClip >> iFRONT) & 0x1 )
		(*pHideSides)[FRONT] = true;
	if ( (nClip >> iBACK) & 0x1 )
		(*pHideSides)[BACK] = true;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSolidObjectInfoClipper::Recalc()
{
	pSrc.Refresh();
	pValue = new CObjectInfo;
	if ( !IsValid( pSrc->GetValue() ) || pSrc->GetValue()->faces.empty() )
	{
		ASSERT( 0 );
		return;
	}
	ClipSolid();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSolidObjectInfoClipper::SetKey( const SClipShare &key ) 
{
	clipInfo = key; 
	pSrc = shareObjInfoPieces.Get( clipInfo.src );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// сплошные объекты не клипаютс€
void CSolidObjectInfoClipper::ClipSolid()
{
	CPieceMap &faces = pSrc->GetValue()->faces;
	hash_map<int, bool> visibleParts;
	
	UnpackParts( &visibleParts, clipInfo.dwParts, clipInfo.nSubBlockID );

	SFacesVector res;
	for ( CPieceMap::const_iterator it = faces.begin(); it != faces.end(); ++it )
	{
		int x, y, z;
		GetPieceCoords( it->first, &x, &y, &z );
		if ( !visibleParts[it->first] )
			continue;
		AddOldTris( &res, it->second );
	}
	Assign( pValue, res );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Clip( const SPlane &plane, SFacesVector *pDst, const SFacesVector &src )
{
	CIntMap old2new;
	
	SFacesVector &dstData = *pDst;
	const int srcSz = src.polys.size() - 1;
	if ( dstData.polys.empty() )
		dstData.polys.push_back( 0 );
	for ( int i = 0; i < srcSz; ++i )
	{
		list<WORD> poly;
		list<WORD> clipPoly;

		poly.insert( poly.end(), src.indices.begin() + src.polys[i], src.indices.begin() + src.polys[i + 1] );
		
		ClipToPlane( src.verts, &dstData.verts, old2new, plane, poly, &clipPoly );
		if ( clipPoly.size() < 3 )
			continue;
		dstData.indices.insert( dstData.indices.end(), clipPoly.begin(), clipPoly.end() );
		dstData.polys.push_back( dstData.indices.size() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWallObjectInfoClipper::GetClipPlane( SPlane *pPlane, short nClip, bool bLeft )
{
	const float F_INV_SQRT2 = 1.0f / SQRT_2;

	bool  bPerpen  = nClip & 0x1;
	bool  bTop     = (nClip >> 3) & 0x1;
	int   nInWidth = (nClip >> 1) & 0x3;
	if ( 0 == nInWidth ) // нет вход€щей клипающей стенки
		return false;
	float fInWidth = ID2Width( nInWidth );
	short nSrcWidthLen = clipInfo.nClip >> 16;
	int   nWidth   = nSrcWidthLen & 0xff;
	float fWidth   = ID2Width( nWidth );
	float fLength  = FP_GRID_STEP * (nSrcWidthLen >> 8);

	// перпендикул€рные стенки
	if ( bPerpen )
	{
		// одинаковой толщины -> клип плоскость под 45 град.
		if ( nInWidth == nWidth )
		{
			if ( bLeft )
			{
				if ( bTop)
					*pPlane = SPlane( CVec3( F_INV_SQRT2, -F_INV_SQRT2, 0 ), 0 );
				else
					*pPlane = SPlane( CVec3( F_INV_SQRT2, F_INV_SQRT2, 0 ), 0 );
			}
			else
			{
				float d = -F_INV_SQRT2 * fLength;
				if ( bTop)
					*pPlane = SPlane( -CVec3( F_INV_SQRT2, -F_INV_SQRT2, 0 ), d );
				else
					*pPlane = SPlane( -CVec3( F_INV_SQRT2, F_INV_SQRT2, 0 ), d );
			}
			return true;
		}
		// разной толщины -> клип плоскость вдоль осей координат
		else if ( nInWidth > nWidth )
		{
			if ( bLeft )
				*pPlane = SPlane( CVec3( 1, 0, 0 ), 0.5f * fInWidth );
			else
				*pPlane = SPlane( CVec3( -1, 0, 0 ), -(fLength - 0.5f * fInWidth) );
			return true;
		}
		return false;
	}
	// параллельные стенки
	/*
	// если вход€ща€€ стена тоньше, то не клипаем
	if ( fInWidth < fWidth )
		return false;
	*/
	if ( bLeft )
	{
		pPlane->n = CVec3( 1.0f, 0, 0 );
		//pPlane->d = 0.5f * fInWidth;
		pPlane->d = 0;
	}
	else
	{
		pPlane->n = CVec3( -1.0f, 0, 0 );
		//pPlane->d = -(fLength - 0.5f * fInWidth);
		pPlane->d = -fLength;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClipToPlane( const vector<SLoadVertex> &srcVerts, 
		vector<SLoadVertex> *pDstVerts, 
		CIntMap &old2new, 
		const SPlane &plane, 
		const list<WORD> &polyIn, 
		list<WORD> *pPolyOut )
{
	if ( polyIn.empty() )
		return;
	list<WORD>::const_iterator iNext, i = polyIn.begin();

	float fNextDot, fCurDot = srcVerts[*i].pos * plane.n;
	bool  bNextIn,  bCurIn  = fCurDot > plane.d;

	for ( ; i != polyIn.end(); ++i )
	{
		iNext = i;
		++iNext;
		iNext = polyIn.end() == iNext ? polyIn.begin() : iNext;
		//
		if ( bCurIn ) // данна€ вершина не требует обработки
		{
			// добавл€ем в выходной буфер
			CIntMap::iterator it = old2new.find( *i );
			if ( it != old2new.end() )
				pPolyOut->push_back( it->second );
			const int ind = pDstVerts->size();
			pDstVerts->push_back( srcVerts[*i] );
			pPolyOut->push_back( ind );
			old2new[*i] = ind;
		}
		fNextDot = srcVerts[*iNext].pos * plane.n;
		bNextIn  = fNextDot > plane.d;
		//
		if ( bCurIn != bNextIn )
		{
			float fScale = (plane.d - fCurDot) / ( fNextDot - fCurDot );
			float f1Scale = 1.0f - fScale;
			SLoadVertex vPlane;
			const SLoadVertex &vCur = srcVerts[*i];
			const SLoadVertex &vNxt = srcVerts[*iNext];
			
			vPlane.pos     = vCur.pos + fScale * (vNxt.pos - vCur.pos);
			vPlane.normal  = fScale * vCur.normal + f1Scale * vNxt.normal;
			vPlane.texU    = fScale * vCur.texU + f1Scale * vNxt.texU;
			vPlane.texV    = fScale * vCur.texV + f1Scale * vNxt.texV;
			vPlane.tex     = vCur.tex + fScale * (vNxt.tex - vCur.tex);
			vPlane.dwColor = NGfx::GetDWORDColor( fScale * NGfx::GetCVec4Color( vCur.dwColor ) + f1Scale * NGfx::GetCVec4Color( vNxt.dwColor ) );
			Normalize( &vPlane.normal );
			Normalize( &vPlane.texU );
			Normalize( &vPlane.texV );
			pPolyOut->push_back( pDstVerts->size() );
			pDstVerts->push_back( vPlane );
		}
		fCurDot = fNextDot;
		bCurIn  = bNextIn;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class COffset
{
	int nOffs;
	vector<WORD> *pD;
public:
	COffset( int nOffset, vector<WORD> *pDst ): nOffs( nOffset ), pD( pDst ) { ASSERT(pDst); }
	void operator() ( WORD i ) { pD->push_back( i + nOffs ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddOldTris( SFacesVector *pDst, const SFacesVector &src )
{
	if ( src.polys.empty() )
		return;
	SFacesVector &dstData = *pDst;
	if ( dstData.polys.empty() )
		dstData.polys.push_back( 0 );
	const int nIOffset = dstData.verts.size();
	const int nPOffset = dstData.indices.size();
	dstData.verts.insert( dstData.verts.end(), src.verts.begin(), src.verts.end() );
	for_each( src.indices.begin(), src.indices.end(), COffset( nIOffset, &dstData.indices ) );
	for_each( src.polys.begin() + 1, src.polys.end(), COffset( nPOffset, &dstData.polys ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int SplitPoly( vector<STriangle> *pDstTris, const list<WORD> &poly )
{
	list<WORD>::const_iterator i1 = poly.begin();
	list<WORD>::const_iterator i2 = poly.begin();
	++i2;
	list<WORD>::const_iterator i3 = i2;
	++i3;
	//
	int ret = 0;
	for ( ; i3 != poly.end(); i2 = i3, ++i3 )
	{
		pDstTris->push_back( STriangle( *i1, *i2, *i3 ) );
		++ret;
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CWallObjectInfoClipper::operator&( CStructureSaver &f )
{	
	f.Add( 1, &clipInfo );
	f.Add( 2, &pSrc );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CSolidObjectInfoClipper::operator&( CStructureSaver &f )
{
	f.Add( 1, &clipInfo );
	f.Add( 2, &pSrc );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x02741123, CObjectInfoLoader )
REGISTER_SAVELOAD_CLASS( 0x02741130, CObjectInfoPiecesLoader )
REGISTER_SAVELOAD_CLASS( 0x02741131, CWallObjectInfoClipper )
REGISTER_SAVELOAD_CLASS( 0x00571120, CSolidObjectInfoClipper )
REGISTER_SAVELOAD_CLASS( 0x020a2171, CAIGeometryConverter )
REGISTER_SAVELOAD_CLASS( 0x023a2110, CAISkinObjectConverter )