#include "StdAfx.h"
#include "wInterface.h"
#include "GView.h"
#include "RWGame.h"
#include "RWSound.h"
#include "GSceneUtils.h"
#include "Transform.h"
#include "DiscretePos.h"
#include "RPGGame.h"
#include "RPGGlobal.h"
#include "RPGItemInfo.h"
#include "RPGUnitInfo.h"
#include "..\Input\Bind.h"
#include "Interface.h"
#include "iGlobalGame.h"
#include "iChapterMap.h"
#include "iMissionUI.h"
#include "iMissionStates.h"
#include "..\Misc\LogStream.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_SHOWVICTORY_TTL = 8000;
const float
	F_MIN_SELECTION_DIST	= 20;
const CVec4
	V_SELECTIONCOLOR_TEAM					= CVec4( 0.1f, 0.1f, 1, 1 ),
	V_SELECTIONCOLOR_TEAM_HILIGHT	= CVec4( 1, 1, 1, 0.5 ),
	V_SELECTIONCOLOR_ENEMY				= CVec4( 1, 0.1f, 0.1f, 1 ),
	V_SELECTIONCOLOR_CORPSE				= CVec4( 0, 0, 0, 1 ),
	V_SELECTIONCOLOR_OBJECT				= CVec4( 0.1f, 1, 0.1f, 1 );
const int
	N_CURSOR_NORMAL					= 281,
	N_CURSOR_BUSY						= 217,
	N_CURSOR_BLOCK					= 218,
	N_CURSOR_MOVE						= 202,
	N_CURSOR_ROTATE					= 202,
	N_CURSOR_HEAL						= 208,
	N_CURSOR_ATTACK					= 203,
	N_CURSOR_ATTACK_MELEE		= 204,
	N_CURSOR_ATTACK_RIFLE		= 210,
	N_CURSOR_ATTACK_PISTOL	= 209,
	N_CURSOR_ATTACK_MACHINEGUN	= 205,
	N_CURSOR_ATTACK_GRENADE	= 206,
	N_CURSOR_ATTACK_HEAD		= 282,
	N_CURSOR_ATTACK_BODY		= 283,
	N_CURSOR_ATTACK_LARM		= 286,
	N_CURSOR_ATTACK_RARM		= 287,
	N_CURSOR_ATTACK_LLEG		= 284,
	N_CURSOR_ATTACK_RLEG		= 285,
	N_CURSOR_OPEN_CLOSE			= 207;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMissionState
