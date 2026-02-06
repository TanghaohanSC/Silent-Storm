#include "StdAfx.h"
#include "iMain.h"
#include "iSideMenu.h"
#include "G2DView.h"
#include "GSceneUtils.h"
#include "RPGGLobal.h"
#include "wInterface.h"
#include "Transform.h"
#include "Interface.h"
#include "iMainMenu.h"
#include "iSaveLoadIC.h"
#include "iGlobalMap.h"
#include "A5Script.h"
#include "..\Input\Bind.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\DBFormat\DataFormat.h"
#include "iMain.h"
#include "iCommonUI.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSideMenuUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSideMenuUI: public CWindow
{
	OBJECT_BASIC_METHODS(CSideMenuUI);
private:
	ZDATA_(CWindow)
	CObj<CHoverButton> pBack;
	CObj<CHoverButton> pAlly;
	CObj<CHoverButton> pAxis;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pBack); f.Add(3,&pAlly); f.Add(4,&pAxis); return 0; }

public:
	CSideMenuUI() {}
	CSideMenuUI( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CSideMenuUI::CSideMenuUI( const SWindowInfo &sInfo ): 
	CWindow( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSideMenuUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOAD:
		{
			pBack = new CHoverButton( sEvent.pLoader->GetControl( "back" ) );
			pBack->AddImageState( CHoverButton::STATE_HOVER, NDb::GetUITexture( 298 ) );
			pAlly = new CHoverButton( sEvent.pLoader->GetControl( "ally" ) );
			pAlly->AddImageState( CHoverButton::STATE_HOVER, NDb::GetUITexture( 299 ) );
			pAxis = new CHoverButton( sEvent.pLoader->GetControl( "axis" ) );
			pAxis->AddImageState( CHoverButton::STATE_HOVER, NDb::GetUITexture( 300 ) );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSideMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSideMenuInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CSideMenuInterface);
private:
	NInput::CBind cmdHelp, cmdExit;
	NInput::CBind cmdBack, cmdAlly, cmdAxis;
	ZDATA
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	CObj<NUI::CSideMenuUI> pSideMenuUICtrl;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCursor); f.Add(3,&pInterface); f.Add(4,&pSideMenuUICtrl); return 0; }

public:
	CSideMenuInterface();

	void Initialize();

	void RenderFrame();

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &eEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMainMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
CSideMenuInterface::CSideMenuInterface():
	cmdHelp( "help" ), cmdExit( "exit" ), cmdBack( "back" ), cmdAlly( "ally" ), cmdAxis( "axis" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSideMenuInterface::Initialize()
{
	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );
	pSideMenuUICtrl = new NUI::CSideMenuUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "sideMenuUI" ) );
	NUI::LoadTemplate( pSideMenuUICtrl, NDb::GetUIContainer( 113 ) );
	pSideMenuUICtrl->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSideMenuInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSideMenuInterface::ProcessEvent( const NInput::SEvent &eEvent )
{
	pCursor->ProcessEvent( eEvent );

	bool bRet = pInterface->ProcessEvent( eEvent );
	if ( bRet )
		return true;

	if ( cmdBack.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() ); 
		return true;
	}

	if ( cmdAlly.ProcessEvent( eEvent ) )
	{
		vector<CObj<NRPG::CGlobalPlayer> > playersSet;
		playersSet.push_back( NRPG::CreateGlobalPlayer() );
		NMainLoop::Command( new NGame::CICBeginGlobal( 4, playersSet ) );
		return true;
	}
	if ( cmdAxis.ProcessEvent( eEvent ) )
	{
		vector<CObj<NRPG::CGlobalPlayer> > playersSet;
		playersSet.push_back( NRPG::CreateGlobalPlayer() );
		NMainLoop::Command( new NGame::CICBeginGlobal( 3, playersSet ) );
		return true;
	}

	if ( cmdExit.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( 0 ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSideMenuInterface::Step()
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
void CSideMenuInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.25f,0.25f,0.25f) );
	pInterface->Draw( GetTime() );
	NGScene::Flip();
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
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB9540910, CSideMenuUI );
REGISTER_SAVELOAD_CLASS( 0xB9540912, CSideMenuInterface );
