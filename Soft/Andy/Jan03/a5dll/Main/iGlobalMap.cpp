#include "StdAfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "RPGGlobal.h"
#include "Sound.h"
#include "..\Input\Bind.h"
#include "iSaveManager.h"
#include "iInterMission.h"
#include "iGlobalMap.h"
#include "iCluesMenu.h"
#include "iMission.h"
#include "iInGameMenu.h"
#include "Interface.h"
#include "iGlobalMapUI.h"
#include "..\Misc\StrProc.h"
#include "..\MiscDll\Commands.h"
#include "..\Misc\BasicShare.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataScenario.h"
#include "scFlowChartItems.h"
#include "scScenarioTracker.h"
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
	NInput::CBind bindClose, bindMenu, bindJournal, bindBaseZone;
	NGlobal::CCmd cmdSetDifficulty;
	ZDATA
	EMode eMode;
	CPtr<NRPG::CGlobalGame> pGame;
	//// global
	CDBPtr<NDb::CGlobalMap> pGlobalMap;
	CDGPtr<CPtrFuncBase<CGlobalInfo> > pGlobalInfo;
	//// interface
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	CObj<NUI::CGlobalMapUI> pGlobalMapUI;
	//// sound
	CObj<NSound::ISoundScene> pSoundScene;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&eMode); f.Add(3,&pGame); f.Add(4,&pGlobalMap); f.Add(5,&pGlobalInfo); f.Add(6,&pCursor); f.Add(7,&pInterface); f.Add(8,&pGlobalMapUI); f.Add(9,&pSoundScene); return 0; }

protected:
	void RenderFrame( const STime &sTime );

public:
	CGlobalMap();

	void Initialize( NRPG::CGlobalGame* pGame, EMode eMode = MODE_NORMAL );

	EMode GetMode() const;

	NUI::ICursor* GetCursor() const;
	NUI::CInterface* GetInterface() const;

	NRPG::CGlobalGame* GetGlobalGame() const;
	NRPG::CGlobalPlayer* GetGlobalPlayer() const;
	NSound::ISoundScene* GetSoundScene() const;
	NDb::CGlobalMap* GetGlobalMap() const;
	CPtrFuncBase<CGlobalInfo>* GetGlobalInfo() const;

	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void Step();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandSetDifficulty( const string &szID, const vector<wstring> &paramsSet, void *pContext );
