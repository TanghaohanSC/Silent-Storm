#include "StdAfx.h"
//
#include "wInterface.h"
#include "wUnitServer.h"
//
#include "RPGUnitMission.h"
#include "RPGMerc.h"
#include "RPGItem.h"
#include "RPGItemInfo.h"
//
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataScenario.h"
#include "..\DBFormat\DataConst.h"
#include "..\DBFormat\DataRPG.h"
//
#include "scFlowChart.h"
#include "scFlowChartItems.h"
#include "scScenarioTracker.h"
#include "scCommands.h"
//
namespace NScenario
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioTracker::CScenarioTracker():
	bScenarioAvailable( false ),
	cmdScenario( "scenario", CommandScenario, this ),
	nZonesOpenOrder( 0 ), nCluesOpenOrder( 0 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScenarioTracker::GetTemplateIDByVariantID( int nVariantID )
{
	if ( bScenarioAvailable )
		return pScenarioFlowChart->GetTemplateIDByVariantID( nVariantID );
	else
		return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SRandomSeed CScenarioTracker::GetRandomSeedForTemplate( int nTemplateID )
{
	if ( bScenarioAvailable )
	{
		CPtr<CScenarioZone> pZone = GetZone( nTemplateID );
		if ( IsValid( pZone ) )
			return pZone->templates[ nTemplateID ].sSeed;
	}
	//
	return SRandomSeed( GetTickCount() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CString *CScenarioTracker::GetClueDescriptionFromObjective( CScenarioClue *pClue )
{
	for ( vector< CPtr<CScenarioObjective> >::iterator i = pClue->objectives.begin();
		i != pClue->objectives.end(); ++i )
	{
		list< CPtr<CScenarioObjective> >::iterator f = find( finishedObjectives.begin(),
			finishedObjectives.end(), (*i).GetPtr() );
		if ( f != finishedObjectives.end() )
			return (*f)->GetDBObjective()->pDescription;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone *CScenarioTracker::GetZone( int nTemplateID )
{
	if ( bScenarioAvailable )
		return pScenarioFlowChart->GetZoneByTemplateID( nTemplateID );
	else
		return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone* CScenarioTracker::GetZoneByDBZone( NDb::CDBScenarioZone *pDBZone )
{
	if ( !bScenarioAvailable )
		return 0;

	return pScenarioFlowChart->GetZoneByDBZone( pDBZone );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::BlockZone( CScenarioZone *pZone )
{
	ASSERT( IsValid( pZone ) );
	if ( !IsValid( pZone ) )
		return;
	//
	if ( !IsZoneBlocked( pZone ) )
		blockedZones.push_back( pZone );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioTracker::IsZoneBlocked( CScenarioZone *pZone )
{
	ASSERT( IsValid( pZone ) );
	if ( !IsValid( pZone ) )
		return false;
	//
	return find( blockedZones.begin(), blockedZones.end(), pZone ) != blockedZones.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioTracker::IsZoneAvailable( CScenarioZone *pZone )
{
	ASSERT( IsValid( pZone ) );
	if ( !IsValid( pZone ) )
		return false;
	//
	return find( availableZones.begin(), availableZones.end(), pZone ) != availableZones.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::CheatTakeClue( string szName )
{
	CPtr<CScenarioClue> pClue = pScenarioFlowChart->GetClueByName( szName );
	if ( IsValid( pClue ) )
	{
		vector<CPtr<NRPG::CUnit> > units;
		takenClues.push_back( pClue );
		ProcessScenario( units );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::CheatDestroyClue( string szName )
{
	CPtr<CScenarioClue> pClue = pScenarioFlowChart->GetClueByName( szName );
	if ( IsValid( pClue ) )
	{
		vector<CPtr<NRPG::CUnit> > units;
		destroyedClues.push_back( pClue );
		ProcessScenario( units );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::CheatOpenZone( CScenarioZone *pZone )
{
	ASSERT( pZone );
	if ( !IsValid( pZone ) )
		return;

	if ( !IsZoneAvailable( pZone ) && !IsZoneBlocked( pZone ) )
		availableZones.push_back( pZone );
	//
	for ( vector< CPtr<CScenarioClue> >::iterator i = pZone->clues.begin(); 
		i != pZone->clues.end(); ++i )
	{
		CPtr<CScenarioObjective> pTake = (*i)->GetObjectiveByType( NDb::OT_CAPTURE );
		CPtr<CScenarioObjective> pDestroy = (*i)->GetObjectiveByType( NDb::OT_DESTROY );
		if ( IsValid( pTake ) )
			OnObjectiveComplete( pTake );
		else if ( IsValid( pDestroy ) )
			OnObjectiveComplete( pDestroy );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::JustFoundClue( CScenarioClue *pClue )
{
	pClue->SetJustFound( true );
	pClue->SetOpenOrder( nCluesOpenOrder );
	++nCluesOpenOrder;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::JustOpenZone( CScenarioZone *pZone )
{
	pZone->SetOpenOrder( nZonesOpenOrder );
	++nZonesOpenOrder;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::OnObjectiveComplete( CScenarioObjective *pObjective )
{
	if ( !IsValid( pObjective ) )
		return;
	//
	if ( IsValid( pObjective->pParentClue ) && !IsClueFound( pObjective->pParentClue ) )
	{
		finishedObjectives.push_back( pObjective );
		// закрываем зоны
		for ( vector< CPtr<CScenarioZone> >::iterator i = pObjective->zonesToBlock.begin();
			i != pObjective->zonesToBlock.end(); ++i )
				BlockZone( *i );
		// добавляем зоны
		for ( vector< CPtr<CScenarioZone> >::iterator i = pObjective->zones.begin();
			i != pObjective->zones.end(); ++i )
				if ( !IsZoneAvailable( *i ) && !IsZoneBlocked( *i ) )
				{
					JustOpenZone( *i );
					availableZones.push_back( *i );
					//
					OutputDebugString( "[SCENARIO TRACKER] zone available: " );
					OutputDebugString( (*i)->GetDBZone()->sSmallDescription.c_str() );
					OutputDebugString( "\n" );
				}
		// открываем составные clue
		for ( vector< CPtr<CScenarioClue> >::iterator i = pObjective->clues.begin();
			i != pObjective->clues.end(); ++i )
		{
			// считаем входящие связи
			int nCluesFound = 0;
			for ( vector< CPtr<CScenarioObjective> >::iterator p = (*i)->parentObjectives.begin(); 
				p != (*i)->parentObjectives.end(); ++p )
					if ( IsClueFound( (*p)->pParentClue ) )
						++nCluesFound;
			// если их достаточно для получения составного clue, то выполняем его objectives
			if ( nCluesFound >= pScenarioFlowChart->GetMinParentToOpen( *i ) )
			{
				OutputDebugString( "[SCENARIO TRACKER] compound clue given: " );
				OutputDebugString( (*i)->GetDBClue()->sSmallDescription.c_str() );
				OutputDebugString( "\n" );
				//
				JustFoundClue( *i );
				//
				for ( vector< CPtr<CScenarioObjective> >::iterator c = (*i)->objectives.begin();
					c != (*i)->objectives.end(); ++c )
						OnObjectiveComplete( *c );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::PostCreateScenario()
{
	if ( !IsValid( pScenarioFlowChart ) )
		return;
	//
	bScenarioAvailable = true;
	availableZones.clear();
	finishedObjectives.clear();
	blockedZones.clear();
	//
	CPtr<CScenarioZone> pBase = pScenarioFlowChart->GetZoneByName( "BASE" );
	if ( IsValid( pBase ) )
	{
		availableZones.push_back( pBase );
		//
		for ( vector< CPtr<CScenarioClue> >::iterator i = pBase->clues.begin();
			i != pBase->clues.end(); ++i )
			if ( (*i)->IsPlaced() && !(*i)->objectives.empty() )
				OnObjectiveComplete( (*i)->objectives.front() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::CreateScenario( int nScenarioID )
{
	CPtr<CScenarioFlowChart> pFlowChart = CreateScenarioFlowChart( nScenarioID, false );
	if ( IsValid( pFlowChart ) )
	{
		pScenarioFlowChart = pFlowChart;
		PostCreateScenario();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::CreateScenario( string szScenarioName )
{
	CPtr<CScenarioFlowChart> pFlowChart = CreateScenarioFlowChart( szScenarioName, false );
	if ( IsValid( pFlowChart ) )
	{
		pScenarioFlowChart = pFlowChart;
		PostCreateScenario();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::GetAvailableZones( list<CPtr<CScenarioZone> > *pZones )
{
	pZones->clear();
	for ( list< CPtr<CScenarioZone> >::iterator i = availableZones.begin();
		i != availableZones.end(); ++i )
			if ( !IsZoneBlocked( *i ) )
				pZones->push_back( *i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::GetAvailableClues( list<CPtr<CScenarioClue> > *pClues )
{
	pClues->clear();
	for ( vector< CObj<CScenarioClue> >::iterator i = pScenarioFlowChart->clues.begin();
		i != pScenarioFlowChart->clues.end(); ++i )
		if ( (*i)->IsPlaced() && IsClueFound( *i ) )
			pClues->push_back( (*i).GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioTracker::IsObjectiveFinished( CScenarioObjective *pObjective )
{
	return find( finishedObjectives.begin(), 
		finishedObjectives.end(), pObjective ) !=	finishedObjectives.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioTracker::IsClueFound( CScenarioClue *pClue )
{
	if ( !IsValid( pClue ) )
		return false;
	//
	for ( vector< CPtr<CScenarioObjective> >::iterator i = pClue->objectives.begin();
		i != pClue->objectives.end(); ++i )
			if ( IsObjectiveFinished( *i ) )
				return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioTracker::IsZoneContainsSomeClue( CScenarioZone *pZone )
{
	for ( vector< CPtr<CScenarioClue> >::iterator i = pZone->clues.begin();
		i != pZone->clues.end(); ++i )
			if ( !IsClueFound( *i ) )
				return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone *CScenarioTracker::GetRecommendedZone()
{
	int nMinDiff = 0xFFFF;
	CPtr<CScenarioZone> pZone = 0;
	//
	for ( list< CPtr<CScenarioZone> >::iterator i = availableZones.begin();
		i != availableZones.end(); ++i )
			if ( !IsZoneBlocked( *i) && IsZoneContainsSomeClue( *i ) && nMinDiff > (*i)->GetDifficulty() )
			{
				nMinDiff = (*i)->GetDifficulty();
				pZone = *i;
			}
	//
	return pZone;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::RevealZone( CScenarioZone *pZone )
{
	ASSERT( pZone );
	if ( !IsValid( pZone ) )
		return;
	//
	if ( pZone->GetDBZone()->bCanBeRevealed )
		OpenZone( pZone );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::OpenZone( CScenarioZone *pZone )
{
	ASSERT( pZone );
	if ( !IsValid( pZone ) )
		return;
	//
	if ( find( availableZones.begin(), availableZones.end(), pZone ) == availableZones.end() )
		availableZones.push_back( pZone );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::DrawScenario()
{
	if ( IsScenarioAvailable() )
	{
		list<CPtr<CScenarioZone> > zones;
		list<CPtr<CScenarioClue> > clues;
		GetAvailableZones( &zones );
		GetAvailableClues( &clues );
		pScenarioFlowChart->DrawFlowChart( zones, clues );
		pScenarioFlowChart->DrawPassingFlowChart( zones );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::PrintScenarioList()
{
	if ( IsScenarioAvailable() )
		pScenarioFlowChart->PrintScenarioList();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::GetPlacedClues( int nTemplateID, list< CPtr<CScenarioClue> > *clues )
{
	ASSERT( clues != 0 );
	//
	if ( !bScenarioAvailable )
		return;
	//
	CPtr<CScenarioZone> pZone = pScenarioFlowChart->GetZoneByTemplateID( nTemplateID );
	if ( !IsValid( pZone ) )
		return;
	//
	clues->clear();
	for (  vector< CPtr<CScenarioClue> >::iterator i = pZone->clues.begin();
		i != pZone->clues.end(); ++i )
			if ( (*i)->IsPlaced() && (*i)->nTemplateID == nTemplateID )
				clues->push_back( *i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::OnScenarioClueTaken( int nID, bool bUnit )
{
	if ( !bScenarioAvailable )
		return;
	//
	CPtr<CScenarioClue> pClue = 0;
	if ( bUnit )
		pClue = pScenarioFlowChart->GetClueByPersID( nID );
	else
		pClue = pScenarioFlowChart->GetClueByItemID( nID );
	if ( IsValid( pClue ) )
		takenClues.push_back( pClue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::OnScenarioClueDestroyed( int nID, bool bUnit )
{
	if ( !bScenarioAvailable )
		return;
	//
	CPtr<CScenarioClue> pClue = 0;
	if ( bUnit )
		pClue = pScenarioFlowChart->GetClueByPersID( nID );
	else
		pClue = pScenarioFlowChart->GetClueByItemID( nID );
	if ( IsValid( pClue ) )
		destroyedClues.push_back( pClue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::ProcessCluesList( const list< CPtr<CScenarioClue> > &clues,
	NDb::EScenarioObjectiveType type )
{
	for ( list< CPtr<CScenarioClue> >::const_iterator i = clues.begin(); i != clues.end(); ++i )
	{
		CPtr<CScenarioObjective> pObjective = (*i)->GetObjectiveByType( type );
		if ( IsValid( pObjective ) )
		{
			JustFoundClue( *i );
			OnObjectiveComplete( pObjective );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::ExpandFlowChart()
{
	ProcessCluesList( takenClues, NDb::OT_CAPTURE );
	ProcessCluesList( destroyedClues, NDb::OT_DESTROY );
	takenClues.clear();
	destroyedClues.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone *CScenarioTracker::GetZoneInWhichClueWasFound( CScenarioClue *pClue )
{
	ASSERT( IsValid( pClue ) );
	if ( !IsValid( pClue ) )
		return 0;
	//
	if ( pClue->IsCompound() )
		return 0;
	//
	ASSERT( !pClue->parentZones.empty() );
	if ( pClue->parentZones.empty() )
		return 0;
	//
	return pClue->parentZones[0];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::GetZonesWhichCanBeOpened( CScenarioClue *pClue, 
	list< CPtr<CScenarioZone> > *pZones )
{
	ASSERT( IsValid( pClue ) );
	ASSERT( pZones != 0 );
	if ( !IsValid( pClue ) || pZones == 0 )
		return;
	//
	pScenarioFlowChart->GetZonesWhichCanBeOpened( pClue, pZones );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioTracker::ProcessScenario( const vector< CPtr<NRPG::CUnit> > &units )
{
	if ( !bScenarioAvailable )
		return;
	//
	for ( vector< CPtr<NRPG::CUnit> >::const_iterator i = units.begin();
		i != units.end(); ++i )
			OnScenarioClueTaken( (*i)->nRPGPersID, true );
	//
	for ( vector< CPtr<NRPG::CUnit> >::const_iterator i = units.begin();
		i != units.end(); ++i )
	{
		CPtr<NRPG::IInventory> pInventory = (*i)->pInventory;
		// слоты
		for ( int n = 0; n < NDb::N_SLOTS; ++n )
		{
			CPtr<NRPG::IInventoryItem> pItem = pInventory->Get( (NDb::ESlot)n );
			if ( IsValid( pItem ) )
				OnScenarioClueTaken( pItem->GetDBItem()->GetRecordID(), false );
		}
		// рюкзак
		const vector<NRPG::SBackPackItem> &items = pInventory->GetItems();
		for ( vector<NRPG::SBackPackItem>::const_iterator b = items.begin(); b != items.end(); ++b )
			OnScenarioClueTaken( (*b).pItem->GetDBItem()->GetRecordID(), false );
	}
	//
	ExpandFlowChart();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioTracker *CreateScenarioTracker( int nID )
{
	CScenarioTracker *pScenario = new CScenarioTracker();
	if ( nID >= 0 )
		pScenario->CreateScenario( nID );
	return pScenario;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NScenario;
//
REGISTER_SAVELOAD_CLASS( 0x51582120, CScenarioTracker );