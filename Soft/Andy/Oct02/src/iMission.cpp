#include "StdAfx.h"
#include "wInterface.h"
#include "Transform.h"
#include "DiscretePos.h"
#include "GView.h"
#include "G2DView.h"
#include "GSceneUtils.h"
#include "Sound.h"
#include "RWGame.h"
#include "RWSound.h"
#include "RPGGame.h"
#include "RPGGlobal.h"
#include "RPGUnit.h"
#include "RPGMerc.h"
#include "RPGUnitInfo.h"
#include "RPGItemInfo.h"
#include "..\Misc\Commands.h"
#include "..\Misc\LogStream.h"
#include "..\Misc\StrProc.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataSound.h"
#include "iMain.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iMission.h"
#include "iInterMission.h"
#include "iChapterMap.h"
#include "iInGameMenu.h"
#include "iCluesMenu.h"
#include "iTeamMngMenu.h"
#include "iAIViewer.h"
#include "iRadTest.h"
#include "iGameStates.h"
#include "aiInterval.h"
#include "aiMap.h"
#include "iMissionUI.h"
#include "iMissionInternal.h"
#include "UnitTracker.h"
#include "PlayerTracker.h"
////
#include "MemObject.h"
#include "BSchemaViewer.h"
#include "MakeBuilding.h"
#include "MapBuildingInfo.h"
#include "scScenarioTracker.h"
#include "scFlowChartItems.h"
#include "RPGDiplomacy.h"
#include "..\DBFormat\DataScenario.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_SCROLL_STEP				= 4,
	N_SCROLL_GUARDBAND	= 4;
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool bShowAllCheat = false;
static float fDefaultCameraFOV = N_FOV;
static ICamera::SCameraLimits defaultCameraLimits( 10, 50, -1.5f, -0.1f, CTRect<float>( -1000, -1000, 1000, 1000 ) );
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandSurrender( const string &szID, const vector<wstring> &paramsSet, void *pContext );
static void CommandTypeCameraParams( const string &szID, const vector<wstring> &paramsSet, void *pContext );
static void CommandSetXPLevel( const string &szID, const vector<wstring> &paramsSet, void *pContext );
////
static void VarCheatSeeAll( const string &szID, const NGlobal::CValue &sValue, void *pContext );
static void VarCheatGodMode( const string &szID, const NGlobal::CValue &sValue, void *pContext );
static void VarCheatTeleport( const string &szID, const NGlobal::CValue &sValue, void *pContext );
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CMission::CMission():
	bHideInterface( false ), bSpecialHideInterface( false ), bPause( false ), actionsInfoSet( UA_MAXVALUE ), nFramesSameCameraPosition(0), vPrevCameraPosition(0,0,0),
	fFOV( fDefaultCameraFOV ), eCameraType( CAMERA_PC ), nLightMode( 0 ), bFreezeCamera( false ), bTraceOk( false ),
	bindExit( "exitgame" ), bindCancel( "cancel" ),	bindContinue( "continue" ),	bindPause( "pause" ), bindMainMenu( "mainmenu" ), bindHideInterface( "hideinterface" ), bindSpecialHideInterface( "hideinterface_special" ),
	bindHero1( "hero_1" ), bindHero2( "hero_2" ), bindHero3( "hero_3" ), bindHero4( "hero_4" ), bindHero5( "hero_5" ), bindHero6( "hero_6" ), bindSelPrev( "hero_prev" ), bindSelNext( "hero_next" ),
	bindSnapShot( "weapon_snapshot" ), bindAimedShot( "weapon_aimedshot" ), bindCarefulShot( "weapon_carefulshot" ), bindShortBurst( "weapon_shortburst" ), bindLongBurst( "weapon_longburst" ), bindSnipeShot( "weapon_snipeshot" ), bindWeaponPrevMode( "weapon_prevmode" ), bindWeaponNextMode( "weapon_nextmode" ),
	bindWeaponReload( "weapon_reload" ), bindPrevSlot( "slot_prev" ), bindNextSlot( "slot_next" ), bindItemUnload( "unload" ),
	bindSwitchLighting("switch_lighting"),
	cmdTypeCamera( "camerapos", CommandTypeCameraParams, this ), cmdSurrender( "surrender", CommandSurrender, this ), 
	varCheatGodMode( "godmode", VarCheatGodMode, this, NGlobal::CValue( L"off" ) ), varCheatSeeAll( "seeall", VarCheatSeeAll, this, NGlobal::CValue( L"off" ) ), varCheatTeleport( "teleport", VarCheatTeleport, this, NGlobal::CValue( L"off" ) ),
	bindCluesMenu( "clues" ),
	bindStartOfTurn( "startofturn" ), bindEndOfTurn( "endofturn" ), 
	bindMove( "move" ), bindAttack( "attack" ), bindFirstAid( "firstaid" ), bindDropCorpse( "dropcorpse" ), bindRotate( "unit_rotate" ), 
	bindSnipeAttack( "snipe_attack" ), bindCollect1AP( "collectap_1ap" ), bindCollect10AP( "collectap_10ap" ), bindCollectMaxAP( "collectap_max" ), bindCollectAllAP( "collectap_all" ),
	bindNormalPose( "pose_normal" ), bindCrawlPose( "pose_crawl" ), bindCrouchPose( "pose_crouch" ), bindRunPose( "pose_run" ), bindStrafe( "pose_strafe" ),
	bindFocusUnit( "focus_unit" ), bindAddFloor("next_floor"), bindSubFloor("prev_floor"),
	bindShadows("toggle_shadows"), bindFog("toggle_fog"), bindHSR("toggle_hsr"), 
	bindNextEnemy( "next_enemy" ), bindCheatTeleport( "cheat_teleport" ),bindExplode( "explode" ), bindShowAI( "showai" ), bindRPGStats( "rpg_stats" ), bindShowSchema( "viewSchema" ), bindShowParticles("switch_particles"), bindCheckBuildings( "checkStability" ), 
	bindShowVisibility( "rpg_visibility" ), bindShowRAD( "showrad" ),
	bindTestIsect("test_intersect"), bindClickOfDeath("click_of_death"), bindExplosion("explosion"),
	cmdSetXPLevel( "setxplevel", CommandSetXPLevel, this )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::Initialize( int _nTemplateID, NRPG::CGlobalGame *_pGlobalGame )
{
	// на самом деле _nTemplateID это ID варианта template-а
	nTemplateID = _nTemplateID;
	pGlobalGame = _pGlobalGame;
	int nMobsLevel = 0;
	SRandomSeed sSeed;
	CObj<NWorld::CPostWorldCreateInfo> pPostInfo;
	list< CPtr<NScenario::CScenarioClue> > clues;
	pGlobalGame->pCurrentZone = 0;
	pGlobalGame->nCurrentTemplateID = 0;
	if ( pGlobalGame->pScenarioTracker->IsScenarioAvailable() )
	{
		int nRealTemplateID = pGlobalGame->pScenarioTracker->GetTemplateIDByVariantID( nTemplateID );
		CPtr<NScenario::CScenarioZone> pZone = pGlobalGame->pScenarioTracker->GetZone( nRealTemplateID );
		if ( IsValid( pZone ) )
		{
			sSeed = pZone->templates[nRealTemplateID].sSeed;
			if ( IsValid( pGlobalGame->pCurrentZone ) && ( pZone != pGlobalGame->pCurrentZone ) )
			{
				pGlobalGame->pGlobalDiplomacy->SetWorld( 0 );
				pGlobalGame->pGlobalDiplomacy->LoadDiplomacy( pGlobalGame->pCurrentZone->GetDBZone()->GetRecordID() );
			}
			pGlobalGame->pCurrentZone = pZone;
			pGlobalGame->pScenarioTracker->GetPlacedClues( nRealTemplateID, &clues );
			nMobsLevel = pZone->nDifficulty;
			pGlobalGame->nCurrentTemplateID = nRealTemplateID;
		}	
	}
	pWorld = NWorld::CreateWorld( _pGlobalGame );
	pGlobalGame->pGlobalDiplomacy->SetWorld( pWorld );
	pWorld->CreateRandom( nTemplateID, true, clues, nMobsLevel, &pPostInfo, sSeed );

	pScene = NGScene::CreateNewView();
	pSoundScene = NSound::CreateSoundScene( NDb::GetMusic( 1 ) ); // CRAP
	pRender = NRender::CreateRenderGame( pWorld, pScene );
	pRenderSound = NRender::CreateRenderSound( pWorld, pSoundScene );
#ifdef _MAPEDIT
	pCursor = NUI::ICursor::CreateEditorCursor();
#else
	pCursor = NUI::ICursor::Create( true, NGfx::GetScreenRect() / 2 );
#endif
	pInterface = new NUI::CInterface( pCursor );

	playersSet.resize( pGlobalGame->players.size() );
	for ( int nTemp = 0; nTemp < pGlobalGame->players.size(); nTemp++ )
	{
		WCHAR wsString[1024];
		swprintf( wsString, L"Player %d", nTemp );
		playersSet[nTemp] = new CPlayerTracker( this, pGlobalGame->players[nTemp], wsString );
	}
	pWorld->InitCorpseCarrying();
	pActivePlayer = playersSet.front();
	CommandState( new CStateEmpty );

	TrackChanges();
	bUpdated = true;

	NDb::CAmbientLight *pLight = GetWorld()->GetDefaultLight();
	if ( pLight )
		GetScene()->SetAmbient( pLight );
	else
		SetLightMode( 0 );

	SetCameraParams( CAMERA_PC, fFOV, defaultCameraLimits );
	pCamera->SetPlacement( GetActivePlayer()->GetCamera() );

	TraceCursor();

	pMissionUI = new NUI::CMissionUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "missionUI" ), this );
	NUI::LoadTemplate( pMissionUI, NDb::GetUIContainer( 123 ) );
	pMissionUI->ShowWindow( NUI::SWTYPE_SHOW );

	vector<CObj<IState> > updatedStatesSet;
	updatedStatesSet.push_back( new CStateWait() );
	updatedStatesSet.push_back( new CStateDragItem() );
	updatedStatesSet.push_back( new CStateUse() );
	updatedStatesSet.push_back( new CStateTeam() );
	updatedStatesSet.push_back( new CStateAttack( false ) );
	updatedStatesSet.push_back( new CStatePickItem() );
	updatedStatesSet.push_back( new CStateMove( false ) );
	updatedStatesSet.push_back( new CStateEmpty() );
	SetUpdatedStates( updatedStatesSet );

	pWorld->RunPostInit( pPostInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::Command( NWorld::CCommand *pCmd )
{
	ASSERT( pCmd );
	pActivePlayer->GetCommander()->Do( pCmd );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::Command( NWorld::CUnit *pUnit, NWorld::CCmd *pCmd, bool bInstantly )
{
	Command( new NWorld::CCmdSetCommand( pUnit, pCmd ) );

	if ( bInstantly )
		Command( new NWorld::CCmdSetCommand( pUnit, new NWorld::CCmdContinue ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::StopAction()
{
	pActivePlayer->GetCommander()->StopAction();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::IsReady() const
{
	if ( !IsRealTime() && ( IsActionExecuted() || ( pWorld->GetCurrentPlayer() != pActivePlayer->GetPlayer() ) ) )
		return false;

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::IsActionExecuted() const
{
	return pWorld->IsExecuting();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::IsRealTime() const
{
	return pWorld->GetCurrentPlayer() == 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::PauseGame( bool bState )
{
	bPause = bState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::IsGamePaused() const
{
	return bPause;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IPlayerTracker* CMission::GetActivePlayer() const
{
	ASSERT( pActivePlayer );
	return pActivePlayer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SetActivePlayer( IPlayerTracker* pPlayer )
{
	ASSERT( find( playersSet.begin(), playersSet.end(), pPlayer ) != playersSet.end() );
	pActivePlayer = pPlayer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::GetPlayers( vector< CPtr<IPlayerTracker> > *pPlayersSet ) const
{
	ASSERT( pPlayersSet );
	pPlayersSet->clear();
	pPlayersSet->resize( playersSet.size() );
	for ( int nTemp = 0; nTemp < playersSet.size(); nTemp++ )
		(*pPlayersSet)[nTemp] = playersSet[nTemp].GetPtr();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::GetUnits( vector< CPtr<IUnitTracker> > *pUnits ) const
{
	pActivePlayer->GetUnits( pUnits );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::GetSelectedUnits( vector< CPtr<IUnitTracker> > *pUnits ) const
{
	pActivePlayer->GetSelectedUnits( pUnits );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CMission::GetGlobalVar( const string &szID, float fDefault ) const
{
	return pGlobalGame->GetGlobalVar( szID, fDefault );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SetGlobalVar( const string &szID, float fValue )
{
	pGlobalGame->SetGlobalVar( szID, fValue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CMission::CountSelected()
{
	return pActivePlayer->CountSelected();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::Select( NWorld::CUnit *pUnit, bool bAdditive )
{
	pActivePlayer->Select( pUnit, bAdditive );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SelectNext()
{
	pActivePlayer->SelectNext();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SelectPrev()
{
	pActivePlayer->SelectPrev();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::IsUpdated() const
{
	return bUpdated;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IState* CMission::GetState() const
{
	return pState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::CommandState( IState *pNewState )
{
	CObj<IState> pHold( pNewState );

	if ( IsValid( pState ) && ( pState->GetType() == IState::FORCED ) )
		return false;

	if ( !pNewState->Initialize( this ) )
		return false;

	if ( IsValid( pState ) )
		pState->Terminate();

	pState = pNewState;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::ResetState()
{
	pState->Terminate();
	pState = 0;
	CommandState( new CStateEmpty );
	UpdateState();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SetUpdatedStates( const vector<CObj<IState> > &_updatedStatesSet )
{
	updatedStatesSet = _updatedStatesSet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CMission::GetStateTarget() const
{
	if ( IsValid( pStateTarget ) )
		return pStateTarget;

	return pTraceObject;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SetStateTarget( CObjectBase* pObject )
{
	pStateTarget = pObject;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CMission::GetUnitsState()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
		return N_UNITSTATE_DEFAULT;

	switch ( unitsSet.front()->GetUnit()->GetState() )
	{
	case NWorld::CUnit::ST_NORMAL_KNIFE:
	case NWorld::CUnit::ST_NORMAL_RIFLE:
	case NWorld::CUnit::ST_NORMAL_MELEE:
	case NWorld::CUnit::ST_NORMAL_PISTOL:
	case NWorld::CUnit::ST_NORMAL_GRENADE:
	case NWorld::CUnit::ST_NORMAL_DEFAULT:
	case NWorld::CUnit::ST_NORMAL_SUB_MACHINE_GUN:
	case NWorld::CUnit::ST_NORMAL_HAND_MACHINE_GUN:
	case NWorld::CUnit::ST_NORMAL_RLAUNCHER:
	case NWorld::CUnit::ST_NORMAL_MEDKIT:
	case NWorld::CUnit::ST_HEALER:
	case NWorld::CUnit::ST_CARRY_CORPSE:
		return N_UNITSTATE_DEFAULT;
	case NWorld::CUnit::ST_MACHINE_GUN:
		return N_UNITSTATE_CANNON;
	case NWorld::CUnit::ST_SNIPE:
		return N_UNITSTATE_SNIPE;
	default:
		ASSERT( 0 );
	}

	return N_UNITSTATE_DEFAULT;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CUnit::EState CMission::GetUnitsWorldState()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
		return NWorld::CUnit::ST_NORMAL_DEFAULT;

	return unitsSet.front()->GetUnit()->GetState();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::GetActionInfo( EUnitAction eAction, SActionInfo *pInfo )
{
	*pInfo = actionsInfoSet[eAction];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::CanDoCommand( NWorld::CCmd *pCmd, bool bNoTarget, SActionInfo *pInfo )
{
	pInfo->eResult = CanDoCommand( pCmd, bNoTarget, &pInfo->nActionAP );

	pInfo->bOk = false;
	pInfo->bEnoughAP = true;
	pInfo->bAvailable = false;
	switch( pInfo->eResult )
	{
	case NWorld::UCR_UNAVAILABLE:
	case NWorld::UCR_INVALID_COMMAND:
		pInfo->bOk = false;
		pInfo->bAvailable = false;
		break;
	case NWorld::UCR_GENERAL_FAILURE:
		pInfo->bOk = false;
		pInfo->bAvailable = true;
		break;
	case NWorld::UCR_OK:
		pInfo->bOk = true;
		pInfo->bEnoughAP = true;
		pInfo->bAvailable = true;
		break;
	case NWorld::UCR_NO_TARGET:
		pInfo->bOk = bNoTarget;
		pInfo->bEnoughAP = true;
		pInfo->bAvailable = true;
		break;
	case NWorld::UCR_NOT_ENOUGH_AP:
		pInfo->bOk = true;
		pInfo->bEnoughAP = false;
		pInfo->bAvailable = true;
		break;
	case NWorld::UCR_PATH_NOT_FOUND:
	case NWorld::UCR_NEED_RELOAD:
	case NWorld::UCR_NO_EQUIPMENT:
	case NWorld::UCR_WEAPON_JAMMED:
	case NWorld::UCR_CRITICALS_BAN:
	case NWorld::UCR_TARGET_OUT_OF_RANGE:
		pInfo->bOk = false;
		pInfo->bEnoughAP = true;
		pInfo->bAvailable = true;
		break;
	default:
		ASSERT( 0 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::EUnitCommandResult CMission::CanDoCommand( NWorld::CCmd *pCmd, bool bNoTarget, int *pnAP )
{
	CObj<NWorld::CCmd> pHolder( pCmd );
	vector< CPtr<IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() == 0 )
		return NWorld::UCR_GENERAL_FAILURE;

	NWorld::EUnitCommandResult eTotalRes = NWorld::UCR_OK;
	for ( vector< CPtr<IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
	{
		CPtr<NWorld::CUnit> pUnit = (*iTemp)->GetUnit();

		if ( bNoTarget )
		{
			if ( CDynamicCast<NWorld::CCmdPath> pMove( pCmd ) )
			{
				if ( pMove->eParams != NAI::PF_USE_DIR )
					pMove->ptDst = (*iTemp)->GetTargetPosition();
				else
					pMove->ptDst = pUnit->GetPosition().pos;
			}
		}

		int nTemp;
		NWorld::EUnitCommandResult eRes = pUnit->CanDo( pCmd, &nTemp, pnAP );
		if ( eRes == NWorld::UCR_OK )
			continue;

		return eRes;
	}

	if ( pnAP && unitsSet.size() != 1 )
		*pnAP = -1;

	return eTotalRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::UpdateActionsInfo()
{
	actionsInfoSet[UA_DEFAULT].eResult = NWorld::UCR_OK;
	actionsInfoSet[UA_DEFAULT].nActionAP = 0;
	actionsInfoSet[UA_DEFAULT].bOk = true;
	actionsInfoSet[UA_DEFAULT].bEnoughAP = true;
	actionsInfoSet[UA_DEFAULT].bAvailable = true;
	
	actionsInfoSet[UA_STOP].eResult = NWorld::UCR_OK;
	actionsInfoSet[UA_STOP].nActionAP = 0;
	actionsInfoSet[UA_STOP].bOk = true;
	actionsInfoSet[UA_STOP].bEnoughAP = true;
	actionsInfoSet[UA_STOP].bAvailable = true;
	
	CanDoCommand( new NWorld::CCmdContinue(), true, &actionsInfoSet[UA_CONTINUE] );
	CanDoCommand( new NWorld::CCmdReload(), true, &actionsInfoSet[UA_WEAPONRELOAD] );

	CanDoCommand( new NWorld::CCmdPath( NAI::SPosition(), NAI::PF_DEFAULT ), true, &actionsInfoSet[UA_MOVE] );
	CanDoCommand( new NWorld::CCmdPath( NAI::SPosition(), NAI::PF_USE_DIR ), true, &actionsInfoSet[UA_LOOK] );

	actionsInfoSet[UA_USE].eResult = NWorld::UCR_OK;
	actionsInfoSet[UA_USE].nActionAP = 0;
	actionsInfoSet[UA_USE].bOk = true;
	actionsInfoSet[UA_USE].bEnoughAP = true;
	actionsInfoSet[UA_USE].bAvailable = true;

	CanDoCommand( new NWorld::CCmdShootObject( 0, 0 ), true, &actionsInfoSet[UA_ATTACK] );

	CanDoCommand( new NWorld::CCmdHeal( 0 ), true, &actionsInfoSet[UA_HEAL] );
	CanDoCommand( new NWorld::CCmdDropCorpse(), true, &actionsInfoSet[UA_DROPCORPSE] );

	CanDoCommand( new NWorld::CCmdWishPose( NAI::RUN ), true, &actionsInfoSet[UA_POSERUN] );
	CanDoCommand( new NWorld::CCmdWishPose( NAI::WALK ), true, &actionsInfoSet[UA_POSEWALK] );
	CanDoCommand( new NWorld::CCmdWishPose( NAI::CROUCH ), true, &actionsInfoSet[UA_POSECROUCH] );
	CanDoCommand( new NWorld::CCmdWishPose( NAI::CRAWL ), true, &actionsInfoSet[UA_POSECRAWL] );

	CanDoCommand( new NWorld::CCmdSnipeAttack(), true, &actionsInfoSet[UA_SNIPE_ATTACK] );
	CanDoCommand( new NWorld::CCmdCollectSnipeAP( NWorld::CSAP_1AP ), true, &actionsInfoSet[UA_COLLECTAP_1AP] );
	CanDoCommand( new NWorld::CCmdCollectSnipeAP( NWorld::CSAP_10AP ), true, &actionsInfoSet[UA_COLLECTAP_10AP] );
	CanDoCommand( new NWorld::CCmdCollectSnipeAP( NWorld::CSAP_MAX ), true, &actionsInfoSet[UA_COLLECTAP_MAX] );
	CanDoCommand( new NWorld::CCmdCollectSnipeAP( NWorld::CSAP_ALL ), true, &actionsInfoSet[UA_COLLECTAP_ALL] );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ICamera* CMission::GetCamera() const
{
	return pCamera;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CMission::GetCameraFOV() const
{
	return fFOV;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::GetCameraParams( ECameraType *pType, float *pFOV, ICamera::SCameraLimits *pLimits )
{
	*pFOV = fFOV;
	*pType = eCameraType;
	*pLimits = cameraLimits;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SetCameraParams( ECameraType eType, float _fFOV, const ICamera::SCameraLimits &limits )
{
	CPtr<ICamera> pNewCamera;

	fFOV = _fFOV;
	eCameraType = eType;
	cameraLimits = limits;
	cameraLimits.sZoneLimit = GetWorld()->GetMapSafeZone();

	pNewCamera = CreateCamera( eType );
	if ( !IsValid( pNewCamera ) )
	{
		ASSERT( 0 );
		return;
	}
	ICamera::SCameraPos sPos;
	if ( pCamera )
	{
		pCamera->GetPlacement( &sPos );
		pNewCamera->SetPlacement( sPos );
	}
	pNewCamera->SetLimits( cameraLimits );
	pCamera = pNewCamera;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::FreezeCamera( bool bState )
{
	bFreezeCamera = bState;
	if ( bState )
		pCamera->GetPlacement( &sFreezePose );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::FocusCameraOnUnit( NWorld::CUnit *pUnit )
{
	ICamera::SCameraPos sPos;
	pCamera->GetPlacement( &sPos );
	sPos.ptAnchor = pUnit->GetPosition().pos.GetCP();
	pCamera->SetPlacement( sPos );
	GetScene()->SetCutFloor( pUnit->GetPosition().pos.GetFloor() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const CVec3& CMission::GetCameraCP() const
{
	return vCameraCP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SHMatrix& CMission::GetCameraPos() const
{
	return sCameraPos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const CTransformStack& CMission::GetCameraTransform() const
{
	return sTransform;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::IVisObj* CMission::GetTraceObject() const
{
	return pTraceObject;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::GetTracePosition( NAI::SPosition *pPos ) const
{
	*pPos = sTraceTile;
	return bTraceOk;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRay CMission::GetTraceRay() const
{
	CRay res;
	CTransformStack sTS = GetCameraTransform();
	MakeProjectiveRay( &res.ptDir, &res.ptOrigin, &sTS, GetScene()->GetScreenRect(), GetCursor()->GetPos() );
	return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::ICursor* CMission::GetCursor() const
{
	return pCursor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::CInterface* CMission::GetInterface() const
{
	return pInterface;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::CMissionUI* CMission::GetMissionUI() const
{
	return pMissionUI;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::IsInterfaceHidden() const
{
	return bHideInterface;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CGlobalGame* CMission::GetRPGGame() const
{
	return pGlobalGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::IWorld* CMission::GetWorld() const
{
	return pWorld;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NGScene::IGameView* CMission::GetScene() const
{
	return pScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NSound::ISoundScene* CMission::GetSoundScene() const
{
	return pSoundScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRender::IRenderGame* CMission::GetRenderGame() const
{
	return pRender;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::Step()
{
	if ( CanRender() )
	{
		if ( ( playersSet.size() > 1 ) && !IsRealTime() && ( pActivePlayer->GetPlayer() != pWorld->GetCurrentPlayer() ) )
		{
			ICamera::SCameraPos sPosition;
			pCamera->GetPlacement( &sPosition );
			pActivePlayer->SetCamera( sPosition );
			pActivePlayer->Deactivate();

			for ( vector< CObj<IPlayerTracker> >::iterator iPlayer = playersSet.begin(); iPlayer != playersSet.end(); iPlayer++ )
			{
				if ( (*iPlayer)->GetPlayer() == pWorld->GetCurrentPlayer() )
				{
					pActivePlayer = (*iPlayer);
					pCamera->SetPlacement( pActivePlayer->GetCamera() );
				}
			}
			pActivePlayer->Activate();
		}

		UpdateLocators();

		if ( bFreezeCamera )
			pCamera->SetPlacement( sFreezePose );

		pCamera->Update( GetTime() );

		vCameraCP = pCamera->GetCP();
		sCameraPos = pCamera->GetPos();
		pCamera->GetTransform( &sTransform, GetScene()->GetScreenRect() );
		if ( vPrevCameraPosition != vCameraCP )
		{
			vPrevCameraPosition = vCameraCP;
			nFramesSameCameraPosition = 0;
		}
		else
			++nFramesSameCameraPosition;

		for ( vector< CObj<IPlayerTracker> >::iterator iPlayer = playersSet.begin(); iPlayer != playersSet.end(); iPlayer++ )
			(*iPlayer)->Update( (*iPlayer)->GetPlayer() == pWorld->GetCurrentPlayer() );

		pInterface->UpdateCursor();

		bTraceOk = false;
		if ( IsRealTime() || !IsActionExecuted() )
		{
			TraceCursor();

			bUpdated = TrackChanges();
			if ( bUpdated )
				UpdateActionsInfo();
		}
		else
		{
			for( int nTemp = 0; nTemp < actionsInfoSet.size(); nTemp++ )
				actionsInfoSet[nTemp].bOk = false;
		}

		if ( ( pState->GetType() != IState::TEMPORARY ) && ( IsUpdated() || ( pState->GetType() == IState::INSTANT ) ) )
			UpdateState();

		pState->Step();

		UpdateSound();

		pStateTarget = 0;
		pInterface->Step( GetTime() );

		bHasCommands |= pActivePlayer->GetCommander()->HasCommands();

		int nFlags = N_RENDERMODE_3D;
		if ( bShowAllCheat )
			nFlags |= N_RENDERMODE_SHOWALL;
		if ( !bHideInterface )
			nFlags |= N_RENDERMODE_2D;
		RenderFrame( nFlags, GetTime(), GetCamera() );
	}
	else
		pRender->ResetTiming();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::OnGetFocus()
{
	pRender->ResetTiming();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::ProcessEvent( const NInput::SEvent &sEvent )
{
	NInput::SetSection( "mission" );

	pCursor->ProcessEvent( sEvent );

	if ( bindExit.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( 0 ); 
		return true;
	}

	if ( bindHideInterface.ProcessEvent( sEvent ) )
		bHideInterface = !bHideInterface;
	else if ( bindSpecialHideInterface.ProcessEvent( sEvent ) )
		bSpecialHideInterface = !bSpecialHideInterface;
	else if ( bindPause.ProcessEvent( sEvent ) )
	{
		bPause = !bPause;
		return true;
	}

	if ( bindMainMenu.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICInGameMenu( GetRPGGame(), TACTICVIEW ) );
		return true;
	}
	else if ( bindCluesMenu.ProcessEvent( sEvent ) )
	{
//		NMainLoop::Command( new CICClues( GetRPGGame() ) );
		NMainLoop::Command( new CICTeamMngMenu( pActivePlayer->GetGlobalPlayer() ) );
		return true;
	}

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

/// SYSTEM( No unit or player allowed! )
	if ( bindShadows.ProcessEvent( sEvent ) )
		pScene->SetNextShadowsMode();
	else if ( bindHSR.ProcessEvent( sEvent ) )
		pScene->SetNextHSRMode();
	else if ( bindFog.ProcessEvent( sEvent ) )
		pScene->SetNextFogMode();
	else if ( bindAddFloor.ProcessEvent( sEvent ) )
		pScene->SetCutFloor( pScene->GetCutFloor() + 1 );
	else if ( bindSubFloor.ProcessEvent( sEvent ) )
		pScene->SetCutFloor( pScene->GetCutFloor() - 1 );
	else if ( bindShowParticles.ProcessEvent( sEvent ) )
		pScene->SetParticleShow( !pScene->GetParticleShow() );
	else if ( bindShowAI.ProcessEvent( sEvent ) )
	{
		ICamera::SCameraPos cameraPos;
		pCamera->GetPlacement( &cameraPos );
		NMainLoop::Command( new CICAIView( pWorld->GetAIMap(), pWorld->GetPathNetwork(), cameraPos ) );
		return true;
	}
	else if ( bindShowRAD.ProcessEvent( sEvent ) )
	{
		ICamera::SCameraPos cameraPos;
		pCamera->GetPlacement( &cameraPos );
		NMainLoop::Command( new CICRadTest( pScene, cameraPos ) );
		return true;
	}
	else if ( bindShowVisibility.ProcessEvent( sEvent ) )
	{
		if ( !pVisibleTracker )
		{
			pVisibleTracker = new CVisibleTracker;
			pVisibleTracker->Update( this );
		}
		else
			pVisibleTracker = 0;
	}
	else if ( bindShowSchema.ProcessEvent( sEvent ) )
	{
		if ( !buildingSchemas.empty() )
		{
			buildingSchemas.clear();
			return true;
		}

		CViewBuildingScheme b( pWorld->GetActive(), pScene, &buildingSchemas );
		b.Sync();
	}
	else if ( bindCheckBuildings.ProcessEvent( sEvent ) )
	{
		CUpdateBuildingStability b( pWorld->GetActive() );
		b.Sync();
	}
	else if ( bindTestIsect.ProcessEvent( sEvent ) )
		TestIntersection();
	else if ( bindClickOfDeath.ProcessEvent( sEvent ) )
		ClickOfDeath();
	else if ( bindExplosion.ProcessEvent( sEvent ) )
		Explosion();
	else if ( bindRPGStats.ProcessEvent( sEvent ) )
	{
		vector< CPtr<IPlayerTracker> > playersSet;
		GetPlayers( &playersSet );
		for ( int i = 0; i < playersSet.size(); ++i )
		{
			if ( !IsValid( playersSet[i] ) )
				continue;
			csRPG << "<color=cyan>" << "PLAYER " << i << ":\n";
			vector< CPtr<IUnitTracker> > units;
			playersSet[i]->GetUnits( &units );
			for ( int j = 0; j < units.size(); ++j )
			{
				if ( !IsValid( units[j] ) || !IsValid( units[j]->GetUnit() ) )
					continue;
				csRPG << "<color=green>" << "Unit " << j << ":";
				units[j]->GetUnit()->GetRPG()->DumpStats();
			}
		}
		csRPG << "\n";
		return true;
	}
	else if ( bindExplode.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetSelectedUnits( &unitsSet );
		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
			Command( (*iTemp)->GetUnit(), new NWorld::CCmdExplode );
		return true;
	}

	if ( bindSwitchLighting.ProcessEvent( sEvent ) )
		SetLightMode( nLightMode + 1 );

	if ( IsRealTime()
		|| GetWorld()->GetCurrentPlayer() == GetActivePlayer()->GetPlayer()
		|| !GetWorld()->CanSeeAction( GetActivePlayer()->GetPlayer() ) )
		pCamera->ProcessEvent( sEvent );

//////////////////////////////////////////
/// Unit control
	if ( !IsRealTime() && ( pWorld->GetCurrentPlayer() != pActivePlayer->GetPlayer() ) )
		return false;

	if ( pState->ProcessEvent( sEvent ) )
		return true;

	if ( pMissionUI->ProcessEvent( sEvent ) )
		return true;

	if ( bindCancel.ProcessEvent( sEvent ) )
	{
		if ( pState->GetType() == IState::FORCED )
			ResetState();
		else if ( IsActionExecuted() )
			StopAction();
		else
		{
			vector<CPtr<IUnitTracker> > unitsSet;
			GetSelectedUnits( &unitsSet );

			for ( vector<CPtr<IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
			{
				if ( IsValid( (*iTemp)->GetUnit()->GetRPG()->GetCannonItemInfo() ) )
					Command( (*iTemp)->GetUnit(), new NWorld::CCmdExitCannon );
				else
					Command( new NWorld::CCmdCancel( (*iTemp)->GetUnit() ) );
			}
		}

		return true;
	}

	if ( !IsRealTime() && IsActionExecuted() )
		return false;

	if ( IsRealTime() )
	{
		if ( bindStartOfTurn.ProcessEvent( sEvent ) )
		{
			vector< CPtr<IUnitTracker> > unitsSet;
			GetUnits( &unitsSet );
			if ( unitsSet.size() > 0 )
				Command( unitsSet.front()->GetUnit(), new NWorld::CCmdStartCombat );
			return true;
		}
	}
	else 
	{
		if ( bindEndOfTurn.ProcessEvent( sEvent ) )
		{
			Command( new NWorld::CCmdEndOfTurn );
			return true;
		}
	}

	if ( bindContinue.ProcessEvent( sEvent ) )
	{
		SActionInfo sInfo;
		GetActionInfo( UA_CONTINUE, &sInfo );
		if ( sInfo.bAvailable && sInfo.bOk )
		{
			vector<CPtr<IUnitTracker> > unitsSet;
			GetSelectedUnits( &unitsSet );

			for ( vector<CPtr<IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
				Command( (*iTemp)->GetUnit(), new NWorld::CCmdContinue() );

			return true;
		}
	}

	/// check for selection
	int nSelection = -1;
	if ( bindHero1.ProcessEvent( sEvent ) )
		nSelection = 0;
	else if ( bindHero2.ProcessEvent( sEvent ) )
		nSelection = 1;
	else if ( bindHero3.ProcessEvent( sEvent ) )
		nSelection = 2;
	else if ( bindHero4.ProcessEvent( sEvent ) )
		nSelection = 3;
	else if ( bindHero5.ProcessEvent( sEvent ) )
		nSelection = 4;
	else if ( bindHero6.ProcessEvent( sEvent ) )
		nSelection = 5;

	if ( nSelection != -1 )
	{
		vector< CPtr<IUnitTracker> > unitsSet;
		GetUnits( &unitsSet );
		if ( nSelection < unitsSet.size() )
		{
			if ( !unitsSet[nSelection]->IsSelected() || ( CountSelected() != 1 ) )
				Select( unitsSet[nSelection]->GetUnit(), false );
			else
				FocusCameraOnUnit( unitsSet[nSelection]->GetUnit() );
		}
	}
	else if ( bindSelPrev.ProcessEvent( sEvent ) )
		SelectPrev();
	else if ( bindSelNext.ProcessEvent( sEvent ) )
		SelectNext();

	if ( bindPrevSlot.ProcessEvent( sEvent ) )
		SelectSlot( -1 );
	if ( bindNextSlot.ProcessEvent( sEvent ) )
		SelectSlot( 1 );

	if ( bindWeaponReload.ProcessEvent( sEvent ) )
		WeaponReload();
	if ( bindItemUnload.ProcessEvent( sEvent ) )
	{
		ResetState();
		CommandState( new CStateUnloadItem );
	}

	if ( bindSnapShot.ProcessEvent( sEvent ) )
		NewWeaponMode( NDb::SM_Snap );
	else if ( bindAimedShot.ProcessEvent( sEvent ) )
		NewWeaponMode( NDb::SM_Aimed );
	else if ( bindCarefulShot.ProcessEvent( sEvent ) )
		NewWeaponMode( NDb::SM_Careful );
	else if ( bindShortBurst.ProcessEvent( sEvent ) )
		NewWeaponMode( NDb::SM_ShortBurst );
	else if ( bindLongBurst.ProcessEvent( sEvent ) )
		NewWeaponMode( NDb::SM_LongBurst );
	else if ( bindSnipeShot.ProcessEvent( sEvent ) )
		NewWeaponMode( NDb::SM_Snipe );

	if ( bindWeaponPrevMode.ProcessEvent( sEvent ) )
		SelectWeaponMode( -1 );
	else if ( bindWeaponNextMode.ProcessEvent( sEvent ) )
		SelectWeaponMode( 1 );

	/// Forced actions
	if ( bindMove.ProcessEvent( sEvent ) )
		CommandState( new CStateMove( true ) );
	if ( bindRotate.ProcessEvent( sEvent ) )
		CommandState( new CStateRotate() );
	if ( bindAttack.ProcessEvent( sEvent ) )
		CommandState( new CStateAttack( true ) );
	if ( bindFirstAid.ProcessEvent( sEvent ) )
		CommandState( new CStateFirstAid() );
	if ( bindDropCorpse.ProcessEvent( sEvent ) ) //// CRAP
		CommandState( new CStateDropCorpse() );

	if ( bindNormalPose.ProcessEvent( sEvent ) )
		CommandState( new CStatePose( NAI::WALK ) );
	else if ( bindCrawlPose.ProcessEvent( sEvent ) )
		CommandState( new CStatePose( NAI::CRAWL ) );
	else if ( bindCrouchPose.ProcessEvent( sEvent ) )
		CommandState( new CStatePose( NAI::CROUCH ) );
	else if ( bindRunPose.ProcessEvent( sEvent ) )
		CommandState( new CStatePose( NAI::RUN ) );
	else if ( bindStrafe.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetSelectedUnits( &unitsSet );
		if ( unitsSet.size() == 1 )
			unitsSet.front()->SetStrafeMove( !unitsSet.front()->IsStrafeMove() );
	}

	if ( bindSnipeAttack.ProcessEvent( sEvent ) )
	{
		SActionInfo sAction;
		GetActionInfo( UA_SNIPE_ATTACK, &sAction );
		if ( sAction.eResult == NWorld::UCR_OK )
		{
			vector< CPtr<NGame::IUnitTracker> > unitsSet;
			GetSelectedUnits( &unitsSet );
			if ( unitsSet.size() == 1 )
				Command( unitsSet.front()->GetUnit(), new NWorld::CCmdSnipeAttack() );
		}
		else
		{
			csGame << GetErrorString( sAction.eResult ) << endl;
		}
	}
	else if ( bindCollect1AP.ProcessEvent( sEvent ) )
		UnitCollectAP( NWorld::CSAP_1AP );
	else if ( bindCollect10AP.ProcessEvent( sEvent ) )
		UnitCollectAP( NWorld::CSAP_10AP );
	else if ( bindCollectMaxAP.ProcessEvent( sEvent ) )
		UnitCollectAP( NWorld::CSAP_MAX );
	else if ( bindCollectAllAP.ProcessEvent( sEvent ) )
		UnitCollectAP( NWorld::CSAP_ALL );

	if ( bindNextEnemy.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetSelectedUnits( &unitsSet );
		if ( unitsSet.size() == 1 )
		{
			CPtr<NWorld::CUnit> pEnemy = unitsSet[0]->GetNextEnemy();
			if ( IsValid( pEnemy ) )
				FocusCameraOnUnit( pEnemy );
		}
	}
	else if ( bindCheatTeleport.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetSelectedUnits( &unitsSet );
		if ( unitsSet.size() == 1 )
		{
			NAI::SUnitPosition sPosition = unitsSet[0]->GetUnit()->GetPosition();
			if( GetTracePosition( &sPosition.pos ) )
				Command( unitsSet[0]->GetUnit(), new NWorld::CCmdTeleport( sPosition ) );
		}

		return true;
	}
	else if ( bindFocusUnit.ProcessEvent( sEvent ) )
	{
		vector< CPtr<IUnitTracker> > unitsSet;
		GetSelectedUnits( &unitsSet );
		if ( unitsSet.size() == 1 )
		{
			for ( vector< CPtr<IUnitTracker> >::iterator iUnit = unitsSet.begin(); iUnit != unitsSet.end(); iUnit++ )
				FocusCameraOnUnit( (*iUnit)->GetUnit() );
		}
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::RenderFrame( int nMode, const STime &sTime, ICamera *pCamera, bool bShowUnits )
{
	NWorld::IPlayer *pViewFrom  = (nMode & N_RENDERMODE_SHOWALL) ? 0 : pActivePlayer->GetPlayer();
	pRender->UpdateViewWorld( !bPause, sTime, pViewFrom, bShowUnits );

	if ( nMode & N_RENDERMODE_3D )
	{
		pRenderSound->Update( pCamera );

		CTransformStack ts;
		pCamera->GetTransform( &ts, pScene->GetScreenRect() );

		const CTRect<float> &rScreen = pCamera->GetScreenRect();
		if ( ( rScreen.Width() != 0 ) && ( rScreen.Height() != 0 ) )
		{
			NGScene::IGameView::SDrawInfo drawInfo;
			drawInfo.pTS = &ts;
			drawInfo.vOrigin = CVec2( rScreen.x1, rScreen.y1 );
			drawInfo.vSize = CVec2( rScreen.x2 - rScreen.x1, rScreen.y2 - rScreen.y1 );
			drawInfo.bUseDefaultClearColor = true;
			drawInfo.vClearColor = CVec3(0.25f,0.25f,0.25f); // not used due to using default clear color
			pScene->Draw( drawInfo );
		}
	}
	if ( !bHideInterface && !bSpecialHideInterface && ( nMode & N_RENDERMODE_2D ) )
		pInterface->Draw( sTime );


	float fFrameTime = NGScene::GetFrameTime();
	static float fMinFrameTime = 1, fMaxFrameTime = 1e-4f, fElapsed = 0;
	static int nFrames;
	fMinFrameTime = Min( fMinFrameTime, fFrameTime );
	fMaxFrameTime = Max( fMaxFrameTime, fFrameTime );
	fElapsed += fFrameTime;
	++nFrames;
	if ( fElapsed > 3 )
	{
		NStr::DebugTrace( "min %f max %f average %f\n", 1 / fMaxFrameTime, 1 / fMinFrameTime, nFrames / fElapsed );
		fMinFrameTime = 1;
		fMaxFrameTime = 1e-4f;
		fElapsed = 0;
		nFrames = 0;
	}

	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMission::TrackChanges()
{
	bool bUpdated = false;

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );
	if ( unitsSet != selectedUnits )
	{
		bUpdated = true;
		selectedUnits = unitsSet;
	}

	bool bNewRealTime = IsRealTime();
	if ( bNewRealTime != bRealTime )
	{
		bUpdated = true;
		bRealTime = bNewRealTime;
	}

	NWorld::IPlayer::SItemInfo sItem;
	CPtr<CObjectBase> pNewPlayerInHand;
	if ( pActivePlayer->GetPlayer()->GetInHandItem( &sItem ) )
		pNewPlayerInHand = sItem.pItem;
	if ( pNewPlayerInHand != pPlayerInHand )
	{
		bUpdated = true;
		pPlayerInHand = pNewPlayerInHand;
	}

	bool bNewHasCommands = pActivePlayer->GetCommander()->HasCommands();
	if ( bNewHasCommands != bHasCommands )
	{
		bUpdated = true;
		bHasCommands = bNewHasCommands;
	}

	bool bNewActionExecuted = IsActionExecuted();
	if ( bNewActionExecuted != bActionExecuted )
	{
		bUpdated = true;
		bActionExecuted = bNewActionExecuted;
	}

	CPtr<CObjectBase> pNewTrackTarget = GetStateTarget();
	if ( pNewTrackTarget != pTrackTarget )
	{
		bUpdated = true;
		pTrackTarget = pNewTrackTarget;
	}

	return bUpdated;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::UpdateState()
{
	if ( ( pState->GetType() == IState::FORCED ) && pState->Initialize( this ) )
		return;
	if ( ( pState->GetType() == IState::TEMPORARY ) && pState->Initialize( this ) )
		return;

	pState->Terminate();
	pState = 0;
	for ( int nTemp = 0; nTemp < updatedStatesSet.size(); nTemp++ )
	{
		if ( CommandState( updatedStatesSet[nTemp] ) )
			return;
	}

	ASSERT( 0 );
	CommandState( new CStateEmpty );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::UpdateSound()
{
	int nEWatch = pWorld->GetEnemyWatchers( pActivePlayer->GetPlayer() );
	list< CPtr<NWorld::CUnit> > visible;
	int nVisibleEnemy = 0;
	pActivePlayer->GetPlayer()->GetVisible( &visible );
	for ( list< CPtr<NWorld::CUnit> >::const_iterator it = visible.begin(); it != visible.end(); ++it )
		if ( !(*it)->IsDead() && !(*it)->IsUnconscious() && (*it)->GetPlayer() != pActivePlayer->GetPlayer() )
			++nVisibleEnemy;
	if ( nEWatch > 0 || nVisibleEnemy > 0 )
		pSoundScene->SetMusic( NDb::GetMusic( 3 ) );
	else
		pSoundScene->FadeOutMusic();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::WeaponReload()
{
	SActionInfo sInfo;
	GetActionInfo( UA_WEAPONRELOAD, &sInfo );
	if ( sInfo.eResult != NWorld::UCR_OK )
	{
		csGame << GetErrorString( sInfo.eResult ) << endl;
		return;
	}

	vector< CPtr<IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );
	for ( int nTemp = 0; nTemp < unitsSet.size(); nTemp++ )
		Command( unitsSet[nTemp]->GetUnit(), new NWorld::CCmdReload );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SelectSlot( int nInc )
{
	vector< CPtr<IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
		return;

	int nSlot = unitsSet[0]->GetUnit()->GetRPG()->GetInventoryInfo()->GetActiveSlot();
	nSlot += nInc;
	if ( nSlot >= NDb::N_SLOTS )
		nSlot = 0;
	if ( nSlot < 0 )
		nSlot = NDb::N_SLOTS - 1;

	Command( unitsSet[0]->GetUnit(), new NWorld::CCmdSetActiveItem( NDb::ESlot( nSlot ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::NewWeaponMode( NDb::EShootMode eMode )
{
	vector< CPtr<IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );
	for ( vector< CPtr<IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
		Command( (*iTemp)->GetUnit(), new NWorld::CCmdShootMode( eMode ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SelectWeaponMode( int nInc )
{
	vector< CPtr<IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
		return;

	CPtr<NRPG::IInventoryItem> pItem = unitsSet[0]->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive();
	if ( IsValid( pItem ) )
	{
		if ( CDynamicCast<NRPG::IWeaponItemInfo> pWeapon( pItem ) )
		{
			int nMode = pWeapon->GetShootMode();
			for ( int nTemp = 0; nTemp < NDb::SM_MAXVALUE; nTemp++ )
			{
				nMode += nInc;
				if ( nMode < 0 )
					nMode = NDb::SM_MAXVALUE - 1;
				if ( nMode >= NDb::SM_MAXVALUE )
					nMode = 0;

				if ( pWeapon->GetDBWeapon()->shootModes[nMode] )
				{
					NewWeaponMode( NDb::EShootMode( nMode ) );
					break;
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::SetLightMode( int _nLightMode )
{
	CDBTable<NDb::CAmbientLight> *pTable = NDatabase::GetTable<NDb::CAmbientLight>();
  CDBIterator<NDb::CAmbientLight> it( *pTable );
	int nCount = _nLightMode;
  while ( it.MoveNext() )
  {
		if ( !it.Get()->bInGameUse )
			continue;
		if ( nCount == 0 )
		{
			nLightMode = _nLightMode;
			pLightSource = 0;
			csSystem << "Light with ID = " << it.Get()->GetRecordID() << " selected" << endl;
			GetScene()->SetAmbient( it.Get() );
			return;
		}
		nCount--;
	}
	if ( _nLightMode == 0 )
	{
		// no lighting in table, using default one
		nLightMode = 0;
		GetScene()->SetAmbient( 0 );
		pLightSource = GetScene()->AddDirectionalLight( CVec3(0.5f,0.4f,0.45f), CVec3( 0.6f,1.4f,-1), CVec3(5,5,0), CVec2( 150, 150 ), 20 );
		GetScene()->SetAmbient( CVec3( 0.20f, 0.20f, 0.20f ) );
	}
	else
		SetLightMode( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::TraceCursor()
{
	if ( nFramesSameCameraPosition < 4 )
		return;
	CTransformStack sTS;
	pCamera->GetTransform( &sTS, GetScene()->GetScreenRect() );
	MakeProjectiveRay( &rTraceRay.ptDir, &rTraceRay.ptOrigin, &sTS, GetScene()->GetScreenRect(), GetCursor()->GetPos() );
	bTraceOk = GetWorld()->TraceTile( rTraceRay, &sTraceTile, GetScene()->GetCutFloor() );

	list< CPtr<NWorld::CUnit> > visibleList;
	GetActivePlayer()->GetPlayer()->GetVisible( &visibleList );

	pTraceObject = 0;
	vector<NWorld::IVisObj*> objectsSet;
	GetWorld()->TraceObjects( rTraceRay, &objectsSet, GetScene()->GetCutFloor() );
	if ( !objectsSet.empty() )
	{
		for ( vector<NWorld::IVisObj*>::iterator iTemp = objectsSet.begin(); iTemp != objectsSet.end(); iTemp++ )
		{
			NWorld::CUnit *pTempUnit = dynamic_cast<NWorld::CUnit*>( *iTemp );
			if ( pTempUnit )
			{
				if ( find( visibleList.begin(), visibleList.end(), pTempUnit ) != visibleList.end() )
				{
					pTraceObject = *iTemp;
					break;
				}
			}

			CDynamicCast<NWorld::IObject> pTempObject( *iTemp );
			if ( pTempObject && pTempObject->IsTargetable() )
			{
				pTraceObject = *iTemp;
				break;
			}

			if ( CDynamicCast<NWorld::IItem> pTempItem( *iTemp ) )
			{
				pTraceObject = *iTemp;
				break;
			}
		}
	}

	if ( !IsValid( pTraceObject ) )
	{
		NAI::SUnitPosition sPosition;
		sPosition.pos = sTraceTile;
		sPosition.bRun = false;
		NWorld::CUnit *pTempUnit = GetWorld()->GetUnitInTile( sPosition );
		if ( find( visibleList.begin(), visibleList.end(), pTempUnit ) != visibleList.end() )
			pTraceObject = pTempUnit;
	}

	if ( pTraceObject != 0 )
	{
		NWorld::CUnit *pTempUnit = dynamic_cast<NWorld::CUnit*>( pTraceObject.GetPtr() );
		if ( pTempUnit )
		{
			bTraceOk = true;
			sTraceTile = pTempUnit->GetPosition().pos;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::UnitCollectAP( NWorld::ECollectSnipeAP eAP )
{
	EUnitAction eAction;
	switch( eAP )
	{
	case NWorld::CSAP_1AP:
		eAction = UA_COLLECTAP_1AP;
		break;
	case NWorld::CSAP_10AP:
		eAction = UA_COLLECTAP_10AP;
		break;
	case NWorld::CSAP_MAX:
		eAction = UA_COLLECTAP_MAX;
		break;
	case NWorld::CSAP_ALL:
		eAction = UA_COLLECTAP_ALL;
		break;
	default:
		ASSERT( 0 );
		return;
	}

	SActionInfo sAction;
	GetActionInfo( eAction, &sAction );
	if ( sAction.eResult != NWorld::UCR_OK )
	{
		csGame << GetErrorString( sAction.eResult ) << endl;
		return;
	}

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetSelectedUnits( &unitsSet );
	if ( unitsSet.size() == 1 )
		Command( unitsSet.front()->GetUnit(), new NWorld::CCmdCollectSnipeAP( eAP ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::UpdateLocators()
{
	ICamera::SCameraPos sPos;
	pCamera->GetPlacement( &sPos );

	if ( pActionLocator != 0 )
	{
		if ( pActionLocator->tTime != 0 )
		{
			if ( pActionLocator->tTime < pRender->GetTime()->GetValue() )
			{
				NStr::DebugTrace( "camera control: time hit\n" );
				sPos.ptAnchor = vPlayerCamera;
				pActionLocator = 0;
			}
			else
				return;
		}
	}

	if ( ( pLastActivePlayer != pWorld->GetCurrentPlayer() ) && IsValid( pActionLocator ) )
	{
		NStr::DebugTrace( "camera control: player change\n" );
		sPos.ptAnchor = vPlayerCamera;
		pActionLocator = 0;
	}
	pLastActivePlayer = pWorld->GetCurrentPlayer();

	if ( !IsRealTime() )
	{
		CPtr<NWorld::CActionLocator> pTempLocator;
		while ( pTempLocator = pWorld->GetActionEvent( pActivePlayer->GetPlayer() ) )
		{
			if ( pActionLocator != 0 )
			{
				if ( pTempLocator->eType > pActionLocator->eType )
					continue;
			}
			else
				vPlayerCamera = sPos.ptAnchor;

			NStr::DebugTrace( "camera control: locator\n" );
			switch( pTempLocator->eType )
			{
			case NWorld::CActionLocator::TYPE_INTERRUPT:
				pTempLocator->pPlayer = pActivePlayer->GetPlayer(); // CRAP
				if ( pTempLocator->pPlayer == pActivePlayer->GetPlayer() )
				{
					sPos.ptAnchor = vPlayerCamera;
					pActionLocator = 0;
				}
				break;
			case NWorld::CActionLocator::TYPE_TURN:
				if ( pTempLocator->pPlayer == pActivePlayer->GetPlayer() )
				{
					sPos.ptAnchor = vPlayerCamera;
					pActionLocator = 0;
				}
				break;
			case NWorld::CActionLocator::TYPE_DIE:
				pActionLocator = 0;
			case NWorld::CActionLocator::TYPE_UNIT:
				if ( pWorld->GetCurrentPlayer() != pActivePlayer->GetPlayer() )
					pActionLocator = pTempLocator;
				break;
			case NWorld::CActionLocator::TYPE_OBJECT:
			case NWorld::CActionLocator::TYPE_WORLD:
				pActionLocator = pTempLocator;
				break;
			}
		}
	}
	else if ( IsValid( pActionLocator ) )
	{
		NStr::DebugTrace( "camera control: realtime\n" );
		sPos.ptAnchor = vPlayerCamera;
		pActionLocator = 0;
	}

	if ( IsValid( pActionLocator ) )
		sPos.ptAnchor = pActionLocator->vPosition;

	pCamera->SetPlacement( sPos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::TestIntersection()
{
	CRay r;
	// CRAP - have to make special function in pGame for this
	CTransformStack sTS = GetCameraTransform();
	MakeProjectiveRay( &r.ptDir, &r.ptOrigin, &sTS, pScene->GetScreenRect(), pCursor->GetPos() );
	r.ptDir *= 100;
	float fT;
	CVec3 vNormal, vColor;
	if ( pScene->TraceScene( r, &fT, &vNormal, &vColor ) )
	{
		CVec3 vPoint = r.Get( fT );
		CPtr<CMemObject> pModel = new CMemObject;
		pModel->CreateSphere( vPoint, 0.06f );
		pIntersectHolder = pScene->CreateMesh( pModel, CVec4(vColor,1), 0 );
		vector<CVec3> points;
		points.push_back( vPoint );
		points.push_back( vPoint + vNormal * 0.2f );
		pIntersectLineHolder = pScene->CreatePolyline( points, CVec3(1,1,1) );
	}
	else
	{
		pIntersectHolder = 0;
		pIntersectLineHolder = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::ClickOfDeath()
{
	pWorld->ClickOfDeath( GetTraceRay(), pScene->GetCutFloor() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMission::Explosion()
{
	pWorld->MakeExplosion( GetTraceRay(), pScene->GetCutFloor() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVisibleTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVisibleTracker::Update( IMission* pMission )
{
	CVec3 vColors[4] = { CVec3( 0, 0, 0 ), CVec3( 1, 0, 0 ), CVec3( 0, 0, 1 ), CVec3( 0, 1, 0 ) };

	bUpdated = false;

	vector< CPtr<IUnitTracker> > unitsSet;
	pMission->GetSelectedUnits( &unitsSet );

	if ( unitsSet != selectedUnits )
	{
		selectedUnits = unitsSet;
		bUpdated = true;
	}

	if ( !bUpdated )
		return;

	nodesSet.clear();
	for ( int nTemp = 0; nTemp < selectedUnits.size(); nTemp++ )
	{
		vector<NRPG::SVisibilitySpot> spotsSet;
		pMission->GetWorld()->GetGame()->GetVisibilityArea( &spotsSet, unitsSet[nTemp]->GetUnit() );

		for ( int nSpot = 0; nSpot < spotsSet.size(); nSpot++ )
		{
			CPtr<CMemObject> pModelBuilder = new CMemObject;
			pModelBuilder->CreateSphere( spotsSet[nSpot].ptPos, 0.06f, 0 );
			CVec3 cr = vColors[spotsSet[nSpot].nCanSee];
			nodesSet.push_back( pMission->GetScene()->CreateMesh( pModelBuilder, CVec4( cr, 1 ), 0 ) );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CViewBuildingScheme
////////////////////////////////////////////////////////////////////////////////////////////////////
CViewBuildingScheme::CViewBuildingScheme( CSyncSrc<NWorld::IVisObj> *pSrc, NGScene::IGameView *_pScene, vector<CObj<CObjectBase> > *_pRes ):
	CSyncDst<NWorld::IVisObj>(pSrc), pScene(_pScene), pRes(_pRes)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CViewBuildingScheme::AddBuilding( const SMapBuilding &info )
{
	pRes->push_back( ViewBuildingSchema( pScene, info.pVariant->GetRecordID(), info.pGrid, info.pos ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUpdateBuildingStability
////////////////////////////////////////////////////////////////////////////////////////////////////
CUpdateBuildingStability::CUpdateBuildingStability( CSyncSrc<NWorld::IVisObj> *pSrc ):
	CSyncDst<NWorld::IVisObj>(pSrc)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpdateBuildingStability::AddBuilding( const SMapBuilding &info )
{
	NBuilding::UpdateBuildingStability( info.pVariant->GetRecordID(), info.pGrid );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// COMMANDS
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarCheatSeeAll( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	CObjectBase *pBase = (CObjectBase*)pContext;
	IMission *pGame = dynamic_cast<IMission*>( pBase );
	ASSERT( pGame != 0 );

	if ( sValue.GetString() == L"on" )
		pGame->Command( new NWorld::CCmdCheat( NWorld::CHEAT_SEEALL, true ) );
	else if ( sValue.GetString() == L"off" )
		pGame->Command( new NWorld::CCmdCheat( NWorld::CHEAT_SEEALL, false ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarCheatShowAll( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	bShowAllCheat = false;
	if ( sValue.GetFloat() != 0 )
		bShowAllCheat = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarCheatGodMode( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	CObjectBase *pBase = (CObjectBase*)pContext;
	IMission *pGame = dynamic_cast<IMission*>( pBase );
	ASSERT( pGame != 0 );

	if ( sValue.GetString() == L"on" )
		pGame->Command( new NWorld::CCmdCheat( NWorld::CHEAT_GODMODE, true ) );
	else if ( sValue.GetString() == L"off" )
		pGame->Command( new NWorld::CCmdCheat( NWorld::CHEAT_GODMODE, false ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarCheatTeleport( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	CObjectBase *pBase = (CObjectBase*)pContext;
	IMission *pGame = dynamic_cast<IMission*>( pBase );
	ASSERT( pGame != 0 );

	if ( sValue.GetString() == L"on" )
		pGame->Command( new NWorld::CCmdCheat( NWorld::CHEAT_TELEPORT, true ) );
	else if ( sValue.GetString() == L"off" )
		pGame->Command( new NWorld::CCmdCheat( NWorld::CHEAT_TELEPORT, false ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandTypeCameraParams( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	CObjectBase *pBase = (CObjectBase*)pContext;
	IMission *pGame = dynamic_cast<IMission*>( pBase );
	ASSERT( pGame != 0 );

	ICamera::SCameraPos pos;
	pGame->GetCamera()->GetPlacement( &pos );
	csSystem << "current camera params:" << endl;
	csSystem << "pitch = " << pos.fPitch << "  yaw = " << pos.fYaw << endl;
	csSystem << "rod = " << pos.fRod << endl;
	csSystem << "anchor = ( " << pos.ptAnchor.x << ", " << pos.ptAnchor.y << ", " << pos.ptAnchor.z << " )" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandSetDefaultCameraLimits( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	if ( paramsSet.size() != 5 )
	{
		csSystem << "Usage: mission_camera_limits FOV minRod maxRod minPitch maxPitch" << endl;
		return;
	}
	fDefaultCameraFOV = atof( NStr::ToAscii( paramsSet[0] ).data() );
	defaultCameraLimits.fMinRod = atof( NStr::ToAscii( paramsSet[1] ).data() );
	defaultCameraLimits.fMaxRod = atof( NStr::ToAscii( paramsSet[2] ).data() );
	defaultCameraLimits.fMinPitch = atof( NStr::ToAscii( paramsSet[3] ).data() );
	defaultCameraLimits.fMaxPitch = atof( NStr::ToAscii( paramsSet[4] ).data() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Console commands
////////////////////////////////////////////////////////////////////////////////////////////////////
static NRPG::CGlobalPlayer* CreateGlobalPlayer( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	int &n = *(int*)pContext;
	if ( n >= paramsSet.size() )
		return 0;
	vector<int> pers;
	while ( n < paramsSet.size() )
	{
		int nPers = wcstol( paramsSet[n].data(), 0, 10 );
		if ( paramsSet[n++] == L"vs" )
			break;
		pers.push_back( nPers );
	}
	if ( pers.empty() )
		return 0;
	return NRPG::CreateGlobalPlayer( pers );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandStartMission( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	if ( paramsSet.size() == 0 )
	{
		csSystem << "usage: #zone [PersID [PersID]] [vs [PersID [PersID]]]" << endl;
		return;
	}

	int nTemp = wcstol( paramsSet.front().data(), 0, 10 );

	int nParam = 1;
	CPtr<NRPG::CGlobalGame> pGlobalGame = NRPG::CreateGlobalGame();
	for(;;)
	{
		CPtr<NRPG::CGlobalPlayer> pGlobalPlayer = CreateGlobalPlayer( szID, paramsSet, &nParam );
		if ( !IsValid( pGlobalPlayer ) )
			break;
		pGlobalGame->players.push_back( pGlobalPlayer.GetPtr() );
	}
	if ( pGlobalGame->players.empty() )
		pGlobalGame->players.push_back( NRPG::CreateGlobalPlayer() );
	//
	csSystem << CC_BLUE << "Loading world ( variant " << nTemp << " ) ..." << endl;
	//
	NMainLoop::Command( new CICBeginMission( nTemp, pGlobalGame ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandSetXPLevel( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	if ( paramsSet.size() < 2 )
		return;
	//
	CObjectBase *pObject = (CObjectBase *)pContext;
	if ( CDynamicCast<CMission> pMission( pObject) )
	{
		CPtr<NWorld::IWorld> pWorld = pMission->GetWorld();
		CPtr<IPlayerTracker> pTracker = pMission->GetActivePlayer();
		if ( !IsValid( pTracker ) )
			return;
		//
		vector< CPtr<NWorld::CUnit> > units;
		CPtr<NWorld::IPlayer> pActivePlayer = pTracker->GetPlayer();
		if ( paramsSet[0] == L"ally" )
			pActivePlayer->GetUnits( &units );
		else if ( paramsSet[0] == L"enemy" )
		{
			vector< CPtr<NWorld::CUnit> > tmpUnits;
			pWorld->GetAllUnits( &tmpUnits );
			for ( vector< CPtr<NWorld::CUnit> >::iterator i = tmpUnits.begin(); 
				i != tmpUnits.end(); ++i )
			{
				if ( (*i)->GetPlayer() != pActivePlayer )
					units.push_back( *i );
			}
		}
		//
		int nLevel = wcstol( paramsSet[1].c_str(), 0, 10 );
		for ( vector< CPtr<NWorld::CUnit> >::iterator i = units.begin(); i != units.end(); ++i )
			(*i)->GetRPG()->GetRPGUnit()->SetXPLevel( nLevel );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandStartMissionByTemplateID( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	int nParam = 1;
	CPtr<NRPG::CGlobalGame> pGlobalGame = NRPG::CreateGlobalGame();
	int nTemp = GetVariantForTemplate( wcstol( paramsSet.front().data(), 0, 10 ), pGlobalGame );
	for(;;)
	{
		CPtr<NRPG::CGlobalPlayer> pGlobalPlayer = CreateGlobalPlayer( szID, paramsSet, &nParam );
		if ( !IsValid( pGlobalPlayer ) )
			break;
		pGlobalGame->players.push_back( pGlobalPlayer.GetPtr() );
	}
	if ( pGlobalGame->players.empty() )
		pGlobalGame->players.push_back( NRPG::CreateGlobalPlayer() );
	//
	csSystem << CC_BLUE << "Loading world ( template " << nTemp << " ) ..." << endl;
	//
	NMainLoop::Command( new CICBeginMission( nTemp, pGlobalGame ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandSurrender( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	CObjectBase *pBase = (CObjectBase*)pContext;
	IMission *pGame = dynamic_cast<IMission*>( pBase );
	ASSERT( pGame != 0 );

	CPtr<NRPG::CGlobalGame> pGlobalGame = pGame->GetRPGGame();
	if ( pGlobalGame->players.size() == 1 )
		NMainLoop::Command( new CICContinueChapter( pGlobalGame ) );
	else
	{
		string szDummy( "" );
		NMainLoop::Command( new CICInterMission( szDummy, L"To many players for ChapterMap" ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICBeginMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICBeginMission::CICBeginMission( int _nVariantID, NRPG::CGlobalGame *_pGlobalGame ):
	nVariantID( _nVariantID ), pGlobalGame( _pGlobalGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICBeginMission::Exec()
{
	NMainLoop::ShowLogo();
	ResetStack();
	CMission *pRes = new CMission();
	pRes->Initialize( nVariantID, pGlobalGame );
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICBeginMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICMapEditBeginMission::CICMapEditBeginMission( int nVariantID, NRPG::CGlobalGame *pGlobalGame ):
	CICBeginMission( nVariantID, pGlobalGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICMapEditBeginMission::Exec()
{
	NMainLoop::ShowLogo();
	CMission *pRes = new CMission();
	pRes->Initialize( nVariantID, pGlobalGame );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(iMission)
	REGISTER_CMD( "map", CommandStartMission )
	REGISTER_CMD( "mission_camera_limits", CommandSetDefaultCameraLimits )
	REGISTER_VAR( "cheat_showall", VarCheatShowAll, 0, false )
	REGISTER_CMD( "template", CommandStartMissionByTemplateID )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
// GetVariantForTemplate
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetVariantForTemplate( int nTemplate, NRPG::CGlobalGame *pGlobalGame )
{
	ASSERT( IsValid( pGlobalGame ) );
	if ( !IsValid( pGlobalGame ) )
		return 0;
	//
	SRandomSeed sSeed = pGlobalGame->pScenarioTracker->GetRandomSeedForTemplate( nTemplate );
	//
	CPtr<NDb::CTemplate> pTemplate = NDb::GetTemplate( nTemplate );
	if ( IsValid( pTemplate ) )
	{
		SRand sRand( sSeed );
		vector<int> dummyTemp;
		return NDb::GetTemplVariant( pTemplate, dummyTemp, -1, &sRand )->GetRecordID();
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB251121B, CMission )
REGISTER_SAVELOAD_CLASS( 0x02511218, CVisibleTracker )
