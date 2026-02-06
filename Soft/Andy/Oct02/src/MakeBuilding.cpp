#include "StdAfx.h"
#include "MakeBuilding.h"
#include "BuildingInfo.h"
#include "BuildingGrid.h"
#include <limits>
#include "Grid.h"
#include "Transform.h"
#include "..\Misc\2Darray.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "BuildingSchema.h"
#include "..\Misc\BasicShare.h"
#include "GGeometry.h"
#include "aiobject.h"
#include "aiobjectloader.h"
#include "..\Misc\HPTimer.h"
#include "MELayers.h"
#include "..\Misc\LogStream.h"
#include "BSPTree.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
	extern CBasicShare<int, NAI::CLoadGeometryInfo> shareAIModel;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
	extern CBasicShare<int, NBuilding::CBuildInfoLoader> shareBuildings;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NBuilding
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int   FLOOR_MAX_SIZE = 1;
const int   WALL_MAX_LEN_2 = WALL_MAX_LEN + 2;
////////////////////////////////////////////////////////////////////////////////////////////////////
using std::numeric_limits;
typedef numeric_limits<int> LIM_INT;
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
float ID2Width( int nWidthID )
{
	switch( nWidthID )
	{
		case WALL_THIN_ID:
			return WALL_THIN;
			break;
		case WALL_MED_ID:
			return WALL_MED;
			break;
	}	
	return WALL_THICK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int Width2ID( float fWidth )
{
	if ( fabs( fWidth - WALL_THIN ) < FP_EPSILON )
		return WALL_THIN_ID;
	if ( fabs( fWidth - WALL_MED ) < FP_EPSILON )
		return WALL_MED_ID;
	return WALL_THICK_ID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Вспомогательный класс, используемый для построения карты стен, перекрытий
template<class TValue> class CNodeMap
{
	int nW, nH;
	int nMinF, nMaxF;
	int nPlaneSz;
	vector<TValue> cells;

	int MakeInd( int nFloor, int nx, int ny ) const 
	{ 
		return (nFloor-nMinF) * nPlaneSz + ny * nW + nx; 
	}
public:
	CNodeMap( int nMinFloor, int nMaxFloor, int nWidth, int nHeight )
	{
		ASSERT( nMinFloor <= nMaxFloor );
		nW = nWidth;
		nH = nHeight;
		nMinF = nMinFloor;
		nMaxF = nMaxFloor;
		nPlaneSz = nW * nH;
		cells.resize( nPlaneSz * ( nMaxF - nMinF + 1 ), TValue() );
	}
	int GetWidth() const { return nW; }
	int GetHeight() const { return nH; }
	int GetMinFloor() const { return nMinF; }
	int GetMaxFloor() const { return nMaxF; }
	const vector<TValue>& GetValues() const { return cells; }
	//
	bool IsValidCoord( int nFloor, int nx, int ny ) const
	{
		const int ind = MakeInd( nFloor, nx, ny );
		return 0 <= ind && ind < cells.size();
	}
	//
	TValue& At( int nFloor, int nx, int ny )
	{
		const int ind = MakeInd( nFloor, nx, ny );
		ASSERT( ind >= 0 && ind < cells.size() );
		return cells[ind];
	}
	//
	const TValue& At( int nFloor, int nx, int ny ) const
	{
		return const_cast<CNodeMap<TValue>*>( this )->At( nFloor, nx, ny );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EPriority // solid part prority
{
	SP_LOW = 0,
	SP_SECONDARY = 1,
	SP_PRIMARY = 2,
};
struct SFragmentPos
{
	const SBuildFragment *pFr;
	CPtr<NDb::CConstructionPart> pCPart;
	CVec3 nSubPos;
	int   nIndex;
	int   nHashID;
	SFragmentPos(): pFr(0), nSubPos(VNULL3) {}
};
struct SSolidElement
{
	vector<SFragmentPos> fragments;
	EPriority nPrority;
	vector<SProjectedSpot> spots;
	SSolidElement(): nPrority(SP_LOW) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// структуры, используемые во время вычисления clip info
struct SGridElement
{
	SBuildFragment *pFragment;
	int nFragmentID;
	float fLength;
	float fThickness;
	float fHeight;
	int nClipGroup;
	ESide side;
	SGridElement() { memset( this, 0, sizeof(*this) ); nFragmentID = -1; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGridNode
{
	vector<SGridElement> elems[4]; // по 4ем напрявлениям
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void AssignLRNode( const NDb::CConstructionPart *pCP, SGridNode *pLNode, SGridNode *pRNode,SBuildFragment *pFr, int nFragmentID, int il,	int ir )
{
/*
#ifdef _DEBUG
	if ( pLNode->elems[il].pFragment || pRNode->elems[ir].pFragment )
	{
		OutputDebugString( "CBuilding: Warning : bad wall map\n" );
	}
#endif // _DEBUG
*/
	int x, y, z;
	GetPartCoords( pFr->nSubBlockID, &x, &y, &z );
	SGridElement e;

	e.pFragment = pFr;
	e.nFragmentID = nFragmentID;
	e.fLength = 2;
	e.fThickness = pCP->fThickness;
	e.fHeight = pCP->nSizeZ;
	e.nClipGroup = pCP->nClipGroup;

	if ( 1 == x )
	{
		e.side = LEFT;
		pLNode->elems[il].push_back( e ) ;
	}
	if ( x == pCP->nSizeX )
	{
		e.side = RIGHT;
		pRNode->elems[ir].push_back( e );	
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SLRNeighbs
{
	CPtr<NDb::CConstructionPart> pCPart;
	SGridNode *pLeft;
	SGridNode *pRight;
	vector<SProjectedSpot> spots;
	SLRNeighbs() : pLeft(0), pRight(0) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// 0-left, 1-top, 2-right, 3-bottom
enum 
{
	ELEFT = 0,
	ETOP,
	ERIGHT,
	EBOTTOM,
};
static void FillGridNodes( CNodeMap<SGridNode> *pGrid, SBuildFragment *pf, int nFragmentID, SLRNeighbs *pNeibs )
{
	if ( !pGrid->IsValidCoord( pf->ptPos.z, pf->ptPos.x, pf->ptPos.y ) )
		return;
	SGridNode &node = pGrid->At( pf->ptPos.z, pf->ptPos.x, pf->ptPos.y );
	SGridNode *pRNode = 0;

	switch ( pf->nRotationID )
	{
		case SDiscretePos::TURN_0:
			{
				pRNode = &pGrid->At( pf->ptPos.z, pf->ptPos.x + 2, pf->ptPos.y );
				AssignLRNode( pNeibs->pCPart, &node, pRNode, pf, nFragmentID, 2, 0 );
			}
			break;
		case SDiscretePos::TURN_90:
			{
				pRNode = &pGrid->At( pf->ptPos.z, pf->ptPos.x, pf->ptPos.y + 2 );
				AssignLRNode( pNeibs->pCPart, &node, pRNode, pf, nFragmentID, 1, 3 );
			}
			break;
		case SDiscretePos::TURN_180:
			{
				pRNode = &pGrid->At( pf->ptPos.z, pf->ptPos.x - 2, pf->ptPos.y );
				AssignLRNode( pNeibs->pCPart, &node, pRNode, pf, nFragmentID, 0, 2 );
			}
			break;
		case SDiscretePos::TURN_270:
			{
				pRNode = &pGrid->At( pf->ptPos.z, pf->ptPos.x, pf->ptPos.y - 2 );
				AssignLRNode( pNeibs->pCPart, &node, pRNode, pf, nFragmentID, 3, 1 );
			}
			break;
		default:
			ASSERT(0);
			break;
	}
	pNeibs->pLeft = &node;
	pNeibs->pRight = pRNode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void MakeWallMap( SRand *pRand, CNodeMap<SGridNode> *pGrid, vector<SBuildFragment> *pWallFrags, vector<SLRNeighbs> *pNeighbsVec )
{
	ASSERT( pNeighbsVec && pWallFrags );
	ASSERT( pWallFrags->size() == pNeighbsVec->size() );

	for ( int i = 0; i < pWallFrags->size(); ++i )
	{
		SBuildFragment &fr = (*pWallFrags)[i];
		NDb::CTConstructionPart *pTConstructionPart = NDb::GetTConstructionPart( fr.nConstructionPartID );
		if ( !IsValid( pTConstructionPart ) )
			continue;
		CPtr<NDb::CConstructionPart> pCP = pTConstructionPart->CreateConstructionPart( pRand );
		if ( !IsValid( pCP ) )
			continue;

		SLRNeighbs *pLRN = &(*pNeighbsVec)[i];
		pLRN->pCPart = pCP;
		FillGridNodes( pGrid, &fr, i, pLRN );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EPriority GetPriority( const NDb::CConstructionPart *pPart, int x, int y )
{
	if ( NDb::CConstructionPart::IsPrimaryPart( pPart->nSubPartsMask, x, y ) )
		return SP_PRIMARY;
	// если одна из сторон является смежной для primary, то приоритет - SP_SECONDARY
	if ( x > 0 && NDb::CConstructionPart::IsPrimaryPart( pPart->nSubPartsMask, x - 1, y ) )
		return SP_SECONDARY;
	if ( x < pPart->nSizeX - 1 && NDb::CConstructionPart::IsPrimaryPart( pPart->nSubPartsMask, x + 1, y ) )
		return SP_SECONDARY;
	if ( y > 0 && NDb::CConstructionPart::IsPrimaryPart( pPart->nSubPartsMask, x, y - 1 ) )
		return SP_SECONDARY;
	if ( y < pPart->nSizeY - 1 && NDb::CConstructionPart::IsPrimaryPart( pPart->nSubPartsMask, x, y + 1 ) )
		return SP_SECONDARY;
	return SP_LOW;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MakeLayerGroupHash( hash_map<int, int> *pID2GID, const vector<NBuilding::SLayerGroup> &groups )
{
	hash_map<int, int> &hash = *pID2GID;
	for ( int i = 0; i < groups.size(); ++i )
	{
		const NBuilding::SLayerGroup g = groups[i];
		if ( g.layers.size() <= 1 )
			continue;
		hash[g.layers.front()] = g.layers.front();
		for ( int i = 1; i < g.layers.size(); ++i )
			hash[g.layers[i]] = g.layers.front();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const int DEF_FLOOR = -100;
struct SFloor
{
	int n;
	SFloor( int nFloor = DEF_FLOOR ): n(nFloor) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFloorGroup
{
	hash_map<int, SFloor> floors;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void MakeFloorGroupHash( hash_map<int, SFloorGroup> *pGID2Floors, const vector<NBuilding::SLayerGroup> &groups )
{
	hash_map<int, SFloorGroup> &hash = *pGID2Floors;
	for ( int i = 0; i < groups.size(); ++i )
	{
		const NBuilding::SLayerGroup g = groups[i];
		if ( g.layers.size() < 1 )
			continue;
		SFloorGroup &fg = hash[g.layers.front()];
		for ( int i = 1; i < g.floor.size(); ++i )
			fg.floors[g.floor[i]] = g.floor.front();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef hash_map<int, CNodeMap<SSolidElement> > CSolidMap;
static void MakeSolidMap( SRand *pRand, CSolidMap *pMap, const vector<SBuildFragment> &solids, 
	const CNodeMap<SSolidElement> &pattern, const vector<NBuilding::SLayerGroup> &groups )
{
	ASSERT( pRand && pMap );
	hash_map<int, int> gids;
	hash_map<int, SFloorGroup> linkedfloors; // linkedfloors[ gid ]

	MakeLayerGroupHash( &gids, groups );
	MakeFloorGroupHash( &linkedfloors, groups );
	for ( int i = 0; i < solids.size(); ++i )
	{
		const SBuildFragment &fr = solids[i];
		NDb::CTConstructionPart *pTConstructionPart = NDb::GetTConstructionPart( fr.nConstructionPartID );
		if ( !IsValid( pTConstructionPart ) )
			continue;
		CPtr<NDb::CConstructionPart> pCP = pTConstructionPart->CreateConstructionPart( pRand );
		if ( !IsValid( pCP ) )
			continue;
		//
		CVec3 ptShift = VNULL3;
		switch ( fr.nRotationID )
		{
			case SDiscretePos::TURN_90:
				ptShift = CVec3( 2 * pCP->nSizeY, 0, 0 );
				break;
			case SDiscretePos::TURN_180:
				ptShift = CVec3( 2 * pCP->nSizeX, 2 * pCP->nSizeY, 0 );
				break;
			case SDiscretePos::TURN_270:
				ptShift = CVec3( 0, 2 * pCP->nSizeX, 0 );
				break;
		}
		for ( int k = 0; k < pCP->nSizeY; ++k )
			for ( int l = 0; l < pCP->nSizeX; ++l )
			{
				for ( int m = 0; m < pCP->nSizeZ; ++m )
				{
					const CVec3 nPos( l, k, m );
					SDiscretePos dpos( 0,  VNULL3, fr.nRotationID );
					SDiscretePos dposm( 0,  fr.ptPos, fr.nRotationID );
					CVec3 pos = nPos;
					pos.x *= 2;
					pos.y *= 2;
					dposm.MoveAndRotate( &pos );
					pos += ptShift;
					CVec3 vv( 1, 1, 0 );
					dpos.MoveAndRotate( &vv );
					CVec3 spos = pos;
					spos.x = vv.x < 0 ? pos.x - 1 : pos.x;
					spos.y = vv.y < 0 ? pos.y - 1 : pos.y;
					// объединение слоев для клиппинга
					int nClipGroupID = gids[fr.nFragmentID];
					nClipGroupID = 0 == nClipGroupID ? fr.nFragmentID : nClipGroupID;
					if ( pMap->find( nClipGroupID ) == pMap->end() )
						pMap->insert( pair<int, CNodeMap<SSolidElement> >( nClipGroupID, pattern ) );
					CNodeMap<SSolidElement> &smap = pMap->find( nClipGroupID )->second;
					// объединение этажей для клиппинга
					SFloorGroup &fgroup = linkedfloors[nClipGroupID];
					int nZ;
					if ( fgroup.floors.empty() || fgroup.floors[spos.z].n == DEF_FLOOR )
						nZ = spos.z;
					else
						nZ = fgroup.floors[spos.z].n;
					//
					if ( !smap.IsValidCoord( nZ, spos.x, spos.y ) )
						continue;
					SSolidElement &e = smap.At( nZ, spos.x, spos.y );
					EPriority pr = GetPriority( pCP, nPos.x, nPos.y );
					if ( e.nPrority > pr ) 
						continue; // в этом узле уже есть более приоритетный блок
					if ( pr > e.nPrority )
						e.fragments.clear();
					e.nPrority  = pr;
					SFragmentPos &frp = *e.fragments.insert( e.fragments.end() );
					frp.pFr = &fr;
					frp.nSubPos = pos;
					frp.pCPart  = pCP;
					frp.nIndex  = i;
					frp.nHashID = GetPartHashID( nPos.x + 1, nPos.y + 1, nPos.z + 1 );
					// заполняем все тайлы занимаемые блоком
					vector<CVec3> tiles;
					tiles.push_back( CVec3( 1, 0, 0 ) );
					tiles.push_back( CVec3( 1, 1, 0 ) );
					tiles.push_back( CVec3( 0, 1, 0 ) );
					dpos.MoveAndRotate( &tiles );
					for ( int s = 0; s < tiles.size(); ++s )
					{
						SSolidElement &e = smap.At( nZ, spos.x + tiles[s].x, spos.y + tiles[s].y );
						if ( e.nPrority < pr )
						{
							e.nPrority  = pr;
							e.fragments.clear();
						}
					}
				}
			}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool topLookupTbl[4][7] = 
{
	{1, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 1, 0, 0}
};
// true, если для стенки iThis входящая стенка (определяемая по delta: i = iThis - delta)
// перпендикулярна и расположена в положительной части Y плоскости
// Соответсвие между индексом и направлением:
// 0-left, 1-top, 2-right, 3-bottom
static inline bool GetBTop( int iThis, int delta )
{
	return topLookupTbl[iThis][3 + delta];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// bit map для клип ID:
// 1st b   - bPerpendicular
// 2-3 b   - nWidth (incom width)
// 4 b     - bTop
// 5-8 b   - cull sides
static inline short MakeClipID( int nWidth, bool bPerpendicular, bool bTop, short cullSides )
{
	return (cullSides & 0xf) << 4 | (int)bTop << 3 | nWidth << 1 | (int)bPerpendicular;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static inline short MakeCullID( const vector<bool> &hideSides )
{
	ASSERT( 4 == hideSides.size() );

	return int( hideSides[0] ) | int( hideSides[1] << 1 ) |
		int( hideSides[2] << 2 ) | int( hideSides[3] << 3 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static short GetWallClipInfo( const CNodeMap<SGridNode> &grid, 
	const SBuildFragment &fr, 
	const NDb::CConstructionPart *pCP, 
	const SGridNode *pNode )
{
	if ( !pNode )
		return 0;
	int   iThis  = -1;
	int   iThick = -1;
	SGridElement geThis, geThick;
	vector<pair<int, float> > indwidth;
	int nClipGroup = pCP->nClipGroup;
	float fThickness = pCP->fThickness;

	{
		// находим iThis
		for ( int i = 0; i < 4 && iThis == -1; ++i )
		{
			for ( int j = 0; j < pNode->elems[i].size(); ++j )
			{
				SBuildFragment *pFr = pNode->elems[i][j].pFragment;
				if ( &fr == pFr )
				{
					iThis = i;
					geThis = pNode->elems[i][j];
					indwidth.push_back( pair<int, float>( i, geThis.fThickness ) );
					break;
				}
			}
		}
		// Находим самую толстую стенку в узле
		float fThick = 0;
		for ( int i = 0; i < 4; ++i )
		{
			if ( i == iThis || pNode->elems[i].empty() )
				continue;
			float fW = 0;
			for ( int j = 0; j < pNode->elems[i].size(); ++j )
			{
				if ( !pNode->elems[i][j].pFragment )
					continue;
				SGridElement e = pNode->elems[i][j];
				SBuildFragment *pFr = e.pFragment;
				fW = Max( fW, e.fThickness );
				if ( e.fThickness > fThick && nClipGroup == e.nClipGroup )
				{
					iThick = i;
					fThick = e.fThickness;
					geThick = e;
				}
			}
			indwidth.push_back( pair<int, float>( i, fW ) );
		}
	}
	if ( -1 == iThis )
		return 0;
	// ASSERT( -1 != iThis );
	//
	vector<bool> hideSides( 4, false ); 
	const int iLEFT   = 0; // индексы сторон в массиве hideSides
	const int iRIGHT  = 1;
	const int iTOP    = 2;
	const int iBOTTOM = 3;
	//
	if ( -1 == iThick )
		return MakeClipID( 0, 0, 0, MakeCullID( hideSides ) );;
	//
	int nInWidth = Width2ID( geThick.fThickness );
	int nWidth = Width2ID( fThickness );
	// пересечение с длинной стенкой ?
	if ( INTERNAL == geThick.side )
		return MakeClipID( nInWidth, 0, 0, MakeCullID( hideSides ) );	
	// Возможно это особый случай - стыкуются более 2х стенок одинаковой толщины, надо проверить
	if ( nInWidth <= nWidth && indwidth.size() > 2 )
	{
		int k, cnt = 0;
		const float fw = fThickness;
		for ( k = 0; k < indwidth.size(); ++k )
			if ( fw == indwidth[k].second )
				++cnt;
		if ( cnt > 2 )
		{
			int iOpposite = (iThis+2) % 4;
			if ( !pNode->elems[iOpposite].empty() )
			{
				// так и есть! это особый случай
				const SGridElement &eopposite = pNode->elems[iOpposite].front();
				if ( eopposite.pFragment && nClipGroup == eopposite.nClipGroup )
				{
					if ( LEFT == geThis.side )
						hideSides[iLEFT] = true;
					else
						hideSides[iRIGHT] = true;
				}
			}
			short cullId = MakeCullID( hideSides );
			int nInW = Width2ID( fw );
			//
			return MakeClipID( nInW, 0, 0, cullId );
		}
	}
	//
	bool bTop = false;
	bool bPerpendicular = false;
	if ( (iThis - iThick) & 0x1 )
	{
		bPerpendicular = true;
		bTop = GetBTop( iThis, iThis - iThick );
	}
	else
	{
		bPerpendicular = false;
		if ( nInWidth == Width2ID( fThickness ) )
		{
			// в этом случае надо выбрать какую из 2х стенок клипать
			
			if ( geThis.side == geThick.side )
			{
				///if ( 0 == iThis || 1 == iThis )
					//nInWidth = 0;
			}
			else if ( LEFT == geThis.side )
			{
				hideSides[iLEFT] = true;
				//nInWidth = 0;
			}
			else
				hideSides[iRIGHT] = true;
		}
	}
	if ( nInWidth < nWidth )
		nInWidth = 0;
	return MakeClipID( nInWidth, bPerpendicular, bTop, MakeCullID( hideSides ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static inline SPoint3 GridCoords( const CVec3 &ptPos, int nRotationID, int x, int y, int z )
{
	SDiscretePos dpos( 0, VNULL3, nRotationID );
	CVec3 pt( x, y, z );
	dpos.MoveAndRotate( &pt );
	pt += CVec3( ptPos.x, ptPos.y, 0 );
	return SPoint3( pt );
}
static void SetVisibility( hash_map<int, bool> *pVisible, const CBuildingGrid &grid, 
	const CVec3 &ptPos, int nRotationID, const SPoint3 &pt )
{
	bool bRes = !grid.IsDestroyed( GridCoords( ptPos, nRotationID, pt.x, pt.y, Float2Int( ptPos.z * 4 + pt.z ) ) );
	(*pVisible)[GetPieceHashID( pt.x + 1, pt.y+ 1, pt.z + 1 )] = bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_SMALL_VOLUME = 0.01f;
int FixSmallPieceID( const NAI::CGeometryInfo::CPieceMap &pieces, int nPieceID )
{
	return nPieceID;

	NAI::CGeometryInfo::CPieceMap::const_iterator it = pieces.find( nPieceID );
	if ( it == pieces.end() )
	{
//		ASSERT(0);
		return nPieceID;
	}
	const NAI::CGeometryInfo::SPiece &pc = it->second;
	if ( pc.fVolume > F_SMALL_VOLUME )
		return nPieceID;

	int x, y, z;
	int partx, party, partz;
	GetPieceCoords( nPieceID, &x, &y, &z );
	GetPartCoords( nPieceID, &partx, &party, &partz );
	int nPartID = GetPartHashID( partx, party, partz );

	float fMinVol = 1e30f;
	int nMinDist = 1e6f;
	int nID = -1;
	for ( int dx = -1; dx <= 1; ++dx )
		for ( int dy = -1; dy <= 1; ++dy )
			for ( int dz = -1; dz <= 1; ++dz )
			{
				it = pieces.find( nPartID | GetPieceHashID( x+dx, y+dy, z+dz ) );
				if ( it != pieces.end() )
				{
					const float fVol = it->second.fVolume;
					if ( fVol > F_SMALL_VOLUME )
					{
						const int nDist = dx * dx + dy * dy + dz * dz;
						if ( nDist < nMinDist || ( nDist == nMinDist && fVol < fMinVol ) )
						{
							fMinVol = fVol;
							nID = it->first;
							nMinDist = nDist;
						}
					}
				}
			}

	if ( nID != -1 )
		return nID;
	return nPieceID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Прописывание в clipinfo видимых частей сплошного объекта на основе данных в pBuildingGrid
static void SetSolidVisibleParts( SClipInfo *pClip, CVec3 ptPos, const CBuildingGrid &grid, bool bParts, NAI::CGeometryInfo *pGI )
{
	hash_map<int, bool> visible;

	if ( !pGI )
	{
		for ( int z = 0; z <= 4; ++z )
		{
			if ( !(z & 0x1) )
			{
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 1, 0, z ) );
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 0, 1, z ) );
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 2, 1, z ) );
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 1, 2, z ) );
			}
			else
			{
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 0, 0, z ) );
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 2, 0, z ) );
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 1, 1, z ) );
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 0, 2, z ) );
				SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( 2, 2, z ) );
			}
		}
	}
	else
	{
		for ( NAI::CGeometryInfo::CPieceMap::const_iterator i = pGI->pieces.begin(); i != pGI->pieces.end(); ++i )
		{
			int nPieceID = FixSmallPieceID( pGI->pieces, i->first );
			int x, y, z;
			GetPartCoords( nPieceID, &x, &y, &z );
			if ( GetPartHashID( x, y, z ) != pClip->nSubBlockID )
				continue;
			GetPieceCoords( nPieceID, &x, &y, &z );
			if ( GetPieceHashID( x,y,z) == 0 )
				continue;
			SetVisibility( &visible, grid, ptPos, pClip->nRotationID, SPoint3( x-1, y-1, z-1 ) );
		}
	}
	// если все куски видны, рисуем блок как цельный
	bool bVis = true;
	for ( hash_map<int, bool>::const_iterator it = visible.begin(); it != visible.end(); ++it )
		bVis = it->second && bVis;
	if ( bVis && !bParts )
		visible.clear();
	
	PackParts( &pClip->dwParts, visible );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
inline void SetWallParts( hash_map<int, bool> *pVisible, const CBuildingGrid &grid, 
	int nFloor, int x, int y, int dx, int dy )
{
	hash_map<int, bool> &visible = *pVisible;
	visible[GetPieceHashID( 1, 1, 2 )] = !grid.IsDestroyed( nFloor, x, y, 2 );
	visible[GetPieceHashID( 1, 1, 4 )] = !grid.IsDestroyed( nFloor, x, y, 4 );		
	visible[GetPieceHashID( 2, 1, 1 )] = !grid.IsDestroyed( nFloor, x + dx, y + dy, 1 );
	visible[GetPieceHashID( 2, 1, 3 )] = !grid.IsDestroyed( nFloor, x + dx, y + dy, 3 );
	visible[GetPieceHashID( 2, 1, 5 )] = !grid.IsDestroyed( nFloor, x + dx, y + dy, 5 );				
	dx = Sign( dx ) * (abs( dx ) + 1);
	dy = Sign( dy ) * (abs( dy ) + 1);
	visible[GetPieceHashID( 3, 1, 2 )] = !grid.IsDestroyed( nFloor, x + dx, y + dy, 2 );
	visible[GetPieceHashID( 3, 1, 4 )] = !grid.IsDestroyed( nFloor, x + dx, y + dy, 4 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Прописывание в clipinfo видимых частей стенки на основе данных в pBuildingGrid
static void SetWallVisibleParts( SClipInfo *pClip, const CVec3 &ptPos, const CBuildingGrid &grid, bool bParts )
{
	hash_map<int, bool> visible;

	int x = ptPos.x + 1;
	int y = ptPos.y + 1;

	switch ( pClip->nRotationID )
	{
		case SDiscretePos::TURN_0:
			SetWallParts( &visible, grid, ptPos.z, x, y, 1, 0 );
			break;
		case SDiscretePos::TURN_90:
			SetWallParts( &visible, grid, ptPos.z, x, y, 0, 1 );
			break;
		case SDiscretePos::TURN_180:
			SetWallParts( &visible, grid, ptPos.z, x, y, -1, 0 );
			break;
		case SDiscretePos::TURN_270:
			SetWallParts( &visible, grid, ptPos.z, x, y, 0, -1 );
			break;
	}
	// если все куски видны, рисуем блок как цельный
	bool bVis = true;
	for ( hash_map<int, bool>::const_iterator it = visible.begin(); it != visible.end(); ++it )
		bVis = it->second && bVis;
	if ( bVis && !bParts )
		visible.clear();
	PackParts( &pClip->dwParts, visible );
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
static inline int GetSolidRoomID( const SClipInfo &clip, const CBuildInfo *pInfo )
{
	SDiscretePos dpos(0, VNULL3, clip.nRotationID);
	CVec3 pt(1,1,0);
	dpos.MoveAndRotate( &pt );
	int x = clip.ptPos.x + pt.x;
	int y = clip.ptPos.y + pt.y;
	int z = clip.ptPos.z - pInfo->nMinFloor;
	if ( z < 0 || z >= pInfo->roomMap.size() )
	{
		ASSERT( 0 );
		return 0;
	}
	const CArray2D<BYTE> &rm = pInfo->roomMap[z];
	if ( x >= rm.GetXSize() || y >= rm.GetYSize() )
	{
		ASSERT( 0 );
		return 0;
	}
	return rm[y][x];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// определить в какой комнате находится кусок стены с материалом nMatID
// 0-й материал должен быть со стороны -Y, 1-ый со стороны +Y
static inline int GetWallRoomID( int nMatID, const SBuildFragment &fr, const CBuildInfo *pInfo )
{
	const CArray2D<BYTE> &rmap = pInfo->roomMap[int( fr.ptPos.z ) - pInfo->nMinFloor];
	if ( fr.ptPos.x >= rmap.GetXSize() || fr.ptPos.y >= rmap.GetYSize() )
	{
		ASSERT(0);
		return 0;
	}

	switch ( fr.nRotationID )
	{
		case SDiscretePos::TURN_0:
			switch ( nMatID )
			{
				case 0:
					return rmap[int(fr.ptPos.y)][int(fr.ptPos.x) + 1];
				default:
					return rmap[int(fr.ptPos.y) + 1][int(fr.ptPos.x) + 1];
			}
		case SDiscretePos::TURN_90:
			switch ( nMatID )
			{
			case 0:
				return rmap[int(fr.ptPos.y) + 1][int(fr.ptPos.x) + 1];
			default:
				return rmap[int(fr.ptPos.y) + 1][int(fr.ptPos.x)];
			}
		case SDiscretePos::TURN_180:
			switch ( nMatID )
			{
			case 0:
				return rmap[int(fr.ptPos.y) + 1][int(fr.ptPos.x) - 1];
			default:
				return rmap[int(fr.ptPos.y)][int(fr.ptPos.x) - 1];
			}
		case SDiscretePos::TURN_270:
			switch ( nMatID )
			{
			case 0:
				return rmap[int(fr.ptPos.y) - 1][int(fr.ptPos.x)];
			default:
				return rmap[int(fr.ptPos.y) - 1][int(fr.ptPos.x) + 1];
			}
	}
	ASSERT( false );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CMixedMaterial* CreatePartMaterial( SRand *pRand, const SRawMixedMaterial &src, NDb::CMaterial *pDefMaterial )
{
	CMixedMaterial *pRet = src.CreateMixedMaterial( pRand );
	if ( pRet )
		return pRet;
	pRet = new CMixedMaterial;
	SMaterialApply a;
	a.mapping   = SMaterialApply::NORMAL;
	a.fScale    = 1;
	a.ptShift   = VNULL2;
	a.nRotation = 0;
	if ( pDefMaterial )
		a.pMaterial = pDefMaterial;
	else
		a.pMaterial = NDb::GetMaterial( NDb::N_DEF_MATERIAL_ID );
	pRet->layers.push_back( a );
	return pRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AddFragment( vector<SStoreyInfo::SFragment> *pRes, const CBuildingGrid &grid, 
	const SClipInfo &_clip, const SBuildFragment &fr, const NDb::CConstructionPart *pCP, 
	bool bWall, bool bParts, int nFragID, const vector<SProjectedSpot> &spots, SRand *pRand )
{
	if ( !IsValid( pCP->pGeometry ) )
		return;
	NAI::CGeometryInfo *pGI = 0;
	if ( IsValid( pCP->pGeometry->pAIGeometry ) )
	{
		CDGPtr< CPtrFuncBase<NAI::CGeometryInfo> > pSrc = NAI::shareAIModel.Get( pCP->pGeometry->pAIGeometry->GetRecordID() );
		pSrc.Refresh();
		pGI = pSrc->GetValue();
	}
	SClipInfo clip(_clip);
	SetSolidVisibleParts( &clip, clip.ptPos, grid, bParts, pGI );
	//
	for ( int i = 0; i < NDb::N_FIRST_GEOMMATERIALS; ++i )
		clip.pMaterials[i] = CreatePartMaterial( pRand, fr.materials[i], pCP->pDefMaterials[i] );
	vector<SStoreyInfo::SSpot> &fspots = pRes->insert( pRes->end(), SStoreyInfo::SFragment( clip, nFragID, pCP->pArmor ) )->spots;
	//for ( int i = 0; i < spots.size(); ++i )
	for ( int i = 0; i < fr.spots.size(); ++i )
	{
		for ( int j = 0 ; j < spots.size(); ++j )
			if ( spots[j].nID == fr.spots[i] )
			{
				const SProjectedSpot &s = spots[j];
				NDb::CTMaterial *pM = NDb::GetTMaterial( s.nMaterialID );
				if ( pM )
					fspots.push_back( SStoreyInfo::SSpot( s.ptOrigin, s.ptNormal, s.ptSize, s.nRotation, pM->GetMaterial( pRand ), s.nMaterialMask ) );
			}
	}

	if ( IsValid( pCP->p2ndGeometry ) )
	{
		clip.pGeometry = pCP->p2ndGeometry;
		clip.nClip = 0;
		for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
			clip.pMaterials[i] = 0;
		for ( int i = 0; i < NDb::N_SECOND_GEOMMATERIALS; ++i )
		{
			int nMIndex = i + NDb::N_FIRST_GEOMMATERIALS;
			clip.pMaterials[i] = CreatePartMaterial( pRand, fr.materials[nMIndex], pCP->pDefMaterials[nMIndex] );
		}
		pRes->push_back( SStoreyInfo::SFragment( clip, 0x20000000 | nFragID, pCP->pArmor  ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline SPart Point2Part( const CVec3 &pt, int nRotationID )
{
	SDiscretePos dpos( 0, pt, nRotationID );

	CVec3 dpt(1, 0, 0);
	dpos.MoveAndRotate( &dpt );
	return SPart( pt.z, dpt.x / 2, dpt.y / 2 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline SPart Point2PartSolid( const CVec3 &pt, int nRotationID )
{
	SDiscretePos dpos( 0, pt, nRotationID );

	CVec3 dpt(1, 1, 0);
	dpos.MoveAndRotate( &dpt );
	return SPart( pt.z, dpt.x / 2, dpt.y / 2 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator==( const SProjectedSpot &a, const SProjectedSpot &b )
{
	return a.nMaterialID == b.nMaterialID && a.nRotation == b.nRotation && a.ptOrigin == b.ptOrigin && 
		a.ptNormal == b.ptNormal && a.ptSize == b.ptSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MakeBuilding( SBuildingInfo *pInfo, const CBuildingGrid &grid, CBuildInfo *pBuildInfo, bool bParts )
{
	ASSERT( pInfo && pBuildInfo );
	SRand rand( grid.GetSeed() );
	pInfo->Clear();
	const int nCutFloor = grid.GetCutFloor();

  // Сплошные объекты
	CNodeMap<SSolidElement> pattern( pBuildInfo->nMinFloor, pBuildInfo->nMaxFloor, pBuildInfo->nMaxX + 2, pBuildInfo->nMaxY + 2 );
	CSolidMap solidMap;
	MakeSolidMap( &rand, &solidMap, pBuildInfo->solidFragments, pattern, pBuildInfo->lgroups );
	//CollectSolidSpots( &solidMap, pBuildInfo );

	for ( hash_map<int, CNodeMap<SSolidElement> >::const_iterator it = solidMap.begin(); it != solidMap.end(); ++it )
	{
		const vector<SSolidElement> &frags = it->second.GetValues();
		for ( int i = 0; i < frags.size(); ++i )
		{
			const SSolidElement &e = frags[i];
			for ( int j = 0; j < e.fragments.size(); ++j )
			{
				const SFragmentPos &frp = e.fragments[j];
				if ( !frp.pFr || floor(frp.pFr->ptPos.z) > nCutFloor || !grid.IsLayerVisible( frp.pFr->nFragmentID ) )
					continue;
				SClipInfo clip;
				clip.ptPos = frp.nSubPos;
				clip.pGeometry  = frp.pCPart->pGeometry;
				clip.nRotationID  = frp.pFr->nRotationID;
				clip.nSubBlockID  = frp.nHashID;
				clip.nRooms[0] = clip.nRooms[1] = GetSolidRoomID( clip, pBuildInfo ); // целиком находится в одной комнате
				//SStoreyInfo &storey = pInfo->GetStorey( floor( clip.ptPos.z ) );
				SStoreyInfo &storey = pInfo->GetPart( Point2PartSolid( clip.ptPos, clip.nRotationID ) );
				AddFragment( &storey.fragments, grid, clip, *frp.pFr, frp.pCPart, false, bParts, frp.nIndex, pBuildInfo->spots, &rand );
			}
		}
	}
	// Стены
	int nWallLayer = MakeFragmentID( LID_WALLS, 0 );
	if ( !grid.IsLayerVisible( nWallLayer ) )
		return;
	CNodeMap<SGridNode> wallGrid( pBuildInfo->nMinFloor, pBuildInfo->nMaxFloor, pBuildInfo->nMaxX + 2, pBuildInfo->nMaxY + 2 );
	vector<SLRNeighbs> neighbs( pBuildInfo->wallFragments.size() );
	MakeWallMap( &rand, &wallGrid, &pBuildInfo->wallFragments, &neighbs );
	//CollectWallSpots( &neighbs, wallGrid, pBuildInfo );

	for ( int i = 0; i < pBuildInfo->wallFragments.size(); ++i )
	{
		const SBuildFragment &fr = pBuildInfo->wallFragments[i];
		NDb::CConstructionPart *pCP = neighbs[i].pCPart;
		if ( !IsValid( pCP ) || !IsValid( pCP->pGeometry ) || floor(fr.ptPos.z) > nCutFloor )
			continue;

		SClipInfo clip;
		clip.ptPos = fr.ptPos;
		clip.pGeometry  = pCP->pGeometry;
		clip.nRotationID  = fr.nRotationID;
		clip.nSubBlockID  = fr.nSubBlockID;
		clip.nClip  = ((2 << 8) + Width2ID( pCP->fThickness )) << 16;
		clip.nClip += GetWallClipInfo( wallGrid, fr, pCP, neighbs[i].pLeft ) << 8; // левый край
		clip.nClip += GetWallClipInfo( wallGrid, fr, pCP, neighbs[i].pRight ); // правый край
		clip.nRooms[0] = 0;//GetWallRoomID( 0, fr, pBuildInfo );
		clip.nRooms[1] = 0;//GetWallRoomID( 1, fr, pBuildInfo );
		//SStoreyInfo &storey = pInfo->GetStorey( clip.ptPos.z );
		SStoreyInfo &storey = pInfo->GetPart( Point2Part( clip.ptPos, clip.nRotationID ) );
		//
		AddFragment( &storey.walls, grid, clip, fr, pCP, true, bParts, i, pBuildInfo->spots, &rand );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MarkCellarWalls( CArray2D<bool> *pWalls, const CArray2D<bool> &cellar )
{
	ASSERT( pWalls );
	pWalls->SetSizes( cellar.GetXSize() + 2, cellar.GetYSize() + 2 );
	pWalls->FillEvery( false );
	CArray2D<bool> &walls = *pWalls;
	const int nX = cellar.GetXSize();
	const int nY = cellar.GetYSize();
	for ( int x = 0; x < nX; ++x )
	{
		walls[0][x] = cellar[0][x];
		walls[nY][x] = cellar[nY-1][x];
	}
	for ( int y = 0; y < nY; ++y )
	{
		walls[y][0] = cellar[y][0];
		walls[y][nX] = cellar[y][nX-1];
	}
	for ( int y = 1; y < nY; ++y )
		for ( int x = 1; x < nX; ++x )
		{
			const bool val = cellar[y][x];
			//bool b1 = x < nX - 1 ? val != cellar[y][x+1] : false;
			//bool b2 = y < nY - 1 ? val != cellar[y+1][x] : false;
			walls[y][x] = val != cellar[y][x-1] || val != cellar[y-1][x] || val != cellar[y-1][x-1];// || b1 || b2;
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsCellarWall( const CVec3 &pt, const CArray2D<bool> &cellarWalls )
{
	if ( pt.z > 0 || pt.x < 0 || pt.x >= cellarWalls.GetXSize() || pt.y < 0 || pt.y >= cellarWalls.GetYSize() )
		return false;
	return cellarWalls[(int)pt.y][(int)pt.x];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsGround( const CVec3 &pt )
{
	return pt.z <= 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void JuncsMoveRotate( const SDiscretePos &dpos, vector<SJunction> *pJuncs )
{
	for ( int i = 0; i < pJuncs->size(); ++i )
		dpos.MoveAndRotate( &(*pJuncs)[i].pt );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SchemaAddSolid( CBuildingSchema *pSchema, const SFragmentPos &frp, 
	const CBuildingGrid &grid, int nAIGeomID, const CArray2D<bool> &cellarWalls )
{
	SClipInfo clip;
	clip.ptPos = frp.nSubPos;
	clip.nRotationID  = frp.pFr->nRotationID;
	clip.nSubBlockID = frp.nHashID; 
	NDb::CRPGArmor *pAr = frp.pCPart->pArmor;
	if ( !pAr )
		return;

	// определяем, какие узлы присутсвуют в блоке
	CDGPtr< CPtrFuncBase<NAI::CGeometryInfo> > pSrc = NAI::shareAIModel.Get( nAIGeomID );
	pSrc.Refresh();
	NAI::CGeometryInfo *pGI = pSrc->GetValue();
	if ( !pGI )
		return;
	//
	SetSolidVisibleParts( &clip, clip.ptPos, grid, true, pGI );
	hash_map<int, bool> parts;
	UnpackParts( &parts, clip.dwParts, frp.nHashID );
	//
	CVec3 ptPos = clip.ptPos;
	ptPos.z *= 4;
	SDiscretePos dpos( 0, ptPos, clip.nRotationID );
	bool bIndestructible = pAr->nDR < 0;
	//
	for ( hash_map<int, bool>::iterator it = parts.begin(); it != parts.end(); ++it )
	{
		if ( !it->second )
			continue;
		int x, y, z;
		GetPieceCoords( it->first, &x, &y, &z );
		if ( 0 == x && 0 == y && 0 == z )
			continue;
//		const bool bGround = clip.ptPos.z <= 0 && z == 1;
		const bool bGround = bIndestructible || clip.ptPos.z <= 0;
		const bool bGroundD = bIndestructible || clip.ptPos.z <= 0 && z - 1 == 1;
		vector<SJunction> juncs;
		const NAI::CGeometryInfo::SPiece &piece = pGI->pieces[it->first];
		const float fWeight = piece.fVolume * pAr->fWeight;

		for ( int i = 0; i < piece.juncs.size(); ++i )
		{
			juncs.push_back( SJunction( piece.juncs[i].pt, bGround && piece.juncs[i].bGround ) );
		}
		CVec3 pt( x-1, y-1, z-1 );
		JuncsMoveRotate( dpos, &juncs );
		dpos.MoveAndRotate( &pt );
		bool bCellarWall = IsCellarWall( pt, cellarWalls );
		pSchema->AddNode( pAr, pt, bCellarWall, fWeight, juncs );
		continue;

		if ( y == 1 || y == 3 )
		{
			if ( x == 1 || x == 3 )
			{
				// 2 вертикальных стерженя + 2 гор.
				CVec3 pt( x-1, y-1, z-1 );

				juncs.push_back( SJunction( (y == 3 ? CVec3( x-1, y-2, z-1 ) : CVec3( x-1, y, z-1 )) ) );
				juncs.push_back( SJunction( (x == 3 ? CVec3( x-2, y-1, z-1 ) : CVec3( x, y-1, z-1 )) ) );
				juncs.push_back( SJunction( CVec3( pt.x, pt.y, pt.z-1 ), bGroundD ) );
				juncs.push_back( SJunction( CVec3( pt.x, pt.y, pt.z+1 ) ) );
				JuncsMoveRotate( dpos, &juncs );
				dpos.MoveAndRotate( &pt );
				bool bCellarWall = bIndestructible || IsCellarWall( pt, cellarWalls );
				pSchema->AddNode( pAr, pt, bCellarWall, fWeight, juncs );
			}
			else
			{
				// 2 горизонтальных стерженя + 2||3 вертик.
				CVec3 ptC( x-1, y-1, z-1 );

				juncs.push_back( SJunction( CVec3( x-2, y-1, z-1 ), bGround ) );
				juncs.push_back( SJunction( CVec3( x, y-1, z-1 ), bGround ) );
				juncs.push_back( SJunction( y == 1 ? CVec3( x-1, y, z-1 ) : CVec3( x-1, y-2, z-1 ), bGround ) );
				if ( z == 3 || z == 1 )
					juncs.push_back( SJunction( CVec3( x-1, y-1, z ), bGround ) );
				if ( z == 3 || z == 5 )
					juncs.push_back( SJunction( CVec3( x-1, y-1, z-2 ), bGroundD ) );
				JuncsMoveRotate( dpos, &juncs );
				dpos.MoveAndRotate( &ptC );
				bool bCellarWall = bIndestructible || IsCellarWall( ptC, cellarWalls );
				pSchema->AddNode( pAr, ptC, bCellarWall, fWeight, juncs );
			}
		}
		else if ( x == 1 || x == 3 )
		{
			ASSERT( y == 2 );
			CVec3 ptC( x-1, y-1, z-1 );

			juncs.push_back( SJunction( CVec3( x-1, y-2, z-1 ), bGround ) );
			juncs.push_back( SJunction( CVec3( x-1, y, z-1 ), bGround ) );
			juncs.push_back( SJunction( x == 1 ? CVec3( x, y-1, z-1 ) : CVec3( x-2, y-1, z-1 ), bGround ) );
			if ( z == 3 || z == 1 )
				juncs.push_back( SJunction( CVec3( x-1, y-1, z ), bGround ) );
			if ( z == 3 || z == 5 )
				juncs.push_back( SJunction( CVec3( x-1, y-1, z-2 ), bGroundD ) );
			JuncsMoveRotate( dpos, &juncs );
			dpos.MoveAndRotate( &ptC );
			bool bCellarWall = bIndestructible || IsCellarWall( ptC, cellarWalls );
			pSchema->AddNode( pAr, ptC, bCellarWall, fWeight, juncs );
		}
		else // x=2; y=2;
		{
			ASSERT( y == 2 && x == 2 );
			// 6 стержней
			CVec3 ptC( x-1, y-1, z-1 );

			juncs.push_back( SJunction( CVec3( x-1, y-1, z-2 ), bGround ) );
			juncs.push_back( SJunction( CVec3( x-1, y-1, z ) ) );
			juncs.push_back( SJunction( CVec3( x-2, y-1, z-1 ) ) );
			juncs.push_back( SJunction( CVec3( x, y-1, z-1 ) ) );
			juncs.push_back( SJunction( CVec3( x-1, y-2, z-1 ) ) );
			juncs.push_back( SJunction( CVec3( x-1, y, z-1 ) ) );
			JuncsMoveRotate( dpos, &juncs );
			dpos.MoveAndRotate( &ptC );
			bool bCellarWall = bIndestructible || IsCellarWall( ptC, cellarWalls );
			pSchema->AddNode( pAr, ptC, bCellarWall, fWeight, juncs );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SchemaAddWall( CBuildingSchema *pSchema, const SBuildFragment &fr, const CBuildingGrid &grid, 
	int nAIGeomID, const CArray2D<bool> &cellarWalls, NDb::CRPGArmor *pAr )
{
	// определяем, какие узлы присутсвуют в блоке
	CDGPtr< CPtrFuncBase<NAI::CGeometryInfo> > pSrc = NAI::shareAIModel.Get( nAIGeomID );
	pSrc.Refresh();
	NAI::CGeometryInfo *pGI = pSrc->GetValue();
	if ( !pGI )
		return;

	SClipInfo clip;
	hash_map<int, bool> parts;

	clip.nRotationID = fr.nRotationID;
	clip.nSubBlockID = fr.nSubBlockID; 
	SetSolidVisibleParts( &clip, fr.ptPos, grid, true, pGI );
	UnpackParts( &parts, clip.dwParts, fr.nSubBlockID );
	CVec3 ptPos = fr.ptPos;
	ptPos.z *= 4;
	SDiscretePos dpos( 0, ptPos, fr.nRotationID );
	bool bIndestructible = pAr->nDR < 0 ;
	for ( hash_map<int, bool>::iterator it = parts.begin(); it != parts.end(); ++it )
	{
		if ( !it->second )
			continue;
		int x, y, z;
		GetPieceCoords( it->first, &x, &y, &z );
//		const bool bGround = fr.ptPos.z <= 0 && z == 1;
		const bool bGround = bIndestructible || fr.ptPos.z <= 0;
		const bool bGroundD = bIndestructible || fr.ptPos.z <= 0 && z - 1 == 1;
		vector<SJunction> juncs;
		const NAI::CGeometryInfo::SPiece &piece = pGI->pieces[it->first];
		const float fWeight = piece.fVolume * pAr->fWeight;

		for ( int i = 0; i < piece.juncs.size(); ++i )
		{
			juncs.push_back( SJunction( piece.juncs[i].pt, bGround && piece.juncs[i].bGround ) );
		}
		CVec3 pt( x-1, y-1, z-1 );
		JuncsMoveRotate( dpos, &juncs );
		dpos.MoveAndRotate( &pt );
		bool bCellarWall = bIndestructible || IsCellarWall( pt, cellarWalls );
		pSchema->AddNode( pAr, pt, bCellarWall, fWeight, juncs );
		continue;

		if ( x & 0x1 )
		{
			// 2 вертикальных стерженя + 1
			CVec3 pt( x-1, y-1, z-1 );

			juncs.push_back( SJunction( CVec3( x == 3 ? CVec3( x-2, y-1, z-1 ) : CVec3( x, y-1, z-1 ) ), bGroundD ) );
			JuncsMoveRotate( dpos, &juncs );
			dpos.MoveAndRotate( &pt );
			bool bCellarWall = bIndestructible || IsCellarWall( pt, cellarWalls );
			juncs.push_back( SJunction( CVec3( pt.x, pt.y, pt.z-1 ), 1 ) );
			juncs.push_back( SJunction( CVec3( pt.x, pt.y, pt.z+1 ), 1 ) );
			pSchema->AddNode( pAr, pt, bCellarWall, fWeight, juncs );
		}
		else
		{
			// 2 горизонтальных стерженя + 1
			CVec3 ptC( x-1, y-1, z-1 );

			juncs.push_back( SJunction( CVec3( x-2, y-1, z-1 ), bGround ) );
			juncs.push_back( SJunction( CVec3( x, y-1, z-1 ), bGround ) );
			if ( z == 3 || z == 1 )
				juncs.push_back( SJunction( CVec3( x-1, y-1, z ), bGround ) );
			if ( z == 3 || z == 5 )
				juncs.push_back( SJunction( CVec3( x-1, y-1, z-2 ), bGroundD ) );
			JuncsMoveRotate( dpos, &juncs );
			dpos.MoveAndRotate( &ptC );
			bool bCellarWall = bIndestructible || IsCellarWall( ptC, cellarWalls );
			pSchema->AddNode( pAr, ptC, bCellarWall, fWeight, juncs );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MakeBuildingSchema( CBuildingSchema *pSchema, CBuildingGrid *pGrid, CBuildInfo *pBuildInfo )
{
	if ( !pGrid || !pSchema || !pBuildInfo )
	{
		ASSERT( 0 );
		return;
	}
	SRand rand( pGrid->GetSeed() );
	CArray2D<bool> cellarWalls;
	MarkCellarWalls( &cellarWalls, pBuildInfo->cellar );
	int nJuncReserve = pBuildInfo->wallFragments.size() * 15 + pBuildInfo->solidFragments.size() * 9;
	int nRodReserve = pBuildInfo->wallFragments.size() * 22 + pBuildInfo->solidFragments.size() * 12;
	pSchema->Reserve( nJuncReserve, nRodReserve );

  // Сплошные объекты
	CNodeMap<SSolidElement> pattern( pBuildInfo->nMinFloor, pBuildInfo->nMaxFloor, pBuildInfo->nMaxX + 2, pBuildInfo->nMaxY + 2 );
	CSolidMap solidMap;
	MakeSolidMap( &rand, &solidMap, pBuildInfo->solidFragments, pattern, pBuildInfo->lgroups );

	for ( hash_map<int, CNodeMap<SSolidElement> >::const_iterator it = solidMap.begin(); it != solidMap.end(); ++it )
	{
		const vector<SSolidElement> &frags = it->second.GetValues();
		for ( int i = 0; i < frags.size(); ++i )
		{
			const SSolidElement &e = frags[i];
			for ( int j = 0; j < e.fragments.size(); ++j )
			{
				const SFragmentPos &frp = e.fragments[j];
				if ( !frp.pFr )
					continue;
				if ( IsValid( frp.pCPart->pGeometry ) && IsValid( frp.pCPart->pGeometry->pAIGeometry ) )
					SchemaAddSolid( pSchema, frp, *pGrid, frp.pCPart->pGeometry->pAIGeometry->GetRecordID(), cellarWalls );
				if ( IsValid( frp.pCPart->p2ndGeometry ) && IsValid( frp.pCPart->p2ndGeometry->pAIGeometry ) )
					SchemaAddSolid( pSchema, frp, *pGrid, frp.pCPart->p2ndGeometry->pAIGeometry->GetRecordID(), cellarWalls );
			}
		}
	}
	// Стены
	CNodeMap<SGridNode> wallGrid( pBuildInfo->nMinFloor, pBuildInfo->nMaxFloor, pBuildInfo->nMaxX + 2, pBuildInfo->nMaxY + 2 );
	vector<SLRNeighbs> neighbs( pBuildInfo->wallFragments.size() );
	MakeWallMap( &rand, &wallGrid, &pBuildInfo->wallFragments, &neighbs );

	for ( int i = 0; i < pBuildInfo->wallFragments.size(); ++i )
	{
		const SBuildFragment &fr = pBuildInfo->wallFragments[i];
		NDb::CConstructionPart *pCP = neighbs[i].pCPart;
		if ( !IsValid( pCP ) || !IsValid( pCP->pArmor ) )
			continue;
		if ( IsValid( pCP->pGeometry ) && IsValid( pCP->pGeometry->pAIGeometry ) )
			SchemaAddWall( pSchema, fr, *pGrid, pCP->pGeometry->pAIGeometry->GetRecordID(), cellarWalls, pCP->pArmor );
		if ( IsValid( pCP->p2ndGeometry ) && IsValid( pCP->p2ndGeometry->pAIGeometry ) )
			SchemaAddWall( pSchema, fr, *pGrid, pCP->p2ndGeometry->pAIGeometry->GetRecordID(), cellarWalls, pCP->pArmor );
	}
	//
	const vector<CJunction> &juncs = pSchema->GetJuncs();
	for ( vector<CJunction>::const_iterator pJ = juncs.begin(); pJ != juncs.end(); ++pJ )
	{
		if ( /*pJ->IsFilled() &&*/ (pJ->IsGround() || pJ->IsCellarWall()) )
			pGrid->SetIndestructible( SPoint3( pJ->ptJ.x, pJ->ptJ.y, pJ->ptJ.z ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UpdateBuildingStability( int nBuildingID, CBuildingGrid *pGrid, int nMaxIt )
{
	CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pLoader = NGScene::shareBuildings.Get( nBuildingID );
	pLoader.Refresh();
	NBuilding::CBuildInfo *pBuildInfo = pLoader->GetValue();
	if ( !pBuildInfo )
		return false;

	bool bExec = true;
	int  nIterations = -1;
	int nMaxIterations = nMaxIt - 1;
	NHPTimer::STime t;
	NHPTimer::GetTime( &t );

	CBuildingSchema *pSchema = pGrid->GetSchema();
	if ( !IsValid( pSchema ) )
	{
		NHPTimer::STime tschema;
		NHPTimer::GetTime( &tschema );
		pSchema = new CBuildingSchema();
		MakeBuildingSchema( pSchema, pGrid, pBuildInfo );
		pSchema->Start();
		pGrid->SetSchema( pSchema );
		float fSchemaTime = NHPTimer::GetTimePassed( &tschema );
		csSystem << "<font size=16pt>";
		csSystem << "Building stability update: Schema: " << fSchemaTime << endl;
		if ( ++nIterations == nMaxIterations )
			return true;
	}
	while ( bExec )
	{
		bExec = pSchema->Recalc( pGrid );
		if ( bExec )
			pSchema->Reset();
		if ( ++nIterations == nMaxIterations )
			break;
	}
//#ifdef _DEBUG
	double fTime = NHPTimer::GetTimePassed( &t );
	csSystem << "<font size=16pt>";
	csSystem << "Building stability update: " << nIterations + 1 << "iterations; Time: " << fTime << endl;
//#endif
	if ( !bExec )
		pGrid->SetSchema( 0 );
	return bExec;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static float fTotalHP = 0;
static float fMaxHP = 0;
static int nTotalNodes = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AddNodeHP( CBuildingGrid *pGrid, const SBuildFragment &fr, int nHashID, int nAGeometryID, 
											const CVec3 &ptPos, NDb::CRPGArmor *pArmor )
{
	// определяем, какие узлы присутсвуют в блоке
	CDGPtr< CPtrFuncBase<NAI::CGeometryInfo> > pSrc = NAI::shareAIModel.Get( nAGeometryID );
	pSrc.Refresh();
	NAI::CGeometryInfo *pGI = pSrc->GetValue();
	if ( !pGI || !pArmor )
	{
		ASSERT(0);
		return;
	}
	//
	hash_map<int, bool> parts;
	parts[nHashID] = true;
	SplitOptimized( &parts );
	//
	SDiscretePos dpos( 0, CVec3( ptPos.x, ptPos.y, ptPos.z * 4), fr.nRotationID );
	//
	for ( hash_map<int, bool>::iterator it = parts.begin(); it != parts.end(); ++it )
	{
		NAI::CGeometryInfo::CPieceMap::const_iterator ipiece = pGI->pieces.find( it->first );
		if ( !it->second || ipiece == pGI->pieces.end() )
			continue;
		int nID = FixSmallPieceID( pGI->pieces, it->first );
		int x, y, z;
		GetPieceCoords( nID, &x, &y, &z );
		CVec3 pt( x-1, y-1, z-1 );
		dpos.MoveAndRotate( &pt );
		//const float fVol = pow( ipiece->second.fVolume, 1.0f/3.0f );
		if ( pArmor->nDR > 0 )
		{
			float fHP = 2.0f * pArmor->nVP * ipiece->second.fVolume; //!
	//		ASSERT(fHP > 1);
			if ( !_isnan( fHP ) )
			{
#ifdef _DEBUG
				fTotalHP += fHP;
				fMaxHP = Max( fMaxHP, fHP );
				nTotalNodes++;
#endif
			}
			else
			{
	//			ASSERT(0);
				fHP = 10;
			}
			pGrid->AddHP( pt, ceil( fHP ) );
		}
		else
			pGrid->SetIndestructible( pt );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void BuildingHP( CBuildInfo *pBuildInfo, CBuildingGrid *pGrid )
{
	if ( !pGrid || !pBuildInfo )
	{
		ASSERT( 0 );
		return;
	}
	SRand rand( pGrid->GetSeed() );
	pGrid->Reset();
	if ( !pGrid->NeedComputeStability() )
		return;

	fMaxHP = 0;
	fTotalHP = 0;
	nTotalNodes = 0;
  // Сплошные объекты
	CNodeMap<SSolidElement> pattern( pBuildInfo->nMinFloor, pBuildInfo->nMaxFloor, pBuildInfo->nMaxX + 2, pBuildInfo->nMaxY + 2 );
	CSolidMap solidMap;
	MakeSolidMap( &rand, &solidMap, pBuildInfo->solidFragments, pattern, pBuildInfo->lgroups );

	for ( hash_map<int, CNodeMap<SSolidElement> >::const_iterator it = solidMap.begin(); it != solidMap.end(); ++it )
	{
		const vector<SSolidElement> &frags = it->second.GetValues();
		for ( int i = 0; i < frags.size(); ++i )
		{
			const SSolidElement &e = frags[i];
			for ( int j = 0; j < e.fragments.size(); ++j )
			{
				const SFragmentPos &frp = e.fragments[j];
				if ( !frp.pFr ) 
					continue;
				if ( IsValid( frp.pCPart->pGeometry ) && IsValid( frp.pCPart->pGeometry->pAIGeometry ) )
					AddNodeHP( pGrid, *frp.pFr, frp.nHashID, frp.pCPart->pGeometry->pAIGeometry->GetRecordID(), frp.nSubPos, frp.pCPart->pArmor );
				if ( IsValid( frp.pCPart->p2ndGeometry ) && IsValid( frp.pCPart->p2ndGeometry->pAIGeometry ) )
					AddNodeHP( pGrid, *frp.pFr, frp.nHashID, frp.pCPart->p2ndGeometry->pAIGeometry->GetRecordID(), frp.nSubPos, frp.pCPart->pArmor );
			}
		}
	}
	// Стены
	CNodeMap<SGridNode> wallGrid( pBuildInfo->nMinFloor, pBuildInfo->nMaxFloor, pBuildInfo->nMaxX + 2, pBuildInfo->nMaxY + 2 );
	vector<SLRNeighbs> neighbs( pBuildInfo->wallFragments.size() );
	MakeWallMap( &rand, &wallGrid, &pBuildInfo->wallFragments, &neighbs );

	for ( int i = 0; i < pBuildInfo->wallFragments.size(); ++i )
	{
		const SBuildFragment &fr = pBuildInfo->wallFragments[i];
		NDb::CConstructionPart *pCP = neighbs[i].pCPart;
		if ( !IsValid( pCP ) )
			continue;
		if ( IsValid( pCP->pGeometry ) && IsValid( pCP->pGeometry->pAIGeometry ) )
			AddNodeHP( pGrid, fr, fr.nSubBlockID, pCP->pGeometry->pAIGeometry->GetRecordID(), fr.ptPos, pCP->pArmor );
		if ( IsValid( pCP->p2ndGeometry ) && IsValid( pCP->p2ndGeometry->pAIGeometry ) )
			AddNodeHP( pGrid, fr, fr.nSubBlockID, pCP->p2ndGeometry->pAIGeometry->GetRecordID(), fr.ptPos, pCP->pArmor );
	}
#ifdef _DEBUG
	char buf[256];
	sprintf( buf, "Average building node HP = %f MaxHP = %f (%f,%d)\n", fTotalHP / nTotalNodes, fMaxHP, fTotalHP, nTotalNodes );
	OutputDebugString( buf );
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CBuildingInfoHold::CBuildingInfoHold( CBuildingGrid *pGrid, int nBuildingID, bool bSplit )
	:pBuildingGrid( pGrid ), bSplitParts(bSplit)
{
	pBuildInfo = NGScene::shareBuildings.Get( nBuildingID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SBuildingInfo& CBuildingInfoHold::GetInfo()
{
	bool bInfo = pBuildInfo.Refresh();
	bool bGrid = pBuildingGrid.Refresh();
	if ( bInfo || bGrid )
	{
		MakeBuilding( &info, *pBuildingGrid, pBuildInfo->GetValue(), bSplitParts );
	}
	return info;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NBuilding;
REGISTER_SAVELOAD_CLASS( 0xA0242120, CBuildingInfoHold );