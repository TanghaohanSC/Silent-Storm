#include "StdAfx.h"
#include "GSceneUtils.h"
#include "GMemFormat.h"
#include "RPGGlobal.h"
#include "RPGMerc.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataRPG.h"
#include "scScenarioTracker.h"
#include "RPGDiplomacy.h"
//
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
float CGlobalGame::GetGlobalVar( const string &szID, float fDefault ) const
{
	hash_map<string,float>::const_iterator iTemp = globalVars.find( szID );
	if ( iTemp == globalVars.end() )
		return fDefault;
	//
	return iTemp->second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalGame::SetGlobalVar( const string &szID, float fValue )
{
	globalVars[szID] = fValue;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalPlayer::GetAliveUnits( vector< CPtr<NRPG::CUnit> > *pUnits )
{
	ASSERT( pUnits != 0 );
	if ( pUnits == 0 )
		return;
	//
	pUnits->clear();
	for ( vector<CPtr<CMerc> >::iterator i = mercs.begin();	i != mercs.end(); ++i )
		if ( !IsUnitMarkedDead( (*i)->pRPGUnit ) )
			pUnits->push_back( (*i)->pRPGUnit.GetPtr() );
	//
	hash_map< CPtr<NRPG::CUnit>, SUnitDeployData, SPtrHash >::iterator i;
	for ( i = deployData.unitsDeployData.begin(); i != deployData.unitsDeployData.end(); ++i )
		if ( IsValid( i->second.pCorpse ) && 
			!IsUnitMarkedDead( i->second.pCorpse ) && 
			find( pUnits->begin(), pUnits->end(), i->second.pCorpse.GetPtr() ) == pUnits->end() )
				pUnits->push_back( i->second.pCorpse.GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalPlayer::CaptureUnit( NRPG::CUnit *pCarrier, NRPG::CUnit *pUnit )
{
	ASSERT( IsValid( pUnit ) );
	ASSERT( IsValid( pCarrier ) );
	if ( !IsValid( pUnit ) || !IsValid( pCarrier ) )
		return;
	//
	SUnitDeployData &data = deployData.unitsDeployData[ pCarrier ];
	data.pCorpse = pUnit;
	data.bCorpseAlive = true;
	data.bCorpseEnemy = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalPlayer::RescueUnit( NRPG::CUnit *pCarrier, NRPG::CUnit *pUnit )
{
	ASSERT( IsValid( pUnit ) );
	ASSERT( IsValid( pCarrier ) );
	if ( !IsValid( pUnit ) || !IsValid( pCarrier ) )
		return;
	//
	SUnitDeployData &data = deployData.unitsDeployData[ pCarrier ];
	data.pCorpse = pUnit;
	data.bCorpseAlive = true;
	data.bCorpseEnemy = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalPlayer::TakeUnitCorpse( NRPG::CUnit *pCarrier, NRPG::CUnit *pUnit )
{
	ASSERT( IsValid( pUnit ) );
	ASSERT( IsValid( pCarrier ) );
	if ( !IsValid( pUnit ) || !IsValid( pCarrier ) )
		return;
	//
	SUnitDeployData &data = deployData.unitsDeployData[ pCarrier ];
	data.pCorpse = pUnit;
	data.bCorpseAlive = false;
	data.bCorpseEnemy = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalPlayer::FreeUnit( NRPG::CUnit *pCarrier )
{
	ASSERT( IsValid( pCarrier ) );
	if ( !IsValid( pCarrier ) )
		return;
	//
	deployData.unitsDeployData[ pCarrier ].pCorpse = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalPlayer::MarkUnitAsDead( NRPG::CUnit *pUnit )
{
	ASSERT( IsValid( pUnit ) );
	if ( !IsValid( pUnit ) )
		return;
	//
	if ( !IsUnitMarkedDead( pUnit ) )
		deadUnits.push_back( pUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalPlayer::MarkUnitAsAlive( NRPG::CUnit *pUnit )
{
	ASSERT( IsValid( pUnit ) );
	if ( !IsValid( pUnit ) )
		return;
	//
	vector< CPtr<NRPG::CUnit> >::iterator i =
		find( deadUnits.begin(), deadUnits.end(), pUnit );
	if ( i != deadUnits.end() )
		deadUnits.erase( i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGlobalPlayer::IsUnitMarkedDead( NRPG::CUnit *pUnit )
{
	ASSERT( IsValid( pUnit ) );
	if ( !IsValid( pUnit ) )
		return true;
	//
	return find( deadUnits.begin(), deadUnits.end(), pUnit ) != deadUnits.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalPlayer::AddMerc( CMerc *pMerc )
{
	ASSERT( IsValid( pMerc ) );
	if ( !IsValid( pMerc ) )
		return;
	//
	mercs.push_back( pMerc );
	totalMercs.push_back( pMerc );
	deployData.unitsDeployData[ pMerc->pRPGUnit.GetPtr() ] = SUnitDeployData();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CGlobalGame* CreateGlobalGame( int nScenarioID )
{
	CGlobalGame *pGame = new CGlobalGame();
	pGame->pScenarioTracker = NScenario::CreateScenarioTracker( nScenarioID );
	pGame->pGlobalDiplomacy = new NRPG::CGlobalDiplomacy();
	//pGame->pScenarioTracker = NScenario::CreateScenarioTracker( 1 );
	return pGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CGlobalPlayer* CreateGlobalPlayer()
{
	enum
	{
		PC_SOLDIER = 54,
		PC_GRENADER = 53,
		PC_SNIPER = 14,
		PC_MEDIC = 34,
		PC_SCOUT = 2,
		PC_ENGINEER = 3
	};
	//
	CGlobalPlayer *pPlayer = new CGlobalPlayer();
	pPlayer->AddMerc( CreateMerc( NDb::GetPers(PC_SOLDIER) ) );
	pPlayer->AddMerc( CreateMerc( NDb::GetPers(PC_GRENADER) ) );
	pPlayer->AddMerc( CreateMerc( NDb::GetPers(PC_SNIPER) ) );
	return pPlayer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CGlobalPlayer* CreateGlobalPlayer( const vector<int> &personages )
{
	CGlobalPlayer *pPlayer = new CGlobalPlayer();
	//
	for ( int i = 0; i < personages.size(); ++i )
	{
		NDb::CRPGPers *pMerc = NDb::GetPers( personages[i] );
		if ( IsValid( pMerc ) )
			pPlayer->AddMerc( CreateMerc( pMerc ) );
	}
	//
	return pPlayer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NRPG;
//
REGISTER_SAVELOAD_CLASS( 0x02511014, CMerc );
REGISTER_SAVELOAD_CLASS( 0x02511015, CGlobalGame );
REGISTER_SAVELOAD_CLASS( 0x53082180, CGlobalPlayer );