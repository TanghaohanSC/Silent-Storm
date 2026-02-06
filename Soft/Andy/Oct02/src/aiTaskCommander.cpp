#include "stdafx.h"

#include "wMain.h"
#include "wInterface.h"
#include "wUnitServer.h"

#include "RPGUnitInfo.h"

#include "aiUnit.h"
#include "aiCommander.h"
#include "aiControl.h"

#include "..\dbformat\datamap.h"

#include "MapBuild.h"
#include "BuildingInfo.h"

#include "aiTaskCommander.h"

namespace NAI
{
//////////////////////////////////////////////////////////////////////////////////////	
// CAITaskCommander
//////////////////////////////////////////////////////////////////////////////////////	
NWorld::CWorld *CAITaskCommander::GetWorld() const
{ 
	return pAICommander->GetWorld(); 
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAITaskCommander::RemoveTask( CTask *pTask )
{
	ASSERT( IsValid( pTask ) );
	//
	TasksToAddOrRemove.push_back( pTask );
	TasksAddFlag.push_back( false );
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAITaskCommander::AddTask( CTask *pTask )
{
	ASSERT( IsValid( pTask ) );
	ASSERT( !pTask->IsEmpty() );
	//
	TasksToAddOrRemove.push_back( pTask );
	TasksAddFlag.push_back( true );
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAITaskCommander::Segment()
{
	list< CObj<CTask> >::iterator i = TasksToAddOrRemove.begin();
	list<bool>::iterator b = TasksAddFlag.begin();
	for ( ; i != TasksToAddOrRemove.end(); ++i, ++b )
	{
		if ( *b )
			Tasks.push_back( *i );
		else
		{
			list< CObj<CTask> >::iterator k = find( Tasks.begin(), Tasks.end(), *i );
			ASSERT( k !=  Tasks.end() );
			if ( k != Tasks.end() )
				Tasks.erase( k );
		}
	}
	//
	TasksToAddOrRemove.clear();
	TasksAddFlag.clear();
}
//////////////////////////////////////////////////////////////////////////////////////	
bool CAITaskCommander::HasActiveTasks() const
{
	for ( list< CObj<CTask> >::const_iterator i = Tasks.begin(); i != Tasks.end(); ++i )
		if (  IsValid( *i ) && (*i)->IsActive() )
			return true;
	//
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////	
bool CAITaskCommander::CanGetCommand() const
{
	if ( Tasks.empty() || !HasActiveTasks() )
		return false;
	//
	if ( GetWorld()->IsRealTime() )
		return true;
	//
	for ( list< CObj<CTask> >::const_iterator i = Tasks.begin(); i != Tasks.end(); ++i )
		if (  IsValid( *i ) && (*i)->GetUnitServer()->IsPerformingAction() )
			return false;
	//
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////	
NWorld::CCommand* CAITaskCommander::GetCommand()
{
	if ( !CanGetCommand() )
		return 0;
	//
	for ( list< CObj<CTask> >::const_iterator i = Tasks.begin(); i != Tasks.end(); ++i )
	{
		if ( IsValid( *i ) && (*i)->IsActive() && 
			!(*i)->IsEndOfTask() && ( GetWorld()->IsRealTime() || !(*i)->IsEndOfTurn() ) )
		{
			NWorld::CCmd *pCmd = (*i)->GetCommand();
			if ( IsValid( pCmd ) )
				return new NWorld::CCmdSetCommand( (*i)->GetUnitServer(), pCmd );
		}
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAITaskCommander::IsUnderControl( IAIUnit *pAIUnit ) const
{
	ASSERT( IsValid( pAIUnit ) );
	//
	return IsUnderControl( pAIUnit->GetUnitServer() );
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAITaskCommander::OnTurnStarted()
{
	for ( list< CObj<CTask> >::iterator i = Tasks.begin(); i != Tasks.end(); ++i )
		(*i)->OnNewTurn();
}
//////////////////////////////////////////////////////////////////////////////////////	
bool CAITaskCommander::IsEndOfTurn() const
{
	for ( list< CObj<CTask> >::const_iterator i = Tasks.begin(); i != Tasks.end(); ++i )
	{
		if ( IsValid( *i ) && (*i)->IsActive() && !(*i)->IsEndOfTurn() )
			return false;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////	
bool CAITaskCommander::IsUnderControl( NWorld::CUnitServer *pUnitServer ) const
{
	ASSERT( IsValid( pUnitServer ) );
	//
	for ( list< CObj<CTask> >::const_iterator i = Tasks.begin(); i != Tasks.end(); ++i )
	{
		if ( (*i)->GetUnitServer() == pUnitServer )
			return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAITaskCommander::CreateRoute( NWorld::CUnitServer *pUnitServer, SMapUnit sMapUnit, IPathNetwork *pPathNetwork )
{
	ASSERT( IsValid( pUnitServer ) );
	ASSERT( IsValid( pPathNetwork ) );
	//
	CPtr<CTask> pTask = new CTask( pUnitServer, true );
	if ( sMapUnit.route.size() == 0 )
		pTask->AddLookAround();
	else
	{
		char buf[256];
		sprintf( buf, "[AI TASK COMMANDER] Adding route ( %d waypoints ) ... ", sMapUnit.route.size() );
		OutputDebugString( buf );
		for ( vector<CPtr<CMapWaypoint> >::iterator i = sMapUnit.route.begin(); i != sMapUnit.route.end(); ++i )
		{
			sprintf( buf, " [ %s ] " ,(*i)->pName->szName );
			OutputDebugString( buf );

			// добавляем Waypoint
			CVec3 ptPos = (*i)->pos.ptPos;
			int nFloor = (*i)->pos.nFloor;

			NAI::SPosition sNearestPlace = 
				GetNearestPosition( ptPos, pPathNetwork );

			pTask->AddCommand( new CTaskCommandGoto( sNearestPlace ) );
			// добавляем комманды
			for ( vector<NAI::SCommand>::iterator c = (*i)->commands.begin(); c != (*i)->commands.end(); ++c )
			{
				CTaskCommand *pCommand = 0;
				switch ( (*c).cmd )
				{
					case CMD_POSE:
						pCommand = new CTaskCommandChangePose( (*c).pose );
						break;
					case CMD_DIR:
						pCommand = new CTaskCommandChangeDirection( (*c).dir );
						break;
					case CMD_WAIT:
						pCommand = new CTaskCommandWait( (*c).time );
					break;
				}
				if ( IsValid( pCommand ) )
					pTask->AddCommand( pCommand );
			}
		}
		OutputDebugString( "\n" );
	}
	//
	if ( !pTask->IsEmpty() )
	{
		pTask->DelayExecution( random.Get( 1, 20 ) );
		IAIControl *pAIControl = CreateAITaskControl( pAICommander,
			pTask, AI_CONTROL_INTERRUPTABLE );
		pAICommander->GetAIUnit( pUnitServer )->AssignControl( pAIControl );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTask
////////////////////////////////////////////////////////////////////////////////////////////////////
CTask::CTask( NWorld::CUnitServer *_pUnitServer, bool _bCircled ): 
	pUnitServer(_pUnitServer),	bCircled(_bCircled), 
	nCurrentCommand(-1), tTime(0), bActive( true )
{	
	ASSERT( IsValid( pUnitServer ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTask::AddLookAround()
{
	AddCommand( new CTaskCommandChangePose( NAI::WALK ) );
	int n = random.Get( 3, 6 );
	for ( int i = 0; i < n; ++i )
	{
		AddCommand( new CTaskCommandWait( random.Get( 1, 6 ) ) );
		AddCommand( new CTaskCommandChangeDirection( random.Get( 1, 8 ) - 1 ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTask::DebugOutput()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTask::AddCommand( CTaskCommand *pCmd )
{
	ASSERT( IsValid( pCmd ) );
	//
	pCmd->SetUnitServer( pUnitServer );
	Commands.push_back( pCmd );
	SetToBeginning();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTask::OnNewTurn()
{
	if ( nCurrentCommand >= 0 && nCurrentCommand < (int)Commands.size() )
		Commands[nCurrentCommand]->OnNewTurn();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCmd *CTask::GetCommand()
{
	NWorld::CCmd *pCmd = 0;

	if ( !pUnitServer->GetWorld()->IsRealTime() )	tTime = 0;
	if ( pUnitServer && pUnitServer->GetWorld()->GetTime()->GetValue() < tTime )
		return pCmd;

	if ( !pUnitServer->IsPerformingAction() && !(nCurrentCommand >= (int)Commands.size()) ) 
	{
		if ( pUnitServer->HasCommand() && pUnitServer->HasEnoughAP() )
			return new NWorld::CCmdContinue();

		if ( nCurrentCommand == -1 || !pUnitServer->HasCommand() && Commands[nCurrentCommand]->IsEndOfCommand() )
		{
			++nCurrentCommand;
			if ( bCircled && nCurrentCommand == (int)Commands.size() && Commands.size() > 1 )
				nCurrentCommand = 0;
		}
		// возвращаем следующую комманду
		if ( nCurrentCommand > -1 && nCurrentCommand < (int)Commands.size() )
			pCmd = Commands[nCurrentCommand]->GetCommand();
	}
	//
	if ( IsEndOfTask() )
	{
		CDynamicCast<CAICommander> pAICommander( pUnitServer->GetPlayer()->GetCommander() );
		if ( IsValid( pAICommander ) )
			pAICommander->GetAIUnit( pUnitServer )->OnControlFinished();
	}
	//
	return pCmd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask::IsEndOfTurn() const
{
	ASSERT( IsValid( pUnitServer ) );
	//
	CPtr<NWorld::CWorld> pWorld = pUnitServer->GetWorld();
	if ( !pWorld->IsUnitActive( pUnitServer ) || 
		pUnitServer->IsDead() || pUnitServer->IsUnconscious() )
		return true;
	//
	if ( pWorld->GetTime()->GetValue() < tTime && !pWorld->IsRealTime() )
		return true;
	//
	if ( nCurrentCommand >= (int)Commands.size() )
		return true;
	//
	if ( !pUnitServer->IsPerformingAction() && pUnitServer->HasCommand() && !pUnitServer->HasEnoughAP() ||
		   nCurrentCommand >= 0 && Commands[nCurrentCommand]->IsEndOfUnitTurn() )
		return  true;
	else
		return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// // выполнение начинается с запозданием
void CTask::DelayExecution( int nTime ) 
{ 
	tTime = pUnitServer->GetWorld()->GetTime()->GetValue() + nTime*1000; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommand
////////////////////////////////////////////////////////////////////////////////////////////////////
NWorld::CCmd *CTaskCommand::GetCommand()
{
	if ( Commands.empty() )
		Do();

	NWorld::CCmd *pRes = 0;
	if ( !Commands.empty() )
	{
		pRes = Commands.front().Extract();
		Commands.pop_front();
	}

	return pRes;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommandGoto
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTaskCommandGoto::Do()
{
	DoCommand( new NWorld::CCmdPath( ptPosition ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommandChangePose
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTaskCommandChangePose::Do() 
{
	DoCommand( new NWorld::CCmdWishPose( nPose ) );
	SUnitPosition ptPosition = pUnitServer->GetPosition(); ptPosition.SetPose( nPose );
	DoCommand( new NWorld::CCmdPath( ptPosition.pos, PF_USE_POSE ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommandWait
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTaskCommandWait::Do()
{
	if ( !bIsWaiting )
	{
		tTime = NWorld::pCurrentWorld->GetTime()->GetValue() + tLength*1000;
		bNewTurnStarted = false;
		bIsWaiting = true;
	}

	if ( NWorld::pCurrentWorld->IsRealTime() && tTime < NWorld::pCurrentWorld->GetTime()->GetValue() )
		bIsWaiting = false;
	if ( !NWorld::pCurrentWorld->IsRealTime() && bNewTurnStarted )
	{
		bIsWaiting = false;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTaskCommandWait::IsEndOfCommand()
{
	return !bIsWaiting;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTaskCommandWait::IsEndOfUnitTurn() 
{
	return !bNewTurnStarted;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTaskCommandWait::OnNewTurn() 
{
	bNewTurnStarted = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommandChangeDirection
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTaskCommandChangeDirection::Do()	
{
	SUnitPosition ptPosition = pUnitServer->GetPosition();
	ptPosition.pos.p.SetDirection( nDirection );
	DoCommand( new NWorld::CCmdPath( ptPosition.pos, PF_USE_DIR ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}

using namespace NAI;
REGISTER_SAVELOAD_CLASS( 0x51222140, CAITaskCommander )
REGISTER_SAVELOAD_CLASS( 0x51812130, CTask )
REGISTER_SAVELOAD_CLASS( 0x51812131, CTaskCommandGoto )
REGISTER_SAVELOAD_CLASS( 0x51222141, CTaskCommandChangePose )
REGISTER_SAVELOAD_CLASS( 0x51222142, CTaskCommandChangeDirection )
REGISTER_SAVELOAD_CLASS( 0x51222143, CTaskCommandWait )
