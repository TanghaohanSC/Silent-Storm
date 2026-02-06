#include "StdAfx.h"
#include "Gfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "..\Misc\StrProc.h"
#include "..\MiscDll\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "RPGUnit.h"
#include "RPGMerc.h"
#include "RPGGlobal.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iFaceGen.h"
#include "iGlobalMap.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
const int
	N_MAX_HEADS = 6,
	N_MAX_VOICES = 3;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFaceGenScroll
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFaceGenScroll: public CScroll
{
	OBJECT_BASIC_METHODS(CFaceGenScroll);
private:
	ZDATA_(CScroll)
	CPtr<CProgressBar> pBar;
	CObj<CFlashButton> pPlus;
	CObj<CFlashButton> pMinus;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CScroll*)this); f.Add(2,&pBar); f.Add(3,&pPlus); f.Add(4,&pMinus); return 0; }

public:
	CFaceGenScroll() {}
	CFaceGenScroll( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CFaceGenScroll::CFaceGenScroll( const SWindowInfo &sInfo ):
	CScroll( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFaceGenScroll::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pPlus = new CFlashButton( sEvent.pLoader->GetControl( "plus" ) );
			pMinus = new CFlashButton( sEvent.pLoader->GetControl( "minus" ) );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pBar = GetUIWindow<CProgressBar>( this, "progress" );
			break;
		}
	}

	return CScroll::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFaceGenScroll::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	SetPageStep( 1 );

	pBar->SetValue( float( GetValue() + 1 ) / ( GetMaxValue() + 1 ) );
	pPlus->SetStyle( STYLE_VISIBLE, GetValue() != GetMaxValue() );
	pMinus->SetStyle( STYLE_VISIBLE, GetValue() != 0 );

	CScroll::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFaceGenUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFaceGenUI: public CWindow
{
	OBJECT_BASIC_METHODS(CFaceGenUI);
private:
	ZDATA_(CWindow)
	int nVoice;
	CObj<NRPG::CUnit> pMerc;
	CDBPtr<NDb::CSide> pSide;
	CDBPtr<NDb::CRPGPers> pPers;
	CDBPtr<NDb::CComplexHead> pHead;
	CDBPtr<NDb::CNationality> pNationality;
	vector<CDBPtr<NDb::CComplexHead> > customHeads;
	////
	bool bChanged;
	////
	CObj<NRPG::CUnit> pTempMerc;
	CObj<CHoverButton> pPlay;
	CObj<CHoverButton> pBack;
	CObj<CUnitView> pUnitView;
	CObj<CFaceGenScroll> pFaceScroll;
	CObj<CFaceGenScroll> pVoiceScroll;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&nVoice); f.Add(3,&pMerc); f.Add(4,&pSide); f.Add(5,&pPers); f.Add(6,&pHead); f.Add(7,&pNationality); f.Add(8,&customHeads); f.Add(9,&bChanged); f.Add(10,&pTempMerc); f.Add(11,&pPlay); f.Add(12,&pBack); f.Add(13,&pUnitView); f.Add(14,&pFaceScroll); f.Add(15,&pVoiceScroll); return 0; }

protected:
	void UpdateUnit();