////////////////////////////////////////////////////////////////////////////////////////////////////
CMissionState::CMissionState( IGlobalGame *_pMissionGame, NUI::CMissionUI *_pMissionUI ):
	pMissionGame( _pMissionGame ), pMissionUI( _pMissionUI )
{
	pTileHilightPos = new NGScene::CCFBTransform;
	pTileHilightLG = pMissionGame->GetScene()->CreateLightGroup();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionState::Terminate()
{
	pTileHilight = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionState::ShowTileHilight()
{
	SFBTransform fbPos;

	NAI::SPosition sTraceTile;
	if ( pMissionGame->GetTracePosition( &sTraceTile ) )
	{
		MakeMatrix( &fbPos, 0, 0, sTraceTile.GetCP() );
		pTileHilightPos->Set( fbPos );

		if ( !IsValid( pTileHilight ) )
			pTileHilight = pMissionGame->GetScene()->CreateMesh( NDb::GetModel( 82 ), pTileHilightPos, NGScene::SRoomInfo( pTileHilightLG ) );
	}
	else
	{
		pTileHilight = 0;
	}

	bShowTileHilight = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionState::HideTileHilight()
{
	pTileHilight = 0;
	bShowTileHilight = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionState::Step()
{
	if ( GetGame()->IsInterfaceHidden() )
		pTileHilight = 0;
	else if ( bShowTileHilight )
		ShowTileHilight();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IGlobalGame* CMissionState::GetMissionGame() const
{
	return pMissionGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::CMissionUI* CMissionState::GetInterface() const
{
	return pMissionUI;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateWait
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateWait::Initialize( IGlobalGame *pGame )
{
	CStateBase::Initialize( pGame );

	if ( GetGame()->IsRealTime() || !GetGame()->IsActionExecuted() )
		return false;

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::SCursorInfo CStateWait::GetCursorInfo() const
{
	return NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_BUSY ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateTeam
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateTeam::CStateTeam(): 
	bindModifier( "modifier" ), bModifier( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateTeam::CStateTeam( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI ):
	CMissionState( pMissionGame, pMissionUI ), bindModifier( "modifier" ), bModifier( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateTeam::Initialize( IGlobalGame *pGame )
{
	CMissionState::Initialize( pGame );

	CObjectBase* pObject = GetGame()->GetStateTarget();
	if ( !IsValid( pObject ) )
		return false;
	if ( CDynamicCast<NWorld::CUnit> pUnit( pObject ) )
	{
		if ( pUnit->GetPlayer() != GetMissionGame()->GetActivePlayer()->GetPlayer() )
			return false;

		ShowTileHilight();
		pTraceSelection = GetMissionGame()->GetRenderGame()->Select( pUnit, V_SELECTIONCOLOR_TEAM );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTeam::Terminate()
{
	CMissionState::Terminate();
	pTraceSelection = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateTeam::ProcessEvent( const NInput::SEvent &sEvent )
{
	bindModifier.ProcessEvent( sEvent );
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTeam::Step()
{
	CMissionState::Step();
	bModifier = bindModifier.IsActive();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateTeam::OnLButtonUp()
{
	CObjectBase* pObject = GetGame()->GetStateTarget();
	if ( !IsValid( pObject ) )
		return false;

	if ( CDynamicCast<NWorld::CUnit> pUnit( pObject ) )
		GetMissionGame()->Select( pUnit, bModifier );

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateMove
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateMove::CStateMove(): 
	bindStrafe( "pose_strafe" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateMove::CStateMove( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI, bool _bForced ):
	CMissionState( pMissionGame, pMissionUI ), 	bindStrafe( "pose_strafe" ), bForced( _bForced )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateMove::Initialize( IGlobalGame *pGame )
{
	CMissionState::Initialize( pGame );

	bAnchorSet = false;

	SActionInfo sGeneralInfo;
	pGame->GetActionInfo( UA_MOVE, &sGeneralInfo );
	if ( ( GetType() == FORCED ) && ( !sGeneralInfo.bOk || !sGeneralInfo.bEnoughAP ) )
	{
		csGame << GetErrorString( sGeneralInfo.eResult ) << endl;
		return false;
	}

	ShowTileHilight();
	UpdateCursor();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateMove::ProcessEvent( const NInput::SEvent &sEvent )
{
	bindStrafe.ProcessEvent( sEvent );
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IState::EType CStateMove::GetType() const
{
	if ( bForced )
		return FORCED;

	return UPDATED;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateMove::OnLButtonUp()
{
	DoMove( GetType() == FORCED );

	if ( GetType() == FORCED )
		GetGame()->ResetState();

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateMove::OnLButtonDown()
{
	vAnchor = GetGame()->GetCursor()->GetPos();
	bAnchorSet = true;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateMove::OnLButtonDblClk()
{
	DoMove( true );

	if ( GetType() == FORCED )
		GetGame()->ResetState();

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::SCursorInfo CStateMove::GetCursorInfo() const
{
	return sCursorInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateMove::DoMove( bool bInstant )
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	NAI::SPosition pos;
	if ( !GetMissionGame()->GetTracePosition( &pos ) )
		return;

	vector< NAI::SPosition > unitPlaces;
	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
	{
		NAI::SPosition posUnit = (*iTemp)->GetUnit()->GetPosition().pos;
		unitPlaces.push_back( posUnit );
	}

	GetGame()->GetWorld()->GetPathNetwork()->FormationMoveTo( &unitPlaces, pos  );

	for ( int nUnit = 0; nUnit < unitsSet.size(); ++nUnit )
		unitsSet[nUnit]->SetTargetPosition( unitPlaces[nUnit], bInstant, bindStrafe.IsActive() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateMove::UpdateCursor()
{
	NAI::SPosition pos;
	if ( !GetMissionGame()->GetTracePosition( &pos ) )
	{
		sCursorInfo = NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_BLOCK ) );
		return;
	}

	NAI::SPathPlace p( pos.p );
	p.SetPose( NAI::CM_STAND );
	if ( GetGame()->GetWorld()->GetPathNetwork()->IsNativePassable( p ) )
		sCursorInfo = NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_MOVE ) );
	else
		sCursorInfo = NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_BLOCK ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateMove::Step()
{
	CMissionState::Step();

	if ( bAnchorSet && ( fabs( GetGame()->GetCursor()->GetPos() - vAnchor ) > F_MIN_SELECTION_DIST ) )
		GetGame()->CommandState( new CStateSelection( GetMissionGame(), GetInterface(), vAnchor ) );

	UpdateCursor();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateAttack
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateAttack::CStateAttack(): 
	eHitLocation( NAI::HL_ANY ),
	bindHitLocationHead( "hitlocation_head" ), bindHitLocationBody( "hitlocation_body" ), 
	bindHitLocationLArm( "hitlocation_larm" ), bindHitLocationRArm( "hitlocation_rarm" ), bindHitLocationLLeg( "hitlocation_lleg" ), bindHitLocationRLeg( "hitlocation_rleg" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateAttack::CStateAttack( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI, bool _bForced ):
	CMissionState( pMissionGame, pMissionUI ), eHitLocation( NAI::HL_ANY ), bForced( _bForced ),
	bindHitLocationHead( "hitlocation_head" ), bindHitLocationBody( "hitlocation_body" ), 
	bindHitLocationLArm( "hitlocation_larm" ), bindHitLocationRArm( "hitlocation_rarm" ), bindHitLocationLLeg( "hitlocation_lleg" ), bindHitLocationRLeg( "hitlocation_rleg" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateAttack::Initialize( IGlobalGame *pGame )
{
	CMissionState::Initialize( pGame );

	SActionInfo sGeneralInfo;
	pGame->GetActionInfo( UA_ATTACK, &sGeneralInfo );
	if ( ( GetType() == FORCED ) && ( !sGeneralInfo.bOk || !sGeneralInfo.bEnoughAP ) )
	{
		csGame << GetErrorString( sGeneralInfo.eResult ) << endl;
		return false;
	}

	if ( GetType() != FORCED )
	{
		CObjectBase* pObject = GetGame()->GetStateTarget();
		if ( !IsValid( pObject ) )
			return false;

		NWorld::CUnit* pUnit = dynamic_cast<NWorld::CUnit*>( pObject );
		if ( !IsValid( pUnit ) )
			return false;

		if ( pUnit->GetPlayer() == GetGame()->GetActivePlayer()->GetPlayer() )
			return false;
	}

	ShowTileHilight();
	UpdateTraceSelection();
	UpdateBlockedState();
	UpdateCursorInfo();
	UpdateCursor();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::Terminate()
{
	CMissionState::Terminate();
	pTraceSelection = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IState::EType CStateAttack::GetType() const
{
	if ( bForced )
		return FORCED;

	return UPDATED;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateAttack::OnLButtonUp()
{
	SActionInfo sInfo;
	CObj<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( !IsValid( pCmd ) )
		return true;

	GetGame()->CanDoCommand( pCmd, false, &sInfo );
	if ( !sInfo.bAvailable || !sInfo.bOk )
	{
		csGame << GetErrorString( sInfo.eResult ) << endl;
		return false;
	}

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );
	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
		GetGame()->Command( (*iTemp)->GetUnit(), pCmd );

	if ( GetType() == FORCED )
		GetGame()->ResetState();

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::SCursorInfo CStateAttack::GetCursorInfo() const
{
	return sCursorInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCmd* CStateAttack::GetTargetCmd()
{
	CObjectBase* pTargetObject = GetGame()->GetStateTarget();
	if ( IsValid( pTargetObject ) )
	{
		NWorld::IVisObj* pObject = dynamic_cast<NWorld::IVisObj*>( pTargetObject );
		if ( IsValid( pObject ) )
			return new NWorld::CCmdShootObject( pObject, 0, eHitLocation );
	}

	NAI::SPosition pos;
	if ( !GetMissionGame()->GetTracePosition( &pos ) )
		return 0;

	return new NWorld::CCmdShootTile( pos.GetCP() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::UpdateCursor()
{
	NAI::EHitLocation eNewHitLocation = NAI::HL_ANY;

	if ( bindHitLocationHead.IsActive() )
		eNewHitLocation = NAI::HL_HEAD;
	else if ( bindHitLocationBody.IsActive() )
		eNewHitLocation = NAI::HL_BODY;
	else if ( bindHitLocationLArm.IsActive() )
		eNewHitLocation = NAI::HL_LHAND;
	else if ( bindHitLocationRArm.IsActive() )
		eNewHitLocation = NAI::HL_RHAND;
	else if ( bindHitLocationLLeg.IsActive() )
		eNewHitLocation = NAI::HL_LLEG;
	else if ( bindHitLocationRLeg.IsActive() )
		eNewHitLocation = NAI::HL_RLEG;

	if ( eNewHitLocation != eHitLocation )
	{
		eHitLocation = eNewHitLocation;
		UpdateCursorInfo();
	}

	if ( !bActionUnavailable )
	{
		switch ( eHitLocation )
		{
		case NAI::HL_ANY:
			{
				int nDefault = N_CURSOR_ATTACK;
				vector< CPtr<NGame::IUnitTracker> > unitsSet;
				GetGame()->GetSelectedUnits( &unitsSet );
				if ( unitsSet.size() == 1 )
				{
					NWorld::CUnit::EState eState = unitsSet[0]->GetUnit()->GetState();
					switch( eState )
					{
						case NWorld::CUnit::ST_NORMAL_MELEE:
							nDefault = N_CURSOR_ATTACK_MELEE;
							break;
						case NWorld::CUnit::ST_NORMAL_GRENADE:
							nDefault = N_CURSOR_ATTACK_GRENADE;
							break;
						case NWorld::CUnit::ST_NORMAL_PISTOL:
							nDefault = N_CURSOR_ATTACK_PISTOL;
							break;
						case NWorld::CUnit::ST_NORMAL_RIFLE:
							nDefault = N_CURSOR_ATTACK_RIFLE;
							break;
						case NWorld::CUnit::ST_NORMAL_SUB_MACHINE_GUN:
						case NWorld::CUnit::ST_NORMAL_HAND_MACHINE_GUN: // ??????????????????????
						case NWorld::CUnit::ST_NORMAL_RLAUNCHER: // ??????????????????????
							nDefault = N_CURSOR_ATTACK_MACHINEGUN;
							break;
					}
				}

				sCursorInfo.pTexture = NDb::GetUITexture( nDefault );
			}
			break;
		case NAI::HL_HEAD:
			sCursorInfo.pTexture = NDb::GetUITexture( N_CURSOR_ATTACK_HEAD );
			break;
		case NAI::HL_BODY:
			sCursorInfo.pTexture = NDb::GetUITexture( N_CURSOR_ATTACK_BODY );
			break;
		case NAI::HL_LHAND:
			sCursorInfo.pTexture = NDb::GetUITexture( N_CURSOR_ATTACK_LARM );
			break;
		case NAI::HL_RHAND:
			sCursorInfo.pTexture = NDb::GetUITexture( N_CURSOR_ATTACK_RARM );
			break;
		case NAI::HL_LLEG:
			sCursorInfo.pTexture = NDb::GetUITexture( N_CURSOR_ATTACK_LLEG );
			break;
		case NAI::HL_RLEG:
			sCursorInfo.pTexture = NDb::GetUITexture( N_CURSOR_ATTACK_RLEG );
			break;
		default:
			ASSERT( 0 );
		}
	}
	else
		sCursorInfo = NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_BLOCK ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::UpdateCursorInfo()
{
	sCursorInfo.wsText = L"";

	int nSelectedCount = GetGame()->CountSelected();
	if ( nSelectedCount )
	{
		CPtr<CObjectBase> pTraceObject = GetGame()->GetStateTarget();

		int nTotalShootAP = 0;
		int nMin = 0x7fffffff, nMax = -0x7fffffff; // numeric_limits<int>::max(), numeric_limits<int>::min();
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );
		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
		{
			CPtr<NWorld::IWorld> pWorld = GetGame()->GetWorld();

			int nToHit = 0;

			NAI::SPosition pos;
			bool bTraceOk = GetMissionGame()->GetTracePosition( &pos );

			if ( CDynamicCast<NWorld::CUnit> pUnit( pTraceObject ) )
			{
				if ( CDynamicCast<NRPG::IWeaponItemInfo> pWeapon( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( !pWeapon->GetDBWeapon()->bBazookaLogic )
						nToHit = pWorld->GetGame()->GetCompositeToHit( (*iTemp)->GetUnit(), pUnit, eHitLocation, pWorld->IsFirstTurn() );
					else
					{
						if ( bTraceOk )
						nToHit = pWorld->GetGame()->GetBazookaToHit( (*iTemp)->GetUnit(), pos.GetCP(),	NAI::THL_MIDDLE, pWorld->IsFirstTurn() );
					}
				}
				else if ( CDynamicCast<NRPG::IGrenadeItem> pGrenade( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( bTraceOk )
						nToHit = pWorld->GetGame()->GetGrenadeCompositeToHit( (*iTemp)->GetUnit(), pos.GetCP(), pWorld->IsFirstTurn(), pGrenade->GetDBGrenade() );
				}
				else if ( CDynamicCast<NRPG::IMeleeWeaponItem> pMelee( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( pMelee->GetDBMeleeWeapon()->bThrowing )
						nToHit = pWorld->GetGame()->GetCompositeToHit( (*iTemp)->GetUnit(), pUnit, eHitLocation, pWorld->IsFirstTurn() );	
				}
			}
			else if ( bTraceOk )
			{
				if ( CDynamicCast<NRPG::IGrenadeItem> pGrenade( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
					nToHit = pWorld->GetGame()->GetGrenadeCompositeToHit( (*iTemp)->GetUnit(), pos.GetCP(), pWorld->IsFirstTurn(), pGrenade->GetDBGrenade() );
				else	if ( CDynamicCast<NRPG::IWeaponItemInfo> pWeapon( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( !pWeapon->GetDBWeapon()->bBazookaLogic )
						nToHit = pWorld->GetGame()->GetTileCompositeToHit( (*iTemp)->GetUnit(), pos.GetCP(), NAI::THL_MIDDLE, pWorld->IsFirstTurn() );
					else
						nToHit = pWorld->GetGame()->GetBazookaToHit( (*iTemp)->GetUnit(), pos.GetCP(),
							NAI::THL_MIDDLE, pWorld->IsFirstTurn() );
				}
				else if ( CDynamicCast<NRPG::IMeleeWeaponItem> pMelee( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( pMelee->GetDBMeleeWeapon()->bThrowing )
						nToHit = pWorld->GetGame()->GetTileCompositeToHit( (*iTemp)->GetUnit(), pos.GetCP(), NAI::THL_MIDDLE, pWorld->IsFirstTurn() );	
				}
			}

			nMin = min( nMin, nToHit );
			nMax = max( nMax, nToHit );

			WCHAR wsString[256];
			if ( unitsSet.size() == 1 )
			{
				if ( !GetGame()->IsRealTime() )
					swprintf( wsString, L"<normal>%2d%%<br>AP: %d", nMin, nActionAP );
				else
					swprintf( wsString, L"<normal>%2d%%", nMin );
			}
			else
				swprintf( wsString, L"<normal>%2d-%2d%%", nMin, nMax );

			sCursorInfo.wsText = wsString;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::UpdateBlockedState()
{
	nActionAP = 0;
	bActionUnavailable = true;

	SActionInfo sInfo;
	CObj<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( !IsValid( pCmd ) )
		return;

	GetGame()->CanDoCommand( pCmd, false, &sInfo );

	nActionAP = sInfo.nActionAP;
	bActionUnavailable = !sInfo.bAvailable || !sInfo.bOk;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::UpdateTraceSelection()
{
	pTraceSelection = 0;

	CObjectBase* pObject = GetGame()->GetStateTarget();
	if ( !IsValid( pObject ) )
		return;

	if ( CDynamicCast<NWorld::IVisObj> pVisObject( pObject ) )
		pTraceSelection = GetGame()->GetRenderGame()->Select( pVisObject, V_SELECTIONCOLOR_ENEMY );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::Step()
{
	CMissionState::Step();

	if ( GetType() == FORCED )
	{
		UpdateBlockedState();
		UpdateCursorInfo();
	}

	UpdateCursor();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateUse
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateUse::Initialize( IGlobalGame *pGame )
{
	CMissionState::Initialize( pGame );

	SActionInfo sGeneralInfo;
	pGame->GetActionInfo( UA_USE, &sGeneralInfo );
	if ( ( GetType() == FORCED ) && ( !sGeneralInfo.bOk || !sGeneralInfo.bEnoughAP ) )
	{
		csGame << GetErrorString( sGeneralInfo.eResult ) << endl;
		return false;
	}

	CObjectBase* pTargetObject = GetGame()->GetStateTarget();
	if ( !IsValid( pTargetObject ) )
		return false;

	NWorld::IVisObj* pObject = dynamic_cast<NWorld::IVisObj*>( pTargetObject );
	if ( !IsValid( pObject ) )
		return false;

	bool bRet = false;
	bool bTileHilight = false;
	CVec4 vHilightColor( V_SELECTIONCOLOR_OBJECT );
	if ( CDynamicCast<NWorld::CUnit> pDeadUnit( pObject ) )
	{
		bRet = pDeadUnit->IsDead();
		bTileHilight = false;
		vHilightColor = V_SELECTIONCOLOR_CORPSE;
	}
	else if ( CDynamicCast<NWorld::IObject> pTempObject( pObject ) )
	{
		bTileHilight = true;
		vHilightColor = V_SELECTIONCOLOR_OBJECT;

		if ( CDynamicCast<NWorld::ICannon> pCannon( pTempObject.GetPtr() ) )
			bRet = !pCannon->IsBroken();
		else if ( CDynamicCast<NWorld::IWindowDoor> pWindowDoor( pTempObject.GetPtr() ) )
			bRet = !pWindowDoor->IsBroken();
	}

	if ( !bRet )
		return false;

	if ( bTileHilight )
		ShowTileHilight();
	else
		HideTileHilight();

	pTraceSelection = GetGame()->GetRenderGame()->Select( pObject, vHilightColor );

	SActionInfo sInfo;
	CObj<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( IsValid( pCmd ) )
		GetGame()->CanDoCommand( GetTargetCmd(), false, &sInfo );

	if ( sInfo.bOk )
	{
		WCHAR wsBuffer[1024] = L"";
		if ( !GetGame()->IsRealTime() )
			swprintf( wsBuffer, L"AP: %d", sInfo.nActionAP );
		sCursorInfo = NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_OPEN_CLOSE ), wsBuffer );
	}
	else
		sCursorInfo = NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_BLOCK ) );

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateUse::Terminate()
{
	CMissionState::Terminate();
	pTraceSelection = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateUse::OnLButtonUp()
{
	SActionInfo sInfo;
	CObj<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( !IsValid( pCmd ) )
		return true;

	GetGame()->CanDoCommand( pCmd, false, &sInfo );
	if ( !sInfo.bAvailable || !sInfo.bOk )
	{
		csGame << GetErrorString( sInfo.eResult ) << endl;
		return false;
	}

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() < 1 )
		return false;

	GetGame()->Command( unitsSet.front()->GetUnit(), pCmd );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::SCursorInfo CStateUse::GetCursorInfo() const
{
	return sCursorInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCmd* CStateUse::GetTargetCmd()
{
	CObjectBase* pObject = GetGame()->GetStateTarget();
	if ( !IsValid( pObject ) )
		return 0;

	if ( CDynamicCast<NWorld::CUnit> pDeadUnit( pObject ) )
	{
		if ( pDeadUnit->IsDead() )
		{
			if ( !pDeadUnit->GetCorpseCarrier() )
				return new NWorld::CCmdTakeCorpse( pDeadUnit );
			else
				return new NWorld::CCmdDropCorpse;
		}
	}
	else if ( CDynamicCast<NWorld::IObject> pTempObject( pObject ) )
	{
		if ( CDynamicCast<NWorld::ICannon> pCannon( pTempObject.GetPtr() ) )
		{
			if ( !pCannon->IsBroken() )
			{
				if ( !pCannon->IsOccupied() )
					return new NWorld::CCmdCannon( pTempObject );
				else
					return new NWorld::CCmdExitCannon;
			}
		}
		else if ( CDynamicCast<NWorld::IWindowDoor> pWindowDoor( pTempObject.GetPtr() ) )
		{
			if ( !pWindowDoor->IsBroken() )
				return new NWorld::CCmdOpenClose( pTempObject, !pWindowDoor->IsOpen() );
		}
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatePickItem
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStatePickItem::Initialize( IGlobalGame *pGame )
{
	CMissionState::Initialize( pGame );

	CObjectBase* pTargetObject = GetGame()->GetStateTarget();
	if ( !IsValid( pTargetObject ) )
		return false;

	NWorld::IVisObj* pObject = dynamic_cast<NWorld::IVisObj*>( pTargetObject );
	if ( !IsValid( pObject ) )
		return false;

	if ( CDynamicCast<NWorld::IItem> pItem( pObject ) )
	{
		if ( !IsValid( pItem->GetInvItem() ) )
			return false;

		HideTileHilight();
		pTraceSelection = GetGame()->GetRenderGame()->Select( pObject, V_SELECTIONCOLOR_OBJECT );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatePickItem::Terminate()
{
	CMissionState::Terminate();
	pTraceSelection = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStatePickItem::OnLButtonUp()
{
	SActionInfo sInfo;
	CObj<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( !IsValid( pCmd ) )
		return true;

	GetGame()->CanDoCommand( pCmd, false, &sInfo );
	if ( !sInfo.bAvailable || !sInfo.bOk )
	{
		csGame << GetErrorString( sInfo.eResult ) << endl;
		return false;
	}

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
		return false;

	GetGame()->Command( unitsSet[0]->GetUnit(), pCmd );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::SCursorInfo CStatePickItem::GetCursorInfo() const
{
	return NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_OPEN_CLOSE ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCmd* CStatePickItem::GetTargetCmd()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
		return 0;

	CPtr<CObjectBase> pObject = GetGame()->GetStateTarget();
	if ( CDynamicCast<NWorld::IItem> pTempItem( pObject ) )
	{
		const NUI::SInterfaceState &sState = GetInterface()->GetInterfaceState();

		NWorld::SItem sSource;
		sSource.eType = NWorld::SItem::GROUND;
		sSource.pItem = pTempItem->GetInvItem();
		sSource.pWorldItem = pTempItem;
		sSource.pUnit = 0;
		sSource.nSlot = -1;

		if ( !sState.bInventory )
		{
			NWorld::SItem sTarget;
			sTarget.eType = NWorld::SItem::BACKPACK;
			sTarget.pUnit = unitsSet[0]->GetUnit();
			sTarget.sPosition = CTPoint<int>( -1, -1 );
			return new NWorld::CCmdMoveInventoryItem( sSource, sTarget );
		}

		return new NWorld::CCmdMoveInventoryItem( sSource, NWorld::SItem( unitsSet[0]->GetUnit(), NWorld::SItem::HAND ) );
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// FORCED STATES
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateRotate
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateRotate::Initialize( IGlobalGame *pGame )
{
	CMissionState::Initialize( pGame );

	SActionInfo sGeneralInfo;
	pGame->GetActionInfo( UA_LOOK, &sGeneralInfo );
	if ( ( GetType() == FORCED ) && ( !sGeneralInfo.bOk || !sGeneralInfo.bEnoughAP ) )
	{
		csGame << GetErrorString( sGeneralInfo.eResult ) << endl;
		return false;
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateRotate::OnLButtonUp()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	NAI::SPosition pos;
	if ( !GetMissionGame()->GetTracePosition( &pos ) )
		return true;

	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
	{
		NAI::EDirection eDir = GetGame()->GetWorld()->GetPathNetwork()->GetClosestDir( (*iTemp)->GetUnit()->GetPosition().pos.p, pos.p );	
		NAI::SPosition sPos = (*iTemp)->GetUnit()->GetPosition().pos;
		sPos.p.SetDirection( eDir );
		GetGame()->Command( (*iTemp)->GetUnit(), new NWorld::CCmdPath( sPos, NAI::PF_USE_DIR ) );
	}

	GetGame()->ResetState();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::SCursorInfo CStateRotate::GetCursorInfo() const
{
	return NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_ROTATE ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateFirstAid
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateFirstAid::Initialize( IGlobalGame *pGame )
{
	CMissionState::Initialize( pGame );

	SActionInfo sGeneralInfo;
	pGame->GetActionInfo( UA_HEAL, &sGeneralInfo );
	if ( ( GetType() == FORCED ) && ( !sGeneralInfo.bOk || !sGeneralInfo.bEnoughAP ) )
	{
		csGame << GetErrorString( sGeneralInfo.eResult ) << endl;
		return false;
	}

	ShowTileHilight();

	SActionInfo sInfo;
	CPtr<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( IsValid( pCmd ) )
		GetGame()->CanDoCommand( pCmd, false, &sInfo );


	if ( sInfo.bAvailable )
	{
		WCHAR wsBuffer[1024] = L"";
		if ( GetGame()->IsRealTime() )
			swprintf( wsBuffer, L"AP: %d", sInfo.nActionAP );
		sCursorInfo = NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_HEAL ), wsBuffer );
	}
	else
		sCursorInfo = NUI::SCursorInfo( NDb::GetUITexture( N_CURSOR_BLOCK ) );

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateFirstAid::OnLButtonUp()
{
	SActionInfo sInfo;
	CObj<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( !IsValid( pCmd ) )
		return true;

	GetGame()->CanDoCommand( pCmd, false, &sInfo );
	if ( !sInfo.bAvailable || !sInfo.bOk )
	{
		csGame << GetErrorString( sInfo.eResult ) << endl;
		return false;
	}

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
		GetGame()->Command( (*iTemp)->GetUnit(), pCmd );

	if ( GetType() == FORCED )
		GetGame()->ResetState();

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::SCursorInfo CStateFirstAid::GetCursorInfo() const
{
	return sCursorInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCmd* CStateFirstAid::GetTargetCmd()
{
	CObjectBase* pObject = GetGame()->GetStateTarget();
	if ( !IsValid( pObject ) )
		return 0;

	NWorld::CUnit* pUnit = dynamic_cast<NWorld::CUnit*>( pObject );
	if ( !IsValid( pUnit ) )
		return 0;

	if ( pUnit->GetPlayer() != GetMissionGame()->GetActivePlayer()->GetPlayer() )
		return 0;

	return new NWorld::CCmdHeal( pUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatePose
////////////////////////////////////////////////////////////////////////////////////////////////////
CStatePose::CStatePose( NAI::EPose _ePose ):
	ePose( _ePose )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStatePose::Initialize( IGlobalGame *pGame )
{
	CStateBase::Initialize( pGame );

	EUnitAction eAction;
	switch( ePose )
	{
	case NAI::RUN:
		eAction = UA_POSERUN;
		break;
	case NAI::WALK:
		eAction = UA_POSEWALK;
		break;
	case NAI::CROUCH:
		eAction = UA_POSECROUCH;
		break;
	case NAI::CRAWL:
		eAction = UA_POSECRAWL;
		break;
	default:
		eAction = UA_POSEWALK;
		ASSERT( 0 );
	}

	SActionInfo sInfo;
	GetGame()->GetActionInfo( eAction, &sInfo );
	if ( sInfo.eResult != NWorld::UCR_OK )
		return false;

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	pGame->GetSelectedUnits( &unitsSet );
	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
	{
		NAI::SUnitPosition uPos = (*iTemp)->GetUnit()->GetPosition();
		uPos.SetPose( ePose );
		pGame->Command( (*iTemp)->GetUnit(), new NWorld::CCmdWishPose( ePose ) );
		pGame->Command( (*iTemp)->GetUnit(), new NWorld::CCmdPath( uPos.pos, NAI::PF_USE_POSEDIR ) );
			//GetGame()->Command( new NWorld::CCmdGo( (*iTemp)->GetUnit() ) );
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateSelection
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateSelection::CStateSelection( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI, const CVec2 &_vAnchor ): 
	CMissionState( pMissionGame, pMissionUI ), vAnchor( _vAnchor )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateSelection::Initialize( IGlobalGame *pGame )
{
	CMissionState::Initialize( pGame );

	pSelection = new NUI::CImage( NUI::SWindowInfo( GetInterface(), NUI::SPoint( 0, 0 ), NUI::SPoint( 0, 0 ), "selection", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_TRANSPARENT | NUI::STYLE_BOTTOMMOST ) );
	pSelection ->SetColor( NGfx::SPixel8888( 0, 0, 0, 0x7F ) );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateSelection::Terminate()
{
	pSelection = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateSelection::OnLButtonUp()
{
	bool bKeepSelection = false;
	CVec2 vCursorPos = GetGame()->GetCursor()->GetPos();
	CVec2 vScreenRect = GetGame()->GetScene()->GetScreenRect();
	CTransformStack sTS = GetMissionGame()->GetCameraTransform();

	NUI::SRect sRect( Min( vAnchor.x, vCursorPos.x ), Min( vAnchor.y, vCursorPos.y ), Max( vAnchor.x, vCursorPos.x ), Max( vAnchor.y, vCursorPos.y ) );

	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetUnits( &unitsSet );
	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
	{
		CVec2 vScreenPos;
		if ( !TestRayInFrustrum( (*iTemp)->GetUnit()->GetPosition().GetCP(), &sTS, vScreenRect, &vScreenPos ) )
			continue;
		if ( ( vScreenPos.x < sRect.x1 ) || ( vScreenPos.x > sRect.x2 ) || ( vScreenPos.y < sRect.y1 ) || ( vScreenPos.y > sRect.y2 ) )
			continue;

		GetGame()->Select( (*iTemp)->GetUnit(), bKeepSelection );
		bKeepSelection = true;
	}

	GetGame()->ResetState();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateSelection::Step()
{
	CMissionState::Step();

	CVec2 vCursorPos = GetGame()->GetCursor()->GetPos();
	CVec2 vScreenRect = GetGame()->GetScene()->GetScreenRect();

	NUI::SRect sRect( vAnchor.x * 1024 / vScreenRect.x, vAnchor.y * 768 / vScreenRect.y, vCursorPos.x * 1024 / vScreenRect.x, vCursorPos.y * 768 / vScreenRect.y );
	pSelection->SetSize( NUI::SPoint( abs( sRect.Width() ), abs( sRect.Height() ) ) );
	pSelection->SetPosition( NUI::SPoint( Min( sRect.x1, sRect.x2 ), Min( sRect.y1, sRect.y2 ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateVictory
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateVictory::CStateVictory():
	bindEndMission( "endmission" ), bindContinueMission( "continuemission" ), bComplete( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateVictory::Initialize( IGlobalGame *pGame )
{
	CStateBase::Initialize( pGame );

	if ( bComplete )
		return false;
	if ( !GetGame()->GetWorld()->IsWinnerPlayer( GetGame()->GetActivePlayer()->GetPlayer() ) )
		return false;

	pWindow = new NUI::CWindow( NUI::SWindowInfo( pGame->GetInterface(), NUI::SPoint( 0, 0 ), NUI::SPoint( 0, 0 ), "win", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_TOPMOST ) );
	NUI::LoadTemplate( pWindow, NDb::GetUIContainer( 154 ) );
	pWindow->ShowWindow( NUI::SWTYPE_SHOW );

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateVictory::Terminate()
{
	pWindow = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateVictory::ProcessEvent( const NInput::SEvent &sEvent )
{
	if ( bindEndMission.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICContinueChapter(	GetGame()->GetRPGGame() ) );
		return true;
	}

	if ( bindContinueMission.ProcessEvent( sEvent ) )
	{
		bComplete = true;
		GetGame()->ResetState();
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB1007120, CStateWait )
REGISTER_SAVELOAD_CLASS( 0xB1007121, CStateTeam )
REGISTER_SAVELOAD_CLASS( 0xB1007122, CStateMove )
REGISTER_SAVELOAD_CLASS( 0xB1007123, CStateAttack )
REGISTER_SAVELOAD_CLASS( 0xB1007124, CStateUse )
REGISTER_SAVELOAD_CLASS( 0xB1007125, CStatePickItem )
REGISTER_SAVELOAD_CLASS( 0xB1007126, CStateRotate )
REGISTER_SAVELOAD_CLASS( 0xB1007127, CStateFirstAid )
REGISTER_SAVELOAD_CLASS( 0xB1007128, CStateSelection )
REGISTER_SAVELOAD_CLASS( 0xB1007119, CStateVictory )
