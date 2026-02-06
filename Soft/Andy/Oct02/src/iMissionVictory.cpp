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
#include "iChapterMap.h"
#include "iMissionVictory.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMissionVictoryInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMissionVictoryInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CMissionVictoryInterface);
private:
	NInput::CBind bindExit;
	NInput::CBind bindContinueMission, bindEndMission;

	ZDATA
	CPtr<IMission> pMission;
	////
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	////
	CObj<NUI::CWindow> pVictoryUI;
	CObj<NUI::CScreenShot> pScreenShot;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pMission); f.Add(3,&pCursor); f.Add(4,&pInterface); f.Add(5,&pVictoryUI); f.Add(6,&pScreenShot); return 0; }

public:
	CMissionVictoryInterface();

	void Initialize( IMission *pMission );

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &eEvent );
	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CMissionVictoryInterface::CMissionVictoryInterface():
	bindExit( "cancel" ), bindContinueMission( "continuemission" ), bindEndMission( "endmission" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionVictoryInterface::Initialize( IMission *_pMission )
{
	pMission = _pMission;

	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pScreenShot = new NUI::CScreenShot( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "screenshot", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_BOTTOMMOST ) );
	pScreenShot->Generate();

	pVictoryUI = new NUI::CWindow( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "victoryscreen", NUI::STYLE_ENABLED ) );
	NUI::LoadTemplate( pVictoryUI, NDb::GetUIContainer( 154 ) );
	pVictoryUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionVictoryInterface::Step()
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
void CMissionVictoryInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMissionVictoryInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}
	else if ( bindContinueMission.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}
	else if ( bindEndMission.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		NMainLoop::Command( new NGame::CICContinueChapter( pMission->GetRPGGame() ) );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionVictoryInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( GetTime() );
	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICMissionVictory::CICMissionVictory( IMission *_pMission ):
	pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICMissionVictory::Exec()
{
	CMissionVictoryInterface *pRes = new CMissionVictoryInterface();
	pRes->Initialize( pMission );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB0910080, CMissionVictoryInterface );
