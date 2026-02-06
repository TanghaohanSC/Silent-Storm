#include "StdAfx.h"

#include "wMain.h"
#include "wUnitServer.h"
#include "wUnitSounds.h"
#include "wOSBase.h"
#include "wTurnBased.h"

#include "aiUnit.h"
#include "aiSignal.h"
#include "aiTaskCommander.h"
#include "aiTacticalCommander.h"
#include "aiControl.h"

#include "RPGUnitInfo.h"
#include "RPGItemSet.h"

#include "aiCommander.h"

bool bForbidAI = false;

#include "..\Misc\Commands.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataConst.h"
#include "aiWeapon.h"
#include "aiInventory.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAICommander
////////////////////////////////////////////////////////////////////////////////////////////////////
CAICommander::CAICommander( NWorld::CWorld *_pWorld, NWorld::CPlayer *_pPlayer ): 
	pTaskCommander(0), pPlayer(_pPlayer), pWorld( _pWorld ), bAITurn( false )
{
	pTaskCommander = new CAITaskCommander( this );
	pTacticalCommander = CreateAITacticalCommander( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnUnitAdded( NWorld::CUnitServer *pUnitServer )
{
	ASSERT( IsValid( pUnitServer ) );
	//DEBUG{
	char szStr[128];
	NRPG::SUnitInfo UnitInfo;
	pUnitServer->GetInfo( &UnitInfo );
	sprintf( szStr, "[AI COMMANDER] %d-th unit added. MaxVP = %d, MaxAP = %d\n", Units.size(), UnitInfo.nMaxHP, UnitInfo.nMaxAP );
	OutputDebugString( szStr );
	//DEBUG}
	//
	Units.push_back( CreateAIUnit( pUnitServer, pUnitServer->GetPlayer() == pPlayer ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::GenerateCommand()
{
	// проверка на конец хода
	if ( bForbidAI || IsEndOfTurn() && !pWorld->IsRealTime() )
	{
		Do( new NWorld::CCmdEndOfTurn() );
		return;
	}
	// если выполняется какое-либо действие, то комманд не будет
	if ( !pWorld->IsRealTime() && pWorld->IsAction() )
		return;
	// получаем следующую комманду
	CObj<NWorld::CCommand> pCommand = pTaskCommander->GetCommand();
	// получение боевых комманд
	if ( !IsValid( pCommand ) )
		pCommand = pTacticalCommander->GetCommand();
	// Передаем комманду на исполнение
	CDynamicCast<NWorld::CCmdUnit> pCmdUnit( pCommand );
	if ( IsValid( pCmdUnit ) && IsValid( pCmdUnit->pUnit ) && 
		pWorld->IsUnitActive( pCmdUnit->pUnit ) && 
		!pCmdUnit->pUnit->IsDead() && !pCmdUnit->pUnit->IsUnconscious() )
	{
		Do( pCmdUnit );
		Do( new NWorld::CCmdSetCommand( pCmdUnit->pUnit, new NWorld::CCmdContinue() ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAICommander::IsEndOfTurn()
{
	bool bRes;
	//
	if ( pWorld->IsRealTime() )
	{
		bRes = false;
	}
		else
	{
		bRes = true;
		//
		bRes &= pTaskCommander->IsEndOfTurn();
		//
		bRes &= pTacticalCommander->IsEndOfTurn();
	}
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnTurnStarted()
{
	UnLockAllObjects();
	pTacticalCommander->OnTurnStarted();
	pTaskCommander->OnTurnStarted();
	bAITurn = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnTurnFinished()
{
	bAITurn = false;
	pTacticalCommander->OnTurnFinished();
	pTaskCommander->OnTurnFinished();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnCancelAction()
{
	pTacticalCommander->OnCancelAction();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::Synchronize()
{
	for ( list< CObj<IAIUnit> >::iterator i = Units.begin(); i != Units.end(); ++i )
		if ( !(*i)->GetUnitServer()->IsDead() && !(*i)->GetUnitServer()->IsUnconscious() )
			(*i)->Synchronize();
	//
	pTacticalCommander->Synchronize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnPassControl( NWorld::CPlayer *_pPlayer )
{
	if ( _pPlayer == pPlayer  )
		bAITurn = true;
	else
		bAITurn = false;
	//
	if ( bAITurn )
		Synchronize();
	//
	pTacticalCommander->OnPassControl( pPlayer );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::Segment()
{
	if ( bForbidAI )
		return;
	//
	pTacticalCommander->Segment();
	pTaskCommander->Segment();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnUnitWasKilled( NWorld::CUnitServer *pUnit )
{
	if ( bForbidAI )
		return;
	//
	pTaskCommander->OnUnitWasKilled( pUnit );
	pTacticalCommander->OnUnitWasKilled( pUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnTBSEvent( NWorld::ETBSEvent event )
{
	if ( bForbidAI )
		return;
	//
	switch ( event )
	{
		case NWorld::TBS_START_NEW_TURN:
			OnTurnStarted();
			break;			
		case NWorld::TBS_FINISH_OWN_TURN:
			OnTurnFinished();
			break;
		case NWorld::TBS_CANCEL_ACTION:
			OnCancelAction();
			break;
		case NWorld::TBS_START_REAL_TIME:
			OnStartRealTime();
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnSeeUnit( NWorld::CUnitServer *pWatcher, NWorld::CUnitServer *pTarget ) 
{
	if ( bForbidAI )
		return;
	//
	if ( !pTarget->IsDead() && !pWatcher->IsDead() &&
		!pTarget->IsUnconscious() && !!pWatcher->IsUnconscious() )
		pWatcher->GetWorld()->WantTurnBased( pPlayer );
	//
	pTacticalCommander->OnSeeUnit( pWatcher, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::LockObject( NWorld::CObjectServerBase *pObject )
{
	if ( !IsObjectLocked( pObject ) )
		LockedObjects.push_back( pObject );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::UnLockObject( NWorld::CObjectServerBase *pObject )
{
	list< CPtr<NWorld::CObjectServerBase> >::iterator i = 
		find( LockedObjects.begin(), LockedObjects.end(), pObject );
	if ( i != LockedObjects.end() )
		LockedObjects.erase( i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAICommander::IsObjectLocked( NWorld::CObjectServerBase *pObject )
{
	return find( LockedObjects.begin(), LockedObjects.end(), pObject ) != LockedObjects.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::UnLockAllObjects()
{
	LockedObjects.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::ProcessAISignals()
{
	for ( list< CObj<IAIUnit> >::iterator i = Units.begin(); i != Units.end(); ++i )
		if ( !(*i)->GetUnitServer()->IsDead() && !(*i)->GetUnitServer()->IsUnconscious() )
		{
			CPtr<IAISignal> pAISignal = pWorld->GetAISignalManager()->Get( *i );
			if ( IsValid( pAISignal ) )
				pAISignal->Process( *i );
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIUnit *CAICommander::GetAIUnit( NWorld::CUnitServer *pUnit )
{
	for ( list< CObj<IAIUnit> >::iterator i = Units.begin(); i != Units.end(); ++i )
		if ( (*i)->GetUnitServer() == pUnit )
			return *i;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAICommander::HasVisibleEnemies()
{
	list< CPtr<NWorld::CUnit> > VisibleUnits;
	pPlayer->GetVisible( &VisibleUnits );
	for ( list< CPtr<NWorld::CUnit> >::const_iterator i = VisibleUnits.begin(); i != VisibleUnits.end(); ++i )
		if ( (*i)->GetPlayer() != pPlayer )
			return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::OnStartRealTime()
{
	pTacticalCommander->OnStartRealTime();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICommander::WantTurnBased()
{
	GetWorld()->WantTurnBased( pPlayer );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(aiCommander)
	REGISTER_VAR_EX( "game_noai", NGlobal::VarBoolHandler, &bForbidAI, 0, true )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NAI;
REGISTER_SAVELOAD_CLASS( 0x02731170, CAICommander )
