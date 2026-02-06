#include "StdAfx.h"
#include "iMain.h"
#include "GView.h"
#include "Gfx.h"
#include "GScene.h"
#include "G2DView.h"
#include "A5Script.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Misc\LogStream.h"
#include "..\Misc\BasicShare.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
extern void DumpMemoryStats();
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NMainLoop
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const string szSavePath( ".\\save\\" );
static STime currentTime;
static bool bAppIsActive = false;
static list< CPtr<CInterfaceCommand> > cmds;
static list< CObj<IInterfaceBase> > interfaces;
////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowLogo()
{
	CObj<NGScene::I2DGameView> p2DScene = NGScene::CreateNew2DView();

	NGScene::ClearScreen( CVec3( 0, 0, 0 ) );
	NDb::CTexture *pLogo = NDb::GetTexture( 1744 );
	if ( pLogo )
	{
		CVec2 vSize = p2DScene->GetViewportSize();
		CTRect<int> window( 0, 0, Float2Int(vSize.x), Float2Int(vSize.y) );
		CRectLayout rl;

		p2DScene->StartNewFrame();
		rl.scale.x = vSize.x / 1024.0f;
		rl.scale.y = vSize.y / 768.0f;
		rl.AddRect( 0, 0, CRectLayout::STextureCoord( CTRect<float>( 0, pLogo->nHeight, pLogo->nWidth, 0 ) ) );
		p2DScene->CreateDynamicRects( pLogo,  rl, CTPoint<int>( 0, 0 ), window );
		p2DScene->Flush();
	}
	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInterfaceCommand
void CInterfaceCommand::ResetStack()
{
	interfaces.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterfaceCommand::SetInterface( IInterfaceBase *pNewInterface )
{
	ASSERT( IsValid( pNewInterface ) );
	interfaces.clear();
	interfaces.push_back( pNewInterface );
	pNewInterface->OnGetFocus();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IInterfaceBase* CInterfaceCommand::GetInterface() const
{
  if ( interfaces.empty() )
    return 0;
  return interfaces.back();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterfaceCommand::PushInterface( IInterfaceBase *pNewInterface )
{
	ASSERT( IsValid( pNewInterface ) );
	interfaces.push_back( pNewInterface );
	pNewInterface->OnGetFocus();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInterfaceCommand::PopInterface()
{
	if ( interfaces.empty() )
		return;
	interfaces.pop_back();
	if ( !interfaces.empty() )
		interfaces.back()->OnGetFocus();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInterfaceObject
////////////////////////////////////////////////////////////////////////////////////////////////////
const STime IInterfaceObject::GetTime()
{
	return currentTime;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInterfaceBase
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IInterfaceBase::CanRender()
{
	if ( bAppIsActive && NGScene::Is3DActive() )
		return true;
	Sleep( 10 );
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICLoad
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICLoad::Exec()
{
	ShowLogo();
	string szFileWithPath = szSavePath + szFileName;
#ifndef _DEBUG
	try
#endif
	{
		CFileStream f;
		f.OpenRead( szFileWithPath.c_str() );
		CSharedHolder hold;
		interfaces.clear();
		CStructureSaver file( f, CStructureSaver::READ );
		file.Add( 2, &interfaces );
		SerializeShared( &file );
		ASSERT( !interfaces.empty() );
	}
#ifndef _DEBUG
	catch(...)
	{
	}
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICSave
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICSave::Exec()
{
	string szFileWithPath = szSavePath + szFileName;
#ifndef _DEBUG
	try
#endif
	{
		CFileStream f;
		f.OpenWrite( szFileWithPath.c_str() );
		CStructureSaver file( f, CStructureSaver::WRITE );
		file.Add( 2, &interfaces );
		SerializeShared( &file );
	}
#ifndef _DEBUG
	catch(...)
	{
	}
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICExitModal
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICExitModal::Exec() 
{
	PopInterface(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// NMainLoop
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool bWireFrame = false;
static bool bShowFPSStats = false;
static NInput::CBind cWireframe( "wireframe" ), cLoad( "load" ), cSave( "save" ), cScreenShot( "screenshot" ), cDump( "memorystats" );
////////////////////////////////////////////////////////////////////////////////////////////////////
static void ProcessStandardEvents( const NInput::SEvent &eEvent )
{
	if ( cWireframe.ProcessEvent( eEvent ) )
		NGScene::SetWireframe( bWireFrame = !bWireFrame );
	else if ( cLoad.ProcessEvent( eEvent ) )
	{
		Command( new NMainLoop::CICLoad("quicksave.sav") );
		csGame << "QuickLoad done." << endl;
	}
	else if ( cSave.ProcessEvent( eEvent ) )
	{
		Command( new NMainLoop::CICSave("quicksave.sav") );
		csGame << "QuickSave done." << endl;
	}
	else if ( cScreenShot.ProcessEvent( eEvent ) )
	{
		NGScene::MakeScreenShot();
		csGame << "ScreenShot created." << endl;
	}
	else if ( cDump.ProcessEvent( eEvent ) )
		DumpMemoryStats();

	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool ProcessInterfaceCmds()
{
	while ( !cmds.empty() )
	{
		CPtr<CInterfaceCommand> pCmd = cmds.front();
		cmds.pop_front();
		if ( !IsValid( pCmd ) )
			return false;
		pCmd->Exec();
	}
	return !interfaces.empty();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool StepApp( bool bActive, bool bSetGamma )
{
	if ( bActive )
		NGfx::CheckBackBufferSize();
	bAppIsActive = bActive;
	NGfx::SetGamma( bSetGamma );
	if ( !ProcessInterfaceCmds() )
		return false;
	ASSERT( IsValid( interfaces.back() ) );

	// commands processing
	NInput::SEvent event;
	while ( NInput::GetEvent( &event ) )
	{
		if ( !interfaces.back()->ProcessEvent( event ) )
			ProcessStandardEvents( event );
		if ( !ProcessInterfaceCmds() )
			return false;
	}
	interfaces.back()->ProcessEvent( event );
	if ( !ProcessInterfaceCmds() )
		return false;

	currentTime = event.mMessage.tTime;
	interfaces.back()->Step();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DoneInterface()
{
	interfaces.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Command( CInterfaceCommand *pCmd )
{
	cmds.push_back( pCmd );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetInterfaceStackDepth()
{
	return interfaces.size();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void LoadSavedGame( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
	{
		csSystem << "load \"name\"" << endl;
		return;
	}

	csSystem << "loading " << szParams.front() << "..." << endl;
	Command( new CICLoad( NStr::ToAscii( szParams.front() ).c_str() ) );
	csSystem << "done." << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SaveGame( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
	{
		csSystem << "save \"name\"" << endl;
		return;
	}

	csSystem << "saving " << szParams.front() << "..." << endl;
	Command( new CICSave( NStr::ToAscii( szParams.front() ).c_str() ) );
	csSystem << "done." << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(iMain)
	REGISTER_CMD( "load", LoadSavedGame )
	REGISTER_CMD( "save", SaveGame )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace 
////////////////////////////////////////////////////////////////////////////////////////////////////
