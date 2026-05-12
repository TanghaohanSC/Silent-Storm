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
#include "iRenderWorld.h"
#include "iCommonUI.h"
#include "iSideMenu.h"
#include "iHeroMenu.h"
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
	N_SIDE_AXIS = 1,
	N_SIDE_ALLIES = 2,
	N_SIDEMENU_CAMERA = 47,
	N_SIDEMENU_TEMPLATE = 2387;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScriptHoverButton
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScriptHoverButton: public CHoverButton
{
	OBJECT_BASIC_METHODS(CScriptHoverButton);
private:
	ZDATA_(CHoverButton)
	CPtr<NGame::CRenderBaseInterface> pInterface;
	////
	bool bHoverNotify;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CHoverButton*)this); f.Add(2,&pInterface); f.Add(3,&bHoverNotify); return 0; }

protected:
	void OnAction();

public:
	CScriptHoverButton() {}
	CScriptHoverButton( const SWindowInfo &sInfo, NGame::CRenderBaseInterface *pInterface );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CScriptHoverButton::CScriptHoverButton( const SWindowInfo &sInfo, NGame::CRenderBaseInterface *_pInterface ):
	CHoverButton( sInfo ), pInterface( _pInterface ), bHoverNotify( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScriptHoverButton::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( IsMouseCover() != bHoverNotify )
	{
		NWorld::CCommand *pCmd = 
			new NWorld::CCmdCallScriptFunction( "OnScriptNotify", "si", GetWindowID().c_str(), IsMouseCover() ? 1 : 0 );
		pInterface->Command( pCmd );
	}

	bHoverNotify = IsMouseCover();

	CHoverButton::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScriptHoverButton::OnAction()
{
	NWorld::CCommand *pCmd = new NWorld::CCmdCallScriptFunction( "OnScriptNotify", "si", GetWindowID().c_str(), 2 );
	pInterface->Command( pCmd );

	CHoverButton::OnAction();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSideMenuUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSideMenuUI: public CWindow
{
	OBJECT_BASIC_METHODS(CSideMenuUI);
public:
	enum ESide
	{
		SIDE_NONE,
		SIDE_AXIS,
		SIDE_ALLIES
	};

private:
	ZDATA_(CWindow)
	CPtr<NGame::CRenderBaseInterface> pInterface;
	////
	ESide eSide;
	CObj<CHoverButton> pBack;
	CObj<CHoverButton> pNext;
	CObj<CHoverButton> pAxis;
	CObj<CHoverButton> pAllies;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pInterface); f.Add(3,&eSide); f.Add(4,&pBack); f.Add(5,&pNext); f.Add(6,&pAxis); f.Add(7,&pAllies); return 0; }
public:
	CSideMenuUI() {}
	CSideMenuUI( const SWindowInfo &sInfo, NGame::CRenderBaseInterface *pInterface );

	ESide GetSide() const { return eSide; }

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CSideMenuUI::CSideMenuUI( const SWindowInfo &sInfo, NGame::CRenderBaseInterface *_pInterface ): 
	CWindow( sInfo ), pInterface( _pInterface ), eSide( SIDE_NONE )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSideMenuUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_NOTIFY:
			{
				if ( sEvent.szID == "axis" )
				{
					eSide = SIDE_AXIS;
					return true;
				}
				else if ( sEvent.szID == "allies" )
				{
					eSide = SIDE_ALLIES;
					return true;
				}

				break;
			}
		case EVENT_TEMPLATELOAD:
			{
				pBack = new CHoverButton( sEvent.pLoader->GetControl( "cancel" ) );
				pBack->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11130 ) + GetDBString( 11174 ) );
				pBack->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11129 ) + GetDBString( 11174 ) );

				pAxis = new CScriptHoverButton( sEvent.pLoader->GetControl( "axis" ), pInterface );
				pAxis->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11130 ) + GetDBString( 11131 ) );
				pAxis->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11129 ) + GetDBString( 11131 ) );

				pNext = new CScriptHoverButton( sEvent.pLoader->GetControl( "next" ), pInterface );
				pNext->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11130 ) + GetDBString( 11132 ) );
				pNext->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11129 ) + GetDBString( 11132 ) );

				pAllies = new CScriptHoverButton( sEvent.pLoader->GetControl( "allies" ), pInterface );
				pAllies->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11130 ) + GetDBString( 11132 ) );
				pAllies->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11129 ) + GetDBString( 11132 ) );
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
// CSideMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSideMenuInterface: public CRenderBaseInterface
{
	OBJECT_BASIC_METHODS(CSideMenuInterface);
private:
	NInput::CBind bindClose, bindNext;

	ZDATA_(CRenderBaseInterface)
	CObj<NUI::CSideMenuUI> pSideMenuUI;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CRenderBaseInterface*)this); f.Add(2,&pSideMenuUI); return 0; }

