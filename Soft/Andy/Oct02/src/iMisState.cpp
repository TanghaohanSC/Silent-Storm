#include "StdAfx.h"
#include <limits>
#include "wInterface.h"
#include "GView.h"
#include "RWGame.h"
#include "RWSound.h"
#include "GSceneUtils.h"
#include "Transform.h"
#include "RPGGame.h"
#include "RPGGlobal.h"
#include "RPGItemInfo.h"
#include "RPGUnitInfo.h"
#include "..\Input\Bind.h"
#include "iMain.h"
#include "Interface.h"
#include "IDecorators.h"
#include "iMissionGame.h"
#include "iMissionInternal.h"
#include "iMisState.h"
#include "iUnitState.h"
#include "iChapterMap.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTurnSwitch
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateTurnSwitch::CStateTurnSwitch():
	bindOK( "dialog_ok" ), bPauseState( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateTurnSwitch::~CStateTurnSwitch()
{
	if ( IsValid( pInterface ) )
		pInterface->GetGame()->PauseGame( bPauseState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateTurnSwitch::Initialize( IMissionInterface* _pInterface )
{
	pInterface = _pInterface;

	pContainer = NUI::IWindow::Create( pInterface->GetGame()->GetInterface(), NUI::SRect( 0, 0, 1024, 768 ), "", NUI::STYLE_ENABLED | NUI::STYLE_TOPMOST );
	NUI::LoadTemplate( pContainer, NDb::GetUIContainer( 57 ) );
	pContainer->ShowWindow( NUI::SWTYPE_SHOW );
	
	bPauseState = pInterface->GetGame()->IsGamePaused();
	pInterface->GetGame()->PauseGame( true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateTurnSwitch::Step()
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateTurnSwitch::ProcessEvent( const NInput::SEvent &sEvent )
{
	bool bRet = false;

	if ( bindOK.ProcessEvent( sEvent ) )
	{
		CPtr<CStateMove> pState = new CStateMove;
		pState->Initialize( pInterface );
		pInterface->SetState( pState );

		bRet = true;
	}

	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTurnSwitch
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateEndGame::CStateEndGame():
	bindOK( "dialog_ok" ), bindLButtonUp( "leftbutton_up" ), bindRButtonUp( "rightbutton_up" ), bPauseState( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CStateEndGame::~CStateEndGame()
{
	if ( IsValid( pInterface ) )
		pInterface->GetGame()->PauseGame( bPauseState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStateEndGame::Initialize( IMissionInterface* _pInterface, const wstring &wsText )
{
	pInterface = _pInterface;

	pContainer = NUI::IWindow::Create( pInterface->GetGame()->GetInterface(), NUI::SRect( 0, 0, 1024, 768 ), "", NUI::STYLE_ENABLED | NUI::STYLE_TOPMOST );
	NUI::LoadTemplate( pContainer, NDb::GetUIContainer( 86 ) );
	pContainer->ShowWindow( NUI::SWTYPE_SHOW );
	
	bPauseState = pInterface->GetGame()->IsGamePaused();
	pInterface->GetGame()->PauseGame( true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateEndGame::Step()
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStateEndGame::ProcessEvent( const NInput::SEvent &sEvent )
{
	bool bRet = false;

	if ( bindOK.ProcessEvent( sEvent ) || bindLButtonUp.ProcessEvent( sEvent ) || bindRButtonUp.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		NMainLoop::Command( new NGame::CICEndMission( pInterface->GetGame() ) );

		bRet = true;
	}

	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB21c1161, CStateTurnSwitch )
REGISTER_SAVELOAD_CLASS( 0xB21c1162, CStateEndGame )
*/