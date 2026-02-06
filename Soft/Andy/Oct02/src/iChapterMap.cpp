#include "StdAfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "RPGGlobal.h"
#include "Sound.h"
#include "..\Input\Bind.h"
#include "ChapterInfo.h"
#include "iInterMission.h"
#include "iGlobalMap.h"
#include "iChapterMap.h"
#include "iCluesMenu.h"
#include "iInGameMenu.h"
#include "Interface.h"
#include "iChapterMapUI.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Misc\BasicShare.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
#include "scScenarioTracker.h"
#include "RPGMerc.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicShare<int, CChapterInfoLoader> shareChapterInfo(140);
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CChapterMap
////////////////////////////////////////////////////////////////////////////////////////////////////
class CChapterMap: public IChapterMap
{
	OBJECT_NOCOPY_METHODS(CChapterMap);
private:
	NInput::CBind bindExit;
	NInput::CBind bindExitChapter;
	NInput::CBind bindMenu, bindJournal;
	ZDATA
	CPtr<NRPG::CGlobalGame> pGame;
	//// chapter
	CDGPtr<CPtrFuncBase<CChapterInfo> > pChapterInfo;
	//// interface
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	CObj<NUI::CChapterMapUI> pChapterMapUI;
	//// sound
	CObj<NSound::ISoundScene> pSoundScene;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pGame); f.Add(3,&pChapterInfo); f.Add(4,&pCursor); f.Add(5,&pInterface); f.Add(6,&pChapterMapUI); f.Add(7,&pSoundScene); return 0; }

protected:
	void RenderFrame( const STime &sTime );

public:
	CChapterMap();

	void Initialize( NRPG::CGlobalGame* pGame );

	float GetGlobalVar( const string &szID, float fDefault = 0 ) const;
	void SetGlobalVar( const string &szID, float fValue );

	NUI::ICursor* GetCursor() const;
	NUI::CInterface* GetInterface() const;

	NRPG::CGlobalGame* GetGlobalGame() const;
	NSound::ISoundScene* GetSoundScene() const;
	CPtrFuncBase<CChapterInfo>* GetChapterInfo() const;

	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void Step();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CChapterMap::CChapterMap():
	bindExit( "exitgame" ), bindExitChapter( "exitchapter" ), bindMenu( "menu" ), bindJournal( "clues" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CChapterMap::Initialize( NRPG::CGlobalGame *_pGame )
{
	pGame = _pGame;

	pChapterInfo = shareChapterInfo.Get( pGame->nChapterMapID );

	pSoundScene = NSound::CreateSoundScene( 0 ); // CRAP
#ifdef _MAPEDIT
	pCursor = NUI::ICursor::CreateEditorCursor();
#else
	pCursor = NUI::ICursor::Create( true, NGfx::GetScreenRect() / 2 );
#endif

	pInterface = new NUI::CInterface( pCursor );

	pChapterMapUI = new NUI::CChapterMapUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "chaptermapUI" ), this );
	NUI::LoadTemplate( pChapterMapUI, NDb::GetUIContainer( 147 ) );
	pChapterMapUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CChapterMap::GetGlobalVar( const string &szID, float fDefault ) const
{
	hash_map<string,float>::const_iterator iTemp = pGame->globalVars.find( szID );
	if ( iTemp == pGame->globalVars.end() )
		return fDefault;

	return iTemp->second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CChapterMap::SetGlobalVar( const string &szID, float fValue )
{
	pGame->globalVars[szID] = fValue;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::ICursor* CChapterMap::GetCursor() const
{
	return pCursor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::CInterface* CChapterMap::GetInterface() const
{
	return pInterface;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CGlobalGame* CChapterMap::GetGlobalGame() const
{
	return pGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NSound::ISoundScene* CChapterMap::GetSoundScene() const
{
	return pSoundScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CPtrFuncBase<CChapterInfo>* CChapterMap::GetChapterInfo() const
{
	return pChapterInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CChapterMap::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CChapterMap::ProcessEvent( const NInput::SEvent &sEvent )
{
	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( 0 ); 
		return true;
	}

	if ( bindExitChapter.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICContinueGlobal( pGame ) ); 
		return true;
	}

	if ( bindMenu.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICInGameMenu( pGame, CHAPTERMAP ) ); 
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
void CChapterMap::Step()
{
	if ( CanRender() )
	{
		pInterface->UpdateCursor();
		pInterface->Step( GetTime() );
		RenderFrame( GetTime() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CChapterMap::RenderFrame( const STime &sTime )
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( sTime );
	NGScene::Flip();
	MarkNewDGFrame();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICChapterMap
////////////////////////////////////////////////////////////////////////////////////////////////////
CICBeginChapter::CICBeginChapter( int _nTemplateID, NRPG::CGlobalGame *_pGlobalGame ):
	nTemplateID( _nTemplateID ), pGlobalGame( _pGlobalGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICBeginChapter::Exec()
{
	pGlobalGame->bChapterMapSet = true;
	pGlobalGame->nChapterMapID = nTemplateID;

	CDGPtr<CPtrFuncBase<CChapterInfo> > pChapterInfo = shareChapterInfo.Get( nTemplateID );
	pChapterInfo.Refresh();
	pGlobalGame->vChapterPos = pChapterInfo->GetValue()->vDeployPos;

	CChapterMap *pRes = new CChapterMap();
	pRes->Initialize( pGlobalGame );
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICChapterMap
////////////////////////////////////////////////////////////////////////////////////////////////////
CICContinueChapter::CICContinueChapter( NRPG::CGlobalGame *_pGlobalGame ):
	pGlobalGame( _pGlobalGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICContinueChapter::Exec()
{
	if ( !pGlobalGame->bChapterMapSet )
	{
		ASSERT( 0 );
		string szCfg("");
		NMainLoop::Command( new CICInterMission( szCfg, L"Can't continue chapter! No chapters set." ) );
		return;
	}
	//
	int nSize = pGlobalGame->players.size();
	ASSERT( nSize > 0 );
	for ( int n = 0; n < nSize; ++n )
	{
		vector< CPtr<NRPG::CUnit> > units;
		pGlobalGame->players[n]->GetAliveUnits( &units );
		pGlobalGame->pScenarioTracker->ProcessScenario( units );
		//
		for ( vector<CPtr<NRPG::CMerc> >::iterator i = pGlobalGame->players[n]->mercs.begin();
			i != pGlobalGame->players[n]->mercs.end(); ++i )
				pGlobalGame->players[n]->deployData.unitsDeployData[ (*i)->pRPGUnit.GetPtr() ].pCorpse = 0;
	}
	//
	CChapterMap *pRes = new CChapterMap();
	pRes->Initialize( pGlobalGame );
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB2301230, CChapterMap )
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandStartChapter( const string &szID, const vector<wstring> &paramsSet, void *pContext );
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(iChapterMap)
	REGISTER_CMD( "chapter", CommandStartChapter )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandStartChapter( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	if ( paramsSet.empty() )
	{
		csSystem << "usage:" << szID << "#chapter" << endl;
		return;
	}

	int nTemp = wcstol( paramsSet.front().data(), 0, 10 );
	csSystem << CC_BLUE << "Loading chapter ( template " << nTemp << " ) ..." << endl;
	CPtr<NRPG::CGlobalGame> pGlobalGame = NRPG::CreateGlobalGame();
	pGlobalGame->players.push_back( NRPG::CreateGlobalPlayer() );
	NMainLoop::Command( new CICBeginChapter( nTemp, pGlobalGame ) );
}
