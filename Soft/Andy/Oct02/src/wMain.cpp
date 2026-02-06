#include "StdAfx.h"
#include "Transform.h"
#include "MapBuild.h"
#include "wUnitServer.h"
#include "RPGGame.h"
#include "RPGGlobal.h"
#include "RPGUnitInfo.h"
#include "wObject.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataAI.h"
#include "..\DBFormat\DataGeometry.h"
#include "RPGUnitMission.h"
#include "RPGItemInfo.h"
#include "RPGAttackMech.h"
#include "RPGItem.h"
#include "GSceneUtils.h"
#include "aiPath.h"
#include "aiMap.h"
#include "aiCollider.h"
#include "wDebris.h"
#include "BuildingGrid.h"
#include "GAnimParticles.h"
#include "GAnimation.h"
#include "MakeBuilding.h"
#include "wExplTracker.h"
#include "wBuilding.h"
#include "wGrenade.h"
#include "wKnife.h"
#include "wRocket.h"
#include "wBullet.h"
#include "wMisc.h"
#include "aiCommander.h"
#include "aiTaskCommander.h"
#include "wAckBase.h"
#include "wAck.h"
#include "wTerrain.h"
#include "..\DBFormat\DataAck.h"
#include "aiMoves.h"
#include "aiMultiMoves.h"
#include "..\DBFormat\DataTerrain.h"
#include "..\DBFormat\DataRPG.h"
#include "aiJob.h"
#include "aiVolumeCalcer.h"
#include "aiSignal.h"
#include "A5Script.h"
#include "..\DBFormat\DataScenario.h"
#include "scFlowChartItems.h"
#include "scScenarioTracker.h"
#include "RPGDiplomacy.h"
#include "RPGUnit.h"
#include "..\Misc\LogStream.h"
#include "iMain.h"
#include "iMission.h"
#include "iChapterMap.h"
#include "RPGMerc.h"
#include "wUnitStates.h"
#include "..\Misc\StrProc.h"
//
#include "wMain.h"
//
////////////////////////////////////////////////////////////////////////////////////////////////////
extern vector<SSphere> sphereParticles; // for AI Sound test visualisation
////////////////////////////////////////////////////////////////////////////////////////////////////
extern CPtr<NScript::CScript> pScript;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NCollider
{
	extern int nRepeatedCalls;
	extern int nTotalCalls;
	extern int nRepeatedCallsMov;
	extern int nTotalCallsMov;
}
namespace NWorld
{
const int DW_SEGMENT_TIME = 50;
const int N_SOUND_PARTICLE_ID = 48;
const int N_MAX_MOVE_TO_BLOCK_REAL_TIME = 10;
////////////////////////////////////////////////////////////////////////////////////////////////////
CWorld *pCurrentWorld = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CalcTransform( SObjectPlace *pRes, const SMapPosition &p )
{
	pRes->ptPos = p.ptPos; 
	pRes->ptScale = p.ptScale; 
	pRes->fAngle = ToRadian( p.fRotation );
	pRes->nFloor = p.nFloor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer
////////////////////////////////////////////////////////////////////////////////////////////////////
CPlayer::CPlayer( const wstring &_wsName, NRPG::CGlobalPlayer *_pGlobalPlayer ): 
	wsName( _wsName ), pGlobalPlayer( _pGlobalPlayer ), nCheatFlags( 0 ) 
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlayer::SetCheat( int nCheat, bool bOn )
{
	if ( bOn )
		nCheatFlags |= nCheat;
	else
		nCheatFlags &= ( ~nCheat );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlayer::IsCheatEnabled( int nCheat ) const
{
	return ( ( nCheatFlags & nCheat ) != 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlayer::GetVisible( list< CPtr<CUnit> > *pRes ) const
{
	pRes->clear();
	for ( TUnitSet::const_iterator i = visible.begin(); i != visible.end(); ++i )
		pRes->push_back( i->GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlayer::GetSounds( vector<IVisObj*> *pRes )
{
	typedef SAISound<CUnitServer> TPlayer;
	list<TPlayer> sounds;
	for ( int k = 0; k < units.size(); ++k )
		units[k]->AddSounds( &sounds );
	FilterSounds( &sounds, visible );
	for ( list<TPlayer>::iterator i = sounds.begin(); i != sounds.end(); ++i )
	{
		const TPlayer &s = *i;
		for ( int k = 0; k < s.objects.size(); ++k )
			pRes->push_back( s.objects[k] );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlayer::GetUnits( CUnitSet *pRes ) const
{
	pRes->clear();
	for ( int k = 0; k < units.size(); ++k )
		pRes->push_back( units[k].GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlayer::GetInHandItem( SItemInfo *pInfo ) const
{
	for ( int nTemp = 0; nTemp < units.size(); nTemp++ )
	{
		CPtr<CUnit> pUnit = units[nTemp];
		CPtr<NRPG::IInventoryItem> pItem = pUnit->GetRPG()->GetInventoryInfo()->Get( NDb::SLOT_HAND );
		if ( IsValid( pItem ) )
		{
			pInfo->pUnit = pUnit;
			pInfo->pItem = pItem;
			return true;
		}
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CWorld
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_TEST_HIDDEN_DELTA = 10000;
CWorld::CWorld( NRPG::CGlobalGame *_pGlobalGame ):
	CDebrisController(), pGlobalGame( _pGlobalGame )
{ 
	tPrev = 0; 
	tHiddenDelta = N_TEST_HIDDEN_DELTA;
	pTime = new CCTime( N_TEST_HIDDEN_DELTA ); 
	pAimTime = new CCTime(0);
	pShow = new CWorldSyncSrc;
	pShowUnits = new CWorldSyncSrc;
	pGlobalAck = new CGlobalAck();
	pOwnScript = new NScript::CScript();
	pOwnScript->pWorld = this;
	pAIJobManager = NAI::CreateAIJobManager();
	pAISignalManager = NAI::CreateAISignalManager( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer* CWorld::GetUnit( CUnit *pUnit ) const
{
	CUnitServer *pRes = dynamic_cast<CUnitServer*>(pUnit); 
	ASSERT( pRes ); 
	return pRes; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::AddObject( const SObjectPlace &pos, bool bLightmap, 
	NDb::CObject *pO, NRPG::IObject *pRPG, bool bOpen, int nPassageZoneID, int nPassageObjectID, int nAPRadius )
{
	if ( pO->pDoor || pO->pGun || pO->pPassage )
	{
		if ( !pO->pModels[0] || !pO->pModels[0]->pModel || !pO->pModels[0]->pModel->pSkeleton )
		{
			ASSERT(0);
			return;
		}
	}
	//
	if ( pO->pDoor )
	{
		CWindowDoor *pWD = new CWindowDoor( this, pos, bLightmap, pO, pRPG, GetTime(), bOpen );
		objects.push_back( pWD );
		miscObjects.push_back( pWD );
	}
	else if ( pO->pGun )
	{
		CCannon *pWD = new CCannon( this, pos, bLightmap, pO, pRPG, GetTime() );
		objects.push_back( pWD );
		miscObjects.push_back( pWD );
	}
	else if ( pO->pPassage )
	{
		CPassageObject *pWD = new CPassageObject( this, 
			pos, bLightmap, pO, pRPG, GetTime(), nPassageZoneID, nPassageObjectID, nAPRadius );
		objects.push_back( pWD );
		miscObjects.push_back( pWD );
	}
	else
	{
		NDb::CContainerModel *pCont = pO->pModels[0];
		ASSERT(pCont);
		if ( !pCont )
			return;
		CObjectServerBase *pBase;
		if ( pCont->pModel && pCont->pModel->pSkeleton )
			pBase = *objects.insert( objects.end(), new CAnimObjectServer( this, pos, bLightmap, pO, pRPG, GetTime() ) );
		else
			pBase = *objects.insert( objects.end(), new CObjectServer( this, pos, bLightmap, pO, pRPG ) );

		if ( pBase->NeedSegment() )
			segmentObjects.push_back( pBase );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer *CWorld::GetDeployedDeadUnit( const NAI::SPathPlace &aiPos, NRPG::CUnit *pRPGUnit )
{
	CUnitServer *pUS = 0;
	pUS = GetUnitServer( pRPGUnit );
	if ( pUS == 0 )
	{
		NAI::SUnitPosition p;
		p.pos.SetNetwork( pPathNetwork );
		p.pos.p = aiPos;
		CPtr<NRPG::IUnitMission> pRPG = NRPG::CreateUnit( pRPGUnit, 0 );
		pUS = new CUnitServer( this, pRPG, pRPG->GetModel(), pDeployedDeadUnitsPlayer, p );
		units.push_back( pUS );
		pDeployedDeadUnitsPlayer->AddUnit( pUS );
	}
	return pUS;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer* CWorld::AddUnit( const NAI::SPathPlace &aiPos, NRPG::IUnitMission *_pRPG, CPlayer *pPlayer )
{
	NAI::SUnitPosition p;
	p.pos.SetNetwork( pPathNetwork );
	p.pos.p = aiPos;
	//p.pos.p.nDirection = 0;
	p.SetPose( NAI::WALK );
	if ( !p.IsValid() )
		return 0;
	CUnitServer *pUS = new CUnitServer( this, _pRPG, _pRPG->GetModel(), pPlayer, p );
	units.push_back( pUS );
	OnUnitAdded( pUS );
	pGlobalAck->AddAck( pUS );
	pPlayer->AddUnit( pUS );
	return pUS;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::AddBuilding( const SMapBuilding &info )
{
	CBuilding *pBuilding = new CBuilding( pShow, info, this );
	buildings.push_back( pBuilding );
	miscObjects.push_back( pBuilding );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::AttachMiscObject( CTimedObject *p )
{
	p->Attach( pShow, this );
	miscObjects.push_back( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::CreateParticle( const CVec3 &ptPos, const CQuat &rot, NDb::CEffect *pEffect )
{
  CFBMatrixStack<4> m;
  m.Init();
	m.Push( ptPos, rot );
	NGScene::CCFBTransform *pPlace = new NGScene::CCFBTransform( m.Get() );
	AttachMiscObject( new CDParticles( pPlace, pEffect ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::AddHitLocator( CHitLocator* pLocator )
{
	eventHits.push_back( pLocator );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::AddActionLocator( CActionLocator* pLocator, STime tShowTime )
{
	pLocator->tTime = 0;
	if ( tShowTime != 0 )
		pLocator->tTime = pAimTime->GetValue() + tShowTime;
	eventActions.push_back( pLocator );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::CreateFakeTerrainInfo( STerrainInfo *pTerrain )
{
	STerrainInfo &terrain = *pTerrain;
	
	terrain.nWidth = 128;
	terrain.nHeight = 128;
	terrain.typeMap.SetSizes( terrain.nWidth + 1, terrain.nHeight + 1 );
	terrain.heightMap.SetSizes( terrain.nWidth + 1, terrain.nHeight + 1 );
	terrain.color.SetSizes( terrain.nWidth + 1, terrain.nHeight + 1 );
	terrain.color.FillEvery( 0xffffffff );

	CDBTable<NDb::CTerrainTile> *pTileTable = NDatabase::GetTable<NDb::CTerrainTile>();
	CDBIterator<NDb::CTerrainTile> iTempTile( *pTileTable );
	if ( iTempTile.MoveNext() )
		terrain.typeMap.FillEvery( iTempTile.Get()->GetRecordID() );
	else
	{
		ASSERT(0 && "Hudozhniki suki ubili vse tiles" );
		terrain.typeMap.FillZero();
	}

	terrain.heightMap.FillZero();
/*	
	for ( int nTempY = 0; nTempY <= terrain.nHeight; nTempY++ )
	{
		for ( int nTempX = 0; nTempX <= terrain.nWidth; nTempX++ )
		{
#define PARAM( ValX, ValY ) FP_4PI * ( (float)ValX / terrain.nWidth + (float)ValY / terrain.nWidth ) + FP_4PI * cos ( ( (float)ValX / terrain.nWidth ) * ( (float)ValX / terrain.nWidth ) ) + sin ( ( (float)ValY / terrain.nHeight ) * ( (float)ValY / terrain.nHeight ) )
			terrain.heightMap[nTempY][nTempX] = 4 * ( 32 * sin( PARAM( nTempX, nTempY )  ) + 16 * sin( 2.0f * PARAM( nTempX, nTempY ) ) + 8 * sin( 4.0f * PARAM( nTempX, nTempY ) ) );
			
			terrain.typeMap[nTempY][nTempX] = 6;
			if ( terrain.heightMap[nTempY][nTempX] > 32 )
				terrain.typeMap[nTempY][nTempX] = 8;
		}
	}
	
	/*STerrainHole tHole;
	tHole.bVisible = true;
	tHole.nHeight = 20;
	tHole.vPolygon.push_back( CVec2( 10 + 0, 10 + 0 ) );
	tHole.vPolygon.push_back( CVec2( 10 + 3, 10 + 0 ) );
	tHole.vPolygon.push_back( CVec2( 10 + 8, 10 + 5 ) );
	tHole.vPolygon.push_back( CVec2( 10 + 5, 10 + 5 ) );
	tHole.vPolygon.push_back( CVec2( 10 + 5, 10 + 8 ) );
	tHole.vPolygon.push_back( CVec2( 10 + 0, 10 + 3 ) );
	terrain.holes.push_back( tHole );
	*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SCompareInterruptStrength
{
	bool operator()( const SInterruptInfo::SNotice &a, const SInterruptInfo::SNotice &b ) const
	{
		return a.fStrength < b.fStrength;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::CheckInterrupt( SInterruptInfo *info )
{
	if ( info->events.empty() )
		return;
	// detect if interrupts are mutual or not
	for ( list<SInterruptInfo::SNotice>::iterator i = info->events.begin(); i != info->events.end(); ++i )
	{
		SInterruptInfo::SNotice &n = *i;
		for ( list<SInterruptInfo::SNotice>::iterator k = i; k != info->events.end(); ++k )
		{
			if ( k->pWho == n.pWhom && k->pWhom == n.pWho )
			{
				k->bIsMutual = true;
				n.bIsMutual = true;
				break;
			}
		}
	}
	CPlayer *pWhoseTurn = GetTBSCurrentPlayer();
	// determine interrupts strength
	bool bCancelAllAction = false;
	for ( list<SInterruptInfo::SNotice>::iterator i = info->events.begin(); i != info->events.end(); )
	{
		if ( i->pWho->GetTBSPlayer() == pWhoseTurn )
		{
			bCancelAllAction = true;
			i = info->events.erase( i );
		}
		else
		{
			int nInteruptStr = i->pWho->GetUnitRPG()->CheckInterrupt( i->pWhom->GetUnitRPG(), i->bIsMutual, i->bWasShot );
			if ( nInteruptStr < 0 || i->pWho->WasInterrupted( i->pWhom ) || !i->pWho->CanSpendAP( i->pWho->GetActionAP( NRPG::AC_MOVE_SIDE ) ) )
				i = info->events.erase( i );
			else
			{
				i->fStrength = nInteruptStr;
				++i;
			}
		}
	}
	if ( info->events.empty() )
	{
		if ( bCancelAllAction )
			GlobalSituationChanged();
		return;
	}
	// sort them out
	info->events.sort ( SCompareInterruptStrength() );
	list<CUnitServer*> res;
	CUnitServer *pWhom = 0;
	CUnitServer *pWho = 0;
	CPlayer *pTestPlayer = info->events.back().pWho->GetTBSPlayer();
	while ( !info->events.empty() )
	{
		const SInterruptInfo::SNotice &e = info->events.back();
		pWho = e.pWho;
		if ( pWho->GetTBSPlayer() != pTestPlayer )
			break;
		if ( find( res.begin(), res.end(), pWho ) == res.end() )
			res.push_back( pWho );
		if ( !pWhom )
			pWhom = e.pWhom;
		pWho->MarkInterrupted( e.pWhom );
		info->events.pop_back();
		AttachMiscObject( new C3DSound( pWho->GetPosition().GetEyePosition(), NDb::GetSound(11) ) ); // CRAP multiple sounds in one place, direct ID specified
	}
	//
	if ( IsValid( pWho ) )
		GetGlobalAck()->OnInterrupt( pWho );
	AddInterrupt( res );
	if ( pWhom )
		AddActionLocator( new CActionLocator( CActionLocator::TYPE_INTERRUPT, pWhom, pWhom->GetPosition().GetCP() ), 1000 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::UpdateVisible()
{
	SInterruptInfo info;
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
		(*i)->UpdateVisible( &info );
	UpdatePlayersVisibleSets();
	CheckInterrupt( &info );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::GetAllUnits( vector< CPtr<NWorld::CUnit> > *pUnits )
{
	pUnits->clear();
	for ( list<CObj<CUnitServer> >::const_iterator i = units.begin(); i != units.end(); ++i )
		pUnits->push_back( i->GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::GetAllUnits( list<CPtr<CUnitServer> > *pRes )
{
	pRes->clear();
	for ( list<CObj<CUnitServer> >::const_iterator k = units.begin(); k != units.end(); ++k )
		pRes->push_back( k->GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::GetUnitsNear( const CVec3 &pos, list<CPtr<CUnitServer> > *pRes, float fRadius )
{
	pRes->clear();
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		CVec3 ptDest = (*i)->GetPosition().GetCenter();
		if ( fabs( ptDest - pos ) <  fRadius )
			pRes->push_back( (*i).GetPtr() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::SelectFindPathUnits( CUnit *pWho, list<CObjectBase*> *pRes )
{
	list<CPtr<CUnit> > visible;
	pWho->GetPlayer()->GetVisible( &visible );
	visible.remove( pWho );
	list<CObjectBase*> &vis = *pRes;
	for ( list<CPtr<CUnit> >::iterator i = visible.begin(); i != visible.end(); ++i )
	{
		if ( !GetUnit(*i)->IsDead() && !GetUnit(*i)->IsUnconscious() && !GetUnit(*i)->IsMoving() )
			vis.push_back( *i );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::EAction CWorld::GetMoveActionType( const NAI::SUnitPosition &src, const NAI::SUnitPosition &dst, bool bCorpse )
{
	NAI::ETransitionType tt = NAI::GetTransitionType( pPathNetwork, src.pos.p, dst.pos.p );
	switch ( tt )
	{
		case NAI::TT_MOVE:
			return bCorpse ? NRPG::AC_MOVE_CORPSE_SIDE : NRPG::AC_MOVE_SIDE;
		case NAI::TT_MOVE_DIAGONAL:
			return bCorpse ? NRPG::AC_MOVE_CORPSE_DIAGONAL : NRPG::AC_MOVE_DIAGONAL;
		case NAI::TT_INTERGRID:
			if ( fabs( src.GetCPNoHeight() - dst.GetCPNoHeight() ) > FP_GRID_STEP )
				return bCorpse ? NRPG::AC_MOVE_CORPSE_DIAGONAL : NRPG::AC_MOVE_DIAGONAL;
			else
				return bCorpse ? NRPG::AC_MOVE_CORPSE_SIDE : NRPG::AC_MOVE_SIDE;
		case NAI::TT_SAME:
		case NAI::TT_INTERGRID_SAME:
			//ASSERT( 0 );
			return NRPG::AC_NONE;//AC_MOVE_SIDE; //AC_ROTATE,
		case NAI::TT_TURN:
			return NRPG::AC_NONE;// AC_ROTATE; // CRAP
		case NAI::TT_CLIMB_1:  return NRPG::AC_CLIMB_1;
		case NAI::TT_CLIMB_2:  return NRPG::AC_CLIMB_2;
		case NAI::TT_CLIMB_3:  return NRPG::AC_CLIMB_3;
		case NAI::TT_CLIMB_4:  return NRPG::AC_CLIMB_4;
		case NAI::TT_JUMP:     return NRPG::AC_JUMP;
		case NAI::TT_POSE:
			{
				switch ( dst.GetPose() )
				{
				case NAI::RUN:
					return NRPG::AC_POSE_RUN;
				case NAI::WALK:
					return NRPG::AC_POSE_WALK;
				case NAI::CROUCH:
					return NRPG::AC_POSE_CROUCH;
				case NAI::CRAWL:
					return NRPG::AC_POSE_CRAWL;
				default:
					ASSERT(0);
					break;
				}
				break;
			}
		case NAI::TT_LADDER_UP:
		case NAI::TT_LADDER_DOWN:
			return NRPG::AC_LADDER;
		case NAI::TT_LADDER_MOVE:
			return NRPG::AC_LADDER_MOVE;
		default:
			ASSERT( 0 );
			break;
	}
	return NRPG::AC_MOVE_SIDE;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::PrepareAllPaths( NAI::CMultiMovesTable *pTable, list<NAI::SPathPlace> *pResult, CUnit *pWho,
	const NAI::SPathPlace &ptSrc, int nPriceLimit, CUnit *pIgnore, bool bCheckSuicide )
{
	static int nCosts[ NAI::N_MOVE_TYPES ];
	vector<NAI::SPoint> points;
	CUnitServer *pUS = GetUnit( pWho );
	if ( !IsValid( pUS ) )
		return;
	ASSERT( nPriceLimit >=0 );
	if ( nPriceLimit <= 0 )
	{
		pResult->push_back( ptSrc );
		return;
	}

	pPathNetwork->Unlock( pWho );
	list<CObjectBase*> vis;
	SelectFindPathUnits( pWho, &vis );
	vis.remove( pIgnore );

	NAI::EPose curPose = pUS->GetWishPose();
	const NRPG::IUnitMissionInfo *pRPG = pUS->GetRPG();
	if ( pUS->IsCarryingCorpse() )
	{
		if ( curPose == NAI::RUN )
			nCosts[ NAI::MT_MOVE_STAND ] = pRPG->GetActionAP( NAI::RUN, NRPG::AC_MOVE_CORPSE_SIDE );
		else
			nCosts[ NAI::MT_MOVE_STAND ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_MOVE_CORPSE_SIDE );
		nCosts[ NAI::MT_MOVE_CRAWL ] = pRPG->GetActionAP( NAI::CRAWL, NRPG::AC_MOVE_CORPSE_SIDE );
		nCosts[ NAI::MT_MOVE_CROUCH ] = pRPG->GetActionAP( NAI::CROUCH, NRPG::AC_MOVE_CORPSE_SIDE );

		if ( curPose == NAI::RUN )
			nCosts[ NAI::MT_MOVE_STAND_DIAG ] = pRPG->GetActionAP( NAI::RUN, NRPG::AC_MOVE_CORPSE_DIAGONAL );
		else
			nCosts[ NAI::MT_MOVE_STAND_DIAG ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_MOVE_CORPSE_DIAGONAL );
		nCosts[ NAI::MT_MOVE_CRAWL_DIAG ] = pRPG->GetActionAP( NAI::CRAWL, NRPG::AC_MOVE_CORPSE_DIAGONAL );
		nCosts[ NAI::MT_MOVE_CROUCH_DIAG ] = pRPG->GetActionAP( NAI::CROUCH, NRPG::AC_MOVE_CORPSE_DIAGONAL );
	}
	else
	{
		if ( curPose == NAI::RUN )
			nCosts[ NAI::MT_MOVE_STAND ] = pRPG->GetActionAP( NAI::RUN, NRPG::AC_MOVE_SIDE );
		else
			nCosts[ NAI::MT_MOVE_STAND ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_MOVE_SIDE );
		nCosts[ NAI::MT_MOVE_CRAWL ] = pRPG->GetActionAP( NAI::CRAWL, NRPG::AC_MOVE_SIDE );
		nCosts[ NAI::MT_MOVE_CROUCH ] = pRPG->GetActionAP( NAI::CROUCH, NRPG::AC_MOVE_SIDE );

		if ( curPose == NAI::RUN )
			nCosts[ NAI::MT_MOVE_STAND_DIAG ] = pRPG->GetActionAP( NAI::RUN, NRPG::AC_MOVE_DIAGONAL );
		else
			nCosts[ NAI::MT_MOVE_STAND_DIAG ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_MOVE_DIAGONAL );
		nCosts[ NAI::MT_MOVE_CRAWL_DIAG ] = pRPG->GetActionAP( NAI::CRAWL, NRPG::AC_MOVE_DIAGONAL );
		nCosts[ NAI::MT_MOVE_CROUCH_DIAG ] = pRPG->GetActionAP( NAI::CROUCH, NRPG::AC_MOVE_DIAGONAL );
	}
	nCosts[ NAI::MT_TURN ] = 1;
	nCosts[ NAI::MT_CLIMB_1 ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_CLIMB_1 );
	nCosts[ NAI::MT_CLIMB_2 ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_CLIMB_2 );
	nCosts[ NAI::MT_CLIMB_3 ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_CLIMB_3 );
	nCosts[ NAI::MT_CLIMB_4 ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_CLIMB_4 );
	nCosts[ NAI::MT_JUMP ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_JUMP );
	nCosts[ NAI::MT_POSE_WALK_CROUCH ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_POSE_CROUCH );
	nCosts[ NAI::MT_POSE_WALK_CRAWL ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_POSE_CRAWL );
	nCosts[ NAI::MT_POSE_CROUCH_CRAWL ] = pRPG->GetActionAP( NAI::CRAWL, NRPG::AC_POSE_CROUCH );
	nCosts[ NAI::MT_LADDER_UP ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_LADDER );
	nCosts[ NAI::MT_LADDER_DOWN ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_LADDER );
	nCosts[ NAI::MT_LADDER_MOVE ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_LADDER_MOVE );
	nCosts[ NAI::MT_ZERO ] = 1;
	
	pTable->PrepareAllPaths( pResult, pPathNetwork, ptSrc, nCosts, nPriceLimit, vis,
		bCheckSuicide, pUS->IsCarryingCorpse() );

	pPathNetwork->Lock( pWho, pUS->GetPosition().pos.p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NAI::CPath* CWorld::FindPath( CUnit *pWho, const NAI::SPathPlace &ptSrc, 
	const vector<NAI::SPathPlace> &ptDst,	CUnit *pIgnore, bool bCheckSuicide, 
	NAI::EFindPathParams eParams, bool bStrafe, bool bCanFindNotExactPath, bool bIgnoreAllUnits )
{
	static int nCosts[NAI::N_MOVE_TYPES];
	vector<NAI::SPoint> points;
	CUnitServer *pUS = GetUnit( pWho );
	if ( !IsValid( pUS ) )
		return 0;

	pPathNetwork->Unlock( pWho );
	pPathNetwork->ClearDynamicLocks( pWho );

	NAI::SPathPlace a( ptSrc );
	a.SetMoving( 0 );
	// when src and dst points are in one move, do not construct big data structures etc.
	// just return the way
	int nBestCost = NAI::N_PF_MAXWEIGHT, nCurCost;
	NAI::SPathPlace bestDest;
	if ( ptSrc.GetPose() != NAI::CM_INACTIVE )
	{
		for ( vector<NAI::SPathPlace>::const_iterator i = ptDst.begin(); i != ptDst.end(); ++i )
		{
			int nDir1 = 0, nDir2 = 7;
			int nPose1 = NAI::CM_LAY, nPose2 = NAI::CM_INACTIVE;
			if ( eParams & NAI::PF_USE_DIR )
			{
				nDir1 = i->GetDirection();
				nDir2 = i->GetDirection();
			}
			if ( eParams & NAI::PF_USE_POSE )
			{
				nPose1 = i->GetPose();
				nPose2 = i->GetPose();		
			}
			for ( int dir = nDir1; dir <= nDir2; ++dir )
			{
				for ( int pose = nPose1; pose <= nPose2; ++pose )
				{
					NAI::SPathPlace where( *i );
					where.SetDirection( dir );
					where.SetPose( pose );
					NAI::ETransitionType tt = NAI::GetTransitionType( pPathNetwork, a, where );
					nCurCost = NAI::N_PF_MAXWEIGHT;
					switch ( tt )
					{
					case NAI::TT_SAME:
						nCurCost = 0; break;
					case NAI::TT_TURN:
						nCurCost = 1; break;
					case NAI::TT_POSE:
						nCurCost = pUS->IsCarryingCorpse() ? NAI::N_PF_MAXWEIGHT : 2; break;
					case NAI::TT_INTERGRID_SAME:
						nCurCost = 0; break;
					}
					if ( ( nCurCost < nBestCost ) && ( pPathNetwork->IsPassable( where ) ) )
					{
						nBestCost = nCurCost;
						bestDest = where;		
					}
				}
			}
		}
	}

	NAI::CPath *pRes = 0;
	if ( nBestCost < NAI::N_PF_MAXWEIGHT )
	{
		pRes = new NAI::CPath;
		pRes->pNet = pPathNetwork;
		pRes->points.push_back( a );
		pRes->points.push_back( bestDest );
		return pRes;
	}

	list<CObjectBase*> vis;
	if ( !bIgnoreAllUnits )
	{		
		SelectFindPathUnits( pWho, &vis );
		vis.remove( pIgnore );
	}

	if ( bCheckSuicide )
	{
		int nMax = 10;
		int nMaxDiag = 15;
		nCosts[ NAI::MT_MOVE_STAND ] = nMax;
		nCosts[ NAI::MT_MOVE_STAND_DIAG ] = nMaxDiag;
		nCosts[ NAI::MT_MOVE_CROUCH ] = nMax;
		nCosts[ NAI::MT_MOVE_CROUCH_DIAG ] = nMaxDiag;
		nCosts[ NAI::MT_MOVE_CRAWL ] = nMax;
		nCosts[ NAI::MT_MOVE_CRAWL_DIAG ] = nMaxDiag;
		nCosts[ NAI::MT_TURN ] = 1;
		nCosts[ NAI::MT_CLIMB_1 ] = nMax;
		nCosts[ NAI::MT_CLIMB_2 ] = nMax;
		nCosts[ NAI::MT_CLIMB_3 ] = nMax;
		nCosts[ NAI::MT_CLIMB_4 ] = nMax;
		nCosts[ NAI::MT_JUMP ] = nMax;
		nCosts[ NAI::MT_POSE_WALK_CROUCH ] = 
			nCosts[ NAI::MT_POSE_WALK_CRAWL ] = nCosts[ NAI::MT_POSE_CROUCH_CRAWL ] = 1;			
		nCosts[ NAI::MT_LADDER_UP ] = nMax;
		nCosts[ NAI::MT_LADDER_DOWN ] = nMax;
		nCosts[ NAI::MT_LADDER_MOVE ] = nMax;
		nCosts[ NAI::MT_ZERO ] = 1;
		pRes = NAI::FindPath( pPathNetwork, ptSrc, ptDst, nCosts, nMax * 3 - 1, bStrafe, vis, true, pUS->IsCarryingCorpse(), eParams );
	}

	if ( !pRes )
	{
		NAI::EPose curPose = pUS->GetWishPose();
		const NRPG::IUnitMissionInfo *pRPG = pUS->GetRPG();
		int nSideCost, nDiagCost;
		if ( pUS->IsCarryingCorpse() )
			nSideCost = pRPG->GetActionAP( curPose, NRPG::AC_MOVE_CORPSE_SIDE );
		else
			nSideCost = pRPG->GetActionAP( curPose, NRPG::AC_MOVE_SIDE );
		if ( pUS->IsCarryingCorpse() )
			nDiagCost = pRPG->GetActionAP( curPose, NRPG::AC_MOVE_CORPSE_DIAGONAL );
		else
			nDiagCost = pRPG->GetActionAP( curPose, NRPG::AC_MOVE_DIAGONAL );
		int nAddStand = (curPose == NAI::WALK || curPose == NAI::RUN) ? 0 : 2;
		int nAddCrouch = (curPose == NAI::CROUCH) ? 0 : 2;
		int nAddCrawl = (curPose == NAI::CRAWL) ? 0 : 2;
		nCosts[ NAI::MT_MOVE_STAND ] = nSideCost + nAddStand;
		nCosts[ NAI::MT_MOVE_STAND_DIAG ] = nDiagCost + nAddStand;
		nCosts[ NAI::MT_MOVE_CROUCH ] = nSideCost + nAddCrouch;
		nCosts[ NAI::MT_MOVE_CROUCH_DIAG ] = nDiagCost + nAddCrouch;
		nCosts[ NAI::MT_MOVE_CRAWL ] = nSideCost + nAddCrawl;
		nCosts[ NAI::MT_MOVE_CRAWL_DIAG ] = nDiagCost + nAddCrawl;
		nCosts[ NAI::MT_TURN ] = 1;
		nCosts[ NAI::MT_CLIMB_1 ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_CLIMB_1 );
		nCosts[ NAI::MT_CLIMB_2 ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_CLIMB_2 );
		nCosts[ NAI::MT_CLIMB_3 ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_CLIMB_3 );
		nCosts[ NAI::MT_CLIMB_4 ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_CLIMB_4 );
		nCosts[ NAI::MT_JUMP ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_JUMP );
		nCosts[ NAI::MT_POSE_WALK_CROUCH ] = //pRPG->GetActionAP( NAI::WALK, NRPG::AC_POSE_CRAWL );
			nCosts[ NAI::MT_POSE_WALK_CRAWL ] = nCosts[ NAI::MT_POSE_CROUCH_CRAWL ] = 1;			
		nCosts[ NAI::MT_LADDER_UP ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_LADDER );
		nCosts[ NAI::MT_LADDER_DOWN ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_LADDER );
		nCosts[ NAI::MT_LADDER_MOVE ] = pRPG->GetActionAP( NAI::WALK, NRPG::AC_LADDER_MOVE );
		nCosts[ NAI::MT_ZERO ] = 1;
		pRes = NAI::FindPath( pPathNetwork, ptSrc, ptDst, nCosts, 0x7FFFFFFF, bStrafe, vis, false, pUS->IsCarryingCorpse(), eParams, bCanFindNotExactPath );
	}

	pPathNetwork->Lock( pWho, pUS->GetPosition().pos.p );
	pPathNetwork->RestoreDynamicLocks( pWho );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::IsValidPath( const NAI::CPath &path, CUnit *pWho )
{
	list<CObjectBase*> vis;
	SelectFindPathUnits( pWho, &vis );
	pPathNetwork->LockSelected( vis );
	bool bRes = NAI::IsValidPath( path );
	pPathNetwork->UnlockSelected();
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::TraceTile( const CRay &ray, NAI::SPosition *pRes, int nMaxFloor )
{
	CVec3 ptDst;
	vector<NAI::SInterval> intervals;
	GetAIMap()->Trace( ray, &intervals, TS_GO_OVER | TS_PASS_BLOCKER, NAI::CFloorsSet(), NAI::IAIMap::STH_SPLIT_TERR_HG );
	ptDst = ray.ptOrigin - ray.ptDir * ( (ray.ptOrigin.z - 1) / ray.ptDir.z );
	bool bRet = false;
	for ( int k = 0; k < intervals.size(); ++k )
	{
		NAI::SInterval &interv = intervals[k];
		if ( interv.enter.fT < 0 )
			continue;
		if ( interv.pSrc->nFloor > nMaxFloor )
			continue;
		ptDst = ray.ptOrigin + interv.enter.fT * ray.ptDir;
		bRet = pPathNetwork->SetOnFloor( pRes, interv.pSrc->nFloor, ptDst );
		if ( pRes->GetFloor() <= nMaxFloor )
			return bRet;
	}
	// fallback to root terrain in case none did help
	bRet = pPathNetwork->SetOnLayer( pRes, 0, ptDst );
	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::TraceObjects( const CRay &ray, vector<IVisObj*> *pRes, int nMaxFloor )
{
	CVec3 ptDst;
	vector<NAI::SInterval> intervals;
	GetAIMap()->Trace( ray, &intervals, TS_PICK|TS_GO_OVER );
	ptDst = ray.ptOrigin - ray.ptDir * ( (ray.ptOrigin.z - 1) / ray.ptDir.z );
	for ( int k = 0; k < intervals.size(); ++k )
	{
		NAI::SInterval &interv = intervals[k];
		if ( interv.enter.fT < 0 )
			continue;
		//if ( interv.pUserData == 0 )
		//	continue;
		ptDst = ray.ptOrigin + interv.enter.fT * ray.ptDir;
		//NAI::SPosition test;
		//pPathNetwork->SetOnFloor( &test, interv.pSrc->nFloor, ptDst );
		//int nCurFloor = test.GetFloor();
		int nCurFloor = interv.pSrc->nFloor;
		if ( nCurFloor > nMaxFloor )
			continue;
		if ( find( pRes->begin(), pRes->end(), interv.pSrc->pUserData ) == pRes->end() )
		{
			pRes->push_back( interv.pSrc->pUserData );
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::TraceRay( const CRay &ray, 
	int nMaxFloor, NWorld::IVisObj **ppUserData, int *pUserID, CVec3 *pPoint )
{
	CFWContext world( &pCurrentWorld, this );
	vector<NAI::SInterval> intervals;
	GetAIMap()->Trace( ray, &intervals, TS_FRAGMENTED );
	NAI::SInterval *pBest = 0;
	float fBest = 1e30f;
	for ( int k = 0; k < intervals.size(); ++k )
	{
		NAI::SInterval &interv = intervals[k];
		if ( interv.enter.fT < 0 )
			continue;
		if ( interv.pSrc->nFloor > nMaxFloor )
			continue;
		if ( interv.enter.fT < fBest )
		{
			fBest	= interv.enter.fT;
			pBest = &interv;
		}
	}
	if ( !pBest )
		return false;
	else
	{
		*ppUserData = pBest->pSrc->pUserData;
		*pUserID = pBest->nUserID;
		*pPoint = ray.Get( fBest );
		return true;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::ClickOfDeath( const CRay &ray, int nMaxFloor )
{
	NWorld::IVisObj *pUserData;
	int nUserID;
	CVec3 ptPoint;
	if ( TraceRay( ray, nMaxFloor, &pUserData, &nUserID, &ptPoint ) )
		miscObjects.push_back( new CClickOfDeath( GetActiveCounter(), pUserData, nUserID, ray ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnit* CWorld::GetUnit( const NAI::SUnitPosition &pos )
{
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( (*i)->GetPosition().pos == pos.pos && !(*i)->IsDead() && !(*i)->IsUnconscious() )
			return (*i);
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnit* CWorld::GetUnitInTile( const NAI::SUnitPosition &pos )
{
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( !(*i)->IsUnconscious() && !(*i)->IsDead() && ( (*i)->GetPosition().GetCP() == pos.GetCP() ) )
			return (*i);
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const CTRect<float>& CWorld::GetMapSafeZone() const
{
	return sMapSafeZone;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::IsPersonSlotUsed( int nUnitID, const ClueToSlot &clueToSlot, int *pPersID )
{
	for ( ClueToSlot::const_iterator i = clueToSlot.begin();
		i != clueToSlot.end(); ++i )
			if ( i->second.nUnitID == nUnitID )
			{
				*pPersID = i->first->GetDBClue()->nPersID;
				return true;
			}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::DistributeClues( int nTemplateID, const SMapInfo &mapInfo,
	const list< CPtr<NScenario::CScenarioClue> > &clues,
	ClueToSlot *personClueToSlot,	ClueToSlot *itemClueToSlot )
{
	ASSERT( personClueToSlot != 0 );
	ASSERT( itemClueToSlot != 0 );
	//
	personClueToSlot->clear();
	itemClueToSlot->clear();
	//
	vector<SClueSlot> personSlots;
	vector<SClueSlot> itemSlots;
	// размещаем person clues
	for ( vector<SClueSlot>::const_iterator slot = mapInfo.slots.begin();
		slot != mapInfo.slots.end(); ++slot )
		if ( slot->bPersSlot )
			personSlots.push_back( *slot );
	int nPersonSlots = personSlots.size();
	//
	for ( list< CPtr<NScenario::CScenarioClue> >::const_iterator clue = clues.begin();
		clue != clues.end(); ++clue )
			if ( !personSlots.empty() && (*clue)->GetDBClue()->clueType == NDb::CT_PERSON &&
				(*clue)->GetDBClue()->nPersID > 0 )
			{
				int n = random.Get( 0, personSlots.size() );
				SClueSlot &slot = personSlots[n];
				(*personClueToSlot)[ *clue ] = slot;
				if ( slot.bInventorySlot )
					itemSlots.push_back( slot );
				personSlots.erase( personSlots.begin() + n );
			}
	// размещаем item clues
	for ( vector<SClueSlot>::const_iterator slot = mapInfo.slots.begin();
		slot != mapInfo.slots.end(); ++slot )
		if ( !slot->bPersSlot )
			itemSlots.push_back( *slot );
	int nItemSlots = itemSlots.size();
	//
	for ( list< CPtr<NScenario::CScenarioClue> >::const_iterator clue = clues.begin();
		clue != clues.end(); ++clue )
			if ( !itemSlots.empty() && (*clue)->GetDBClue()->clueType == NDb::CT_ITEM && 
				(*clue)->GetDBClue()->nItemID > 0 )
			{
				int n = random.Get( 0, itemSlots.size() );
				(*itemClueToSlot)[ *clue ] = itemSlots[ n ];
				itemSlots.erase( itemSlots.begin() + n );
			}
	//
	char szStr[128];
	sprintf( szStr, "[SCENARIO TRACKER] %d person slots, %d item slots\n", nPersonSlots, nItemSlots );
	OutputDebugString( szStr );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::PlaceItemSlotsToMap( const ClueToSlot &clueToSlot )
{
	char szStr[128];
	//
	for ( ClueToSlot::const_iterator i = clueToSlot.begin();
		i != clueToSlot.end(); ++i )
	{
		if ( !i->second.bInventorySlot )
		{
			CPtr<NRPG::IInventoryItem> pItem = NRPG::CreateItem( NDb::GetRPGItem( i->first->GetDBClue()->nItemID )->pSuccessor );
			if ( IsValid( pItem ) )
			{
				// помещаем на карту
				CQuat rot( ToRadian( i->second.pos.fRotation ), CVec3(0,0,1) );
				AddFrozenItem( i->second.pos.ptPos, rot, pItem, i->second.pos.nFloor );
				sprintf( szStr, "[SCENARIO TRACKER] item %d was placed on the map\n", i->first->GetDBClue()->nItemID );
				OutputDebugString( szStr );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::PlaceItemSlotsToInventory( const ClueToSlot &clueToSlot )
{
	char szStr[128];
	//
	for ( ClueToSlot::const_iterator i = clueToSlot.begin();
		i != clueToSlot.end(); ++i )
	{
		if ( i->second.bInventorySlot )
		{
			CPtr<NRPG::IInventoryItem> pItem = NRPG::CreateItem( NDb::GetRPGItem( i->first->GetDBClue()->nItemID )->pSuccessor );
			if ( IsValid( pItem ) )
			{
				// помещаем в inventory
				CPtr<NWorld::CUnitServer> pUnitServer = GetUnitServerByPersID( i->second.pPers->nRPGPersID );
				if ( IsValid( pUnitServer ) )
				{
					CPtr<NRPG::IInventory> pInventory = pUnitServer->GetUnitRPG()->GetInventory();
					CTPoint<int> position;
					pInventory->FindPlace( pItem, &position );
					pInventory->Place( position, pItem );
					sprintf( szStr, "[SCENARIO TRACKER] item %d was placed in an inventory\n", i->first->GetDBClue()->nItemID );
					OutputDebugString( szStr );
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::CreateRandom( int nTemplateID, 
	bool bBuildingStability, const list< CPtr<NScenario::CScenarioClue> > &clues, 
	int nMobsLevel, CObj<CPostWorldCreateInfo> *pPostInfo, SRandomSeed sSeed )
{
	CFWContext world( &pCurrentWorld, this );
	CDGPtr<CFuncBase<STime> > pRefreshTime(pTime);
	pRefreshTime.Refresh();
	//
	pAIMap = NAI::CreateAIMap( this );
	pPathNetwork = NAI::CreateNodesNetwork( pAIMap );
	pRPGGame = NRPG::CreateGame( pAIMap, pPathNetwork );
	
	SMapInfo mapInfo;
	vector<string> params;
	if ( !BuildMap( nTemplateID, params, pPathNetwork, &mapInfo, -1, sSeed ) )
	{
		CreateDefault();
		return;
	}
	nRootLayersGroup = 0;
	pDefaultLight = mapInfo.pDefaultLight;
	sMapSafeZone = mapInfo.sMapSafeZone;
	
	pTerrainInfo = new CTerrainInfoHolder( mapInfo.terrain );
	pTerrain = new NWorld::CTerrain( pShow, pTerrainInfo, pAIMap, pTime, mapInfo.nBaseTerrainFloor, mapInfo.holesList, mapInfo.wallsList );

	list<SMapElement>::const_iterator i;
	for ( i = mapInfo.items.begin(); i != mapInfo.items.end(); ++i )
	{
		int nStages = 0;
		for ( ; nStages < NDb::N_DESTROY_STAGES && IsValid( i->pObject->pModels[nStages] ); ++nStages );

		if ( nStages > 0 )
		{
			NRPG::IObject *pRPGObject;
			pRPGObject = NRPG::CreateObject( nStages - 1, i->pObject->pModels[0]->pModel );
			if ( !pRPGObject )
				continue;
			SObjectPlace place;
			CalcTransform( &place, i->pos ); // CRAP
			AddObject( place, i->bLightmap, 
				i->pObject, pRPGObject, i->bOpen, i->nPassageZoneID, i->nPassageObjectID, i->nAPRadius );
		}
		else
			ASSERT(0);
	}
	for ( list<SMapRPGElement>::const_iterator i = mapInfo.rpgitems.begin(); i != mapInfo.rpgitems.end(); ++i )
	{
		if ( IsValid( i->pItem ) && IsValid( i->pItem->pSuccessor) )
		{
			CQuat rot( ToRadian( i->pos.fRotation ), CVec3(0,0,1) );
			AddFrozenItem( i->pos.ptPos, rot, NRPG::CreateItem( i->pItem->pSuccessor ), i->pos.nFloor );
		}
		else
			ASSERT(0);
	}
	vector<SMapBuilding>::const_iterator ib;
	for ( ib = mapInfo.buildings.begin(); ib != mapInfo.buildings.end(); ++ib )
	{
		if ( !bBuildingStability )
			ib->pGrid->ToggleStability();
		AddBuilding( *ib );
	}

	ClueToSlot personClueToSlot, itemClueToSlot;
	DistributeClues( nTemplateID, mapInfo, clues, &personClueToSlot, &itemClueToSlot );
	PlaceItemSlotsToMap( itemClueToSlot );
	//CreateFakeTerrainInfo( &terrain );
	//
	// после этого момента не ставить объекты, влияющие на проходимость
	pAIMap->Sync();
	// deploy units
	// put some enemies
	//
	vector<NAI::SPathPlace> empty;
	pPathNetwork->UpdateColouring( empty );
	// до этого момента не ставить персов
	pDeployedDeadUnitsPlayer = new CPlayer( L"Deployed dead units fake player", 0 );
	pDeployedDeadUnitsPlayer->SetCommander( new NWorld::CCommander );
	CPlayer *pPlayer = new CPlayer( L"AI Player", 0 );
	RegisterPlayer( pPlayer );

	CPtr<NAI::CAICommander> pAICommander = new NAI::CAICommander( this, pPlayer );
	pPlayer->SetCommander( pAICommander );

	nAIUnitsCreated = 0;
	list<SMapUnit>::const_iterator iu;
	for ( iu = mapInfo.units.begin(); iu != mapInfo.units.end(); ++iu )
	{
		int nPersID;
		if ( !iu->bSlot || ( iu->bSlot && IsPersonSlotUsed( iu->nUnitID, personClueToSlot, &nPersID ) ) )
		{
			CDBPtr<NDb::CRPGPers> pPers = 0;
			if ( iu->bSlot )
				pPers = NDb::GetPers( nPersID );
			else
				pPers = iu->pPers;
			//
			ASSERT( IsValid( pPers ) );
			//
			if ( IsValid( pPers ) )
			{
				CPtr<NRPG::IUnitMission> pRPG = NRPG::CreateUnit( pPers, iu->nScenarioPlayer );
				pRPG->GetRPGUnit()->SetXPLevel( nMobsLevel );
				int nDiplomacy = iu->nDiplomacy;
				if ( nDiplomacy < 0 )
					nDiplomacy = pGlobalGame->pGlobalDiplomacy->GetPlayerDiplomacy( pRPG->GetScenarioPlayer() );
				pRPG->GetDiplomacy()->SetDiplomacy( nDiplomacy );
				//
				NAI::SPosition pos;
				pPathNetwork->SetOnFloor( &pos, iu->pos.nFloor, iu->pos.ptPos );
				CPtr<CUnitServer> pUnitServer = AddUnit( pos.p, pRPG, pPlayer );
				// создаем route-ы
				if ( IsValid( pUnitServer ) )
				{
					++nAIUnitsCreated;
					pAICommander->GetAITaskCommander()->CreateRoute( pUnitServer, (*iu), pPathNetwork );
				}
			}
		}
	}
	PlaceItemSlotsToInventory( itemClueToSlot );

	nPartiesAdded = 0;
	// copy deploy spots
	for ( int k = 0; k < mapInfo.deploySpots.size(); ++k )
	{
		SDeploySpot &s = mapInfo.deploySpots[k];
		SSphere t( s.pos.ptPos, 3 );
		vector<NAI::SPathPlace> res;
		pPathNetwork->GetNearPlaces( t, &res );
		if ( res.empty() )
		{
			ASSERT( 0 && "no path network near deploy spot" );
			continue;
		}
		NAI::SPathPlace p = res[0];
		p.SetDirection( pPathNetwork->GetClosestDir( p.GetLayer(), ToRadian( s.pos.fRotation ) ) );
		deploySpots.push_back( p );
	}
	//
	CheckStability();
	StartGame();
	if ( IsValid( pOwnScript ) )
	{
		pScript = pOwnScript;
	}
	if ( pPostInfo )
	{
		*pPostInfo = new CPostWorldCreateInfo;
		(*pPostInfo)->scripts = mapInfo.scripts;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::RunPostInit( CPostWorldCreateInfo *pPostInfo )
{
	if ( !pPostInfo )
	{
		ASSERT(0);
		return;
	}
	if ( IsValid( pScript ) )
	{
		for ( list<CDBPtr<NDb::CScript> >::const_iterator is = pPostInfo->scripts.begin(); is != pPostInfo->scripts.end(); ++is )
		{
			int nRet = pScript->DoString( (*is)->strCode.c_str() );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::CreateDefault()
{
	CFWContext world( &pCurrentWorld, this );
	CDGPtr<CFuncBase<STime> > pRefreshTime(pTime);
	pRefreshTime.Refresh();
	//
	pAIMap = NAI::CreateAIMap( this );
	pPathNetwork = NAI::CreateNodesNetwork( pAIMap );
	pRPGGame = NRPG::CreateGame( pAIMap, pPathNetwork );
	
	nRootLayersGroup = pPathNetwork->CreateLayersGroup( 16, 16, CVec2(0,0), 0, 0 );

	STerrainInfo sInfo;
	CreateFakeTerrainInfo( &sInfo );
	pTerrainInfo = new CTerrainInfoHolder( sInfo );
	list<SMapHole> holes;
	list<SMapWall> walls;
	pTerrain = new NWorld::CTerrain( pShow, pTerrainInfo, pAIMap, pTime, nRootLayersGroup, holes, walls );

	pAIMap->Sync();

	vector<NAI::SPathPlace> empty;
	pPathNetwork->UpdateColouring( empty );

	StartGame();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::GetPassageDeployPlace( CPassageObject *pPassage, 
	CUnitServer *pUS,	const vector<NAI::SPathPlace> &lockedPlaces, NAI::SPathPlace *pPathPlace )
{
	vector<NAI::SPathPlace> places;
	vector<NAI::SPathPlace> approaches;
	pPassage->GetApproaches( &approaches, GetPathNetwork() );
	for ( vector<NAI::SPathPlace>::iterator i = approaches.begin(); i != approaches.end(); ++i )
	{
		NAI::CMultiMovesTable MovesTable;
		NAI::EPose pose = pUS->GetWishPose();
		pUS->SetWishPose( NAI::WALK );
		list<NAI::SPathPlace> tmpPlaces;
		PrepareAllPaths( &MovesTable, &tmpPlaces, pUS, *i, pPassage->GetAPRadius(), pUS, true );
		for ( list<NAI::SPathPlace>::iterator j = tmpPlaces.begin(); j != tmpPlaces.end(); ++j )
		{
			bool bCorrect = true;
			for ( vector<NAI::SPathPlace>::const_iterator l = lockedPlaces.begin(); l != lockedPlaces.end(); ++l )
				if ( (*j).GetLayer() == (*l).GetLayer() && (*j).GetX() == (*l).GetX() && (*j).GetY() == (*l).GetY() )
				{
					bCorrect = false;
					break;
				}
			if ( bCorrect )
				places.push_back( *j );
		}
		pUS->SetWishPose( pose );
	}
	//
	if ( places.size() > 0 )
	{
		*pPathPlace = places[ random.Get( 0, places.size() ) ];
		return true;
	}
	else
		return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IPlayer* CWorld::AddPlayer( const wstring &wsName, NRPG::CGlobalPlayer *pGlobalPlayer, CCommander *_pCommander )
{
	CFWContext world( &pCurrentWorld, this );
  //  
	CPlayer *pRes = new CPlayer( wsName, pGlobalPlayer );
	RegisterPlayer( pRes );
	pRes->SetCommander( _pCommander );

	if ( nRootLayersGroup < 0 )
		return pRes;
	// deploy place
	vector<NAI::SPathPlace> lockedPlaces;
	bool bSetDeploySpot = true;
	NAI::SPathPlace currentDeployPlace;
	if ( nPartiesAdded < deploySpots.size() )
		currentDeployPlace = deploySpots[ nPartiesAdded ];
	else
	{
		CVec3 ptCenter = CVec3( 1 * FP_GRID_STEP, 8 * FP_GRID_STEP, 0 );
		NAI::SPosition pp;
		pPathNetwork->SetOnLayer( &pp, 0, ptCenter );
		currentDeployPlace = pp.p;
		currentDeployPlace.SetDirection( 0 );
	}
	// поворачиваем deploy так, чтобы смотреть в центр зоны
	currentDeployPlace = pPathNetwork->GetDeployPlace( currentDeployPlace, true );
	//
	list< CPtr<CPassageObject> > passageObjects;
	GetPassageObjects( pGlobalPlayer->deployData.nPassageZoneID, &passageObjects );
	//
  for ( int i = 0; i < pGlobalPlayer->mercs.size(); i++ )
	{
		if ( pGlobalPlayer->IsUnitMarkedDead( pGlobalPlayer->mercs[i]->pRPGUnit ) )
			continue;
		//
		NAI::SPathPlace place;
 		CPtr<CUnitServer> pUS = AddUnit( currentDeployPlace, NRPG::CreateUnit( pGlobalPlayer->mercs[i], 0 ), pRes );
		ASSERT( IsValid( pUS ) );
		if ( !IsValid( pUS ) )
			continue;
		//
		if ( pGlobalPlayer->deployData.bPassage )
		{
			// переход
			CPtr<CPassageObject> pPassageObject = 0;
			for ( list< CPtr<CPassageObject> >::iterator j = passageObjects.begin(); j != passageObjects.end(); ++j )
				if ( (*j)->GetPassageObjectID() == 
					pGlobalPlayer->deployData.unitsDeployData[ pGlobalPlayer->mercs[i]->pRPGUnit.GetPtr() ].nPassageObjectID )
				{
					pPassageObject = *j;
					break;
				}
			//
			ASSERT( IsValid( pPassageObject ) );
			if ( !IsValid( pPassageObject ) )
				continue;
			//
			NAI::SPathPlace tmpPlace;
			if ( GetPassageDeployPlace( pPassageObject, pUS, lockedPlaces, &tmpPlace ) )
				place = tmpPlace;
			else
			{
				ASSERT( 0 && "Incorrect location of passage object" );
				place = currentDeployPlace;
				currentDeployPlace = pPathNetwork->GetDeployPlace( currentDeployPlace, false );
			}
		}
		else
		{
			// обычный deploy
			place = currentDeployPlace;
			currentDeployPlace = pPathNetwork->GetDeployPlace( currentDeployPlace, false );
		}
		//
		lockedPlaces.push_back( place );
		place.SetPose( NAI::CM_STAND );
		NAI::SUnitPosition pos;
		pos = pUS->GetPosition();
		pos.pos.p = place;
		pUS->SetPosition( pos );
		if ( bSetDeploySpot )
		{
			pRes->SetDeploySpot( place );
			bSetDeploySpot = false;
		}
	}
	//
	pGlobalPlayer->deployData.bPassage = false;
	++nPartiesAdded;
	pRes->OnTBSEvent( TBS_START_NEW_TURN );
	OnNewPlayerTurn( pRes );
	StartGame();
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::GetActiveUnits( IPlayer *_pPlayer, list<CUnit*> *pRes )
{
	CDynamicCast<CPlayer> pPlayer( _pPlayer );
	CPlayer::CUnitSet units;
	pPlayer->GetUnits( &units );
	pRes->clear();
	for ( CPlayer::CUnitSet::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( IsUnitActive( *i ) )
			pRes->push_back( *i );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// for debug purposes
static void VisualizeSoundRadius( CVec3 ptCenter, float fRadius )
{
	sphereParticles.clear();
	for ( int nPhi = 0; nPhi < 360; nPhi += 2 )
	{
		float fPhi = 360.0 / 3.1415 * nPhi;
		CVec3 ptSphereCenter = ptCenter;
		ptSphereCenter.x += fRadius * cos( fPhi );
		ptSphereCenter.y += fRadius * sin( fPhi );
		ptSphereCenter.z += 0.3f;
		sphereParticles.push_back( SSphere( ptSphereCenter, 0.05f ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::CreateSoundStuff( vector<CObj<CTimedObject> > *stuff, CVec3 ptPos )
{
	// create particle to show user where enemy is
	CTimedObject *pParticles = new CDParticles( ptPos,
		CQuat(0,CVec3(0,0,1)), NDb::GetEffect(N_SOUND_PARTICLE_ID) );
	pParticles->Attach( pShowUnits, this );
	stuff->push_back( pParticles );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::SetAudible( CUnitServer *pHearer, CUnitServer *pSource )
{
	vector<CObj<CTimedObject> > stuff;
	CreateSoundStuff( &stuff, pSource->GetPosition().GetCP() );
	pHearer->HearSound( stuff, pSource, pSource->GetPosition().pos.p );
	pHearer->SetAudible( pSource, true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::MakeAISound( NDb::CAISound *pAISound, CDumbUnitServer *_pWho, int nSoundType, NDb::CSound *pSound )
{
	CDynamicCast<CUnitServer> pWho( _pWho );
	if ( !IsValid( pWho ) )
		return;

	CVec3 ptFrom = pWho->GetPosition().GetCP();

	vector<CObj<CTimedObject> > stuff;
	//CTimedObject *pSound = _pSound;
	if ( pSound == 0 )
		pSound = pAISound->pSound;
	if ( pSound )
	{
		CTimedObject *p = new C3DSound( ptFrom, pSound );
		p->Attach( pShowUnits, this );
		stuff.push_back( p );
		pWho->AttachMiscObject( p );
	}
	CreateSoundStuff( &stuff, pWho->GetPosition().GetCP() );
	// DEBUG{
	//float fTmpRadius = pAISound->GetRadiusFromAISoundType( pWho->GetAISoundType() ) * FP_GRID_STEP;
	//VisualizeSoundRadius( pWho->GetPosition().GetCP(), fTmpRadius );
	// EODEBUG}

	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		CUnitServer *pTarget = *i;
		if ( pTarget != pWho )
		{
			float fDistance = 
				fabs( pTarget->GetPosition().GetCP() - pWho->GetPosition().GetCP() ) / FP_GRID_STEP;
			// 
			if ( fDistance > pTarget->GetUnitRPG()->GetAISoundConstants()->nExitRadius )
				pTarget->SetAudible( pWho, false );
			//
			if ( pTarget->IsAudible( pWho ) || 
				pTarget->CanHearSound( pWho->GetPosition().GetCP(), pAISound, nSoundType, _pWho ) )
			{
				pTarget->HearSound( stuff, pWho, pWho->GetPosition().pos.p );
				pTarget->SetAudible( pWho, true );
				GetAISignalManager()->Add( NAI::CreateAIUnitSoundSignal( pWho, pTarget, pAISound->fRadius ) );
			}
			else
				pTarget->ClearSound( pWho );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::MakeSound( const CVec3 &ptCenter, NDb::CSound *pSound )
{
	if ( pSound )
		AttachMiscObject( new C3DSound( ptCenter, pSound ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::PlaceAllUnits()
{
	for ( list<CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
		(*i)->PlaceOnPassablePlace();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::StartGame()
{
	pAIMap->Sync();
	PlaceAllUnits();
	pAIMap->Sync();
	UpdateVisible();
	StartTBSGame();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::ExecuteCommand( CCommand *_pCmd ) 
{
	CObj<CCommand> pHold(_pCmd); 
	CDynamicCast<CCmdUnit> pUnitCmd( _pCmd );
	if ( pUnitCmd )
	{
		CDynamicCast<CUnitServer> pUS( pUnitCmd->pUnit );
		ASSERT( pUS );
		ASSERT( IsUnitActive( pUS ) );
		pUS->Do( _pCmd );
	}
	else
		ASSERT( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CHitLocator* CWorld::GetHitEvent()
{
	if ( eventHits.empty() )
		return 0;
	CHitLocator* pPart = eventHits.front().Extract();
	eventHits.pop_front();
	return pPart;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CActionLocator* CWorld::GetActionEvent( IPlayer *pViewFrom )
{
	CDynamicCast<CPlayer> pPlayer( pViewFrom );
	if ( eventActions.empty() )
		return 0;
	CPtr<CActionLocator> pPart = eventActions.front().Extract();
	eventActions.pop_front();
	if ( pPlayer && pPart->pUnit )
	{
		if ( !pPlayer->CanSee( GetUnit( pPart->pUnit ) ) )
			return GetActionEvent( pViewFrom );
	}
	return pPart.Extract();;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::GetAcknowledgement( IPlayer *pPlayer, vector<CPtr<CAckEvent> > *pEventsSet )
{
	CPtr<NDb::CDBAckSequence> pSeq = pGlobalAck->GetSequence( pPlayer );

	if ( !IsValid( pSeq ) )
		return false;

	int nCount = 0;
	while ( pSeq->pDBAckInfo[nCount] ) nCount++;

	pEventsSet->resize( nCount );
	for ( int nTemp = 0; nTemp < nCount; nTemp++ )
	{
		CPtr<NWorld::CUnit> pUnit = 0;
		for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
		{
			if ( (*i)->GetUnitRPG()->GetRPGPersID() == pSeq->pDBAckInfo[nTemp]->nRPGPersID )	
			{
				pUnit = *i;
				break;
			}
		}
		ASSERT( pUnit );

		(*pEventsSet)[nTemp] = new CAckEvent( pSeq->nPriority, pUnit, pSeq->pDBAckInfo[nTemp] );
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::Explode( const CVec3 &ptEpicentre, int nPower )
{
	AttachMiscObject( new CDGrassEvent( ptEpicentre ) );
	CreateParticle( ptEpicentre, CQuat(random.GetFloat(0,10000),CVec3(0,0,1)), NDb::GetEffect( 51 ) );
	//
	MakeSound( ptEpicentre, NDb::GetSound(63) );
	//
	list< CObj<CBuilding> >::const_iterator it;
	for ( it = buildings.begin(); it != buildings.end(); ++it )
		(*it)->Explode( ptEpicentre, nPower );
	SSphere sphere;
	sphere.ptCenter = ptEpicentre;
	sphere.fRadius = 15;
	ActivateDebris( sphere, GetAIMap(), pTime );
	//
	CheckStability();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::GenerateDebris( NDb::CDebrisMaterial *pDebrisMaterial, const CVec3 &ptCenter, const CVec3 &ptDir, int nDebris )
{
	if ( !pDebrisMaterial )
		return;
	for ( int i=0; i<nDebris; ++i )
	{
		NDb::CDebris *pDebris = pDebrisMaterial->GetDebris();
		if ( !pDebris )
			continue;
		SRand rand;
		CPtr<NDb::CModel> pModel = pDebris->pModel->CreateModel( &rand );
		CVec3 randVel;
		randVel.x = random.GetFloat(-1,1);
		randVel.y = random.GetFloat(-1,1);
		randVel.z = random.GetFloat(1,3);
		AddDebris( pModel, pAIMap, ptCenter + CVec3(0,0,1), QNULL, randVel, pTime );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::CheckStability()
{
	bool bNeedRecalc = true;
	while ( bNeedRecalc )
	{
		pAIMap->Sync();
		bNeedRecalc = false;
		list< CObj<CObjectServerBase> >::const_iterator io;
		for ( io = objects.begin(); io != objects.end(); )
		{
			CObjectServerBase *pTest = *io++;
			if ( !pTest->CheckStability() )
				bNeedRecalc = true;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::Segment()
{
	TTBSWorld::Segment();
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
		(*i)->Segment();
	CallSegment( &miscObjects );
	for ( list< CPtr<CObjectServerBase> >::iterator i = segmentObjects.begin(); i != segmentObjects.end(); )
	{
		if ( !IsValid(*i) || (*i)->Segment() )
			i = segmentObjects.erase( i );
		else
			++i;
	}

	SSphere changed;
	bool bChanged = CDebrisController::Segment( &changed );

	MarkNewDGFrame();
	pAIMap->Sync();

	pGlobalAck->OnSegment();
	pAIJobManager->Segment();
	pAISignalManager->Segment();

	pScript = pOwnScript;
	pOwnScript->ExecuteThreads();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::CanSeeAction( IPlayer *_pPlayer )
{
	CDynamicCast<CPlayer> pPlayer( _pPlayer );
	return IsRealTime() || CanPlayerSeeAction( pPlayer );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::UpdateWorld( STime tScene, IPlayer *pPlayer )
{
	CFWContext world( &pCurrentWorld, this );
	ASSERT( IsValid( pTime ) );
	STime tCurrent = pTime->GetValue();
	int nSkipped = 0;
	tScene += tHiddenDelta;
	while ( tScene >= tCurrent ) // CRAP - time wrap problem
	{
		tCurrent += DW_SEGMENT_TIME;
		pTime->Set( tCurrent );
		MarkNewDGFrame();
		CDGPtr<CFuncBase<STime> > pRefreshTime(pTime);
		pRefreshTime.Refresh();
		pAimTime->Set( tCurrent - tHiddenDelta );
		CDGPtr<CFuncBase<STime> > pRefreshAimTime( pAimTime );
		pRefreshAimTime.Refresh();
		Segment();
		if ( IsValid(pPlayer) && CanSkip( CDynamicCast<CPlayer>(pPlayer) ) && nSkipped < 5 )
		{
			tHiddenDelta += DW_SEGMENT_TIME;
			tScene += DW_SEGMENT_TIME;
			nSkipped++;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::KillObject( CObjectServerBase *pOS )
{
	objects.remove( pOS );
	if ( CDynamicCast<NWorld::IDynamicObject> pDyn(pOS) )
	{
		CObj<NWorld::IDynamicObject> pObj(pDyn);
		miscObjects.remove( pObj );
	}
/*	CWObject *pO = pOS;
	showObjects.Remove( pO );
	GenerateDebris( pOS->GetDebrisMaterial(), pOS->GetPosition(), ptDir, 5 );
	objects.remove( pOS );
	ActivateDebris( SSphere( pOS->GetPosition(), 5 ), GetAIMap(), pTime );*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CCoverInfo* CWorld::CalcCovers( const CVec3 &ptSrc, const NRPG::CAttackPortion &attack, 
	CUnit *pIgnore, CUnit *pTarget, int nTargetUserID, float fMinClearDistance )
{
	return pRPGGame->CalcCovers( ptSrc, attack, pIgnore, pTarget, nTargetUserID, fMinClearDistance );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CCoverInfo* CWorld::CalcCoversForTile( const CVec3 &ptSrc, const NRPG::CAttackPortion &attack, CUnit *pIgnore,
	const CVec3 &ptTarget, float fMinClearDistance )
{
	return pRPGGame->CalcCoversForTile( ptSrc, attack, pIgnore, ptTarget, fMinClearDistance );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::PerformMeleeAttack( const NRPG::CAttackPortion &ap, const CRay &ray, const vector<NRPG::IAttackable*> &ignores )
{
	pRPGGame->ProcessMeleeAttackPortion( ap, ray, ignores );
/*
	for ( int k = 0; k < exps.size(); ++k )
	{
		const NRPG::SWound &w = exps[k];
		NDb::CRPGArmor *pArmor = w.pArmor;
		if ( !pArmor )
			continue;
		NDb::CSound *pS = NDb::GetSound( pArmor->pSoundShot );
		//if ( pS )
		//	MakeSound( w.ptWhere, pS );
		if ( pArmor->pShotEffect )
		{
			CVec3 dir = w.ptSmokeDir;
			CQuat rndX( random.GetFloat(0,10000), CVec3(1,0,0) );
			if ( fabs( dir.x ) < 0.999f )
				rndX = CQuat( acos( dir.x ), CVec3( 0, -dir.z, dir.y ), true ) * rndX;
			else if ( dir.x < 0 )
				rndX = CQuat( FP_PI, CVec3(0,1,0) ) * rndX;
			SRand rnd;
			CreateParticle( w.ptWhere, rndX, pArmor->pShotEffect->GetEffect( &rnd ) );
		}
	}
*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::PerformRangedAttack( const NRPG::CAttackPortion &ap, const CRay &ray, const vector<NRPG::IAttackable*> &ignores, STime sCast, NDb::CModel *pTrailModel, float fTrailSpeed )
{
	vector<NRPG::STrailPoint> trail;
	pRPGGame->ProcessRangedAttackPortion( ap, ray, ignores, &trail );

	miscObjects.push_back( new CBulletServer( this, trail, sCast, pTrailModel, fTrailSpeed ) );
/*
	for ( int k = 0; k < exps.size(); ++k )
	{
		const NRPG::SWound &w = exps[k];
		NDb::CRPGArmor *pArmor = w.pArmor;
		if ( !pArmor )
			continue;
		NDb::CSound *pS = NDb::GetSound( pArmor->pSoundShot );
		//if ( pS )
		//	MakeSound( w.ptWhere, pS );
		if ( pArmor->pShotEffect )
		{
			CVec3 dir = w.ptSmokeDir;
			CQuat rndX( random.GetFloat(0,10000), CVec3(1,0,0) );
			if ( fabs( dir.x ) < 0.999f )
				rndX = CQuat( acos( dir.x ), CVec3( 0, -dir.z, dir.y ), true ) * rndX;
			else if ( dir.x < 0 )
				rndX = CQuat( FP_PI, CVec3(0,1,0) ) * rndX;
			SRand rnd;
			CreateParticle( w.ptWhere, rndX, pArmor->pShotEffect->GetEffect( &rnd ) );
		}
	}
*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::ThrowGrenade( const CVec3 &vFrom, const CVec3 &vSpeed, STime tThrow, 
	float fTFly, NDb::CModel *pModel, NDb::CRPGGrenade *pRPGGrenade, CUnitServer *pUnitServer )
{
	miscObjects.push_back( new CGrenadeServer( this, vFrom, vSpeed, tThrow, 
		fTFly, pModel, pRPGGrenade, pUnitServer ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::ThrowKnife( const CVec3 &vFrom, const CVec3 &vSpeed, STime tThrow, float fDistance,
	NDb::CModel *pModel, NRPG::CAttackPortion &attack, NRPG::IInventoryItem *pIItem )
{
	miscObjects.push_back( new CKnifeServer( this, vFrom, vSpeed, tThrow, 
		fDistance, pModel, attack, pIItem ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::LaunchRocket( const CVec3 &vFrom, const CVec3 &vSpeed,
		STime tThrow, float fDistance, NDb::CModel *pModel, NRPG::CAttackPortion &attack, 
		NRPG::IClipItem *pRocket, CUnitServer *pIgnored, NDb::CEffect *pEffect   )
{
	miscObjects.push_back( new CRocketServer( this, vFrom, vSpeed, tThrow, 
		fDistance, pModel, attack, pRocket, pIgnored, pEffect ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_UNIT_REACH_DISTANCE = 20.0f;
const float F_UNIT_REACH_COLLIDER_RADIUS = 0.3f;
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::FindCloseGroundItems( CUnit *pU, vector<SItem> *pRes )
{
	CVec3 tracePos = pU->GetPosition().GetCenter();
	SBound bound;
	bound.SphereInit( tracePos, F_UNIT_REACH_DISTANCE + 1.0f );
	CPtr<NAI::CCollider> pCollider = new NAI::CCollider;
	NAI::CCollider &collider = *pCollider;
	GetAIMap()->PrepareCollider( &collider, bound, F_UNIT_REACH_COLLIDER_RADIUS, TS_PASS_BLOCKER );//TS_TERRAINS|TS_OBJECTS );

	list< CObj<CDFrozenItem> >::iterator it;
	for ( it = showFrozenItems.begin(); it != showFrozenItems.end(); ++it )
	{
		if ( !(*it)->GetInvItem() )
			continue;
		CPtr<NDb::CModel> pModel = (*it)->GetModel();
		bool bReach = false;
		vector<bool> results;
		vector<SSphere> spheres;
		vector<CVec3> vels;	
		vector<SMassSphere> massSpheres;
		CVec3 massCenter;
		NAI::GetSpheres( pModel, &massSpheres, &massCenter );
		if ( massSpheres.size() )
		{
			for ( int nSphere = 0; nSphere < massSpheres.size(); ++nSphere )
			{
				CVec3 from;
				(*it)->GetMatrix().RotateHVector( &from, massSpheres[nSphere].ptCenter );
				if ( fabs2( from - tracePos ) > sqr(F_UNIT_REACH_DISTANCE) )
					continue;
				spheres.push_back( SSphere( from, massSpheres[nSphere].fRadius ) );
				vels.push_back( tracePos - from );
			}
		}
		else
		{
			// CRAP
			CVec3 from = (*it)->GetPos();
			if ( fabs2( from - tracePos ) < sqr(F_UNIT_REACH_DISTANCE) )
			{
				spheres.push_back( SSphere( from, 0.01f ) );
				vels.push_back( tracePos - from );
			}
		}
		collider.CollideCheck( spheres, vels, &results );
		for ( int i = 0; i < results.size(); ++i )
			if ( !results[i] )
				bReach = true;
		if ( bReach )
		{
			SItem &item = *pRes->insert( pRes->end() );
			item.eType = SItem::GROUND;
			item.pItem = (*it)->GetInvItem();
			item.pWorldItem = (*it);
			item.pUnit = 0;
			item.nSlot = -1;
		}
	}
	list< CObj<CUnitServer> >::iterator itU;
	for ( itU = units.begin(); itU != units.end(); ++itU )
	{
		if ( !(*itU)->IsDead() || !(*itU)->IsUnconscious() )
			continue;
		CVec3 from = (*itU)->GetPosition().GetCP(); // CRAP
		if ( fabs2(from - tracePos) > sqr(F_UNIT_REACH_DISTANCE) )
			continue;
		vector<bool> results;
		vector<SSphere> spheres;
		vector<CVec3> vels;	
		spheres.push_back( SSphere( from, 0.1f ) );
		vels.push_back( tracePos - from );
		collider.CollideCheck( spheres, vels, &results );
		if ( results[0] )
			continue;
		CPtr<NRPG::IInventoryInfo> pDeadInventory = (*itU)->GetRPG()->GetInventoryInfo();
		const vector<NRPG::SBackPackItem> &backItems = pDeadInventory->GetItems();
		vector<NRPG::SBackPackItem>::const_iterator itI;
		for ( itI = backItems.begin(); itI != backItems.end(); ++itI )
		{
			SItem &item = *pRes->insert( pRes->end() );
			item.eType = SItem::GROUND;
			item.pItem = itI->pItem;
			item.pUnit = *itU;
			item.nSlot = -1;
		}
		for ( int nSlot = 0; nSlot < NDb::N_SLOTS; ++nSlot )
			if ( pDeadInventory->Get( (NDb::ESlot)nSlot ) )
			{
				SItem &item = *pRes->insert( pRes->end() );
				item.eType = SItem::GROUND;
				item.pItem = pDeadInventory->Get( (NDb::ESlot)nSlot );
				item.pUnit = *itU;
				item.nSlot = nSlot;
			}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::IsWinnerPlayer( IPlayer *pPlayer )
{
	CDynamicCast<CPlayer> p( pPlayer );
	if ( nAIUnitsCreated == 0 && nPartiesAdded <= 1 )
		return false;
	return !HasEnemies( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::MakeExplosion( const CRay &ray, int nMaxFloor )
{
	CPtr<CUnitServer> pThrower = 0;
	if ( !units.empty() )
		pThrower = units.front();
	//
	if ( IsValid( pThrower ) )
	{
		int nUserID;
		CVec3 ptPoint;
		NWorld::IVisObj *pUserData;
		if ( TraceRay( ray, nMaxFloor, &pUserData, &nUserID, &ptPoint ) )
			AddGrenadeExplosion( ptPoint, NDb::GetRPGGrenade( 4 ), pThrower );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::AddGrenadeExplosion( const CVec3 &vStartPosition, NDb::CRPGGrenade *pRPGGrenade,
	CUnitServer *pUnitServer )
{
	SRand rnd;
	if ( pRPGGrenade->pExplodeEffect )
		CreateParticle( vStartPosition, CQuat(random.GetFloat(0,10000),CVec3(0,0,1)), pRPGGrenade->pExplodeEffect->GetEffect( &rnd ) );
	AttachMiscObject( new CDFlash( vStartPosition + CVec3(0,0,5), CVec3(1,1,1), 10, 3 ) );
	MakeSound( vStartPosition, pRPGGrenade->pSound );
	GetAISignalManager()->Add( NAI::CreateAIGrenadeSoundSignal( vStartPosition ) );
	miscObjects.push_back( new CVoxelExplTracker( vStartPosition, pRPGGrenade, pUnitServer, this ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer *CWorld::GetUnitServer( string szName )
{
	string szUpperName = szName;
	NStr::ToUpper( szUpperName );
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		CDBPtr<NDb::CRPGPers> pPers = (*i)->GetUnitRPG()->GetRPGUnit()->pPers;
		if ( !IsValid( pPers->pName ) )
			continue;
		//
		string szUnitName = NStr::ToAscii( pPers->pName->szStr );
		NStr::ToUpper( szUnitName );
		if ( szUnitName == szUpperName )
			return *i;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer *CWorld::GetUnitServer( NRPG::IUnitMissionInfo *pUnitMission )
{
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
		if ( (*i)->GetUnitRPG() == pUnitMission )
			return *i;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer *CWorld::GetUnitServer( NRPG::CUnit *pRPGUnit )
{
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
		if ( (*i)->GetUnitRPG()->GetRPGUnit() == pRPGUnit )
			return *i;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer *CWorld::GetUnitServerByPersID( int nPersID )
{
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
		if ( (*i)->GetUnitRPG()->GetRPGPersID() == nPersID )
			return *i;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWorld::UsePassageObject( CUnitServer *pUS, int nPassageZoneID )
{
	ASSERT( IsValid( pUS ) );
	if ( !IsValid( pUS ) )
		return false;
	//
	CPtr<NScenario::CScenarioZone> pZone = GetGlobalGame()->pCurrentZone;
	if ( !IsValid( pZone ) )
		return false;
	//
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer = pUS->GetPlayer()->GetGlobalPlayer();
	if ( !IsValid( pGlobalPlayer ) )
		return false;
	//
	list< CPtr<CPassageObject> > passageObjects;
	GetPassageObjects( nPassageZoneID, &passageObjects );
	// проверяем все ли стоят рядом с найденными объектами перехода
	bool bCanPass = true;
	hash_map< CPtr<CUnitServer>, CPtr<CPassageObject>, SPtrHash > passagesForUnits;
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( !(*i)->IsUnconscious() && 
			!(*i)->GetUnitRPG()->IsDead() && (*i)->GetPlayer() == pUS->GetPlayer() )
		{
			bool bCanUnitPass = false;
			for (	list< CPtr<CPassageObject> >::iterator j = passageObjects.begin(); j != passageObjects.end(); ++j )
				if ( (*j)->CanPass( *i ) )
				{
					bCanUnitPass = true;
					passagesForUnits[ (*i).GetPtr() ] = *j;
					break;
				}	
			//
			bCanPass &= bCanUnitPass;
		}
	}
	//
	if ( !bCanPass )
		return false;
	//
	/*
	for ( list< CObj<CUnitServer> >::iterator l = units.begin(); l != units.end(); ++l )
		if ( (*l)->GetPlayer() == pUS->GetPlayer() && 
			(*l)->IsUnconscious() && !IsValid( (*l)->GetCorpseCarrier() ) )
			{
				(*l)->GetUnitRPG()->GetRPGUnit()->Kill();
			}
	*/
	//
	if ( nPassageZoneID <= 0 )
	{
		// выходим на chapter map
		NMainLoop::Command( new NGame::CICContinueChapter( GetGlobalGame() ) );
		return true;
	}
	// ищем в какой template переходим
	for ( hash_map< int, NScenario::CScenarioZone::STemplate >::iterator i = pZone->templates.begin();
			i != pZone->templates.end(); ++i )
	{
		if ( i->first != GetGlobalGame()->nCurrentTemplateID )
		{
			// ищем passage objects
			SMapInfo info;
			vector<string> strParams;
			int nVariantID = i->second.nVariantID;
			if ( !BuildMap( nVariantID, strParams, 0, &info, -1, i->second.sSeed ) )
				continue;
			//
			for ( list<SMapElement>::const_iterator j = info.items.begin(); j != info.items.end(); ++j )
			{
				if ( IsValid( (*j).pObject->pPassage ) && (*j).nPassageZoneID == nPassageZoneID )
				{
					// заполняем данные о переходе
					NRPG::SDeployData &deployData = pGlobalPlayer->deployData;
					deployData.bPassage = true;
					deployData.nPassageZoneID = nPassageZoneID;
					//
					hash_map< CPtr<CUnitServer>, CPtr<CPassageObject>, SPtrHash >::iterator u;
					for ( u = passagesForUnits.begin(); u != passagesForUnits.end(); ++u )
						deployData.unitsDeployData[ u->first->GetUnitRPG()->GetRPGUnit() ].nPassageObjectID =
							u->second->GetPassageObjectID();
					// переходим
					NMainLoop::Command( new NGame::CICBeginMission( nVariantID, GetGlobalGame() ) );
					return true;
				}
			}
		}
	}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::OnFrozenItemDestroyed( int nItemID )
{
	GetGlobalGame()->pScenarioTracker->OnScenarioClueDestroyed( nItemID, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::OnNewPlayerTurn( CPlayer *pPlayer )
{
	GetGlobalAck()->OnNewTurnStarted( pPlayer );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::OnRealTimeStarted()
{
	GetGlobalAck()->OnRealTimeStarted();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CWorld::GetEnemyWatchers( IPlayer *pPlayer ) const
{
	return TTBSWorld::GetEnemyWatchers( dynamic_cast<CPlayer*>( pPlayer ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::OnAction( bool bStartAction )
{
	for ( list<CObj<CUnitServer> >::iterator u = units.begin(); u != units.end(); ++u )
	{
		CUnitServer *pUS = *u;
		pUS->animator.IdleBan( NAnimation::E_NO_IDLE_ON_ACTION, bStartAction );
	}
	if ( !bStartAction )
		CheckStability();
	pPathNetwork->Freeze( bStartAction );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::GetPassageObjects( int nPassageZoneID, list< CPtr<CPassageObject> > *pPassageObjects )
{
	pPassageObjects->clear();
	// ищем объекты перехода этой зоны перехода
	for ( list< CObj<CObjectServerBase> >::iterator i = objects.begin(); i != objects.end(); ++i )
		if ( CDynamicCast<CPassageObject> pPassage( *i ) )
			if ( pPassage->GetPassageZoneID() == nPassageZoneID )
				pPassageObjects->push_back( pPassage.GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::InitCorpseCarrying()
{
	list< CPtr<CPlayer> > players;
	GetPlayersList( &players );
	for ( list< CPtr<CPlayer> >::iterator i = players.begin(); i != players.end(); ++i )
		InitPlayerCorpseCarrying( *i );	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::InitPlayerCorpseCarrying( CPlayer *pPlayer )
{
	ASSERT( IsValid( pPlayer ) );
	if ( !IsValid( pPlayer ) )
		return;
	//
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer = pPlayer->GetGlobalPlayer();
	if ( !IsValid( pGlobalPlayer ) )
		return;
	//
	vector< CPtr<CUnit> > units;
	pPlayer->GetUnits( &units );
	for ( vector< CPtr<CUnit> >::iterator i = units.begin(); i != units.end(); ++i )
		if ( CDynamicCast<CUnitServer> pUS( *i ) )
		{
			NRPG::SUnitDeployData &deployData = 
				pGlobalPlayer->deployData.unitsDeployData[ pUS->GetUnitRPG()->GetRPGUnit() ];
			CPtr<NRPG::CUnit> pCorpse =	deployData.pCorpse;
			if ( IsValid( pCorpse ) )
			{
				CPtr<CUnitServer> pCorpseUS = GetDeployedDeadUnit( pUS->GetPosition().pos.p, pCorpse );
				if ( IsValid( pCorpseUS ) )
				{
						pUS->Do( new CCmdSetCommand( pUS.GetPtr(), new CCmdTakeCorpseOnDeploy( pUS, pCorpseUS, !deployData.bCorpseAlive ) ) );
						pUS->Do( new CCmdSetCommand( pUS.GetPtr(), new CCmdContinue() ) );
				}
			}
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWorld::GetScenarioPlayerUnits( int nScenarioPlayer, vector< CPtr<CUnitServer> > *pUnits )
{
	pUnits->clear();
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
		if ( !(*i)->IsDead() && !(*i)->IsUnconscious() &&
			(*i)->GetUnitRPG()->GetScenarioPlayer() == nScenarioPlayer )
		{
			pUnits->push_back( (*i).GetPtr() );
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace NWorld

REGISTER_SAVELOAD_CLASS_NM( 0x0251101b, CWorld, NWorld );
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0x02731131, CPlayer )
BASIC_REGISTER_CLASS( CPostWorldCreateInfo )
