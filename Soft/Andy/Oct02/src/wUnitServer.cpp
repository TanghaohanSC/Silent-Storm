#include "StdAfx.h"
#include "wUnitServer.h"
#include "RPGItem.h"
#include "RPGUnitMission.h"
#include "RPGGame.h"
#include "wObject.h"
#include "wMain.h"
#include "wAckBase.h"
#include "wUnitStates.h"
#include "wUnitMove.h"
#include "wUnitAttack.h"
#include "..\DBFormat\DataAI.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\Misc\RandomGen.h"
#include "RPGUnit.h"
#include "aiPath.h"
#include "aiSignal.h"
#include "scScenarioTracker.h"
#include "RPGGlobal.h"
#include "RPGDiplomacy.h"

namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCommandExecute
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCommandExecute::StartAction( CWorld *pWorld, EActionType actionType )
{
	switch ( actionType )
	{
		case NORMAL:
			pAction = pWorld->GetActiveCounter();
			break;
		case SKIPPABLE:
			pAction = pWorld->GetSkippableCounter();
			break;
		case NOBLOCK:
			pAction = new CActionCounter;
			break;
		default:
			ASSERT( 0 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPathViewer
////////////////////////////////////////////////////////////////////////////////////////////////////
CPathViewer::CPathViewer( CUnitServer *_pUS ):
	pUS( _pUS )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPathViewer::SetPath( NAI::CPath *pPath )
{
	if ( IsValid( pMove ) )
	{
		IExecMove *pExec = dynamic_cast<IExecMove*>( pMove.GetPtr() );
		pExec->SetNewPath( pPath, NAI::PF_DEFAULT );
	}
	else
		pMove = CreateSimpleMoveExecutor( pUS, pPath, NAI::PF_DEFAULT );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CPathViewer::GetResult() const
{
	if ( !IsValid( pMove ) )
	{
		ASSERT( 0 );
		return 0;
	}

	return pMove->GetActionAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPathViewer::GetPoints( vector<SPathPoint> *pRes )
{
	if ( !IsValid( pMove ) )
	{
		ASSERT( 0 );
		return;
	}

	IExecMove *pExec = dynamic_cast<IExecMove*>( pMove.GetPtr() );
	list<IExecMove::SPathPoint> pointsList;
	pExec->GetPathPoints( &pointsList );

	pRes->resize( pointsList.size() );

	int nCount = 0;
	for ( list<IExecMove::SPathPoint>::const_iterator iTemp = pointsList.begin(); iTemp != pointsList.end(); iTemp++ )
	{
		(*pRes)[nCount] = SPathPoint( iTemp->nAP, iTemp->nFloor, iTemp->vPoint );
		nCount++;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitServer
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer::CUnitServer( CWorld *pWorld, NRPG::IUnitMission *_pRPG, NDb::CModel *pModel, 
	CPlayer *_pPlayer, const NAI::SUnitPosition &pos )
	:CDumbUnitServer( pWorld, _pRPG, pModel, pos )
{
	pPlayer = _pPlayer;
	bCallTimeLabel = false;
	SetState( new CUnitStateNormal( this ) );
	bIsRunningForcedAction = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::OnSuffersDamage( float fAP )
{
	if ( fAP > 0.2f )
		GetWorld()->GetGlobalAck()->OnSuffersLightDamage( this );
	else
		GetWorld()->GetGlobalAck()->OnSuffersHardDamage( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::OnUnitMadeUnconscious()
{
	lostUnits.clear();
	pExec = 0;
	pCurrentCmd = 0;
	SetState( new CUnitStateUnconscious( this ) );
	GetWorld()->GetAISignalManager()->Add( NAI::CreateAICorpseSignal( this ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::OnUnitWasKilled()
{
	// CRAP
	lostUnits.clear();
	GetWorld()->UnitWasKilled( this );
	GetWorld()->AddActionLocator( new CActionLocator( CActionLocator::TYPE_DIE, this, GetPosition().GetCP() ), 1000 );
	// убираем ack-и этого unit-а
	GetWorld()->GetGlobalAck()->OnUnitDied( this );
	// новое состояние
	pState->OnDeath();
	pExec = 0;
	pCurrentCmd = 0;
	SetState( new CUnitStateDeath( this ) );
	GetWorld()->GetAISignalManager()->Add( NAI::CreateAICorpseSignal( this ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::ProcessCritical( NDb::ECritical eCA )
{
	ASSERT( eCA < NDb::N_CRIT_TYPES );
	if ( eCA == NDb::C_NONE ||  eCA >= NDb::N_CRIT_TYPES )
		return;
	if ( IsPerformingAction() )
		criticals.push_back( eCA );
	else
		ProcessCriticalImmediately( eCA );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::ProcessCriticalImmediately( NDb::ECritical eCA )
{
	ASSERT( eCA < NDb::N_CRIT_TYPES );
	if ( eCA == NDb::C_NONE ||  eCA >= NDb::N_CRIT_TYPES )
		return;
	//
	pState->ProcessCritical( eCA );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::RunCriticalExecutor( CCommandExecute *p )
{
	ASSERT( p );
	if ( !p )
		return;
	ASSERT( !IsPerformingAction() );
	pExec = p;
	animator.AlignTime();
	ASSERT( pExec->GetStartAP() == 0 ); // actually critical stuff should not spend APs
	pExec->Run();
	if ( pExec->GetState() != CCommandExecute::RUNNING )
		pExec = 0;
	else
		bIsRunningForcedAction = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::SetState( CUnitState *_pState ) 
{ 
	ASSERT( IsValid( _pState ) );
	if ( !IsValid( _pState ) )
		return;
	//
	if ( IsValid( pState ) )
		pState->OnStateFinished();
	pState = _pState;
	pState->OnStateStarted();
	pState->DisableCriticals();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::CheckCmdExecState()
{
	if ( !IsValid(pExec) )
	{
		pExec = 0;
		return;
	}
	if ( pExec->GetState() != CCommandExecute::RUNNING )
	{
		//ASSERT( pExec->GetState() != CCommandExecute::FAILED );
		if ( pExec->GetState() == CCommandExecute::FINISHED && !bIsRunningForcedAction )
			pCurrentCmd = 0;
		pExec = 0;
		bIsRunningForcedAction = false;
		while ( !criticals.empty() && !IsPerformingAction() )
		{
			NDb::ECritical eCA = criticals.front();
			criticals.pop_front();
			pState->ProcessCritical( eCA );
		}
	}
	else
		CallTimeLabel();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::Do( CCommand *_pCmd )
{
	ASSERT( criticals.empty() );
	CObj<CCommand> pCmdNew( _pCmd );
	// check is dead
	ASSERT( !IsDead() );
	if ( IsDead() && !IsUnconscious() )
		return;
	ASSERT( !bIsRunningForcedAction );
	if ( bIsRunningForcedAction )
		return;
	if ( CDynamicCast<CCmdCancel> p( _pCmd ) )
	{
		//OutputDebugString(" CCmdCancel \n");
		CancelAction();
		//
		CancelSnipe(); // нельзя вызывать в CancelAction()
		CancelHeal();
		//
		pCurrentCmd = 0;
		return;
	}
	else if ( CDynamicCast<CCmdSetCommand> p( _pCmd ) )
	{
		if ( CDynamicCast<CCmdEmpty> pEmpty(p->GetCmd()) )
		{
			//OutputDebugString(" CCmdEmpty \n");
			return;
		}
		if ( IsValid( pCurrentCmd ) && IsValid( pExec ) && pExec->IsExecuting() )
		{
			// some command is being executed - have to cancel previous and set new target
			if ( CDynamicCast<CCmdContinue> pContinue(p->GetCmd()) )
			{
				//OutputDebugString(" CCmdContinue, pExec is valid \n");
				return;
			}
			if ( CDynamicCast<CCmdPath> pCmdPath(p->GetCmd()) )
			{
				if ( CDynamicCast<IExecMove> pMove(pExec) )
				{
					//OutputDebugString(" CCmdPath, pExec is valid and is a MoveExec\n");
					vector<NAI::SPathPlace> dst;
					dst.push_back( pCmdPath->ptDst.p );
					NAI::SPathPlace src;
					pMove->GetSearchFromPosition( &src );
					CPtr<NAI::CPath> pPath = GetWorld()->FindPath( this, src, dst, 0, true, pCmdPath->eParams, pCmdPath->bStrafe );
					if ( IsValid( pPath ) )
						pMove->SetNewPath( pPath, pCmdPath->eParams );
					/*else
					{
						CancelAction();
						pCurrentCmd = 0;
					}*/
					return;
				}
				/// else OutputDebugString(" CCmdPath, pExec is valid and is not a MoveExec\n");
			}
			pExec->Cancel();
			if ( !IsCriticalsFailCommand( p->GetCmd() ) )
				pAutoRunCmd = p->GetCmd();
		}
		else if ( CDynamicCast<CCmdContinue> pContinue(p->GetCmd()) )
		{
			//OutputDebugString(" CCmdContinue, else \n");
//			ASSERT( IsValid( pCurrentCmd ) );
			RefreshExecutor();
			if ( IsValid( pExec ) )
			{
				if ( !pExec->IsExecuting() )
				{
					if ( CanSpendAP( pExec->GetStartAP() ) )
					{
						animator.AlignTime();
						pExec->Run();
						CheckCmdExecState();
					}
				}
				else
				{
					//OutputDebugString(" ASSERT(0) \n");
					ASSERT( 0 );
				}
			}
//			else
//				ASSERT( 0 );
		}
		else
		{
			/*if ( CDynamicCast<CCmdPath> pCmdPath(p->GetCmd()) )
				OutputDebugString(" CCmdPath, pExec is not valid \n");
			else
				OutputDebugString(" Some other command \n");*/
			pCurrentCmd = p->GetCmd();
			if ( !IsCriticalsFailCommand( pCurrentCmd ) )
			{
				EUnitCommandResult eResult;
				pExec = pState->CreateExecutor( pCurrentCmd, &eResult );
			}
			if ( !IsValid( pExec ) )
				pCurrentCmd = 0; // impossible
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::DynamicallyLockWay( CPtr<NAI::CPath> pPath )
{
	NAI::IPathNetwork *pNet = GetWorld()->GetPathNetwork();
	pNet->ClearDynamicLocks( this );
	if ( !IsDead() && !IsUnconscious() && IsValid( pPath ) )
	{
		pNet->ChangeDynamicLocks( this, pPath->points );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CUnitServer::CanDo( CCmd *p, int *pnStartAP, int *pnFullAP )
{
	CPtr<CCmd> pCmdHolder = p;

	if ( pnStartAP )
		*pnStartAP = -1;
	if ( pnFullAP )
		*pnFullAP = -1;
	//
	if ( IsCriticalsFailCommand( p ) )
		return UCR_CRITICALS_BAN;
	//
	if ( CDynamicCast<CCmdEmpty> pEmpty(p) )
		return UCR_OK;
	/*
	// Reload
	CDynamicCast<CCmdShootTile> pShootTile( p );
	CDynamicCast<CCmdShootObject> pShootObject( p );
	if ( IsValid( pShootTile ) || IsValid( pShootObject ) )
	{
		if ( GetActionType( this ) == AT_SHOOT )
		{
			CDynamicCast<NRPG::IWeaponItem> pWeaponItem( GetUnitRPG()->GetWeaponItem() );
			if ( IsValid( pWeaponItem ) )
			{
				if ( !pWeaponItem->IsWorking() )
					return UCR_WEAPON_JAMMED;
				else if ( !pWeaponItem->HasAmmo() )
					return UCR_NEED_RELOAD;
			}
		}
	}
	*/
	//
	if ( CDynamicCast<CCmdContinue> pContinue(p) )
	{
		if ( IsPerformingAction() )
			return UCR_UNAVAILABLE;

		RefreshExecutor();
		if ( IsValid( pExec ) )
		{
			int nActionAP = pExec->GetActionAP();
			if ( pnStartAP )
				*pnStartAP = pExec->GetStartAP();
			if ( pnFullAP )
				*pnFullAP = nActionAP;

			if ( !CanSpendAP( nActionAP ) )
				return UCR_NOT_ENOUGH_AP;

			return UCR_OK;
		}
		return UCR_UNAVAILABLE;
	}

	CDynamicCast<CCmdPath> pMove(p);
	if ( pMove && ( !pnStartAP ) )
	{
		// Искать путь в случае, когда юнит стоит, а не лежит, гораздо быстрее. Поэтому проверка того, существует ли путь,
		// производится с WishPose = STAND.
		vector<NAI::SPathPlace> dst;
		dst.push_back( pMove->ptDst.p );
		NAI::EPose curPose = GetWishPose();
		SetWishPose( NAI::WALK );
		const NAI::SUnitPosition &pos = GetPosition();
		CPtr<NAI::CPath> pPath = GetWorld()->FindPath( this, pos.pos.p, dst, 0, true, pMove->eParams, pMove->bStrafe, true );
		SetWishPose( curPose );
		if ( IsValid( pPath ) )
			return UCR_OK;
		return UCR_PATH_NOT_FOUND;
	}

	EUnitCommandResult eResult = UCR_OK;
	CObj<CCommandExecute> pHold = pState->CreateExecutor( p, &eResult );
	if ( IsValid( pHold ) )
	{
		int nActionAP = pHold->GetActionAP();

		if ( pnStartAP )
			*pnStartAP = pHold->GetStartAP();
		if ( pnFullAP )
			*pnFullAP = nActionAP;

		if ( !CanSpendAP( nActionAP ) )
			return UCR_NOT_ENOUGH_AP;

		return eResult;
	}

	if ( eResult == UCR_OK ) // CRAP exact reason should be returned by CreateExecutor
		eResult = UCR_GENERAL_FAILURE;

	return eResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::HasEnoughAP()
{
	ASSERT( !IsPerformingAction() );
	RefreshExecutor();
	if ( IsValid( pExec ) )
		return CanSpendAP( pExec->GetStartAP() );
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnit::EState CUnitServer::GetState()
{
	if ( typeid(*pState) == typeid(CUnitStateSniping) )
		return ST_SNIPE;
	if ( typeid(*pState) == typeid(CUnitStateUsingCannon) )
		return ST_MACHINE_GUN;
	if ( typeid(*pState) == typeid(CUnitStateCorpseCarrier) )
		return ST_CARRY_CORPSE;
	if ( typeid(*pState) == typeid(CUnitStateHealer) )
		return ST_HEALER;
	switch ( GetActionType( this ) )
	{
		case AT_NONE: return ST_NORMAL_DEFAULT;
		case AT_THROW:
		case AT_SHOOT:
		case AT_BAZOOKA:
			break;//return ST_NORMAL;
		case AT_MELEE: return ST_NORMAL_MELEE;
		case AT_GRENADE: return ST_NORMAL_GRENADE;
		case AT_FIRSTAID: return ST_NORMAL_MEDKIT;
		default:
			ASSERT( 0 );
			break;
	}
	switch ( GetUnitRPG()->GetWeaponType() )
	{
		case NDb::WT_PISTOL: return ST_NORMAL_PISTOL;
		case NDb::WT_RIFLE: return ST_NORMAL_RIFLE;
		case NDb::WT_SUB_MACHINE_GUN: return ST_NORMAL_SUB_MACHINE_GUN;
		case NDb::WT_KNIFE: return ST_NORMAL_KNIFE;
		case NDb::WT_MACHINE_GUN: return ST_NORMAL_HAND_MACHINE_GUN;
		case NDb::WT_RLAUNCHER: return ST_NORMAL_RLAUNCHER;
		case NDb::WT_MINE_DETECTOR:
		case NDb::WT_DEFAULT:
			break;
		default:
			ASSERT( 0 );
			break;
	}
	return ST_NORMAL_DEFAULT;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::RefreshExecutor()
{
	if ( IsValid( pExec ) )
		return;
	if ( !IsValid( pCurrentCmd ) )
		return;

	EUnitCommandResult eResult;
	pExec = pState->CreateExecutor( pCurrentCmd, &eResult );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::CancelAction()
{
	if ( bIsRunningForcedAction )
		return;
	if ( IsValid( pExec ) && pExec->IsExecuting() )
		pExec->Cancel();
	else
	{
		//		pCurrentCmd = 0;
		pExec = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::OnTBSEvent( ETBSEvent event )
{
	switch ( event )
	{
		case TBS_START_NEW_TURN:
			lostUnits.clear();
			if ( !IsDead() && !IsUnconscious() )
			{
				GetUnitRPG()->StartNewTurn( GetPosition().GetCP() );
				animator.StartNewTurn( GetPosition() );
				pState->OnStartNewTurn();
				wasInterruptedList.clear();
			}
			break;			
		case TBS_FINISH_OWN_TURN:
			pState->OnFinishOwnTurn();
			break;			
		case TBS_START_REAL_TIME:
			if ( !IsDead() && !IsUnconscious() )
				GetUnitRPG()->StartRealTime();
			pState->OnStartRealTime();
			break;			
		case TBS_ACTION_FINISH:
			ASSERT( !IsPerformingAction() );
			//if ( pExec->IsValid() ) return;
			if ( !IsDead() && !IsUnconscious() )
			{
				NAI::IPathNetwork *pNet = GetWorld()->GetPathNetwork();
				pNet->Unlock( this );
				if ( !pNet->IsPassable( GetPosition().pos.p ) )
				{
					GetUnitRPG()->Kill();
					KillUnit( CVec3(0,0,1) );
				}
				pNet->Lock( this, GetPosition().pos.p );
			}
			pState->OnActionFinish();
			break;			
		case TBS_CANCEL_ACTION:
			CancelAction();
			break;
		case TBS_RECALC_COMMAND:
			// is called when something happens that might affect current command execution plan
			pExec = 0;
			break;
		default:
			ASSERT( 0 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::IsPerformingAction() const
{
	return IsValid( pExec ) && pExec->IsExecuting();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::GetVisible( vector<CPtr<CUnit> > *pTarget ) const
{
	pTarget->resize( visible.size() );
	int nTemp = 0;
	for ( list<CPtr<CUnitServer> >::const_iterator i = visible.begin(); i != visible.end(); ++i )
	{
		(*pTarget)[nTemp] = i->GetPtr();
		nTemp++;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::GetInfo( NRPG::SUnitInfo *pInfo ) const
{
	GetUnitRPG()->GetInfo( GetPose(), pInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IPlayer* CUnitServer::GetPlayer() const 
{ 
	return pPlayer; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::IUnitMissionInfo* CUnitServer::GetRPG() const 
{ 
	return GetUnitRPG(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::Segment()
{
	STime tCur = GetWorld()->GetTime()->GetValue();
	int nDoNotFreeze = 0;
	while ( IsValid( pExec ) && pExec->IsExecuting() )
	{
		NAI::SUnitPosition p;
		if ( pExec->IsWaitingForPath( &p ) )
		{
			CObjectBase* locker = GetWorld()->GetPathNetwork()->GetWhoLocksThisPlace( p.pos.p );
			CDynamicCast<CUnitServer> pUS(locker);
			if ( pUS && !pUS->IsMoving() )
			{
				if ( pUS.GetPtr() == this ) // BUG - unit locks himself
				{
					ASSERT(0);
				}
				// set new path
				if ( CDynamicCast<IExecMove> pMove( pExec ) )
				{
					NAI::SPathPlace desired;
					NAI::EFindPathParams eParams;
					pMove->GetDesiredPlace( &desired, &eParams );
					vector<NAI::SPathPlace> dst;
					dst.push_back( desired );
					NAI::SPathPlace src;
					pMove->GetSearchFromPosition( &src );
					CPtr<NAI::CPath> pPath = GetWorld()->FindPath( this, src, dst, 0, true, eParams, IsStrafing() );
					if ( IsValid( pPath ) )
						pMove->SetNewPath( pPath, eParams );
					else
					{
						CancelAction();
						pMove->FullCancel();
						pCurrentCmd = 0;
					}
				}
			}
			break;
		}
		if ( bCallTimeLabel && tCur >= animator.GetTimeLabel1() )
			bCallTimeLabel = pExec->TimeLabelReached();
		if ( tCur >= animator.GetTimeEnd() )
		{
			if ( bCallTimeLabel )
			{
				// prevent situation when time mark is set wrong (behind animation finish)
				bCallTimeLabel = pExec->TimeLabelReached();
				// new animation could be set
				if ( tCur < animator.GetTimeEnd() )
					break;
			}
			pExec->AnimationFinished();
			CheckCmdExecState();
		}
		else
			break;
		++nDoNotFreeze;
		if ( nDoNotFreeze > 100 )
			__debugbreak(); // somehow we freezed here?
		if ( nDoNotFreeze > 150 )
			break;
	}
	if ( IsValid(pAutoRunCmd) && ( !IsPerformingAction() )  )
	{
		pCurrentCmd = pAutoRunCmd;
		pAutoRunCmd = 0;
		Do( new CCmdSetCommand( this, new CCmdContinue ) );
	}
	CDumbUnitServer::Segment();
	pState->Segment();
	FetchRPGAcks();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::UpdateVisible( SInterruptInfo *pRes )
{
	if ( IsDead() || IsUnconscious() )
	{
		visible.clear();
		return;
	}
	
	list< CPtr<CUnit> > playerVisible;
	GetPlayer()->GetVisible( &playerVisible );
	list<CPtr<CUnitServer> > origVis = visible;
	bool bDoInterrupt = false;
	if ( !pPlayer->IsCheatEnabled( CHEAT_SEEALL ) )
	{
		list<CPtr<CUnitServer> > res, oldVisible = visible;
		GetWorld()->GetUnitsNear( GetPosition().GetEyePosition(), &res, GetRPG()->GetSightDistance( GetPose() ) );
		
		visible.clear();
		for ( list<CPtr<CUnitServer> >::iterator i = res.begin(); i != res.end(); ++i )
		{
			CUnitServer *pEnemy = *i;
			if ( pEnemy->pPlayer == pPlayer )
				continue;
			if ( !GetWorld()->GetGame()->CheckVisibility( this, pEnemy ) )
			{
				// враг потерян из вида
				if ( find( oldVisible.begin(), oldVisible.end(), pEnemy ) != oldVisible.end() )
				{
					GetWorld()->SetAudible( this, pEnemy );
					if ( find( lostUnits.begin(), lostUnits.end(), pEnemy ) == lostUnits.end() )
						lostUnits.push_back( pEnemy );
				}
				//
				continue;
			}

			// говорим commander-у, что увидели Unit
			GetPlayer()->GetCommander()->OnSeeUnit( this, pEnemy );
			//
			if ( !( GetWorld()->GetCurrentPlayer() == GetPlayer() && IsAudible( pEnemy ) ) )
			{
				if ( find_if( playerVisible.begin(), 
					playerVisible.end(), SPtrTest(pEnemy) ) == playerVisible.end() && 
					!pEnemy->IsDead() && !pEnemy->IsUnconscious() )
				{
					list< CPtr<CUnitServer> >::iterator lost = find( lostUnits.begin(), lostUnits.end(), pEnemy );
					if ( lost != lostUnits.end() )
						lostUnits.erase( lost );
					GetWorld()->GetGlobalAck()->OnEnemyBecomesVisible( this, pEnemy, GetWorld()->IsRealTime() );
					pRes->AddEvent( this, pEnemy );
				}
			}
			//
			visible.push_back( pEnemy );
		}
	}
	else
	{
		visible.clear();
		GetWorld()->GetAllUnits( &visible );
	}
	FilterSounds( visible ); // FilterSounds( &aiSounds, visible );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NAI::CPath* CUnitServer::GetCurrentPath()
{
	RefreshExecutor();
	if ( IsValid( pExec ) )
		return pExec->GetCurrentPath();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IPathViewer* CUnitServer::CreatePathViewer()
{
	return new CPathViewer( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::GetCurrentCommandName( string *pName ) const
{
	if ( !IsValid( pExec ) )
		return false;
	
	const char *pszName = typeid( *pExec ).name();
	const char *pszRealName = strstr( pszName, "::" );
	if ( !pszRealName )
		(*pName) = pszName;
	else
		(*pName) = pszRealName + 2;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::CanHearSound( const CVec3 &ptFrom, NDb::CAISound *pSound, int nSoundType, CDumbUnitServer *pWho )
{
	return !IsDead() && !IsUnconscious() && GetUnitRPG()->CanHearSound( ptFrom, 
		GetPosition().GetCP(), pSound, nSoundType, pWho->GetUnitRPG() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 CUnitServer::GetAttackOrigin() const
{
	return GetAttackOrigin( GetPosition() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 CUnitServer::GetAttackOrigin( const NAI::SUnitPosition &from ) const
{
	if ( CCannon *pCannon = animator.GetCannon() )
		return pCannon->GetPosition() + CVec3(0,0,0.722f); // CRAP, should be correct position
	NDb::CAnimWeaponType *pType = GetUnitRPG()->GetAnimWeaponType();
	if ( !pType )
		return GetPosition().GetEyePosition();
	CVec3 rel;
	switch ( from.GetPose() )
	{
		case NAI::CRAWL:
			rel = pType->crawl;
			break;
		case NAI::CROUCH:
			rel = pType->crouch;
			break;
		case NAI::WALK:
		case NAI::RUN:
			rel = pType->stand;
			break;
	}
	CQuat q( from.GetDirection(), CVec3(0,0,1) );
	return from.GetCP() + q.Rotate(rel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CUnitServer::GetMinClearDistance() const
{
	if ( CCannon *pCannon = animator.GetCannon() )
		return 1.0f; // CRAP, should be correct position
	NDb::CAnimWeaponType *pType = GetUnitRPG()->GetAnimWeaponType();
	if ( !pType )
		return 0;
	return pType->fMinDistance;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const CObjectBase* CUnitServer::GetAttackIgnore() const
{
	if ( CCannon *pCannon = animator.GetCannon() )
		return pCannon;
	return this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::UpdateCriticalsState()
{ 
	pState->DisableCriticals(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::IsCriticalsFailCommand( CCmd *pCmd )
{
	EActionType eType = GetActionType( this );
	if ( CDynamicCast<CCmdPath> pTmpCmd(pCmd) ) // CHANGE POSE / MOVE
	{
		if ( 
			GetUnitRPG()->HasCritical( NDb::C_MOTIONLESS ) ||
			GetUnitRPG()->HasCritical( NDb::C_PATIENT ) )
				return true;
	}
	else if ( CDynamicCast<CCmdShootObject> pTmpCmd(pCmd) )
	{
		if ( eType == AT_MELEE ) 
		// MELEE UNIT / OBJECT
		{
			if ( 
				GetUnitRPG()->HasCritical( NDb::C_BLIND ) ||
				GetUnitRPG()->HasCritical( NDb::C_MOTIONLESS ) ||
				GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) ||
				GetUnitRPG()->HasCritical( NDb::C_PATIENT ) )
					return true;
		}
		else
		// SHOOT UNIT / OBJECT
		{
			if ( 
				GetUnitRPG()->HasCritical( NDb::C_BLIND ) ||
				GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ))
					return true;
		}
	}
	else if ( CDynamicCast<CCmdShootTile> pTmpCmd(pCmd) ) 
	{
		if ( eType == AT_GRENADE ) 
		// THROW GRENADE
		{
			if ( 
				GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) )
				return true;
		}
		else
		if ( eType == AT_MELEE )
		// MELEE TILE
		{
			if ( 
				GetUnitRPG()->HasCritical( NDb::C_MOTIONLESS ) ||
				GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) ||
				GetUnitRPG()->HasCritical( NDb::C_PATIENT ))
					return true;
		}
		// SHOOT TILE
		{
			if ( 
				GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) )
				return true;
		}
	}
	else if ( CDynamicCast<CCmdHeal> pTmpCmd(pCmd) ) // HEAL
	{
		if ( 
			GetUnitRPG()->HasCritical( NDb::C_BLIND ) ||
			GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) )
				return true;
	}
	else if ( CDynamicCast<CCmdOpenClose> pTmpCmd(pCmd) ) // OPEN / CLOSE
	{
	}
	else if ( CDynamicCast<CCmdCannon> pTmpCmd(pCmd) ) // USE CANNON
	{
		if ( 
			GetUnitRPG()->HasCritical( NDb::C_BLIND ) ||
			GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) )
				return true;
	}
	else if ( CDynamicCast<CCmdExitCannon> pTmpCmd(pCmd) ) // STOP USING CANNON
	{
	}
	else if ( CDynamicCast<CCmdTakeCorpse> pTmpCmd(pCmd) ) // TAKE CORPSE
	{
		if ( 
			GetUnitRPG()->HasCritical( NDb::C_BLIND ) ||
			GetUnitRPG()->HasCritical( NDb::C_MOTIONLESS ) ||
			GetUnitRPG()->HasCritical( NDb::C_ENCUMBRANCE ) ||
			GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) ||
			GetUnitRPG()->HasCritical( NDb::C_PATIENT ))
				return true;
	}
	else if ( CDynamicCast<CCmdDropCorpse> pTmpCmd(pCmd) ) // THROW CORPSE
	{
	}
	else if ( CDynamicCast<CCmdReload> pTmpCmd(pCmd) ) // RELOAD WEAPON
	{
		if ( 
			GetUnitRPG()->HasCritical( NDb::C_BLIND ) ||
			GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) )
				return true;
	}
	else if ( CDynamicCast<CCmdMoveInventoryItem> pTmpCmd(pCmd) ) // MOVE ITEM
	{
		if ( 
			GetUnitRPG()->HasCritical( NDb::C_BLIND ) ||
			GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) )
				return true;
	}
	else if ( CDynamicCast<CCmdSetActiveItem> pTmpCmd(pCmd) ) // SET ACTIVE ITEM
	{
		if ( 
			GetUnitRPG()->HasCritical( NDb::C_BLIND ) ||
			GetUnitRPG()->HasCritical( NDb::C_IDLE_HAND ) )
				return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::CanSnipe() const
{
	// есть снайперское оружие
	if ( !IsValid( GetUnitRPG()->GetWeaponItem() ) || !GetUnitRPG()->GetWeaponItem()->GetDBWeapon()->bScope )
		return false;
	//
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::IsSniping() const
{
	if ( CDynamicCast<CUnitStateSniping> pTmpState(pState) )
		return true;
	else
		return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::CollectSnipeAP( int nExtraAP )
{
	if ( CDynamicCast<CUnitStateSniping> pSnipingState(pState) )
		pSnipingState->CollectSnipeAP( nExtraAP );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::CancelSnipe()
{
	if ( CDynamicCast<CUnitStateSniping> pSnipingState(pState) )
		pSnipingState->CancelSnipe();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CComplexHead* CUnitServer::GetDBHead()
{
	return GetUnitRPG()->GetRPGPers()->pHead;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::IsCapPresent()
{
	return !IsDead() && !IsUnconscious();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::FetchRPGAcks()
{
	int nAckID;
	NRPG::IUnitMissionInfo *pUnitMission;
	while ( GetUnitRPG()->GetAck( &nAckID, &pUnitMission ) )
	{
		CPtr<CUnitServer> pAttacker = GetWorld()->GetUnitServer( pUnitMission );
		CPtr<CUnitServer> pTarget = GetWorld()->GetUnitServer( GetUnitRPG() );
		//
		switch( nAckID )
		{
			case N_ACK_CRITICAL:
				GetWorld()->GetGlobalAck()->OnDoCriticalDamage( pAttacker, pTarget );
				break;
			case N_ACK_DEATH:
				GetWorld()->GetGlobalAck()->OnUnitWasKilled( pAttacker, pTarget );
				break;
			case N_ACK_SKILL:
				GetWorld()->GetGlobalAck()->OnSkillIncreased( pTarget );
				break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::CancelHeal()
{
	if ( CDynamicCast<CUnitStateHealer> pHealer( pState ) )
	{
		pHealer->Cancel();
		SetState( new CUnitStateNormal( this ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitServer::ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor )
{
	bool isDead = GetUnitRPG()->IsDead();
	int nRet = CDumbUnitServer::ProcessAttack( nUserID, pAttack, pArmor );
	if ( !isDead && GetUnitRPG()->IsDead() )
	{
		if ( IsValid( GetCorpseCarrier() ) )
		{
			CPtr<NRPG::CGlobalPlayer> pGlobalPlayer = GetCorpseCarrier()->GetPlayer()->GetGlobalPlayer();
			if ( IsValid( pGlobalPlayer ) )
					pGlobalPlayer->deployData.unitsDeployData[ GetUnitRPG()->GetRPGUnit() ].bCorpseAlive = false;
		}
		GetWorld()->GetGlobalGame()->pScenarioTracker->OnScenarioClueDestroyed( this->GetUnitRPG()->GetRPGPersID(), true );
		//
		if ( IsValid(pAttack->pAttacker) )
		{
			const vector< CPtr<CUnitServer> > &units = GetWorld()->GetUnitServer(pAttack->pAttacker)->GetTBSPlayer()->GetPlayerUnits();
			int nXP = CDumbUnitServer::GetUnitRPG()->GetXP( units.size() );
			for ( int i = 0; i < units.size(); ++i )
				units[i]->GetUnitRPG()->GetRPGUnit()->AddXP(nXP);
		}
	}
	return nRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::IsMoving() const 
{ 
	CDynamicCast<IExecMove> pMove(pExec);
	if ( GetWorld()->GetCurrentPlayer() ) // turn-based mode
	{
		return false;
	}
	if ( !pMove )
		return false;
	else
		return ( !pExec->IsWaitingForPath() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitServer::GetCarefulShotExtraAP()
{
	CDynamicCast<NRPG::IWeaponItem> pWeaponItem( GetUnitRPG()->GetWeaponItem() );
		if ( IsValid( pWeaponItem ) && pWeaponItem->GetShootMode() != NDb::SM_Careful )
			return 0;
	//
	int nRes = 0;
	//
	if ( animator.IsAiming() )
		nRes = GetActionAP( NRPG::AC_SHOOT );
	else
		nRes = GetActionAP( NRPG::AC_PREPARE_AND_SHOOT );
	//
	NRPG::SUnitInfo Info;
	GetInfo( &Info );
	nRes = max( 0, Info.nAP - nRes );
	//
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::HasLostUnits() 
{ 
	for ( list< CPtr<CUnitServer> >::iterator i = lostUnits.begin(); i != lostUnits.end(); ++i )
		if ( !(*i)->IsDead() && !(*i)->IsUnconscious() )
			return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::IsUnconscious() const
{
	return CDynamicCast<CUnitStateUnconscious>( pState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitServer::GetBarrelDir( CRay *pRay )
{
	ASSERT( pRay != 0 );
	if ( pRay == 0 )
		return false;
	//
	CPtr<NRPG::IWeaponItem> pWeaponItem = GetUnitRPG()->GetWeaponItem();
	if ( !IsValid( pWeaponItem ) )
		return false;
	//
	CDBPtr<NDb::CRPGWeapon> pDBWeapon = pWeaponItem->GetDBWeapon();
	if ( !IsValid( pDBWeapon ) )
		return false;
	//
	NAnimation::SBonePose barrel;
	if ( !animator.GetBarrelPos( pDBWeapon->GetModel()->pGeometry, &barrel ) )
		return false;
	//
	pRay->ptOrigin = barrel.pos;
	barrel.rot.GetXAxis( &pRay->ptDir );
	Normalize( &pRay->ptDir );
	if ( GetUnitRPG()->GetWeaponType() == NDb::WT_RLAUNCHER )
		pRay->ptDir = -pRay->ptDir; // это не баг, так сделано художниками
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0x0251101c, CUnitServer )
BASIC_REGISTER_CLASS( CCommandExecute )
REGISTER_SAVELOAD_CLASS( 0x02682140, CPathViewer )