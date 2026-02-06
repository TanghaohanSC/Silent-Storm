#include "StdAfx.h"
#include "Gfx.h"
#include "iMain.h"
#include "GView.h"
#include "G2DView.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "RPGMerc.h"
#include "RPGUnit.h"
#include "RPGGlobal.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iTeamMngMenu.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_MAX_UNITS = 20;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitPortraitView
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitPortraitView: public CUnitView
{
	OBJECT_NOCOPY_METHODS(CUnitPortraitView)
private:
	ZDATA_(CUnitView)
	CObj<CImage> pImage;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUnitView*)this); f.Add(2,&pImage); return 0; }

public:
	CUnitPortraitView() {}
	CUnitPortraitView( const SWindowInfo &sInfo, NRPG::CUnit *pUnit );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitPortraitView::CUnitPortraitView( const SWindowInfo &sInfo, NRPG::CUnit *pUnit ):
	CUnitView( sInfo, 0 )
{
	pImage = new CImage( SWindowInfo( this, SPoint( 0, 0 ), GetSize(), "", STYLE_ENABLED | STYLE_VISIBLE | STYLE_BOTTOMMOST ) );
	pImage->SetImage( NDb::GetUITexture( 535 ) );

	SetUnit( pUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitPortraitView::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	SRect sScrWindow;
	SPoint sScrPosition;
	if ( !ClientToScreen( &sScrPosition, &sScrWindow ) )
		return;

	VirtualToScreen( &sScrPosition, &sScrWindow );

	SRect sDummyRect;
	SPoint sSize( GetSize() );
	VirtualToScreen( &sSize, &sDummyRect );

	CRectLayout sLayout;
	sLayout.AddRect( 0, 0, CTRect<float>( 0, 0, sSize.x, sSize.y ) );

	pView->CreateDynamicClearRects( sLayout, sScrPosition, sScrWindow, 1 );
	CUnitView::Draw( sTime, pView );
	pView->CreateDynamicClearRects( sLayout, sScrPosition, sScrWindow, 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitPortraitState
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitPortraitState: public CWindow
{
	OBJECT_NOCOPY_METHODS(CUnitPortraitState)
private:
	ZDATA_(CWindow)
	CPtr<NRPG::CUnit> pUnit;
	////
	CObj<CImage> pState;
	CObj<CImage> pSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pUnit); f.Add(3,&pState); f.Add(4,&pSelection); return 0; }

public:
	CUnitPortraitState() {}
	CUnitPortraitState( const SWindowInfo &sInfo, NRPG::CGlobalPlayer *pPlayer, NRPG::CUnit *pUnit );

	void SetSelected( bool bState );

	NRPG::CUnit* GetUnit() const;

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitPortraitState::CUnitPortraitState( const SWindowInfo &sInfo, NRPG::CGlobalPlayer *pPlayer, NRPG::CUnit *_pUnit ):
	CWindow( sInfo ), pUnit( _pUnit )
{
	pState = new CImage( SWindowInfo( this, SPoint( 0, 0 ), GetSize(), "", STYLE_ENABLED ) );
	pSelection = new CImage( SWindowInfo( this, SPoint( 0, 0 ), GetSize(), "", STYLE_ENABLED | STYLE_TOPMOST ) );
	pSelection->SetImage( NDb::GetUITexture( 537 ) );
	if ( IsValid( pUnit ) )
	{
		if ( pUnit->IsDead() )
		{
			pState->SetImage( NDb::GetUITexture( 538 ) );
			pState->SetStyle( STYLE_VISIBLE, true );
		}
		else
		{
			for ( int nTemp = 0; nTemp < pPlayer->mercs.size(); nTemp++ )
			{
				if ( !IsValid( pPlayer->mercs[nTemp] ) )
					continue;

				if ( pPlayer->mercs[nTemp]->pRPGUnit == pUnit.GetPtr() )
				{
					pState->SetImage( NDb::GetUITexture( 536 ) );
					pState->SetStyle( STYLE_VISIBLE, true );
					break;
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitPortraitState::SetSelected( bool bState )
{
	pSelection->SetStyle( STYLE_VISIBLE, bState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CUnit* CUnitPortraitState::GetUnit() const
{
	return pUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitPortraitState::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONDBLCLK:
		{
			SendMessage( GetParent(), SEvent( EVENT_NOTIFY, GetWindowID() ) );
			return true;
		}
	case EVENT_LBUTTONUP:
		return true;
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitCharacterPanel
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitCharacterPanel: public CWindow
{
	OBJECT_NOCOPY_METHODS(CUnitCharacterPanel)
private:
	ZDATA_(CWindow)
	CPtr<NRPG::CUnit> pUnit;
	////
	CPtr<CText> pLevel;
	CPtr<CImage> pClass;
	//// Primary
	CPtr<CText> pEvasion;
	CPtr<CText> pStrength;
	CPtr<CText> pDexterity;
	CPtr<CText> pIntelligence;
	CPtr<CText> pActionPoints;
	CPtr<CText> pVitalityPoints;
	CPtr<CProgressBar> pEvasionBar;
	CPtr<CProgressBar> pStrengthBar;
	CPtr<CProgressBar> pDexterityBar;
	CPtr<CProgressBar> pIntelligenceBar;
	CPtr<CProgressBar> pActionPointsBar;
	CPtr<CProgressBar> pVitalityPointsBar;
	//// Secondary
	CPtr<CText> pHide;
	CPtr<CText> pSpot;
	CPtr<CText> pBurst;
	CPtr<CText> pMelee;
	CPtr<CText> pSnipe;
	CPtr<CText> pMedicine;
	CPtr<CText> pShooting;
	CPtr<CText> pThrowing;
	CPtr<CText> pInterrupt;
	CPtr<CText> pEngineering;
	CPtr<CProgressBar> pHideBar;
	CPtr<CProgressBar> pSpotBar;
	CPtr<CProgressBar> pBurstBar;
	CPtr<CProgressBar> pMeleeBar;
	CPtr<CProgressBar> pSnipeBar;
	CPtr<CProgressBar> pMedicineBar;
	CPtr<CProgressBar> pShootingBar;
	CPtr<CProgressBar> pThrowingBar;
	CPtr<CProgressBar> pInterruptBar;
	CPtr<CProgressBar> pEngineeringBar;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pUnit); f.Add(3,&pLevel); f.Add(4,&pClass); f.Add(5,&pEvasion); f.Add(6,&pStrength); f.Add(7,&pDexterity); f.Add(8,&pIntelligence); f.Add(9,&pActionPoints); f.Add(10,&pVitalityPoints); f.Add(11,&pEvasionBar); f.Add(12,&pStrengthBar); f.Add(13,&pDexterityBar); f.Add(14,&pIntelligenceBar); f.Add(15,&pActionPointsBar); f.Add(16,&pVitalityPointsBar); f.Add(17,&pHide); f.Add(18,&pSpot); f.Add(19,&pBurst); f.Add(20,&pMelee); f.Add(21,&pSnipe); f.Add(22,&pMedicine); f.Add(23,&pShooting); f.Add(24,&pThrowing); f.Add(25,&pInterrupt); f.Add(26,&pEngineering); f.Add(27,&pHideBar); f.Add(28,&pSpotBar); f.Add(29,&pBurstBar); f.Add(30,&pMeleeBar); f.Add(31,&pSnipeBar); f.Add(32,&pMedicineBar); f.Add(33,&pShootingBar); f.Add(34,&pThrowingBar); f.Add(35,&pInterruptBar); f.Add(36,&pEngineeringBar); return 0; }

public:
	CUnitCharacterPanel() {}
	CUnitCharacterPanel( const SWindowInfo &sInfo, NRPG::CUnit *_pUnit );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitCharacterPanel::CUnitCharacterPanel( const SWindowInfo &sInfo, NRPG::CUnit *_pUnit ):
	CWindow( sInfo ), pUnit( _pUnit )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitCharacterPanel::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pLevel = GetUIWindow<CText>( this, "level" );
			pClass = GetUIWindow<CImage>( this, "class" );

			pEvasion = GetUIWindow<CText>( this, "evasion" );
			pStrength = GetUIWindow<CText>( this, "strength" );
			pDexterity = GetUIWindow<CText>( this, "dexterity" );
			pIntelligence = GetUIWindow<CText>( this, "intelligence" );
			pActionPoints = GetUIWindow<CText>( this, "ap" );
			pVitalityPoints = GetUIWindow<CText>( this, "vp" );
			pEvasionBar = GetUIWindow<CProgressBar>( this, "evasion_bar" );
			pStrengthBar = GetUIWindow<CProgressBar>( this, "strength_bar" );
			pDexterityBar = GetUIWindow<CProgressBar>( this, "dexterity_bar" );
			pIntelligenceBar = GetUIWindow<CProgressBar>( this, "intelligence_bar" );
			pActionPointsBar = GetUIWindow<CProgressBar>( this, "ap_bar" );
			pVitalityPointsBar = GetUIWindow<CProgressBar>( this, "vp_bar" );

			pHide = GetUIWindow<CText>( this, "hide" );
			pSpot = GetUIWindow<CText>( this, "spot" );
			pBurst = GetUIWindow<CText>( this, "burst" );
			pMelee = GetUIWindow<CText>( this, "melee" );
			pSnipe = GetUIWindow<CText>( this, "snipe" );
			pMedicine = GetUIWindow<CText>( this, "medicine" );
			pShooting = GetUIWindow<CText>( this, "shooting" );
			pThrowing = GetUIWindow<CText>( this, "throwing" );
			pInterrupt = GetUIWindow<CText>( this, "interrupt" );
			pEngineering = GetUIWindow<CText>( this, "engineering" );
			pHideBar = GetUIWindow<CProgressBar>( this, "hide_bar" );
			pSpotBar = GetUIWindow<CProgressBar>( this, "spot_bar" );
			pBurstBar = GetUIWindow<CProgressBar>( this, "burst_bar" );
			pMeleeBar = GetUIWindow<CProgressBar>( this, "melee_bar" );
			pSnipeBar = GetUIWindow<CProgressBar>( this, "snipe_bar" );
			pMedicineBar = GetUIWindow<CProgressBar>( this, "medicine_bar" );
			pShootingBar = GetUIWindow<CProgressBar>( this, "shooting_bar" );
			pThrowingBar = GetUIWindow<CProgressBar>( this, "throwing_bar" );
			pInterruptBar = GetUIWindow<CProgressBar>( this, "interrupt_bar" );
			pEngineeringBar = GetUIWindow<CProgressBar>( this, "engineering_bar" );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitCharacterPanel::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !IsValid( pUnit ) )
		return;

	WCHAR wsText[256];

	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_LEVEL ) );
	pLevel->SetText( wsText );
	if ( pUnit->pClass )
		pClass->SetImage( pUnit->pClass->pIcon );

	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_STR ) );
	pStrength->SetText( wsText );
	pStrengthBar->SetValue( pUnit->Skills( NDb::ST_STR ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_DEX ) );
	pDexterity->SetText( wsText );
	pDexterityBar->SetValue( pUnit->Skills( NDb::ST_DEX ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_INT ) );
	pIntelligence->SetText( wsText );
	pIntelligenceBar->SetValue( pUnit->Skills( NDb::ST_INT ).GetProgress() );

	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_IC ) );
	pEvasion->SetText( wsText );
	pEvasionBar->SetValue( pUnit->Skills( NDb::ST_IC ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_AP ) );
	pActionPoints->SetText( wsText );
	pActionPointsBar->SetValue( pUnit->Skills( NDb::ST_AP ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_VP ) );
	pVitalityPoints->SetText( wsText );
	pVitalityPointsBar->SetValue( pUnit->Skills( NDb::ST_VP ).GetProgress() );

	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_STEALTH ) );
	pHide->SetText( wsText );
	pHideBar->SetValue( pUnit->Skills( NDb::ST_STEALTH ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_SPOT ) );
	pSpot->SetText( wsText );
	pSpotBar->SetValue( pUnit->Skills( NDb::ST_SPOT ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_BURST ) );
	pBurst->SetText( wsText );
	pBurstBar->SetValue( pUnit->Skills( NDb::ST_BURST ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_MELEE ) );
	pMelee->SetText( wsText );
	pMeleeBar->SetValue( pUnit->Skills( NDb::ST_MELEE ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_SNIPE ) );
	pSnipe->SetText( wsText );
	pSnipeBar->SetValue( pUnit->Skills( NDb::ST_SNIPE ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_MEDICINE ) );
	pMedicine->SetText( wsText );
	pMedicineBar->SetValue( pUnit->Skills( NDb::ST_MEDICINE ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_SHOOTING ) );
	pShooting->SetText( wsText );
	pShootingBar->SetValue( pUnit->Skills( NDb::ST_SHOOTING ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_THROWING ) );
	pThrowing->SetText( wsText );
	pThrowingBar->SetValue( pUnit->Skills( NDb::ST_THROWING ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_INTERRUPT ) );
	pInterrupt->SetText( wsText );
	pInterruptBar->SetValue( pUnit->Skills( NDb::ST_INTERRUPT ).GetProgress() );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_ENGINEERING ) );
	pEngineering->SetText( wsText );
	pEngineeringBar->SetValue( pUnit->Skills( NDb::ST_ENGINEERING ).GetProgress() );

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTeamMngUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTeamMngUI: public CWindow
{
	OBJECT_NOCOPY_METHODS(CTeamMngUI)
public:
	enum EPanel
	{
		PANEL_DEFAULT,
		PANEL_NONE,
		PANEL_CHARACTER,
		PANEL_INVENTORY
	};

private:
	ZDATA_(CWindow)
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer;
	////
	EPanel ePanel;
	CPtr<NRPG::CUnit> pUnit;
	////
	CObj<CWindow> pPanel;
	CPtr<CWindow> pPanelBase;
	////
	CObj<CFlashButton> pCloseButton;
	CPtr<CUnitPortraitView> pSelected;
	vector<CObj<CUnitPortraitView> > unitsViewSet;
	vector<CObj<CUnitPortraitState> > unitsStateSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pGlobalPlayer); f.Add(3,&ePanel); f.Add(4,&pUnit); f.Add(5,&pPanel); f.Add(6,&pPanelBase); f.Add(7,&pCloseButton); f.Add(8,&pSelected); f.Add(9,&unitsViewSet); f.Add(10,&unitsStateSet); return 0; }

