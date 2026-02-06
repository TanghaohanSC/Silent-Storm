#include "stdafx.h"
//
#include "A5Script.h"
#include "scriptCommon.h"
#include "..\Misc\RandomGen.h"
#include "scScenarioTracker.h"
#include "scFlowChartItems.h"
#include "wUICommands.h"
#include "rpgGlobal.h"
//
#include "scriptScenario.h"
//
namespace NScript
{
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( ScenarioGiveClue, "sb[true]b[false]" )
	string szClueName = luaParams[ 0 ].s;
	bool bTake = luaParams[ 1 ].b;
	bool bImmediately = luaParams[ 2 ].b;
	CPtr<NScenario::CScenarioTracker> pTracker = pScript->GetScenarioTracker();
	if ( IsValid( pTracker ) )
	{
		CPtr<NScenario::CScenarioClue> pClue = pTracker->GetClueByName( szClueName );
		if ( IsValid( pClue ) )
		{
			if ( bTake )
				pTracker->CheatTakeClue( pClue, bImmediately );
			else
				pTracker->CheatDestroyClue( pClue, bImmediately );
		}
	}
	return 0;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( ScenarioOpenZone, "s" )
	CPtr<NScenario::CScenarioTracker> pTracker = pScript->GetScenarioTracker();
	if ( IsValid( pTracker ) )
	{
		string szZoneName = luaParams[ 0 ].s;
		CPtr<NScenario::CScenarioZone> pZone = pTracker->GetZoneByName( szZoneName );
		if ( IsValid( pZone ) )
			pTracker->OpenZone( pZone );
	}
	return 0;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( ScenarioBlockZone, "s" )
	CPtr<NScenario::CScenarioTracker> pTracker = pScript->GetScenarioTracker();
	if ( IsValid( pTracker ) )
	{
		string szZoneName = luaParams[ 0 ].s;
		CPtr<NScenario::CScenarioZone> pZone = pTracker->GetZoneByName( szZoneName );
		if ( IsValid( pZone ) )
			pTracker->BlockZone( pZone );
	}
	return 0;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( ExitToChapter, "" )
	pScript->AddUICommand( new NWorld::CUICmdContinueChapter() );
	return 0;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( ClueShow, "s" )
	CPtr<NScenario::CScenarioTracker> pTracker = pScript->GetScenarioTracker();
	if ( IsValid( pTracker ) )
	{
		CPtr<NScenario::CScenarioClue> pClue = pTracker->GetClueByName( luaParams[ 0 ].s );
		if ( IsValid( pClue ) )
			pScript->AddUICommand( new NWorld::CUICmdShowClue( pClue ) );
	}
	return 0;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( ClueIsFound, "s" )
	CPtr<NScenario::CScenarioTracker> pTracker = pScript->GetScenarioTracker();
	if ( IsValid( pTracker ) )
	{
		CPtr<NScenario::CScenarioClue> pClue = pTracker->GetClueByName( luaParams[ 0 ].s );
		if ( IsValid( pClue ) && pTracker->IsClueFound( pClue ) )
		{
			pScript->PushNumber( 1 );
			return 1;
		}
	}
	pScript->PushNil();
	return 1;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( GetCurrentZoneAILevel, "" )
	CPtr<NScenario::CScenarioZone> pZone = pScript->GetGlobalGame()->pCurrentZone;
	if ( IsValid( pZone ) )
		pScript->PushNumber( pZone->GetDifficulty() );
	else
		pScript->PushNil();
	return 1;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
}