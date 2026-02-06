#include "StdAfx.h"
#include <limits>
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
#include "ConsoleStream.h"
#include "iMain.h"
#include "Interface.h"
#include "IDecorators.h"
#include "iMissionGame.h"
#include "iMissionUI.h"
#include "iUnitState.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"

#include "..\Misc\StrProc.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
const CVec4
	V_SELECTIONCOLOR_TEAM					= CVec4( 0.1f, 0.1f, 1, 1 ),
	V_SELECTIONCOLOR_TEAM_HILIGHT	= CVec4( 1, 1, 1, 0.5 ),
	V_SELECTIONCOLOR_ENEMY				= CVec4( 1, 0.1f, 0.1f, 1 ),
	V_SELECTIONCOLOR_CORPSE				= CVec4( 0, 0, 0, 1 ),
	V_SELECTIONCOLOR_OBJECT				= CVec4( 0.1f, 1, 0.1f, 1 );
////////////////////////////////////////////////////////////////////////////////////////////////////
// EUnitCommandResult -> Wide String
////////////////////////////////////////////////////////////////////////////////////////////////////
static wstring GetErrorString( NWorld::EUnitCommandResult eResult )
{
	switch( eResult )
	{
	case NWorld::UCR_NOT_ENOUGH_AP:
		return L"<color=beige>Not enough AP";
	case NWorld::UCR_PATH_NOT_FOUND:
		return L"<color=beige>Path not found";
	case NWorld::UCR_CRITICALS_BAN:
		return L"<color=beige>Action blocked by critical";
	case NWorld::UCR_NEED_RELOAD:
		return L"<color=beige>Need reload!";
	case NWorld::UCR_WEAPON_JAMMED:
		return L"<color=beige>Weapon jamed!";
	}

	return L"";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
IStateTracker* IStateTracker::Create( NGame::IMissionGame *pGame, CMissionUI *pMissionUI )
{
	CStateTracker* pState = new CStateTracker;
	pState->Initialize( pGame, pMissionUI );
	return pState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateTracker::CStateTracker(): 
	eType( UPDATED ), pStateCreator( 0 ), bUpdated( false ), bStateUpdateBlocked( false ), bRealTime( false ), bHasCommands( false ), bActionExecuted( false ), actionsInfoSet( UA_MAXVALUE ),
	bindExplode( "explode" ), bindCheatTeleport( "cheat_teleport" ),
	bindNextEnemy( "next_enemy" ), 
	bindCancel( "cancel" ),	bindMove( "move" ), bindUse( "use" ), bindAttack( "attack" ), bindFirstAid( "firstaid" ), bindDropCorpse( "dropcorpse" ), bindRotate( "unit_rotate" ), bindWeaponReload( "weapon_reload" ),
	bindNormalPose( "pose_normal" ), bindCrawlPose( "pose_crawl" ), bindCrouchPose( "pose_crouch" ), bindRunPose( "pose_run" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::Initialize( NGame::IMissionGame* _pGame, CMissionUI *_pInterface )
{
	pGame = _pGame;
	pInterface = _pInterface;
	pTileHilightPos = new NGScene::CCFBTransform;

	bUpdated = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::ShowTileHilight()
{
	SFBTransform fbPos;
	MakeMatrix( &fbPos, 0, 0, GetGame()->GetTracePosition().GetCP() );
	pTileHilightPos->Set( fbPos );

	pTileHilight = GetGame()->GetScene()->CreateMesh( NDb::GetModel( 82 ), pTileHilightPos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::HideTileHilight()
{
	pTileHilight = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CStateTracker::GetGroupState()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

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
IStateTracker::EType CStateTracker::GetStateType() const
{
	return eType;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CUnit::EState CStateTracker::GetGroupWorldState()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
		return NWorld::CUnit::ST_NORMAL_DEFAULT;

	return unitsSet.front()->GetUnit()->GetState();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const vector<SActionInfo>& CStateTracker::GetGroupActionsInfo() const
{
	return actionsInfoSet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::NewPose( NAI::EPose ePos )
{
	int nUnitAction = 0;
	switch( ePos )
	{
	case NAI::RUN:
		nUnitAction = UA_POSERUN;
		break;
	case NAI::WALK:
		nUnitAction = UA_POSEWALK;
		break;
	case NAI::CROUCH:
		nUnitAction = UA_POSECROUCH;
		break;
	case NAI::CRAWL:
		nUnitAction = UA_POSECRAWL;
		break;
	default:
		ASSERT( 0 );
		return;
	}

	const SActionInfo &sInfo = GetGroupActionsInfo()[nUnitAction];
	if ( !sInfo.bAvailable || !sInfo.bOk || !sInfo.bEnoughAP )
	{
		csGame << GetErrorString( sInfo.eResult ) << endl;
		return;
	}

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );
	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
	{
		NAI::SUnitPosition uPos = (*iTemp)->GetUnit()->GetPosition();
		uPos.SetPose( ePos );
		GetGame()->Command( (*iTemp)->GetUnit(), new NWorld::CCmdWishPose( ePos ) );
		GetGame()->Command( (*iTemp)->GetUnit(), new NWorld::CCmdPath( uPos.pos, NAI::PF_USE_POSEDIR ) );
			//GetGame()->Command( new NWorld::CCmdGo( (*iTemp)->GetUnit() ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::Update()
{
	TrackChanges();

	SFBTransform fbPos;
	MakeMatrix( &fbPos, 0, 0, GetGame()->GetTracePosition().GetCP() );
	pTileHilightPos->Set( fbPos );

	if ( IsSelectionUpdated() )
		ResetState();

	if ( IsUpdated() )
	{
		UpdateGroupActionsInfo();

		if ( !bStateUpdateBlocked )
			UpdateState();
	}
	else if ( !GetGame()->IsRealTime() && GetGame()->IsActionExecuted() )
		UpdateGroupActionsInfo();

	pState->Step();

	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::TrackChanges()
{
	bUpdated = false;
	bSelectionUpdated = false;

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );
	if ( unitsSet != selectedUnits )
	{
		selectedUnits = unitsSet;
		bUpdated = true;
		bSelectionUpdated = true;
	}

	bool bNewRealTime = GetGame()->IsRealTime();
	if ( bNewRealTime != bRealTime )
	{
		bUpdated = true;
		bRealTime = bNewRealTime;
	}

	bool bNewHasCommands = GetGame()->GetActivePlayer()->GetCommander()->HasCommands();
	if ( bNewHasCommands != bHasCommands )
	{
		bUpdated = true;
		bHasCommands = bNewHasCommands;
	}

	bool bNewActionExecuted = GetGame()->IsActionExecuted();
	if ( bNewActionExecuted != bActionExecuted )
	{
		bUpdated = true;
		bActionExecuted = bNewActionExecuted;
	}

	CPtr<CObjectBase> pNewTraceObject = GetGame()->GetTraceObject();
	if ( pNewTraceObject != pTraceObject )
	{
		bUpdated = true;
		pTraceObject = pNewTraceObject;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::UpdateState()
{
	typedef CObjectBase* (*TNewStateObject)();
	TNewStateObject states[] = 
	{
		CStateWait::NewCStateWait,
		CStateDropItem::NewCStateDropItem,
		CStateUse::NewCStateUse,
		CStateTeam::NewCStateTeam,
		CStateAttack::NewCStateAttack,
		CStatePickItem::NewCStatePickItem,
		CStateMove::NewCStateMove,
		CStateEmpty::NewCStateEmpty
	};

	if ( eType == FORCED )
	{
		if ( SetState( pStateCreator, eType ) )
			return;
	}

	for ( int nTemp = 0; nTemp < ARRAY_SIZE( states ); nTemp++ )
	{
		if ( SetState( states[nTemp], UPDATED ) )
			return;
	}

	ASSERT( 0 );
	SetState( CStateEmpty::NewCStateEmpty, UPDATED );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::UpdateGroupActionsInfo()
{
	actionsInfoSet[UA_STOP].eResult = NWorld::UCR_OK;
	actionsInfoSet[UA_STOP].nActionAP = 0;
	actionsInfoSet[UA_STOP].bOk = true;
	actionsInfoSet[UA_STOP].bEnoughAP = true;
	actionsInfoSet[UA_STOP].bAvailable = true;
	
	CanGroupDo( new NWorld::CCmdPath( NAI::SPosition(), NAI::PF_DEFAULT ), true, &actionsInfoSet[UA_MOVE] );
	CanGroupDo( new NWorld::CCmdPath( NAI::SPosition(), NAI::PF_USE_DIR ), true, &actionsInfoSet[UA_LOOK] );

	actionsInfoSet[UA_USE].eResult = NWorld::UCR_OK;
	actionsInfoSet[UA_USE].nActionAP = 0;
	actionsInfoSet[UA_USE].bOk = true;
	actionsInfoSet[UA_USE].bEnoughAP = true;
	actionsInfoSet[UA_USE].bAvailable = true;

	CanGroupDo( new NWorld::CCmdHeal( 0 ), true, &actionsInfoSet[UA_HEAL] );
	CanGroupDo( new NWorld::CCmdShootObject( 0, 0, 0 ), true, &actionsInfoSet[UA_ATTACK] );
	CanGroupDo( new NWorld::CCmdDropCorpse(), false, &actionsInfoSet[UA_DROPCORPSE] );
	CanGroupDo( new NWorld::CCmdReload(), false, &actionsInfoSet[UA_WEAPONRELOAD] );

	CanGroupDo( new NWorld::CCmdWishPose( NAI::RUN ), false, &actionsInfoSet[UA_POSERUN] );
	CanGroupDo( new NWorld::CCmdWishPose( NAI::WALK ), false, &actionsInfoSet[UA_POSEWALK] );
	CanGroupDo( new NWorld::CCmdWishPose( NAI::CROUCH ), false, &actionsInfoSet[UA_POSECROUCH] );
	CanGroupDo( new NWorld::CCmdWishPose( NAI::CRAWL ), false, &actionsInfoSet[UA_POSECRAWL] );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::CanGroupDo( NWorld::CCmd *pCmd, bool bNoTarget, SActionInfo *pInfo )
{
	pInfo->eResult = CanGroupDo( pCmd, bNoTarget , &pInfo->nActionAP );

	pInfo->bOk = false;
	pInfo->bEnoughAP = true;
	pInfo->bAvailable = false;
	if ( ( pInfo->eResult == NWorld::UCR_OK ) || ( pInfo->eResult == NWorld::UCR_NOT_ENOUGH_AP ) )
		pInfo->bOk = true;
	if ( pInfo->eResult == NWorld::UCR_NOT_ENOUGH_AP )
		pInfo->bEnoughAP = false;
	if ( pInfo->eResult != NWorld::UCR_UNAVAILABLE )
		pInfo->bAvailable = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::EUnitCommandResult CStateTracker::CanGroupDo( NWorld::CCmd *pCmd, bool bNoTarget, int *pnAP )
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() == 0 )
		return NWorld::UCR_GENERAL_FAILURE;

	CObj<NWorld::CCmd> pHolder( pCmd );
	NWorld::EUnitCommandResult eTotalRes = NWorld::UCR_OK;
	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
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
			else if ( CDynamicCast<NWorld::CCmdHeal> pHeal( pCmd ) )
				pHeal->pTarget = pUnit;
			else if ( CDynamicCast<NWorld::CCmdShootObject> pShoot( pCmd ) )
				pShoot->pTarget = pUnit;
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
bool CStateTracker::SetState( TStateObjectCreate _pStateCreator, EType _eType )
{
	ASSERT( _pStateCreator );

	CPtr<CObjectBase> pObject = _pStateCreator();
	if ( !IsValid( pObject ) )
		return false;
	CStateBase* _pState = dynamic_cast<CStateBase*>( pObject.GetPtr() );
	if ( !IsValid( _pState ) )
		return false;

	if ( !_pState->Initialize( this, _eType ) )
		return false;

	eType = _eType;
	pState = _pState;
	pStateCreator = _pStateCreator;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTracker::ResetState()
{
	SetState( CStateEmpty::NewCStateEmpty, UPDATED );
	UpdateState();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateTracker::ProcessEvent( const NInput::SEvent &sEvent )
{
	if ( bindCancel.ProcessEvent( sEvent ) )
	{
		if ( eType == FORCED )
			ResetState();
		else if ( GetGame()->IsActionExecuted() )
			GetGame()->StopAction();
		else
		{
			vector< CPtr<NGame::IUnitTracker> > unitsSet;
			GetGame()->GetSelectedUnits( &unitsSet );

			for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
				GetGame()->Command( new NWorld::CCmdCancel( (*iTemp)->GetUnit() ) );
		}

		return true;
	}

	if ( !GetGame()->IsReady() )
		return false;

	//[CRAP BEGIN]
	if ( bindExplode.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );
		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
				GetGame()->Command( (*iTemp)->GetUnit(), new NWorld::CCmdExplode );
		return true;
	}
	//[CRAP END]

	/// Menu actions
	if ( bindMove.ProcessEvent( sEvent ) )
	{
		if ( GetGame()->CountSelected() == 0 )
			return true;

		if ( actionsInfoSet[UA_MOVE].bOk )
			SetState( CStateMove::NewCStateMove, FORCED );
		else
			csGame << GetErrorString( actionsInfoSet[UA_MOVE].eResult ) << endl;

		return true;
	}
	else if ( bindRotate.ProcessEvent( sEvent ) )
	{
		if ( GetGame()->CountSelected() == 0 )
			return true;

		if ( actionsInfoSet[UA_LOOK].bOk )
			SetState( CStateRotate::NewCStateRotate, FORCED );
		else
			csGame << GetErrorString( actionsInfoSet[UA_LOOK].eResult ) << endl;

		return true;
	}
	else if ( bindUse.ProcessEvent( sEvent ) )
	{
		if ( GetGame()->CountSelected() == 0 )
			return true;

		if ( actionsInfoSet[UA_USE].bOk )
			SetState( CStateUse::NewCStateUse, FORCED );
		else
			csGame << GetErrorString( actionsInfoSet[UA_USE].eResult ) << endl;

		return true;
	}
	else if ( bindAttack.ProcessEvent( sEvent ) )
	{
		if ( GetGame()->CountSelected() == 0 )
			return true;

		if ( actionsInfoSet[UA_ATTACK].bOk )
			SetState( CStateAttack::NewCStateAttack, FORCED );
		else
			csGame << GetErrorString( actionsInfoSet[UA_ATTACK].eResult ) << endl;

		return true;
	}
	else if ( bindFirstAid.ProcessEvent( sEvent ) )
	{
		if ( GetGame()->CountSelected() == 0 )
			return true;

		if ( actionsInfoSet[UA_HEAL].bOk )
			SetState( CStateFirstAid::NewCStateFirstAid, FORCED );
		else
			csGame << GetErrorString( actionsInfoSet[UA_HEAL].eResult ) << endl;

		return true;
	}
	else if ( bindDropCorpse.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );
		if ( unitsSet.empty() )
			return true;

		const SActionInfo &sInfo = actionsInfoSet[UA_DROPCORPSE];
		if ( sInfo.bAvailable && sInfo.bOk && sInfo.bEnoughAP )
		{
			for ( int nTemp = 0; nTemp < unitsSet.size(); nTemp++ )
				GetGame()->Command( unitsSet[nTemp]->GetUnit(), new NWorld::CCmdDropCorpse );
		}
		else
			csGame << GetErrorString( actionsInfoSet[UA_DROPCORPSE].eResult ) << endl;

		return true;
	}
	else if ( bindWeaponReload.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );
		if ( unitsSet.empty() )
			return true;

		const SActionInfo &sInfo = actionsInfoSet[UA_WEAPONRELOAD];
		if ( sInfo.bAvailable && sInfo.bOk && sInfo.bEnoughAP )
		{
			for ( int nTemp = 0; nTemp < unitsSet.size(); nTemp++ )
				GetGame()->Command( unitsSet[nTemp]->GetUnit(), new NWorld::CCmdReload );
		}
		else
			csGame << GetErrorString( actionsInfoSet[UA_WEAPONRELOAD].eResult ) << endl;

		return true;
	}

	if ( bindNormalPose.ProcessEvent( sEvent ) )
		NewPose( NAI::WALK );
	else if ( bindCrawlPose.ProcessEvent( sEvent ) )
		NewPose( NAI::CRAWL );
	else if ( bindCrouchPose.ProcessEvent( sEvent ) )
		NewPose( NAI::CROUCH );
	else if ( bindRunPose.ProcessEvent( sEvent ) )
		NewPose( NAI::RUN );

	if ( bindNextEnemy.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );
		if ( unitsSet.size() == 1 )
		{
			CPtr<NWorld::CUnit> pEnemy = unitsSet[0]->GetNextEnemy();
			if ( IsValid( pEnemy ) )
				GetGame()->FocusCameraOnUnit( pEnemy );
		}
	}

	if ( bindCheatTeleport.ProcessEvent( sEvent ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );
		if ( unitsSet.size() == 1 )
		{
			NAI::SUnitPosition sPosition = unitsSet[0]->GetUnit()->GetPosition();
			sPosition.pos = GetGame()->GetTracePosition();
			GetGame()->Command( unitsSet[0]->GetUnit(), new NWorld::CCmdTeleport( sPosition ) );
		}

		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateTracker::ProcessMessage( const SEvent &sEvent )
{
	if ( !GetGame()->IsReady() )
		return false;

	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONUP:
		pState->OnLButtonUp();
		return true;
	case EVENT_LBUTTONDOWN:
		pState->OnLButtonDown();
		return true;
	case EVENT_LBUTTONDBLCLK:
		pState->OnLButtonDblClk();
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateBase
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateBase::Initialize( CStateTracker* _pTracker, IStateTracker::EType eType )
{
	pTracker = _pTracker;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CMissionUI* CStateBase::GetInterface() const
{
	return pTracker->GetInterface();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateTracker* CStateBase::GetTracker() const
{
	return pTracker;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NGame::IMissionGame* CStateBase::GetGame() const
{
	return pTracker->GetGame();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateEmpty
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateEmpty::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	GetInterface()->SetCursorType( N_CURSOR_BLOCK );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateWait
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateWait::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	if ( GetGame()->IsRealTime() || !GetGame()->IsActionExecuted() )
		return false;

	GetInterface()->SetCursorType( N_CURSOR_BUSY );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateTeam
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateTeam::CStateTeam(): 
	bindModifier( "modifier" ), bModifier( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateTeam::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	NWorld::IVisObj* pObject = GetGame()->GetTraceObject();
	if ( !IsValid( pObject ) )
		return false;
	if ( CDynamicCast<NWorld::CUnit> pUnit( pObject ) )
	{
		if ( pUnit->GetPlayer() != GetGame()->GetActivePlayer()->GetPlayer() )
			return false;

		GetTracker()->ShowTileHilight();
		pTraceSelection = GetGame()->GetRenderGame()->Select( pUnit, V_SELECTIONCOLOR_TEAM );
		GetInterface()->SetCursorType( N_CURSOR_NORMAL );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTeam::OnLButtonUp()
{
	CObjectBase* pObject = GetGame()->GetTraceObject();
	if ( !IsValid( pObject ) )
		return;

	if ( CDynamicCast<NWorld::CUnit> pUnit( pObject ) )
		GetGame()->Select( pUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTeam::Step()
{
	CStateBase::Step();
	bModifier = bindModifier.IsActive();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateMove
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateMove::CStateMove(): 
	bindStrafe( "pose_strafe" ), bSelection( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateMove::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	if ( eType == CStateTracker::FORCED )
	{
		const SActionInfo &sInfo = GetTracker()->GetGroupActionsInfo()[UA_MOVE];
		if ( !sInfo.bAvailable || !sInfo.bOk || !sInfo.bEnoughAP )
		{
			csGame << GetErrorString( sInfo.eResult ) << endl;
			return false;
		}
	}

	pSelectionZone = IImage::Create( GetInterface()->GetClientWindow(), SRect( 0, 0, 0, 0 ), "selectionzone", STYLE_ENABLED | STYLE_TOPMOST | STYLE_TRANSPARENT );
	pSelectionZone->SetColor( NGfx::SPixel8888( 0, 0, 0, 0x7F ) );

	GetTracker()->ShowTileHilight();
	GetInterface()->SetCursorType( N_CURSOR_NORMAL );
	UpdateCursor();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateMove::OnLButtonUp()
{
	if ( !bSelection )
		return;

	bSelection = false;
	GetGame()->FreezeCamera( false );
	GetTracker()->SetStateUpdateBlock( false );

	IStateTracker::EType eType = GetTracker()->GetStateType();

	CVec2 vCursorPos = GetGame()->GetCursor()->GetPos();
	if ( ( fabs2( vSelectionAnchor - vCursorPos ) < FP_EPSILON2 ) || ( eType == CStateTracker::FORCED ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );

		vector< NAI::SPosition > unitPlaces;
		NAI::SPosition pos = GetGame()->GetTracePosition();
		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
		{
			NAI::SPosition posUnit = (*iTemp)->GetUnit()->GetPosition().pos;
			unitPlaces.push_back( posUnit );
		}

		GetGame()->GetWorld()->GetPathNetwork()->FormationMoveTo( &unitPlaces, pos  );

		for ( int nUnit = 0; nUnit < unitsSet.size(); ++nUnit )
			unitsSet[nUnit]->SetTargetPosition( unitPlaces[nUnit], eType == CStateTracker::FORCED, bindStrafe.IsActive() );

		if ( eType == CStateTracker::FORCED )
			GetTracker()->ResetState();
	}
	else
	{
		bool bKeepSelection = false;
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetUnits( &unitsSet );

		SRect sRect( Min( vSelectionAnchor.x, vCursorPos.x ), Min( vSelectionAnchor.y, vCursorPos.y ), 
											Max( vSelectionAnchor.x, vCursorPos.x ), Max( vSelectionAnchor.y, vCursorPos.y ) );
		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
		{
			CVec2 vScreenPos;
			CTransformStack sTS = GetGame()->GetCameraTransform();
			if ( TestRayInFrustrum( (*iTemp)->GetUnit()->GetPosition().GetCP(), &sTS, GetGame()->GetScene()->GetScreenRect(), &vScreenPos ) )
			{
				if ( ( vScreenPos.x > sRect.x1 ) && ( vScreenPos.x < sRect.x2 ) && ( vScreenPos.y > sRect.y1 ) && ( vScreenPos.y < sRect.y2 ) )
				{
					GetGame()->Select( (*iTemp)->GetUnit(), bKeepSelection );
					bKeepSelection = true;
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateMove::OnLButtonDown()
{
	bSelection = true;
	GetGame()->FreezeCamera( true );
	GetTracker()->SetStateUpdateBlock( true );
	vSelectionAnchor = GetGame()->GetCursor()->GetPos();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateMove::OnLButtonDblClk()
{
	bSelection = false;
	GetGame()->FreezeCamera( false );
	GetTracker()->SetStateUpdateBlock( false );

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	vector< NAI::SPosition > unitPlaces;
	NAI::SPosition pos = GetGame()->GetTracePosition();
	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
	{
		NAI::SPosition posUnit = (*iTemp)->GetUnit()->GetPosition().pos;
		unitPlaces.push_back( posUnit );
	}

	GetGame()->GetWorld()->GetPathNetwork()->FormationMoveTo( &unitPlaces, pos  );

	for ( int nUnit = 0; nUnit < unitsSet.size(); ++nUnit )
		unitsSet[nUnit]->SetTargetPosition( unitPlaces[nUnit], true, bindStrafe.IsActive() );

	if ( GetTracker()->GetStateType() == CStateTracker::FORCED )
		GetTracker()->ResetState();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateMove::UpdateCursor()
{
	if ( bSelection )
	{
		GetInterface()->SetCursorType( N_CURSOR_NORMAL );
		return;
	}
	NAI::SPathPlace p( GetGame()->GetTracePosition().p );
	p.SetPose( NAI::CM_STAND );
	if ( GetGame()->GetWorld()->GetPathNetwork()->IsPassable( p ) )
		GetInterface()->SetCursorType( N_CURSOR_MOVE );
	else
		GetInterface()->SetCursorType( N_CURSOR_BLOCK );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateMove::Step()
{
	CStateBase::Step();

	UpdateCursor();

	pSelectionZone->SetStyle( STYLE_VISIBLE, bSelection );
	if ( bSelection )
	{
		CVec2 vCursorPos = GetGame()->GetCursor()->GetPos();
		CVec2 vScreenRect = GetGame()->GetScene()->GetScreenRect();

		SRect sRect( vSelectionAnchor.x * 1024 / vScreenRect.x, vSelectionAnchor.y * 768 / vScreenRect.y, vCursorPos.x * 1024 / vScreenRect.x, vCursorPos.y * 768 / vScreenRect.y );

		SPoint sPosition;
		pSelectionZone->GetParent()->ScreenToClient( SPoint( Min( sRect.x1, sRect.x2 ), Min( sRect.y1, sRect.y2 ) ), &sPosition );
		pSelectionZone->SetPosition( sPosition );
		pSelectionZone->SetSize( SPoint( abs( sRect.Width() ), abs( sRect.Height() ) ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateRotate
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateRotate::CStateRotate()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateRotate::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	GetInterface()->SetCursorType( N_CURSOR_NORMAL );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateRotate::OnLButtonUp()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
	{
		NAI::EDirection eDir = GetGame()->GetWorld()->GetPathNetwork()->GetClosestDir( (*iTemp)->GetUnit()->GetPosition().pos.p, GetGame()->GetTracePosition().p );	
		NAI::SPosition sPos = (*iTemp)->GetUnit()->GetPosition().pos;
		sPos.p.SetDirection( eDir );
		GetGame()->Command( (*iTemp)->GetUnit(), new NWorld::CCmdPath( sPos, NAI::PF_USE_DIR ) );
	}

	if ( GetTracker()->GetStateType() == CStateTracker::FORCED )
		GetTracker()->ResetState();
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
bool CStateAttack::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	if ( eType == CStateTracker::FORCED )
	{
		const SActionInfo &sInfo = GetTracker()->GetGroupActionsInfo()[UA_ATTACK];
		if ( !sInfo.bAvailable || !sInfo.bOk || !sInfo.bEnoughAP )
		{
			csGame << GetErrorString( sInfo.eResult ) << endl;
			return false;
		}
	}

	if ( eType != CStateTracker::FORCED )
	{
		NWorld::IVisObj* pObject = GetGame()->GetTraceObject();
		if ( !IsValid( pObject ) )
			return false;

		NWorld::CUnit* pUnit = dynamic_cast<NWorld::CUnit*>( pObject );
		if ( !IsValid( pUnit ) )
			return false;

		if ( pUnit->GetPlayer() == GetGame()->GetActivePlayer()->GetPlayer() )
			return false;
	}

	GetTracker()->ShowTileHilight();
	UpdateTraceSelection();
	UpdateBlockedState();
	UpdateCursorInfo();
	UpdateCursor();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::OnLButtonUp()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	NWorld::IVisObj* pObject = GetGame()->GetTraceObject();

	if ( IsValid( pObject ) )
	{
		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
			GetGame()->Command( (*iTemp)->GetUnit(), new NWorld::CCmdShootObject( pObject, 0, 0, eHitLocation ) );
	}
	else
	{
		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
			GetGame()->Command( (*iTemp)->GetUnit(), new NWorld::CCmdShootTile( GetGame()->GetTracePosition().GetCP() ) );
	}

	if ( GetTracker()->GetStateType() == CStateTracker::FORCED )
		GetTracker()->ResetState();
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

				GetInterface()->SetCursorType( nDefault, wsCursorText );
			}
			break;
		case NAI::HL_HEAD:
			GetInterface()->SetCursorType( N_CURSOR_ATTACK_HEAD, wsCursorText );
			break;
		case NAI::HL_BODY:
			GetInterface()->SetCursorType( N_CURSOR_ATTACK_BODY, wsCursorText );
			break;
		case NAI::HL_LHAND:
			GetInterface()->SetCursorType( N_CURSOR_ATTACK_LARM, wsCursorText );
			break;
		case NAI::HL_RHAND:
			GetInterface()->SetCursorType( N_CURSOR_ATTACK_RARM, wsCursorText );
			break;
		case NAI::HL_LLEG:
			GetInterface()->SetCursorType( N_CURSOR_ATTACK_LLEG, wsCursorText );
			break;
		case NAI::HL_RLEG:
			GetInterface()->SetCursorType( N_CURSOR_ATTACK_RLEG, wsCursorText );
			break;
		default:
			ASSERT( 0 );
		}
	}
	else
	{
		wsCursorText = L"";
		GetInterface()->SetCursorType( N_CURSOR_BLOCK );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::UpdateCursorInfo()
{
	wsCursorText = L"";

	int nSelectedCount = GetGame()->CountSelected();
	if ( nSelectedCount )
	{
		CPtr<NWorld::IVisObj> pTraceObject = GetGame()->GetTraceObject();

		int nTotalShootAP = 0;
		int nMin = 0x7fffffff, nMax = -0x7fffffff; // numeric_limits<int>::max(), numeric_limits<int>::min();
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );
		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
		{
			CPtr<NWorld::IWorld> pWorld = GetGame()->GetWorld();

			int nToHit = 0;

			if ( CDynamicCast<NWorld::CUnit> pUnit( pTraceObject ) )
			{
				if ( CDynamicCast<NRPG::IWeaponItemInfo> pWeapon( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( !pWeapon->GetDBWeapon()->bBazookaLogic )
						nToHit = pWorld->GetGame()->GetCompositeToHit( (*iTemp)->GetUnit(), pUnit, 0, eHitLocation, pWorld->IsFirstTurn() );
					else
						nToHit = pWorld->GetGame()->GetBazookaToHit( (*iTemp)->GetUnit(), GetGame()->GetTracePosition().GetCP(),
							NAI::THL_MIDDLE, pWorld->IsFirstTurn() );
				}
				else if ( CDynamicCast<NRPG::IGrenadeItem> pGrenade( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
					nToHit = pWorld->GetGame()->GetGrenadeCompositeToHit( (*iTemp)->GetUnit(), pUnit->GetPosition().GetCP(), pWorld->IsFirstTurn(), pGrenade->GetDBGrenade() );
				else if ( CDynamicCast<NRPG::IMeleeWeaponItem> pMelee( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( pMelee->GetDBMeleeWeapon()->bThrowing )
						nToHit = pWorld->GetGame()->GetCompositeToHit( (*iTemp)->GetUnit(), pUnit, 0, eHitLocation, pWorld->IsFirstTurn() );	
				}
			}
			else
			{
				if ( CDynamicCast<NRPG::IGrenadeItem> pGrenade( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
					nToHit = pWorld->GetGame()->GetGrenadeCompositeToHit( (*iTemp)->GetUnit(), GetGame()->GetTracePosition().GetCP(), pWorld->IsFirstTurn(), pGrenade->GetDBGrenade() );
				else	if ( CDynamicCast<NRPG::IWeaponItemInfo> pWeapon( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( !pWeapon->GetDBWeapon()->bBazookaLogic )
						nToHit = pWorld->GetGame()->GetTileCompositeToHit( (*iTemp)->GetUnit(), GetGame()->GetTracePosition().GetCP(), NAI::THL_MIDDLE, pWorld->IsFirstTurn() );
					else
						nToHit = pWorld->GetGame()->GetBazookaToHit( (*iTemp)->GetUnit(), GetGame()->GetTracePosition().GetCP(),
							NAI::THL_MIDDLE, pWorld->IsFirstTurn() );
				}
				else if ( CDynamicCast<NRPG::IMeleeWeaponItem> pMelee( (*iTemp)->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive() ) )
				{
					if ( pMelee->GetDBMeleeWeapon()->bThrowing )
						nToHit = pWorld->GetGame()->GetTileCompositeToHit( (*iTemp)->GetUnit(), GetGame()->GetTracePosition().GetCP(), NAI::THL_MIDDLE, pWorld->IsFirstTurn() );	
				}
			}

			nMin = min( nMin, nToHit );
			nMax = max( nMax, nToHit );

			WCHAR wsString[256];
			if ( unitsSet.size() == 1 )
				swprintf( wsString, L"<normal>%2d%%<br>AP: %d", nMin, nActionAP );
			else
				swprintf( wsString, L"<normal>%2d-%2d%%", nMin, nMax );

			wsCursorText = wsString;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::UpdateBlockedState()
{
	CPtr<NWorld::IVisObj> pTraceObject = GetGame()->GetTraceObject();

	bActionUnavailable = false;
	nActionAP = 0;
	if ( IsValid( pTraceObject ) && ( GetTracker()->CanGroupDo( new NWorld::CCmdShootObject( pTraceObject, 0, 0, eHitLocation ), false, &nActionAP ) != NWorld::UCR_OK ) )
		bActionUnavailable = true;
	else if ( !IsValid( pTraceObject ) && ( GetTracker()->CanGroupDo( new NWorld::CCmdShootTile( GetGame()->GetTracePosition().GetCP() ), false, &nActionAP ) != NWorld::UCR_OK ) )
		bActionUnavailable = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::UpdateTraceSelection()
{
	pTraceSelection = 0;

	NWorld::IVisObj* pObject = GetGame()->GetTraceObject();
	if ( !IsValid( pObject ) )
		return;

	NWorld::CUnit* pUnit = dynamic_cast<NWorld::CUnit*>( pObject );
	if ( !IsValid( pUnit ) )
		return;

	if ( pUnit->GetPlayer() == GetGame()->GetActivePlayer()->GetPlayer() )
		return;

	pTraceSelection = GetGame()->GetRenderGame()->Select( pUnit, V_SELECTIONCOLOR_ENEMY );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateAttack::Step()
{
	CStateBase::Step();

	if ( GetTracker()->GetStateType() == CStateTracker::FORCED )
	{
		UpdateBlockedState();
		UpdateCursorInfo();
	}

	UpdateCursor();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateUse
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateUse::CStateUse()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateUse::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	if ( eType == CStateTracker::FORCED )
	{
		const SActionInfo &sInfo = GetTracker()->GetGroupActionsInfo()[UA_USE];
		if ( !sInfo.bAvailable || !sInfo.bOk || !sInfo.bEnoughAP )
			return false;
	}

	NWorld::IVisObj* pObject = GetGame()->GetTraceObject();
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
		GetTracker()->ShowTileHilight();
	else
		GetTracker()->HideTileHilight();

	pTraceSelection = GetGame()->GetRenderGame()->Select( pObject, vHilightColor );

	int nActionAP = 0;
	NWorld::EUnitCommandResult eResult = NWorld::UCR_UNAVAILABLE;
	CObj<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( IsValid( pCmd ) )
		eResult = GetTracker()->CanGroupDo( GetTargetCmd(), false, &nActionAP );

	if ( eResult == NWorld::UCR_OK )
	{
		WCHAR wsBuffer[1024];
		swprintf( wsBuffer, L"%d", nActionAP );
		GetInterface()->SetCursorType( N_CURSOR_OPEN_CLOSE, wsBuffer );
	}
	else
		GetInterface()->SetCursorType( N_CURSOR_BLOCK );

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateUse::OnLButtonUp()
{
	CObjectBase* pObject = GetGame()->GetTraceObject();
	if ( !IsValid( pObject ) )
		return;

	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() < 1 )
		return;

	CObj<NWorld::CCmd> pCmd = GetTargetCmd();
	if ( IsValid( pCmd ) )
		GetGame()->Command( unitsSet.front()->GetUnit(), GetTargetCmd() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCmd* CStateUse::GetTargetCmd()
{
	CObjectBase* pObject = GetGame()->GetTraceObject();
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
// CStateFirstAid
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateFirstAid::CStateFirstAid()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateFirstAid::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	if ( eType == CStateTracker::FORCED )
	{
		const SActionInfo &sInfo = GetTracker()->GetGroupActionsInfo()[UA_HEAL];
		if ( !sInfo.bAvailable || !sInfo.bOk || !sInfo.bEnoughAP )
		{
			csGame << GetErrorString( sInfo.eResult ) << endl;
			return false;
		}
	}

	if ( eType != CStateTracker::FORCED )
	{
		NWorld::IVisObj* pObject = GetGame()->GetTraceObject();
		if ( !IsValid( pObject ) )
			return false;

		NWorld::CUnit* pUnit = dynamic_cast<NWorld::CUnit*>( pObject );
		if ( !IsValid( pUnit ) )
			return false;

		if ( pUnit->GetPlayer() != GetGame()->GetActivePlayer()->GetPlayer() )
			return false;
	}

	GetTracker()->ShowTileHilight();

	int nActionAP = 0;
	CPtr<NWorld::CUnit> pTarget = GetTarget();
	NWorld::EUnitCommandResult eResult = NWorld::UCR_UNAVAILABLE;
	if ( IsValid( pTarget ) )
		eResult = GetTracker()->CanGroupDo( new NWorld::CCmdHeal( pTarget ), false, &nActionAP );

	if ( eResult == NWorld::UCR_OK)
	{
		WCHAR wsBuffer[1024];
		swprintf( wsBuffer, L"%d", nActionAP );
		GetInterface()->SetCursorType( N_CURSOR_HEAL, wsBuffer );
	}
	else
		GetInterface()->SetCursorType( N_CURSOR_BLOCK );

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateFirstAid::OnLButtonUp()
{
	CPtr<NWorld::CUnit> pTarget = GetTarget();
	if ( IsValid( pTarget ) )
	{
		vector< CPtr<NGame::IUnitTracker> > unitsSet;
		GetGame()->GetSelectedUnits( &unitsSet );

		for ( vector< CPtr<NGame::IUnitTracker> >::iterator iTemp = unitsSet.begin(); iTemp != unitsSet.end(); iTemp++ )
			GetGame()->Command( (*iTemp)->GetUnit(), new NWorld::CCmdHeal( pTarget ) );

		if ( GetTracker()->GetStateType() == CStateTracker::FORCED )
			GetTracker()->ResetState();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CUnit* CStateFirstAid::GetTarget()
{
	NWorld::IVisObj* pObject = GetGame()->GetTraceObject();
	if ( !IsValid( pObject ) )
		return 0;

	NWorld::CUnit* pUnit = dynamic_cast<NWorld::CUnit*>( pObject );
	if ( !IsValid( pUnit ) )
		return 0;

	if ( pUnit->GetPlayer() != GetGame()->GetActivePlayer()->GetPlayer() )
		return 0;

	return pUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatePickItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CStatePickItem::CStatePickItem()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStatePickItem::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	NWorld::IVisObj* pObject = GetGame()->GetTraceObject();
	if ( !IsValid( pObject ) )
		return false;

	if ( CDynamicCast<NWorld::IItem> pItem( pObject ) )
	{
		if ( !IsValid( pItem->GetInvItem() ) )
			return false;

		GetTracker()->HideTileHilight();
		pTraceSelection = GetGame()->GetRenderGame()->Select( pObject, V_SELECTIONCOLOR_OBJECT );
		GetInterface()->SetCursorType( N_CURSOR_OPEN_CLOSE );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatePickItem::OnLButtonUp()
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	GetGame()->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() < 1 )
		return;

	CPtr<CObjectBase> pObject = GetGame()->GetTraceObject();

	if ( CDynamicCast<NWorld::IItem> pTempItem( pObject ) )
	{
		const SInterfaceState &sState = GetInterface()->GetInterfaceState();

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
			GetGame()->Command( unitsSet[0]->GetUnit(), new NWorld::CCmdMoveInventoryItem( sSource, sTarget ) );
		}
		else
			GetGame()->Command( unitsSet[0]->GetUnit(), new NWorld::CCmdMoveInventoryItem( sSource, NWorld::SItem( NWorld::SItem::HAND ) ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateDropItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateDropItem::CStateDropItem()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateDropItem::Initialize( CStateTracker* pTracker, IStateTracker::EType eType )
{
	CStateBase::Initialize( pTracker, eType );

	if ( !GetGame()->IsRealTime() && ( GetGame()->GetActivePlayer()->GetPlayer() != GetGame()->GetWorld()->GetCurrentPlayer() ) )
		return false;
	if ( !IsValid( GetGame()->GetActivePlayer()->GetPlayer()->GetInHandItem() ) )
		return false;

	CPtr<NWorld::CPlayerItem> pItem = GetGame()->GetActivePlayer()->GetPlayer()->GetInHandItem();
	NUI::SPoint sCellSize( 29, 29 );

	const NUI::SPoint &sInventoryItemSize = pItem->GetItem()->GetSize();
	NUI::SPoint sItemSize( sCellSize.x * sInventoryItemSize.x - 1, sCellSize.y * sInventoryItemSize.y - 1 );

	CPtr<NDb::CRPGItem> pRPGItem( pItem->GetItem()->GetDBItem() );
	pModel = NUI::IModel::Create( GetInterface(), SRect( 0, 0, sItemSize.x, sItemSize.y ), "icon", STYLE_ENABLED | STYLE_VISIBLE | STYLE_TOPMOST | STYLE_TRANSPARENT );
	if ( pRPGItem->pModel )
	{
		const NDb::SCameraParams &sCamera = pRPGItem->sCameras[NDb::CAMERA_NORMAL];

		SRand sRnd;
		CVec3 vForwardDir;
		CQuat q = CQuat( sCamera.fYaw, V3_AXIS_Z ) * CQuat( sCamera.fPitch, V3_AXIS_X );
		q.GetYAxis( &vForwardDir );

		CVec3 vCP( sCamera.vAnchor - vForwardDir * sCamera.fDistance );
		SFBTransform res;
		MakeMatrix( &res, sCamera.fPitch, sCamera.fYaw, vCP );

		pModel->SetModel( pRPGItem->pModel->CreateModel( &sRnd ) );
		pModel->SetTransform( new CFBTransform( res ) );
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateDropItem::Step()
{
	CVec2 vScreenRect = GetGame()->GetScene()->GetScreenRect();
	const NUI::SPoint &sSize = pModel->GetSize();

	CVec2 vCursorPos = GetGame()->GetCursor()->GetPos();
	vCursorPos.x = vCursorPos.x * 1024 / vScreenRect.x;
	vCursorPos.y = vCursorPos.y * 768 / vScreenRect.y;

	NUI::SPoint sPosition( vCursorPos.x - sSize.x / 2, vCursorPos.y - sSize.y / 2 );
	pModel->SetPosition( sPosition );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateDropItem::OnLButtonUp()
{
	CPtr<NWorld::CPlayerItem> pItem = GetGame()->GetActivePlayer()->GetPlayer()->GetInHandItem();
	if ( !IsValid( pItem ) )
		return;

	NWorld::SItem sTarget;
	sTarget.eType = NWorld::SItem::GROUND;
	GetGame()->Command( pItem->GetSource().pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::SItem( NWorld::SItem::HAND ), sTarget ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB9221220, CStateTracker )
REGISTER_SAVELOAD_CLASS( 0xB9221221, CStateBase )
REGISTER_SAVELOAD_CLASS( 0xB9221222, CStateEmpty )
REGISTER_SAVELOAD_CLASS( 0xB9221223, CStateWait )
REGISTER_SAVELOAD_CLASS( 0xB9221224, CStateTeam )
REGISTER_SAVELOAD_CLASS( 0xB9221225, CStateMove )
REGISTER_SAVELOAD_CLASS( 0xB9221226, CStateAttack )
REGISTER_SAVELOAD_CLASS( 0xB9221227, CStateUse )
REGISTER_SAVELOAD_CLASS( 0xB9221228, CStateFirstAid )
REGISTER_SAVELOAD_CLASS( 0xB9221229, CStateRotate )
REGISTER_SAVELOAD_CLASS( 0xB922122A, CStatePickItem )
REGISTER_SAVELOAD_CLASS( 0xB922122B, CStateDropItem )
