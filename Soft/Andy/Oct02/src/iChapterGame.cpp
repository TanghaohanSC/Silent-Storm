#include "StdAfx.h"
#include "iMain.h"
#include "GView.h"
#include "G2DView.h"
#include "wInterface.h"
#include "RWGame.h"
#include "RWSound.h"
#include "GSceneUtils.h"
#include "GResource.h"
#include "Transform.h"
#include "Camera.h"
#include "Grid.h"
#include "RPGGame.h"
#include "RPGGlobal.h"
#include "iMain.h"
#include "iInterMission.h"
#include "Interface.h"
#include "iGlobalGame.h"
#include "iGlobalGameInternal.h"
#include "iChapterGame.h"
#include "iChapterMap.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Misc\BasicShare.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
static CBasicShare<int, CChapterInfoLoader> shareChapterInfo(140);
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CChapterGame
////////////////////////////////////////////////////////////////////////////////////////////////////
class CChapterGame: public IChapterGame
{
	OBJECT_BASIC_METHODS(CChapterGame)
private:
	ZDATA_(CGlobalGame)
	int nTemplateID;
	CPtr<NRPG::CGlobalGame> pGame;
	//// chapter
	CDGPtr<CPtrFuncBase<CChapterInfo> > pChapterInfo;
	//// interface
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	//// sound
	CObj<NSound::ISoundScene> pSoundScene;
	ZEND

public:
	CChapterGame();

	void Initialize( int nTemplateID, NRPG::CGlobalGame* pGame );

	NUI::ICursor* GetCursor() const;
	NUI::CInterface* GetInterface() const;

	NRPG::CGlobalGame* GetGlobalGame() const;
	NSound::ISoundScene* GetSoundScene() const;
	CPtrFuncBase<CChapterInfo>* GetChapterInfo() const;

	void Step( const STime &sTime );
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void RenderFrame( const STime &sTime );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CChapterGame::CChapterGame()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CChapterGame::Initialize( int _nTemplateID, NRPG::CGlobalGame *_pGame )
{
	pGame = _pGame;
	nTemplateID = _nTemplateID;

	pChapterInfo = shareChapterInfo.Get( nTemplateID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::ICursor* CChapterGame::GetCursor() const
{
	return pCursor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NUI::CInterface* CChapterGame::GetInterface() const
{
	return pInterface;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CGlobalGame* CChapterGame::GetGlobalGame() const
{
	return pGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NSound::ISoundScene* CChapterGame::GetSoundScene() const
{
	return pSoundScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CPtrFuncBase<CChapterInfo>* CChapterGame::GetChapterInfo() const
{
	return pChapterInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CChapterGame::Step( const STime &sTime )
{
	pInterface->UpdateCursor();
	pInterface->Step( sTime );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CChapterGame::ProcessEvent( const NInput::SEvent &sEvent )
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CChapterGame::RenderFrame( const STime &sTime )
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( sTime );
	NGScene::Flip();
	MarkNewDGFrame();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CreateChapterGame
////////////////////////////////////////////////////////////////////////////////////////////////////
IChapterGame* CreateChapterGame( int nTemplate, NRPG::CGlobalGame *pGame )
{
	CChapterGame *pChapterGame = new CChapterGame;
	pChapterGame->Initialize( nTemplate, pGame );
	return pChapterGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface commands
////////////////////////////////////////////////////////////////////////////////////////////////////
CICBeginChapter::CICBeginChapter( int _nTemplate, NRPG::CGlobalGame* _pGame ):
	nTemplate( _nTemplate ), pGame( _pGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICBeginChapter::Exec()
{
	IChapterGame* pGame = CreateChapterGame( nTemplate, pGame );
	NMainLoop::Command( new CICChapterMap( pGame ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CICContinueChapter::CICContinueChapter( NRPG::CGlobalGame* _pGame ):
	pGame( _pGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICContinueChapter::Exec()
{
//	IChapterGame* pGame = CreateChapterGame( nTemplate, gamesSet );
//	NMainLoop::Command( new CICChapterMap( pGame ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Start ChapterGame
////////////////////////////////////////////////////////////////////////////////////////////////////
static void StartGame( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
	{
		csSystem << "usage: #zone" << endl;
		return;
	}

	int nTemp = wcstol( szParams.front().data(), 0, 10 );
	csSystem << CC_BLUE << "Loading game ( template " << nTemp << " ) ..." << endl;

	vector< CPtr<NRPG::CGlobalGame> > gamesSet;
	gamesSet.push_back( CreateNewGame() );
	NMainLoop::Command( new NGame::CICBeginChapter( nTemp, gamesSet ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB2406140, CChapterGame )
START_REGISTER(iChapterGame)
	REGISTER_CMD( "chapter", StartGame )
FINISH_REGISTER
