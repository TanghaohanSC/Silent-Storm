#include "StdAfx.h"
#include "Transform.h"
#include "GView.h"
#include "G2DView.h"
#include "GSceneUtils.h"
#include "wInterface.h"
#include "Sound.h"
#include "RWGame.h"
#include "RWSound.h"
#include "RPGGame.h"
#include "RPGGlobal.h"
#include "Interface.h"
#include "iMain.h"
#include "iMainMenu.h"
#include "iRenderWorld.h"
#include "iSaveLoad.h"
#include "iCommonUI.h"
#include "iSideMenu.h"
#include "iOptionsMenu.h"
#include "..\Misc\StrProc.h"
#include "..\MiscDll\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataSound.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataCamera.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_MAINMENU_CAMERA = 26,
	N_MAINMENU_TEMPLATE = 2425,
	N_LOGO_FLASHTIME = 2000;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFlashImage
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFlashImage: public CWindow
{
	OBJECT_BASIC_METHODS(CFlashImage);
private:
	ZDATA_(CWindow)
	float fCoeff;
	STime sMorphTime;
	CPtr<CImage> pActive;
	CPtr<CImage> pBackground;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&fCoeff); f.Add(3,&sMorphTime); f.Add(4,&pActive); f.Add(5,&pBackground); return 0; }

public:
	CFlashImage() {}
	CFlashImage( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CFlashImage::CFlashImage( const SWindowInfo &sInfo ):
	CWindow( sInfo ), fCoeff( 0 ), sMorphTime( 0 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFlashImage::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOADCOMPLETE:
		{
			pActive = GetUIWindow<CImage>( this, "active" );
			pBackground = GetUIWindow<CImage>( this, "background" );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFlashImage::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	float fTargetCoeff = float( sTime % ( N_LOGO_FLASHTIME * 2 ) ) / N_LOGO_FLASHTIME;
	if ( fTargetCoeff > 1 )
		fTargetCoeff = 2 - fTargetCoeff;

	fCoeff = CalcFlashCoeff( fCoeff, fTargetCoeff, sTime, sMorphTime );
	sMorphTime = sTime;

	pActive->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fCoeff ) );

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMainMenuUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMainMenuUI: public CWindow
{
	OBJECT_BASIC_METHODS(CMainMenuUI);
private:
	ZDATA_(CWindow)
	CObj<CFlashImage> pLogo;
	CObj<CHoverButton> pCredits;
	CObj<CHoverButton> pOptions;
	CObj<CHoverButton> pCampaign;
	CObj<CHoverButton> pLoadGame;
	CObj<CHoverButton> pQuitGame;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pLogo); f.Add(3,&pCredits); f.Add(4,&pOptions); f.Add(5,&pCampaign); f.Add(6,&pLoadGame); f.Add(7,&pQuitGame); return 0; }
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
			pLogo = new CFlashImage( sEvent.pLoader->GetControl( "logo" ) );

			pCredits = new CHoverButton( sEvent.pLoader->GetControl( "credits" ) );
			pCredits->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10901 ) + GetDBString( 10905 ) );
			pCredits->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10900 ) + GetDBString( 10905 ) );

			pOptions = new CHoverButton( sEvent.pLoader->GetControl( "options" ) );
			pOptions->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10901 ) + GetDBString( 10904 ) );
			pOptions->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10900 ) + GetDBString( 10904 ) );

			pCampaign = new CHoverButton( sEvent.pLoader->GetControl( "campaign" ) );
			pCampaign->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10901 ) + GetDBString( 10902 ) );
			pCampaign->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10900 ) + GetDBString( 10902 ) );

			pLoadGame = new CHoverButton( sEvent.pLoader->GetControl( "load" ) );
			pLoadGame->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10901 ) + GetDBString( 10903 ) );
			pLoadGame->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10900 ) + GetDBString( 10903 ) );

			pQuitGame = new CHoverButton( sEvent.pLoader->GetControl( "quit" ) );
			pQuitGame->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10901 ) + GetDBString( 10906 ) );
			pQuitGame->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10900 ) + GetDBString( 10906 ) );
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
// CMainMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMainMenuInterface: public CRenderBaseInterface
{
	OBJECT_BASIC_METHODS(CMainMenuInterface);
private:
	NInput::CBind bindCampaign, bindLoadGame, bindOptions, bindCredits, bindQuitGame;

	ZDATA_(CRenderBaseInterface)
	CObj<NUI::CMainMenuUI> pMainMenuUI;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CRenderBaseInterface*)this); f.Add(2,&pMainMenuUI); return 0; }