public:
	CTeamMngUI() {}
	CTeamMngUI( const SWindowInfo &sInfo, NRPG::CGlobalPlayer *_pGlobalPlayer );

	void Set( NRPG::CUnit *pUnit, EPanel ePanel = PANEL_DEFAULT );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CTeamMngUI::CTeamMngUI( const SWindowInfo &sInfo,  NRPG::CGlobalPlayer *_pGlobalPlayer ):
	CWindow( sInfo ), pGlobalPlayer( _pGlobalPlayer ), unitsViewSet( N_MAX_UNITS ), unitsStateSet( N_MAX_UNITS ), ePanel( PANEL_NONE )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTeamMngUI::Set( NRPG::CUnit *_pUnit, EPanel _ePanel )
{
	if ( IsValid( _pUnit ) )
		pUnit = _pUnit;
	if ( _ePanel != PANEL_DEFAULT )
		ePanel = _ePanel;

	pPanel = 0;
	switch( ePanel )
	{
	case PANEL_NONE:
	case PANEL_CHARACTER:
		{
			pPanel = new CUnitCharacterPanel( SWindowInfo( pPanelBase, SPoint( 0, 0 ), pPanelBase->GetSize(), "", STYLE_ENABLED | STYLE_VISIBLE ), pUnit );
			LoadTemplate( pPanel, NDb::GetUIContainer( 318 ) );
			break;
		}
	case PANEL_INVENTORY:
		{
			pPanel = new CWindow( SWindowInfo( pPanelBase, SPoint( 0, 0 ), pPanelBase->GetSize(), "", STYLE_ENABLED | STYLE_VISIBLE ) );
			LoadTemplate( pPanel, NDb::GetUIContainer( 319 ) );
			break;
		}
	}

	for( int nTemp = 0; nTemp < unitsStateSet.size(); nTemp++ )
	{
		CPtr<CUnitPortraitState> pState = unitsStateSet[nTemp];

		if( pState->GetUnit() == pUnit )
			pState->SetSelected( true );
		else
			pState->SetSelected( false );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTeamMngUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			for( int nTemp = 0; nTemp < unitsStateSet.size(); nTemp++ )
			{
				CPtr<CUnitPortraitState> pState = unitsStateSet[nTemp];

				if ( pState->GetWindowID() != sEvent.szID )
					continue;

				Set( pState->GetUnit() );
				return true;
			}
			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pCloseButton = new CFlashButton( sEvent.pLoader->GetControl( "cancel" ) );

			for ( int nTemp = 0; nTemp < N_MAX_UNITS; nTemp++ )
			{
				CPtr<NRPG::CUnit> pUnit;
				if ( ( nTemp < pGlobalPlayer->totalMercs.size() ) && IsValid( pGlobalPlayer->totalMercs[nTemp] ) )
					pUnit = pGlobalPlayer->totalMercs[nTemp]->pRPGUnit;

				unitsViewSet[nTemp] = new CUnitPortraitView( sEvent.pLoader->GetControl( NStr::Format( "unit_view_%d", nTemp ) ), pUnit );
				unitsStateSet[nTemp] = new CUnitPortraitState( sEvent.pLoader->GetControl( NStr::Format( "unit_%d", nTemp ) ), pGlobalPlayer, pUnit );
			}

			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pPanelBase = GetUIWindow<CWindow>( this, "panel" );

			for ( int nTemp = 0; nTemp < pGlobalPlayer->mercs.size(); nTemp++ )
			{
				CPtr<NRPG::CUnit> pUnit = pGlobalPlayer->mercs[nTemp]->pRPGUnit;

				if ( pUnit->IsDead() )
					continue;

				Set( pUnit, PANEL_CHARACTER );
				break;
			}
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTeamMngUI::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	SRect sScrWindow;
	SPoint sScrPosition;
	if ( !ClientToScreen( &sScrPosition, &sScrWindow ) )
		return;

	VirtualToScreen( &sScrPosition, &sScrWindow );

	SRect sDummyRect;
	SPoint sSize( GetSize() );
	VirtualToScreen( &sSize, &sDummyRect );

	CRectLayout sLayout;
	sLayout.AddRect( 0, 0, CTRect<float>( 0, 0, sSize.x, sSize.y ) );

	pView->CreateDynamicClearRects( sLayout, sScrPosition, sScrWindow, 0 );

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInGameMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTeamMngMenuInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CTeamMngMenuInterface);
private:
	NInput::CBind bindExit;
	NInput::CBind bindCharacter, bindInventory;

	ZDATA
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer;
	////
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	////
	CObj<NUI::CTeamMngUI> pMenuUI;
	CObj<NUI::CScreenShot> pScreenShot;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pGlobalPlayer); f.Add(3,&pCursor); f.Add(4,&pInterface); f.Add(5,&pMenuUI); f.Add(6,&pScreenShot); return 0; }

