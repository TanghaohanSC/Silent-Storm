#include "StdAfx.h"
#include "GView.h"
#include "G2DView.h"
#include "Transform.h"
#include "wInterface.h"
#include "RPGItemInfo.h"
#include "..\Misc\StrProc.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataAck.h"
#include "..\DBFormat\DataInterface.h"
#include "..\DBFormat\DataRPG.h"
#include "Sound.h"
#include "iMission.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iMissionUI.h"
#include "iLogPanel.h"
#include "iTopPanel.h"
#include "iUnitPanel.h"
#include "iInventoryPanel.h"
#include "iCharacterPanel.h"
#include "iActionDecorator.h"
#include "UIWrap.h"
#include "iUnitIconBar.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_ASK_TTL = 3000;
const int
	N_BASELEVEL = 3,
	N_MAXLEVELS_COUNT = 8;
const int
	N_LOGPANEL_PAD = 20; // pad-zone in points
const int
	N_SCROLL_STEP				= 4,
	N_SCROLL_GUARDBAND	= 4;
const int
	N_HITPTRACKER_TTL		= 2000;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAckIcon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAckIcon: public CWindow
{
	OBJECT_NOCOPY_METHODS(CAckIcon);
private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	bool bActive;
	STime sTime;
	CPtr<CText> pText;
	CPtr<CUnitHead> pHead;
	CPtr<NWorld::CAckEvent> pEvent;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&bActive); f.Add(4,&sTime); f.Add(5,&pText); f.Add(6,&pHead); f.Add(7,&pEvent); return 0; }
	CObj<NSound::ISound2D> pSound;

