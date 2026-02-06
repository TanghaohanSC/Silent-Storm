#include "StdAfx.h"
#include "Gfx.h"
#include "WinFrame.h"
#include "iMain.h"
#include "iInput.h"
#include "BasicDB.h"
#include "iMission.h" // CRAP, to start from mission
#include "RPGGlobal.h" // CRAP, to start from mission
/////////////////////////////////////////////////////////////////////////////////////
extern void RegisterDataClasses( int nBase );
extern void RegisterInterfaceClasses( int nBase );
extern void RegisterGSceneClasses( int nBase );
extern void RegisterWorldClasses( int nBase );
extern void RegisterRPGClasses( int nBase );
/////////////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF );
#endif // _DEBUG
	//
	RegisterDataClasses( 0 );
	RegisterInterfaceClasses( 0x1000000 );
	RegisterGSceneClasses( 0x2000000 );
	RegisterWorldClasses( 0x3000000 );
	RegisterRPGClasses( 0x4000000 );
	{
		CFileStream f;
		if ( f.OpenRead( "c:\\A5\\game.db" ) ) // CRAP path
		{
			CStructureSaver file( f, CStructureSaver::READ );
			NDatabase::Serialize( file );
		}
		// CRAP - in case of game.db absense no need to run
	}
	NMainLoop::BindStandardMsgs(); // CRAP
	//
	//vector<string> szParams;
	//NStrProc::SplitStringWithMultipleBrackets( lpCmdLine, szParams, ' ' );
	if ( !NWinFrame::InitApplication( hInstance, "A5", "A5" ) )
		return 0;
	if ( !NGfx::Init3D( NWinFrame::GetWnd() ) )
	{
		ASSERT(0); // DX8 not found
		return 0;
	}
	//
	//Main3DLoop();
	NGfx::SetMode( 800, 600, 16, NGfx::WINDOWED );//NGfx::FULL_SCREEN );///WINDOWED );
	NMainLoop::Command( new CICMission( CreateNewGame() ) );
	for (;;)
	{
		NInput::PumpMessages();
		if ( NInput::IsExit() )
			break;
		if ( !NMainLoop::StepApp() )
			break;
	}
	//
	NGfx::Done3D();
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