////////////////////////////////////////////////////////////////////////////////////////////////////
CGlobalMap::CGlobalMap():
	bindClose( "cancel" ), bindMenu( "menu" ), bindJournal( "clues" ), bindBaseZone( "basezone" ),
	cmdSetDifficulty( "difficulty", CommandSetDifficulty, this )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalMap::Initialize( NRPG::CGlobalGame *_pGame, EMode _eMode )
{
	eMode = _eMode;
	pGame = _pGame;

	pGlobalMap = NDb::GetGlobalMap( pGame->nGlobalMapID );
	pGlobalInfo = shareGlobalInfo.Get( pGame->nGlobalMapID );

	pSoundScene = NSound::CreateSoundScene( 0 ); // CRAP
#ifdef _MAPEDIT
	pCursor = NUI::ICursor::CreateEditorCursor();
#else
	pCursor = NUI::ICursor::Create( true );
#endif

	pInterface = new NUI::CInterface( pCursor );

	pGlobalMapUI = new NUI::CGlobalMapUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "globalmapUI" ), this );
	NUI::LoadTemplate( pGlobalMapUI, NDb::GetUIContainer( 175 ) );
	pGlobalMapUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IGlobalMap::EMode CGlobalMap::GetMode() const
{
	return eMode;
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
NRPG::CGlobalPlayer* CGlobalMap::GetGlobalPlayer() const
{
	ASSERT( pGame->players.size() == 1 );
	return pGame->players.front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NSound::ISoundScene* CGlobalMap::GetSoundScene() const
{
	return pSoundScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CGlobalMap* CGlobalMap::GetGlobalMap() const
{
	return pGlobalMap;
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
	NInput::SetSection( "game" );

	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( ( eMode == MODE_SHOW ) && bindClose.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() ); 
		return true;
	}

	if ( bindMenu.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICInGameMenu( pGame->players.front() ) ); 
		return true;
	}
	else if ( bindJournal.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new CICClues( pGame ) ); 
		return true;
	}

	if ( bindBaseZone.ProcessEvent( sEvent ) )
	{
		vector<string> templParams;
		CPtr<NScenario::CScenarioZone> pZone = pGame->pScenarioTracker->GetZoneByDBZone( pGlobalMap->pBaseZone );
		if ( IsValid( pZone ) )
			NMainLoop::Command( new NGame::CICBeginMission( pZone, -1, templParams, pGame ) );
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
CICBeginGame::CICBeginGame( int _nTemplateID, const vector<CObj<NRPG::CGlobalPlayer> > &_playersSet ):
	nTemplateID( _nTemplateID ), playersSet( _playersSet )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// silent-storm-port r46: trace + null-guards around CICBeginGame::Exec so the
// crash chain logs known progress (was hitting AV before queueing the next
// command, leaving no useful trace).
static void ss_bg_trace(const char* s) {
	FILE* fp = NULL; fopen_s(&fp, "silent_storm_step_trace.log", "a");
	if (fp) { fprintf(fp, "[BG] %s\n", s); fclose(fp); }
}

void CICBeginGame::Exec()
{
	char _buf[256];
	sprintf_s(_buf, "BG::Exec.0 entry tmpl=%d playersSet.size=%d", nTemplateID, (int)playersSet.size());
	ss_bg_trace(_buf);
	int nScenarioID = -1;
	CPtr<NDb::CGlobalMap> pGlobalMap = NDb::GetGlobalMap( nTemplateID );
	sprintf_s(_buf, "BG::Exec.1 pGlobalMap=%p", (void*)pGlobalMap.GetPtr());
	ss_bg_trace(_buf);
	if ( IsValid( pGlobalMap ) && IsValid( pGlobalMap->pScenario ) )
	{
		nScenarioID = pGlobalMap->pScenario->GetRecordID();
		sprintf_s(_buf, "BG::Exec.2 nScenarioID=%d", nScenarioID);
		ss_bg_trace(_buf);
	}
	else
	{
		ss_bg_trace("BG::Exec.2 no scenario (pGlobalMap/pScenario invalid)");
	}

	ResetStack(); // just the way to unregister "scenario" command
	ss_bg_trace("BG::Exec.3 ResetStack ok");
	CPtr<NRPG::CGlobalGame> pGame = NRPG::CreateGlobalGame( nScenarioID );
	sprintf_s(_buf, "BG::Exec.4 CreateGlobalGame ok pGame=%p", (void*)pGame.GetPtr());
	ss_bg_trace(_buf);
	if ( !pGame )
	{
		ss_bg_trace("BG::Exec.4a pGame null, aborting chain");
		return;
	}
	pGame->players = playersSet;
	ss_bg_trace("BG::Exec.5 players assigned");

	pGame->bGlobalMapSet = true;
	pGame->nGlobalMapID = nTemplateID;

	if ( !IsValid( pGlobalMap ) )
	{
		ss_bg_trace("BG::Exec.6 pGlobalMap invalid, skipping zone setup");
		// silent-storm-port r46: still queue CICBeginMission with pZone=0 so
		// CMission::Initialize traces fire — we can then see what bails next.
		vector<string> templParams;
		NMainLoop::Command( new NGame::CICBeginMission( (NScenario::CScenarioZone*)0, -1, templParams, pGame ) );
		return;
	}

	sprintf_s(_buf, "BG::Exec.6 pBaseZone=%p", (void*)pGlobalMap->pBaseZone.GetPtr());
	ss_bg_trace(_buf);
	CPtr<NScenario::CScenarioZone> pZone;
	if ( IsValid( pGame->pScenarioTracker ) && IsValid( pGlobalMap->pBaseZone ) )
	{
		pZone = pGame->pScenarioTracker->GetZoneByDBZone( pGlobalMap->pBaseZone );
		sprintf_s(_buf, "BG::Exec.7 GetZoneByDBZone -> pZone=%p", (void*)pZone.GetPtr());
		ss_bg_trace(_buf);
	}
	else
	{
		ss_bg_trace("BG::Exec.7 skipping zone lookup (tracker or pBaseZone invalid)");
	}

	NMainLoop::CSaveManager *pSaveManager = NMainLoop::GetSaveManager();
	if ( pSaveManager )
		pSaveManager->ClearSlot( NMainLoop::S_SLOT_ACTIVE );
	ss_bg_trace("BG::Exec.8 SaveManager cleared");

	vector<string> templParams;
	NMainLoop::Command( new NGame::CICBeginMission( pZone, -1, templParams, pGame ) );
	ss_bg_trace("BG::Exec.9 CICBeginMission queued");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICContinueGlobal
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
		csSystem << CC_RED << L"ERROR: Can't continue global! No global set!" << endl;
		return;
	}

	CGlobalMap *pRes = new CGlobalMap();
	pRes->Initialize( pGame );
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICShowGlobal
////////////////////////////////////////////////////////////////////////////////////////////////////
CICShowGlobal::CICShowGlobal( NRPG::CGlobalGame *_pGame ):
	pGame( _pGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICShowGlobal::Exec()
{
	if ( !pGame->bGlobalMapSet )
	{
		ASSERT( 0 );
		return;
	}

	CGlobalMap *pRes = new CGlobalMap();
	pRes->Initialize( pGame, IGlobalMap::MODE_SHOW );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandSetDifficulty( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	if ( paramsSet.size() < 1 )
		return;
	//
	CObjectBase *pObject = (CObjectBase *)pContext;
	CDynamicCast<CGlobalMap> pMap((pObject));
	if ( pMap )
		pMap->GetGlobalGame()->ChangeDifficulty( wcstol( paramsSet[ 0 ].c_str(), 0, 10 ) );
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
	NMainLoop::Command( new CICBeginGame( nTemp, playersSet ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(iGlobalMap)
	REGISTER_CMD( "global", CommandStartGlobal )
FINISH_REGISTER
