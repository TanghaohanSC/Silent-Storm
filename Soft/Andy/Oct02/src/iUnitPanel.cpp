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
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "Sound.h"
#include "iMission.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iUnitPanel.h"
#include "iGameStates.h"
#include "iActionDecorator.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_MAXUNITS_COUNT = 7,
	N_SLOT_ACTIVATE_CLICKTIME	= 250;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitTab
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitTab: public CActionDecorator<CWindow>
{
	OBJECT_BASIC_METHODS(CUnitTab);
private:
	ZDATA_(TBaseClass)
	CPtr<NGame::IMission> pMission;
	////
	CPtr<NGame::IUnitTracker> pUnit;
	CPtr<CText> pAP;
	CPtr<CText> pName;
	CPtr<CImage> pDisable;
	CPtr<CImage> pSelectedTab;
	CPtr<CImage> pUnselectedTab;
	CObj<CLineBar> pLife;
	CObj<CLineBar> pHealedLife;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&pMission); f.Add(3,&pUnit); f.Add(4,&pAP); f.Add(5,&pName); f.Add(6,&pDisable); f.Add(7,&pSelectedTab); f.Add(8,&pUnselectedTab); f.Add(9,&pLife); f.Add(10,&pHealedLife); return 0; }

public:
	CUnitTab() {}
	CUnitTab( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool CanHandleState( NGame::IState *pState ) const;
	CObjectBase* GetTarget() const;

	void Set( NGame::IUnitTracker *pUnit );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitTab::CUnitTab( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	TBaseClass( sInfo, _pMission ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitTab::CanHandleState( NGame::IState *pState ) const
{
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CUnitTab::GetTarget() const
{
	return pUnit->GetUnit();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitTab::Set( NGame::IUnitTracker *_pUnit )
{
	pUnit = _pUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitTab::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pLife = new CLineBar( sEvent.pLoader->GetControl( "life" ) );
			pHealedLife = new CLineBar( sEvent.pLoader->GetControl( "life_healed" ) );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pAP = GetUIWindow<CText>( this, "ap" );
			pName = GetUIWindow<CText>( this, "name" );

			pDisable = GetUIWindow<CImage>( this, "disable" );
			pSelectedTab = GetUIWindow<CImage>( this, "ut_selected" );
			pUnselectedTab = GetUIWindow<CImage>( this, "ut_unselected" );
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitTab::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !IsValid( pUnit ) )
	{
		SetStyle( STYLE_VISIBLE, false );
		return;
	}

	SetStyle( STYLE_VISIBLE, true );
	pDisable->SetStyle( STYLE_VISIBLE, !pUnit->IsActive() );
	pSelectedTab->SetStyle( STYLE_VISIBLE, pUnit->IsSelected() );
	pUnselectedTab->SetStyle( STYLE_VISIBLE, !pUnit->IsSelected() );

	NRPG::SUnitInfo sUnitInfo;
	pUnit->GetUnit()->GetInfo( &sUnitInfo );

	WCHAR wsText[256];

	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", sUnitInfo.nAP );
	pAP->SetText( wsText );
	pAP->SetStyle( STYLE_VISIBLE, !pMission->IsRealTime() );

	wstring wsName( L"Unknown" );
	if ( pUnit->GetUnit()->GetRPG()->GetRPGPers()->pName )
		wsName = pUnit->GetUnit()->GetRPG()->GetRPGPers()->pName->szStr;
	pName->SetText( L"<nowrap><font face=Courier size=16pt>" + wsName );

	pLife->Set( float( sUnitInfo.nHP ) / sUnitInfo.nMaxHP );
	pHealedLife->Set( float( sUnitInfo.nHP + sUnitInfo.nHealedHP ) / sUnitInfo.nMaxHP );

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitsTab
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitsTabBar: public CWindow
{
	OBJECT_BASIC_METHODS(CUnitsTabBar)
private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	vector< CPtr<CUnitTab> > tabsSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&tabsSet); return 0; }

public:
	CUnitsTabBar() {}
	CUnitsTabBar( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitsTabBar::CUnitsTabBar( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission ), tabsSet( N_MAXUNITS_COUNT )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitsTabBar::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			for ( int nTemp = 0; nTemp < tabsSet.size(); nTemp++ )
			{
				tabsSet[nTemp] = new CUnitTab( sEvent.pLoader->GetControl( NStr::Format( "hero_%d", ( nTemp + 1 ) ) ), pMission );
				tabsSet[nTemp]->Set( 0 );
			}

			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitsTabBar::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	vector< CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetUnits( &unitsSet );

	int nCount = 0;
	for ( int nTemp = 0; nTemp < tabsSet.size(); nTemp++ )
	{
		tabsSet[nTemp]->Set( 0 );
		if ( nTemp < unitsSet.size() )
		{
			if ( !unitsSet[nTemp]->GetUnit()->IsDead() )
			{
				tabsSet[nCount]->Set( unitsSet[nTemp] );
				tabsSet[nCount]->SetStyle( STYLE_VISIBLE, true );
				tabsSet[nCount]->SetStyle( STYLE_TOPMOST, unitsSet[nTemp]->IsSelected() );
				nCount++;
			}
		}
	}

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitFace
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitFace: public CActionDecorator<CWindow>
{
	OBJECT_BASIC_METHODS(CUnitFace)
private:
	ZDATA_(TBaseClass)
	CPtr<NGame::IMission> pMission;
	CPtr<NGame::IUnitTracker> pUnit;
	////
	CObj<CLineBar> pLife;
	CObj<CLineBar> pPCLife;
	CObj<CLineBar> pHealedLife;
	CObj<CUnitHead> pUnitHead;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&pMission); f.Add(3,&pUnit); f.Add(4,&pLife); f.Add(5,&pPCLife); f.Add(6,&pHealedLife); f.Add(7,&pUnitHead); return 0; }

public:
	CUnitFace() {}
	CUnitFace( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool CanHandleState( NGame::IState *pState ) const;
	CObjectBase* GetTarget() const;

	void Set( NGame::IUnitTracker *pUnit );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitFace::CUnitFace( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	TBaseClass( sInfo, _pMission ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitFace::CanHandleState( NGame::IState *pState ) const
{
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CUnitFace::GetTarget() const
{
	return pUnit->GetUnit();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitFace::Set( NGame::IUnitTracker *_pUnit )
{
	pUnit = _pUnit;
	if ( IsValid( pUnit ) )
		pUnitHead->SetUnit( pUnit->GetUnit() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitFace::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONDOWN:
		return true;
	case EVENT_LBUTTONUP:
		{
			SendMessage( GetParent(), SEvent( EVENT_NOTIFY, GetWindowID() ) );
			return true;
		}
	case EVENT_TEMPLATELOAD:
		{
			pLife = new CLineBar( sEvent.pLoader->GetControl( "life" ) );
//			pPCLife = new CLineBar( GetUIWindow<IImage>( this, "pclife", sEvent.pLoader ) );
			pHealedLife = new CLineBar( sEvent.pLoader->GetControl( "life_healed" ) );
			pUnitHead = new CUnitHead( sEvent.pLoader->GetControl( "face" ), pMission->GetRenderGame(), 2.1f );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitFace::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !IsValid( pUnit ) )
	{
		SetStyle( STYLE_VISIBLE, false );
		return;
	}

	SetStyle( STYLE_VISIBLE, true );

	NRPG::SUnitInfo sUnitInfo;
	pUnit->GetUnit()->GetInfo( &sUnitInfo );

	pLife->Set( (float)sUnitInfo.nHP / sUnitInfo.nMaxHP );
//	pPCLife->Set( 1 /*(float)sUnitInfo.nHP / sUnitInfo.nMaxHP*/ );
	pHealedLife->Set( (float)( sUnitInfo.nHP + sUnitInfo.nHealedHP ) / sUnitInfo.nMaxHP );

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSlotReloadImage
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSlotReloadButton: public CButton
{
	OBJECT_NOCOPY_METHODS(CSlotReloadButton)
private:
	ZDATA_(CButton)
	CPtr<NGame::IMission> pMission;
	////
	CObj<CModel> pModel;
	CObj<CToolTip> pToolTip;
	CDBPtr<NDb::CRPGItem> pItem;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CButton*)this); f.Add(2,&pMission); f.Add(3,&pModel); f.Add(4,&pToolTip); f.Add(5,&pItem); return 0; }

public:
	CSlotReloadButton() {}
	CSlotReloadButton( const SWindowInfo &sInfo, NGame::IMission *pMission );

	void Set( NDb::CRPGItem *pItem );
	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CSlotReloadButton::CSlotReloadButton( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CButton( sInfo ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSlotReloadButton::Set( NDb::CRPGItem *_pItem )
{
	if ( _pItem == pItem )
		return;

	pItem = _pItem;
	if ( pItem->pModel )
	{
		const NDb::SCameraParams &sCamera = pItem->sCameras[NDb::CAMERA_RELOADBUTTON];

		CVec3 vForwardDir;
		CQuat q = CQuat( sCamera.fYaw, V3_AXIS_Z ) * CQuat( sCamera.fPitch, V3_AXIS_X );
		q.GetYAxis( &vForwardDir );

		CVec3 vCP( sCamera.vAnchor - vForwardDir * sCamera.fDistance );
		SFBTransform res;
		MakeMatrix( &res, sCamera.fPitch, sCamera.fYaw, vCP );

		SRand sRnd;
		pModel->SetModel( pItem->pModel->CreateModel( &sRnd ) );
		pModel->SetTransform( new CFBTransform( res ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSlotReloadButton::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATECREATE:
		{
			pModel = new CModel( SWindowInfo( this, SPoint( 0, 0 ), GetSize(), "", STYLE_ENABLED | STYLE_VISIBLE | STYLE_TRANSPARENT | STYLE_TOPMOST ) );
			pToolTip = new CToolTip( SWindowInfo( GetInterface(), SPoint( 0, 0 ), SPoint( 0, 0 ), "tooltip", STYLE_ENABLED ) );
			SetToolTip( pToolTip );
			break;
		}
	}

	return CButton::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSlotReloadButton::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetSelectedUnits( &unitsSet );

	ASSERT( unitsSet.size() == 1 );
	if ( unitsSet.size() != 1 )
		return;

	CPtr<NGame::IUnitTracker> pUnit = unitsSet.front();

	int nActionAP = 0;
	NWorld::EUnitCommandResult eResult = pUnit->GetUnit()->CanDo( new NWorld::CCmdReload, 0, &nActionAP );

	CPtr<NDb::CString> pString;
	if ( ( eResult == NWorld::UCR_OK ) || ( eResult == NWorld::UCR_NOT_ENOUGH_AP ) )
		pString = NDb::GetString( 4313 );
	else
		pString = NDb::GetString( 4314 );

	wstring wsToolTipTemplate( L"%s" );
	if ( IsValid( pString ) )
		wsToolTipTemplate = pString->szStr;

	if ( nActionAP != -1 )
		pToolTip->SetVal( L"ap", nActionAP );
	else
		pToolTip->SetVal( L"ap", L"N/A" );

	NGfx::SPixel8888 sColor( 0xFF, 0xFF, 0xFF, 0xFF );
	if ( eResult == NWorld::UCR_NOT_ENOUGH_AP )
		sColor = NGfx::SPixel8888( 0x5F, 0x5F, 0xBF, 0xFF );

	SetColor( sColor );
	pModel->SetColor( sColor );

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInfoPanelSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
class CInfoPanelSlot: public CSlot
{
	OBJECT_NOCOPY_METHODS(CInfoPanelSlot)
private:
	ZDATA_(CSlot)
	CPtr<NGame::IMission> pMission;
	////
	NDb::ESlot eType;
	CPtr<NWorld::CUnit> pUnit;
	////
	CPtr<CWindow> pFade;
	CObj<CText> pWeaponAmmoText;
	CPtr<CImage> pAmmoBackground;
	CObj<CProgressBar> pWeaponAmmo;
	CObj<CSlotReloadButton> pReload;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CSlot*)this); f.Add(2,&pMission); f.Add(3,&eType); f.Add(4,&pUnit); f.Add(5,&pFade); f.Add(6,&pWeaponAmmoText); f.Add(7,&pAmmoBackground); f.Add(8,&pWeaponAmmo); f.Add(9,&pReload); return 0; }

protected:
	void Take( int nX, int nY );
	void Place( int nX, int nY, const NWorld::SItem &sItem );
	void GetItemsList( vector<SItem> *pItemsSet );
	bool CanTransferFrom( NWorld::CUnit *pUnit );

public:
	CInfoPanelSlot() {}
	CInfoPanelSlot( const SWindowInfo &sInfo, NGame::IMission *pMission, NDb::ESlot eType );

	void Set( NWorld::CUnit *pUnit );
	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInfoPanelSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
CInfoPanelSlot::CInfoPanelSlot( const SWindowInfo &sInfo, NGame::IMission *_pMission, NDb::ESlot _eType ):
	CSlot( sInfo, _pMission, 1, 1, NDb::CAMERA_SLOT, false ), pMission( _pMission ), eType( _eType )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelSlot::Set( NWorld::CUnit *_pUnit )
{
	pUnit = _pUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelSlot::Take( int nX, int nY )
{
	ASSERT( IsValid( pUnit ) );

	NWorld::SItem sSource;
	sSource.eType = NWorld::SItem::SLOT;
	sSource.nSlot = eType;
	sSource.pItem = pUnit->GetRPG()->GetInventoryInfo()->Get( eType );
	sSource.pUnit = pUnit;
	pMission->CommandState( new NGame::CStateMoveItem( pUnit, sSource, NWorld::SItem( pUnit, NWorld::SItem::HAND ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelSlot::Place( int nX, int nY, const NWorld::SItem &sItem )
{
	ASSERT( IsValid( pUnit ) );

	NWorld::SItem sTarget;
	sTarget.eType = NWorld::SItem::SLOT;
	sTarget.nSlot = eType;
	sTarget.pUnit = pUnit;

	pMission->Command( sItem.pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::SItem( sItem.pUnit, NWorld::SItem::HAND ), sTarget ) );
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelSlot::GetItemsList( vector<SItem> *pItemsSet )
{
	ASSERT( IsValid( pUnit ) );

	CPtr<NRPG::IInventoryInfo> pInfo = pUnit->GetRPG()->GetInventoryInfo();
	CPtr<NRPG::IInventoryItem> pItem = pInfo->Get( eType );
	if ( IsValid( pItem ) )
	{
		SItem &sItem = *pItemsSet->insert( pItemsSet->end() );
		sItem.sPos = CTPoint<int>( 0, 0 );
		sItem.pItem = pItem;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInfoPanelSlot::CanTransferFrom( NWorld::CUnit *_pUnit )
{
	if ( pUnit == _pUnit )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInfoPanelSlot::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONDOWN:
		{
			NWorld::IPlayer::SItemInfo sInfo;
			if ( !GetDragItem( &sInfo ) && ( pUnit->GetRPG()->GetInventoryInfo()->GetActiveSlot() != eType ) )
				pMission->Command( pUnit, new NWorld::CCmdSetActiveItem( eType ) );

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pReload = new CSlotReloadButton( sEvent.pLoader->GetControl( "weapon_reload" ), pMission );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pFade = GetUIWindow<CWindow>( this, "fade" );
			pWeaponAmmo = GetUIWindow<CProgressBar>( this, "ammo" );
			pWeaponAmmoText = GetUIWindow<CText>( this, "ammo_text" );
			pAmmoBackground = GetUIWindow<CImage>( this, "ammo_background" );
			break;
		}
	}

	if ( CSlot::ProcessMessage( sEvent ) )
		return true;

	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONDOWN:
		{
			NWorld::IPlayer::SItemInfo sInfo;
			if ( GetDragItem( &sInfo ) )
				Place( sEvent.nX, sEvent.nY, NWorld::SItem( sInfo.pUnit, NWorld::SItem::HAND ) );
			else if ( pUnit->GetRPG()->GetInventoryInfo()->GetActiveSlot() == eType )
				Take( sEvent.nX, sEvent.nY );
			return true;
		}
	case EVENT_LBUTTONUP:
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelSlot::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !IsValid( pUnit ) )
		return;

	CPtr<NRPG::IInventoryInfo> pInventory = pUnit->GetRPG()->GetInventoryInfo();
	pFade->SetStyle( STYLE_VISIBLE, pInventory->GetActiveSlot() != eType );

	CPtr<NRPG::IInventoryItem> pItem = pInventory->Get( eType );
	if ( CDynamicCast<NRPG::IWeaponItemInfo> pWeapon( pItem ) )
	{
		CPtr<NRPG::IClipItem> pRPGClipItem = pWeapon->GetInnerClip();
		pReload->SetStyle( STYLE_VISIBLE, true );
		pReload->Set( pRPGClipItem->GetDBItem() );
		pWeaponAmmo->SetStyle( STYLE_VISIBLE, true );
		pWeaponAmmo->SetValue( float( pRPGClipItem->GetIncQuantity() ) / pRPGClipItem->GetMaxIncQuantity() );
		pAmmoBackground->SetStyle( STYLE_VISIBLE, true );

		WCHAR wsBuffer[1024];
		swprintf( wsBuffer, L"<font face=Courier size=16pt><center>%d/%d", pRPGClipItem->GetIncQuantity(), pRPGClipItem->GetMaxIncQuantity() );
		pWeaponAmmoText->SetText( wsBuffer );
		pWeaponAmmoText->SetStyle( STYLE_VISIBLE, true );
	}
	else
	{
		pReload->SetStyle( STYLE_VISIBLE, false );
		pWeaponAmmo->SetStyle( STYLE_VISIBLE, false );
		pWeaponAmmoText->SetStyle( STYLE_VISIBLE, false );
		pAmmoBackground->SetStyle( STYLE_VISIBLE, false );
	}

	CSlot::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInfoPanelSpecialSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
class CInfoPanelSpecialSlot: public CWindow
{
	OBJECT_NOCOPY_METHODS(CInfoPanelSpecialSlot)
private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	CPtr<NWorld::CUnit> pUnit;
	////
	CObj<CText> pWeaponAmmoText;
	CPtr<CImage> pAmmoBackground;
	CObj<CItemModel> pItemModel;
	CObj<CProgressBar> pWeaponAmmo;
	CObj<CSlotReloadButton> pReload;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pUnit); f.Add(4,&pWeaponAmmoText); f.Add(5,&pAmmoBackground); f.Add(6,&pItemModel); f.Add(7,&pWeaponAmmo); f.Add(8,&pReload); return 0; }

public:
	CInfoPanelSpecialSlot() {}
	CInfoPanelSpecialSlot( const SWindowInfo &sInfo, NGame::IMission *pMission );

	void Set( NWorld::CUnit *pUnit );
	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInfoPanelSpecialSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
CInfoPanelSpecialSlot::CInfoPanelSpecialSlot( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelSpecialSlot::Set( NWorld::CUnit *_pUnit )
{
	pUnit = _pUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInfoPanelSpecialSlot::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pReload = new CSlotReloadButton( sEvent.pLoader->GetControl( "weapon_reload" ), pMission );
			pItemModel = new CItemModel( sEvent.pLoader->GetControl( "view" ), pMission );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pWeaponAmmo = GetUIWindow<CProgressBar>( this, "ammo" );
			pWeaponAmmoText = GetUIWindow<CText>( this, "ammo_text" );
			pAmmoBackground = GetUIWindow<CImage>( this, "ammo_background" );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelSpecialSlot::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !IsValid( pUnit ) )
	{
		SetStyle( STYLE_VISIBLE, false );
		return;
	}
	CPtr<NRPG::IWeaponItemInfo> pItem = pUnit->GetRPG()->GetCannonItemInfo();
	if ( !IsValid( pItem ) )
	{
		SetStyle( STYLE_VISIBLE, false );
		return;
	}

	SetStyle( STYLE_VISIBLE, true );

	pItemModel->Set( pItem, NDb::CAMERA_SLOT );

	CPtr<NRPG::IClipItem> pRPGClipItem = pItem->GetInnerClip();
	pReload->Set( pRPGClipItem->GetDBItem() );
	pWeaponAmmo->SetValue( float( pRPGClipItem->GetIncQuantity() ) / pRPGClipItem->GetMaxIncQuantity() );

	WCHAR wsBuffer[1024];
	swprintf( wsBuffer, L"<font face=Courier size=16pt><center>%d/%d", pRPGClipItem->GetIncQuantity(), pRPGClipItem->GetMaxIncQuantity() );
	pWeaponAmmoText->SetText( wsBuffer );

	CWindow::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInfoPanelSingleUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CInfoPanelSingleUnit: public CWindow
{
	OBJECT_BASIC_METHODS(CInfoPanelSingleUnit)
private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	CPtr<NGame::IUnitTracker> pUnit;
	////
	CPtr<CWindow> pAPText;
	CObj<CUnitFace> pUnitFace;
	CObj<CImageNumber> pAP;
	CObj<CInfoPanelSlot> pLeftSlot;
	CObj<CInfoPanelSlot> pRightSlot;
	CObj<CInfoPanelSpecialSlot> pSpecialSlot;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pUnit); f.Add(4,&pAPText); f.Add(5,&pUnitFace); f.Add(6,&pAP); f.Add(7,&pLeftSlot); f.Add(8,&pRightSlot); f.Add(9,&pSpecialSlot); return 0; }

public:
	CInfoPanelSingleUnit() {}
	CInfoPanelSingleUnit( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CInfoPanelSingleUnit::CInfoPanelSingleUnit( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInfoPanelSingleUnit::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "face" )
			{
				pMission->FocusCameraOnUnit( pUnit->GetUnit() );
				return true;
			}
			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pAP = new CImageNumber( sEvent.pLoader->GetControl( "ap" ), CImageNumber::TYPE_UNITINFOPANEL );
			pUnitFace = new CUnitFace( sEvent.pLoader->GetControl( "face" ), pMission );
			pLeftSlot = new CInfoPanelSlot( sEvent.pLoader->GetControl( "slot_left" ), pMission, NDb::SLOT_1 );
			pRightSlot = new CInfoPanelSlot( sEvent.pLoader->GetControl( "slot_right" ), pMission, NDb::SLOT_2 );
			pSpecialSlot = new CInfoPanelSpecialSlot( sEvent.pLoader->GetControl( "slot_double" ), pMission );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pAPText = GetUIWindow<CWindow>( this, "ap_text" );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelSingleUnit::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
	{
		SetStyle( STYLE_VISIBLE, false );
		return;
	}

	pUnit = unitsSet[0];
	SetStyle( STYLE_VISIBLE, true );

	if ( !pMission->IsRealTime() )
	{
		NRPG::SUnitInfo sUnitInfo;
		pUnit->GetUnit()->GetInfo( &sUnitInfo );
		pAP->Set( sUnitInfo.nAP );
		pAP->SetStyle( STYLE_VISIBLE, true );
		pAPText->SetStyle( STYLE_VISIBLE, true );
	}
	else
	{
		pAP->SetStyle( STYLE_VISIBLE, false );
		pAPText->SetStyle( STYLE_VISIBLE, false );
	}

	pUnitFace->Set( pUnit );
	pLeftSlot->Set( pUnit->GetUnit() );
	pRightSlot->Set( pUnit->GetUnit() );
	pSpecialSlot->Set( pUnit->GetUnit() );

	CWindow::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInfoPanelMultipleUnits
////////////////////////////////////////////////////////////////////////////////////////////////////
class CInfoPanelMultipleUnits: public CWindow
{
	OBJECT_BASIC_METHODS(CInfoPanelMultipleUnits)
private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	CPtr<NGame::IUnitTracker> pUnit;
	////
	vector<CPtr<CImage> > selectionsSet;
	vector<CObj<CUnitFace> > facesSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pUnit); f.Add(4,&selectionsSet); f.Add(5,&facesSet); return 0; }

public:
	CInfoPanelMultipleUnits() {}
	CInfoPanelMultipleUnits( const SWindowInfo &sInfo, NGame::IMission *pMission );

	void Set( NGame::IUnitTracker *pUnit );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CInfoPanelMultipleUnits::CInfoPanelMultipleUnits( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission ), selectionsSet( N_MAXUNITS_COUNT ), facesSet( N_MAXUNITS_COUNT )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInfoPanelMultipleUnits::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			for ( int nTemp = 0; nTemp < facesSet.size(); nTemp++ )
			{
				facesSet[nTemp] = new CUnitFace( sEvent.pLoader->GetControl( NStr::Format( "hero_%d", ( nTemp + 1 ) ) ), pMission );
				facesSet[nTemp]->Set( 0 );
			}
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			for ( int nTemp = 0; nTemp < facesSet.size(); nTemp++ )
				selectionsSet[nTemp] = GetUIWindow<CImage>( this, NStr::Format( "hero_%d_selection", ( nTemp + 1 ) ) );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoPanelMultipleUnits::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( pMission->CountSelected() < 2 )
	{
		SetStyle( STYLE_VISIBLE, false );
		return;
	}

	SetStyle( STYLE_VISIBLE, true );

	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetUnits( &unitsSet );
	for ( int nTemp = 0; nTemp < facesSet.size(); nTemp++ )
	{
		if ( nTemp < unitsSet.size() )
		{
			facesSet[nTemp]->Set( unitsSet[nTemp] );
			facesSet[nTemp]->SetStyle( STYLE_VISIBLE, true );
			selectionsSet[nTemp]->SetStyle( STYLE_VISIBLE, unitsSet[nTemp]->IsSelected() );
		}
		else
		{
			facesSet[nTemp]->SetStyle( STYLE_VISIBLE, false );
			selectionsSet[nTemp]->SetStyle( STYLE_VISIBLE, false );
		}
	}

	CWindow::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitPanel::CUnitPanel( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitPanel::ProcessMessage( const SEvent &sEvent )
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
			pUnitsTabBar = new CUnitsTabBar( sEvent.pLoader->GetControl( "unitstabbar" ), pMission );
			pInfoPanelSingleUnit = new CInfoPanelSingleUnit( sEvent.pLoader->GetControl( "infopanel_singleunit" ), pMission );
			pInfoPanelMultipleUnits = new CInfoPanelMultipleUnits( sEvent.pLoader->GetControl( "infopanel_multipleunits" ), pMission );
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
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0521151, CUnitTab );
REGISTER_SAVELOAD_CLASS( 0xB0521152, CUnitsTabBar );
REGISTER_SAVELOAD_CLASS( 0xB0521153, CUnitFace );
REGISTER_SAVELOAD_CLASS( 0xB0521155, CInfoPanelSlot );
REGISTER_SAVELOAD_CLASS( 0xB0521156, CInfoPanelSingleUnit );
REGISTER_SAVELOAD_CLASS( 0xB0521157, CInfoPanelMultipleUnits );
REGISTER_SAVELOAD_CLASS( 0xB0521158, CUnitPanel );
REGISTER_SAVELOAD_CLASS( 0xB0521159, CSlotReloadButton );
REGISTER_SAVELOAD_CLASS( 0xB052115A, CInfoPanelSpecialSlot );
