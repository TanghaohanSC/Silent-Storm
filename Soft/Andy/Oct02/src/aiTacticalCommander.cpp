#include "StdAfx.h"

#include "..\misc\HPTimer.h"
#include "..\misc\RandomGen.h"

#include "RPGItem.h"
#include "RPGUnitMission.h"

#include "..\DBFormat\DataRPG.h"

#include "wMain.h"
#include "wObject.h"
#include "wDynObject.h"
#include "wInterface.h"
#include "wUnitServer.h"
#include "wUnitAttack.h"
#include "wUnitCommands.h"

#include "aiJob.h"
#include "aiLog.h"
#include "aiUnit.h"
#include "aiState.h"
#include "aiWeapon.h"
#include "aiPlayer.h"
#include "aiAction.h"
#include "aiIterator.h"
#include "aiInventory.h"
#include "aiCriterion.h"
#include "aiCommander.h"
#include "aiMoveAction.h"
#include "aiAttackAction.h"
#include "aiTaskCommander.h"
#include "aiCompoundAction.h"
#include "aiScaleConverter.h"
#include "aiSignal.h"
#include "aiControl.h"

#include "aiTacticalCommander.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAITacticalCommander
////////////////////////////////////////////////////////////////////////////////////////////////////
CAITacticalCommander::CAITacticalCommander( CAICommander *_pAICommander ):
	CAIJob(0), nIterationID(0), bEndOfTurn(true), bBestTurnFound(true), 
	pAICommander(_pAICommander), bNeedContinueAction(false)
{
	ASSERT( IsValid( pAICommander ) );
	//
	pAILog = CreateAILogContainer();
	pAIState = CreateAIState( GetWorld(), this );
	pAICriterion = CreateAIDamageCriterion( pAIState );
	//
	CreateActions();
	//
	pAIUnitIterator = CreateAIUnitIterator( pAIState );
	pAIActionIterator = CreateAIActionIterator( pAIState, actions.size() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::CreateActions()
{
	//actions.push_back( CreateAIShootAction( pAIState, this ) );
	actions.push_back( CreateAIMoveAndShootCompoundAction( pAIState, this ) );
	//actions.push_back( CreateAIShootAndHideCompoundAction( pAIState, this ) );
	//actions.push_back( CreateAIMoveToHideAction( pAIState, this ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::ResetActions()
{
	for ( vector< CObj<CAIAction> >::iterator i = actions.begin(); i != actions.end(); ++i )
		(*i)->Reset();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::OnTurnStarted()
{
 	OutputDebugString("[AI TACTICAL COMMANDER] Turn started\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::OnTurnFinished() 
{ 
	OutputDebugString("[AI TACTICAL COMMANDER] Turn finished\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::OnStartRealTime()
{
	CheckForReleasableUnits();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::OnCancelAction()
{
	if ( !IsAITurn() )
		return;
	//
	bNeedContinueAction = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::OnPassControl( NWorld::CPlayer *pPlayer )
{
	OutputDebugString("[AI TACTICAL COMMANDER] Control passed\n");
	//
	CheckForReleasableUnits();
	//
	if ( IsAITurn() )
	{
		vector< CPtr<IAIUnit> > *AllyUnits = pAIState->GetAllyAIPlayer()->GetUnits();
		for ( vector< CPtr<IAIUnit> >::iterator i = AllyUnits->begin(); i != AllyUnits->end(); ++i )
			(*i)->DebugOutput();
		//
		Think();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAITacticalCommander::IsPerformingAction()
{
	return pAIState->IsPerformingAction();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAITacticalCommander::IsEndOfTurn()
{
	return bEndOfTurn || !IsAITurn();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::Segment()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAITacticalCommander::IsIdleJob()
{
	return !IsAITurn();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAITacticalCommander::IsJobFinished()
{
	return pAIUnitIterator->IsEnd() || bBestTurnFound || GetWorld()->IsRealTime() || !IsAITurn();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::OnJobFinished()
{
	bBestTurnFound = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::StartCurrentAction()
{
	if ( !IsJobFinished() )
	{
		while ( !pAIActionIterator->IsEnd() && !actions[ pAIState->GetCurrentAction() ]->IsUsable() )
			pAIActionIterator->Next();
		//
		if ( !pAIActionIterator->IsEnd() )
		{
			IAIJobManager *pAIJobManager = pAIState->GetWorld()->GetAIJobManager();
			actions[ pAIState->GetCurrentAction() ]->SetMaxAP( pAIState->GetCurrentAIUnit()->GetAP() );
			actions[ pAIState->GetCurrentAction() ]->Think();
			CDynamicCast<IAIJob> pJob( actions[ pAIState->GetCurrentAction() ] );
			pAIJobManager->WaitForJob( this, pJob );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::Think()
{
	IAIJobManager *pAIJobManager = pAIState->GetWorld()->GetAIJobManager();
	pAIJobManager->Remove( this );
	// не наш ход или не осталось врагов или союзников
	if ( !IsAITurn() || 
		pAIState->GetEnemyAIPlayer()->GetUnits()->empty() || 
		pAIState->GetAllyAIPlayer()->GetUnits()->empty() )
	{
		bBestTurnFound = true;
		bEndOfTurn = true;
		return;
	}
	//
	pAIState->GetAllyAIPlayer()->CancelActions();
	bEndOfTurn = false;
	bBestTurnFound = false;
	bNeedContinueAction = false;
	//
	pAIState->OnTurnStarted();
	pAILog->Clear();
	Commands.clear();
	pAIJobManager->Add( this );
	ResetActions();
	pAIUnitIterator->First();
	pAIActionIterator->First();
	fBestExpediency = 0;
	OutputDebugString("[AI TACTICAL COMMANDER] Think process started\n");
	OutputDebugString("[AI TACTICAL COMMANDER] First unit\n");
	StartCurrentAction();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::DoJob()
{
	// DEBUG{
	char szStr[128];
	// DEBUG}
	if ( !pAIActionIterator->IsEnd() )
	{
		float fTmpExpediency = pAICriterion->GetExpediency();
		// DEBUG{
		sprintf( szStr,"[AI TACTICAL COMMANDER :: ACTION FINISHED] Expediency = %f ", fTmpExpediency );
		OutputDebugString( szStr );
		// DEBUG}
		if ( fTmpExpediency > fBestExpediency )
		{
			pAILog->Clear();
			pAILog->Add( actions[ pAIState->GetCurrentAction() ]->GetAILog() );
			//
			fBestExpediency = fTmpExpediency;
			//
			OutputDebugString( " [BEST EXPEDIENCY] " );
		}
		//
		OutputDebugString( "\n" );
		//
		actions[ pAIState->GetCurrentAction() ]->GetAILog()->RollBack();
		actions[ pAIState->GetCurrentAction() ]->GetAILog()->Clear();
		pAIActionIterator->Next();
	}
	//
	if ( pAIActionIterator->IsEnd() )
	{
		ResetActions();
		pAIUnitIterator->Next();
		pAIActionIterator->First();
		//
		pAILog->GetCommands( &Commands );
		// DEBUG{
		sprintf( szStr, "[AI TACTICAL COMMANDER] %d commands fetched\n", Commands.size() );
		OutputDebugString( szStr );
		// DEBUG}
		pAILog->Clear();
		//
		if ( !IsJobFinished() )
		{
			fBestExpediency = 0;
			OutputDebugString("[AI TACTICAL COMMANDER] Next unit\n");
		}
	}
	//
	StartCurrentAction();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCommand* CAITacticalCommander::GetCommand()
{
	NWorld::CCommand *pRes = 0;
	if ( !bEndOfTurn && !GetWorld()->IsRealTime() &&
			 !( IsValid( pLastCommandedUnit ) && pLastCommandedUnit->IsPerformingAction() ) )
	{
		if ( bNeedContinueAction )
		{
			// продолжаем прерванное действие
			bNeedContinueAction = false;
			if ( IsValid( pLastCommandedUnit ) )
			{
				pRes = new NWorld::CCmdSetCommand( pLastCommandedUnit, new NWorld::CCmdEmpty() );
				OutputDebugString("[AI TACTICAL COMMANDER] Command continued\n");
			}
		}
		else 
		{
			// выдаем следующую комманду
			if ( !Commands.empty() )				
			{
				pRes = Commands.front().Extract();
				Commands.pop_front();
				OutputDebugString("[AI TACTICAL COMMANDER] Command given\n");
			}
			//
			if ( CDynamicCast<NWorld::CCmdUnit> pCmdUnit( pRes ) )
				if ( CDynamicCast<NWorld::CUnitServer> pUnitServer( pCmdUnit->pUnit ) )	
					pLastCommandedUnit = pUnitServer;
			//
			if ( Commands.empty() && IsJobFinished() && !IsValid( pRes ) )
				bEndOfTurn = true;
		}
	}
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAITacticalCommander::CannonCanDamageEnemy( NWorld::CCannon *pCannon )
{
	vector< CPtr<IAIUnit> > *vEnemyUnits = pAIState->GetEnemyAIPlayer()->GetUnits();
	for ( vector< CPtr<IAIUnit> >::iterator i = vEnemyUnits->begin(); i != vEnemyUnits->end(); ++i )
	{
		if ( (*i)->GetUnitServer()->CanDo( new NWorld::CCmdCannon( pCannon ) ) == NWorld::UCR_OK )
			return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCannon *CAITacticalCommander::FindNearestCannon( IAIUnit *pUnit )
{
	NWorld::CCannon *pRes = 0;
	list< CObj<NWorld::IDynamicObject> > *miscObjects = GetWorld()->GetMiscObjects();
	float fMinDistance = 10; // максимальный радиус поиска пулемета в метрах 
	for ( list< CObj<NWorld::IDynamicObject> >::iterator i = miscObjects->begin(); i != miscObjects->end(); ++i )
	{
		if ( CDynamicCast<NWorld::CCannon> pCannon( *i ) )
		{
			if ( pCannon->GetItem()->HasAmmo() && !pCannon->IsBroken() && !pCannon->IsOccupied() )
			{
				// не вполне корректно, т.к. надо-бы проверять кол-во AP необходимое, чтобы дойти до cannon
				float fDistance = fabs( pCannon->GetPosition() - pUnit->GetUnitServer()->GetPosition().GetCP() );

				if ( !pAICommander->IsObjectLocked( pCannon ) && fDistance < fMinDistance )
				{
					NWorld::EUnitCommandResult eResult;
					CPtr<NWorld::CCmdCannon> pCmd = new NWorld::CCmdCannon( pCannon );
					CPtr<NWorld::CCommandExecute> pExec = NWorld::CreateActionExecutor( pUnit->GetUnitServer(), pCmd, &eResult );

					bool bCannonCanDamageEnemy = CannonCanDamageEnemy( pCannon );

					if ( pExec && CannonCanDamageEnemy( pCannon ) )
					// можем использовать cannon и есть в кого стрелять
					{
						pRes = pCannon;
						fMinDistance = fDistance;
					}
				}
			}
		}
	}
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::UseCannon( IAIUnit *pAIUnit )
{
	/*
	if ( !IsValid( pAIUnit ) )
		return;

	NWorld::CCannon *pCannon = pAIUnit->GetCannon();
	if ( IsValid( pCannon ) )
	{
		pAILog->Clear();
		// проверяем, что cannon в рабочем состоянии
		if ( !( pCannon->GetItem()->HasAmmo() && !pCannon->IsBroken() && CannonCanDamageEnemy( pCannon ) ) )
		{
			// отвязываемся от cannon
			pAIUnit->SetCannon( 0 );
			pAILog->Add( new CAILogExitCannon( pAIUnit, pCannon ) );
			pAICommander->UnLockObject( pCannon );
		}
	}
	else		
	{
		// ищем cannon, который имеет смысл использовать
		CPtr<NWorld::CCannon> pTmpCannon = FindNearestCannon( pAIUnit );
		if ( IsValid( pTmpCannon ) )
		{
			pAILog->Clear();
			pAIUnit->SetCannon( pTmpCannon );
			pAILog->Add( new CAILogUseCannon( pAIUnit, pTmpCannon ) );
			pAICommander->LockObject( pTmpCannon );
		}
	}
	*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::OnUnitWasKilled( NWorld::CUnitServer *pUnit )
{
	CPtr<IAIUnit> pAIUnit = pAICommander->GetAIUnit( pUnit );
	DismissUnit( pAIUnit );
	if ( pAICommander->IsAITurn() )
		Think();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAITacticalCommander::IsUnderControl( IAIUnit *_pAIUnit )
{
	return pAIState->IsContain( _pAIUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::DismissUnit( IAIUnit *_pAIUnit )
{
	pAIState->GetEnemyAIPlayer()->RemoveUnit( _pAIUnit );
	pAIState->GetAllyAIPlayer()->RemoveUnit( _pAIUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::AddAllyUnit( IAIUnit *_pAIUnit )
{
	pAIState->GetAllyAIPlayer()->AddUnit( _pAIUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::AddEnemyUnit( IAIUnit *_pAIUnit )
{
	pAIState->GetEnemyAIPlayer()->AddUnit( _pAIUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::CheckForReleasableUnits()
{
	vector< CPtr<IAIUnit> > UnitsToRelease;
	vector< CPtr<IAIUnit> > *Units = pAIState->GetAllyAIPlayer()->GetUnits();
	for ( vector< CPtr<IAIUnit> >::iterator i = Units->begin(); i != Units->end(); ++i )
		if ( !(*i)->HasVisibleEnemies() || 
			(*i)->GetUnitServer()->IsDead() || (*i)->GetUnitServer()->IsUnconscious() )
			UnitsToRelease.push_back( *i );
	//
	for ( vector< CPtr<IAIUnit> >::iterator i = UnitsToRelease.begin(); i != UnitsToRelease.end(); ++i )
	{
		CPtr<IAIUnit> pAIUnit = *i;
		pAIUnit->OnControlFinished();
		CPtr<NWorld::CUnitServer> pUnitServer = pAIUnit->GetUnitServer();
		if ( pUnitServer->IsDead() || pUnitServer->IsUnconscious() )
			continue;
		//
		CPtr<CTask> pTask = new CTask( pUnitServer, false );
		// идем к последнему месту где видели врага
		IAIUnit *pEnemy = 0;
		SPosition Position;
		pAIUnit->GetLastSeenEnemy( &Position, &pEnemy );
		if ( IsValid( pEnemy ) && 
			!pEnemy->GetUnitServer()->IsDead() && !pEnemy->GetUnitServer()->IsUnconscious() )
		{
			Position.p.SetPose( NAI::WALK );
			pTask->AddCommand( new CTaskCommandGoto( Position ) );
		}
		pTask->AddLookAround();

		IAIControl *pAIControl = CreateAITaskControl( pAICommander,
			pTask, AI_CONTROL_ERASABLE );
		pAICommander->GetAIUnit( pUnitServer )->AssignControl( pAIControl );
	}
	//
	UnitsToRelease.clear();
	if ( pAIState->GetAllyAIPlayer()->GetUnits()->empty() )
		pAIState->GetEnemyAIPlayer()->GetUnits()->clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::OnSeeUnit( NWorld::CUnitServer *pWatcher, NWorld::CUnitServer *pTarget )
{
	if ( pWatcher->IsDead() || pTarget->IsDead() ||
	pWatcher->IsUnconscious() || pTarget->IsUnconscious() )
		return;
	//
	if ( pWatcher->GetPlayer() == pTarget->GetPlayer() )
		return;
	//
	CDynamicCast<CAICommander> pTmpCommander( pWatcher->GetPlayer()->GetCommander() );
	if ( pTmpCommander != pAICommander )
		return;
	//
	CPtr<IAIUnit> pAIWatcher = pAICommander->GetAIUnit( pWatcher );
	CPtr<IAIUnit> pAITarget = pAICommander->GetAIUnit( pTarget );
	if ( !IsValid( pAITarget ) || !IsValid( pAIWatcher ) )
		return;
	//
	if ( !IsUnderControl( pAITarget ) )
		AddEnemyUnit( pAITarget );
	if ( !IsUnderControl( pAIWatcher ) )
		pAIWatcher->AssignControl( CreateAITacticalControl( pAICommander, pAIWatcher ) );
	//
	if ( IsUnderControl( pAIWatcher ) )
	{
//		GetWorld()->GetAISignalManager()->Add( CreateAIRevealEnemySignal( pWatcher->GetPosition().GetCP() ) );
		pAIWatcher->SetLastSeenEnemy( pAITarget );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIUnit *CAITacticalCommander::GetDangerousAttackableEnemy( IAIUnit *pAIUnit )
{
	CPtr<CScaleConverter> pConverter = new CScaleConverter();
	pConverter->AddPoint( -0xFFFF, 500.f );
	pConverter->AddPoint( 0, 1000.f );
	pConverter->AddPoint( 5, 350.f );
	pConverter->AddPoint( 10, 200.f );
	pConverter->AddPoint( 20, 100.f );
	pConverter->AddPoint( 30, 0.f );
	pConverter->AddPoint( 0xFFFF, 0.0f );
	//
	int nD = 0;
	IAIUnit *pRes = 0;
	vector< CPtr<IAIUnit> > *pEnemyUnits;
	if ( pAIState->GetAllyAIPlayer()->IsContain( pAIUnit ) )
		pEnemyUnits = pAIState->GetEnemyAIPlayer()->GetUnits();
	else
		pEnemyUnits = pAIState->GetAllyAIPlayer()->GetUnits();
	//
	for ( vector< CPtr<IAIUnit> >::iterator i = pEnemyUnits->begin(); i != pEnemyUnits->end(); ++i )
	{
		if ( !(*i)->GetUnitServer()->IsDead() || !(*i)->GetUnitServer()->IsUnconscious() )
		{
			int nTmpD = 0;
			int nTmpMaxToHit, nTmpHitCover;
			NDb::EShootMode eTmpShootMode;
			float fDistance = fabs( (*i)->GetPosition().GetCP() - pAIUnit->GetPosition().GetCP() );
			CPtr<IAIWeapon> pTmpWeapon = (*i)->GetAIInventory()->GetBestWeapon( pAIUnit, 
				(*i)->GetAP(), &nTmpHitCover, &nTmpD, &eTmpShootMode, &nTmpMaxToHit );
			nTmpD += pConverter->Convert( fDistance ); // близкие unit-ы ,более опасные

			if ( nTmpD > nD && ( !pAIUnit->IsUsingCannon() || ( pAIUnit->IsUsingCannon() && 
				( pAIUnit->GetUnitServer()->CanDo( new NWorld::CCmdShootTile( (*i)->GetPosition().GetCP() ) ) == NWorld::UCR_OK ) ) ) )
			{
				nD = nTmpD;
				pRes = *i;
			}
		}
	}
	//	
	if ( !IsValid( pRes ) ) // никто не может нанести вред ( например, все далеко )
		pRes = GetNearestEnemy( pAIUnit );
	//
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIUnit *CAITacticalCommander::GetNearestAttackableEnemy( IAIUnit *pAIUnit )
{
	IAIUnit *pRes = 0;
	float fDistance = 0xFFFF;
	vector< CPtr<IAIUnit> > *pEnemyUnits;
	if ( pAIState->GetAllyAIPlayer()->IsContain( pAIUnit ) )
		pEnemyUnits = pAIState->GetEnemyAIPlayer()->GetUnits();
	else
		pEnemyUnits = pAIState->GetAllyAIPlayer()->GetUnits();

	for ( vector< CPtr<IAIUnit> >::iterator i = pEnemyUnits->begin(); i != pEnemyUnits->end(); ++i )
	{
		if ( !(*i)->GetUnitServer()->IsDead() || !(*i)->GetUnitServer()->IsDead() )
		{
			float fTmpDistance = fabs2( (*i)->GetPosition().GetCP() - pAIUnit->GetPosition().GetCP() );

			if ( fTmpDistance < fDistance && ( !pAIUnit->IsUsingCannon() || ( pAIUnit->IsUsingCannon() && 
				( pAIUnit->GetUnitServer()->CanDo( new NWorld::CCmdShootTile( (*i)->GetPosition().GetCP() ) ) == NWorld::UCR_OK ) ) ) )
			{
				fDistance = fTmpDistance;
				pRes = *i;
			}
		}
	}
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIUnit *CAITacticalCommander::GetNearestEnemy( IAIUnit *pAIUnit )
{ 
	float fDistance;
	if ( pAIState->GetAllyAIPlayer()->IsContain( pAIUnit ) )
		return pAIState->GetEnemyAIPlayer()->GetNearestUnit( pAIUnit, &fDistance );
	else
		return pAIState->GetAllyAIPlayer()->GetNearestUnit( pAIUnit, &fDistance );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIUnit *CAITacticalCommander::GetNearestAlly( IAIUnit *pAIUnit )
{ 
	float fDistance;
	if ( pAIState->GetAllyAIPlayer()->IsContain( pAIUnit ) )
		return pAIState->GetAllyAIPlayer()->GetNearestUnit( pAIUnit, &fDistance );
	else
		return pAIState->GetEnemyAIPlayer()->GetNearestUnit( pAIUnit, &fDistance );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalCommander::Synchronize()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CWorld *CAITacticalCommander::GetWorld() 
{ 
	return pAICommander->GetWorld(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAITacticalCommander::IsAITurn() 
{ 
	return pAICommander->IsAITurn(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAITacticalCommander *CreateAITacticalCommander( CAICommander *pAICommander )
{
	return new CAITacticalCommander( pAICommander );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}

using namespace NAI;
REGISTER_SAVELOAD_CLASS( 0x51422140, CAITacticalCommander );