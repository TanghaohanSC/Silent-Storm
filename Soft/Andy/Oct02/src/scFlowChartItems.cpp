#include "StdAfx.h"
//
#include "..\DBFormat\DataScenario.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataFormat.h"
//
#include "MapBuild.h"
//
#include "scFlowChartItems.h"
#include <fstream>
//
namespace NScenario
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioZone
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone::CScenarioZone( NDb::CDBScenarioZone *_pDBZone, int _nInnerID ):
	pDBZone( _pDBZone ), pParentZone( 0 ), nDifficulty( 0 ), bDifCalculated( false ), 
	bProcessed( false ), nParentObj( 0 ), bPassed( false ), bMarked( false ), 
	bInaccessible( true ), nDistance( 0 ), bBlocked( false ), pPassingParentClue( 0 ),
	bProcessedAsFront( false ), nInnerID( _nInnerID ), nOpenOrder( 0 )
{
	ASSERT( IsValid( pDBZone ) );
	//
	for (	vector<int>::iterator i = pDBZone->templatesIDs.begin(); 
		i != pDBZone->templatesIDs.end(); ++i )
	{
		if ( (*i) == 0 )
			continue;
		//
		STemplate t;
		GetVariantInfo( *i, t.sSeed, &t );
		templates[ *i ] = t;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScenarioZone::GetDefaultTemplateID()
{
	if ( pDBZone->templatesIDs.empty() )
		return -1;

	return pDBZone->templatesIDs.front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScenarioZone::GetTemplateIDByVariantID( int nVariantID )
{
	for ( hash_map< int, STemplate >::iterator i = templates.begin();
		i != templates.end(); ++i )
			if ( i->second.nVariantID == nVariantID )
				return i->first;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioZone::GetVariantInfo( int nTemplateID, SRandomSeed sSeed, STemplate *variantData )
{
	ASSERT( variantData != 0 )	;
	ASSERT( nTemplateID > 0 );
	if ( variantData == 0 || nTemplateID <= 0 )
		return;
	//
	CDBPtr<NDb::CTemplate> pTemplate = NDb::GetTemplate( nTemplateID );
	if ( !IsValid( pTemplate ) )
		return;
	//
	SRand rand( sSeed );
	vector<int> intParams;
	int nVariantID = NDb::GetTemplVariant( pTemplate, intParams, -1, &rand )->GetRecordID();
	variantData->nVariantID = nVariantID;
	//
	if ( GetDBZone()->templatesIDs[1] == 0 )
	{
		variantData->nItemSlots = GetDBZone()->nItemSlots;
		variantData->nEmptyItemSlots = GetDBZone()->nItemSlots;
		variantData->nPersonSlots = GetDBZone()->nPersonSlots;
		variantData->nEmptyPersonSlots = GetDBZone()->nPersonSlots;
		variantData->nInventorySlots = 0;
		variantData->nEmptyInventorySlots = 0;
		return;
	}
	//
	SMapInfo info;
	vector<string> strParams;
	if ( BuildMap( nVariantID, strParams, 0, &info, -1, sSeed ) )
	{
		int nItemSlots = 0;
		int nPersonSlots = 0;
		int nInventorySlots = 0;
		for ( vector<SClueSlot>::iterator i = info.slots.begin(); i != info.slots.end(); ++i )
		{
			if ( (*i).bPersSlot )
			{
				++nPersonSlots;
				if ( (*i).bInventorySlot )
					++nInventorySlots;
			}
			else
				++nItemSlots;
		}
		//
		variantData->nItemSlots = nItemSlots;
		variantData->nEmptyItemSlots = nItemSlots;
		variantData->nPersonSlots = nPersonSlots;
		variantData->nEmptyPersonSlots = nPersonSlots;
		variantData->nInventorySlots = nInventorySlots;
		variantData->nEmptyInventorySlots = nInventorySlots;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioZone::SetPassingParentClue( CScenarioClue *pClue )
{
	if ( IsValid( pClue ) && !IsValid( pPassingParentClue ) )
		pPassingParentClue = pClue;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioZone::CanPlaceClue( NDb::EScenarioClueType type )
{
	int nPlaced = 0;
	int nPlacedAll = 0;
	for ( vector< CPtr<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
	{
		if ( !(*clue)->GetDBClue()->bPermanent )
		{
		++nPlacedAll;
		if ( (*clue)->GetDBClue()->clueType == type )
			++nPlaced;
		}
	}
	//
	int nSpace = 0;
	if ( type == NDb::CT_ITEM )
		nSpace = GetDBZone()->nItemSlots;
	else if ( type == NDb::CT_PERSON )
		nSpace = GetDBZone()->nPersonSlots;
	else
		ASSERT( 0 && " Unknown clue type " );
	//
	return nPlaced < nSpace && nPlacedAll < GetDBZone()->nCluesMaxNumber;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScenarioZone::GetTemplateIDForClue( CScenarioClue *pClue )
{
	ASSERT( IsValid( pClue ) );
	if ( !IsValid( pClue ) )
		return 0;
	//
	vector<int> &templatesIDs = GetDBZone()->templatesIDs;
	//
	NDb::EScenarioClueType type = pClue->GetDBClue()->clueType;
	int nMax = 0;
	for ( int i = 0; i < templatesIDs.size(); ++i )
		if ( templatesIDs[i] > 0 )
		{
			if ( type == NDb::CT_PERSON )
				nMax += templates[ templatesIDs[i] ].nEmptyPersonSlots;
			else
				nMax += templates[ templatesIDs[i] ].nEmptyItemSlots;
		}
	//
	if ( nMax <= 0 )
		return 0;
	//
	int k = 0;
	int n = random.Get( 0, nMax );
	while ( n > 0 )
	{
		if ( templatesIDs[k] > 0 )
		{
			if ( type == NDb::CT_PERSON )
				n -= templates[ templatesIDs[k] ].nEmptyPersonSlots;
			else
				n -= templates[ templatesIDs[k] ].nEmptyItemSlots;
		}
		if ( n >= 0 )
			++k;
	}
	//
	ASSERT( k >= 0 && k < templatesIDs.size() );
	if ( !( k >= 0 && k < templatesIDs.size() ) )
		return 0;
	//
	if ( type == NDb::CT_PERSON )
		--templates[ templatesIDs[k] ].nEmptyPersonSlots;
	else
		--templates[ templatesIDs[k] ].nEmptyItemSlots;
	//
	return templatesIDs[k];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioZone::PlaceClue( CScenarioClue *pClue )
{
	ASSERT( IsValid( pClue ) );
	//
	clues.push_back( pClue );
	pClue->SetPlaced();
	pClue->nTemplateID = GetTemplateIDForClue( pClue );
	//
	ASSERT( find( pClue->parentZones.begin(), pClue->parentZones.end(), this ) ==
		pClue->parentZones.end() );
	//
	pClue->parentZones.push_back( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioZone::RemoveClue( CScenarioClue *pClue )
{
	ASSERT( IsValid( pClue ) );
	//
	vector< CPtr<CScenarioClue> >::iterator clue = find( clues.begin(), clues.end(), pClue );
	if ( clue != clues.end() )
		clues.erase( clue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioZone::AddBlocker( CScenarioObjective *pBlocker )
{
	if ( find( blockers.begin(), blockers.end(), pBlocker ) == blockers.end() )
		blockers.push_back( pBlocker );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioZone::DrawNode( fstream &file, int nID, bool bAvailable, bool bDrawBlockers )
{
	file << "  " << pDBZone->sSmallDescription;
	if ( nID > 0 )
		file << "_" << nID;
	file << "[shape = ";
	if ( bMarked )
		file << "doubleoctagon";
	else
		file << "box";
	//if ( bBlocked )
	//	file << ", color = red";
	if ( bAvailable )
		file << ", fontcolor=darkgreen";
	if ( bInaccessible || nID > 0 )
		file << ", style=dotted";
	else if ( bPassed )
		file << ", style=filled";
	file << ", label = \"";
	if ( !bInaccessible )
		file << nDistance << "\\n";
	file << pDBZone->sSmallDescription << "\\n" << nDifficulty;
	//
	if ( bDrawBlockers )
	{
		if ( !blockers.empty() )
			file << "\\nBlockers:\\n";
		for ( vector< CPtr<CScenarioObjective> >::iterator i = blockers.begin();
			i != blockers.end(); ++i )
				file << (*i)->pParentClue->GetDBClue()->sSmallDescription << "\\n";
	}
	//
	file << "\"];" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioZone::DrawRelationship( fstream &file )
{
	for ( vector< CPtr<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		if ( (*clue)->IsPlaced() )
			file << "  " << pDBZone->sSmallDescription << " -> " << (*clue)->GetDBClue()->sSmallDescription << ";" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioZone::SetCluesInaccessible( bool _bInaccessible )
{
	for ( vector< CPtr<CScenarioClue> >::iterator clue = clues.begin(); clue != clues.end(); ++clue )
		(*clue)->bInaccessible = _bInaccessible;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioClue
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioClue::CScenarioClue( NDb::CDBScenarioClue *_pDBClue ):
	pDBClue( _pDBClue ), bPlaced( false ), bCompound( false ), 
	bInaccessible( true ), bPassed( false ), pPassingParentZone( 0 ), 
	bChildrenUnlocked( false ), bJustFound( false ), nOpenOrder( 0 )
{
	ASSERT( IsValid( pDBClue ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioClue::SetPassingParentZone( CScenarioZone *pZone )
{
	if ( IsValid( pZone ) && !IsValid( pPassingParentZone ) )
		pPassingParentZone = pZone;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioClue::SetPassingParentClue( CScenarioClue *pClue )
{
	if ( !IsValid( pClue ) )
		return;
	if ( find( passingParentClues.begin(), 
		passingParentClues.end(), pClue )	== passingParentClues.end() )
			passingParentClues.push_back( pClue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioObjective *CScenarioClue::GetObjectiveByType( NDb::EScenarioObjectiveType type )
{
	for (	vector< CPtr<CScenarioObjective> >::iterator i = objectives.begin();
		i != objectives.end(); ++i )
			if ( (*i)->GetDBObjective()->type == type )
				return *i;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioClue::CanPlaceObjective( CScenarioObjective *pObjective )
{
	ASSERT( IsValid( pObjective ) );
	//
	for ( vector< CPtr<NDb::CDBScenarioObjective> >::iterator objective = pDBClue->objectives.begin();
		objective != pDBClue->objectives.end(); ++objective )
			if ( *objective == pObjective->GetDBObjective() )
				return TRUE;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioClue::PlaceObjective( CScenarioObjective *pObjective )
{
	ASSERT( IsValid( pObjective ) );
	ASSERT( !pObjective->IsPlaced() );
	ASSERT( CanPlaceObjective( pObjective ) );
	//
	objectives.push_back( pObjective );
	pObjective->SetPlaced();
	pObjective->pParentClue = this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioClue::DrawNode( fstream &file, bool bAvailable )
{
	file << "  " << pDBClue->sSmallDescription << "[shape = ellipse";
	if ( bAvailable )
		file << ", fontcolor=darkgreen";
	if ( bInaccessible )
		file << ", style=dotted";
	else if ( bPassed )
		file << ", style=filled";
	file << ", label = \"" << pDBClue->sSmallDescription;
	//file << "\\n" << nLock;
	file << "\"];" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioClue::IsCorrect()
{
	for (	vector< CPtr<CScenarioObjective> >::iterator objective = objectives.begin();
		objective != objectives.end(); ++objective )
			if ( (*objective)->IsCorrect() )
				return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioClue::DrawRelationship( fstream &file )
{
	for (	vector< CPtr<CScenarioObjective> >::iterator objective = objectives.begin();
		objective != objectives.end(); ++objective )
		if ( (*objective)->IsCorrect() )
		{
			for ( vector< CPtr<CScenarioClue> >::iterator clue = (*objective)->clues.begin(); clue != (*objective)->clues.end(); ++clue )
				if ( (*clue)->IsPlaced() )
				{
					file << "  " << pDBClue->sSmallDescription << " -> " << (*clue)->GetDBClue()->sSmallDescription;
					file << " [ weight = 2 ];" << endl;
				}
			//
			for ( vector< CPtr<CScenarioZone> >::iterator zone = (*objective)->zones.begin(); zone != (*objective)->zones.end(); ++zone )
			{
				file << "  " << pDBClue->sSmallDescription << " -> " << (*zone)->GetDBZone()->sSmallDescription;
				file << " [ weight = 2 ];" << endl;
			}
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioClue::RemoveParentObjective( CScenarioObjective *pObjective )
{
	vector< CPtr<CScenarioObjective> >::iterator parent = 
		find( parentObjectives.begin(), parentObjectives.end(), pObjective );
	if ( parent != parentObjectives.end() )
		parentObjectives.erase( parent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioClue::RemoveChildObjective( CScenarioObjective *pObjective )
{
	pObjective->SetPlaced( false );
	pObjective->zones.clear();
	//
	for ( vector< CPtr<CScenarioClue> >::iterator clue = pObjective->clues.begin();
		clue != pObjective->clues.end(); ++clue )
			(*clue)->RemoveParentObjective( pObjective );
	pObjective->clues.clear();
	//
	vector< CPtr<CScenarioObjective> >::iterator child = 
		find( objectives.begin(), objectives.end(), pObjective );
	if ( child != objectives.end() )
		objectives.erase( child );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioClue::ClearLinks()
{
	// child objectives
	while ( !objectives.empty() )
		RemoveChildObjective( objectives.front() );
	// parent zones
	for ( vector< CPtr<CScenarioZone> >::iterator i = parentZones.begin();
		i != parentZones.end(); ++i )
			(*i)->RemoveClue( this );
	// parent clues
	while ( !parentObjectives.empty() )
	{
		parentObjectives.front()->RemoveClue( this );
		RemoveParentObjective( parentObjectives.front() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioObjective
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioObjective::CScenarioObjective( NDb::CDBScenarioObjective *_pDBObjective ):
	pDBObjective( _pDBObjective ), bPlaced( false ), bProcessedAsFront( false ), pParentZone( 0 )
{
	ASSERT( IsValid( pDBObjective ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioObjective::RemoveZone( CScenarioZone *pZone )
{
	vector< CPtr<CScenarioZone> >::iterator zone = 
		find( zones.begin(), zones.end(), pZone );
	if ( zone != zones.end() )
		zones.erase( zone );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioObjective::RemoveClue( CScenarioClue *pClue )
{
	vector< CPtr<CScenarioClue> >::iterator clue = 
		find( clues.begin(), clues.end(), pClue );
	if ( clue != clues.end() )
		clues.erase( clue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScenarioObjective::BlockZones( bool bBlock )
{
	for ( vector< CPtr<CScenarioZone> >::iterator i = zonesToBlock.begin();
		i != zonesToBlock.end(); ++i )
			(*i)->bBlocked = bBlock;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScenarioObjective::IsZonesToBlockPassed()
{
	for ( vector< CPtr<CScenarioZone> >::iterator i = zonesToBlock.begin(); i != zonesToBlock.end(); ++i )
		if ( !(*i)->bProcessedAsFront )
			return false;
	//
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone *CreateScenarioZone( NDb::CDBScenarioZone *pDBZone, int nInnerID )
{
	return new CScenarioZone( pDBZone, nInnerID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioClue *CreateScenarioClue( NDb::CDBScenarioClue *pDBClue )
{
	return new CScenarioClue( pDBClue );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioObjective *CreateScenarioObjective( NDb::CDBScenarioObjective *pDBObjective )
{
	return new CScenarioObjective( pDBObjective );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NScenario;
//
REGISTER_SAVELOAD_CLASS( 0x50982101, CScenarioZone );
REGISTER_SAVELOAD_CLASS( 0x50982102, CScenarioClue );
REGISTER_SAVELOAD_CLASS( 0x50982103, CScenarioObjective );