public:
	CSideMenuInterface();

	void Initialize();

	void Step();
	bool ProcessEvent( const NInput::SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSideMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
CSideMenuInterface::CSideMenuInterface():
	bindClose( "cancel" ), bindNext( "next" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void ss_sm_trace(const char* s) {
	FILE* fp = NULL; fopen_s(&fp, "silent_storm_step_trace.log", "a");
	if (fp) { fprintf(fp, "[SM] %s\n", s); fclose(fp); }
}
void CSideMenuInterface::Initialize()
{
	ss_sm_trace("SMI::Init.0 entry");
	// silent-storm-port r35: match MainMenu's lightweight init — no 3D world
	// (the SideMenu just stacks on top of MainMenu). Camera/scene setup
	// would crash because parent::Initialize tries to CreateWorld.
	InitializeUIOnly();
	ss_sm_trace("SMI::Init.1 InitializeUIOnly ok");

	NDb::CUIContainer *pContainer = NDb::GetUIContainer( 353 );
	ss_sm_trace(pContainer ? "SMI::Init.2 UIContainer 353 FOUND" : "SMI::Init.2 UIContainer 353 MISSING");
	if ( pContainer )
	{
		pSideMenuUI = new NUI::CSideMenuUI( NUI::SWindowInfo( GetInterface(), NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "mainmenuUI" ), this );
		ss_sm_trace("SMI::Init.3 CSideMenuUI created");
		NUI::LoadTemplate( pSideMenuUI, pContainer );
		ss_sm_trace("SMI::Init.4 LoadTemplate ok");
		pSideMenuUI->ShowWindow( NUI::SWTYPE_SHOW );
		ss_sm_trace("SMI::Init.5 ShowWindow ok");
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSideMenuInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	NInput::SetSection( "menu" );

	if ( CRenderBaseInterface::ProcessEvent( sEvent ) )
		return true;

	if ( bindClose.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() ); 
		return true;
	}
	else if ( bindNext.ProcessEvent( sEvent ) )
	{
		switch( pSideMenuUI->GetSide() )
		{
		case NUI::CSideMenuUI::SIDE_AXIS:
			{
				NMainLoop::Command( new NGame::CICHeroMenu( NDb::GetDBSide( N_SIDE_AXIS ) ) ); 
				break;
			}
		case NUI::CSideMenuUI::SIDE_ALLIES:
			{
				NMainLoop::Command( new NGame::CICHeroMenu( NDb::GetDBSide( N_SIDE_ALLIES ) ) ); 
				break;
			}
		}
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" void ss_dbg_text_banner(const char* text);
extern "C" void ss_dbg_text_push(int virtX, int virtY, unsigned attr, const char* text);

static void ss_r38_sidemenu_overlay()
{
	ss_dbg_text_banner( "SILENT STORM  -  CHOOSE SIDE  (state: CSideMenuInterface, UIContainer 353)" );
	ss_dbg_text_push( 200, 300, 0x2f, "  [ A ]   AXIS               (Germany / German Army)" );
	ss_dbg_text_push( 200, 340, 0x1f, "  [ L ]   ALLIES             (USA / United Kingdom / USSR)" );
	ss_dbg_text_push( 200, 420, 0x07, "  ENTER selects, ESC returns to Main Menu." );
	ss_dbg_text_push( 200, 460, 0x06, "  (auto-firing CICHeroMenu(AXIS) in 3s for chain smoke-test)" );
}

void CSideMenuInterface::Step()
{
	static int _sCount = 0;
	++_sCount;
	CRenderBaseInterface::Step();

	// silent-storm-port r38: keep a fallback overlay alive so the screen
	// reflects the state-machine advance even when the data-driven UI
	// hasn't been rendered to pixels yet.
	ss_r38_sidemenu_overlay();

	// silent-storm-port r37: smoke-test SideMenu -> HeroMenu (AXIS). The
	// real UI control flow is bindNext.ProcessEvent on ENTER, but for
	// headless runs we time it out so the trace exercises the entire
	// chain to mission-launch without keyboard input. Single-shot.
	{
		static bool s_autoFired = false;
		if ( !s_autoFired && _sCount > 180 )  // ~3s after SideMenu init
		{
			s_autoFired = true;
			ss_sm_trace("SMI::Step.AUTO firing CICHeroMenu(AXIS)");
			NDb::CSide *pSide = NDb::GetDBSide( N_SIDE_AXIS );
			if ( pSide )
			{
				NMainLoop::Command( new NGame::CICHeroMenu( pSide ) );
			}
			else
			{
				ss_sm_trace("SMI::Step.AUTO pSide NULL -- skipping");
			}
		}
	}

	// silent-storm-port r35: GetCamera() and pSideMenuUI are null/empty when
	// we used InitializeUIOnly — skip 3D scene + screen-rect wiring.
	if ( !GetCamera() )
		return;

	if ( CanRender() )
	{
		NUI::SRect sScrWindow;
		NUI::SPoint sScrPosition;
		NUI::CWindow *pClientWindow = NUI::GetUIWindow<NUI::CWindow>( pSideMenuUI, "clientview" );
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
CICSideMenu::CICSideMenu()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICSideMenu::Exec()
{
	CSideMenuInterface *pRes = new CSideMenuInterface;
	pRes->Initialize();
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB1112180, CSideMenuUI );
REGISTER_SAVELOAD_CLASS( 0xB1112181, CScriptHoverButton );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB111218A, CSideMenuInterface );
