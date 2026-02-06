#include "StdAfx.h"
#include "GSceneUtils.h"
#include "GView.h"
#include "G2DView.h"
#include "Transform.h"
#include "DiscretePos.h"
#include "wInterface.h"
#include "RPGItemInfo.h"
#include "RPGUnitInfo.h"
#include "RWGame.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "Sound.h"
#include "iMission.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iInventoryPanel.h"
#include "iGameStates.h"
#include "iActionDecorator.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_BACKPACK_WIDTH	= 7,
	N_BACKPACK_HEIGHT = 12;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitShow
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitModelShow: public CActionDecorator<CWindow>
{
	OBJECT_BASIC_METHODS(CUnitModelShow);
private:
	ZDATA_(TBaseClass)
	CPtr<NGame::IMission> pMission;
	////
	bool bButtonDown;
	float fAngle;
	SPoint sLastPoint;
	CTimeCounter sTimer;
	CObj<CObjectBase> pMouseCapture;
	CObj<NGScene::ILight> pAmbientDirectional;
	CObj<NGScene::IGameView> p3DView;
	CPtr<NGame::IUnitTracker> pUnit;
	CPtr<NRender::IShowUnit> pInventoryUnit;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&pMission); f.Add(3,&bButtonDown); f.Add(4,&fAngle); f.Add(5,&sLastPoint); f.Add(6,&sTimer); f.Add(7,&pMouseCapture); f.Add(8,&pAmbientDirectional); f.Add(9,&p3DView); f.Add(10,&pUnit); f.Add(11,&pInventoryUnit); return 0; }

