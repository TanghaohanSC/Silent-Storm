#include "StdAfx.h"
#include "..\Main\Gfx.h"
#include "..\Main\GInit.h"
#include "WinFrame.h"
#include "..\Main\iMain.h"
#include "..\Input\Bind.h"
#include "..\FileIO\BasicDB.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Main\GResource.h" // CRAP за неимением лучшего, вообще-то должна быть поддержка версий
#include "..\Main\Sound.h"
#include "..\DBFormat\DataMap.h"
#include <crtdbg.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool bExitSampling;
#include "DbgHelp.h"
#include "psapi.h"
//#include <vtuneapi.h>
//#pragma comment(lib, "vtuneapi.lib")
#pragma comment(lib, "psapi.lib")
namespace NGfx
{
	extern int nTotalFrames;
	extern NHPTimer::STime timeFrameStart;
}
static int nLimitFrame = 150;
static HANDLE hMainThread;
static DWORD samples[1024];
static int nSamplesPtr;
struct Int { int n; Int() : n(0) {} };
static hash_map<DWORD,Int> gatheredSamples;
static void DumpSamples()
{
	for ( int k = 0; k < nSamplesPtr; ++k )
		++gatheredSamples[samples[k]].n;
}
static DWORD WINAPI VTuneThread( void* )
{
	MODULEINFO moduleInfo;
	GetModuleInformation( GetCurrentProcess(), GetModuleHandle(0), &moduleInfo, sizeof(moduleInfo) );
	DWORD nStart = 0x400000;
	SymInitialize(GetCurrentProcess(), 0, TRUE );
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
	for(;!bExitSampling;)
	{
		Sleep(1);
		if ( NGfx::nTotalFrames < nLimitFrame )
			continue;
		NHPTimer::STime t = NGfx::timeFrameStart;
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_CONTROL;
		SuspendThread( hMainThread );
		GetThreadContext( hMainThread, &ctx );
		STACKFRAME stackFrame;
		Zero( stackFrame );
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrPC.Offset = ctx.Eip;
		//stackFrame.AddrPC.Segment = ctx.SegCs;
		stackFrame.AddrStack.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = ctx.Esp;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = ctx.Ebp;
		//stackFrame.AddrFrame.Segment = ctx.SegSs;
		DWORD dwAddress = 0;
		for( int nSafetyCounter = 0; nSafetyCounter < 100; ++nSafetyCounter )
		{ 
				//(PREAD_PROCESS_MEMORY_ROUTINE)CH_ReadProcessMemory,
			bool bOk = StackWalk( IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), hMainThread, 
				&stackFrame, &ctx, 0, SymFunctionTableAccess, SymGetModuleBase, 0 );
			if ( !bOk || stackFrame.AddrPC.Offset == 0 )
				break;
			if ( stackFrame.AddrPC.Offset > nStart && stackFrame.AddrPC.Offset < nStart + moduleInfo.SizeOfImage )
			{
				dwAddress = stackFrame.AddrPC.Offset;
				samples[nSamplesPtr++] = dwAddress;
				if ( nSamplesPtr == ARRAY_SIZE(samples) )
				{
					DumpSamples();
					nSamplesPtr = 0;
				}
				//break;
			}
		}
		ResumeThread( hMainThread );
//		if ( NHPTimer::GetTimePassed( &t ) > 1 / 20.0f )
//			VTResume();
//		else
//			VTPause();
		// don`t forget about sampling during application shut down
	}
	DumpSamples();
	SymCleanup( GetCurrentProcess() );
	FILE *p = fopen( "samples.txt", "w" );
	for ( hash_map<DWORD,Int>::iterator i = gatheredSamples.begin(); i != gatheredSamples.end(); ++i )
	{
		char szBuf[1024];
		int nOffset = i->first;
		sprintf( szBuf, "0x%x %d %d\n", nOffset - 0x401000, nOffset - 0x400000, i->second.n );
		fwrite( szBuf, strlen(szBuf), 1, p );
	}
	fclose(p);
	return 0;
}
static void StartLagProfiling()
{
	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, 0, FALSE,  DUPLICATE_SAME_ACCESS );
	DWORD dwThread;
	CreateThread( 0, 1024, VTuneThread, 0, 0, &dwThread );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma comment(linker, "/include:_ForceDataFormat")
