#include "StdAfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "GSceneUtils.h"
#include "Transform.h"
#include "Camera.h"
#include "RPGGlobal.h"
#include "A5Script.h"
#include "..\Input\Bind.h"
#include "..\Misc\Commands.h"
#include "..\Misc\LogStream.h"
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "Interface.h"
#include "iMainMenu.h"
#include "iInterMission.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Mission interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////
class CInterMissionInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CInterMissionInterface);
	//
	NInput::CBind bindExit, bindCancel;
	ZDATA
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	////
	CObj<NUI::CText> pText;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCursor); f.Add(3,&pInterface); f.Add(4,&pText); return 0; }

public:
	CInterMissionInterface();

	void Initialize( const string &szConfig, const wstring &wsMessage );

	void RenderFrame();

	virtual void Step();
	virtual void OnGetFocus();
	virtual bool ProcessEvent( const NInput::SEvent &eEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CInterMissionInterface
//
////////////////////////////////////////////////////////////////////////////////////////////////////
CInterMissionInterface::CInterMissionInterface():
	bindExit( "exit" ), bindCancel( "cancel" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterMissionInterface::Initialize( const string &szConfig, const wstring &wsMessage )
{
	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pText = new NUI::CText( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_TRANSPARENT | NUI::STYLE_TOPMOST ) );
	if ( wsMessage.empty() )
		pText->SetText( L"<font face=Courier size=30pt><color=red><center>INTERMISSION<br><color=white>ESC - exit<br>Use console to start game" );
	else
		pText->SetText( L"<font face=Courier size=30pt><color=red><center>INTERMISSION<br><color=white>ESC - exit<br><color=red>Game stop with message: " + wsMessage );

	if ( !szConfig.empty() )
		NGlobal::LoadConfig( szConfig.c_str() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterMissionInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInterMissionInterface::ProcessEvent( const NInput::SEvent &eEvent )
{
	pCursor->ProcessEvent( eEvent );

	bool bRet = pInterface->ProcessEvent( eEvent );
	if ( bRet )
		return true;

	if ( bindExit.ProcessEvent( eEvent ) || bindCancel.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( 0 ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterMissionInterface::Step()
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
void CInterMissionInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( GetTime() );
	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMD
////////////////////////////////////////////////////////////////////////////////////////////////////
static void StartMainMenu( const vector<wstring> &szParams, void *pContext )
{
	csSystem << CC_BLUE << "Main menu..." << endl;
	NMainLoop::Command( new CICMainMenu() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICInterMission::CICInterMission( const string &_szConfig, const wstring &_wsMessage ): 
	szConfig( _szConfig ), wsMessage( _wsMessage )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICInterMission::Exec()
{
	ResetStack();
	CInterMissionInterface *pRes = new CInterMissionInterface();
	pRes->Initialize( szConfig, wsMessage );
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
REGISTER_SAVELOAD_CLASS( 0x02511018, CInterMissionInterface );
