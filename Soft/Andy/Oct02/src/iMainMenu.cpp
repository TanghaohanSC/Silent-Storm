#include "StdAfx.h"
#include "iMain.h"
#include "iMainMenu.h"
#include "G2DView.h"
#include "GSceneUtils.h"
#include "Transform.h"
#include "Interface.h"
#include "iSaveLoadIC.h"
#include "A5Script.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "iCommonUI.h"
#include "iSideMenu.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMainMenuUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMainMenuUI: public CWindow
{
	OBJECT_BASIC_METHODS(CMainMenuUI);
private:
	ZDATA_(CWindow)
	CObj<CHoverButton> pPlayGame;
	CObj<CHoverButton> pLoadGame;
	CObj<CHoverButton> pQuitGame;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pPlayGame); f.Add(3,&pLoadGame); f.Add(4,&pQuitGame); return 0; }
public:
	CMainMenuUI() {}
	CMainMenuUI( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CMainMenuUI::CMainMenuUI( const SWindowInfo &sInfo ): 
	CWindow( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMainMenuUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOAD:
		{
			pPlayGame = new CHoverButton( sEvent.pLoader->GetControl( "playgame" ) );
			pPlayGame->AddImageState( CHoverButton::STATE_HOVER, NDb::GetUITexture( 257 ) );
			pLoadGame = new CHoverButton( sEvent.pLoader->GetControl( "loadgame" ) );
			pLoadGame->AddImageState( CHoverButton::STATE_HOVER, NDb::GetUITexture( 258 ) );
			pQuitGame = new CHoverButton( sEvent.pLoader->GetControl( "quitgame" ) );
			pQuitGame->AddImageState( CHoverButton::STATE_HOVER, NDb::GetUITexture( 259 ) );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMainMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMainMenuInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CMainMenuInterface);
private:
	NInput::CBind cmdHelp, cmdExit;
	NInput::CBind cmdPlayGame, cmdLoadGame, cmdQuitGame;
	ZDATA
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	CObj<NUI::CMainMenuUI> pMainMenuUICtrl;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCursor); f.Add(3,&pInterface); f.Add(4,&pMainMenuUICtrl); return 0; }

public:
	CMainMenuInterface();

	void Initialize();

	void RenderFrame();

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &eEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMainMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
CMainMenuInterface::CMainMenuInterface():
	cmdHelp( "help" ), cmdExit( "exit" ), cmdPlayGame( "playgame" ), cmdLoadGame( "loadgame" ), cmdQuitGame( "quitgame" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMainMenuInterface::Initialize()
{
	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );
	pMainMenuUICtrl = new NUI::CMainMenuUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "interMissionUI" ) );
	NUI::LoadTemplate( pMainMenuUICtrl, NDb::GetUIContainer( 112 ) );
	pMainMenuUICtrl->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMainMenuInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMainMenuInterface::ProcessEvent( const NInput::SEvent &eEvent )
{
	pCursor->ProcessEvent( eEvent );

	bool bRet = pInterface->ProcessEvent( eEvent );
	if ( bRet )
		return true;

	if ( cmdPlayGame.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( new CICSideMenu() ); 
		return true;
	}

	if ( cmdLoadGame.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( new NGame::CICSaveLoad() ); 
		return true;
	}

	if ( cmdExit.ProcessEvent( eEvent ) || cmdQuitGame.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( 0 ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMainMenuInterface::Step()
{
	MarkNewDGFrame();
	if ( CanRender() )
	{
		pInterface->UpdateCursor();
		pInterface->Step( GetTime() );
		RenderFrame();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMainMenuInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.25f,0.25f,0.25f) );
	pInterface->Draw( GetTime() );
	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMainMenu
////////////////////////////////////////////////////////////////////////////////////////////////////
CICMainMenu::CICMainMenu()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICMainMenu::Exec()
{
	ResetStack();
	CMainMenuInterface *pRes = new CMainMenuInterface;
	pRes->Initialize();
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Start custom game
////////////////////////////////////////////////////////////////////////////////////////////////////
static void StartMainMenu( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	NMainLoop::Command( new CICMainMenu() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB2540910, CMainMenuUI );
REGISTER_SAVELOAD_CLASS( 0xB2540912, CMainMenuInterface );
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(iMainMenu)
	REGISTER_CMD( "mainmenu", StartMainMenu )
FINISH_REGISTER
