#include "StdAfx.h"
#include "RPGGlobal.h"
#include "RPGMerc.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
/////////////////////////////////////////////////////////////////////////////////////
// CGlobalGame
/////////////////////////////////////////////////////////////////////////////////////
void CGlobalGame::Serialize( CStructureSaver *pFile )
{
	pFile->AddContainer( 1, &totalMercs );
	pFile->AddContainer( 2, &mercs );
}
/////////////////////////////////////////////////////////////////////////////////////
} // namespace
/////////////////////////////////////////////////////////////////////////////////////
using namespace NRPG;
void RegisterMissionRPGClasses( int nBase );
void RegisterRPGClasses( int nBase )
{
	REGISTER_SAVELOAD_CLASS( nBase + 0, CMerc );
	REGISTER_SAVELOAD_CLASS( nBase + 1, CGlobalGame );
	RegisterMissionRPGClasses( nBase + 0x1000000 );
}
/////////////////////////////////////////////////////////////////////////////////////
CGlobalGame* CreateNewGame()
{
	CGlobalGame *pGame = new CGlobalGame;
	CMerc *pMerc;
	pMerc = new CMerc;
	pMerc->nMaxHP = 10;
	pGame->mercs.push_back( pMerc );
	pMerc = new CMerc;
	pMerc->nMaxHP = 7;
	pGame->mercs.push_back( pMerc );
	pMerc = new CMerc;
	pMerc->nMaxHP = 3;
	pGame->mercs.push_back( pMerc );
	return pGame;;
}