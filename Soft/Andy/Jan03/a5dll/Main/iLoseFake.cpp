#include "StdAfx.h"
#include "Gfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "..\MiscDll\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iMission.h"
#include "iLoseFake.h"
#include "iMainMenu.h"
#include "iSaveLoad.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLoseMenuUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLoseMenuUI: public CWindow
{
	OBJECT_NOCOPY_METHODS(CLoseMenuUI);
private:
	ZDATA_(CWindow)
	CObj<CHoverButton> pExit;
	CObj<CHoverButton> pLoad;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pExit); f.Add(3,&pLoad); return 0; }

public:
	CLoseMenuUI() {}
	CLoseMenuUI( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CLoseMenuUI::CLoseMenuUI( const SWindowInfo &sInfo ):
	CWindow( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CLoseMenuUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pExit = new CHoverButton( sEvent.pLoader->GetControl( "exitgame" ) );
			pExit->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11197 ) + GetDBString( 11235 ) );
			pExit->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11198 ) + GetDBString( 11235 ) );

			pLoad = new CHoverButton( sEvent.pLoader->GetControl( "load" ) );
			pLoad->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11197 ) + GetDBString( 11236 ) );
			pLoad->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11198 ) + GetDBString( 11236 ) );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLoseMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLoseMenuInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CLoseMenuInterface);
private:
	NInput::CBind bindLoadGame, bindExitGame;

	ZDATA
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	////
	CObj<NUI::CLoseMenuUI> pUI;
	CObj<NUI::CScreenShot> pScreenShot;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCursor); f.Add(3,&pInterface); f.Add(4,&pUI); f.Add(5,&pScreenShot); return 0; }

public:
	CLoseMenuInterface();

	void Initialize( NGScene::CScreenshotTexture *pScreenShotTexture = 0 );

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &eEvent );
	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CLoseMenuInterface::CLoseMenuInterface():
	bindLoadGame( "save" ), bindExitGame( "exitgame" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLoseMenuInterface::Initialize( NGScene::CScreenshotTexture *pScreenShotTexture )
{
	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pScreenShot = new NUI::CScreenShot( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "clues", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_BOTTOMMOST ) );
	if ( !IsValid( pScreenShotTexture ) )
	{
		pScreenShot->SetMode( NUI::CScreenShot::BLACKANDWHITE, CVec4( 0.5f, 0.5f, 0.5f, 1 ) );
		pScreenShot->Generate();
	}
	else
		pScreenShot->SetTexture( pScreenShotTexture );

	pUI = new NUI::CLoseMenuUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "exitmenu", NUI::STYLE_ENABLED ) );
	NUI::LoadTemplate( pUI, NDb::GetUIContainer( 371 ) );
	pUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLoseMenuInterface::Step()
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
void CLoseMenuInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CLoseMenuInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	NInput::SetSection( "menu" );

	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindLoadGame.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NGame::CICSaveLoadMenu( LOAD, pScreenShot->GetTexture() ) );
		return true;
	}
	else if ( bindExitGame.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICMainMenu() );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLoseMenuInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( GetTime() );
	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICLoseMenu
////////////////////////////////////////////////////////////////////////////////////////////////////
CICLoseMenu::CICLoseMenu( NGScene::CScreenshotTexture *_pScreenShotTexture ):
	pScreenShotTexture( _pScreenShotTexture )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICLoseMenu::Exec()
{
	CLoseMenuInterface *pRes = new CLoseMenuInterface();
	pRes->Initialize( pScreenShotTexture );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB1217151, CLoseMenuUI );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB1217150, CLoseMenuInterface );
