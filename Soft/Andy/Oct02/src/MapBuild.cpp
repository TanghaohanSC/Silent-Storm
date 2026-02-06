#include "StdAfx.h"
#include "DG.h"
#include "MapBuild.h"
#include "Grid.h"
#include "Transform.h"
#include "BuildingGrid.h"
#include "..\Misc\BasicShare.h"
#include "BuildingInfo.h"
#include "BuildingClip.h"
#include "MapBuildTerrain.h"
#include "MakeBuilding.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataScenario.h"
#include "..\Misc\StrProc.h"
#include "aiWaypoint.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataObject.h"
#include "..\DBFormat\DataRPG.h"
#include "aiGrid.h"

const float WALL_HEIGHT = 2.5f;  // высота этажа
const int CLUE_SLOT_ID = 188;
const int EXPLOSION_ID = 189;

namespace NGScene
{
	extern CBasicShare<int, NBuilding::CBuildInfoLoader> shareBuildings;
}
CBasicShare<int, NAI::CWaypointLoader> shareWaypoints(133);
CBasicShare<int, NAI::CUnitAIInfoLoader> shareUnits(134);
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace
{
struct SExplosion
{
	SMapPosition pos;
	float fPower;
	float fRadius;
	bool bTerrAlignment;
	SExplosion(): bTerrAlignment(true), fRadius(10) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMapBuilder
{
	struct SLayerPlace
	{
		int nWidth, nHeight;
		CVec2 vOrigin;
		float fRotation;
	};

	SMapPosition posInitial;
	bool bTerrAlignmentInitial;
	CVec3 ptParentBuildingInitial;

	SMapInfo &info;
	vector<SExplosion> explosions;
	vector<int> flags;
	CPtr<NAI::IPathNetwork> pNet;
	int nRoom;
	SRand rand;
	hash_map<int, CObj<CMapWaypoint> > waypoints;
	int nMaxDepth;
	SLayerPlace rootPlace;
	NAI::CLayersGroup *pRootLayersGroup;
	bool bBuildTerrain;

	void AddSimpleElements( SMapInfo *pDst, SMapBuilding *pB, NDb::CTemplVariant *pVar, const SMapPosition &parent, bool bTerrAlign );
	void AddWaypoints( SMapInfo *pDst, NDb::CTemplVariant *pVar, const SMapPosition &parent, bool bTerrAlign );
	void WriteLightRooms( SMapInfo *pInfo, NBuilding::CBuildingGrid *pGrid );
	void AddBuildingObjects( int *pMinFloor, int *pMaxFloor, SRand *pRand, SMapInfo *pDst, 
		const vector<NBuilding::SBuildFragment> &frags, const SMapPosition &pos );
	bool CreateBuilding( SMapInfo *pDst, SMapBuilding *pRes, NBuilding::CBuildInfo *pInfo, const SMapPosition &pos );
	void CalcLayerPlace( SLayerPlace *pRes, const SMapPosition &pos, const NDb::CTemplate* pTemplate, int nBorder );
	int CreateLayersGroup( const SLayerPlace &place, int nFirstFloor );
	void CreateGrids( SMapBuilding *pRes, const vector<NBuilding::SBuildFragment> &fragments, 
		const SMapPosition &pos );
	void TraverseTemplateTree( NDb::CTemplate* pTemplate,	SMapInfo *pFree, 
		const SMapPosition &pos, int nDepth, NAI::SAlternativeGridInfo *pAltGrid,
		bool bTerrAlign, const CVec3 &ptParentBuilding, int nPlacementID = -1 );
	void TraverseTerrainTree( NDb::CTemplate* pTemplate, float fDZ,	const SMapPosition &pos, int nDepth, int nPlacementID = -1 );
	void ResolveRoutes( SMapInfo *pInfo );
	int GetGlobalRoomID( NBuilding::CBuildingGrid *pGrid, int nFloor, int nLocal )
	{
		if ( nLocal == 0 )
			return 0;
		int nTest = pGrid->GetRoomGlobal( nFloor, nLocal );
		if ( nTest == 0 )
		{
			nTest = nRoom;
			pGrid->AddRoom( nFloor, nLocal, nRoom++ );
		}
		return nTest;
	}

	void AddMapHole( list<SMapHole> *pList, const TPolygonsList &polygonsList, int nHeight, bool bVisible );
	void BuildHolesList();
	void GenerateWalls();

public:
	CMapBuilder( SMapInfo *pRes, NAI::IPathNetwork *_pNet, SRandomSeed sSeed = SRandomSeed() ): 
			info(*pRes), nRoom(1), pNet(_pNet), nMaxDepth(-1)
	{
		rand.seed = sSeed;

		posInitial.nFloor = 0;
		posInitial.fRotation = 0;
		posInitial.ptPos = VNULL3;

		bTerrAlignmentInitial = true;

		ptParentBuildingInitial = VNULL3;

		bBuildTerrain = true;
	}
	void SetParams( const vector<string> &strParams );
	void SetMaxDepth( int nDepth ) { nMaxDepth = nDepth; }
	void SetPos( const SMapPosition &pos ) { posInitial = pos; }
	void SetTerrAlignment( bool bTerr ) { bTerrAlignmentInitial = bTerr; }
	void SetParentBuildingPos( const CVec3 &ptPos ) { ptParentBuildingInitial = ptPos; }
	void SetBuildTerrain( bool bBuild ) { bBuildTerrain = bBuild; }

	bool BuildMap( int nPlacementID );
	bool BuildTerrain( int nPlacementID );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// Map generation
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void RotatePt( CVec3 *pVec, float fAngle )
{
  const float fAng = -ToRadian( fAngle );
  float fc = cos( fAng );
  float fs = sin( fAng );
	
  float x = fc * pVec->x + fs * pVec->y;
  pVec->y = -fs * pVec->x + fc * pVec->y;
  pVec->x = x;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void CalcPosition( SMapPosition *pRes, T *pFin, const SMapPosition &parent )
{
	CVec3 ptLocal( FP_GRID_STEP * pFin->ptPos.x, FP_GRID_STEP * pFin->ptPos.y, pFin->nFloor * WALL_HEIGHT );
	RotatePt( &ptLocal, parent.fRotation );
	pRes->nFloor = parent.nFloor + pFin->nFloor;
	pRes->ptPos = parent.ptPos + ptLocal;
	pRes->fRotation = parent.fRotation + pFin->fRotation;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void CalcPositionSubElement( SMapPosition *pRes, CVec3 ptSubElement, T *pFin, const SMapPosition &parent )
{
	CVec2 ptSrcPos = pFin->ptPos;
	RotatePt( &ptSubElement, pFin->nRotation );
	pFin->ptPos += CVec2( ptSubElement.x, ptSubElement.y );
	CalcPosition( pRes, pFin, parent );
	pFin->ptPos = ptSrcPos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::AddBuildingObjects( int *pMinFloor, int *pMaxFloor, SRand *pRand, SMapInfo *pDst, 
	const vector<NBuilding::SBuildFragment> &frags, const SMapPosition &pos )
{
	for ( int i = 0; i < frags.size(); ++i )
	{
		const NBuilding::SBuildFragment &fr = frags[i];
		NDb::CTConstructionPart *pTCP = NDb::GetTConstructionPart( fr.nConstructionPartID );
		if ( !pTCP )
			continue;
		CPtr<NDb::CConstructionPart> pCP = pTCP->CreateConstructionPart( pRand );
		if ( !IsValid( pCP ) )
			continue;
		*pMaxFloor = Max( *pMaxFloor, int(fr.ptPos.z + 0.5f) + pCP->nSizeZ );
		*pMinFloor = Min( *pMinFloor, int(floor(fr.ptPos.z)) );
		if ( !IsValid( pCP->pObject ) || fr.nSubBlockID != NBuilding::GetPartHashID( 1, 1, 1 ) )
			continue;
		//
		CVec3 ptLocal( FP_GRID_STEP * fr.ptPos.x, FP_GRID_STEP * fr.ptPos.y, fr.ptPos.z * WALL_HEIGHT );
		RotatePt( &ptLocal, pos.fRotation );
		SMapElement me;
		me.pos.nFloor = pos.nFloor + floor( fr.ptPos.z );
		me.pos.ptPos  = pos.ptPos + ptLocal;
		me.pos.fRotation = pos.fRotation + RotationIDToAngle( fr.nRotationID );
		me.pos.ptScale = CVec3(1,1,1);
		me.pObject = pCP->pObject;
		me.nRelFloor = floor( fr.ptPos.z );
		me.bTerrAlignment = false;
		me.bOpen = fr.nObjectFlags & NBuilding::OF_OPEN;
		pDst->items.push_back( me );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMapBuilder::CreateBuilding( SMapInfo *pDst, SMapBuilding *pRes, NBuilding::CBuildInfo *pInfo,	const SMapPosition &pos )
{
	if ( !pInfo || (pInfo->wallFragments.empty() && pInfo->solidFragments.empty()) || ( 0 == pInfo->nMaxX && 0 == pInfo->nMaxY ) )
		return false;
	pRes->mpos = pos;
	MakeMatrix( &pRes->pos, CVec3(1,1,1), pos.ptPos, ToRadian( pos.fRotation ) );
	pRes->pGrid = new NBuilding::CBuildingGrid;
	SRand brnd( pRes->pGrid->GetSeed() );
	// создание объектов, встроенных в здание (окна\двери..)
	pInfo->nMinFloor = 100;
	pInfo->nMaxFloor = -100;
	AddBuildingObjects( &pInfo->nMinFloor, &pInfo->nMaxFloor, &brnd, pDst, pInfo->solidFragments, pos );
	AddBuildingObjects( &pInfo->nMinFloor, &pInfo->nMaxFloor, &brnd, pDst, pInfo->wallFragments, pos );
	pInfo->nMinFloor = pInfo->nMinFloor == 100 ? 0 : pInfo->nMinFloor;
	pInfo->nMaxFloor = pInfo->nMaxFloor == -100 ? 0 : pInfo->nMaxFloor;
	//
	pRes->pGrid->Setup( pInfo->nMaxX, pInfo->nMaxY, // CRAP rotations are not accounted in max size calcs
		pInfo->nMinFloor, pInfo->nMaxFloor, VNULL2, pRes->pos );
	NBuilding::BuildingHP( pInfo, pRes->pGrid );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::AddSimpleElements( SMapInfo *pDst, SMapBuilding *pB, NDb::CTemplVariant *pVar, 
	const SMapPosition &parent, bool bTerrAlign )
{
  if ( !IsValid( pVar ) )
    return;
  {
		const int n = pVar->pFinalElements.size();
		for ( int i = 0; i < n; ++i )
		{
			NDb::CFinalElement *pFin = pVar->pFinalElements[i];
			if ( IsValid( pFin ) && IsValid( pFin->pObject ) )
			{
				//
				if ( IsValid( pFin->pObject->pObject ) )
				{
					CPtr<NDb::CObject> pObject( pFin->pObject->pObject->CreateObject( &rand, flags ) );
					if ( pObject->bIsDeploySpot )
					{
						SMapPosition pos;
						CalcPosition( &pos, pFin, parent );
						pos.ptPos.z += pFin->fDZ;
						pDst->deploySpots.push_back( SDeploySpot( pos ) );
					}
					if ( IsValid( pObject->pModels[0] ) )
					{
						SMapElement me;
						CalcPosition( &me.pos, pFin, parent );
						me.pos.ptPos.z += pFin->fDZ;
						me.pos.ptScale = pFin->ptScale;
						me.nRelFloor = pFin->nFloor;
						me.bLightmap = pFin->bLightmap;
						me.bOpen = pFin->bOpen;
						me.bTerrAlignment = bTerrAlign;
						me.pObject = pObject;
						me.nPassageZoneID = pFin->nPassageZoneID;
						me.nPassageObjectID = pFin->nPassageObjectID;
						me.nAPRadius = pFin->nAPRadius;
						pDst->items.push_back( me );
						if ( fabs2( pObject->pModels[0]->ptAmbientColor ) > FP_EPSILON )
						{
							SMapBuilding::SAmbientLight l;
							l.color = pObject->pModels[0]->ptAmbientColor;
							l.nFloor = pFin->nFloor;
							l.bLightmap = pFin->bLightmap;
							pB->lights.push_back( l );
						}
					}
				}
				else if ( IsValid( pFin->pObject->pRPGItem ) )
				{
					if ( CLUE_SLOT_ID == pFin->pObject->pRPGItem->GetRecordID() )
					{
						SClueSlot cs;
						CalcPosition( &cs.pos, pFin, parent );
						cs.pos.ptPos.z += pFin->fDZ;
						cs.bTerrAlignment = bTerrAlign;
						pDst->slots.push_back( cs );
					}
					else if ( EXPLOSION_ID == pFin->pObject->pRPGItem->GetRecordID() )
					{
						SExplosion me;
						CalcPosition( &me.pos, pFin, parent );
						me.pos.ptPos.z += pFin->fDZ;
						me.fPower = pFin->fPower;
						me.fRadius = pFin->fRadius;
						me.bTerrAlignment = bTerrAlign;
						explosions.push_back( me );
					}
					else
					{
						SMapRPGElement me;
						CalcPosition( &me.pos, pFin, parent );
						me.pos.ptPos.z += pFin->fDZ;
						me.pos.ptScale = pFin->ptScale;
						me.nRelFloor = pFin->nFloor;
						me.bOpen = pFin->bOpen;
						me.bTerrAlignment = bTerrAlign;
						me.pItem = pFin->pObject->pRPGItem;
						pDst->rpgitems.push_back( me );
					}
				}
			}
		}
  }
  //
  {
		const int n = pVar->pUnits.size();
		for ( int i = 0; i < n; ++i )
		{
			NDb::CUnit *pUnit = pVar->pUnits[i];
			if ( IsValid( pUnit ) )
			{
				SMapUnit mu;
				CalcPosition( &mu.pos, pUnit, parent );
				mu.nUnitID = pUnit->GetRecordID();
				mu.pPers = pUnit->pMonster;
				mu.bSlot = pUnit->bClueSlot;
				if ( pUnit->bClueSlot || pUnit->bClueInventorySlot )
				{
					SClueSlot cs;
					CalcPosition( &cs.pos, pUnit, parent );
					cs.nUnitID = pUnit->GetRecordID();
					cs.pPers = pUnit->pMonster;
					cs.bPersSlot = pUnit->bClueSlot;
					cs.bInventorySlot = pUnit->bClueInventorySlot;
					pDst->slots.push_back( cs );
				}
				pDst->units.push_back( mu );
			}
		}
  }
  //
  {
		const int n = pVar->explosions.size();
		for ( int i = 0; i < n; ++i )
		{
			NDb::CExplosion *pExpl = pVar->explosions[i];
			if ( IsValid( pExpl ) )
			{
				SExplosion me;
				CalcPosition( &me.pos, pExpl, parent );
				me.pos.ptPos.z += pExpl->fDZ;
				me.fPower = pExpl->fPower;
				explosions.push_back( me );
			}
		}
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::WriteLightRooms( SMapInfo *pInfo, NBuilding::CBuildingGrid *pGrid )
{
//	for ( list<SMapElement>::iterator i = pInfo->items.begin(); i != pInfo->items.end(); ++i )
//		if ( i->bTerrAlignment ) // для окон и дверей комнаты не прописываем
//			i->nRoomID = GetGlobalRoomID( pGrid, i->nRelFloor, i->nRoomID );		
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void SetStuffOnLayer( T *pDst, T *pSrc, int nMaxFloor )
{
	for ( T::iterator i = pSrc->begin(); i != pSrc->end(); )
	{
		T::iterator k = i++;
		int nFloor = k->pos.nFloor;
		ASSERT( nFloor >= nMaxFloor );
		if ( nFloor <= nMaxFloor )
			pDst->splice( pDst->end(), *pSrc, k );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void SetObjectsOnLayer( T *pDst, T *pSrc, int nMaxFloor )
{
	for ( T::iterator i = pSrc->begin(); i != pSrc->end(); )
	{
		T::iterator k = i++;
		if ( !IsValid( k->pObject ) )
			continue;
		int nFloor = k->pos.nFloor;
		ASSERT( nFloor >= nMaxFloor );
		if ( nFloor <= nMaxFloor )
		{
			pDst->splice( pDst->end(), *pSrc, k );
			NDb::ETrafic trafic = NDb::T_GO_OVER;
			NDb::CContainerModel *pCM = k->pObject->pModels[0];
			if ( IsValid( pCM ) && IsValid( pCM->pModel ) )
			{
				NDb::CGeometry *pG = pCM->pModel->pGeometry;
				if ( IsValid( pG ) && IsValid( pG->pAIGeometry ) )
					trafic = pG->pAIGeometry->traficability;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void SetWaypointsOnLayer( T *pDst, T *pSrc, int nMaxFloor )
{
	for ( T::iterator i = pSrc->begin(); i != pSrc->end(); )
	{
		T::iterator j = i++;
		T::reference k = *j;
		int nFloor = k->pos.nFloor;
		ASSERT( nFloor >= nMaxFloor );
		if ( nFloor <= nMaxFloor )
			pDst->splice( pDst->end(), *pSrc, j );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetOnLayer( SMapInfo *pInfo, SMapInfo *pFree, int nMaxFloor )
{
	for ( int k = 0; k < pFree->deploySpots.size(); ++k )
		pInfo->deploySpots.push_back( pFree->deploySpots[k] );
	pFree->deploySpots.clear();
	for ( list<SMapRPGElement>::const_iterator i = pFree->rpgitems.begin(); i != pFree->rpgitems.end(); ++i )
		pInfo->rpgitems.push_back( *i );
	for ( vector<SClueSlot>::const_iterator i = pFree->slots.begin(); i != pFree->slots.end(); ++i )
		pInfo->slots.push_back( *i );
	pFree->slots.clear();
	pFree->rpgitems.clear();
	pInfo->scripts.insert( pInfo->scripts.end(), pFree->scripts.begin(), pFree->scripts.end() );
	pFree->scripts.clear();
	SetObjectsOnLayer( &pInfo->items, &pFree->items, nMaxFloor );
	SetStuffOnLayer( &pInfo->units, &pFree->units, nMaxFloor );
	SetWaypointsOnLayer( &pInfo->waypoints, &pFree->waypoints, nMaxFloor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void Transfer( SMapInfo *pDst, SMapInfo *pSrc )
{
	pDst->items.splice( pDst->items.end(), pSrc->items );
	pDst->units.splice( pDst->units.end(), pSrc->units );
	pDst->deploySpots.insert( pDst->deploySpots.end(), pSrc->deploySpots.begin(), pSrc->deploySpots.end() );
	pDst->rpgitems.insert( pDst->rpgitems.end(), pSrc->rpgitems.begin(), pSrc->rpgitems.end() );
	pDst->scripts.insert( pDst->scripts.end(), pSrc->scripts.begin(), pSrc->scripts.end() );
	pDst->slots.insert( pDst->slots.end(), pSrc->slots.begin(), pSrc->slots.end() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::CalcLayerPlace( SLayerPlace *pRes, const SMapPosition &pos, const NDb::CTemplate* pTemplate, int nBorder )
{
	float fBorder = nBorder * FP_GRID_STEP;
	CVec2 ptOrigin(pos.ptPos.x + fBorder, pos.ptPos.y + fBorder );
	CVec3 ptShift( -FP_GRID_STEP, -FP_GRID_STEP, 0);
	RotatePt( &ptShift, pos.fRotation );
	pRes->nWidth = pTemplate->nWidth + 1 + 2 - 2 * nBorder;
	pRes->nHeight = pTemplate->nHeight + 1 + 2 - 2 * nBorder;
	pRes->vOrigin = ptOrigin + CVec2(ptShift.x, ptShift.y);
	pRes->fRotation = ToRadian( pos.fRotation );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CMapBuilder::CreateLayersGroup( const SLayerPlace &place, int nFirstFloor )
{
	OutputDebugString(" CREATING GROUP \n");
	return pNet->CreateLayersGroup(
		place.nWidth, place.nHeight,
		place.vOrigin, place.fRotation, nFirstFloor
		);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::CreateGrids( SMapBuilding *pRes, const vector<NBuilding::SBuildFragment> &fragments, 
	const SMapPosition &pos )
{
	SRand crapSeed;
	for ( int i = 0; i < fragments.size(); ++i )
	{
		const NBuilding::SBuildFragment &frag = fragments[i];
		if ( frag.nSubBlockID != NBuilding::GetPartHashID( 1, 1, 1 ) && frag.nSubBlockID != 0 )
			continue;
		NDb::CTConstructionPart *pTCP = NDb::GetTConstructionPart( frag.nConstructionPartID );
		if ( !pTCP )
			continue;
		CPtr<NDb::CConstructionPart> pCP = pTCP->CreateConstructionPart( &crapSeed );
		if ( !IsValid( pCP ) || !IsValid( pCP->pGeometry ) || !IsValid( pCP->pGeometry->pAIGeometry ) )
			continue;
		CPtr<NDb::CAIGeometry> pAIG = pCP->pGeometry->pAIGeometry;
		NDb::ETrafic trafic = pAIG->traficability;
		if ( trafic == NDb::T_GRID )
		{
			CVec3 ptOrigin( frag.ptPos.x * FP_GRID_STEP, frag.ptPos.y * FP_GRID_STEP, frag.ptPos.z * WALL_HEIGHT );
			RotatePt( &ptOrigin, pos.fRotation );
			ptOrigin += pos.ptPos;
			CVec3 ptShift( -FP_GRID_STEP, -FP_GRID_STEP, 0 );
			RotatePt( &ptShift, pos.fRotation );
			int nWidth = pCP->nSizeX * 2 + 1 + 2;
			int nHeight = pCP->nSizeY * 2 + 1 + 2;
			if ( frag.nRotationID == SDiscretePos::TURN_90 ||  frag.nRotationID == SDiscretePos::TURN_270 )
				swap( nWidth, nHeight );
			CVec3 ptLOrigin( ptOrigin + ptShift );
//			int nNewLayer = pNet->CreateLayer(
//				nWidth, nHeight,
//				CVec2( ptLOrigin.x, ptLOrigin.y ), 
//				ToRadian( (float)pos.nRotation ), pos.nFloor + floor( frag.ptPos.z )
//				);
//			pRes->solidHGs[i/*frag.nFragmentID*/] = SMapBuilding::SSpecialSolidInfo( pNet->GetDefaultHGroup( nNewLayer ), nNewLayer );
		}
//		else if ( trafic == NDb::T_BREAK )
//		{
//			int nLayer = pRes->GetStorey( floor( frag.ptPos.z ) ).nLayer;
//			pRes->solidHGs[i/*frag.nFragmentID*/] = SMapBuilding::SSpecialSolidInfo( pNet->GetFloor( nLayer ), -1 );
//		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::TraverseTemplateTree( NDb::CTemplate* pTemplate, SMapInfo *pFree, 
	const SMapPosition &pos, int nDepth, NAI::SAlternativeGridInfo *pAltGrid, 
	bool bParentTerrAlign, const CVec3 &ptParentBuilding, int nPlacementID )
{
  if ( !IsValid( pTemplate ) || (nMaxDepth >= 0 && nDepth > nMaxDepth ) )
    return;
  ASSERT( !pTemplate->variants.empty() );
  
  NDb::CTemplVariant *pVar = NDb::GetTemplVariant( pTemplate, flags, nPlacementID, &rand );
	if ( !IsValid( pVar ) )
		return;

	// ландшафт
  const int nRecW = pTemplate->nWidth;
  const int nRecH = pTemplate->nHeight;
	
	if ( bBuildTerrain && IsValid( pNet ) && nDepth > 1 )
		BlendTerrainInfo( &info.terrain, pVar->GetRecordID(), pos.ptPos, ToRadian( pos.fRotation ), &rand, flags );

	const float fWHalf = 0.5f * FP_GRID_STEP * nRecW;
	const float fHHalf = 0.5f * FP_GRID_STEP * nRecH;

	// есть ли необходимость выравнивать по террейну объекты?
	CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pBuildInfo = NGScene::shareBuildings.Get( pVar->GetRecordID() );
	pBuildInfo.Refresh();
	NBuilding::CBuildInfo *pBInfo = pBuildInfo->GetValue();
	bool bTerrAlign = bParentTerrAlign && pBInfo->wallFragments.empty() && pBInfo->solidFragments.empty();
	CVec3 ptBuilding( fWHalf, fHHalf, 0 );
	if ( bParentTerrAlign )
	{
		RotatePt( &ptBuilding, pos.fRotation );
		ptBuilding.x += pos.ptPos.x;
		ptBuilding.y += pos.ptPos.y;
	}
	else
		ptBuilding = ptParentBuilding;

	// Calc layer place
	SLayerPlace layerPlace;
	int nTemplateBorder = nDepth == 1 ? Min( Max(pVar->nBorder,0), Min( nRecW / 2, nRecH / 2 ) )  : 0;
	CalcLayerPlace( &layerPlace, pos, pTemplate, nTemplateBorder );
	if ( nDepth == 1 )
	{
		rootPlace = layerPlace;
		if ( IsValid( pNet ) )
			CreateLayersGroup( layerPlace, -2 );
	}
	// traverse childs
  const int nmax = pVar->rects.size();
	SMapInfo nested;
	NAI::SAlternativeGridInfo nestedAltGrids;
	for ( int i = 0; i < nmax; ++i )
	{
		// преобразование координат при поворотах
		NDb::CRectangle *pRec = pVar->rects[i];
		if ( !IsValid( pRec ) )
			continue;
		SMapPosition child;
		CVec3 ptLocal( pRec->ptCenter.x, pRec->ptCenter.y, pRec->nFloor * WALL_HEIGHT + pRec->fDZ );
		RotatePt( &ptLocal, pos.fRotation );
		child.ptPos = pos.ptPos + ptLocal;
		child.nFloor = pos.nFloor + pRec->nFloor;
		child.fRotation = int( pos.fRotation + pRec->fRotation + 360 * 100 ) % 360;
		TraverseTemplateTree( pRec->pTemplate, &nested, child, nDepth + 1, &nestedAltGrids, bTerrAlign, ptBuilding );
	}
	// скрипты
	if ( IsValid( pVar->pScript ) )
		nested.scripts.push_back( pVar->pScript.GetPtr() );
	// здания
	SMapBuilding b;
	SMapPosition buildingPos( pos );
	buildingPos.ptPos.z += GetMeterHeightCheck( info.terrain, ptBuilding.x, ptBuilding.y );
  // объекты и юниты
	SMapPosition posObjects = pos;
	posObjects.ptPos.z = bTerrAlign ? pos.ptPos.z : buildingPos.ptPos.z;
  AddSimpleElements( &nested, &b, pVar, posObjects, bTerrAlign );
	AddWaypoints( &nested, pVar, posObjects, bTerrAlign );
	ResolveRoutes( &nested );

	int nNewGroup = -1;
	if ( IsValid( pNet ) && CreateBuilding( &nested, &b, pBInfo, buildingPos ) )
	{
		b.pVariant = pVar;
		b.pGrid->SetBaseFloor( pos.nFloor );
		// create AI layers and mark rooms on them
		WriteLightRooms( &nested, b.pGrid );
		float fSinL = sin( layerPlace.fRotation ), fSinR = sin( rootPlace.fRotation ),
			fCosR = cos( rootPlace.fRotation );
		const float fEpsilon = 1e-5f;
		bool bSkip = true;
		if ( nDepth > 1 )
			bSkip = fabs( fSinL - fSinR ) < fEpsilon || fabs( fSinL + fSinR ) < fEpsilon ||
								 fabs( fSinL - fCosR ) < fEpsilon || fabs( fSinL + fCosR ) < fEpsilon;
		if ( !bSkip )
		{
			int nLayersCount = pBInfo->nMaxFloor + 1;
			if ( pBInfo->nMinFloor < 0 )
				nLayersCount -= pBInfo->nMinFloor;
			nNewGroup = CreateLayersGroup( layerPlace, pBInfo->nMinFloor + pos.nFloor );
		}
		for ( int nRelFloor = pBInfo->nMinFloor; nRelFloor <= pBInfo->nMaxFloor; ++nRelFloor )
		{
			int nZ = nRelFloor + pos.nFloor; // абсолютный номер этажа
			SetOnLayer( &info, &nested, nZ );	
			//
			b.stories.push_back( SMapBuilding::SStorey( nRelFloor, nZ ) );
		}
		CreateGrids( &b, pBInfo->solidFragments, pos );
		info.buildings.push_back( b );
		//ladders
		vector<NBuilding::SLadder> &lads = pBInfo->ladders;
		if( !lads.empty() )
		{
			for ( int i = 0; i < lads.size(); ++i )
			{
				NBuilding::SLadder &lad = lads[i];
				if ( lad.nID <= 0 )
					continue;
				//const float fLadFloors = lad.nHeight * NAI::F_LADDER_STEP / NBuilding::WALL_HEIGHT;
				int x = lad.pos.ptMove.x + 1; // CRAP вычитается в aiGrid
				int y = lad.pos.ptMove.y + 1;
				if ( nNewGroup >= 0 )
					pNet->CreateLadder( x, y, lad.nHeight, lad.pos.nRotation, nNewGroup, lad.pos.ptMove.z );
				else
				{
					CVec2 vDisplacement = layerPlace.vOrigin - rootPlace.vOrigin;
					x += vDisplacement.x / FP_GRID_STEP;
					y += vDisplacement.y / FP_GRID_STEP;
					// CRAP{
					if ( x < 0 )
						x = 0;
					if ( y < 0 )
						y = 0;
					if ( x >= pTemplate->nWidth )
						x = pTemplate->nWidth - 1;
					if ( y >= pTemplate->nWidth )
						y = pTemplate->nWidth - 1;
					// }CRAP
					pNet->CreateLadder( x, y, lad.nHeight, lad.pos.nRotation, 0, lad.pos.ptMove.z );
				}
			}
		}
	}
	// add layer to exceptions list
	if ( nDepth != 1 && nNewGroup != -1 )
	{
		nestedAltGrids.nLayersGroup = nNewGroup;
		pAltGrid->children.push_back( nestedAltGrids );
	}
	else
		pAltGrid->children.insert( pAltGrid->children.end(), nestedAltGrids.children.begin(), nestedAltGrids.children.end() );
	// if marked as grid creating then should create grid
	if ( nDepth == 1 )
	{
		//
		SetOnLayer( &info, &nested, 0 );
		SetOnLayer( &info, &nested, 1000 );
		ResolveRoutes( &info );
	}
	Transfer( pFree, &nested );
	if ( nDepth == 1 )
	{
		ASSERT( pFree->items.empty() );
		ASSERT( pFree->units.empty() );
	}
	///////////////
	//pNet->CreateLadder(3, 3, 2, 1, 2);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::TraverseTerrainTree( NDb::CTemplate* pTemplate, float fDZ,	const SMapPosition &pos, int nDepth, int nPlacementID )
{
	if ( !IsValid( pTemplate ) || (nMaxDepth >= 0 && nDepth > nMaxDepth ) )
		return;
	NDb::CTemplVariant *pVar = NDb::GetTemplVariant( pTemplate, flags, nPlacementID, &rand );
	if ( !IsValid( pVar ) )
		return;
	if ( nDepth > 1 )
		BlendTerrainInfo( &info.terrain, pVar->GetRecordID(), pos.ptPos, ToRadian( pos.fRotation ), &rand, flags );
	for ( int i = 0; i < pVar->rects.size(); ++i )
	{
		// преобразование координат при поворотах
		NDb::CRectangle *pRec = pVar->rects[i];
		if ( !IsValid( pRec ) )
			continue;
		SMapPosition child;
		CVec3 ptLocal( pRec->ptCenter.x, pRec->ptCenter.y, pRec->nFloor * WALL_HEIGHT + pRec->fDZ );
		RotatePt( &ptLocal, pos.fRotation );
		child.ptPos = pos.ptPos + ptLocal;
		child.nFloor = pos.nFloor + pRec->nFloor;
		child.fRotation = int( pos.fRotation + pRec->fRotation + 360 * 100 ) % 360;
		TraverseTerrainTree( pRec->pTemplate, pRec->fDZ, child, nDepth + 1 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::AddMapHole( list<SMapHole> *pList, const TPolygonsList &polygonsList, int nHeight, bool bVisible )
{
	SMapHole &sHole = *pList->insert( pList->end() );
	sHole.nFloor = info.nBaseTerrainFloor;
	sHole.nHeight = nHeight;
	sHole.bVisible = bVisible;
	sHole.polygonsList = polygonsList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::BuildHolesList()
{
	info.nBaseTerrainFloor = 0;
	
	list<SMapHole> &holesList = info.holesList;
	const STerrainInfo &sInfo = info.terrain;
	for( vector<STerrainHole>::const_iterator iTempHole = sInfo.holes.begin(); iTempHole != sInfo.holes.end(); iTempHole++ )
	{
		bool bHandled = false;
		list<SMapHole> newHolesList;

		TPolygonsList sourcePolygonsList;
		sourcePolygonsList.push_back( iTempHole->vPolygon );
		for ( list<SMapHole>::iterator iTempGroupHole = holesList.begin(); iTempGroupHole != holesList.end(); iTempGroupHole++ )
		{
			TPolygonsList intPolygonsList, subPolygonsList, outClippedPolygonsList;
			ClipPolygon( iTempGroupHole->polygonsList, sourcePolygonsList, &intPolygonsList, &subPolygonsList );
			ClipPolygon( sourcePolygonsList, iTempGroupHole->polygonsList, 0, &outClippedPolygonsList );

			if ( !intPolygonsList.empty() )
				AddMapHole( &newHolesList, intPolygonsList, iTempGroupHole->nHeight + iTempHole->nHeight, iTempGroupHole->bVisible == iTempHole->bVisible );
			if ( !subPolygonsList.empty() )
				AddMapHole( &newHolesList, subPolygonsList, iTempGroupHole->nHeight, iTempGroupHole->bVisible );

			sourcePolygonsList = outClippedPolygonsList;
		}

		if ( !sourcePolygonsList.empty() )
			AddMapHole( &newHolesList, sourcePolygonsList, iTempHole->nHeight, iTempHole->bVisible );
		holesList = newHolesList;
	}

	GenerateWalls();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float FP_EPS = 1e-05f;
////////////////////////////////////////////////////////////////////////////////////////////////////
static inline bool IsEqual( const CVec2 &v1, const CVec2 &v2, float fEps = FP_EPS )
{
	return fabs( v1 - v2 ) < fEps;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SEdge
{
	CVec2 vBeg;
	CVec2 vEnd;

	SEdge() {}
	SEdge( const CVec2 &_vBeg, const CVec2 &_vEnd ): vBeg( _vBeg ), vEnd( _vEnd ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::GenerateWalls()
{
	for ( list<SMapHole>::const_iterator iHole = info.holesList.begin(); iHole != info.holesList.end(); iHole++ )
	{
		int nBaseHeight = iHole->nHeight;
		list<SEdge> edgesList;
		const SMapHole &sHole = *iHole;

		for ( TPolygonsList::const_iterator iPolygon = sHole.polygonsList.begin(); iPolygon != sHole.polygonsList.end(); iPolygon++ )
		{
			const vector<CVec2> &pointsSet = *iPolygon;
			for ( int nTemp = 0; nTemp < pointsSet.size(); nTemp++ )
			{
				const CVec2 &vBeg = pointsSet[nTemp];
				const CVec2 &vEnd = pointsSet[( nTemp + 1 ) % pointsSet.size()];

				edgesList.push_back( SEdge( vBeg, vEnd ) );
			}
		}

		list<SMapHole>::const_iterator iNext = iHole;
		iNext++;

		for ( list<SMapHole>::const_iterator iHole = info.holesList.begin(); iHole != iNext; iHole++ )
		{
			const SMapHole &sHole = *iHole;
			for ( TPolygonsList::const_iterator iPolygon = sHole.polygonsList.begin(); iPolygon != sHole.polygonsList.end(); iPolygon++ )
			{
				const vector<CVec2> &pointsSet = *iPolygon;
				for ( int nTemp = 0; nTemp < pointsSet.size(); nTemp++ )
				{
					const CVec2 &vBeg = pointsSet[nTemp];
					const CVec2 &vEnd = pointsSet[( nTemp + 1 ) % pointsSet.size()];

					for ( list<SEdge>::iterator iEdge = edgesList.begin(); iEdge != edgesList.end(); )
					{
						SEdge &sEdge = *iEdge;
						if ( IsEqual( sEdge.vBeg, vEnd ) && IsEqual( sEdge.vEnd, vBeg ) )
							iEdge = edgesList.erase( iEdge );
						else
							iEdge++;
					}
				}
			}
		}
		for ( list<SMapHole>::const_iterator iHole = iNext; iHole != info.holesList.end(); iHole++ )
		{
			const SMapHole &sHole = *iHole;
			for ( TPolygonsList::const_iterator iPolygon = sHole.polygonsList.begin(); iPolygon != sHole.polygonsList.end(); iPolygon++ )
			{
				const vector<CVec2> &pointsSet = *iPolygon;
				for ( int nTemp = 0; nTemp < pointsSet.size(); nTemp++ )
				{
					const CVec2 &vBeg = pointsSet[nTemp];
					const CVec2 &vEnd = pointsSet[( nTemp + 1 ) % pointsSet.size()];

					for ( list<SEdge>::iterator iEdge = edgesList.begin(); iEdge != edgesList.end(); )
					{
						SEdge &sEdge = *iEdge;
						if ( IsEqual( sEdge.vBeg, vEnd ) && IsEqual( sEdge.vEnd, vBeg ) )
						{
							SMapWall &sWall = *info.wallsList.insert( info.wallsList.end() );

							sWall.nHeightMin = sHole.nHeight;
							sWall.nHeightMax = nBaseHeight;
							sWall.vBeg = sEdge.vBeg;
							sWall.vEnd = sEdge.vEnd;

							iEdge = edgesList.erase( iEdge );
						}
						else
							++iEdge;
					}
				}
			}
		}

		for ( list<SEdge>::iterator iEdge = edgesList.begin(); iEdge != edgesList.end(); iEdge++ )
		{
			SEdge &sEdge = *iEdge;

			SMapWall &sWall = *info.wallsList.insert( info.wallsList.end() );

			sWall.nHeightMin = 0;
			sWall.nHeightMax = nBaseHeight;
			sWall.vBeg = sEdge.vBeg;
			sWall.vEnd = sEdge.vEnd;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void FixHeights( STerrainInfo &terrain, T *pSet )
{
	for ( T::iterator i = pSet->begin(); i != pSet->end(); ++i )
		if ( i->bTerrAlignment )
			i->pos.ptPos.z += GetMeterHeightCheck( terrain, i->pos.ptPos.x, i->pos.ptPos.y );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void FixObjHeights( STerrainInfo &terrain, T *pSet )
{
	for ( T::iterator i = pSet->begin(); i != pSet->end(); ++i )
		if ( (*i)->bTerrAlign )
			(*i)->pos.ptPos.z += GetMeterHeightCheck( terrain, (*i)->pos.ptPos.x, (*i)->pos.ptPos.y );	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::SetParams( const vector<string> &strParams )
{
	CDBTable<NDb::CAttribute> *pAttrTable = NDatabase::GetTable<NDb::CAttribute>();
	if ( pAttrTable )
	{
		hash_map<string, int> attrmap;
		CDBIterator<NDb::CAttribute> it( *pAttrTable );
		while ( it.MoveNext() )
			attrmap[it.Get()->szName] = it.Get()->GetRecordID();
		for ( int i = 0; i < strParams.size(); ++i )
		{
			hash_map<string, int>::const_iterator it = attrmap.find( strParams[i] );
			if ( it != attrmap.end() )
				flags.push_back( it->second );
		}
	}
	sort( flags.begin(), flags.end() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::AddWaypoints( SMapInfo *pDst, NDb::CTemplVariant *pVar, const SMapPosition &parent, bool bTerrAlign )
{
	if ( !IsValid( pVar ) )
		return;
	// добавляем waypoints из текущей расстановки
	for ( int i = 0; i < pVar->waypoints.size(); ++i )
	{
		NDb::CWaypoint *pdbW = pVar->waypoints[i];
		if ( !IsValid( pdbW ) || !IsValid( pdbW->pName ) )
		{
			ASSERT(0);
			continue;
		}
		CDGPtr< CPtrFuncBase<NAI::CWaypoint> > pWPLoader = shareWaypoints.Get( pdbW->GetRecordID() );
		if ( !IsValid( pWPLoader ) )
			continue;
		pWPLoader.Refresh();
		CPtr<NAI::CWaypoint> pW = pWPLoader->GetValue();
		if ( !IsValid( pW ) )
			continue;
		CMapWaypoint *pMW = new CMapWaypoint;
		pMW->pName = pdbW->pName;
		pMW->commands = pW->commands;
		pMW->bTerrAlign = bTerrAlign;
		CalcPosition( &pMW->pos, pW.GetPtr() , parent );
		info.waypoints.push_back( pMW );
		waypoints[pdbW->pName->GetRecordID()] = pMW;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapBuilder::ResolveRoutes( SMapInfo *pInfo )
{
	// обходим всех добавленных юнитов и проверяем их маршруты, уже установленные флажки пропускаем, 
	// если на карте появились новые подходящие флажки - вставляем в нужном порядке в маршрут
	for ( list<SMapUnit>::iterator i = pInfo->units.begin(); i != pInfo->units.end(); ++i )
	{
		SMapUnit &u = *i;
		//
		CDGPtr< CPtrFuncBase<NAI::CUnitAIInfo> > pUnitLoader = shareUnits.Get( u.nUnitID );
		if ( !IsValid( pUnitLoader ) )
			continue;
		pUnitLoader.Refresh();
		CPtr<NAI::CUnitAIInfo> pU = pUnitLoader->GetValue();
		if ( !IsValid( pU ) || pU->routes.empty() )
			continue;
		//
		hash_map<int, CPtr<CMapWaypoint> > route;
		for ( int j = 0; j < u.route.size(); ++j )
			route[u.route[j]->pName->GetRecordID()] = u.route[j];
		u.route.clear();
		//
		for ( int j = 0; j < pU->routes.front().waypoints.size(); ++j )
		{
			NDb::CWaypointName *pWN = NDb::GetWaypointName( pU->routes.front().waypoints[j] );
			if ( !IsValid( pWN ) )
				continue;
			CPtr<CMapWaypoint> &pOldMW = route[pWN->GetRecordID()];
			if ( IsValid( pOldMW ) )
				u.route.push_back( pOldMW );
			else
			{
				CObj<CMapWaypoint> &pMW = waypoints[pWN->GetRecordID()];
				if ( IsValid( pMW ) )
					u.route.push_back( pMW.GetPtr() );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMapBuilder::BuildMap( int nPlacementID )
{
  CDBTable<NDb::CTemplVariant> *pVarTable = NDatabase::GetTable<NDb::CTemplVariant>();
  if ( !pVarTable )
    return false;
	NDb::CTemplVariant *pVar = pVarTable->GetRecord( nPlacementID );
	if ( !pVar || !pVar->pTemplate )
	{
		ASSERT( 0 );
		return false;
	}
	bool bRet = true;

	if ( bBuildTerrain )
	{
		info.terrain.nWidth  = pVar->pTemplate->nWidth;// + 1;
		info.terrain.nHeight = pVar->pTemplate->nHeight;// + 1;
		bRet = LoadRootTerrain( nPlacementID, &info.terrain, &rand, flags );
	}

	info.pDefaultLight = pVar->pLight;
	
	SMapInfo freeInfo;
	NAI::SAlternativeGridInfo altGrids;
	TraverseTemplateTree( pVar->pTemplate, &freeInfo, posInitial, 1, &altGrids, bTerrAlignmentInitial, ptParentBuildingInitial, nPlacementID );
	altGrids.nLayersGroup = 0;
	if ( IsValid( pNet ) )
	{
		FixHeights( info.terrain, &info.items );
		FixHeights( info.terrain, &explosions );
		FixHeights( info.terrain, &info.deploySpots );
		FixHeights( info.terrain, &info.rpgitems );
		FixHeights( info.terrain, &info.slots );
		FixObjHeights( info.terrain, &info.waypoints );
	}
	// Заданные в редакторе разрушения 
	for ( int j = 0; j < explosions.size(); ++j )
		for ( int i = 0; i < info.buildings.size(); ++i )
		{
			info.buildings[i].pGrid->Explode( explosions[j].pos.ptPos, explosions[j].fPower, explosions[j].fRadius );
		}
	// Build holes, add HG for each terrain part
	if ( IsValid( pNet ) )
		BuildHolesList();
	// 
	if ( IsValid( pNet ) )
	{
		// create special layers to resolve walking on different grids troubles
		pNet->CreateAlternativeGrids( altGrids );
	}
	//
	ClearTerrainCache();

	// output borders
	CVec2 vMapSize( (rootPlace.nWidth - 1) * FP_GRID_STEP, (rootPlace.nHeight - 1) * FP_GRID_STEP );
	CVec2 vMapBorder( rootPlace.vOrigin );
	info.sMapSafeZone = CTRect<float>( vMapBorder.x, vMapBorder.y, vMapSize.x + vMapBorder.x, vMapSize.y + vMapBorder.y );

	// add borders to terrain
	if ( bBuildTerrain )
	{
		float fX1 = rootPlace.vOrigin.x - FP_GRID_STEP;
		float fX2 = rootPlace.vOrigin.x + rootPlace.nWidth * FP_GRID_STEP;
		float fY1 = rootPlace.vOrigin.y - FP_GRID_STEP;
		float fY2 = rootPlace.vOrigin.y + rootPlace.nHeight * FP_GRID_STEP;
		NDb::CMaterial *pSpot = NDb::GetMaterial(1904); // CRAP
		for ( int k = 0; k < rootPlace.nWidth + 1; ++k )
		{
			float fX = rootPlace.vOrigin.x + (k - 1) * FP_GRID_STEP;
			info.terrain.spots.push_back( STerrainSpot( pSpot, CVec2( fX, fY1 ), CVec2(FP_GRID_STEP, FP_GRID_STEP*0.5f), ToRadian(0.0f) ) );
			info.terrain.spots.push_back( STerrainSpot( pSpot, CVec2( fX + FP_GRID_STEP, fY2 ), CVec2(FP_GRID_STEP, FP_GRID_STEP*0.5f), ToRadian(180.0f) ) );
		}
		for ( int k = 0; k < rootPlace.nHeight + 1; ++k )
		{
			float fY = rootPlace.vOrigin.y + (k - 1) * FP_GRID_STEP;
			info.terrain.spots.push_back( STerrainSpot( pSpot, CVec2( fX1, fY + FP_GRID_STEP ), CVec2(FP_GRID_STEP, FP_GRID_STEP*0.5f), ToRadian(270.0f) ) );
			info.terrain.spots.push_back( STerrainSpot( pSpot, CVec2( fX2, fY ), CVec2(FP_GRID_STEP, FP_GRID_STEP*0.5f), ToRadian(90.0f) ) );
		}
	}
	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMapBuilder::BuildTerrain( int nPlacementID )
{
	CDBTable<NDb::CTemplVariant> *pVarTable = NDatabase::GetTable<NDb::CTemplVariant>();
	if ( !pVarTable )
		return false;
	NDb::CTemplVariant *pVar = pVarTable->GetRecord( nPlacementID );
	if ( !pVar || !pVar->pTemplate )
	{
		ASSERT( 0 );
		return false;
	}
	bool bRet = true;
	info.terrain.nWidth  = pVar->pTemplate->nWidth;// + 1;
	info.terrain.nHeight = pVar->pTemplate->nHeight;// + 1;

	bRet = LoadRootTerrain( nPlacementID, &info.terrain, &rand, flags );

	CVec3 ptCenter( 0, 0, 0 );
	SMapPosition pos;
	pos.nFloor = 0;
	pos.fRotation = 0;
	pos.ptPos = ptCenter;
	TraverseTerrainTree( pVar->pTemplate, 0, pos, 1, nPlacementID );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool BuildMap( int nPlacementID, const vector<string> &strParams, 
	NAI::IPathNetwork *pNet, SMapInfo *pInfo, int nDepth, SRandomSeed sSeed )
{
	if ( nDepth == 0 )
		return false;
	CMapBuilder builder( pInfo, pNet, sSeed );
	builder.SetParams( strParams );
	builder.SetMaxDepth( nDepth );
	return builder.BuildMap( nPlacementID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool BuildTerrain( int nPlacementID, SMapInfo *pInfo, int nDepth )
{
	if ( nDepth == 0 )
		return false;
	CMapBuilder builder( pInfo, 0, SRandomSeed() );
	builder.SetMaxDepth( nDepth );
	return builder.BuildTerrain( nPlacementID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool BuildMapEditMap( int nMapID, NAI::IPathNetwork *pNet, SMapInfo *pInfo, int nDepth, const SMapPosition &pos, const CVec3 &ptBuilding, bool bTerrAlign )
{
	if ( nDepth == 0 )
		return false;
	CMapBuilder builder( pInfo, pNet, SRandomSeed() );
	builder.SetMaxDepth( nDepth );
	builder.SetPos( pos );
	builder.SetTerrAlignment( bTerrAlign );
	builder.SetParentBuildingPos( ptBuilding );
	builder.SetBuildTerrain( false );
	return builder.BuildMap( nMapID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