public:
	CTeamMngMenuInterface();

	void Initialize( NRPG::CGlobalPlayer *pGlobalPlayer );

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CTeamMngMenuInterface::CTeamMngMenuInterface():
	bindExit( "cancel" ), bindCharacter( "character" ), bindInventory( "inventory" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTeamMngMenuInterface::Initialize( NRPG::CGlobalPlayer *_pGlobalPlayer )
{
	pGlobalPlayer = _pGlobalPlayer;

	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pScreenShot = new NUI::CScreenShot( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "clues", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_BOTTOMMOST ) );
	pScreenShot->Generate();

	pMenuUI = new NUI::CTeamMngUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "ingamemenu", NUI::STYLE_ENABLED ), pGlobalPlayer );
	NUI::LoadTemplate( pMenuUI, NDb::GetUIContainer( 316 ) );
	pMenuUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTeamMngMenuInterface::Step()
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
void CTeamMngMenuInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTeamMngMenuInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}

	if ( bindCharacter.ProcessEvent( sEvent ) )
	{
		pMenuUI->Set( 0, NUI::CTeamMngUI::PANEL_CHARACTER );
		return true;
	}
	else if ( bindInventory.ProcessEvent( sEvent ) )
	{
		pMenuUI->Set( 0, NUI::CTeamMngUI::PANEL_INVENTORY );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTeamMngMenuInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( GetTime() );
	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICTeamMngMenu::CICTeamMngMenu( NRPG::CGlobalPlayer *_pGlobalPlayer ):
	pGlobalPlayer( _pGlobalPlayer )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICTeamMngMenu::Exec()
{
	CTeamMngMenuInterface *pRes = new CTeamMngMenuInterface();
	pRes->Initialize( pGlobalPlayer );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB0925140, CTeamMngMenuInterface );
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0925141, CTeamMngUI );
REGISTER_SAVELOAD_CLASS( 0xB0925142, CUnitCharacterPanel );
REGISTER_SAVELOAD_CLASS( 0xB0925143, CUnitPortraitView );
REGISTER_SAVELOAD_CLASS( 0xB0925144, CUnitPortraitState );