public:
	CMainMenuInterface();

	void Initialize();

	void Step();
	bool ProcessEvent( const NInput::SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMainMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
CMainMenuInterface::CMainMenuInterface():
	bindCampaign( "campaign" ), bindLoadGame( "load" ), bindOptions( "options" ), bindCredits( "credits" ), bindQuitGame( "quit" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMainMenuInterface::Initialize()
{
	CRenderBaseInterface::Initialize( N_MAINMENU_TEMPLATE );

	CPtr<NDb::CDBCamera> pDBCamera = NDb::GetDBCamera( N_MAINMENU_CAMERA );
	ICamera::SCameraPos sCameraPos( pDBCamera->vAnchor, pDBCamera->fDistance, pDBCamera->fPitch, pDBCamera->fYaw, pDBCamera->fRoll, pDBCamera->fFOV );
	GetCamera()->SetPlacement( sCameraPos );

	pMainMenuUI = new NUI::CMainMenuUI( NUI::SWindowInfo( GetInterface(), NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "mainmenuUI" ) );
	NUI::LoadTemplate( pMainMenuUI, NDb::GetUIContainer( 347 ) );
	pMainMenuUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMainMenuInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	NInput::SetSection( "menu" );

	if ( CRenderBaseInterface::ProcessEvent( sEvent ) )
		return true;

	if ( bindCampaign.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICSideMenu() ); 
		return true;
	}
	else if ( bindLoadGame.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NGame::CICSaveLoadMenu( NGame::LOAD ) ); 
		return true;
	}
	else if ( bindOptions.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NGame::CICOptions( NGame::OS_EMPTY ) ); 
		return true;
	}
	else if ( bindCredits.ProcessEvent( sEvent ) )
	{
		return true;
	}
	else if ( bindQuitGame.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( 0 ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMainMenuInterface::Step()
{
	CRenderBaseInterface::Step();

	if ( CanRender() )
	{
		NUI::SRect sScrWindow;
		NUI::SPoint sScrPosition;
		NUI::CWindow *pClientWindow = NUI::GetUIWindow<NUI::CWindow>( pMainMenuUI, "clientview" );
		const NUI::SPoint &sScrSize = pClientWindow->GetSize();
		pClientWindow->ClientToScreen( &sScrPosition, &sScrWindow );

		NUI::SRect sScrClientRect( sScrPosition.x, sScrPosition.y, sScrPosition.x + sScrSize.x, sScrPosition.y + sScrSize.y );
		GetCamera()->SetScreenRect( CTRect<float>( float( sScrClientRect.x1 ) / 1024.0f, float( sScrClientRect.y1 ) / 768.0f, float( sScrClientRect.x2 ) / 1024.0f, float( sScrClientRect.y2 ) / 768.0f ) );

		RenderFrame( GetTime(), GetCamera() );
	}
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
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB2540910, CMainMenuUI );
REGISTER_SAVELOAD_CLASS( 0xB2540911, CFlashImage );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB2540912, CMainMenuInterface );
////////////////////////////////////////////////////////////////////////////////////////////////////
// Start mainmenu
////////////////////////////////////////////////////////////////////////////////////////////////////
static void StartMainMenu( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	NMainLoop::Command( new NGame::CICMainMenu() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(iMainMenu)
	REGISTER_CMD( "mainmenu", StartMainMenu )
FINISH_REGISTER