public:
	CUnitModelShow() {}
	CUnitModelShow( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool CanHandleState( NGame::IState *pState ) const;
	CObjectBase* GetTarget() const;

	void Set( NGame::IUnitTracker *pUnit );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitModelShow::CUnitModelShow( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	TBaseClass( sInfo, _pMission ), pMission( _pMission ), bButtonDown( false ), fAngle( 0.0f ), sLastPoint( 0, 0 )
{
	p3DView = NGScene::CreateNewFastInterfaceView();//CreateNewView();
	//p3DView->SetFastMode();
	p3DView->SetAmbient( 0 );
	pAmbientDirectional = p3DView->AddDirectionalLight( CVec3(0.5f,0.4f,0.45f), CVec3( 0.6f,1.4f,-1), CVec3(5,5,0), CVec2( 150, 150 ), 20 );
	p3DView->SetAmbient( CVec3( 0.5f, 0.5f, 0.5f ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitModelShow::CanHandleState( NGame::IState *pState ) const
{
	if ( CDynamicCast<NGame::CStateDragItem> pDrag( pState ) )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CUnitModelShow::GetTarget() const
{
	return pUnit->GetUnit();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitModelShow::Set( NGame::IUnitTracker *_pUnit )
{
	if ( pUnit != _pUnit )
	{
		pUnit = _pUnit;
		pInventoryUnit = NRender::CreateShowUnit( p3DView, pUnit->GetUnit(), sTimer.GetTime(), pMission->GetRenderGame() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitModelShow::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_MOUSEMOVE:
		{
			GetInterface()->SetCursorInfo( SCursorInfo() );

			if ( !bButtonDown )
				break;

			fAngle += float( sEvent.nX - sLastPoint.x ) / 10;
			sLastPoint.x = sEvent.nX;
			sLastPoint.y = sEvent.nY;
			break;
		}
	}

	if ( TBaseClass::ProcessMessage( sEvent ) )
		return true;

	switch ( sEvent.nEvent )
	{
	case EVENT_LBUTTONUP:
		{
			bButtonDown = false;
			pMouseCapture = 0;
			return true;
		}
	case EVENT_LBUTTONDOWN:
		{
			bButtonDown = true;
			sLastPoint.x = sEvent.nX;
			sLastPoint.y = sEvent.nY;
			pMouseCapture = GetInterface()->CreateMouseCapture( this );
			return true;
		}
	case EVENT_MOUSECAPTURELOSE:
		{
			bButtonDown = false;
			pMouseCapture = 0;
			break;
		}
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitModelShow::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	sTimer.Advance( true, GetTickCount() );
	pInventoryUnit->Update( fAngle );

	TBaseClass::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitModelShow::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	SRect sWindow;
	SPoint sPosition;
	if ( !ClientToScreen( &sPosition, &sWindow ) )
		return;

	pView->Flush();
	CVec2 vPos( ( sWindow.x2 + sWindow.x1 ) / 2, ( sWindow.y2 + sWindow.y1 ) / 2 + 190 );
	SHMatrix sMatrix;
	CTransformStack ts;
	MakeMatrix( &sMatrix, ToRadian( 0 ), ToRadian( 90.0f ), CVec3( 10, (float)( 512 - vPos.x ) * 5 / 1024, (float)( vPos.y - 384 ) * 3.75f / 768 ) );
	ts.Init();
	ts.MakeParallel( 5, 3.75f, 0, 20 );
	ts.SetCamera( sMatrix );
	//ts.PushScale( 2 );
	//NGScene::Clear( CVec3(0.5f, 0.5f, 0.5f ) );
	NGScene::IGameView::SDrawInfo drawInfo;
	drawInfo.pTS = &ts;
	drawInfo.vOrigin = CVec2( sPosition.x / 1024.0f, sPosition.y / 768.0f );
	drawInfo.vSize = CVec2( sWindow.Width() / 1024.0f, sWindow.Height() / 768.0f );
	drawInfo.bOverlay = true;
	p3DView->Draw( drawInfo );

	TBaseClass::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CBackPackSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
class CBackPackSlot: public CSlot
{
	OBJECT_BASIC_METHODS(CBackPackSlot);
private:
	ZDATA_( CSlot )
	CPtr<NGame::IMission> pMission;
	////
	CPtr<NGame::IUnitTracker> pUnit;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,( CSlot *)this); f.Add(2,&pMission); f.Add(3,&pUnit); return 0; }

protected:
	void Take( int nX, int nY );
	void Place( int nX, int nY, const NWorld::SItem &sItem );
	void GetItemsList( vector<SItem> *pItemsSet );
	bool CanTransferFrom( NWorld::CUnit *pUnit );

public:
	CBackPackSlot() {}
	CBackPackSlot( const SWindowInfo &sInfo, NGame::IMission *pMission );

	void Set( NGame::IUnitTracker *pUnit );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CBackPackSlot::CBackPackSlot( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CSlot( sInfo, _pMission, N_BACKPACK_WIDTH, N_BACKPACK_HEIGHT, NDb::CAMERA_NORMAL, true ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CBackPackSlot::Set( NGame::IUnitTracker *_pUnit )
{
	pUnit = _pUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CBackPackSlot::Take( int nX, int nY )
{
	SPoint sPos;
	GetInSlotPos( nX, nY, &sPos );

	const vector<NRPG::SBackPackItem> &itemsSet = pUnit->GetUnit()->GetRPG()->GetInventoryInfo()->GetItems();
	for ( int nTemp = 0; nTemp < itemsSet.size(); nTemp++ )
	{
		const NRPG::SBackPackItem &sItem = itemsSet[nTemp];

		const SPoint &sItemPos = itemsSet[nTemp].sPos;
		const SPoint &sItemSize = itemsSet[nTemp].pItem->GetSize();

		if ( ( sItemPos.x <= sPos.x ) && ( sItemPos.x + sItemSize.x > sPos.x  ) && ( sItemPos.y <= sPos.y ) && ( sItemPos.y + sItemSize.y > sPos.y  ) )
		{
			NWorld::SItem sInvItem;
			sInvItem.eType = NWorld::SItem::BACKPACK;
			sInvItem.pItem = sItem.pItem;
			sInvItem.pUnit = pUnit->GetUnit();
			sInvItem.sPosition = sItem.sPos;
			pMission->CommandState( new NGame::CStateMoveItem( pUnit->GetUnit(), sInvItem, NWorld::SItem( pUnit->GetUnit(), NWorld::SItem::HAND ) ) );
			return;
		}
	}

	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CBackPackSlot::Place( int nX, int nY, const NWorld::SItem &sItem )
{
	NWorld::IPlayer::SItemInfo sInfo;
	if ( !GetDragItem( &sInfo ) )
		return;

	const CTPoint<int> &sSize = sInfo.pItem->GetSize();

	SPoint sPos;
	GetItemInSlotPos( nX, nY, sSize, &sPos );

	NWorld::SItem sTarget;
	sTarget.eType = NWorld::SItem::BACKPACK;
	sTarget.pUnit = pUnit->GetUnit();
	sTarget.sPosition = sPos;

	pMission->Command( sItem.pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::SItem( sInfo.pUnit, NWorld::SItem::HAND ), sTarget ) );
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CBackPackSlot::GetItemsList( vector<SItem> *pItemsSet )
{
	const vector<NRPG::SBackPackItem> &itemsSet = pUnit->GetUnit()->GetRPG()->GetInventoryInfo()->GetItems();
	pItemsSet->resize( itemsSet.size() );
	for ( int nTemp = 0; nTemp < itemsSet.size(); nTemp++ )
	{
		SItem &sItem = (*pItemsSet)[nTemp];
		sItem.sPos = itemsSet[nTemp].sPos;
		sItem.pItem = itemsSet[nTemp].pItem;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBackPackSlot::CanTransferFrom( NWorld::CUnit *_pUnit )
{
	if ( pUnit->GetUnit() == _pUnit )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBackPackSlot::ProcessMessage( const SEvent &sEvent )
{
	if ( CSlot::ProcessMessage( sEvent ) )
		return true;

	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONDOWN:
		{
			NWorld::IPlayer::SItemInfo sInfo;
			if ( GetDragItem( &sInfo ) )
				Place( sEvent.nX, sEvent.nY, NWorld::SItem( sInfo.pUnit, NWorld::SItem::HAND ) );
			else
				Take( sEvent.nX, sEvent.nY );

			return true;
		}
	case EVENT_LBUTTONUP:
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CInventoryPanel::CInventoryPanel( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventoryPanel::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_MOUSEMOVE:
		{
			GetInterface()->SetCursorInfo( SCursorInfo() );
			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pBackPack = new CBackPackSlot( sEvent.pLoader->GetControl( "backpack" ), pMission );
			pUnitModelShow = new CUnitModelShow( sEvent.pLoader->GetControl( "unitview" ), pMission );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pClose = GetUIWindow<CButton>( this, "inventory" );
			pClose->AddImageState( 0, NDb::GetUITexture( 437 ) );
			pRepair = GetUIWindow<CButton>( this, "repair" );
			pRepair->SetStyle( STYLE_ENABLED, false );
			pRepair->AddImageState( 0, NDb::GetUITexture( 423 ) );
			pUnload = GetUIWindow<CButton>( this, "unload" );
			pUnload->AddImageState( 0, NDb::GetUITexture( 397 ) );
			pArrange = GetUIWindow<CButton>( this, "arrange" );
			pArrange->SetStyle( STYLE_ENABLED, false );
			pArrange->AddImageState( 0, NDb::GetUITexture( 442 ) );
			break;
		}
	}

	if ( CWindow::ProcessMessage( sEvent ) )
		return true;

	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONUP:
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONDBLCLK:
	case EVENT_RBUTTONUP:
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONDBLCLK:
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventoryPanel::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !GetStyle( STYLE_VISIBLE ) )
		return;

	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
	{
		SetStyle( STYLE_VISIBLE, false );
		return;
	}

	pBackPack->Set( unitsSet[0] );
	pUnitModelShow->Set( unitsSet[0] );

	CWindow::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0521141, CUnitModelShow );
REGISTER_SAVELOAD_CLASS( 0xB0521142, CBackPackSlot );
REGISTER_SAVELOAD_CLASS( 0xB0521143, CInventoryPanel );