public:
	CFaceGenUI() {}
	CFaceGenUI( const SWindowInfo &sInfo, NDb::CSide *pSide, NDb::CNationality *pNationality, NDb::CRPGPers *pPers, NDb::CComplexHead *pHead );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CFaceGenUI::CFaceGenUI( const SWindowInfo &sInfo, NDb::CSide *_pSide, NDb::CNationality *_pNationality, NDb::CRPGPers *_pPers, NDb::CComplexHead *_pHead ): 
	CWindow( sInfo ), pSide( _pSide ), pNationality( _pNationality ), pPers( _pPers ), pHead( _pHead ), nVoice( 0 ), bChanged( false )
{
	if ( pPers->bIsFemale )
	{
		customHeads.resize( pNationality->customFemaleHeads.size() );
		for ( int nTemp = 0; nTemp < pNationality->customFemaleHeads.size(); nTemp++ )
			customHeads[nTemp] = pNationality->customFemaleHeads[nTemp];
	}
	else
	{
		customHeads.resize( pNationality->customMaleHeads.size() );
		for ( int nTemp = 0; nTemp < pNationality->customMaleHeads.size(); nTemp++ )
			customHeads[nTemp] = pNationality->customMaleHeads[nTemp];
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFaceGenUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "face" )
			{
				int nValue = pFaceScroll->GetValue();
				ASSERT( nValue < customHeads.size() );
				if ( nValue < customHeads.size() )
					pHead = customHeads[nValue];

				UpdateUnit();
				return true;
			}
			else if ( sEvent.szID == "face" )
			{
				nVoice = pVoiceScroll->GetValue();
				UpdateUnit();
				return true;
			}

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pPlay = new CHoverButton( sEvent.pLoader->GetControl( "play" ) );
			pPlay->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10894 ) + GetDBString( 10896 ) );
			pPlay->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10892 ) + GetDBString( 10896 ) );
			pPlay->AddTextState( CHoverButton::STATE_DISABLED, GetDBString( 10893 ) + GetDBString( 10896 ) );

			pBack = new CHoverButton( sEvent.pLoader->GetControl( "cancel" ) );
			pBack->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10894 ) + GetDBString( 10895 ) );
			pBack->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10892 ) + GetDBString( 10895 ) );
			pBack->AddTextState( CHoverButton::STATE_DISABLED, GetDBString( 10893 ) + GetDBString( 10895 ) );

			pFaceScroll = new CFaceGenScroll( sEvent.pLoader->GetControl( "face" ) );
			pFaceScroll->SetStyle( SCRLSTYLE_HORZ, true );
			pVoiceScroll = new CFaceGenScroll( sEvent.pLoader->GetControl( "voice" ) );
			pVoiceScroll->SetStyle( SCRLSTYLE_HORZ, true );

			pUnitView = new CUnitView( sEvent.pLoader->GetControl( "unitshow" ), 0, 0.5f );
			UpdateUnit();
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pFaceScroll->SetMaxValue( N_MAX_HEADS - 1 );
			pVoiceScroll->SetMaxValue( N_MAX_VOICES - 1 );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFaceGenUI::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFaceGenUI::UpdateUnit()
{
	pTempMerc = NRPG::CreateMerc( pPers, pHead );
	pUnitView->SetUnit( pTempMerc, CUnitView::CAMERA_FACEGEN );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFaceGenMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFaceGenMenuInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CFaceGenMenuInterface);
private:
	NInput::CBind bindClose, bindPlay;

	ZDATA
	CDBPtr<NDb::CSide> pSide;
	CDBPtr<NDb::CRPGPers> pPers;
	CDBPtr<NDb::CComplexHead> pHead;
	////
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	////
	CObj<NUI::CFaceGenUI> pMenuUI;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pSide); f.Add(3,&pPers); f.Add(4,&pHead); f.Add(5,&pCursor); f.Add(6,&pInterface); f.Add(7,&pMenuUI); return 0; }

public:
	CFaceGenMenuInterface();

	void Initialize( NDb::CSide *pSide, NDb::CRPGPers *pPers, NDb::CComplexHead *pHead );

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &eEvent );
	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CFaceGenMenuInterface::CFaceGenMenuInterface():
	bindClose( "cancel" ), bindPlay( "play" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFaceGenMenuInterface::Initialize( NDb::CSide *_pSide, NDb::CRPGPers *_pPers, NDb::CComplexHead *_pHead )
{
	pSide = _pSide;
	pPers = _pPers;
	pHead = _pHead;

	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pMenuUI = new NUI::CFaceGenUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "facegenUI", NUI::STYLE_ENABLED ), pSide, pPers->pNationality, pPers, pHead );
	NUI::LoadTemplate( pMenuUI, NDb::GetUIContainer( 361 ) );
	pMenuUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFaceGenMenuInterface::Step()
{
	MarkNewDGFrame();
	if ( CanRender() )
	{
		pInterface->UpdateCursor();
		pInterface->Step( GetTime() );
		RenderFrame();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFaceGenMenuInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFaceGenMenuInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	NInput::SetSection( "menu" );

	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindClose.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}
	else if ( bindPlay.ProcessEvent( sEvent ) )
	{
		NRPG::CGlobalPlayer *pPlayer = NRPG::CreateGlobalPlayer( pSide );
		pPlayer->mercs.push_back( NRPG::CreateMerc( pPers, pHead, true ) );

		vector<CObj<NRPG::CGlobalPlayer> > playersSet;
		playersSet.push_back( pPlayer );

		NMainLoop::Command( new NGame::CICBeginGame( pSide->nGlobalMapID, playersSet ) ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFaceGenMenuInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( GetTime() );
	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICFaceGen::CICFaceGen( NDb::CSide *_pSide, NDb::CRPGPers *_pPers, NDb::CComplexHead *_pHead ):
	pSide( _pSide ), pPers( _pPers ), pHead( _pHead )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICFaceGen::Exec()
{
	ASSERT( IsValid( pPers->pNationality ) );
	if ( !IsValid( pPers->pNationality ) )
	{
		csSystem << CC_RED << L"ERROR: Pers nationality not set!" << endl;
		return;
	}

	CFaceGenMenuInterface *pRes = new CFaceGenMenuInterface();
	pRes->Initialize( pSide, pPers, pHead );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB1204020, CFaceGenUI );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB1204021, CFaceGenMenuInterface );
