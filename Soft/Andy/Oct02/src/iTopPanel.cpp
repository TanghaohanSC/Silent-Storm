#include "StdAfx.h"
#include "GView.h"
#include "G2DView.h"
#include "wInterface.h"
#include "RPGUnitInfo.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "Sound.h"
#include "iMission.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iTopPanel.h"
#include "..\Misc\StrProc.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_FLASH_TIME = 3000;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CEnemyTurnProgressBar
////////////////////////////////////////////////////////////////////////////////////////////////////
class CEnemyTurnProgressBar: public CWindow
{
	OBJECT_BASIC_METHODS(CEnemyTurnProgressBar)
private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	CPtr<CProgressBar> pBar;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pBar); return 0; }

public:
	CEnemyTurnProgressBar() {}
	CEnemyTurnProgressBar( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CEnemyTurnProgressBar::CEnemyTurnProgressBar( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CEnemyTurnProgressBar::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pBar = GetUIWindow<CProgressBar>( this, "bar" );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CEnemyTurnProgressBar::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !GetStyle( STYLE_VISIBLE ) )
		return;

	pBar->SetValue( 1.0f );

	if ( pMission->GetActivePlayer()->GetPlayer() == pMission->GetWorld()->GetCurrentPlayer() )
		return;

	int nValue = 0, nMaxValue = 0;
	CPtr<NWorld::IPlayer> pEnemyPlayer = pMission->GetWorld()->GetCurrentPlayer();
	vector< CPtr<NWorld::CUnit> > playerUnitsSet;
	pEnemyPlayer->GetUnits( &playerUnitsSet );
	for ( int nTemp = 0; nTemp < playerUnitsSet.size(); nTemp++ )
	{
		NRPG::SUnitInfo sInfo;
		playerUnitsSet[nTemp]->GetInfo( &sInfo );

		nValue += sInfo.nAP;
		nMaxValue += sInfo.nMaxAP;
	}

	pBar->SetValue( (float)( nMaxValue - nValue) / nMaxValue );

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTopBar
////////////////////////////////////////////////////////////////////////////////////////////////////
CTopBar::CTopBar( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission ), eMode( NONE ), sModeTime( 0 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTopBar::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_MOUSEMOVE:
		{
			GetInterface()->SetCursorInfo( SCursorInfo() );
			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pEnemyTurn = new CEnemyTurnProgressBar( sEvent.pLoader->GetControl( "enemyturn" ), pMission );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pMiniMap = GetUIWindow<CButton>( this, "minimap" );
			pMiniMap->AddImageState( 0, NDb::GetUITexture( 468 ) );
			pMiniMap->SetStyle( STYLE_ENABLED, false );
//			pMainMenu = GetUIWindow<CButton>( this, "mainmenu" );

			pText = GetUIWindow<CText>( this, "text" );
			pFlash = GetUIWindow<CWindow>( this, "flash" );
			pPlayerTurn = GetUIWindow<CWindow>( this, "playerturn" );
			break;
		}
	}

	if ( CWindow::ProcessMessage( sEvent ) )
		return true;

	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONUP:
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONDBLCLK:
	case EVENT_RBUTTONUP:
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONDBLCLK:
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTopBar::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	EMode ePrevMode = eMode;
	if ( pMission->IsRealTime() || ( pMission->GetActivePlayer()->GetPlayer() == pMission->GetWorld()->GetCurrentPlayer() ) )
		eMode = PLAYER_TURN;
	else
		eMode = ENEMY_TURN;

	if ( eMode != ePrevMode )
	{
		sModeTime = sTime;

		CPtr<NDb::CString> pString;
		if ( pMission->GetWorld()->IsInterrupt() )
			pString = NDb::GetString( 904 );
		else if ( eMode == ENEMY_TURN )
			pString = NDb::GetString( 905 );
		else if ( eMode == PLAYER_TURN )
			pString = NDb::GetString( 903 );

		if ( IsValid( pString ) )
			pText->SetText( pString->szStr );
	}

	if ( pMission->GetWorld()->IsInterrupt() && ( ( sTime - sModeTime ) < N_FLASH_TIME ) )
	{
		pFlash->SetStyle( STYLE_VISIBLE, true );
		pEnemyTurn->SetStyle( STYLE_VISIBLE, false );
		pPlayerTurn->SetStyle( STYLE_VISIBLE, false );
	}
	else if ( eMode == PLAYER_TURN )
	{
		pFlash->SetStyle( STYLE_VISIBLE, false );
		pEnemyTurn->SetStyle( STYLE_VISIBLE, false );
		pPlayerTurn->SetStyle( STYLE_VISIBLE, true );
	}
	else if ( eMode == ENEMY_TURN )
	{
		pFlash->SetStyle( STYLE_VISIBLE, false );
		pEnemyTurn->SetStyle( STYLE_VISIBLE, true );
		pPlayerTurn->SetStyle( STYLE_VISIBLE, false );
	}

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0521163, CEnemyTurnProgressBar );
REGISTER_SAVELOAD_CLASS( 0xB0521164, CTopBar );
