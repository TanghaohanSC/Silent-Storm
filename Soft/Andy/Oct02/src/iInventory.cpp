#include "StdAfx.h"
#include "GView.h"
#include "G2DView.h"
#include "wInterface.h"
#include "Transform.h"
#include "DiscretePos.h"
#include "GSceneUtils.h"
#include "RPGItemInfo.h"
#include "RPGUnitInfo.h"
#include "RWGame.h"
#include "..\Misc\StrProc.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "iMain.h"
#include "IGlobalGame.h"
#include "Interface.h"
#include "IDecorators.h"
#include "iCommonUI.h"
#include "iInventory.h"
#include "iInventoryIC.h"
#include "ScreenShot.h"
/*
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_BACKPACK_WIDTH	= 12,
	N_BACKPACK_HEIGHT = 8,
	N_GROUND_WIDTH	= 32,
	N_GROUND_HEIGHT = 32;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSingleSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
CSingleSlot::CSingleSlot( IWindow *pContainer, NGame::IGlobalGame *_pGame, NWorld::CUnit *_pUnit, NDb::ESlot _eType ):
	CSlot( pContainer, _pGame, 1, 1 ), pGame( _pGame ), pUnit( _pUnit ), eType( _eType )
{
	pInfo = pUnit->GetRPG()->GetInventoryInfo();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSingleSlot::Place( int nX, int nY, NRPG::IInventoryItem *pItem )
{
	NWorld::SItem sTarget;
	sTarget.eType = NWorld::SItem::SLOT;
	sTarget.nSlot = eType;
	sTarget.pUnit = pUnit;

	pGame->Command( pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::CCmdMoveInventoryItem::UNIT_OR_GROUND, sTarget ) );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSingleSlot::Activate( int nX, int nY, NRPG::IInventoryItem *pItem )
{
	//// CRAP

	if ( !IsValid( pItem ) )
	{
		CPtr<NWorld::CPlayerItem> pPlayerItem = pGame->GetActivePlayer()->GetPlayer()->GetInHandItem();
		if ( ( pPlayerItem->GetSource().eType == NWorld::SItem::SLOT ) && ( pPlayerItem->GetSource().nSlot == eType ) )
		{
			pGame->Command( pUnit, new NWorld::CCmdSetActiveItem( eType ) );
			return true;
		}
	}
	else
	{
		pGame->Command( pUnit, new NWorld::CCmdSetActiveItem( eType ) );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSingleSlot::GetItemsList( vector<SItem> *pItemsSet )
{
	CPtr<NRPG::IInventoryItem> pItem = pInfo->Get( eType );
	if ( IsValid( pItem ) )
	{
		SItem &sItem = *pItemsSet->insert( pItemsSet->end() );
		sItem.bActive = false;
		sItem.sPos = CTPoint<int>( 0, 0 );
		sItem.nSlot = eType;
		sItem.pItem = pItem;
		sItem.pUnit = pUnit;
		if ( pInfo->GetActiveSlot() == eType )
			sItem.bActive = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSingleSlot::CreateDragAndDrop( const SItem &sItem )
{
	CPtr<NRPG::IInventoryItem> pItem = pInfo->Get( eType );
	if ( !IsValid( pItem ) )
		return;

	NWorld::SItem sSource;
	sSource.eType = NWorld::SItem::SLOT;
	sSource.nSlot = eType;
	sSource.pItem = sItem.pItem;
	sSource.pUnit = sItem.pUnit;
	pGame->Command( pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::CCmdMoveInventoryItem::PLAYERHAND, sSource ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CBackPackSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
CBackPackSlot::CBackPackSlot( IWindow *pContainer, NGame::IGlobalGame *_pGame, NWorld::CUnit *_pUnit ):
	CSlot( pContainer, _pGame, N_BACKPACK_WIDTH, N_BACKPACK_HEIGHT ), pGame( _pGame ), pUnit( _pUnit )
{
	pInfo = pUnit->GetRPG()->GetInventoryInfo();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBackPackSlot::Place( int nX, int nY, NRPG::IInventoryItem *pItem )
{
	SPoint sPos;
	GetInSlotPos( nX, nY, &sPos );

	const CTPoint<int> &sSize = pItem->GetSize();
	sPos.x -= sSize.x / 2;
	sPos.y -= sSize.y / 2;

	NWorld::SItem sTarget;
	sTarget.eType = NWorld::SItem::BACKPACK;
	sTarget.pUnit = pUnit;
	sTarget.sPosition = sPos;

	pGame->Command( pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::CCmdMoveInventoryItem::UNIT_OR_GROUND, sTarget ) );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBackPackSlot::Activate( int nX, int nY, NRPG::IInventoryItem *pItem )
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CBackPackSlot::GetItemsList( vector<SItem> *pItemsSet )
{
	const vector<NRPG::SBackPackItem> &itemsSet = pInfo->GetItems();
	pItemsSet->resize( itemsSet.size() );
	for ( int nTemp = 0; nTemp < itemsSet.size(); nTemp++ )
	{
		SItem &sItem = (*pItemsSet)[nTemp];
		sItem.bActive = false;
		sItem.sPos = itemsSet[nTemp].sPos;
		sItem.pItem = itemsSet[nTemp].pItem;
		sItem.pUnit = pUnit;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CBackPackSlot::CreateDragAndDrop( const SItem &sItem )
{
	NWorld::SItem sInvItem;
	sInvItem.eType = NWorld::SItem::BACKPACK;
	sInvItem.pItem = sItem.pItem;
	sInvItem.pUnit = pUnit;
	sInvItem.sPosition = sItem.sPos;
	pGame->Command( pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::CCmdMoveInventoryItem::PLAYERHAND, sInvItem ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitShow
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitModelShow::CUnitModelShow( IWindow *pContainer, NGame::IGlobalGame *_pGame, NWorld::CUnit *pUnit ):
	TBaseClass( pContainer ), pGame( _pGame ), fAngle( 0.0f )
{
	p3DView = NGScene::CreateNewView();
	p3DView->SetFogMode( NGScene::FOG_NONE );//7
	p3DView->SetAmbient( NDb::GetAmbientLight(7), NGScene::IGameView::LT_INVENTORY );
	pInventoryUnit = NRender::CreateShowInventoryUnit( p3DView, pUnit, sTimer.GetTime(), pGame->GetRenderGame() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitModelShow::SetAngle( float _fAngle )
{
	fAngle = _fAngle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitModelShow::ProcessMessage( const SEvent &sEvent )
{
	switch ( sEvent.nEvent )
	{
		case EVENT_MOUSEUPDATECURSOR:
		{
			GetInterface()->SetCursor( SCursorInfo() );
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitModelShow::Draw( const SScene &sScene )
{
	sScene.pView->Flush();
	sTimer.Advance( true, GetTickCount() );
	pInventoryUnit->Update( fAngle );

	SRect sWindow;
	SPoint sPosition;
	if ( !GetThis()->ClientToScreen( &sPosition, &sWindow ) )
		return;

	CVec2 vPos( ( sWindow.x2 + sWindow.x1 ) / 2, ( sWindow.y2 + sWindow.y1 ) / 2 + 190 );
	SHMatrix sMatrix;
	CTransformStack ts;
	MakeMatrix( &sMatrix, ToRadian( 0 ), ToRadian( 90.0f ), CVec3( 10, (float)( 512 - vPos.x ) * 5 / 1024, (float)( vPos.y - 384 ) * 3.75f / 768 ) );
	ts.Init();
	ts.MakeParallel( 5, 3.75f, 0, 20 );
	ts.SetCamera( sMatrix );
	//ts.PushScale( 2 );
	//NGScene::Clear( CVec3(0.5f, 0.5f, 0.5f ) );
	p3DView->DrawOver( &ts );

	TBaseClass::Draw( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInventoryUnitView
////////////////////////////////////////////////////////////////////////////////////////////////////
CInventoryUnitView::CInventoryUnitView( IWindow *pContainer, NGame::IGlobalGame *_pGame, NWorld::CUnit *_pUnit ):
	TBaseClass( pContainer ), pGame( _pGame ), pUnit( _pUnit ), bButtonDown( false ), fAngle( 0.0f )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventoryUnitView::ProcessMessage( const SEvent &sEvent )
{
	if ( TBaseClass::ProcessMessage( sEvent ) )
		return true;

	struct SSlot
	{
		NDb::ESlot eType;
		const char *sName;
	};
	SSlot sSlots[] =
	{
		{ NDb::SLOT_BODY_L1, "body_l1" },
		{ NDb::SLOT_BODY_R1, "body_r1" },
		{ NDb::SLOT_BODY_L2, "body_l2" },
		{ NDb::SLOT_BODY_R2, "body_r2" },
		{ NDb::SLOT_LEG_L1, "leg_l1" },
		{ NDb::SLOT_LEG_R1, "leg_r1" },
		{ NDb::SLOT_LEG_L2, "leg_l2" },
		{ NDb::SLOT_LEG_R2, "leg_r2" },
		{ NDb::SLOT_LEG_L3, "leg_l3" },
		{ NDb::SLOT_LEG_R3, "leg_r3" },
		{ NDb::SLOT_BELT_L1, "belt_l1" },
		{ NDb::SLOT_BELT_R1, "belt_r1" },
		{ NDb::SLOT_BELT_M1, "belt_m1" },
		{ NDb::SLOT_BELT_MEDIUM_L1, "belt_medium_l1" },
		{ NDb::SLOT_BELT_MEDIUM_R1, "belt_medium_r1" },
		{ NDb::SLOT_BELT_MEDIUM_L2, "belt_medium_l2" },
		{ NDb::SLOT_BELT_MEDIUM_R2, "belt_medium_r2" },
		{ NDb::SLOT_WAISTBELT_L1, "waistbelt_l1" },
		{ NDb::SLOT_WAISTBELT_R1, "waistbelt_r1" }
	};

	switch ( sEvent.nEvent )
	{
		case EVENT_LBUTTONUP:
		{
			bButtonDown = false;
			return true;
		}
		case EVENT_LBUTTONDOWN:
		{
			bButtonDown = true;
			sLastPoint.x = sEvent.nX;
			sLastPoint.y = sEvent.nY;
			return true;
		}
		case EVENT_MOUSEMOVE:
		{
			if ( !bButtonDown )
				break;

			fAngle += float( sEvent.nX - sLastPoint.x ) / 10;
			sLastPoint.x = sEvent.nX;
			sLastPoint.y = sEvent.nY;
			break;
		}
		case EVENT_TEMPLATELOAD:
		{
			pUnitShow = new CUnitModelShow( GetUIWindow<IWindow>( this, "view", sEvent.pLoader ), pGame, pUnit );
			pUnitShow->SetStyle( STYLE_TRANSPARENT, true );

			if ( CDynamicCast<NRPG::IUniformItem> pUniform( pUnit->GetRPG()->GetInventoryInfo()->GetUniform() ) )
			{
				for ( int nTemp = 0; nTemp < ARRAY_SIZE(sSlots); nTemp++ )
				{
					CPtr<IWindow> pWindow = GetUIWindow<IWindow>( this, sSlots[nTemp].sName, sEvent.pLoader );

					if ( !pUniform->GetDBUniform()->bSlots[sSlots[nTemp].eType] )
					{
						pWindow->SetStyle( STYLE_VISIBLE, false );
						continue;
					}

					CPtr<CSingleSlot> pSlot = new CSingleSlot( pWindow, pGame, pUnit, sSlots[nTemp].eType );
					containersSet.push_back( &(*pSlot) );
				}
			}
			break;
		}
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryUnitView::Update( const SScene &sScene )
{
	pUnitShow->SetAngle( fAngle );
	TBaseClass::Update( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInventoryUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
CInventoryUnit::CInventoryUnit( IWindow *pContainer, NGame::IGlobalGame *_pGame, NWorld::CUnit *_pUnit ):
	TBaseClass( pContainer ), pGame( _pGame ), pUnit( _pUnit )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventoryUnit::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOAD:
		{
			pBackground = new CBackground( GetUIWindow<IWindow>( this, "background", sEvent.pLoader ), F_BASE_TRANSPARENCY );

			pWeapon = new CSingleSlot( GetUIWindow<IWindow>( this, "weapon", sEvent.pLoader ), pGame, pUnit, NDb::SLOT_WEAPON );
			pBackPack = new CBackPackSlot( GetUIWindow<IWindow>( this, "backpack", sEvent.pLoader ), pGame, pUnit );
			pUnitView = new CInventoryUnitView( GetUIWindow<IWindow>( this, "pers", sEvent.pLoader ), pGame, pUnit );

			break;
		}
	}

	if ( TBaseClass::ProcessMessage( sEvent ) )
		return true;

	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONUP:
		return true;
	case EVENT_LBUTTONDOWN:
		return true;
	case EVENT_LBUTTONDBLCLK:
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInventoryUI
////////////////////////////////////////////////////////////////////////////////////////////////////
CInventoryUI::CInventoryUI( IWindow *pContainer, NGame::IGlobalGame *_pGame, NWorld::CUnit *_pUnit ):
	CSlot( pContainer, _pGame, N_GROUND_WIDTH, N_GROUND_HEIGHT ), pGame( _pGame ), pUnit( _pUnit )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventoryUI::Place( int nX, int nY, NRPG::IInventoryItem *pItem )
{
	NWorld::SItem sTarget;
	sTarget.eType = NWorld::SItem::GROUND;
	pGame->Command( pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::CCmdMoveInventoryItem::UNIT_OR_GROUND, sTarget ) );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventoryUI::Activate( int nX, int nY, NRPG::IInventoryItem *pItem )
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryUI::GetItemsList( vector<SItem> *pItemsSet )
{
	pItemsSet->clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryUI::CreateDragAndDrop( const SItem &sItem )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventoryUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOAD:
		{
//			pUnitPanel = new CUnitPanel( GetUIWindow<IWindow>( this, "unitpanel", sEvent.pLoader ), pGame );
			pControlPanel = new CControlPanel( GetUIWindow<IWindow>( this, "controlpanel", sEvent.pLoader ), pGame, NGame::DIALOG );
			pInventoryUnit = new CInventoryUnit( GetUIWindow<IWindow>( this, "inventoryunit", sEvent.pLoader ), pGame, pUnit );
			break;
		}
		case EVENT_TEMPLATELOADCOMPLETE:
		{
			wstring szMenu( L"<font size=18pt face=Courier><center>Menu" );
			wstring szCancel( L"<font size=18pt face=Courier><center>Cancel" );

			CPtr<IWindow> pWindow;
			pWindow = GetUIWindow<IWindow>( pControlPanel, "slot2", sEvent.pLoader );
			pCPMenu = new CControlPanelButton( pWindow, szMenu, "menu" );
			pWindow = GetUIWindow<IWindow>( pControlPanel, "slot1", sEvent.pLoader );
			pCPCancel = new CControlPanelButton( pWindow, szCancel, "cancel" );
			break;
		}
	}

	return CSlot::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInventoryDesktop
////////////////////////////////////////////////////////////////////////////////////////////////////
CInventoryDesktop::CInventoryDesktop( IWindow *pContainer, NGame::IGlobalGame *_pGame, NWorld::CUnit *_pUnit, CPtrFuncBase<NGfx::CTexture> *_pBackgroundTexture ):
	NGame::CDesktop( pContainer ), pGame( _pGame ), pUnit( _pUnit ), pBackgroundTexture( _pBackgroundTexture )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventoryDesktop::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOAD:
		{
			pInventoryUI = new CInventoryUI( GetUIWindow<IWindow>( this, "view", sEvent.pLoader ), pGame, pUnit );
			break;
		}
	}

	return NGame::CDesktop::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryDesktop::Draw( const SScene &sScene )
{
	DrawBackground( sScene );
	NGame::CDesktop::Draw( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryDesktop::DrawBackground( const SScene &sScene )
{
	SRect sScrWindow;
	SPoint sScrPosition;
	if ( !ClientToScreen( &sScrPosition, &sScrWindow ) )
		return;
	VirtualToScreen( sScene, &sScrPosition, &sScrWindow );

	CRectLayout sLayout;
	sLayout.scale = CTPoint<float>( sScene.pView->GetViewportSize().x / 512.0f, sScene.pView->GetViewportSize().y / 512.0f );
	sLayout.AddRect( 0, 0, CTRect<float>( 0, 0, 512, 512 ) );
	sScene.pView->CreateDynamicRects( pBackgroundTexture, sLayout, sScrPosition, sScrWindow );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInventoryInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CInventoryInterface: public NMainLoop::CInterfaceBase
{
	OBJECT_NOCOPY_METHODS(CInventoryInterface);
private:
	NInput::CBind bindExit, bindInventory;
	ZDATA_(NMainLoop::CInterfaceBase)
	CObj<IGlobalGame> pGame;
	////
	CPtr<IUnitTracker> pUnit;
	CObj<NUI::IWindow> pInventory;
	CObj<NUI::CInventoryDesktop> pInventoryCtrl;
	vector< CPtr<IUnitTracker> > selectedUnitsSet;
	CDGPtr<CPtrFuncBase<NGfx::CTexture> > pBackgroundTexture;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(NMainLoop::CInterfaceBase*)this); f.Add(2,&pGame); f.Add(3,&pUnit); f.Add(4,&pInventory); f.Add(5,&pInventoryCtrl); f.Add(6,&selectedUnitsSet); f.Add(7,&pBackgroundTexture); return 0; }

public:
	CInventoryInterface();
	~CInventoryInterface();

	void Initialize( IGlobalGame *pGame, IUnitTracker *_pUnit );
	void Terminate();
	void Step();
	bool ProcessEvent( const NInput::SEvent &eEvent );

	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CInventoryInterface::CInventoryInterface():	
	bindExit( "cancel" ), bindInventory( "inventory" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CInventoryInterface::~CInventoryInterface()
{
	Terminate();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryInterface::Initialize( IGlobalGame *_pGame, IUnitTracker *_pUnit )
{
	pGame = _pGame;
	pUnit = _pUnit;

	pBackgroundTexture = new NGScene::CScreenshotTexture;
	pBackgroundTexture.Refresh();

	if ( pUnit )
		pGame->Select( pUnit->GetUnit() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryInterface::Terminate()
{
	if ( IsValid( pGame ) )
	{
		if ( IsValid( pInventoryCtrl ) )
			pGame->PopDesktop();

		for ( int nTemp = 0; nTemp < selectedUnitsSet.size(); nTemp++ )
			pGame->Command( selectedUnitsSet[nTemp]->GetUnit(), new NWorld::CCmdInventoryLeave() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryInterface::Step()
{
	pGame->Step();

	vector< CPtr<IUnitTracker> > newSelectedUnitsSet;
	pGame->GetSelectedUnits( &newSelectedUnitsSet );
	if ( newSelectedUnitsSet != selectedUnitsSet )
	{
		for ( int nTemp = 0; nTemp < selectedUnitsSet.size(); nTemp++ )
			pGame->Command( selectedUnitsSet[nTemp]->GetUnit(), new NWorld::CCmdInventoryLeave() );

		selectedUnitsSet = newSelectedUnitsSet;
		if ( selectedUnitsSet.size() != 1 )
		{
			NMainLoop::Command( new NMainLoop::CICExitModal() );
			return;
		}

		if ( IsValid( pInventoryCtrl ) )
			pGame->PopDesktop();

		//pGame->Command( new NWorld::CCmdEnterInventory( pUnit ) );
		pInventory = NUI::IWindow::Create( pGame->GetInterface(), NUI::SRect( 0, 0, 1024, 768 ), "inventory" );
		pInventoryCtrl = new NUI::CInventoryDesktop( pInventory, pGame, (*selectedUnitsSet.begin())->GetUnit(), pBackgroundTexture );
		NUI::LoadTemplate( pInventoryCtrl, NDb::GetUIContainer( 120 ) );
		pInventoryCtrl->ShowWindow( NUI::SWTYPE_SHOW );

		pGame->PushDesktop( pInventoryCtrl );
	}
	
	if ( CanRender() )
		pGame->RenderFrame( N_RENDERMODE_2D, GetTime() );
	else
		pGame->GetRenderGame()->ResetTiming();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventoryInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	if ( bindExit.ProcessEvent( sEvent ) || bindInventory.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}

	if ( pGame->ProcessEvent( sEvent ) )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICInventory::CICInventory( IGlobalGame *_pGame, IUnitTracker *_pUnit ):
	pGame( _pGame ), pUnit( _pUnit )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICInventory::Exec()
{
	CInventoryInterface *pRes = new CInventoryInterface();
	pRes->Initialize( pGame, pUnit );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB2443952, CSingleSlot );
REGISTER_SAVELOAD_CLASS( 0xB2443953, CBackPackSlot );
REGISTER_SAVELOAD_CLASS( 0xB2443955, CInventoryUI );
REGISTER_SAVELOAD_CLASS( 0xB2443956, CInventoryUnit );
REGISTER_SAVELOAD_CLASS( 0xB2443957, CUnitModelShow );
REGISTER_SAVELOAD_CLASS( 0xB2443959, CInventoryDesktop );
REGISTER_SAVELOAD_CLASS( 0xB244395A, CInventoryUnitView );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB0243960, CInventoryInterface );
*/