public:
	CAckIcon() {}
	CAckIcon( const SWindowInfo &sInfo, NGame::IMission *_pMission ): CWindow( sInfo ), pMission( _pMission ) {}

	void Set( NWorld::CAckEvent *pEvent );

	void Activate();
	void Deactivate();
	const STime& GetEndTime();

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
inline wstring ConvertLineBreaks( const wstring &szStr )
{
	wstring szRet;
	for ( wstring::const_iterator i = szStr.begin(); i != szStr.end(); )
	{
		switch ( wchar_t(*i) )
		{
			case L'\n':
				szRet += L"<br>";
				break;
			case L'\r':
				szRet += L"<br>";
				++i;
				if ( i != szStr.end() && *i == L'\n' )
					++i;
				continue;
			case 133: // symbol L'…'
				szRet += L"...";
				break;
			default:
				szRet += *i;
				break;
		}
		++i;
	}
	return szRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAckIcon::Set( NWorld::CAckEvent *_pEvent )
{
	pEvent = _pEvent;

	if ( pEvent->pUnit && pEvent->pAckInfo )
	{
		pHead->SetUnit( pEvent->pUnit );
		if ( pEvent->pAckInfo->pSequence )
			pHead->SetSequence( pEvent->pAckInfo->pSequence );
	}
	if ( pEvent->pAckInfo && pEvent->pAckInfo->pText )
		pText->SetText( ConvertLineBreaks( pEvent->pAckInfo->pText->szStr ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAckIcon::Activate()
{
	bActive = true;
	sTime = GetTickCount() + N_ASK_TTL;

	if ( pEvent->pAckInfo && pEvent->pAckInfo->pSound )
		pSound = pMission->GetSoundScene()->Add2DSound( pEvent->pAckInfo->pSound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAckIcon::Deactivate()
{
	bActive = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const STime& CAckIcon::GetEndTime()
{
	return sTime;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAckIcon::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_LBUTTONDOWN:
			return true;
		case EVENT_LBUTTONUP:
		{
			if ( bActive )
				sTime = GetTickCount();

			return true;
		}
		case EVENT_MOUSEMOVE:
		{
			GetInterface()->SetCursorInfo( SCursorInfo() );
			break;
		}
		case EVENT_TEMPLATELOAD:
		{
			pHead = new CUnitHead( sEvent.pLoader->GetControl( "face" ), pMission->GetRenderGame(), 1.8f );
			break;
		}
		case EVENT_TEMPLATELOADCOMPLETE:
		{
			pText = GetUIWindow<CText>( this, "text" );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLevelSwitchBar
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLevelSwitchBar: public CWindow
{
	OBJECT_BASIC_METHODS(CLevelSwitchBar)
private:
	enum
	{
		STATE_VISIBLE,
		STATE_HIDDEN
	};
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	CPtr<CButton> pUp;
	CPtr<CButton> pDown;
	vector<CPtr<CButton> > buttonsSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pUp); f.Add(4,&pDown); f.Add(5,&buttonsSet); return 0; }

public:
	CLevelSwitchBar() {}
	CLevelSwitchBar( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CLevelSwitchBar::CLevelSwitchBar( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission ), buttonsSet( N_MAXLEVELS_COUNT )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CLevelSwitchBar::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			int nSelectedFloor = -1;
			if ( sEvent.szID.compare( "up" ) == 0 )
				nSelectedFloor = pMission->GetScene()->GetCutFloor() + 1 + N_BASELEVEL;
			else if ( sEvent.szID.compare( "down" ) == 0  )
				nSelectedFloor = pMission->GetScene()->GetCutFloor() - 1 + N_BASELEVEL;
			else if ( sEvent.szID.compare( "level_1" ) == 0  )
				nSelectedFloor = 0;
			else if ( sEvent.szID.compare( "level_2" ) == 0  )
				nSelectedFloor = 1;
			else if ( sEvent.szID.compare( "level_3" ) == 0  )
				nSelectedFloor = 2;
			else if ( sEvent.szID.compare( "level_4" ) == 0  )
				nSelectedFloor = 3;
			else if ( sEvent.szID.compare( "level_5" ) == 0  )
				nSelectedFloor = 4;
			else if ( sEvent.szID.compare( "level_6" ) == 0  )
				nSelectedFloor = 5;
			else if ( sEvent.szID.compare( "level_7" ) == 0  )
				nSelectedFloor = 6;
			else if ( sEvent.szID.compare( "level_8" ) == 0  )
				nSelectedFloor = 7;

			if ( nSelectedFloor != -1 )
			{
				pMission->GetScene()->SetCutFloor( nSelectedFloor - N_BASELEVEL );
				return true;
			}

			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pUp = GetUIWindow<CButton>( this, "up" );
			pUp->AddImageState( 0, NDb::GetUITexture( 399 ) );
			pDown = GetUIWindow<CButton>( this, "down" );
			pDown->AddImageState( 0, NDb::GetUITexture( 398 ) );

			for ( int nTemp = 0; nTemp < buttonsSet.size(); nTemp++ )
			{
				buttonsSet[nTemp] = GetUIWindow<CButton>( this, NStr::Format( "level_%d", ( nTemp + 1 ) ) );
				if ( nTemp < N_BASELEVEL )
				{
					buttonsSet[nTemp]->AddImageState( STATE_VISIBLE, NDb::GetUITexture( 405 ) );
					buttonsSet[nTemp]->AddImageState( STATE_HIDDEN, NDb::GetUITexture( 407 ) );
				}
				else
				{
					buttonsSet[nTemp]->AddImageState( STATE_VISIBLE, NDb::GetUITexture( 404 ) );
					buttonsSet[nTemp]->AddImageState( STATE_HIDDEN, NDb::GetUITexture( 406 ) );
				}
			}

			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLevelSwitchBar::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	int nSelectedFloor = pMission->GetScene()->GetCutFloor();

	for ( int nTemp = 0; nTemp < buttonsSet.size(); nTemp++ )
	{
		if ( nTemp - N_BASELEVEL <= nSelectedFloor )
			buttonsSet[nTemp]->SetActiveState( STATE_VISIBLE );
		else
			buttonsSet[nTemp]->SetActiveState( STATE_HIDDEN );
	}

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitPanel
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMissionUnitPanel: public CUnitPanel
{
	OBJECT_BASIC_METHODS(CMissionUnitPanel)
private:
	enum
	{
		ACTION_STATE_BEGINCOMBAT,
		ACTION_STATE_ENDOFTURN
	};
	ZDATA_(CUnitPanel)
	CPtr<CMissionUI> pInterface;
	CPtr<NGame::IMission> pMission;
	////
	CPtr<CButton> pEndOfTurn;
	CPtr<CButton> pStartOfTurn;
	CObj<CUnitIconsBar> pUnitIconsBar;
	CObj<CLevelSwitchBar> pLevelSwitchBar;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUnitPanel*)this); f.Add(2,&pInterface); f.Add(3,&pMission); f.Add(4,&pEndOfTurn); f.Add(5,&pStartOfTurn); f.Add(6,&pUnitIconsBar); f.Add(7,&pLevelSwitchBar); return 0; }

public:
	CMissionUnitPanel() {}
	CMissionUnitPanel( const SWindowInfo &sInfo, NGame::IMission *pMission, CMissionUI *pInterface );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CMissionUnitPanel::CMissionUnitPanel( const SWindowInfo &sInfo, NGame::IMission *_pMission, CMissionUI *_pInterface ):
	CUnitPanel( sInfo, _pMission ), pMission( _pMission ), pInterface( _pInterface )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMissionUnitPanel::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pUnitIconsBar = new CUnitIconsBar( sEvent.pLoader->GetControl( "uniticonbar" ), pMission );
			pLevelSwitchBar = new CLevelSwitchBar( sEvent.pLoader->GetControl( "levelswitch" ), pMission );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pEndOfTurn = GetUIWindow<CButton>( this, "endofturn" );
			pEndOfTurn->AddImageState( 0, NDb::GetUITexture( 379 ) );

			pStartOfTurn = GetUIWindow<CButton>( this, "startofturn" );
			pStartOfTurn->AddImageState( 0, NDb::GetUITexture( 469 ) );
			break;
		}
	}

	return CUnitPanel::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUnitPanel::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	pEndOfTurn->SetStyle( STYLE_VISIBLE, !pMission->IsRealTime() );
	pStartOfTurn->SetStyle( STYLE_VISIBLE, pMission->IsRealTime() );

	CUnitPanel::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CItemText
////////////////////////////////////////////////////////////////////////////////////////////////////
class CItemText: public CActionDecorator<CImage>
{
	OBJECT_BASIC_METHODS(CItemText)
private:
	ZDATA_(TBaseClass)
	CPtr<CMissionUI> pMissionUI;
	CPtr<NGame::IMission> pMission;
	////
	NWorld::SItem sItem;
	CObj<CTextDraw> pText;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&pMissionUI); f.Add(3,&pMission); f.Add(4,&sItem); f.Add(5,&pText); return 0; }

public:
	CItemText() {}
	CItemText( const SWindowInfo &sInfo, NGame::IMission *pMission, CMissionUI *pMissionUI, const NWorld::SItem &sItem );

	bool CanHandleState( NGame::IState *pState ) const;
	CObjectBase* GetTarget() const;

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CItemText::CItemText( const SWindowInfo &sInfo, NGame::IMission *_pMission, CMissionUI *_pMissionUI, const NWorld::SItem &_sItem ):
	TBaseClass( sInfo, _pMission ), pMission( _pMission ), pMissionUI( _pMissionUI ), sItem( _sItem )
{
	wstring wsText( L"<color=black>[UNKNOWN]" );
	if ( sItem.pItem->GetDBItem()->pName )
		wsText = L"<color=black>" + sItem.pItem->GetDBItem()->pName->szStr;

	pText = new CTextDraw( SPoint( 0, 0 ), SPoint( -1, -1 ), wsText );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CItemText::CanHandleState( NGame::IState *pState ) const
{
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CItemText::GetTarget() const
{
	return sItem.pWorldItem;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CItemText::ProcessMessage( const SEvent &sEvent )
{
	switch ( sEvent.nEvent )
	{
	case EVENT_MOUSEENTER:
		{
			SetColor( NGfx::SPixel8888( 0x0, 0x0, 0xFF, 0xFF ) );
			break;
		}
	case EVENT_MOUSEEXIT:
		{
			SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF ) );
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CItemText::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	SetSize( pText->GetSize( pView ) );

	TBaseClass::Draw( sTime, pView );

	pText->Draw( this, sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CItemText
////////////////////////////////////////////////////////////////////////////////////////////////////
class CEnemyIcon: public CActionDecorator<CImage>
{
	OBJECT_BASIC_METHODS(CEnemyIcon)
private:
	ZDATA_(TBaseClass)
	CPtr<CMissionUI> pMissionUI;
	CPtr<NGame::IMission> pMission;
	////
	bool bOwner;
	float fAngle;
	CPtr<CImage> pImage;
	CPtr<NWorld::CUnit> pEnemy;
	CDBPtr<NDb::CUITexture> pTexture;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&pMissionUI); f.Add(3,&pMission); f.Add(4,&bOwner); f.Add(5,&fAngle); f.Add(6,&pImage); f.Add(7,&pEnemy); f.Add(8,&pTexture); return 0; }

public:
	CEnemyIcon() {}
	CEnemyIcon( const SWindowInfo &sInfo, NGame::IMission *pMission, CMissionUI *pMissionUI );

	bool CanHandleState( NGame::IState *pState ) const;
	CObjectBase* GetTarget() const;

	void SetPosition( const SPoint &_sPosition );

	void Set( NWorld::CUnit *pEnemy, bool bOwner, float _fAngle );
	NWorld::CUnit* GetUnit() const;

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CEnemyIcon::CEnemyIcon( const SWindowInfo &sInfo, NGame::IMission *_pMission, CMissionUI *_pMissionUI ):
	TBaseClass( sInfo, _pMission ), pMission( _pMission ), pMissionUI( _pMissionUI )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CEnemyIcon::CanHandleState( NGame::IState *pState ) const
{
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CEnemyIcon::GetTarget() const
{
	return pEnemy;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CEnemyIcon::SetPosition( const SPoint &_sPosition )
{
	TBaseClass::SetSize( SPoint( 0, 0 ) );
	TBaseClass::SetPosition( _sPosition );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CEnemyIcon::Set( NWorld::CUnit *_pEnemy, bool _bOwner, float _fAngle )
{
	int pNormalIcons[8] = { 446, 447, 448, 449, 450, 451, 452, 453 };
	int pDisabledIcons[8] = { 454, 455, 456, 457, 458, 459, 460, 461 };

	fAngle = _fAngle;
	bOwner = _bOwner;
	pEnemy = _pEnemy;

	int nID = min( max( Float2Int( fAngle / 45 ), 0 ), 7 );

	if ( bOwner )
	{
		if ( fAngle == -1 )
			pTexture = NDb::GetUITexture( 464 );
		else
			pTexture = NDb::GetUITexture( pNormalIcons[nID] );
	}
	else
	{
		if ( fAngle == -1 )
			pTexture = NDb::GetUITexture( 463 );
		else
			pTexture = NDb::GetUITexture( pDisabledIcons[nID] );
	}

	SetImage( pTexture );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CEnemyIcon::ProcessMessage( const SEvent &sEvent )
{
	switch ( sEvent.nEvent )
	{
	case EVENT_RBUTTONDOWN:
		return true;
	case EVENT_RBUTTONUP:
		pMission->FocusCameraOnUnit( pEnemy );
		return true;
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CEnemyIcon::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	SPoint sNewSize( pTexture->nWidth, pTexture->nHeight );
	SPoint sSize = GetSize();
	SPoint sPosition = GetPosition();

	const SPoint &sParentSize = pMissionUI->GetClientWindow()->GetSize();
	const SPoint &sParentPosition = pMissionUI->GetClientWindow()->GetPosition();
	SRect sViewRect( sParentPosition.x, sParentPosition.y, sParentPosition.x + sParentSize.x, sParentPosition.y + sParentSize.y );
	sPosition.x = min( max( sViewRect.x1 + sNewSize.x / 2, sPosition.x ), sViewRect.x2 - sNewSize.x / 2 );
	sPosition.y = min( max( sViewRect.y1 + sNewSize.y / 2, sPosition.y ), sViewRect.y2 - sNewSize.y / 2 );

	TBaseClass::SetSize( sNewSize );
	TBaseClass::SetPosition( SPoint( sPosition.x + sSize.x / 2 - sNewSize.x / 2, sPosition.y + sSize.y / 2 - sNewSize.y / 2 ) );

	TBaseClass::Draw( sTime, pView );
}
//////////////////////////////////////////////////////////////////////////////////////
class CHitTracker: public CText
{
	OBJECT_BASIC_METHODS(CHitTracker);
private:
	ZDATA_(CText)
	CPtr<CWindow> pClientWindow;
	CPtr<NGame::IMission> pMission;
	////
	int nHitValue;
	bool bComplete;
	CVec3 vBegPoint;
	STime sBegTime;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CText*)this); f.Add(2,&pClientWindow); f.Add(3,&pMission); f.Add(4,&nHitValue); f.Add(5,&bComplete); f.Add(6,&vBegPoint); f.Add(7,&sBegTime); return 0; }

public:
	CHitTracker() {}
	CHitTracker( const SWindowInfo &sInfo, NGame::IMission *pMission, NWorld::CHitLocator *pLocator, const STime &sTime );

	bool IsComplete() const;

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CHitTracker::CHitTracker( const SWindowInfo &sInfo, NGame::IMission *_pMission, NWorld::CHitLocator *pLocator, const STime &sTime ):
	CText( sInfo ), pMission( _pMission ), sBegTime( sTime ), nHitValue( pLocator->nHitValue ), vBegPoint( pLocator->vPosition ), bComplete( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CHitTracker::IsComplete() const
{
	return bComplete;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CHitTracker::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( bComplete )
		return;

	bComplete = true;

	CVec2 vScreenPoint;
	CVec2 vScreenRect = pMission->GetScene()->GetScreenRect();
	CTransformStack sTS = pMission->GetCameraTransform();
	if ( !TestRayInFrustrum( vBegPoint, &sTS, vScreenRect, &vScreenPoint ) )
		return;

	vScreenPoint.x = vScreenPoint.x * 1024 / vScreenRect.x;
	vScreenPoint.y = vScreenPoint.y * 768 / vScreenRect.y;

	SPoint sPosition;
	GetParent()->ScreenToClient( SPoint( vScreenPoint.x, vScreenPoint.y ), &sPosition );

	float fWeight = float( sTime - sBegTime ) / N_HITPTRACKER_TTL;
	if ( fWeight > 1 )
		return;

	sPosition.x += 40 * fWeight;
	sPosition.y += -40 * fWeight;

	WCHAR wsText[256];
	int nAlpha = ( 1 - fWeight ) * 0xFF;
	swprintf( wsText, L"<color=%.2xff0000>%d", nAlpha, nHitValue );

	SetPosition( sPosition );
	SetText( wsText );

	bComplete = false;

	CText::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMissionUI
////////////////////////////////////////////////////////////////////////////////////////////////////
CMissionUI::CMissionUI():
	bindCancel( "cancel" ),	bindShowItems( "showitems" ), bindInventory( "inventory" ), bindCharacter( "character" ),
	bindPoseSubMenu( "submenu_poseselect" ), bindWeaponModeSubMenu( "submenu_weaponmode" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CMissionUI::CMissionUI( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), bindCancel( "cancel" ),	bindShowItems( "showitems" ), bindInventory( "inventory" ), bindCharacter( "character" ), 
	bindPoseSubMenu( "submenu_poseselect" ), bindWeaponModeSubMenu( "submenu_weaponmode" ),
	pMission( _pMission ), nAckSequenceCounter( -1 ), sCameraScrollUpdate( 0 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CWindow* CMissionUI::GetClientWindow() const
{
	return pClientWindow;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SInterfaceState& CMissionUI::GetInterfaceState() const
{
	return sInterfaceState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUI::SetInterfaceState( const SInterfaceState &_sInterfaceState )
{
	sInterfaceState = _sInterfaceState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMissionUI::ProcessEvent( const NInput::SEvent &sEvent )
{
	if ( bindCancel.ProcessEvent( sEvent ) )
	{
		if ( ( sInterfaceState.eIconsSet == SInterfaceState::IS_POSES ) || ( sInterfaceState.eIconsSet == SInterfaceState::IS_WEAPONMODES ) )
		{
			sInterfaceState.eIconsSet = SInterfaceState::IS_MAIN;
			return true;
		}
		else if ( sInterfaceState.bInventory || sInterfaceState.bCharacter )
		{
			sInterfaceState.bCharacter = false;
			sInterfaceState.bInventory = false;
			return true;
		}
	}

	if ( bindInventory.ProcessEvent( sEvent ) )
	{
		if ( pMission->CountSelected() )
			sInterfaceState.bInventory = !sInterfaceState.bInventory;
		return true;
	}
	else if ( bindCharacter.ProcessEvent( sEvent ) )
	{
		if ( pMission->CountSelected() )
			sInterfaceState.bCharacter = !sInterfaceState.bCharacter;
		return true;
	}

	if ( bindPoseSubMenu.ProcessEvent( sEvent ) )
		sInterfaceState.eIconsSet = SInterfaceState::IS_POSES;
	else if ( bindWeaponModeSubMenu.ProcessEvent( sEvent ) )
		sInterfaceState.eIconsSet = SInterfaceState::IS_WEAPONMODES;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMissionUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_MOUSEMOVE:
		{
			GetInterface()->SetCursorInfo( pMission->GetState()->GetCursorInfo() );
			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pLogPanel = new CLogPanel( SWindowInfo( this, SPoint( 0, 0 ), SPoint( 0, 0 ), "logpanel", STYLE_ENABLED | STYLE_VISIBLE | STYLE_TOPMOST | STYLE_TRANSPARENT ), STREAM_GAME );

			pTopBar = new CTopBar( sEvent.pLoader->GetControl( "topbar" ), pMission );
			pUnitPanel = new CMissionUnitPanel( sEvent.pLoader->GetControl( "unitpanel" ), pMission, this );
			pInventoryPanel = new CInventoryPanel( sEvent.pLoader->GetControl( "inventorypanel" ), pMission );
			pCharacterPanel = new CCharacterPanel( sEvent.pLoader->GetControl( "characterpanel" ), pMission );

			ackIconsSet.resize( NDb::N_ACKINFO_MAX_COUNT );
			for( int nTemp = 0; nTemp < NDb::N_ACKINFO_MAX_COUNT; nTemp++ )
			{
				ackIconsSet[nTemp] = new CAckIcon( sEvent.pLoader->GetControl( NStr::Format( "ack_%d", nTemp ) ), pMission );
				ackIconsSet[nTemp]->SetStyle( STYLE_VISIBLE, false );
			}
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pPause = GetUIWindow<CImage>( this, "pause" );
			pClientWindow = GetUIWindow<CWindow>( this, "view" );

			pInventory = GetUIWindow<CButton>( this, "inventory" );
			pInventory->AddImageState( 0, NDb::GetUITexture( 395 ) );
			pInventory->SetCursorInfo( GetInterface()->GetDefaultCursorInfo() );
			pCharacter = GetUIWindow<CButton>( this, "character" );
			pCharacter->AddImageState( 0, NDb::GetUITexture( 383 ) );
			pCharacter->SetCursorInfo( GetInterface()->GetDefaultCursorInfo() );

			break;
		}
	}

	bool bRet = CWindow::ProcessMessage( sEvent );

	/// CRAP
	switch( sEvent.nEvent )
	{
	case EVENT_MOUSEMOVE:
		{
			CPtr<NGame::IState> pState = pMission->GetState();
			if ( ( pState->GetType() == NGame::IState::FORCED ) || ( pState->GetType() == NGame::IState::TEMPORARY ) )
			{
				SCursorInfo sStateCursor = pState->GetCursorInfo();
				if ( sStateCursor.pTexture != GetInterface()->GetCursorInfo().pTexture )
				{
					sStateCursor.wsText = L"";
					GetInterface()->SetCursorInfo( sStateCursor );
				}
			}
			break;
		}
	}

	if ( bRet )
		return true;

	return pMission->GetState()->ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUI::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( pMission->CountSelected() != 1 )
	{
		sInterfaceState.bInventory = false;
		sInterfaceState.bCharacter = false;
	}

	const SPoint &sSize = pClientWindow->GetSize();
	const SPoint &sPosition = pClientWindow->GetPosition();
	SRect sClientRect( 0, 0, 1024, 768 );
	if ( !pMission->IsInterfaceHidden() )
		sClientRect = SRect( 0, 32, 1024, 596 );
	if ( sInterfaceState.bCharacter )
		sClientRect.x1 = 512;
	if ( sInterfaceState.bInventory )
		sClientRect.x2 = 512;

	pClientWindow->SetSize( SPoint( sClientRect.Width(), sClientRect.Height() ) );
	pClientWindow->SetPosition( SPoint( sClientRect.x1, sClientRect.y1 ) );

	pPause->SetStyle( STYLE_VISIBLE, pMission->IsGamePaused() );

	pInventory->SetStyle( STYLE_VISIBLE, !sInterfaceState.bInventory );
	pInventoryPanel->SetStyle( STYLE_VISIBLE, sInterfaceState.bInventory );
	pCharacter->SetStyle( STYLE_VISIBLE, !sInterfaceState.bCharacter );
	pCharacterPanel->SetStyle( STYLE_VISIBLE, sInterfaceState.bCharacter );

	SRect sScrWindow;
	SPoint sScrPosition;
	const SPoint &sScrSize = pClientWindow->GetSize();
	pClientWindow->ClientToScreen( &sScrPosition, &sScrWindow );
	SRect sScrClientRect( sScrPosition.x, sScrPosition.y, sScrPosition.x + sScrSize.x, sScrPosition.y + sScrSize.y );
	pMission->GetCamera()->SetScreenRect( CTRect<float>( sScrClientRect.x1 / 1024.0f, sScrClientRect.y1 / 768.0f, sScrClientRect.x2 / 1024.0f, sScrClientRect.y2 / 768.0f ) );

	SRect sLogRect( sClientRect );
	sLogRect.x1 += N_LOGPANEL_PAD;
	sLogRect.y1 += N_LOGPANEL_PAD;
	sLogRect.x2 -= N_LOGPANEL_PAD;
	sLogRect.y2 -= N_LOGPANEL_PAD;
	sLogRect.y2 = sLogRect.y1 + sLogRect.Height() / 2;
	pLogPanel->SetSize( SPoint( sLogRect.Width(), sLogRect.Height() ) );
	pLogPanel->SetPosition( SPoint( sLogRect.x1, sLogRect.y1 ) );

	CWindow::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUI::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	UpdateAck();
	UpdateHits( sTime );
	UpdateItems();
	UpdateEnemies();
	UpdateCameraScroll( sTime );

	CVec2 vScreenRect = pView->GetViewportSize();
	CTransformStack sTS = pMission->GetCameraTransform();
	for( hash_map<DWORD, SItem>::iterator iTemp = itemsMap.begin(); iTemp != itemsMap.end(); iTemp++ )
	{
		CVec2 vRes;
		if ( !TestRayInFrustrum( iTemp->second.sItem.pWorldItem->GetPos(), &sTS, vScreenRect, &vRes ) )
		{
			iTemp->second.pText->SetStyle( STYLE_VISIBLE, false );
			continue;
		}

		vRes.x = vRes.x * 1024 / vScreenRect.x;
		vRes.y = vRes.y * 768 / vScreenRect.y;

		iTemp->second.pText->SetStyle( STYLE_VISIBLE, true );
		iTemp->second.pText->SetPosition( SPoint( vRes.x, vRes.y ) );
	}

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUI::UpdateAck()
{
	int nPriority = -1;
	if ( ackEventsSet.size() > 0 )
	{
		if ( ( nAckSequenceCounter >= 0 ) && ( nAckSequenceCounter < ackEventsSet.size() ) )
			nPriority = ackEventsSet[nAckSequenceCounter]->nPriority;
		else
			nPriority = ackEventsSet[0]->nPriority;
	}

	vector<CPtr<NWorld::CAckEvent> > newAckEventsSet;
	while( pMission->GetWorld()->GetAcknowledgement( pMission->GetActivePlayer()->GetPlayer(), &newAckEventsSet ) )
	{
		if ( newAckEventsSet.empty() )
			continue;

		if ( ( newAckEventsSet.size() > 0 ) && ( newAckEventsSet[0]->nPriority > nPriority ) )
		{
			if ( nAckSequenceCounter > 0 )
			{
				ackIconsSet[nAckSequenceCounter]->Deactivate();

				for( int nTemp = 0; nTemp < NDb::N_ACKINFO_MAX_COUNT; nTemp++ )
					ackIconsSet[nTemp]->SetStyle( STYLE_VISIBLE, false );
			}

			nAckSequenceCounter = -1;
			nPriority = newAckEventsSet[0]->nPriority;
			ackEventsSet = newAckEventsSet;
		}
	}

	if ( nAckSequenceCounter >= 0 )
	{
		if ( ackIconsSet[nAckSequenceCounter]->GetEndTime() > GetTickCount() )
			return;

		ackIconsSet[nAckSequenceCounter]->Deactivate();
	}

	nAckSequenceCounter++;
	if ( nAckSequenceCounter >= ackEventsSet.size() )
	{
		nAckSequenceCounter = -1;
		for( int nTemp = 0; nTemp < NDb::N_ACKINFO_MAX_COUNT; nTemp++ )
			ackIconsSet[nTemp]->SetStyle( STYLE_VISIBLE, false );

		ackEventsSet.clear();
		return;
	}

	CPtr<CAckIcon> pIcon = ackIconsSet[nAckSequenceCounter];
	pIcon->Set( ackEventsSet[nAckSequenceCounter] );
	pIcon->Activate();
	pIcon->SetStyle( STYLE_VISIBLE, true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUI::UpdateItems()
{
	if ( bindShowItems.IsActive() )
	{
		hash_map<DWORD, SItem> newItemsMap;

		vector<CPtr<NGame::IUnitTracker> > unitsSet;
		pMission->GetSelectedUnits( &unitsSet );

		for ( int nTemp = 0; nTemp < unitsSet.size(); nTemp++ )
		{
			vector<NWorld::SItem> tempItems;
			pMission->GetWorld()->FindCloseGroundItems( unitsSet[nTemp]->GetUnit(), &tempItems );

			for ( int nTemp = 0; nTemp < tempItems.size(); nTemp++ )
			{
				const NWorld::SItem &sItem = tempItems[nTemp];

				if ( !IsValid( sItem.pItem ) )
					continue;
				if ( !IsValid( sItem.pWorldItem ) )
					continue;

				DWORD dwKey = DWORD( sItem.pItem.GetPtr() );
				hash_map<DWORD, SItem>::iterator iFindRes = itemsMap.find( dwKey );
				if ( iFindRes == itemsMap.end() )
				{
					SItem &sMapItem = newItemsMap[dwKey];
					sMapItem.sItem = sItem;
					sMapItem.pText = new CItemText( SWindowInfo( this, SPoint( 0, 0 ), SPoint( 0, 0 ), "", STYLE_ENABLED | STYLE_VISIBLE ), pMission, this, sItem );
				}
				else
					newItemsMap[dwKey] = iFindRes->second;
			}
		}

		itemsMap = newItemsMap;
	}
	else
		itemsMap.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUI::UpdateHits( const STime &sTime )
{
	CPtr<NWorld::CHitLocator> pTempLocator;
	while( pTempLocator = pMission->GetWorld()->GetHitEvent() )
		hitsList.push_back( new CHitTracker( SWindowInfo( pClientWindow, SPoint( 0, 0 ), SPoint( 120, 20 ), "hit", STYLE_ENABLED | STYLE_TRANSPARENT | STYLE_TOPMOST | STYLE_VISIBLE ), pMission, pTempLocator, sTime ) );

	for ( list<CObj<CHitTracker> >::iterator iTemp = hitsList.begin(); iTemp != hitsList.end(); )
	{
		if ( !(*iTemp)->IsComplete() )
			iTemp++;
		else
			iTemp = hitsList.erase( iTemp );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUI::UpdateEnemies()
{
	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetUnits( &unitsSet );

	CVec2 vScreenRect = pMission->GetScene()->GetScreenRect();
	CTransformStack sTS = pMission->GetCameraTransform();

	SRect sViewRect;
	SPoint sViewPosition;
	pClientWindow->ClientToScreen( &sViewPosition, &sViewRect );

	if ( sViewRect.Width() == 0 )
	{
		enemyIconsList.clear();
		return;
	}

	hash_map<CPtr<NWorld::CUnit>,bool,SPtrHash> enemySet;
	for ( int nTemp = 0; nTemp < unitsSet.size(); nTemp++ )
	{
		vector<CPtr<NWorld::CUnit> > visibleUnits;
		unitsSet[nTemp]->GetUnit()->GetVisible( &visibleUnits );

		for ( vector<CPtr<NWorld::CUnit> >::const_iterator iEnemy = visibleUnits.begin(); iEnemy != visibleUnits.end(); iEnemy++ )
		{
			bool &bValue = enemySet[*iEnemy];
			if ( unitsSet[nTemp]->IsSelected() )
				bValue = true;
		}
	}

	list<CObj<CEnemyIcon> > newEnemyIconsList;
	list<CObj<CEnemyIcon> >::iterator iOldIcons = enemyIconsList.begin();
	for ( hash_map<CPtr<NWorld::CUnit>,bool,SPtrHash>::iterator iTemp = enemySet.begin(); iTemp != enemySet.end(); iTemp++ )
	{
		bool bVisible = iTemp->second;
		CPtr<NWorld::CUnit> pEnemy = iTemp->first;

		if ( pEnemy->IsDead() || pEnemy->IsUnconscious() )
			continue;

		CEnemyIcon *pIcon;
		if ( iOldIcons != enemyIconsList.end() )
		{
			pIcon = (*iOldIcons);
			iOldIcons++;
		}
		else
		{
			pIcon = new CEnemyIcon( SWindowInfo( this, SPoint( 0, 0 ), SPoint( 0, 0 ), "", STYLE_ENABLED | STYLE_VISIBLE ), pMission, this );
		}

		CVec2 vScreenPos;
		CVec3 vEyePosition( pEnemy->GetPosition().GetEyePosition() );
		vEyePosition += CVec3( 0, 0, 0.6f );
		TestRayInFrustrum( vEyePosition, &sTS, vScreenRect, &vScreenPos );
		vScreenPos.x = vScreenPos.x * 1024 / vScreenRect.x;
		vScreenPos.y = vScreenPos.y * 768 / vScreenRect.y;

		bool bRet = false;
		if ( ( sViewRect.x1 < vScreenPos.x ) && ( sViewRect.x2 > vScreenPos.x ) && ( sViewRect.y1 < vScreenPos.y ) && ( sViewRect.y2 > vScreenPos.y ) )
			bRet = true;

		float fAngle = -1;
		if ( !bRet )
		{
			fAngle = ToDegree( atan2( vScreenPos.x - ( sViewRect.x2 - sViewRect.x1 ) / 2, -( vScreenPos.y - ( sViewRect.y2 - sViewRect.y1 ) / 2 ) ) );
			if ( fAngle < 0 )
				fAngle += 360;
		}

		const NUI::SPoint &sSize = pIcon->GetSize();
		vScreenPos.x = max( min( vScreenPos.x, sViewRect.x2 ), sViewRect.x1 );
		vScreenPos.y = max( min( vScreenPos.y, sViewRect.y2 ), sViewRect.y1 );

		pIcon->Set( pEnemy, bVisible, fAngle );
		pIcon->SetPosition( SPoint( vScreenPos.x, vScreenPos.y ) );
		newEnemyIconsList.push_back( pIcon );
	}

	enemyIconsList = newEnemyIconsList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMissionUI::UpdateCameraScroll( const STime &sTime )
{
	STime sDelta = sTime - sCameraScrollUpdate;
	sDelta = Min( sDelta, (STime)100 );
	sCameraScrollUpdate = sTime;

	CPtr<ICamera> pCamera = pMission->GetCamera();
	CPtr<NUI::ICursor> pCursor = pMission->GetCursor();
	CPtr<NGScene::IGameView> pView = pMission->GetScene();

	const CVec2 &vPos = pCursor->GetPos();
	const CVec2 &vScreenRect = pView->GetScreenRect();

	ICamera::SCameraPos sCameraPos;
	pCamera->GetPlacement( &sCameraPos );

	CVec3 vStrafeDir( pCamera->GetStrafeDir() ), vForwardDir( pCamera->GetForwardDir() );
	vStrafeDir.z = 0;
	vForwardDir.z = 0;
	Normalize( &vStrafeDir );
	Normalize( &vForwardDir );

	int nMask = 0;
	float fStep = (float)( N_SCROLL_STEP * sDelta ) / 1000.0f;
	if ( vPos.x < N_SCROLL_GUARDBAND )
	{
		nMask |= 1;
		sCameraPos.ptAnchor -= vStrafeDir * fStep;
	}
	if ( vPos.x > vScreenRect.x - N_SCROLL_GUARDBAND )
	{
		nMask |= 2;
		sCameraPos.ptAnchor += vStrafeDir * fStep;
	}
	if ( vPos.y < N_SCROLL_GUARDBAND )
	{
		nMask |= 4;
		sCameraPos.ptAnchor += vForwardDir * fStep;
	}
	if ( vPos.y > vScreenRect.y - N_SCROLL_GUARDBAND )
	{
		nMask |= 8;
		sCameraPos.ptAnchor -= vForwardDir * fStep;
	}

	pCamera->SetPlacement( sCameraPos );
/*
	if ( nMask != 0 )
	{
		switch( nMask )
		{
		case 1:
			SetCursorType( N_CURSOR_LEFT );
			break;
		case 2:
			SetCursorType( N_CURSOR_RIGHT );
			break;
		case 4:
			SetCursorType( N_CURSOR_UP );
			break;
		case 8:
			SetCursorType( N_CURSOR_DOWN );
			break;
		case 5:
			SetCursorType( N_CURSOR_UP_LEFT );
			break;
		case 6:
			SetCursorType( N_CURSOR_RIGHT_UP );
			break;
		case 9:
			SetCursorType( N_CURSOR_LEFT_DOWN );
			break;
		case 10:
			SetCursorType( N_CURSOR_DOWN_RIGHT );
			break;
		default:
			ASSERT( 0 );
			break;
		}
	}
*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0241940, CAckIcon );
REGISTER_SAVELOAD_CLASS( 0xB0241942, CMissionUI );
REGISTER_SAVELOAD_CLASS( 0xB0241943, CLevelSwitchBar );
REGISTER_SAVELOAD_CLASS( 0xB0241946, CMissionUnitPanel );
REGISTER_SAVELOAD_CLASS( 0xB0241947, CItemText );
REGISTER_SAVELOAD_CLASS( 0xB0241948, CEnemyIcon );
REGISTER_SAVELOAD_CLASS( 0xB0241949, CHitTracker );
