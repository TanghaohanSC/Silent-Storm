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
extern "C" void ss_dbg_glyph_push(int virtX, int virtY, unsigned abgr, int scale_x, int scale_y, const char* text);
extern "C" void ss_dbg_rect_push(int x1, int y1, int x2, int y2, unsigned abgr);

// silent-storm-port Phase 1.5 r8: fallback main-menu renderer.  The shipping
// Complete/game.db (Hammer&Sickle Russian release) does not contain DBCamera
// id=26 or UIContainer id=347 (CMainMenuUI template) — Nival's data-driven
// path has nothing to LoadTemplate from, so without a hand-built scene the
// screen is blank.  Push a visible "MAIN MENU" with the same five command
// labels CMainMenuInterface::ProcessEvent already wires (campaign/load/
// options/credits/quit) through the bgfx debug-text relay used by the rest
// of the runtime.  Clicking still routes via the bound keys.
//
// r41+: also emit big textured glyph rows + a dark rect background so the
// menu reads as actual UI rather than a console overlay.
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

	ss_dbg_text_banner( "SILENT STORM  -  MAIN MENU  (press N/L/O/C/Q  or  ENTER/ESC)" );

	// dbg-text overlay (still emitted for the small status console)
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

	// r41: large textured glyph header + big buttons. 1024x768 virt space.
	// Dark wash behind the panel so the white text reads on top.
	ss_dbg_rect_push( 192, 220, 832, 580, 0xe0202030u );  // dark slate panel
	ss_dbg_rect_push( 192, 220, 832, 270, 0xff5060c0u );  // header bar

	// Title — scale 4 = 32x64 cells
	ss_dbg_glyph_push( 230, 232, 0xffffffffu, 3, 3, "SILENT STORM" );

	// Buttons -- 2x scale fits ~50 chars / 800px width
	ss_dbg_glyph_push( 220, 290, 0xff20e0e0u, 2, 2, " 1)  New Campaign       [ N / Enter ]" );
	ss_dbg_glyph_push( 220, 340, 0xffe0e020u, 2, 2, " 2)  Load Game          [ L / F8    ]" );
	ss_dbg_glyph_push( 220, 390, 0xff20c0e0u, 2, 2, " 3)  Options            [ O         ]" );
	ss_dbg_glyph_push( 220, 440, 0xff60c0c0u, 2, 2, " 4)  Credits            [ C         ]" );
	ss_dbg_glyph_push( 220, 490, 0xff2020e0u, 2, 2, " 5)  Quit Game          [ Q / Esc   ]" );

	ss_dbg_glyph_push( 220, 540, 0xff808080u, 1, 1, " Silent Storm port - phase 1.5 r41 - bgfx 2D, no 3D scene yet." );
}

