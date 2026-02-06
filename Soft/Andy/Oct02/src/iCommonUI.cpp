#include "StdAfx.h"
#include "GSceneUtils.h"
#include "Transform.h"
#include "DiscretePos.h"
#include "GView.h"
#include "G2DView.h"
#include "wInterface.h"
#include "RPGMerc.h"
#include "RPGUnit.h"
#include "RPGItemInfo.h"
#include "RPGUnitInfo.h"
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataInterface.h"
#include "Interface.h"
#include "iMission.h"
#include "iCommonUI.h"
#include "RWGame.h"
#include "iGameStates.h"
#include "iActionDecorator.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_FLASHBUTTON_FLASHTIME = 500;
////////////////////////////////////////////////////////////////////////////////////////////////////
static wstring CStringToWString( NDb::CString *pString )
{
	wstring wsTemp( L"[UNKNOWN]" );
	if ( pString )
		wsTemp = pString->szStr;

	return wsTemp;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLineBar
////////////////////////////////////////////////////////////////////////////////////////////////////
CLineBar::CLineBar( const SWindowInfo &sInfo ):
	CImage( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLineBar::Set( float fBar )
{
	SetSize( SPoint( nBarWidth * fBar, GetSize().y ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CLineBar::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATECREATE:
		{
			nBarWidth = GetSize().x;
			break;
		}
	}

	return CImage::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CImageNumber
////////////////////////////////////////////////////////////////////////////////////////////////////
CImageNumber::CImageNumber( const SWindowInfo &sInfo, EType eType ):
	CWindow( sInfo ), textureIDs( 10 ), nValue( -1 ), sColor( 0xFF, 0xFF, 0xFF, 0xFF )
{
	switch( eType )
	{
	case TYPE_UNITINFOPANEL:
		{
			textureIDs[0] = 369;
			textureIDs[1] = 370;
			textureIDs[2] = 371;
			textureIDs[3] = 372;
			textureIDs[4] = 373;
			textureIDs[5] = 374;
			textureIDs[6] = 375;
			textureIDs[7] = 376;
			textureIDs[8] = 377;
			textureIDs[9] = 378;
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CImageNumber::Set( int _nValue )
{
	if ( nValue == _nValue )
		return;

	nValue = _nValue;
	imagesList.clear();

	if ( nValue < 0 )
		return;

	sRealSize.x = 0;
	sRealSize.y = 0;
	int nX = GetSize().x;
	int nTemp = nValue;
	do
	{
		int nNumber = nTemp % 10;
		nTemp = nTemp / 10;

		CPtr<NDb::CUITexture> pTexture = NDb::GetUITexture( textureIDs[nNumber] );
		CPtr<CImage> pImage = new CImage( SWindowInfo( this, SPoint( nX - pTexture->nWidth, 0 ), SPoint( pTexture->nWidth, pTexture->nHeight ), "number", STYLE_ENABLED | STYLE_VISIBLE | STYLE_TRANSPARENT ) );
		pImage->SetColor( sColor );
		pImage->SetImage( pTexture );
		imagesList.push_back( pImage.GetPtr() );

		nX -= pTexture->nWidth;
		sRealSize.x += pTexture->nWidth;
		sRealSize.y = Max( sRealSize.y, pTexture->nHeight );
	}	while( nTemp > 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CImageNumber::SetColor( const NGfx::SPixel8888 &_sColor )
{
	sColor = _sColor;

	for ( list<CObj<CImage> >::iterator iTemp = imagesList.begin(); iTemp != imagesList.end(); iTemp++ )
		(*iTemp)->SetColor( sColor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SPoint& CImageNumber::GetRealSize() const
{
	return sRealSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CImageNumber::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_LBUTTONDOWN:
			return true;
		case EVENT_LBUTTONUP:
			SendMessage( GetParent(), SEvent( EVENT_NOTIFY, GetWindowID() ) );
			return true;
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CHoverButton
////////////////////////////////////////////////////////////////////////////////////////////////////
CHoverButton::CHoverButton( const SWindowInfo &sInfo ):
	CButton( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CHoverButton::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( GetStyle( STYLE_ENABLED ) )
	{
		if ( IsMouseCover() )
			SetActiveState( STATE_HOVER );
		else
			SetActiveState( STATE_NORMAL );
	}
	else
		SetActiveState( STATE_DISABLED );

	CButton::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFlashButton
////////////////////////////////////////////////////////////////////////////////////////////////////
CFlashButton::CFlashButton( const SWindowInfo &sInfo ):
	CButton( sInfo ), sFlashTime( 0 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFlashButton::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pLight = GetUIWindow<CImage>( this, "light" );
			break;
		}
	}

	return CButton::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFlashButton::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !GetStyle( STYLE_ENABLED ) )
	{
		pLight->SetStyle( STYLE_VISIBLE, false );
		return;
	}

	pLight->SetStyle( STYLE_VISIBLE, true );

	float fCoeff = 1.0f;
	if ( !IsMouseCover() )
	{
		fCoeff = float( ( sTime - sFlashTime ) % ( N_FLASHBUTTON_FLASHTIME * 2 ) ) / N_FLASHBUTTON_FLASHTIME;
		if ( fCoeff > 1 )
			fCoeff = 2 - fCoeff;
	}
	else
		sFlashTime = sTime - N_FLASHBUTTON_FLASHTIME;

	pLight->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fCoeff ) );

	CButton::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitHead
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitHead::CUnitHead( const SWindowInfo &sInfo, NRender::IRenderGame *_pRender, float _fScale ):
	CWindow( sInfo ), pRenderGame( _pRender ), fScale( _fScale )
{
	CTransformStack ts;
	ts.Init();
	//ts.Push( CQuat( -FP_PI2, CVec3(0,0,1) ) );
	pTransform = new NGScene::CCFBTransform;
	pTransform->Set( ts.Get() );

	p3DView = NGScene::CreateNewFastInterfaceView();
	p3DView->SetAmbient( NDb::GetAmbientLight(7), NGScene::IGameView::LT_INVENTORY );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitHead::SetUnit( NWorld::CUnit *pUnit )
{
	pHead = NRender::CreateShowUnitHead( p3DView, pUnit, pRenderGame->GetHeadController(), pTransform );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitHead::SetSequence( NDb::CSequence *pSequence )
{
	ASSERT( IsValid( pHead ) );
	if ( !IsValid( pHead ) )
		return;

	pHead->SetSequence( pSequence );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitHead::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !IsValid( pHead ) )
		return;

	SPoint sSize( (float)GetSize().x * p3DView->GetScreenRect().x / 1024.0f, (float)GetSize().y * p3DView->GetScreenRect().y / 768.0f );

	SRect sWindow;
	SPoint sPosition;
	if ( !ClientToScreen( &sPosition, &sWindow ) )
		return;

	SRect s2DWindow( sWindow );
	SPoint s2DPosition( sPosition );
	VirtualToScreen( &s2DPosition, &s2DWindow );

	CRectLayout sLayout;
	sLayout.AddRect( 0, 0, CTRect<float>( 0, 0, sWindow.Width(), sWindow.Height() ) );
	pView->CreateDynamicClearRects( sLayout, s2DPosition, s2DWindow );
	pView->Flush();

	CVec2 vPos( sPosition.x + sWindow.Width() / 2, sPosition.y + sWindow.Height() / 2 );
	CTransformStack ts;
	SHMatrix sMatrix;
	MakeMatrix( &sMatrix, CVec3( 0, -5 / fScale, 0.048f ), QNULL, CVec3( 1, 1, 1 ) );
	ts.Init();
	ts.MakeProjective( CVec2( 1024, 768 ), 60, 0.1f, 1000 );
	ts.SetCamera( sMatrix );

	SHMatrix sShift;
	Identity( &sShift);
	sShift._14 = (float)( vPos.x - 512 ) / 512;
	sShift._24 = (float)( 384 - vPos.y ) / 384;

	SHMatrix sRes;
	Multiply( &sRes, sShift, ts.Get().forward );
	ts.Init( sRes );

	NGScene::IGameView::SDrawInfo drawInfo;
	drawInfo.pTS = &ts;
	drawInfo.vOrigin = CVec2( sPosition.x / 1024.0f, sPosition.y / 768.0f );
	drawInfo.vSize = CVec2( sWindow.Width() / 1024.0f, sWindow.Height() / 768.0f );
	drawInfo.bOverlay = true;
	p3DView->Draw( drawInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitView
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitView::CUnitView( const SWindowInfo &sInfo, NRender::IRenderGame *_pRenderGame ):
	CWindow( sInfo ), pRenderGame( _pRenderGame )
{
	p3DView = NGScene::CreateNewFastInterfaceView();
	p3DView->SetAmbient( NDb::GetAmbientLight(7), NGScene::IGameView::LT_INVENTORY );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitView::SetUnit( NRPG::CUnit *pUnit )
{
	pInventoryUnit = 0;
	if ( !IsValid( pUnit ) )
		return;

	pInventoryUnit = NRender::CreateShowUnit( p3DView, pUnit, sTimer.GetTime(), pRenderGame );


	const NDb::SCameraParams &sCameraParams = pUnit->GetPers()->sPortraitCamera;
	fYaw = sCameraParams.fYaw;
	fPitch = sCameraParams.fPitch;
	fDistance = sCameraParams.fDistance;
	vAnchor = sCameraParams.vAnchor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitView::SetUnit( NWorld::CUnit *pUnit )
{
	pInventoryUnit = 0;
	if ( !IsValid( pUnit ) )
		return;

	pInventoryUnit = NRender::CreateShowUnit( p3DView, pUnit, sTimer.GetTime(), pRenderGame );

	const NDb::SCameraParams &sCameraParams = pUnit->GetRPG()->GetRPGPers()->sPortraitCamera;
	fYaw = sCameraParams.fYaw;
	fPitch = sCameraParams.fPitch;
	fDistance = sCameraParams.fDistance;
	vAnchor = sCameraParams.vAnchor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitView::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	CWindow::Draw( sTime, pView );

	if ( !IsValid( pInventoryUnit ) )
		return;

	SRect sWindow;
	SPoint sPosition;
	if ( !ClientToScreen( &sPosition, &sWindow ) )
		return;

	sTimer.Advance( true, GetTickCount() );
	pInventoryUnit->Update( 0 );

	pView->Flush();

	CVec2 vPos( sPosition.x + GetSize().x / 2, sPosition.y + GetSize().y / 2 );

	CVec3 vForwardDir;
	CQuat q = CQuat( fYaw, V3_AXIS_Z ) * CQuat( fPitch, V3_AXIS_X );
	q.GetYAxis( &vForwardDir );

	CVec3 vCP( vAnchor - vForwardDir * fDistance );
	SHMatrix sCamera;
	MakeMatrix( &sCamera, fPitch, fYaw, vCP );

	CTransformStack ts;
	ts.Init();
	ts.MakeProjective( CVec2( 1024, 768 ), 60, 0.1f, 300 );
	ts.SetCamera( sCamera );

	SHMatrix sShift;
	Identity( &sShift);
	sShift._14 = (float)( vPos.x - 512 ) / 512;
	sShift._24 = (float)( 384 - vPos.y ) / 384;

	SHMatrix sRes;
	Multiply( &sRes, sShift, ts.Get().forward );
	ts.Init( sRes );

	NGScene::IGameView::SDrawInfo drawInfo;
	drawInfo.pTS = &ts;
	drawInfo.vOrigin = CVec2( sPosition.x / 1024.0f, sPosition.y / 768.0f );
	drawInfo.vSize = CVec2( sWindow.Width() / 1024.0f, sWindow.Height() / 768.0f );
	drawInfo.bOverlay = true;
	p3DView->Draw( drawInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CItemModel
////////////////////////////////////////////////////////////////////////////////////////////////////
CItemModel::CItemModel( const SWindowInfo &sInfo, NGame::IMission *pMission ):
	TBaseClass( sInfo, pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CItemModel::CanHandleState( NGame::IState *pState ) const
{
	if ( CDynamicCast<NGame::CStateUnloadItem> pItem( pState ) )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CItemModel::GetTarget() const
{
	return pItem;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CItemModel::Set( NRPG::IInventoryItem* _pItem, NDb::ECameraType eCameraType )
{
	pItem = _pItem;

	CPtr<NDb::CRPGItem> pRPGItem( pItem->GetDBItem() );

	SRand sRnd;
	if ( pRPGItem->pModel )
	{
		const NDb::SCameraParams &sCamera = pRPGItem->sCameras[eCameraType];

		CVec3 vForwardDir;
		CQuat q = CQuat( sCamera.fYaw, V3_AXIS_Z ) * CQuat( sCamera.fPitch, V3_AXIS_X );
		q.GetYAxis( &vForwardDir );

		CVec3 vCP( sCamera.vAnchor - vForwardDir * sCamera.fDistance );
		SFBTransform res;
		MakeMatrix( &res, sCamera.fPitch, sCamera.fYaw, vCP );

		SetModel( pRPGItem->pModel->CreateModel( &sRnd ) );
		SetTransform( new CFBTransform( res ) );
	}

	pToolTip = new CToolTip( SWindowInfo( GetInterface(), SPoint( 0, 0 ), SPoint( 0, 0 ), "", STYLE_ENABLED ) );
	SetToolTip( pToolTip );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CItemModel::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !IsValid( pItem ) )
	{
		SetStyle( STYLE_VISIBLE, false );
		return;
	}

	CPtr<NDb::CRPGItem> pRPGItem( pItem->GetDBItem() );

	pToolTip->SetVal( L"name", CStringToWString( pRPGItem->pName ) );
	pToolTip->SetVal( L"weight", pItem->GetWeight() );

	if ( CDynamicCast<NRPG::IWeaponItemInfo> pWeapon( pItem ) )
	{
		pToolTip->SetText( GetDBString( 4468 ) );

		CPtr<NDb::CRPGWeapon> pRPGWeapon( pWeapon->GetDBWeapon() );
		CPtr<NRPG::IClipItem> pClipItem( pWeapon->GetInnerClip() );

		pToolTip->SetVal( L"typename", CStringToWString( pRPGWeapon->pWeaponType->pName ) );

		int pShotModeNames[NDb::SM_MAXVALUE] = { 6068, 6069, 6070, 6071, 6072, 6073 };
		wstring wsShotModes;
		for ( int nTemp = 0; nTemp < NDb::SM_MAXVALUE; nTemp++ )
		{
			if ( !pRPGWeapon->shootModes[nTemp] )
				continue;

			WCHAR wsBuffer[256];
			swprintf( wsBuffer, L"%s: R.i.P", GetDBString( pShotModeNames[nTemp] ) );
			wsShotModes += wsBuffer;
		}
		pToolTip->SetVal( L"attackmodes", wsShotModes );

		NRPG::SWeaponInfo sInfo;
		pWeapon->GetInfo( &sInfo );

		pToolTip->SetVal( L"range", sInfo.nRange );
		pToolTip->SetVal( L"damagemin", sInfo.nDmgMin );
		pToolTip->SetVal( L"damagemax", sInfo.nDmgMax );

		pToolTip->SetVal( L"ammotype", CStringToWString( pClipItem->GetDBAmmo()->pName ) );
		pToolTip->SetVal( L"clipsize", pClipItem->GetMaxIncQuantity() );

		pToolTip->SetVal( L"convamateur", sInfo.fMinToHitCoeff );
		pToolTip->SetVal( L"convprofessional", sInfo.fMaxToHitCoeff );

		if ( pRPGWeapon->shootModes[NDb::SM_ShortBurst] )
		{
			WCHAR wsBuffer[256];
			swprintf( wsBuffer, L"<br>Short Burst: %d shots Stability: %d", sInfo.nRoF, sInfo.nRecoil );
			pToolTip->SetVal( L"burstinfo", sInfo.fMaxToHitCoeff );
		}
	}
	else if ( CDynamicCast<NRPG::IGrenadeItem> pClip( pItem ) )
	{
		pToolTip->SetText( GetDBString( 4469 ) );

		CPtr<NDb::CRPGGrenade> pRPGGrenade( pClip->GetDBGrenade() );

		pToolTip->SetVal( L"typename", CStringToWString( pRPGGrenade->pWeaponType->pName ) );
		pToolTip->SetVal( L"delay", pRPGGrenade->nMaxDelay );
	}
	else if ( CDynamicCast<NRPG::IMeleeWeaponItem> pMeleeWeapon( pItem ) )
	{
		pToolTip->SetText( GetDBString( 4512 ) );

		CPtr<NDb::CRPGMeleeWeapon> pRPGMeleeWeapon( pMeleeWeapon->GetDBMeleeWeapon() );

		pToolTip->SetVal( L"typename", CStringToWString( pRPGMeleeWeapon->pWeaponType->pName ) );
		pToolTip->SetVal( L"damagemin", pRPGMeleeWeapon->nDmgMin );
		pToolTip->SetVal( L"damagemax", pRPGMeleeWeapon->nDmgMax );
		pToolTip->SetVal( L"critbonus", pRPGMeleeWeapon->nCriticalBonus );
	}
	else if ( CDynamicCast<NRPG::IClipItem> pClip( pItem ) )
	{
		pToolTip->SetText( GetDBString( 4470 ) );

		CPtr<NDb::CRPGAmmo> pRPGAmmo( pClip->GetDBAmmo() );

		pToolTip->SetVal( L"ammotype", CStringToWString( pRPGAmmo->pName ) );
		pToolTip->SetVal( L"ammomax", pClip->GetMaxIncQuantity() );
		pToolTip->SetVal( L"currammo", pClip->GetIncQuantity() );
		pToolTip->SetVal( L"damagemin", pRPGAmmo->nDmgMin );
		pToolTip->SetVal( L"damagemax", pRPGAmmo->nDmgMax );
	}
	else if ( CDynamicCast<NRPG::IFirstAidItem> pFirstAid( pItem ) )
		pToolTip->SetText( GetDBString( 4511 ) );
	else
		pToolTip->SetText( GetDBString( 4690 ) );

	TBaseClass::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
CSlot::CSlot( const SWindowInfo &sInfo, NGame::IMission *_pMission, int _nWidth, int _nHeight, NDb::ECameraType _eCameraType, bool _bAlwaysHilight ):
	CWindow( sInfo ), pMission( _pMission ), nWidth( _nWidth ), nHeight( _nHeight ), eCameraType( _eCameraType ), bAlwaysHilight( _bAlwaysHilight ), bTrackMouse( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSlot::GetInSlotPos( int nX, int nY, SPoint *pCoords )
{
	SRect sWindow;
	SPoint sPosition;
	ClientToScreen( &sPosition, &sWindow );

	const SPoint &sSize = pSlotView->GetSize();
	SPoint sCellSize( sSize.x / nWidth, sSize.y / nHeight );

	SPoint sLocalSpaceCursor( nX - sPosition.x, nY - sPosition.y );

	pCoords->x = Min( nWidth, Max( 0, sLocalSpaceCursor.x / sCellSize.x ) );
	pCoords->y = Min( nHeight, Max( 0, sLocalSpaceCursor.y / sCellSize.y ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSlot::GetItemInSlotPos( int nX, int nY, const SPoint &sItemSize, SPoint *pCoords )
{
	SRect sWindow;
	SPoint sPosition;
	ClientToScreen( &sPosition, &sWindow );

	const SPoint &sSize = pSlotView->GetSize();
	SPoint sCellSize( sSize.x / nWidth, sSize.y / nHeight );

	SPoint sLocalSpaceCursor( nX - sPosition.x, nY - sPosition.y );

	pCoords->x = Min( nWidth - Min( nWidth, sItemSize.x ), Max( 0, Float2Int( ( float( sLocalSpaceCursor.x ) - float( sItemSize.x * sCellSize.x ) / 2 ) / sCellSize.x ) ) );
	pCoords->y = Min( nHeight - Min( nHeight, sItemSize.y ), Max( 0, Float2Int( ( float( sLocalSpaceCursor.y ) - float( sItemSize.y * sCellSize.y ) / 2 ) / sCellSize.y ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NGame::IMission* CSlot::GetGame()
{
	return pMission;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSlot::GetDragItem( NWorld::IPlayer::SItemInfo *pInfo )
{
	return pMission->GetActivePlayer()->GetPlayer()->GetInHandItem( pInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSlot::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_MOUSEMOVE:
		{
			sMousePoint = SPoint( sEvent.nX, sEvent.nY );
			GetInterface()->SetCursorInfo( SCursorInfo() );
			break;
		}
	case EVENT_MOUSEENTER:
		{
			bTrackMouse = true;
			break;
		}
	case EVENT_MOUSEEXIT:
		{
			bTrackMouse = false;
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pHilight = GetUIWindow<CWindow>( this, "hilight" );
			pSlotView = GetUIWindow<CWindow>( this, "view" );

			hilights.SetSizes( nWidth, nHeight );
			const SPoint &sSize = pSlotView->GetSize();
			for ( int nTempY = 0; nTempY < nHeight; nTempY++ )
			{
				for ( int nTempX = 0; nTempX < nWidth; nTempX++ )
				{
					SPoint sPoint( sSize.x * nTempX, sSize.y * nTempY );
					hilights[nTempY][nTempX].pImage = new CImage( SWindowInfo( pHilight, SPoint( sPoint.x / nWidth, sPoint.y / nHeight ), SPoint( sSize.x / nWidth , sSize.y / nHeight ), "", STYLE_ENABLED | STYLE_TRANSPARENT ) );
				}
			}
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSlot::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	const SPoint &sSize = pSlotView->GetSize();
	SPoint sCellSize( sSize.x / nWidth, sSize.y / nHeight );

	vector<SItem> newInventoryItemsSet;
	GetItemsList( &newInventoryItemsSet );

	vector<SItem> newItemsSet( newInventoryItemsSet.size() );
	for ( int nTemp = 0; nTemp < newInventoryItemsSet.size(); nTemp++ )
	{
		const SItem &sItem = newInventoryItemsSet[nTemp];
		for ( int nItem = 0; nItem < itemsSet.size(); nItem++ )
		{
			const SItem &sTestItem = itemsSet[nItem];
			if ( ( sTestItem.sPos != sItem.sPos ) || ( sTestItem.pItem != sItem.pItem ) )
				continue;

			newItemsSet[nTemp] = itemsSet[nItem];
			break;
		}

		if ( !IsValid( newItemsSet[nTemp].pItem ) )
		{
			newItemsSet[nTemp].sPos = sItem.sPos;
			newItemsSet[nTemp].pItem = sItem.pItem;

			const SPoint &sInventoryItemSize = sItem.pItem->GetSize();
			SPoint sItemCellSize( sCellSize.x * Min( sInventoryItemSize.x, nWidth ), sCellSize.y * Min( sInventoryItemSize.y, nHeight ) );
			SPoint sItemSize( sItemCellSize.x - 1, sItemCellSize.y - 1 );

			CPtr<NDb::CRPGItem> pRPGItem( sItem.pItem->GetDBItem() );
			SPoint sShift( 1 + ( sItemCellSize.x - sItemSize.x ) / 2, 1 + ( sItemCellSize.y - sItemSize.y ) / 2 );
			SPoint sPosition( sItem.sPos.x * sCellSize.x + sShift.x, sItem.sPos.y * sCellSize.y + sShift.y );
			CPtr<CItemModel> pItemModel = new CItemModel( SWindowInfo( pSlotView, sPosition, sItemSize, "icon", STYLE_ENABLED | STYLE_VISIBLE | STYLE_BOTTOMMOST ), pMission );
			pItemModel->Set( sItem.pItem, eCameraType );
			newItemsSet[nTemp].pModel = pItemModel;
//			newItemsSet[nTemp].pImage = IImage::Create( pView, SRect( sPosition.x, sPosition.y, sPosition.x + sItemSize.x, sPosition.y + sItemSize.y ), "icon", STYLE_ENABLED | STYLE_VISIBLE );
//			newItemsSet[nTemp].pImage->SetImage( pTexture );
//			newItemsSet[nTemp].pImage->SetColor( NGfx::SPixel8888( 0xFF, 0, 0, 0xFF ) );

		}
	}

	itemsSet = newItemsSet;

	bool bCanTransfer = true;
	NWorld::IPlayer::SItemInfo sInfo;
	if ( GetDragItem( &sInfo ) )
		bCanTransfer = CanTransferFrom( sInfo.pUnit );

	for ( int nTempY = 0; nTempY < nHeight; nTempY++ )
	{
		for ( int nTempX = 0; nTempX < nWidth; nTempX++ )
		{
			SHilight &sHilight = hilights[nTempY][nTempX];

			sHilight.nID = -1;
			sHilight.pImage->SetStyle( STYLE_VISIBLE, false );
		}
	}

	for ( int nTemp = 0; nTemp < itemsSet.size(); nTemp++ )
	{
		const SItem &sItem = itemsSet[nTemp];
		const SPoint &sPos = sItem.sPos;
		SPoint sSize = sItem.pItem->GetSize();
		sSize = SPoint( min( nWidth, sPos.x + sSize.x ) - sPos.x, min( nHeight, sPos.y + sSize.y ) - sPos.y );

		for ( int nTempY = 0; nTempY < sSize.y; nTempY++ )
		{
			for ( int nTempX = 0; nTempX < sSize.x; nTempX++ )
			{
				SHilight &sHilight = hilights[sPos.y + nTempY][sPos.x + nTempX];
				sHilight.nID = nTemp;
				sHilight.pImage->SetColor( NGfx::SPixel8888( 0x1F, 0x1F, 0xFF, 0x2F ) );
				sHilight.pImage->SetStyle( STYLE_VISIBLE, bAlwaysHilight );
			}
		}
	}

	NWorld::IPlayer::SItemInfo sItemInfo;
	if ( bTrackMouse && GetDragItem( &sItemInfo ) )
	{
		SPoint sSize = sItemInfo.pItem->GetSize();
		SPoint sPos;
		GetItemInSlotPos( sMousePoint.x, sMousePoint.y, sSize, &sPos );
		sSize = SPoint( min( nWidth, sPos.x + sSize.x ) - sPos.x, min( nHeight, sPos.y + sSize.y ) - sPos.y );

		int nLastID = -1;
		bool bReplace = false;
		bool bCanReplace = true;
		for ( int nTempY = 0; nTempY < sSize.y; nTempY++ )
		{
			for ( int nTempX = 0; nTempX < sSize.x; nTempX++ )
			{
				SHilight &sHilight = hilights[sPos.y + nTempY][sPos.x + nTempX];
				if ( sHilight.nID == -1 )
					continue;

				if ( ( nLastID != -1 ) && ( sHilight.nID != nLastID ) )
				{
					bCanReplace = false;
					break;
				}

				nLastID = sHilight.nID;
				if ( sHilight.nID != -1 )
					bReplace = true;
			}
		}

		for ( int nTempY = 0; nTempY < sSize.y; nTempY++ )
		{
			for ( int nTempX = 0; nTempX < sSize.x; nTempX++ )
			{
				SHilight &sHilight = hilights[sPos.y + nTempY][sPos.x + nTempX];

				if ( !bCanTransfer )
				{
					sHilight.pImage->SetColor( NGfx::SPixel8888( 0xFF, 0x1F, 0x1F, 0x2F ) );
					sHilight.pImage->SetStyle( STYLE_VISIBLE, true );
					continue;
				}

				if ( bReplace )
				{
					if ( bCanReplace )
					{
						sHilight.pImage->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0x2F ) );
						sHilight.pImage->SetStyle( STYLE_VISIBLE, true );
					}
					else
					{
						sHilight.pImage->SetColor( NGfx::SPixel8888( 0xFF, 0x1F, 0x1F, 0x2F ) );
						sHilight.pImage->SetStyle( STYLE_VISIBLE, true );
					}
				}
				else
				{
					sHilight.pImage->SetColor( NGfx::SPixel8888( 0x1F, 0xFF, 0x1F, 0x2F ) );
					sHilight.pImage->SetStyle( STYLE_VISIBLE, true );
				}
			}
		}
	}

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB2243951, CLineBar );
REGISTER_SAVELOAD_CLASS( 0xB2243952, CUnitHead );
REGISTER_SAVELOAD_CLASS( 0xB2243953, CImageNumber );
REGISTER_SAVELOAD_CLASS( 0xB2243954, CItemModel );
REGISTER_SAVELOAD_CLASS( 0xB2243955, CFlashButton );
REGISTER_SAVELOAD_CLASS( 0xB2243956, CHoverButton );
REGISTER_SAVELOAD_CLASS( 0xB2243957, CUnitView );
