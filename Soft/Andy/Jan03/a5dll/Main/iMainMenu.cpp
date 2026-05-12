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

	// silent-storm-port Phase 1.5 r8: non-serialized flag — set false when
	// the shipping game.db lacks UIContainer(347), so Step() can paint a
	// fallback debug-text main menu instead of a black screen.
	bool m_bHaveDataMenu;

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
	bindCampaign( "campaign" ), bindLoadGame( "load" ), bindOptions( "options" ), bindCredits( "credits" ), bindQuitGame( "quit" ),
	m_bHaveDataMenu( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" void ss_dbg_text_banner(const char* text);
extern "C" void ss_dbg_text_push(int virtX, int virtY, unsigned attr, const char* text);

// silent-storm-port Phase 1.5 r8: fallback main-menu renderer.  The shipping
// Complete/game.db (Hammer&Sickle Russian release) does not contain DBCamera
// id=26 or UIContainer id=347 (CMainMenuUI template) — Nival's data-driven
// path has nothing to LoadTemplate from, so without a hand-built scene the
// screen is blank.  Push a visible "MAIN MENU" with the same five command
// labels CMainMenuInterface::ProcessEvent already wires (campaign/load/
// options/credits/quit) through the bgfx debug-text relay used by the rest
// of the runtime.  Clicking still routes via the bound keys.
static void ss_r8_render_fallback_menu()
{
	static int s_log_once = 0;
	if ( s_log_once == 0 )
	{
		s_log_once = 1;
		FILE *f = NULL;
		fopen_s( &f, "silent_storm_r8_mainmenu.log", "w" );
		if ( f ) { fprintf( f, "ss_r8_render_fallback_menu invoked — pushing banner+5 rows\n" ); fclose( f ); }
	}

	ss_dbg_text_banner( "SILENT STORM  -  MAIN MENU  (fallback render: game.db missing UI 347 / Cam 26)" );

	// Buttons centered around y=300..520 in 1024x768 virtual space.
	struct Row { int y; unsigned attr; const char *text; };
	static const Row rows[] = {
		{ 300, 0x4f, "  [ N ]   New Campaign       (key: bind 'campaign')" },
		{ 340, 0x4f, "  [ L ]   Load Game          (key: bind 'load')" },
		{ 380, 0x4f, "  [ O ]   Options            (key: bind 'options')" },
		{ 420, 0x4f, "  [ C ]   Credits            (key: bind 'credits')" },
		{ 460, 0x4f, "  [ Q ]   Quit Game          (key: bind 'quit')" },
		{ 540, 0x07, "  ~ to open console, 'mainmenu' to re-enter, ESC to cancel." },
	};
	for ( const Row &r : rows )
		ss_dbg_text_push( 200, r.y, r.attr, r.text );
}

static void ss_mm_trace(const char* s) {
	FILE* fp = NULL; fopen_s(&fp, "silent_storm_step_trace.log", "a");
	if (fp) { fprintf(fp, "[MM] %s\n", s); fclose(fp); }
}
void CMainMenuInterface::Initialize()
{
	ss_mm_trace("MMI::Init.0 entry");
	CRenderBaseInterface::Initialize( N_MAINMENU_TEMPLATE );
	ss_mm_trace("MMI::Init.1 CRenderBase::Init ok");

	// silent-storm-port Phase 1.5 r7: the shipped Complete/game.db (Hammer&Sickle
	// release database, used as the closest-shipping data set) does not contain
	// DBCamera ID=26 or UIContainer ID=347 that the Jan03 source drop expects.
	// Guard each lookup so the main-menu state can boot — without these guards
	// the bare pointer deref crashes immediately inside Initialize.
	CPtr<NDb::CDBCamera> pDBCamera = NDb::GetDBCamera( N_MAINMENU_CAMERA );
	if ( pDBCamera )
	{
		ICamera::SCameraPos sCameraPos( pDBCamera->vAnchor, pDBCamera->fDistance, pDBCamera->fPitch, pDBCamera->fYaw, pDBCamera->fRoll, pDBCamera->fFOV );
		GetCamera()->SetPlacement( sCameraPos );
	}

	pMainMenuUI = new NUI::CMainMenuUI( NUI::SWindowInfo( GetInterface(), NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "mainmenuUI" ) );
	NDb::CUIContainer *pContainer = NDb::GetUIContainer( 347 );
	if ( pContainer )
		NUI::LoadTemplate( pMainMenuUI, pContainer );
	pMainMenuUI->ShowWindow( NUI::SWTYPE_SHOW );

	// If the data-driven main menu didn't materialize (no UIContainer record
	// in this build's DB), render a fallback visible menu so the user can
	// see we reached the main-menu state and re-trigger via the keybinds
	// (campaign/load/options/credits/quit).
	m_bHaveDataMenu = ( pContainer != 0 );
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

	// silent-storm-port Phase 1.5 r8: when the data-driven UIContainer is
	// missing, paint a fallback debug-text menu every frame so the user
	// can see we reached the main-menu state.
	if ( !m_bHaveDataMenu )
		ss_r8_render_fallback_menu();

	if ( CanRender() )
	{
		// Without UIContainer 347 there's no "clientview" child window, so
		// the original screen-rect computation derefs a null pointer.
		// Skip the camera-rect setup when running in fallback mode — the
		// debug-text overlay doesn't need camera placement.
		if ( !m_bHaveDataMenu )
			return;

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
