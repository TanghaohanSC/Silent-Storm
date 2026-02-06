#include "StdAfx.h"
//
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataScenario.h"
#include "wDiplomacy.h"
//
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDiplomacy
////////////////////////////////////////////////////////////////////////////////////////////////////
CDiplomacy::CDiplomacy( unsigned int _nDiplomacy ):
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
CGlobalDiplomacy::CGlobalDiplomacy( int nZoneID )
{
	diplomacy.resize( 16 );
	for ( int i = 0; i < 16; ++i )
		diplomacy[i] = new CDiplomacy();
	LoadDiplomacy( nZoneID );
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
unsigned int CGlobalDiplomacy::GetPlayerDiplomacy( int nPlayer ) 
{ 
	return diplomacy[nPlayer]->GetDiplomacy(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalDiplomacy::SetPlayerDiplomacy( int nPlayer, unsigned int nDiplomacy )
{
	return diplomacy[nPlayer]->SetDiplomacy( nDiplomacy );
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
	diplomacy[nPlayer2]->SetDiplomacyState( nPlayer1, state );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NRPG;
//
REGISTER_SAVELOAD_CLASS( 0x50692160, CDiplomacy )
REGISTER_SAVELOAD_CLASS( 0x50692161, CGlobalDiplomacy )