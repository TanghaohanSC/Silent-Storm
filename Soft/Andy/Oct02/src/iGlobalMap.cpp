#include "StdAfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "RPGGlobal.h"
#include "Sound.h"
#include "..\Input\Bind.h"
#include "iInterMission.h"
#include "iGlobalMap.h"
#include "iCluesMenu.h"
#include "iInGameMenu.h"
#include "Interface.h"
#include "iGlobalMapUI.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Misc\BasicShare.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
static CBasicShare<int, CGlobalInfoLoader> shareGlobalInfo(141);
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalMap
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGlobalMap: public IGlobalMap
{
	OBJECT_NOCOPY_METHODS(CGlobalMap);
private:
	NInput::CBind bindExit, bindMenu, bindJournal;
	ZDATA
	CPtr<NRPG::CGlobalGame> pGame;
	//// global
	CDGPtr<CPtrFuncBase<CGlobalInfo> > pGlobalInfo;
	//// interface
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	CObj<NUI::CGlobalMapUI> pGlobalMapUI;
	//// sound
	CObj<NSound::ISoundScene> pSoundScene;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pGame); f.Add(3,&pGlobalInfo); f.Add(4,&pCursor); f.Add(5,&pInterface); f.Add(6,&pGlobalMapUI); f.Add(7,&pSoundScene); return 0; }

protected:
	void RenderFrame( const STime &sTime );

public:
	CGlobalMap();

	void Initialize( NRPG::CGlobalGame* pGame );

	float GetGlobalVar( const string &szID, float fDefault = 0 ) const;
	void SetGlobalVar( const string &szID, float fValue );

	NUI::ICursor* GetCursor() const;
	NUI::CInterface* GetInterface() const;

	NRPG::CGlobalGame* GetGlobalGame() const;
	NSound::ISoundScene* GetSoundScene() const;
	CPtrFuncBase<CGlobalInfo>* GetGlobalInfo() const;

	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void Step();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CGlobalMap::CGlobalMap():
	bindExit( "exitgame" ), bindMenu( "menu" ), bindJournal( "clues" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalMap::Initialize( NRPG::CGlobalGame *_pGame )
{
	pGame = _pGame;

	pGlobalInfo = shareGlobalInfo.Get( pGame->nGlobalMapID );

	pSoundScene = NSound::CreateSoundScene( 0 ); // CRAP
#ifdef _MAPEDIT
	pCursor = NUI::ICursor::CreateEditorCursor();
#else
	pCursor = NUI::ICursor::Create( true, NGfx::GetScreenRect() / 2 );
#endif

	pInterface = new NUI::CInterface( pCursor );

	pGlobalMapUI = new NUI::CGlobalMapUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "globalmapUI" ), this );
	NUI::LoadTemplate( pGlobalMapUI, NDb::GetUIContainer( 175 ) );
	pGlobalMapUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CGlobalMap::GetGlobalVar( const string &szID, float fDefault ) const
{
	hash_map<string,float>::const_iterator iTemp = pGame->globalVars.find( szID );
	if ( iTemp == pGame->globalVars.end() )
		return fDefault;

	return iTemp->second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalMap::SetGlobalVar( const string &szID, float fValue )
{
	pGame->globalVars[szID] = fValue;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::ICursor* CGlobalMap::GetCursor() const
{
	return pCursor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::CInterface* CGlobalMap::GetInterface() const
{
	return pInterface;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CGlobalGame* CGlobalMap::GetGlobalGame() const
{
	return pGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NSound::ISoundScene* CGlobalMap::GetSoundScene() const
{
	return pSoundScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CPtrFuncBase<CGlobalInfo>* CGlobalMap::GetGlobalInfo() const
{
	return pGlobalInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalMap::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGlobalMap::ProcessEvent( const NInput::SEvent &sEvent )
{
	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( 0 ); 
		return true;
	}

	if ( bindMenu.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICInGameMenu( pGame, GLOBALMAP ) ); 
		return true;
	}
	else if ( bindJournal.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICClues( pGame ) ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalMap::Step()
{
	if ( CanRender() )
	{
		pInterface->UpdateCursor();
		pInterface->Step( GetTime() );
		RenderFrame( GetTime() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalMap::RenderFrame( const STime &sTime )
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( sTime );
	NGScene::Flip();
	MarkNewDGFrame();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICBeginGlobal
////////////////////////////////////////////////////////////////////////////////////////////////////
CICBeginGlobal::CICBeginGlobal( int _nTemplateID, const vector<CObj<NRPG::CGlobalPlayer> > &_playersSet ):
	nTemplateID( _nTemplateID ), playersSet( _playersSet )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICBeginGlobal::Exec()
{
	CDGPtr<CPtrFuncBase<CGlobalInfo> > pGlobalInfo = shareGlobalInfo.Get( nTemplateID );
	pGlobalInfo.Refresh();

	CPtr<NRPG::CGlobalGame> pGame = NRPG::CreateGlobalGame( pGlobalInfo->GetValue()->nScenarioID );
	pGame->players = playersSet;

	pGame->bGlobalMapSet = true;
	pGame->nGlobalMapID = nTemplateID;

	CGlobalMap *pRes = new CGlobalMap();
	pRes->Initialize( pGame );
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICGlobalMap
////////////////////////////////////////////////////////////////////////////////////////////////////
CICContinueGlobal::CICContinueGlobal( NRPG::CGlobalGame *_pGame ):
	pGame( _pGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICContinueGlobal::Exec()
{
	if ( !pGame->bGlobalMapSet )
	{
		ASSERT( 0 );
		string szCfg("");
		NMainLoop::Command( new CICInterMission( szCfg, L"Can't continue global! No global set." ) );
		return;
	}

	CGlobalMap *pRes = new CGlobalMap();
	pRes->Initialize( pGame );
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB1808180, CGlobalMap )
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandStartGlobal( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	if ( paramsSet.empty() )
	{
		csSystem << "usage:" << szID << "#global" << endl;
		return;
	}

	int nTemp = wcstol( paramsSet.front().data(), 0, 10 );
	csSystem << CC_BLUE << "Loading global ( template " << nTemp << " ) ..." << endl;

	vector<CObj<NRPG::CGlobalPlayer> > playersSet;
	playersSet.push_back( NRPG::CreateGlobalPlayer() );
	NMainLoop::Command( new CICBeginGlobal( nTemp, playersSet ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(iGlobalMap)
	REGISTER_CMD( "global", CommandStartGlobal )
FINISH_REGISTER
