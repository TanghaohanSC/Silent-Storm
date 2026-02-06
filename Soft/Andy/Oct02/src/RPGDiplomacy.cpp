#include "StdAfx.h"
//
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataScenario.h"
#include "RPGDiplomacy.h"
#include "wMain.h"
#include "wUnitServer.h"
#include "RPGUnitMission.h"
//
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDiplomacy
////////////////////////////////////////////////////////////////////////////////////////////////////
CDiplomacy::CDiplomacy( DWORD _nDiplomacy ):
	nDiplomacy( _nDiplomacy )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::EDiplomacyState CDiplomacy::GetDiplomacyState( int nPlayer )
{
	return NDb::EDiplomacyState( ( nDiplomacy & ( 3 << ( nPlayer << 1 ) ) ) >> ( nPlayer << 1 ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDiplomacy::SetDiplomacyState( int nPlayer, NDb::EDiplomacyState state )
{
	nDiplomacy &= ~( 3 << ( nPlayer << 1 ) );
	nDiplomacy |= int( state ) << ( nPlayer << 1 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalDiplomacy 
////////////////////////////////////////////////////////////////////////////////////////////////////
CGlobalDiplomacy::CGlobalDiplomacy()
{
	diplomacy.resize( 16 );
	for ( int i = 0; i < 16; ++i )
		diplomacy[i] = new CDiplomacy();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalDiplomacy::LoadDiplomacy( int nZoneID )
{
	CDBTable<NDb::CDBPlayer> *pPlayerTable = NDatabase::GetTable<NDb::CDBPlayer>();
	CDBIterator<NDb::CDBPlayer> player( *pPlayerTable );
	while ( pPlayerTable && player.MoveNext() )
	{
		CDBPtr<NDb::CDBPlayer> pDBPlayer = player.Get();
		if ( IsValid( pDBPlayer ) && pDBPlayer->pZone->GetRecordID() == nZoneID )
		{
			ASSERT( pDBPlayer->nPlayer > 0 && pDBPlayer->nPlayer < 16 );
			if ( !( pDBPlayer->nPlayer > 0 && pDBPlayer->nPlayer < 16 ) )
				continue;
			//
			diplomacy[pDBPlayer->nPlayer]->SetDiplomacy( pDBPlayer->nDiplomacy );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CGlobalDiplomacy::GetPlayerDiplomacy( int nPlayer ) 
{ 
	return diplomacy[nPlayer]->GetDiplomacy(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalDiplomacy::SetPlayerDiplomacy( int nPlayer, DWORD nDiplomacy )
{
	diplomacy[nPlayer]->SetDiplomacy( nDiplomacy );
	// устанавливаем DiplomacyState для всех unit-ов этого ScenarioPlayer-а
	if ( !IsValid( pWorld ) )
		return;
	vector< CPtr<NWorld::CUnitServer> > units;
	pWorld->GetScenarioPlayerUnits( nPlayer, &units );
	for ( vector< CPtr<NWorld::CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		CPtr<NRPG::CDiplomacy> pDiplomacy = (*i)->GetUnitRPG()->GetDiplomacy();
		pDiplomacy->SetDiplomacy( nDiplomacy );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::EDiplomacyState CGlobalDiplomacy::GetDiplomacyState( int nPlayer1, int nPlayer2 )
{
	return diplomacy[nPlayer1]->GetDiplomacyState( nPlayer2 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalDiplomacy::SetDiplomacyState( int nPlayer1, int nPlayer2, NDb::EDiplomacyState state )
{
	diplomacy[nPlayer1]->SetDiplomacyState( nPlayer2, state );
	if ( !IsValid( pWorld ) )
		return;
	// устанавливаем DiplomacyState для всех unit-ов этого ScenarioPlayer-а
	vector< CPtr<NWorld::CUnitServer> > units;
	pWorld->GetScenarioPlayerUnits( nPlayer1, &units );
	for ( vector< CPtr<NWorld::CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		CPtr<NRPG::CDiplomacy> pDiplomacy = (*i)->GetUnitRPG()->GetDiplomacy();
		pDiplomacy->SetDiplomacyState( nPlayer2, state );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalDiplomacy::SetWorld( NWorld::IWorld *_pWorld )
{
	if ( CDynamicCast<NWorld::CWorld> pTmpWorld( _pWorld ) )
		pWorld = pTmpWorld;
	else
		ASSERT( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NRPG;
//
REGISTER_SAVELOAD_CLASS( 0x50692160, CDiplomacy )
REGISTER_SAVELOAD_CLASS( 0x50692161, CGlobalDiplomacy )