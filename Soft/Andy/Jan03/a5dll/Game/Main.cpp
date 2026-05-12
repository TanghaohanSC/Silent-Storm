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

// silent-storm-port Phase 1.5: unhandled exception filter — logs crash addr
// so we can map it to a function in silent_storm.map without a debugger.
static LONG WINAPI ss_crash_handler(EXCEPTION_POINTERS* p)
{
	FILE* f = NULL;
	fopen_s(&f, "silent_storm_crash.log", "w");
	if (f)
	{
		fprintf(f, "EXCEPTION 0x%08X at addr 0x%p\n",
			(unsigned)p->ExceptionRecord->ExceptionCode,
			p->ExceptionRecord->ExceptionAddress);
		CONTEXT* ctx = p->ContextRecord;
		fprintf(f, "EIP=0x%08X ESP=0x%08X EBP=0x%08X\n",
			(unsigned)ctx->Eip, (unsigned)ctx->Esp, (unsigned)ctx->Ebp);
		fprintf(f, "EAX=0x%08X EBX=0x%08X ECX=0x%08X EDX=0x%08X\n",
			(unsigned)ctx->Eax, (unsigned)ctx->Ebx,
			(unsigned)ctx->Ecx, (unsigned)ctx->Edx);
		fprintf(f, "ESI=0x%08X EDI=0x%08X\n",
			(unsigned)ctx->Esi, (unsigned)ctx->Edi);
		// Walk a few stack frames manually (EBP-chain) — x86 only
		unsigned* bp = (unsigned*)ctx->Ebp;
		for (int i = 0; i < 16 && bp; ++i)
		{
			__try {
				fprintf(f, "  frame[%2d] ebp=0x%08X ret=0x%08X\n",
					i, (unsigned)bp, (unsigned)bp[1]);
				bp = (unsigned*)bp[0];
			} __except(EXCEPTION_EXECUTE_HANDLER) { break; }
		}
		fclose(f);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

// silent-storm-port: trace progress through WinMain to a log file
static void ss_winmain_trace(const char* msg)
{
	FILE* f = NULL;
	fopen_s(&f, "silent_storm_winmain.log", "a");
	if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	SetUnhandledExceptionFilter(ss_crash_handler);
	// Truncate trace log at start of run
	{ FILE* f = NULL; fopen_s(&f, "silent_storm_winmain.log", "w"); if (f) fclose(f); }
	ss_winmain_trace("01 WinMain entered");

#ifdef _DEBUG
  int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	//tmpFlag |= _CRTDBG_LEAK_CHECK_DF;// | _CRTDBG_CHECK_ALWAYS_DF;
  tmpFlag = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;// | _CRTDBG_CHECK_ALWAYS_DF;
  _CrtSetDbgFlag( tmpFlag );
	//_CrtSetBreakAlloc( 114 );
#else
	srand( GetTickCount() );
#endif // _DEBUG
	ss_winmain_trace("02 about to AddResourceDir");
	NGScene::AddResourceDir( ".\\res" );
	ss_winmain_trace("03 about to RunResourceLoadingThread");
	NGScene::RunResourceLoadingThread();
	ss_winmain_trace("04 resource thread launched");
  // load game database
	ss_winmain_trace("05 about to open game.db");
	try
	{
		CFileStream f;
		f.OpenRead( "game.db" );
		ss_winmain_trace("06 game.db opened, about to Serialize");
		NDatabase::Serialize( f, CStructureSaver::READ );
		ss_winmain_trace("07 NDatabase::Serialize returned");
	}
	catch (...)
	{
		ss_winmain_trace("XX exception during game.db load");
		ASSERT( 0 ); // game.db not found
		MessageBox( 0, "File game.db not found", "Error", MB_OK );
		return 0;
	}

	// init subsystems
	ss_winmain_trace("08 about to InitApplication");
	if ( !NWinFrame::InitApplication( hInstance, "A5", "A5" ) )
		return 0;
	ss_winmain_trace("09 InitApplication ok");

#ifdef SS_USE_BGFX_FACADE
	// Phase 1 Task 9: initialize bgfx on Nival's HWND, then load shaders.
	// This must happen before NGfx::Init3D — NGfx::Init3D ultimately creates
	// the IDirect3DDevice9 facade, which assumes bgfx is already alive.
	ss_winmain_trace("10 about to bgfx bootstrap");
	{
		RECT rc;
		GetClientRect(NWinFrame::GetWnd(), &rc);
		int w = rc.right  - rc.left;
		int h = rc.bottom - rc.top;
		if (w <= 0 || h <= 0) { w = 1280; h = 720; }
		if (!ss_renderer_bootstrap(NWinFrame::GetWnd(), w, h, "silent_storm.cfg"))
		{
			ss_winmain_trace("XX bgfx bootstrap returned false");
			MessageBox(0, "bgfx init failed", "Error", MB_OK);
			return 0;
		}
		ss_winmain_trace("11 bgfx bootstrap ok, loading shaders");
		ss_renderer_load_shaders("shaders");
		ss_winmain_trace("12 shaders loaded");
	}
#endif

	ss_winmain_trace("13 about to NGfx::Init3D");
	if ( !NGfx::Init3D( NWinFrame::GetWnd() ) )
	{
		ss_winmain_trace("XX Init3D returned false");
		ASSERT(0); // DX8 not found
		MessageBox( 0, "Failed to initialize Direct3D8", "Error", MB_OK );
		return 0;
	}
	ss_winmain_trace("14 Init3D ok, about to InitSound");
	if ( !NSound::InitSound( NWinFrame::GetWnd() ) )
	{
		ss_winmain_trace("XX InitSound returned false");
		ASSERT(0); // FMod not found
		MessageBox( 0, "Failed to initialize FMod", "Error", MB_OK );
		return 0;
	}
	ss_winmain_trace("15 InitSound ok, about to InitInput");
	if ( !NInput::InitInput( NWinFrame::GetWnd() ) )
	{
		ss_winmain_trace("XX InitInput returned false");
		ASSERT(0); // DX8input not found
		MessageBox( 0, "Failed to initialize DirectInput8", "Error", MB_OK );
		return 0;
	}
	ss_winmain_trace("16 InitInput ok, about to LoadConfig");

	// Load config & process params
	NGlobal::LoadConfig( ".\\cfg\\autoexec.cfg" );
	ss_winmain_trace("17 LoadConfig done");

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
	ss_winmain_trace("18 about to SetModeFromConfig (Gfx)");
	if ( !NGScene::SetModeFromConfig() )
	{
		ss_winmain_trace("XX SetModeFromConfig returned false");
		ASSERT(0); // no mode found
		MessageBox( 0, "Failed to set display mode", "Error", MB_OK );
		return 0;
	}
	ss_winmain_trace("19 SetModeFromConfig ok, sound");
	if ( !NSound::SetModeFromConfig() )
	{
		ss_winmain_trace("XX sound SetModeFromConfig returned false");
		ASSERT(0);
		MessageBox( 0, "Failed to set sound mode", "Error", MB_OK );
		return 0;
	}
	ss_winmain_trace("20 sound mode ok, about to queue main-loop command");
	if ( bDoLoad )
		NMainLoop::Command( new NMainLoop::CICLoad( string( NMainLoop::S_SLOT_QUICKSAVE ) ) );
	else
		NMainLoop::Command( new CICInterMission( szCfg ) );
	ss_winmain_trace("21 main-loop command queued, entering forever loop");
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