#pragma comment(linker, "/include:_ForceGSceneGraph")
#pragma comment(linker, "/include:_ForceFontFormat")
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SWinToInputMessageConverter
{
	string szCharBuffer;
	
	void ParseChars()
	{
		if ( szCharBuffer.empty() )
			return;
		wstring szRes;
		NStr::ToUnicode( &szRes, szCharBuffer );
		for ( int k = 0; k < szRes.size(); ++k )
			NInput::AddWinMessage( NInput::CT_WIN_CHAR, szRes[k] );
		szCharBuffer = "";
	}
	void Do()
	{
		NWinFrame::SWindowsMsg wMsg;
		while ( NWinFrame::GetMessage( &wMsg ) )
		{
			if ( wMsg.msg == NWinFrame::SWindowsMsg::CHAR )
			{
				for ( int k = 0; k < wMsg.nRep; ++k )
					szCharBuffer += (char)wMsg.nKey;
			}
			else if ( wMsg.msg == NWinFrame::SWindowsMsg::KEY_DOWN )
			{
				ParseChars();
				for ( int k = 0; k < wMsg.nRep; ++k )
					NInput::AddWinMessage( NInput::CT_WIN_KEY, wMsg.nKey );
			}
		}
		ParseChars();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool CheckCD( const char *pszMessage, const char *pszCaption );
static bool CheckTimeExpiration()
{
	CMemoryStream stuff;
	FILETIME date;
	if ( !GetFile( ".\\res\\skeletons.res", 1003665276, &stuff, &date ) )
		return false;
	SYSTEMTIME cur;
	FILETIME curTime;
	GetLocalTime( &cur );
	SystemTimeToFileTime( &cur, &curTime );
	return CompareFileTime( &curTime, &date ) == 1;
}
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	StartLagProfiling();
#ifdef _DEBUG
  int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	//tmpFlag |= _CRTDBG_LEAK_CHECK_DF;// | _CRTDBG_CHECK_ALWAYS_DF;
  tmpFlag = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;// | _CRTDBG_CHECK_ALWAYS_DF;
  _CrtSetDbgFlag( tmpFlag );
	//_CrtSetBreakAlloc( 114 );
#else
	srand( GetTickCount() );
#endif // _DEBUG
	HANDLE hInstanceMutex = CreateMutex( 0, TRUE, "Silent Storm" ); // при выходе система сама должна закрыть hInstanceMutex
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		MessageBox( 0, "Game already started", "Error", MB_OK );
		return 0;
	}
	if ( !CheckTimeExpiration() )
	{
		NGScene::AddResourceDir( ".\\res" );
		NGScene::RunResourceLoadingThread();
	}
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
	//if ( !CheckCD( "Please insert Silent Storm CD", "Warning" ) )
	//	return 0;
	NDb::STranslationStatus translation;
	NDatabase::Import( true );
	NDb::BuildMapLinks( translation );

	// init subsystems
	if ( !NWinFrame::InitApplication( hInstance, " Silent Storm", " Silent Storm" ) ) // такое имя окна чекает auto run
		return 0;
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
	vector<string> mods;
	for ( int i = 0; i < szParams.size(); ++i )
	{
		if ( szParams[i] == "-window" )
			NGlobal::SetVar( "gfx_fullscreen", 0 );
		else if ( szParams[i] == "-fullscreen" )
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
			NGlobal::SetVar( "gfx_anisotropic_filter", 2 );
		else if ( szParams[i] == "-bannp2" )
			NGlobal::SetVar( "gfx_fix_ban_np2", 1 );
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
		if ( szParams[i] == "-translate" )
			translation.bDoTranslate = true;
		if ( szParams[i] == "-tr_original" )
			translation.bUseTranslOrig = true;
		if ( szParams[i] == "-ntr_original" )
			translation.bUseNotTranslOrig = true;
		if ( szParams[i] == "-od_original" )
			translation.bUseOutDatedOrig = true;
		if ( szParams[i] == "-warn_original" )
			translation.bUseWarnOrig = true;
		if ( szParams[i] == "-warn_translation" )
			translation.bUseWarnTranslation = true;
		if ( szParams[i] == "-snd_original" )
			translation.bUseSndOriginal = true;
		if ( szParams[i] == "-snd_nothing" )
			translation.bUseSndNothing = true;
		if ( szParams[i] == "-gamedir" )
		{
			if ( i + 1 < szParams.size() )
				mods.push_back( szParams[++i] );
		}
	}
	//
	for( int i = 0; i < mods.size(); ++i )
	{
		const string &szDir = mods[i];
		try
		{
			CFileStream f;
			f.OpenRead( (szDir + "\\game.db").c_str() );
			NDatabase::Serialize( f, CStructureSaver::READ );
			NDatabase::Import( true );
			NDb::BuildMapLinks( translation );
		}
		catch (...)
		{
			// game.db у мода не обязательно должен быть
			//ASSERT( 0 ); // game.db not found
			//string msg = string( "File " ) + szDir + "\\game.db not found";
			//MessageBox( 0, msg.c_str(), "Silent Storm", MB_OK | MB_ICONWARNING );
		}
		NGScene::AddResourceDir( (string( ".\\" ) + szDir).c_str() );
	}
	//
	if ( !NGScene::SetModeFromConfig( true ) )
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
	if ( !NMainLoop::InitInterface( bDoLoad, szCfg ) )
	{
		ASSERT(0);
		MessageBox( 0, "Failed to initialize interface", "Error", MB_OK );
		return 0;
	}
	//
	for (;;)
	{
		NWinFrame::PumpMessages();
		bool bActive = NWinFrame::IsAppActive();
		NInput::PumpMessages( bActive );
		// feed win32 messages to Input

		SWinToInputMessageConverter convert;
		convert.Do();
		
		if ( NWinFrame::IsExit() )
			break;
		if ( !NMainLoop::StepApp( bActive, bActive ) )
			break;
		if ( !bActive )
			Sleep( 40 );
		NHPTimer::UpdateHPTimerFrequency();
	}
	bExitSampling = true;
	//
	NGlobal::SaveConfig( ".\\cfg\\config.cfg" );
	NMainLoop::DoneInterface();
	NGfx::Done3D();
	NInput::DoneInput();
	NSound::DoneSound();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
