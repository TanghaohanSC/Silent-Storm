#include "StdAfx.h"
//
#include "..\Misc\Commands.h"
#include "..\Misc\RandomGen.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\LogStream.h"
//
#include "..\DBFormat\DataScenario.h"
//
#include "scFlowChartItems.h"
#include "scFlowChart.h"
//
#include <fstream>
//
namespace NScenario
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioFlowChart
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioFlowChart::CScenarioFlowChart( int _nScenarioID, bool _bFull ):
	nScenarioID( _nScenarioID ), bFull( _bFull )
{
	/*
	// CScenarioFlowChartState test // SCENARIO AXIS
	ClearItems();
	LoadItems();
	CObj<CScenarioFlowChartState> pState = new CScenarioFlowChartState( this );
	CObj<CScenarioFlowChartState> pNextState = new CScenarioFlowChartState( this );
	(*pState) += GetZoneByName( "FFight" );
	(*pState) += GetZoneByName( "EVlg" );
	*pNextState = *pState;
	*pNextState += GetZoneByName( "Base" );
	bool bRes = false;
	bRes = pState->IsContain( GetZoneByName( "FFight" ) ); // true
	bRes = pState->IsContain( GetZoneByName( "Base" ) ); // false
	bRes = pNextState->IsContain( GetZoneByName( "FFight" ) ); // true
	bRes = pNextState->IsContain( GetZoneByName( "Base" ) ); // true
	bRes = pNextState->IsContain( GetZoneByName( "GCom" ) ); // false
	bRes = pState->IsContain( pNextState ); // false
	bRes = pNextState->IsContain( pState ); // true
	*/
	//
	int n = 0;
	while ( 1 )
	{
		ClearItems();
		LoadItems();
		if ( !CheckDataCorrectness() )
			return;
		GenerateFlowChart();
		if ( bFull || CheckCorrectness() )
			break;
		//
		if ( ++n > 10 )
		{
			#ifdef _MAPEDIT
			MessageBox( 0, "Can't create correct scenario graph", "Warning", MB_OK | MB_ICONWARNING );
			#endif _MAPEDIT
			OutputDebugString( "[SCENARIO TRACKER] Can't create correct scenario graph\n" );
			break;
		}
	}
	//
	if ( CheckCorrectness() )
	{
		CalculateDifficulties();
		SetDistance();
		MarkInaccessible();
		MarkPassed();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChart::CheckDataCorrectness()
{
	bool bRes = true;
	//
	if ( !IsValid( GetZoneByName( "BASE" ) ) )
		bRes = false;
	if ( !IsValid( GetZoneByName( "FFIGHT" ) ) )
		bRes = false;
	//
	if ( !bRes )
		OutputDebugString( "[SCENARIO TRACKER] incorrect data for scenario graph.\n" );
	//
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::ClearItems()
{
	zones.clear();
	clues.clear();
	objectives.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScenarioFlowChart::GetTemplateIDByVariantID( int nVariantID )
{
	for ( vector< CObj<CScenarioZone> >::iterator i = zones.begin(); i != zones.end(); ++i )
	{
		int nRes = (*i)->GetTemplateIDByVariantID( nVariantID );
		if ( nRes > 0 )
			return nRes;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone* CScenarioFlowChart::GetZoneByTemplateID( int nTemplateID )
{
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
	{
		vector<int> &templatesIDs = (*zone)->GetDBZone()->templatesIDs;
		if ( find( templatesIDs.begin(), templatesIDs.end(), nTemplateID ) != templatesIDs.end() )
			return *zone;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioObjective* CScenarioFlowChart::GetObjectiveByDBObjective( NDb::CDBScenarioObjective *pDBObjective )
{
	ASSERT( IsValid( pDBObjective ) );
	if ( !IsValid( pDBObjective ) )
		return 0;
	//
	for ( vector< CObj<CScenarioObjective> >::iterator objective = objectives.begin(); 
		objective != objectives.end(); ++objective )
		if ( (*objective)->GetDBObjective() == pDBObjective )	
			return *objective;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone *CScenarioFlowChart::GetZoneByName( string _szName )
{
	string szName = _szName;
	NStr::ToUpper( szName );
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
	{
		string szZoneName = (*zone)->GetDBZone()->sSmallDescription;
		NStr::ToUpper( szZoneName );
		if ( szZoneName == szName )
			return *zone;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone *CScenarioFlowChart::GetZoneByDBZone( NDb::CDBScenarioZone *pDBZone )
{
	ASSERT( IsValid( pDBZone ) );
	if ( !IsValid( pDBZone ) )
		return 0;
	//
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
		if ( (*zone)->GetDBZone() == pDBZone )
			return *zone;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioClue *CScenarioFlowChart::GetClueByName( string _szName )
{
	string szName = _szName;
	NStr::ToUpper( szName );
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
	{
		string szClueName = (*clue)->GetDBClue()->sSmallDescription;
		NStr::ToUpper( szClueName );
		if ( szClueName == szName )
			return *clue;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioClue *CScenarioFlowChart::GetClueByItemID( int nItemID )
{
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->GetDBClue()->nItemID == nItemID )
			return *clue;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioClue *CScenarioFlowChart::GetClueByPersID( int nPersID )
{
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->GetDBClue()->nPersID == nPersID )
			return *clue;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioClue *CScenarioFlowChart::GetClueByDBClue( NDb::CDBScenarioClue *pDBClue )
{
	ASSERT( IsValid( pDBClue ) );
	if ( !IsValid( pDBClue ) )
		return 0;
	//
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->GetDBClue() == pDBClue )
			return *clue;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::PlaceClue( CScenarioClue *pClue )
{
	ASSERT( IsValid( pClue ) );
	if ( pClue->IsPlaced() )
		return;
	//
	vector< CPtr<CScenarioZone> > possibleZones;
	CDBPtr<NDb::CDBScenarioClue> pDBClue = pClue->GetDBClue();
	for ( vector< CPtr<NDb::CDBScenarioZone> >::iterator dbZone = pDBClue->zonesToPlace.begin(); 
		dbZone != pDBClue->zonesToPlace.end(); ++dbZone )
	{
		if ( IsValid( *dbZone ) )
		{
			CPtr<CScenarioZone> pZone = GetZoneByDBZone( *dbZone );
			if ( !IsValid( pZone ) )
				continue;

			if ( bFull || pDBClue->bPermanent || pZone->CanPlaceClue( pDBClue->clueType ) )
				possibleZones.push_back( pZone );
		}
	}
	//
	int nSize = possibleZones.size();
	if ( nSize > 0 )
	{
		if ( bFull)
			for ( int i = 0; i < nSize; ++i )
				possibleZones[i]->PlaceClue( pClue );
		else
			possibleZones[ random.Get( 0, nSize ) ]->PlaceClue( pClue );
		//
		possibleZones.clear();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::PlaceObjective( CScenarioObjective *pObjective )
{
	ASSERT( IsValid( pObjective ) );
	ASSERT( !pObjective->IsPlaced() );
	//
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->IsPlaced() && (*clue)->CanPlaceObjective( pObjective ) )
		{
			(*clue)->PlaceObjective( pObjective );
			//
			for ( vector< CPtr<CScenarioClue> >::iterator childClue = pObjective->clues.begin();
				childClue != pObjective->clues.end(); ++childClue )
			{
				(*childClue)->SetPlaced();
				(*childClue)->SetCompound();
			}
			//
			return;
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::SetupLinks( CScenarioObjective *pObjective )
{
	ASSERT( IsValid( pObjective ) );
	//
	if ( pObjective->IsPlaced() )
	{
		// размещаем указатели на открываемые зоны
		pObjective->zones.clear();
		vector< CPtr<NDb::CDBScenarioZone> > &zonesToOpen = pObjective->GetDBObjective()->zonesToOpen;
		for ( vector< CPtr<NDb::CDBScenarioZone> >::iterator zone = zonesToOpen.begin();
			zone != zonesToOpen.end(); ++zone )
		{
			if ( IsValid( *zone ) )
			{
				CPtr<CScenarioZone> pZone = GetZoneByDBZone( *zone );
				if ( IsValid( pZone ) )
				{
					++( pZone->nParentObj );
					pObjective->zones.push_back( pZone );
				}
			}
		}
		// размещаем указатели на блокируемые зоны
		pObjective->zonesToBlock.clear();
		vector< CPtr<NDb::CDBScenarioZone> > &zonesToBlock = pObjective->GetDBObjective()->zonesToBlock;
		for ( vector< CPtr<NDb::CDBScenarioZone> >::iterator zone = zonesToBlock.begin();
			zone != zonesToBlock.end(); ++zone )
		{
			if ( IsValid( *zone ) )
			{
				CPtr<CScenarioZone> pZone = GetZoneByDBZone( *zone );
				if ( IsValid( pZone ) )
				{
					pObjective->zonesToBlock.push_back( pZone );
					pZone->AddBlocker( pObjective );
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::GenerateFlowChart()
{
	// размещаем clue-сы
	vector< CPtr<CScenarioClue> > cluesToPlace;
	for ( vector< CObj<CScenarioClue> >::const_iterator i = clues.begin(); i != clues.end(); ++i )
		cluesToPlace.push_back( (*i).GetPtr() );
	while ( !cluesToPlace.empty() )
	{
		int n = 0;
		if ( !bFull )
			n = random.Get( 0, cluesToPlace.size() );
		PlaceClue( cluesToPlace[ n ] );
		cluesToPlace.erase( cluesToPlace.begin() + n );
	}
	// размещаем objective-ы
	for ( vector< CObj<CScenarioObjective> >::iterator objective = objectives.begin(); 
		objective != objectives.end(); ++objective )
			PlaceObjective( *objective );
	// размещаем objective-ы для состовных clues-ов
	for ( vector< CObj<CScenarioObjective> >::iterator objective = objectives.begin(); 
		objective != objectives.end(); ++objective )
			if ( !(*objective)->IsPlaced() )
				PlaceObjective( *objective );
	//
	for ( vector< CObj<CScenarioObjective> >::iterator objective = objectives.begin(); 
		objective != objectives.end(); ++objective )
			SetupLinks( *objective );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::RemoveClue( CScenarioClue *pClue )
{
	pClue->SetPlaced( false );
	pClue->ClearLinks();
	OutputDebugString( "[SCENARIO TRACKER] clue was removed\n" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::RemoveIncorrectClues()
{
	if ( bFull )
		return;
	//
	while ( true )
	{
		vector< CPtr<CScenarioClue> > cluesToErase;
		for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
			if ( (*clue)->IsPlaced() )
			{
				if ( (*clue)->IsCompound() )
				{
					int nParentCount = 0;
					// проверяем, что на составной clue указывает хотя-бы минимальное кол-во составляющих
					for ( vector< CPtr<CScenarioObjective> >::iterator objective = (*clue)->parentObjectives.begin(); 
						objective != (*clue)->parentObjectives.end(); ++objective )
						if ( (*objective)->IsCorrect() && (*objective)->pParentClue->IsPlaced() )
							++nParentCount;
					//
					if ( nParentCount < GetMinParentToOpen( *clue ) )
						cluesToErase.push_back( (*clue).GetPtr() );
				} 
			}
		//
		for ( vector< CPtr<CScenarioClue> >::iterator clue = cluesToErase.begin(); clue != cluesToErase.end(); ++clue )
				RemoveClue( *clue );
		//
		bool bFinished = cluesToErase.empty();
		cluesToErase.clear();
		if ( bFinished )
			return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::LoadItems()
{
	zones.clear();
	clues.clear();
	objectives.clear();
	// загружаем зоны
	int nInnerID = 0;
	CDBTable<NDb::CDBScenarioZone> *pZoneDBTable = NDatabase::GetTable<NDb::CDBScenarioZone>();
	CDBIterator<NDb::CDBScenarioZone> zone(*pZoneDBTable);
	while ( pZoneDBTable && zone.MoveNext() )
	{
		CDBPtr<NDb::CDBScenarioZone> pDBZone = zone.Get();
		if ( IsValid( pDBZone ) && IsValid( pDBZone->pScenario ) && 
			pDBZone->pScenario->GetRecordID() == nScenarioID )
		{
			zones.push_back( CreateScenarioZone( pDBZone, nInnerID ) );
			++nInnerID;
		}
	}
	// загружаем улики
	CDBTable<NDb::CDBScenarioClue> *pClueDBTable = NDatabase::GetTable<NDb::CDBScenarioClue>();
	CDBIterator<NDb::CDBScenarioClue> clue(*pClueDBTable);
	while ( pClueDBTable && clue.MoveNext() )
	{
		CDBPtr<NDb::CDBScenarioClue> pDBClue = clue.Get();
		if ( IsValid( pDBClue ) && IsValid( pDBClue->pScenario ) && 
			pDBClue->pScenario->GetRecordID() == nScenarioID )
				clues.push_back( CreateScenarioClue( pDBClue ) );
	}
	// загружаем действия
	CDBTable<NDb::CDBScenarioObjective> *pObjectiveDBTable = NDatabase::GetTable<NDb::CDBScenarioObjective>();
	CDBIterator<NDb::CDBScenarioObjective> objective(*pObjectiveDBTable);
	while ( pObjectiveDBTable && objective.MoveNext() )
	{
		CDBPtr<NDb::CDBScenarioObjective> pDBObjective = objective.Get();
		if ( IsValid( pDBObjective ) && IsValid( pDBObjective->pScenario ) && 
			pDBObjective->pScenario->GetRecordID() == nScenarioID )
				objectives.push_back( CreateScenarioObjective( pDBObjective ) );
	}
	// загружаем ссылки на составные clue
	CDBTable<NDb::CDBScenarioObjective2Clue> *pObjective2ClueDBTable = NDatabase::GetTable<NDb::CDBScenarioObjective2Clue>();
	CDBIterator<NDb::CDBScenarioObjective2Clue> objective2clue(*pObjective2ClueDBTable);
	while ( pObjective2ClueDBTable && objective2clue.MoveNext() )
	{
		CDBPtr<NDb::CDBScenarioObjective2Clue> pDBObjective = objective2clue.Get();
		CPtr<CScenarioObjective> pObjective = GetObjectiveByDBObjective( pDBObjective->pObjective );
		CPtr<CScenarioClue> pClue = GetClueByDBClue( pDBObjective->pClue );
		if ( !IsValid( pObjective ) || !IsValid( pClue ) )
			continue; // не относятся к загружаемому сценарию
		//
		bool bNoError = find( pObjective->clues.begin(), pObjective->clues.end(), pClue ) == pObjective->clues.end();
		ASSERT( bNoError ); // ошибка в базе
		if ( !bNoError )
			continue;
		bNoError = find( pClue->parentObjectives.begin(), pClue->parentObjectives.end(), pObjective ) ==
			pClue->parentObjectives.end();
		ASSERT( bNoError ); // ошибка в базе
		if ( !bNoError )
			continue;
		//
		pObjective->clues.push_back( pClue );
		pClue->parentObjectives.push_back( pObjective );
	}
	//
	//DEBUG{
	char szStr[128];
	sprintf( szStr, "[SCENARIO TRACKER] %d zones loaded\n", zones.size() );
	OutputDebugString( szStr );
	sprintf( szStr, "[SCENARIO TRACKER] %d clues loaded\n", clues.size() );
	OutputDebugString( szStr );
	sprintf( szStr, "[SCENARIO TRACKER] %d objectives loaded\n", objectives.size() );
	OutputDebugString( szStr );
	//DEBUG}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::GetParentClues( CScenarioClue *pClue, 
	const vector< CPtr<CScenarioObjective> > _objectives, vector< CPtr<CScenarioClue> > *pClues )
{
	pClues->clear();
	//
	vector< CPtr<CScenarioObjective> > parentObjectives;
	for ( vector< CPtr<CScenarioObjective> >::const_iterator i = _objectives.begin();
		i != _objectives.end(); ++i )
	{
		if ( find( (*i)->clues.begin(), (*i)->clues.end(), pClue ) != (*i)->clues.end() )
			parentObjectives.push_back( (*i).GetPtr() );
	}
	//
	for ( vector< CPtr<CScenarioObjective> >::const_iterator i = parentObjectives.begin();
		i != parentObjectives.end(); ++i )
			if ( find( pClues->begin(), pClues->end(), (*i)->pParentClue ) == pClues->end() )
				pClues->push_back( (*i)->pParentClue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::GetObjectives( vector< CPtr<CScenarioObjective> > *pObjectives )
{
	pObjectives->clear();
	for ( vector< CObj<CScenarioObjective> >::iterator i = objectives.begin(); 
		i != objectives.end(); ++i )
			pObjectives->push_back( (*i).GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScenarioFlowChart::GetMinParentToOpen( CScenarioClue *pClue )
{
	ASSERT( IsValid( pClue ) );
	if ( !IsValid( pClue ) )
		return 0;
	//
	if ( pClue->GetDBClue()->nMinParentToOpen > 0 )
		return pClue->GetDBClue()->nMinParentToOpen;
	else
	{
		vector< CPtr<CScenarioClue> > parentClues;
		vector< CPtr<CScenarioObjective> > parentObjectives;
		GetObjectives( &parentObjectives );
		GetParentClues( pClue, parentObjectives, &parentClues );
		return parentClues.size();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::InitClueLock( CScenarioClue *pClue ) 
{
	ASSERT( IsValid( pClue ) );
	if ( !IsValid( pClue ) )
		return;
	//
	int nParent = GetMinParentToOpen( pClue );	
	pClue->SetLock( nParent );
	pClue->SetCompound( nParent > 0 );
	pClue->bChildrenUnlocked = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::InitCompoundLock()
{
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->IsPlaced() )
			InitClueLock( *clue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::GetChildZones( CScenarioObjective *pObjective, 
	CScenarioZone *pParentZone, CScenarioClue *pParentClue, list< CPtr<CScenarioZone> > *pChildZones, 
	bool bCheckCompound, bool bUseProcessed, bool bSetDistance, bool bUseBlocks )
{
	if ( !IsValid( pObjective ) )
		return;
	//
	pObjective->bProcessedAsFront = true;
	if ( !IsValid( pObjective->pParentZone ) )
		pObjective->pParentZone = pParentZone;
	if ( bUseBlocks && !pObjective->IsZonesToBlockPassed() )
		return;
	//
	if ( bUseBlocks )
		pObjective->BlockZones( true );
	//
	for ( vector< CPtr<CScenarioZone> >::iterator zone = pObjective->zones.begin();
		zone != pObjective->zones.end(); ++zone )
		if ( IsValid( *zone ) && 
			( !bUseBlocks || !(*zone)->bBlocked ) &&
			( !(*zone)->bProcessed || !bUseProcessed ) )
		{
			if ( *zone != pParentZone && 
				find( pChildZones->begin(), pChildZones->end(), *zone ) == pChildZones->end() )
			{
				(*zone)->bProcessedAsFront = true;
				if ( bUseProcessed )
				{
					(*zone)->SetPassingParentClue( pParentClue );
					(*zone)->pParentZone = pParentZone;
					(*zone)->bProcessed = true;
					if ( bSetDistance )
						(*zone)->nDistance = nCurrentDistance;
				}
				//
				pChildZones->push_back( *zone );
			}
		}
	//
	for ( vector< CPtr<CScenarioClue> >::iterator clue = pObjective->clues.begin(); clue != pObjective->clues.end(); ++clue )
		GetChildZones( (*clue), pParentZone, pParentClue, pChildZones, bCheckCompound, bUseProcessed, bSetDistance, bUseBlocks );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::GetChildZones( CScenarioClue *pClue, 
	CScenarioZone *pParentZone, CScenarioClue *pParentClue, list< CPtr<CScenarioZone> > *pChildZones, 
	bool bCheckCompound, bool bUseProcessed, bool bSetDistance, bool bUseBlocks )
{
	if ( pClue->IsPlaced() && ( !pClue->IsLocked() || !bCheckCompound ) )
	{
		if ( bUseProcessed )
			pClue->SetPassingParentZone( pParentZone );
		//
		list< CPtr<CScenarioClue> > unlockedClues;
		for ( vector< CPtr<CScenarioObjective> >::iterator objective = pClue->objectives.begin();
			objective != pClue->objectives.end(); ++objective )
		{
			if ( IsValid( *objective ) && (*objective)->IsPlaced() )
			{
				// учет lock-ов ведется по clue
				if ( bCheckCompound && !pClue->bChildrenUnlocked )
				{
					pClue->bChildrenUnlocked = true;
					//
					for ( vector< CPtr<CScenarioClue> >::iterator clue = (*objective)->clues.begin(); 
						clue != (*objective)->clues.end(); ++clue )
						if ( (*clue)->IsPlaced() && 
							find( unlockedClues.begin(), unlockedClues.end(), *clue ) == unlockedClues.end() )
						{
							unlockedClues.push_back( *clue );
							(*clue)->DecLock();
							if ( bUseProcessed )
								(*clue)->SetPassingParentClue( pClue );
						}
				}
				//
				GetChildZones( *objective, pParentZone, pClue, pChildZones, bCheckCompound, bUseProcessed, bSetDistance, bUseBlocks );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::GetChildZones( CScenarioZone *pZone, 
	list< CPtr<CScenarioZone> > *pChildZones, bool bCheckCompound, 
	bool bUseProcessed, bool bSetDistance, bool bUseBlocks )
{
	if ( bUseBlocks && pZone->bBlocked )
		return;
	//
	pZone->bProcessedAsFront = true;
	for ( vector< CPtr<CScenarioClue> >::iterator clue = pZone->clues.begin();
		clue != pZone->clues.end(); ++clue )
			GetChildZones( *clue, pZone, 0, pChildZones, bCheckCompound, bUseProcessed, bSetDistance, bUseBlocks );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::InitPathFinding()
{
	for ( vector< CObj<CScenarioZone> >::iterator i = zones.begin(); i != zones.end(); ++i )
	{
		(*i)->pParentZone = 0;
		(*i)->bProcessed = false;
		(*i)->bProcessedAsFront = false;
		(*i)->bBlocked = false;
	}
	for ( vector< CObj<CScenarioObjective> >::iterator i = objectives.begin(); i != objectives.end(); ++i )
	{
		(*i)->bProcessedAsFront = false;
		(*i)->pParentZone = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static CScenarioZone *pFinishZone;
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsFinal_Finish( CScenarioZone *pZone )
{
	return pZone == pFinishZone;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsFinal_Calculated( CScenarioZone *pZone )
{
	return pZone->bDifCalculated;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsFinal_Inaccessible( CScenarioZone *pZone )
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChart::GetPath( CScenarioZone *pBegin, 
	list< CPtr<CScenarioZone> > *pPath, bool (* IsFinal)( CScenarioZone *pZone ), 
	bool bCheckCompound, bool bSetDistance, bool bUseBlocks )
{
	pPath->clear();
	//
	list< CPtr<CScenarioZone> > front;
	list< CPtr<CScenarioZone> > newFront;
	list< CPtr<CScenarioZone> > final;
	//
	InitCompoundLock();
	InitPathFinding();
	//
	nCurrentDistance = 0;
	pFinishZone = GetZoneByName( "FFIGHT" );
	pBegin->bProcessed = true;
	pBegin->nDistance = nCurrentDistance;
	front.push_back( pBegin );
	//
	bool bPathFound = false;
	while ( !bPathFound )
	{
		for ( list< CPtr<CScenarioZone> >::iterator zone = front.begin(); zone != front.end(); ++zone )
      if ( IsFinal( *zone ) )
			{
				final.push_back( *zone );
				bPathFound = true;
			}
		//
		if ( !bPathFound )
		{
			if ( front.empty() )
				return false;
			//
			++nCurrentDistance;
			for ( list< CPtr<CScenarioZone> >::iterator zone = front.begin(); zone != front.end(); ++zone )
				GetChildZones( *zone, &newFront, bCheckCompound, true, bSetDistance, bUseBlocks );
			//
			if ( bUseBlocks )
			{
				for ( list< CPtr<CScenarioZone> >::iterator zone = newFront.begin(); zone != newFront.end(); ++zone )
					for ( vector< CPtr<CScenarioObjective> >::iterator i = (*zone)->blockers.begin(); i != (*zone)->blockers.end(); ++i )
						if ( (*i)->bProcessedAsFront )
							GetChildZones( *i, (*i)->pParentZone, (*i)->pParentClue, &newFront, bCheckCompound, true, bSetDistance, bUseBlocks );
			}
		}
		//
		front.clear();
		front = newFront;
		newFront.clear();
	}
	// выбираем зону с наименьшей посчитанной сложностью
	int nMinDif = 0xFFFF;
	CPtr<CScenarioZone> pZone = 0;
	for ( list< CPtr<CScenarioZone> >::iterator zone = final.begin(); zone != final.end(); ++zone )
		if ( (*zone)->bDifCalculated && (*zone)->nDifficulty < nMinDif )
		{
			nMinDif = (*zone)->nDifficulty;
			pZone = *zone;
		}
	if ( !IsValid( pZone ) )
		pZone = final.front();
	// записываем путь
	for ( CPtr<CScenarioZone> pCurrent = pZone; IsValid( pCurrent ); pCurrent = pCurrent->pParentZone )
		pPath->push_front( pCurrent );
	//
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::SetDistance()
{
	list< CPtr<CScenarioZone> > path;
	GetPath( GetZoneByName( "BASE" ), &path, IsFinal_Inaccessible, true, true, true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::MarkPassed()
{
	list< CPtr<CScenarioZone> > path;
	GetPath( GetZoneByName( "BASE" ), &path, IsFinal_Finish, true, false, true );
	//
	list< CPtr<CScenarioZone> > zonesFront;
	list< CPtr<CScenarioClue> > cluesFront;
	list< CPtr<CScenarioZone> > newZonesFront;
	list< CPtr<CScenarioClue> > newCluesFront;
	//
	zonesFront.push_back( GetZoneByName( "FFIGHT" ) );
	//
	while ( !( zonesFront.empty() && cluesFront.empty() ) )
	{
		for ( list< CPtr<CScenarioZone> >::iterator i = zonesFront.begin();
			i != zonesFront.end(); ++i )
		{
			(*i)->bPassed = true;
			//
			if ( IsValid( (*i)->pPassingParentClue ) )
				if ( find( newCluesFront.begin(), 
					newCluesFront.end(), (*i)->pPassingParentClue ) == newCluesFront.end() )
						newCluesFront.push_back( (*i)->pPassingParentClue );
		}
		//
		for ( list< CPtr<CScenarioClue> >::iterator i = cluesFront.begin();
			i != cluesFront.end(); ++i )
		{
			(*i)->bPassed = true;
			//
			if ( IsValid( (*i)->pPassingParentZone ) )
				if ( find( newZonesFront.begin(), 
					newZonesFront.end(), (*i)->pPassingParentZone ) == newZonesFront.end() )
						newZonesFront.push_back( (*i)->pPassingParentZone );
			//
			CScenarioClue *pClue = *i;
			int nPassingParents = (*i)->passingParentClues.size();
			int nMinParentToOpen = GetMinParentToOpen( *i );
			ASSERT( nPassingParents >= nMinParentToOpen );
			int n = Min( nPassingParents, nMinParentToOpen );
			for ( int k = 0; k < n; ++k )
				if ( find( newCluesFront.begin(), 
					newCluesFront.end(), (*i)->passingParentClues[k] ) == newCluesFront.end() )
						newCluesFront.push_back( (*i)->passingParentClues[k] );
		}
		//
		zonesFront.clear(); cluesFront.clear();
		zonesFront = newZonesFront;
		cluesFront = newCluesFront;
		newZonesFront.clear(); newCluesFront.clear();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::MarkInaccessible()
{
	list< CPtr<CScenarioZone> > path;
	GetPath( GetZoneByName( "BASE" ), &path, IsFinal_Inaccessible, true, false, true );
	//
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
		if ( (*zone)->bProcessed )
		{
			(*zone)->bInaccessible = false;
			(*zone)->SetCluesInaccessible( false );
		}
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->IsCompound() && !(*clue)->IsLocked() )
			(*clue)->bInaccessible = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::InitDifficulties()
{
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
	{
		list< CPtr<CScenarioZone> > childZones;
		GetChildZones( *zone, &childZones, false, false, false, false );
		if ( childZones.empty() && (*zone)->nParentObj > 0 )
		{
			(*zone)->nDifficulty = 13;
			(*zone)->bDifCalculated = true;
		}
		else
		{
			(*zone)->nDifficulty = 1;
			(*zone)->bDifCalculated = false;
		}
	}
	//
	GetZoneByName( "FFIGHT" )->nDifficulty = 15;
	GetZoneByName( "BASE" )->nParentObj = 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::SetPathDifficulty( const list< CPtr<CScenarioZone> > &path, bool bMark, bool bCheckBegin )
{
	float fBegin;
	float fEnd = path.back()->nDifficulty;
	if ( bCheckBegin )
		fBegin = path.front()->nDifficulty;
	else
		fBegin = max( 1, fEnd - path.size() + 1 );
	float fStep = ( fEnd - fBegin ) / ( path.size() - 1 );
	float fCurrent = fBegin;
	for ( list< CPtr<CScenarioZone> >::const_iterator zone = path.begin(); zone != path.end(); ++zone )
	{
		(*zone)->nDifficulty = max( 1, fCurrent );
		(*zone)->bDifCalculated = true;
		if ( bMark )
			(*zone)->bMarked = true;
		fCurrent += fStep;
	}
	//
	path.back()->nDifficulty = fEnd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChart::GetUnprocessedEdge( CScenarioZone **ppBegin, CScenarioZone **ppEnd )
{
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
		if ( (*zone)->bDifCalculated )
		{
			list< CPtr<CScenarioZone> > childZones;
			GetChildZones( *zone, &childZones,	false, false, false, false );
			for ( list< CPtr<CScenarioZone> >::iterator child = childZones.begin();
				child != childZones.end(); ++child )
			{
				if ( !(*child)->bDifCalculated )
				{
					*ppBegin = *zone;
					*ppEnd = *child;
					return true;
				}
			}
		}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::CalculateDifficulties()
{
	InitDifficulties();
	//
	list< CPtr<CScenarioZone> > path;
	if ( !GetPath( GetZoneByName( "BASE" ), &path, IsFinal_Finish, true, false, true ) )
	{
		ASSERT( 0 );
		OutputDebugString( "[SCENARIO TRACKER] Can't calculate difficulties. No Path from BASE to FFIGHT.\n" );
		return;
	}
	SetPathDifficulty( path, true, true );
	//
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
		if ( (*zone)->nParentObj == 0 )
		{
			if ( GetPath( *zone, &path, IsFinal_Calculated, false, false, false ) )
				SetPathDifficulty( path, false, false );
		}
	//
	CScenarioZone *pBegin;
	CScenarioZone *pEnd;
	while ( GetUnprocessedEdge( &pBegin, &pEnd ) )
	{
		if ( GetPath( pEnd, &path, IsFinal_Calculated, false, false, false ) )
		{
			path.push_front( pBegin );
			SetPathDifficulty( path, false, true );
		}
		else
		{
			ASSERT( 0 );
			OutputDebugString( "[SCENARIO TRACKER] Can't set difficulties\n" );
			break;
		}
	}
	// окончательная проверка
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
		if ( !(*zone)->bDifCalculated )
		{
			OutputDebugString( "[SCENARIO TRACKER] Bad cycle found\n" );
			if ( GetPath( *zone, &path, IsFinal_Calculated, false, false, false ) )
				SetPathDifficulty( path, false, false );
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::BlockZones( bool bBlock )
{
	for ( vector< CObj<CScenarioObjective> >::iterator i = objectives.begin(); i != objectives.end(); ++i )	
		(*i)->BlockZones( bBlock );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChart::CheckCorrectness()
{
	list< CPtr<CScenarioZone> > path;
	bool bCorrect = GetPath( GetZoneByName( "BASE" ), &path, IsFinal_Finish, true, false, true );
	if ( !bFull )
		bCorrect &= path.size() >= 6;
	//DEBUG{
	if ( bCorrect )
		OutputDebugString( "[SCENARIO TRACKER] Flowchart correct\n" );
	else
		OutputDebugString( "[SCENARIO TRACKER] Flowchart >IN<correct\n" );
	//DEBUG}
	return bCorrect;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::Header( fstream &file, string szFileName )
{
	string str = szFileName + ".dot";
	file.open( str.c_str(), ios_base::out | ios_base::trunc );
	file << "digraph g\n{\n  concentrate=true;\n  nodesep=.3;\n  ranksep=.3;\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::Footer( fstream &file, string szFileName )
{
	file << "}";
	file.close();
	//
	string str = "scengraph\\dot.exe -Tjpg " + szFileName + ".dot -o " + szFileName + ".jpg";
	system( str.c_str() );
}		
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::DrawZones( fstream &file, 
	const list< CPtr<CScenarioZone> > &availableZones )
{
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
	{
		bool bAvailable = ( find( availableZones.begin(), 
			availableZones.end(), (*zone).GetPtr() ) != availableZones.end() );
		(*zone)->DrawNode( file, 0, bAvailable, true );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::DrawClues( fstream &file, 
	const list< CPtr<CScenarioClue> > &availableClues )
{
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->IsPlaced() )
		{
			bool bAvailable = ( find( availableClues.begin(), 
				availableClues.end(), (*clue).GetPtr() ) != availableClues.end() );
			(*clue)->DrawNode( file, bAvailable );
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::DrawZonesRelationships( fstream &file )
{
	for ( vector< CObj<CScenarioZone> >::iterator zone = zones.begin(); zone != zones.end(); ++zone )
		(*zone)->DrawRelationship( file );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::DrawCluesRelationships( fstream &file )
{
	for ( vector< CObj<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->IsPlaced() )
			(*clue)->DrawRelationship( file );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::DrawFlowChart( const list< CPtr<CScenarioZone> > &availableZones, 
	const list< CPtr<CScenarioClue> > &availableClues, const char *pszOutputFile )
{
	fstream file;
	string szFile = pszOutputFile ? pszOutputFile : "scengraph\\scenario";
	Header( file, szFile );
	DrawZones( file, availableZones );
	DrawClues( file, availableClues );
	DrawZonesRelationships( file );
	DrawCluesRelationships( file );
	Footer( file, szFile );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DrawRegularNode( fstream &file, CScenarioZone *pParent, CScenarioZone *pZone, bool bAvailable )
{
	pZone->DrawNode( file, 0, bAvailable );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DrawRefNode( fstream &file, CScenarioZone *pParent, CScenarioZone *pZone, int nID, bool bAvailable )
{
	pZone->DrawNode( file, nID, bAvailable );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DrawRegularRelationship( fstream &file, CScenarioZone *pParent, CScenarioZone *pZone, bool bAvailable )
{
	if ( IsValid( pParent ) )
	{
		file << "  " << pParent->GetDBZone()->sSmallDescription;
		file << " -> " << pZone->GetDBZone()->sSmallDescription << ";" << endl;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DrawRefRelationship( fstream &file, CScenarioZone *pParent, CScenarioZone *pZone, int nID, bool bAvailable )
{
	if ( IsValid( pParent ) )
	{
		file << "  " << pParent->GetDBZone()->sSmallDescription;
		file << " -> " << pZone->GetDBZone()->sSmallDescription << "_" << nID << ";" << endl;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::DrawPassingFlowChart( const list< CPtr<CScenarioZone> > &availableZones )
{
	fstream file;
	Header( file, "scengraph\\passing" );
	PassFlowChart( file, availableZones, DrawRegularNode, DrawRefNode );
	PassFlowChart( file, availableZones, DrawRegularRelationship, DrawRefRelationship );
	Footer( file, "scengraph\\passing" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::PassFlowChart( fstream &file, const list< CPtr<CScenarioZone> > &availableZones,
	void ( * RegularNode )( fstream &file, CScenarioZone *pParent, CScenarioZone *pZone, bool bAvailable ), 
	void ( * RefNode )( fstream &file, CScenarioZone *pParent, CScenarioZone *pZone, int nID, bool bAvailable ) )
{
	int nID = 1;
	list< CPtr<CScenarioZone> > front;
	list< CPtr<CScenarioZone> > newFront;
	//
	InitCompoundLock();
	InitPathFinding();
	//
	CPtr<CScenarioZone> pBase = GetZoneByName( "BASE" );
	RegularNode( file, 0, pBase, true );
	front.push_back( pBase );
	//
	while ( !front.empty() )
	{
		for ( list< CPtr<CScenarioZone> >::iterator zone = front.begin(); zone != front.end(); ++zone )
		{
			list< CPtr<CScenarioZone> > regularChilds;
			list< CPtr<CScenarioZone> > refChilds;
			//
			GetChildZones( *zone, &regularChilds, true, true, false, false );
			GetChildZones( *zone, &refChilds, false, false, false, false );
			//
			for ( list< CPtr<CScenarioZone> >::iterator child = regularChilds.begin(); 
				child != regularChilds.end(); ++child )
			{
				RegularNode( file, *zone, *child, 
					find( availableZones.begin(), availableZones.end(), (*child).GetPtr() ) != availableZones.end() );
				newFront.push_back( *child );
			}
			//
			for ( list< CPtr<CScenarioZone> >::iterator child = refChilds.begin(); 
				child != refChilds.end(); ++child )
			{
				if ( find( regularChilds.begin(), regularChilds.end(), *child ) == regularChilds.end() )
				{
					regularChilds.push_back( *child ); // на каждый узел только одна ссылка
					RefNode( file, *zone, *child, nID, 
						find( availableZones.begin(), availableZones.end(), (*child).GetPtr() ) != availableZones.end() );
					++nID;
				}
			}
		}
		//
		front.clear();
		front = newFront;
		newFront.clear();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::PrintScenarioList()
{
	csSystem << "Available scenarios:" << endl;
	CDBTable<NDb::CDBScenario> *pDBScenarioTable = NDatabase::GetTable<NDb::CDBScenario>();
	CDBIterator<NDb::CDBScenario> scenario(*pDBScenarioTable);
	while ( pDBScenarioTable && scenario.MoveNext() )
		csSystem << "\t" << scenario.Get()->szName << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChart::GetZonesWhichCanBeOpened( CScenarioClue *pClue, 
	list< CPtr<CScenarioZone> > *pZones )
{
	ASSERT( IsValid( pClue ) );
	ASSERT( pZones != 0 );
	if ( !IsValid( pClue ) || pZones == 0 )
		return;
	//
	GetChildZones( pClue, 0,  0, pZones, false, false, false, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioFlowChart *CreateScenarioFlowChart( int nScenarioID, bool bFull )
{
	return new CScenarioFlowChart( nScenarioID, bFull );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioFlowChart *CreateScenarioFlowChart( string szScenarioName, bool bFull )
{
	int nScenarioID = 0;
	//
	CDBTable<NDb::CDBScenario> *pDBScenarioTable = NDatabase::GetTable<NDb::CDBScenario>();
	CDBIterator<NDb::CDBScenario> scenario(*pDBScenarioTable);
	while ( pDBScenarioTable && scenario.MoveNext() )
	{
		CDBPtr<NDb::CDBScenario> pDBScenario = scenario.Get();
		string szUpperScenarioName = szScenarioName; NStr::ToUpper( szUpperScenarioName );
		string szUpperName = pDBScenario->szName; NStr::ToUpper( szUpperName );
		if ( IsValid( pDBScenario ) && szUpperName == szUpperScenarioName )
			return CreateScenarioFlowChart( pDBScenario->GetRecordID(), bFull );
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioFlowChartState
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioFlowChartState::CScenarioFlowChartState( CScenarioFlowChart *_pFlowChart ):
	pFlowChart( _pFlowChart ), bCalculateSignature( true ), pFrontZone( 0 )
{
	ASSERT( IsValid( pFlowChart ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioFlowChartState::CScenarioFlowChartState( CScenarioFlowChartState *_pState )
{
	CopyState( *_pState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartState::CopyState( CScenarioFlowChartState state )
{
	pFlowChart = state.GetFlowChart();
	pFrontZone = state.GetFrontZone();
	state.GetSignature( &signature );
	state.GetZones( &zones );
	bCalculateSignature = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChartState::IsContain( CScenarioZone *pZone )
{
	ASSERT( IsValid( pZone ) );
	if ( !IsValid( pZone ) )
		return false;
	//
	UpdateSignature();
	return ( signature[ pZone->GetInnerID() / 32 ] & ( 1 << ( pZone->GetInnerID() % 32 ) ) ) > 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChartState::IsContainState( CScenarioFlowChartState *pState )
{
	ASSERT( IsValid( pState ) );
	if ( !IsValid( pState ) )
		return false;
	//
	UpdateSignature();
	vector<DWORD> stateSignature;
	pState->GetSignature( &stateSignature );
	if ( stateSignature.size() != signature.size() )
		return false;
  for ( int i = 0; i < stateSignature.size(); ++i )
		if ( ( stateSignature[i] & signature[i] ) != stateSignature[i] )
			return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartState::GetZones( list< CPtr<CScenarioZone> > *pZones ) const
{
	ASSERT( pZones != 0 );
	if ( pZones == 0 )
		return;
	//
	pZones->clear();
	for ( list< CPtr<CScenarioZone> >::const_iterator i = zones.begin(); i != zones.end(); ++i )
		pZones->push_back( *i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartState::operator=( CScenarioFlowChartState &state )
{
	CopyState( state );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChartState::operator==( CScenarioFlowChartState &state )
{
	vector<DWORD> stateSignature;
	state.GetSignature( &stateSignature );
	if ( signature.size() != stateSignature.size() )
		return false;
	//
	for ( int i = 0; i < signature.size(); ++i )
		if ( signature[i] != stateSignature[i] )
			return false;
	//
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartState::operator+=( CScenarioZone *pZone )
{
	ASSERT( IsValid( pZone ) );
	ASSERT( !IsContain( pZone ) );
	if ( !IsValid( pZone ) || IsContain( pZone ) )
		return;
	//
	bCalculateSignature = true;
	zones.push_back( pZone );
	pFrontZone = pZone;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartState::UpdateSignature()
{
	if ( bCalculateSignature )
	{
		CalculateSignature();
		bCalculateSignature = false;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartState::GetSignature( vector<DWORD> *pSignature )
{
	UpdateSignature();
	pSignature->resize( signature.size() );
	for ( int i = 0; i < signature.size(); ++i )
		(*pSignature)[i] = signature[i];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartState::CalculateSignature()
{
	signature.resize( pFlowChart->zones.size() / 32 + 1 );
	for ( int i = 0; i < signature.size(); ++i )
		signature[i] = 0;
	for (	list< CPtr<CScenarioZone> >::iterator i = zones.begin(); i != zones.end(); ++i )
		signature[ (*i)->GetInnerID() / 32 ] |= ( 1 << ( (*i)->GetInnerID() % 32 ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioFlowChart *CScenarioFlowChartState::GetFlowChart() const
{
	return pFlowChart;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone *CScenarioFlowChartState::GetFrontZone() const
{
	return pFrontZone;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartState::AddBranchingZone( CScenarioZone *pZone )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScenarioFlowChartState::GetHashKey()
{
	if ( signature.empty() )
		return 0;
	else
		return signature[0];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioFlowChartPathFinder
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioFlowChartPathFinder::CScenarioFlowChartPathFinder(	CScenarioFlowChart *_pFlowChart ):
	pFlowChart( _pFlowChart )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChartPathFinder::IsStatePassed( CScenarioFlowChartState *pState )
{
	list< CPtr<CScenarioFlowChartState> > &states = passedStates[ pState->GetHashKey() ];
	for ( list< CPtr<CScenarioFlowChartState> >::iterator i = states.begin(); i != states.end(); ++i )
		if ( (*i)->IsContainState( pState ) )
			return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartPathFinder::PassState( CScenarioFlowChartState *pState )
{
	if ( !IsStatePassed( pState ) )
	{
		states.push_back( pState );
		passedStates[ pState->GetHashKey() ].push_back( new CScenarioFlowChartState( pState ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioFlowChartPathFinder::AddFinalState( CScenarioFlowChartState *pState )
{
	ASSERT( IsValid( pState ) );
	if ( !IsValid( pState ) )
		return;
	//
	finalStates[ pState->GetHashKey() ].push_back( new CScenarioFlowChartState( pState ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioFlowChartPathFinder::FindPath( CScenarioFlowChartState *pState )
{
	bool bPathFound = false;
	CPtr<CScenarioFlowChartState> pBaseState = new CScenarioFlowChartState( pFlowChart );
	*pBaseState += pFlowChart->GetZoneByName( "Base" );
	PassState( pBaseState );
	//
	//
	return bPathFound;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NScenario;
//
REGISTER_SAVELOAD_CLASS( 0x50982100, CScenarioFlowChart );
REGISTER_SAVELOAD_CLASS( 0x52792140, CScenarioFlowChartState );
REGISTER_SAVELOAD_CLASS( 0x53092110, CScenarioFlowChartPathFinder );