static void ss_mm_trace(const char* s) {
	FILE* fp = NULL; fopen_s(&fp, "silent_storm_step_trace.log", "a");
	if (fp) { fprintf(fp, "[MM] %s\n", s); fclose(fp); }
}
void CMainMenuInterface::Initialize()
{
	ss_mm_trace("MMI::Init.0 entry");
	// silent-storm-port r34: with r32 schema fill the DB is now fully
	// materialized — try the data-driven UIContainer path inside a thin
	// init that skips world/scene creation. If GetUIContainer(347)
	// still returns null (Hammer&Sickle DB lacks Jan03 menu records),
	// fall back to the dbg-text overlay from r8.
	InitializeUIOnly();
	ss_mm_trace("MMI::Init.1 InitializeUIOnly ok");

	NDb::CUIContainer *pContainer = NDb::GetUIContainer( 347 );
	ss_mm_trace(pContainer ? "MMI::Init.2 UIContainer 347 FOUND" : "MMI::Init.2 UIContainer 347 MISSING");

	if ( pContainer )
	{
		pMainMenuUI = new NUI::CMainMenuUI( NUI::SWindowInfo( GetInterface(), NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "mainmenuUI" ) );
		ss_mm_trace("MMI::Init.3 CMainMenuUI created");
		NUI::LoadTemplate( pMainMenuUI, pContainer );
		ss_mm_trace("MMI::Init.4 LoadTemplate ok");
		pMainMenuUI->ShowWindow( NUI::SWTYPE_SHOW );
		ss_mm_trace("MMI::Init.5 ShowWindow ok");
	}

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
	static int _sCount = 0;
	++_sCount;
	bool _bTrace = (_sCount < 3);
	if (_bTrace) ss_mm_trace("MMI::Step.0 entry");
	CRenderBaseInterface::Step();
	if (_bTrace) ss_mm_trace("MMI::Step.1 parent Step ok");
	// silent-storm-port r34: even when m_bHaveDataMenu is true, we have no
	// 3D camera/render pipeline (only ran InitializeUIOnly). The data-driven
	// CMainMenuUI exists but never reaches the screen — show fallback so
	// user has visible feedback of the main-menu state until we wire up
	// 2D UI rendering through bgfx.
	if ( !GetCamera() )
		ss_r8_render_fallback_menu();

	// silent-storm-port r35: smoke-test menu transitions without requiring
	// human input — after ~5s in MainMenu, programmatically queue
	// CICSideMenu so we can verify the state-machine transition works.
	// One-shot guarded by a static counter; safe across re-entry because
	// PopInterface returns us here and we don't fire again.
	{
		static int s_autoFireCounter = 0;
		static bool s_autoFired = false;
		if ( !s_autoFired && _sCount > 300 )  // ~5s @ 60fps
		{
			s_autoFired = true;
			ss_mm_trace("MMI::Step.AUTO firing CICSideMenu");
			NMainLoop::Command( new CICSideMenu() );
		}
		(void)s_autoFireCounter;  // unused, kept for symmetry
	}

	// silent-storm-port Phase 1.5 r8: when the data-driven UIContainer is
	// missing, paint a fallback debug-text menu every frame so the user
	// can see we reached the main-menu state.
	if ( !m_bHaveDataMenu )
		ss_r8_render_fallback_menu();

	if (_bTrace) ss_mm_trace("MMI::Step.2 CanRender check");
	if ( CanRender() )
	{
		if (_bTrace) ss_mm_trace("MMI::Step.3 CanRender ok");
		// Without UIContainer 347 there's no "clientview" child window, so
		// the original screen-rect computation derefs a null pointer.
		// Skip the camera-rect setup when running in fallback mode — the
		// debug-text overlay doesn't need camera placement.
		if ( !m_bHaveDataMenu )
			return;

		// silent-storm-port r34: pCamera and pRender are null when we
		// only ran InitializeUIOnly() — skip the 3D scene wiring. The
		// UIContainer-driven main menu UI is drawn by pInterface->Draw()
		// during CRenderBaseInterface::Step().
		if ( !GetCamera() )
		{
			if (_bTrace) ss_mm_trace("MMI::Step.4 no camera, returning");
			return;
		}

		NUI::SRect sScrWindow;
		NUI::SPoint sScrPosition;
		NUI::CWindow *pClientWindow = NUI::GetUIWindow<NUI::CWindow>( pMainMenuUI, "clientview" );
		if ( !pClientWindow )
			return;
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
	ss_mm_trace("CICMainMenu::Exec.0 entry");
	ResetStack();
	ss_mm_trace("CICMainMenu::Exec.1 ResetStack ok");
	CMainMenuInterface *pRes = new CMainMenuInterface;
	ss_mm_trace("CICMainMenu::Exec.2 new CMainMenuInterface ok");
	pRes->Initialize();
	ss_mm_trace("CICMainMenu::Exec.3 Initialize ok");
	SetInterface( pRes );
	ss_mm_trace("CICMainMenu::Exec.4 SetInterface ok");
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
