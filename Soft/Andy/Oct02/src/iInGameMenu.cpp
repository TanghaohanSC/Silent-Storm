#include "StdAfx.h"
#include "Gfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "..\Misc\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iMission.h"
#include "iInGameMenu.h"
#include "iOptionsMenu.h"
#include "iChapterMap.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInGameMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CInGameMenuInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CInGameMenuInterface);
private:
	NInput::CBind bindExit;
	NInput::CBind bindOptions, bindEndMission;

	ZDATA
	EInGameMenuType eType;
	CPtr<NRPG::CGlobalGame> pGlobalGame;
	////
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	////
	CObj<NUI::CWindow> pMenuUI;
	CObj<NUI::CScreenShot> pScreenShot;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&eType); f.Add(3,&pGlobalGame); f.Add(4,&pCursor); f.Add(5,&pInterface); f.Add(6,&pMenuUI); f.Add(7,&pScreenShot); return 0; }

public:
	CInGameMenuInterface();

	void Initialize( NRPG::CGlobalGame *pGlobalGame, EInGameMenuType eType );

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &eEvent );
	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CInGameMenuInterface::CInGameMenuInterface():
	bindExit( "cancel" ), bindOptions( "options" ), bindEndMission( "endmission" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInGameMenuInterface::Initialize( NRPG::CGlobalGame *_pGlobalGame, EInGameMenuType _eType )
{
	eType = _eType;
	pGlobalGame = _pGlobalGame;

	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pScreenShot = new NUI::CScreenShot( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "clues", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_BOTTOMMOST ) );
	pScreenShot->Generate();

	pMenuUI = new NUI::CWindow( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "ingamemenu", NUI::STYLE_ENABLED ) );
	NUI::LoadTemplate( pMenuUI, NDb::GetUIContainer( 158 ) );
	pMenuUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInGameMenuInterface::Step()
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
void CInGameMenuInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInGameMenuInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}
	else if ( bindOptions.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NGame::CICOptions( NGame::OS_EMPTY, pScreenShot->GetTexture() ) );
		return true;
	}
	else if ( bindEndMission.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		NMainLoop::Command( new NGame::CICContinueChapter( pGlobalGame ) );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInGameMenuInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );

	pInterface->Draw( GetTime() );

	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICInGameMenu::CICInGameMenu( NRPG::CGlobalGame *_pGlobalGame, EInGameMenuType _eType ):
	pGlobalGame( _pGlobalGame ), eType( _eType )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICInGameMenu::Exec()
{
	CInGameMenuInterface *pRes = new CInGameMenuInterface();
	pRes->Initialize( pGlobalGame, eType );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB0905130, CInGameMenuInterface );
