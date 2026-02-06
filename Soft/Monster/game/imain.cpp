#include "StdAfx.h"
#include "iMain.h"
#include "iInput.h"
#include "iMission.h"
#include "GScene.h"
/////////////////////////////////////////////////////////////////////////////////////
using namespace NMainLoop;
/////////////////////////////////////////////////////////////////////////////////////
static list< CPtr<CInterfaceCommand> > cmds;
static CPtr<CInterfaceBase> pIface;
/////////////////////////////////////////////////////////////////////////////////////
// CInterfaceCommand
void CInterfaceCommand::SetInterface( CInterfaceBase *pNewInterface )
{
	pIface = pNewInterface;
}
/////////////////////////////////////////////////////////////////////////////////////
// CInterfaceBase
/////////////////////////////////////////////////////////////////////////////////////
bool CInterfaceBase::CanRender()
{
	if ( NInput::IsActive() && NGScene::Is3DActive() )
		return true;
	Sleep( 10 );
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////
// CICLoad
/////////////////////////////////////////////////////////////////////////////////////
void CICLoad::Exec()
{
	CFileStream f;
	if ( f.OpenRead( szFileName.c_str() ) )
	{
		pIface = 0;
		CStructureSaver file( f, CStructureSaver::READ );
		file.AddObject( 1, &pIface );
		ASSERT( pIface->IsValid() );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// CICSave
/////////////////////////////////////////////////////////////////////////////////////
void CICSave::Exec()
{
	CFileStream f;
	if ( f.OpenWrite( szFileName.c_str() ) )
	{
		CStructureSaver file( f, CStructureSaver::WRITE );
		file.AddObject( 1, &pIface );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// NMainLoop
/////////////////////////////////////////////////////////////////////////////////////
bool NMainLoop::StepApp()
{
	while ( !cmds.empty() )
	{
		CInterfaceCommand *pCmd = cmds.front();
		if ( !pCmd->IsValid() )
			return false;
		pCmd->Exec();
		cmds.pop_front();
	}
	if ( !pIface->IsValid() )
		return false;
	pIface->Step();
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
void NMainLoop::Command( CInterfaceCommand *pCmd )
{
	cmds.push_back( pCmd );
}
/////////////////////////////////////////////////////////////////////////////////////
enum ECmd
{
	CMD_WIREFRAME = 0x10000000,
	CMD_SAVE = 0x10000001,
	CMD_LOAD = 0x10000002,
};
static bool bWireFrame = false;
void NMainLoop::ProcessStandardMsgs( NInput::SMessage &msg )
{
	switch ( msg.nEventID )
	{
		case CMD_WIREFRAME:
			NGScene::SetWireframe( bWireFrame = !bWireFrame );
			break;
		case CMD_SAVE:
			Command( new NMainLoop::CICSave("game.dat") );
			break;
		case CMD_LOAD:
			Command( new NMainLoop::CICLoad("game.dat") );
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void NMainLoop::BindStandardMsgs()
{
	NInput::RegisterEvent( CMD_WIREFRAME, "wireframe" );
	NInput::RegisterEvent( CMD_SAVE, "save" );
	NInput::RegisterEvent( CMD_LOAD, "load" );
	NInput::Bind( "wireframe", 'W' );
	NInput::Bind( "save", VK_F5 );
	NInput::Bind( "load", VK_F8 );
}
/////////////////////////////////////////////////////////////////////////////////////
extern void RegisterMissionInterfaceClasses( int nBase );
void RegisterInterfaceClasses( int nBase )
{
	RegisterMissionInterfaceClasses( nBase );//+ 0x100000 ); 
}
/////////////////////////////////////////////////////////////////////////////////////
