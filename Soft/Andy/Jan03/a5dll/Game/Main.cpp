#include "StdAfx.h"
#include "..\Main\GInit.h"
#include "WinFrame.h"
#include "..\Main\iMain.h"
#include "..\Input\Bind.h"
#include "..\ADOImport\BasicDB.h"
#include "..\Misc\StrProc.h"
#include "..\MiscDll\Commands.h"
#include "..\Main\GResource.h" // CRAP �� ��������� �������, ������-�� ������ ���� ��������� ������
#include "..\Main\iInterMission.h" // CRAP, to start from mission
#include "..\Main\iSaveManager.h" // CRAP, to start from mission
#include "..\Main\Sound.h"

#ifdef SS_USE_BGFX_FACADE
// Phase 1 Task 9: C-style bootstrap so we avoid the STLport↔MSVC STL ABI
// mismatch on std::string between Game/Main.cpp and the renderer lib.
extern "C" bool ss_renderer_bootstrap(void* hwnd, int width, int height,
                                       const char* cfg_path);
extern "C" bool ss_renderer_load_shaders(const char* shader_dir);
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////
//void DumpMemoryStats() {}

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
#ifdef _DEBUG
  int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	//tmpFlag |= _CRTDBG_LEAK_CHECK_DF;// | _CRTDBG_CHECK_ALWAYS_DF;
  tmpFlag = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;// | _CRTDBG_CHECK_ALWAYS_DF;
  _CrtSetDbgFlag( tmpFlag );
	//_CrtSetBreakAlloc( 114 );
#else
	srand( GetTickCount() );
#endif // _DEBUG
	NGScene::AddResourceDir( ".\\res" );
	NGScene::RunResourceLoadingThread();
  // load game database
	try
	{
		CFileStream f;
		f.OpenRead( "game.db" );
		NDatabase::Serialize( f, CStructureSaver::READ );
	}
	catch (...)
	{
		ASSERT( 0 ); // game.db not found
		MessageBox( 0, "File game.db not found", "Error", MB_OK );
		return 0;
	}

	// init subsystems
	if ( !NWinFrame::InitApplication( hInstance, "A5", "A5" ) )
		return 0;

#ifdef SS_USE_BGFX_FACADE
	// Phase 1 Task 9: initialize bgfx on Nival's HWND, then load shaders.
	// This must happen before NGfx::Init3D — NGfx::Init3D ultimately creates
	// the IDirect3DDevice9 facade, which assumes bgfx is already alive.
	{
		RECT rc;
		GetClientRect(NWinFrame::GetWnd(), &rc);
		int w = rc.right  - rc.left;
		int h = rc.bottom - rc.top;
		if (w <= 0 || h <= 0) { w = 1280; h = 720; }
		if (!ss_renderer_bootstrap(NWinFrame::GetWnd(), w, h, "silent_storm.cfg"))
		{
			MessageBox(0, "bgfx init failed", "Error", MB_OK);
			return 0;
		}
		ss_renderer_load_shaders("shaders");
	}
#endif

	if ( !NGfx::Init3D( NWinFrame::GetWnd() ) )
	{
		ASSERT(0); // DX8 not found
		MessageBox( 0, "Failed to initialize Direct3D8", "Error", MB_OK );
		return 0;
	}
	if ( !NSound::InitSound( NWinFrame::GetWnd() ) )
	{
		ASSERT(0); // FMod not found
		MessageBox( 0, "Failed to initialize FMod", "Error", MB_OK );
		return 0;
	}
	if ( !NInput::InitInput( NWinFrame::GetWnd() ) )
	{
		ASSERT(0); // DX8input not found
		MessageBox( 0, "Failed to initialize DirectInput8", "Error", MB_OK );
		return 0;
	}

	// Load config & process params
	NGlobal::LoadConfig( ".\\cfg\\autoexec.cfg" );

	vector<string> szParams;
	bool bDoLoad = false;
	NStr::SplitStringWithMultipleBrackets( lpCmdLine, szParams, ' ' );
	string szCfg( "start.cfg" );
	for ( int i = 0; i < szParams.size(); ++i )
	{
		if ( szParams[i] == "-fullscreen" )
			NGlobal::SetVar( "gfx_fullscreen", 1 );
		else if ( szParams[i] == "-320" )
			NGlobal::SetVar( "gfx_resolution", 320 );
		else if ( szParams[i] == "-400" )
			NGlobal::SetVar( "gfx_resolution", 400 );
		else if ( szParams[i] == "-640" )
			NGlobal::SetVar( "gfx_resolution", 640 );
		else if ( szParams[i] == "-800" )
			NGlobal::SetVar( "gfx_resolution", 800 );
		else if ( szParams[i] == "-1024" )
			NGlobal::SetVar( "gfx_resolution", 1024 );
		else if ( szParams[i] == "-1280" )
			NGlobal::SetVar( "gfx_resolution", 1280 );
		else if ( szParams[i] == "-1600" )
			NGlobal::SetVar( "gfx_resolution", 1600 );
		else if ( szParams[i] == "-nops" )
			NGlobal::SetVar( "gfx_nopixelshaders", 1 );
		else if ( szParams[i] == "-novs" )
			NGlobal::SetVar( "gfx_novertexshaders", 1 );
		else if ( szParams[i] == "-gfxvalidate" )
			NGlobal::SetVar( "gfx_validate", 1 );
		else if ( szParams[i] == "-aniso" )
			NGlobal::SetVar( "gfx_anisotropic_filter", 1 );
		else if ( szParams[i] == "-bannp2" )
			NGlobal::SetVar( "gfx_fix_ban_np2", 1 );
		else if ( szParams[i] == "-nvrulez" )
			NGlobal::SetVar( "gfx_fix_nv_np2_hack", 1 );
		else if ( szParams[i] == "-dxtoff" )
			NGlobal::SetVar( "gfx_texture_usedxt", 0 );

		if ( szParams[i] == "-nosound" )
			NGlobal::SetVar( "sound_init", 0 );

		if ( szParams[i] == "-noai" )
			NGlobal::SetVar( "game_noai", 1 );

		if ( szParams[i] == "-load" )
			bDoLoad = true;
		if ( szParams[i] == "-cfg" )
		{
			if ( i + 1 < szParams.size() )
				szCfg = szParams[++i];
		}
	}
	//
	if ( !NGScene::SetModeFromConfig() )
	{
		ASSERT(0); // no mode found
		MessageBox( 0, "Failed to set display mode", "Error", MB_OK );
		return 0;
	}
	//
	if ( !NSound::SetModeFromConfig() )
	{
		ASSERT(0);
		MessageBox( 0, "Failed to set sound mode", "Error", MB_OK );
		return 0;
	}
	//
	if ( bDoLoad )
		NMainLoop::Command( new NMainLoop::CICLoad( string( NMainLoop::S_SLOT_QUICKSAVE ) ) );
	else
		NMainLoop::Command( new CICInterMission( szCfg ) );
	for (;;)
	{
		NWinFrame::PumpMessages();
		bool bActive = NWinFrame::IsAppActive();
		NInput::PumpMessages( bActive );
		if ( NWinFrame::IsExit() )
			break;
		if ( !NMainLoop::StepApp( bActive, bActive ) )
			break;
		if ( !bActive )
			Sleep( 40 );
	}
	//
	NGlobal::SaveConfig( ".\\cfg\\config.cfg" );
	NMainLoop::DoneInterface();
	NGfx::Done3D();
	NInput::DoneInput();
	NSound::DoneSound();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
