#include "StdAfx.h"
static void ss_im_trace(const char* s) {
	FILE* fp = NULL; fopen_s(&fp, "silent_storm_step_trace.log", "a");
	if (fp) { fprintf(fp, "[IM] %s\n", s); fclose(fp); }
}
// silent-storm-port Phase 1.5 r2: text draw re-enabled — investigating what
// breaks so the main menu can actually paint.  Define SS_PHASE1_5_SKIP_TEXT_DRAW
// to fall back to the round-1 behaviour (clear-only).
//#define SS_PHASE1_5_SKIP_TEXT_DRAW 1
#include "iMain.h"
#include "G2DView.h"
#include "GSceneUtils.h"
#include "Transform.h"
#include "Camera.h"
#include "RPGGlobal.h"
#include "A5Script.h"
#include "..\Input\Bind.h"
#include "..\MiscDll\Commands.h"
#include "..\MiscDll\LogStream.h"
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
	NInput::CBind bindClose;
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
	void Initialize( NDb::CUIContainer *pUI, NDb::CUITexture *pBackground, const CArray2D<NGfx::SPixel8888> &sScreenShot, bool bScreenShot );

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
	bindClose( "cancel" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterMissionInterface::Initialize( const string &szConfig, const wstring &wsMessage )
{
	ss_im_trace("    IMI::Init.0 entry");
	pCursor = NUI::ICursor::Create();
	ss_im_trace("    IMI::Init.1 Cursor created");
	pInterface = new NUI::CInterface( pCursor );
	ss_im_trace("    IMI::Init.2 CInterface created");
	pText = new NUI::CText( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_TRANSPARENT | NUI::STYLE_TOPMOST ) );
	ss_im_trace("    IMI::Init.3 CText created");
	if ( wsMessage.empty() )
		pText->SetText( L"<font face=Courier size=30pt><color=red><center>INTERMISSION<br><color=white>ESC - exit<br>Use console to start game" );
	else
		pText->SetText( L"<font face=Courier size=30pt><color=red><center>INTERMISSION<br><color=white>ESC - exit<br><color=red>Game stop with message: " + wsMessage );
	ss_im_trace("    IMI::Init.4 SetText ok");
	if ( !szConfig.empty() ) {
		char _buf[128]; sprintf_s(_buf, "    IMI::Init.5 about to LoadConfig '%s'", szConfig.c_str()); ss_im_trace(_buf);
		NGlobal::LoadConfig( szConfig.c_str() );
		ss_im_trace("    IMI::Init.6 LoadConfig returned");
	}
	ss_im_trace("    IMI::Init.7 done");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterMissionInterface::Initialize( NDb::CUIContainer *pUI, NDb::CUITexture *pBackground, const CArray2D<NGfx::SPixel8888> &sScreenShot, bool bScreenShot )
{
	pCursor = NUI::ICursor::Create( false );
	pInterface = new NUI::CInterface( pCursor );

	if ( IsValid( pUI ) )
		NUI::LoadTemplate( pInterface, pUI );

	if ( IsValid( pBackground ) )
	{
		NUI::CImage *pImage = new NUI::CImage( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_BOTTOMMOST ) );
		pImage->SetImage( pBackground );
	}

	if ( bScreenShot )
	{
		NUI::CScreenShot *pScreenShot = new NUI::CScreenShot( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_BOTTOMMOST ) );
		pScreenShot->Set( sScreenShot );
		pScreenShot->SetMode( NUI::CScreenShot::COLOR, CVec4( 0.75f, 0.75f, 0.75f, 1 ) );
	}
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

	if ( bindClose.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( 0 ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterMissionInterface::Step()
{
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"IM::Step #%d enter\n",n); fclose(_f);} ++n; } }
	MarkNewDGFrame();
	if ( CanRender() )
	{
		{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
		  if(_f){fprintf(_f,"IM::Step #%d about to UpdateCursor\n",n); fclose(_f);} ++n; } }
		pInterface->UpdateCursor();
		{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
		  if(_f){fprintf(_f,"IM::Step #%d about to Step\n",n); fclose(_f);} ++n; } }
		pInterface->Step( GetTime() );
		{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
		  if(_f){fprintf(_f,"IM::Step #%d about to RenderFrame\n",n); fclose(_f);} ++n; } }
		RenderFrame();
		{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
		  if(_f){fprintf(_f,"IM::Step #%d RenderFrame ok\n",n); fclose(_f);} ++n; } }
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterMissionInterface::RenderFrame()
{
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"IM::RenderFrame #%d ClearScreen\n",n); fclose(_f);} ++n; } }
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
#ifdef SS_PHASE1_5_SKIP_TEXT_DRAW
	// silent-storm-port Phase 1.5: text rendering still crashes in the font
	// pipeline (font texture not bound to the bgfx facade yet). Skip Draw
	// entirely so the main loop can run and we see the clear-screen color.
#else
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"IM::RenderFrame #%d pInterface->Draw\n",n); fclose(_f);} ++n; } }
	pInterface->Draw( GetTime() );
#endif
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"IM::RenderFrame #%d Flip\n",n); fclose(_f);} ++n; } }
	NGScene::Flip();
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"IM::RenderFrame #%d Flip ok\n",n); fclose(_f);} ++n; } }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMD
////////////////////////////////////////////////////////////////////////////////////////////////////
static void StartMainMenu( const vector<wstring> &szParams, void *pContext )
{
	csSystem << CC_BLUE << "Main menu..." << endl;
	NMainLoop::Command( new NGame::CICMainMenu() );
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
	ss_im_trace("CICInterMission::Exec entry");
	ResetStack();
	ss_im_trace("  ResetStack ok");
	CInterMissionInterface *pRes = new CInterMissionInterface();
	ss_im_trace("  new CInterMissionInterface ok");
	pRes->Initialize( szConfig, wsMessage );
	ss_im_trace("  Initialize ok");
	SetInterface( pRes );
	ss_im_trace("  SetInterface ok");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICLoadWait
////////////////////////////////////////////////////////////////////////////////////////////////////
CICInterMissionWait::CICInterMissionWait( NDb::CUITexture *_pBackground ):
	pBackground( _pBackground ), bScreenShot( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CICInterMissionWait::CICInterMissionWait( NDb::CUIContainer *_pInterface, const CArray2D<NGfx::SPixel8888> &_sScreenShot ):
	pInterface( _pInterface ), sScreenShot( _sScreenShot ), bScreenShot( true )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICInterMissionWait::Exec()
{
	CInterMissionInterface *pRes = new CInterMissionInterface();
	pRes->Initialize( pInterface, pBackground, sScreenShot, bScreenShot );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
REGISTER_SAVELOAD_CLASS( 0x02511018, CInterMissionInterface );
