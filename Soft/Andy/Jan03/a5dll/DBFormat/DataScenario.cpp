#include "StdAfx.h"
#include "DataFormat.h"
#include "..\Misc\StrProc.h"
//
#include "DataScenario.h"
//
namespace NDb
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const char *pszScenarioClueTypes[N_CT_COUNT] = 
{
	"person",
	"item",
	"conclusion"
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const char *pszScenarioObjectiveTypes[N_OT_COUNT] = 
{
	"capture",
	"destroy"
};
////////////////////////////////////////////////////////////////////////////////////////////////////
EScenarioClueType GetClueTypeByName( const string &szName )
{
	string sz( szName );
	NStr::ToLower( sz );
	NStr::TrimBoth( sz );

	for ( int i = 0; i < N_CT_COUNT; ++i )
		if ( sz == pszScenarioClueTypes[i] )
			return (EScenarioClueType)i;
	return N_CT_COUNT;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EScenarioObjectiveType GetObjectiveTypeByName( const string &szName )
{
	string sz( szName );
	NStr::ToLower( sz );
	NStr::TrimBoth( sz );

	for ( int i = 0; i < N_OT_COUNT; ++i )
		if ( sz == pszScenarioObjectiveTypes[i] )
			return (EScenarioObjectiveType)i;
	return N_OT_COUNT;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDBScenarioZone::Import()
{
	NDatabase::ImportField( "Scenario", &pScenario );
	templatesIDs.resize( 3 );
	NDatabase::ImportField( "TemplateID1", &templatesIDs[0] );
	NDatabase::ImportField( "TemplateID2", &templatesIDs[1] );
	NDatabase::ImportField( "TemplateID3", &templatesIDs[2] );
	NDatabase::ImportField( "ItemSlots", &nItemSlots );
	NDatabase::ImportField( "PersonSlots", &nPersonSlots );
	NDatabase::ImportField( "SmallDescription", &sSmallDescription );
	NDatabase::ImportField( "CluesMaxNumber", &nCluesMaxNumber );
	NDatabase::ImportField( "CanBeRevealed", &bCanBeRevealed );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDBScenarioState::Import()
{
	NDatabase::ImportField( "Description", &sDescription );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDBScenarioClue::Import()
{
	string szName;
	NDatabase::ImportField( "Type", &szName );
	clueType = GetClueTypeByName( szName );
	ASSERT( clueType < N_CT_COUNT );
	NDatabase::ImportField( "Scenario", &pScenario );
	NDatabase::ImportField( "State", &pState );
	zonesToPlace.resize( 3 );
	NDatabase::ImportField( "ZoneToPlace1", &zonesToPlace[0] );
	NDatabase::ImportField( "ZoneToPlace2", &zonesToPlace[1] );
	NDatabase::ImportField( "ZoneToPlace3", &zonesToPlace[2] );
	objectives.resize( 2 );
	NDatabase::ImportField( "Objective1", &objectives[0] );
	NDatabase::ImportField( "Objective2", &objectives[1] );
	NDatabase::ImportField( "SmallDescription", &sSmallDescription );
	NDatabase::ImportField( "ItemID", &nItemID );
	NDatabase::ImportField( "PersID", &nPersID );
	NDatabase::ImportField( "Permanent", &bPermanent );
	NDatabase::ImportField( "MinParentToOpen", &nMinParentToOpen );
	NDatabase::ImportField( "Description", &pDescription );
	NDatabase::ImportField( "GiveImmediately", &bGiveImmediately );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDBScenarioObjective::Import()
{
	string szName;
	NDatabase::ImportField( "Type", &szName );
	type = GetObjectiveTypeByName( szName );
	ASSERT( type < N_OT_COUNT );
	zonesToOpen.resize( 3 );
	NDatabase::ImportField( "ZoneToOpen1", &zonesToOpen[0] );
	NDatabase::ImportField( "ZoneToOpen2", &zonesToOpen[1] );
	NDatabase::ImportField( "ZoneToOpen3", &zonesToOpen[2] );
	zonesToBlock.resize( 3 );
	NDatabase::ImportField( "ZoneToBlock1", &zonesToBlock[0] );
	NDatabase::ImportField( "ZoneToBlock2", &zonesToBlock[1] );
	NDatabase::ImportField( "ZoneToBlock3", &zonesToBlock[2] );
	NDatabase::ImportField( "Description", &pDescription );
	NDatabase::ImportField( "Scenario", &pScenario );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDBScenario::Import()
{
	NDatabase::ImportField( "Name", &szName );
	NDatabase::ImportField( "Description", &szDescription );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDBScenarioObjective2Clue::Import()
{
	NDatabase::ImportField( "ObjectiveID", &pObjective );
	NDatabase::ImportField( "ClueID", &pClue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NDb;
//
REGISTER_SAVELOAD_CLASS( 0x50882180, CDBScenarioZone );
REGISTER_SAVELOAD_CLASS( 0x50882181, CDBScenarioState );
REGISTER_SAVELOAD_CLASS( 0x50882182, CDBScenarioClue );
REGISTER_SAVELOAD_CLASS( 0x50982090, CDBScenarioObjective );
REGISTER_SAVELOAD_CLASS( 0x51582130, CDBScenario );
REGISTER_SAVELOAD_CLASS( 0x50392125, CDBScenarioObjective2Clue );