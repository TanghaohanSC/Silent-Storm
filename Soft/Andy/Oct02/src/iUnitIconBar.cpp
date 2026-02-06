#include "StdAfx.h"
#include "GView.h"
#include "G2DView.h"
#include "Transform.h"
#include "wInterface.h"
#include "RPGItemInfo.h"
#include "RPGUnitInfo.h"
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
const int
	N_ICON_UNDEFAINED		= -1,
	N_ICON_UNAVAILABLE	= -2;
////////////////////////////////////////////////////////////////////////////////////////////////////
static int GetUnitsGroupPose( NGame::IMission *pMission )
{
	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetSelectedUnits( &unitsSet );

	int nPose = N_ICON_UNAVAILABLE;
	bool bPoseSet = false;
	for ( int nTemp = 0; nTemp < unitsSet.size(); nTemp++ )
	{
		NAI::EPose ePose = unitsSet[nTemp]->GetUnit()->GetPosition().GetPose();
		if ( !bPoseSet )
		{
			nPose = ePose;
			bPoseSet = true;
		}
		else if ( nPose != ePose )
			nPose = N_ICON_UNDEFAINED;
	}

	return nPose;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int GetUnitsGroupStrafe( NGame::IMission *pMission )
{
	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetSelectedUnits( &unitsSet );

	int nStrafe = N_ICON_UNAVAILABLE;
	bool bPoseSet = false;
	for ( int nTemp = 0; nTemp < unitsSet.size(); nTemp++ )
	{
		int nNewStrafe = 0;
		if ( unitsSet[nTemp]->IsStrafeMove() )
			nNewStrafe = 1;

		if ( !bPoseSet )
		{
			nStrafe = nNewStrafe;
			bPoseSet = true;
		}
		else if ( nStrafe != nNewStrafe )
			nStrafe = N_ICON_UNDEFAINED;
	}

	return nStrafe;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int GetUnitsGroupWeaponMode( NGame::IMission *pMission )
{
	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	pMission->GetSelectedUnits( &unitsSet );

	int nMode = N_ICON_UNAVAILABLE;
	bool bModeSet = false;
	for ( int nTemp = 0; nTemp < unitsSet.size(); nTemp++ )
	{
		CPtr<NRPG::IInventoryItem> pItem = unitsSet[nTemp]->GetUnit()->GetRPG()->GetInventoryInfo()->GetActive();
		CPtr<NRPG::IWeaponItemInfo> pWeapon = dynamic_cast<NRPG::IWeaponItemInfo*>( pItem.GetPtr() );
		if ( IsValid( pWeapon ) )
		{
			NDb::EShootMode eMode = pWeapon->GetShootMode();
			if ( !bModeSet )
			{
				nMode = eMode;
				bModeSet = true;
			}
			else if ( nMode != eMode )
				nMode = N_ICON_UNDEFAINED;
		}
	}

	return nMode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CIconButton
////////////////////////////////////////////////////////////////////////////////////////////////////
class CIconButton: public CButton
{
	OBJECT_BASIC_METHODS(CIconButton)
private:
	enum
	{
		STATE_NORMAL,
		STATE_DISABLED
	};

	ZDATA_(CButton)
	string szID;
	CObj<CImage> pStateNormal;
	CObj<CImage> pSelection;
	CObj<CImage> pStateDisabled;
	CObj<CToolTip> pToolTip;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CButton*)this); f.Add(2,&szID); f.Add(3,&pStateNormal); f.Add(4,&pSelection); f.Add(5,&pStateDisabled); f.Add(6,&pToolTip); return 0; }

protected:
	void OnAction();

public:
	CIconButton() {}
	CIconButton( const SWindowInfo &sInfo );

	CToolTip* GetToolTip() const;

	void Set( NDb::CUITexture *pNormal = 0, NDb::CUITexture *pChecked = 0, NDb::CUITexture *pDisabled = 0, const string &szID = "" );
	void SetColor( const NGfx::SPixel8888 &sColor );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CIconButton::CIconButton( const SWindowInfo &sInfo ):
	CButton( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CToolTip* CIconButton::GetToolTip() const
{
	return pToolTip;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CIconButton::Set( NDb::CUITexture *pNormal, NDb::CUITexture *pChecked, NDb::CUITexture *pDisabled, const string &_szID )
{
	szID = _szID;

	pStateNormal->SetImage( pNormal );
	pStateDisabled->SetImage( pDisabled );

	if ( IsValid( pChecked ) )
	{
		pSelection->SetImage( pChecked );
		pSelection->SetStyle( STYLE_VISIBLE, true );
	}
	else
		pSelection->SetStyle( STYLE_VISIBLE, false );

	SetActiveState( STATE_NORMAL );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CIconButton::SetColor( const NGfx::SPixel8888 &sColor )
{
	pStateNormal->SetColor( sColor );
	pStateDisabled->SetColor( sColor );
	CButton::SetColor( sColor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CIconButton::ProcessMessage( const SEvent &sEvent )
{
	switch ( sEvent.nEvent )
	{
	case EVENT_TEMPLATECREATE:
		{
			CWindow *pWindow;
			pWindow = AddState( STATE_NORMAL );
			pStateNormal = new CImage( SWindowInfo( pWindow, SPoint( 0, 0 ), GetSize(), "", STYLE_ENABLED | STYLE_VISIBLE | STYLE_TRANSPARENT ) );
			pSelection = new CImage( SWindowInfo( pWindow, SPoint( 0, 0 ), GetSize(), "", STYLE_ENABLED | STYLE_VISIBLE | STYLE_TRANSPARENT | STYLE_BOTTOMMOST ) );
			pSelection->SetImage( NDb::GetUITexture( 408 ) );

			pWindow = AddState( STATE_DISABLED );
			pStateDisabled = new CImage( SWindowInfo( pWindow, SPoint( 0, 0 ), GetSize(), "", STYLE_ENABLED | STYLE_VISIBLE | STYLE_TRANSPARENT ) );

			pToolTip = new CToolTip( SWindowInfo( GetInterface(), SPoint( 0, 0 ), SPoint( 0, 0 ), "tooltip", STYLE_ENABLED ) );
			SetToolTip( pToolTip );

			break;
		}
	}

	return CButton::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CIconButton::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( GetStyle( STYLE_ENABLED ) )
		SetActiveState( STATE_NORMAL );
	else
		SetActiveState( STATE_DISABLED );

	CButton::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CIconButton::OnAction()
{
	SendMessage( GetParent(), SEvent( EVENT_NOTIFY, szID ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CIconBarSet
////////////////////////////////////////////////////////////////////////////////////////////////////
class CIconBarSet: public CObjectBase
{
private:
	struct SIconButton
	{
		ZDATA
		int nPriority;
		CPtr<CIconButton> pButton;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nPriority); f.Add(3,&pButton); return 0; }
	};
	ZDATA
	CPtr<NGame::IMission> pMission;
	////
	vector<SIconButton> iconsSet;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pMission); f.Add(3,&iconsSet); return 0; }

protected:
	enum { N_ANY_VALUE = -1 };
	enum ESlot
	{
		SLOT_R1C1	= 0,
		SLOT_R1C2	= 1,
		SLOT_R1C3	= 2,
		SLOT_R1C4	= 3,
		SLOT_R2C1	= 4,
		SLOT_R2C2	= 5,
		SLOT_R2C3	= 6,
		SLOT_R2C4	= 7
	};

public:
	CIconBarSet() {}
	CIconBarSet( NGame::IMission *_pGame, const vector<CPtr<CIconButton> > &_iconsSet );

	NGame::IMission* GetMission() const;

	void CreateButton( int nNormalIcon, int nCheckedIcon, int nDisabledIcon, int nToolTipID, int nSlot, int nPriority, int nUnitAction, int nUnitState, const string &szID );

	virtual void Update();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CIconBarSet::CIconBarSet( NGame::IMission *_pGame, const vector<CPtr<CIconButton> > &_iconsSet ):
	pMission( _pGame )
{
	iconsSet.resize( _iconsSet.size() );
	for ( int nTemp = 0; nTemp < iconsSet.size(); nTemp++ )
		iconsSet[nTemp].pButton = _iconsSet[nTemp];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NGame::IMission* CIconBarSet::GetMission() const
{
	return pMission;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CIconBarSet::CreateButton( int nNormalIcon, int nCheckedIcon, int nDisabledIcon, int nToolTipID, int nSlot, int nPriority, int nUnitAction, int nUnitState, const string &szID )
{
	int nState = pMission->GetUnitsWorldState();
	if ( ( nUnitState != N_ANY_VALUE ) && ( nUnitState != nState ) )
		return;
	if ( iconsSet[nSlot].nPriority < nPriority )
		return;

	NGame::SActionInfo sActionInfo;
	pMission->GetActionInfo( NGame::EUnitAction( nUnitAction ), &sActionInfo );
	if ( !sActionInfo.bAvailable )
		return;

	CPtr<NDb::CUITexture> pNormalIcon, pCheckedIcon, pDisabledIcon;
	if ( nNormalIcon != -1 )
		pNormalIcon = NDb::GetUITexture( nNormalIcon );
	if ( nCheckedIcon != -1 )
		pCheckedIcon = NDb::GetUITexture( nCheckedIcon );
	if ( nDisabledIcon != -1 )
		pDisabledIcon = NDb::GetUITexture( nDisabledIcon );

	iconsSet[nSlot].nPriority = nPriority;

	CPtr<CIconButton> pButton = iconsSet[nSlot].pButton;
	pButton->Set( pNormalIcon, pCheckedIcon, pDisabledIcon, szID );
	pButton->SetStyle( STYLE_VISIBLE, true );
	pButton->SetStyle( STYLE_ENABLED, sActionInfo.bOk );

	CPtr<CToolTip> pToolTip = pButton->GetToolTip();
	pToolTip->SetText( GetDBString( nToolTipID ) );

	if ( sActionInfo.bOk )
		pToolTip->SetVal( L"ap", sActionInfo.nActionAP );
	else
		pToolTip->SetVal( L"ap", L"N/A" );

	if ( sActionInfo.bEnoughAP )
		pButton->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF ) );
	else
		pButton->SetColor( NGfx::SPixel8888( 0x5F, 0x5F, 0xBF, 0xFF ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CIconBarSet::Update()
{
	for( int nTemp = 0; nTemp < iconsSet.size(); nTemp++ )
	{
		iconsSet[nTemp].nPriority = 0xFF;
		iconsSet[nTemp].pButton->SetStyle( STYLE_VISIBLE, false );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMainIconBarSet
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMainIconBarSet: public CIconBarSet
{
	OBJECT_BASIC_METHODS(CMainIconBarSet)
private:
	ZDATA_(CIconBarSet)
	CObj<CIconBarSet> pPoseIconBar;
	CObj<CIconBarSet> pWeaponModeIconBar;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CIconBarSet*)this); f.Add(2,&pPoseIconBar); f.Add(3,&pWeaponModeIconBar); return 0; }

public:
	CMainIconBarSet() {}
	CMainIconBarSet( NGame::IMission *pMission, const vector<CPtr<CIconButton> > &iconsSet );

	virtual void Update();
};                                                                                                  
////////////////////////////////////////////////////////////////////////////////////////////////////
CMainIconBarSet::CMainIconBarSet( NGame::IMission *pMission, const vector<CPtr<CIconButton> > &iconsSet ):
	CIconBarSet( pMission, iconsSet )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMainIconBarSet::Update()
{
	CIconBarSet::Update();

	int nState = GetMission()->GetUnitsState();
	switch( nState )
	{
	case NGame::N_UNITSTATE_DEFAULT:
		{
			// R1C1
			CreateButton( 386, -1, 420, 4277, SLOT_R1C1, 0, NGame::UA_ATTACK, NWorld::CUnit::ST_NORMAL_MELEE, "attack" );
			CreateButton( 385, -1, 425, 4278, SLOT_R1C1, 0, NGame::UA_ATTACK, NWorld::CUnit::ST_NORMAL_KNIFE, "attack" );
			CreateButton( 384, -1, 419, 4279, SLOT_R1C1, 0, NGame::UA_ATTACK, NWorld::CUnit::ST_NORMAL_GRENADE,	"attack" );
			CreateButton( 387, -1, 419, 4280, SLOT_R1C1, 1, NGame::UA_ATTACK, N_ANY_VALUE, "attack" );
			CreateButton( 388, -1, 426, 4291, SLOT_R1C1, 1, NGame::UA_HEAL, NWorld::CUnit::ST_NORMAL_MEDKIT, "firstaid" );
			CreateButton( 443, -1, 475, 4290, SLOT_R1C1, 1, NGame::UA_DROPCORPSE, NWorld::CUnit::ST_CARRY_CORPSE, "dropcorpse" );
			// R1C2
			CreateButton( 474, -1, 510, 4466, SLOT_R1C2, 0, NGame::UA_CONTINUE, N_ANY_VALUE, "continue" );
			CreateButton( 390, -1, 421, 4281, SLOT_R1C2, 1, NGame::UA_MOVE, N_ANY_VALUE, "move" );
			// R1C3, R1C4
			CreateButton( 389, -1, 427, 4282, SLOT_R1C3, 0, NGame::UA_LOOK, N_ANY_VALUE, "unit_rotate" );
			CreateButton( 382, -1, 421, 4283, SLOT_R1C4, 0, NGame::UA_STOP, N_ANY_VALUE, "cancel" );
			// R2C1
			int nWeaponMode = GetUnitsGroupWeaponMode( GetMission() );
			switch( nWeaponMode )
			{
			case N_ICON_UNDEFAINED:
				CreateButton( 516, -1,  -1, 4522, SLOT_R2C1, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_weaponmode" );
				break;
			case NDb::SM_Snap:
				CreateButton( 501, -1,  -1, 4522, SLOT_R2C1, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_weaponmode" );
				break;
			case NDb::SM_Aimed:
				CreateButton( 503, -1,  -1, 4522, SLOT_R2C1, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_weaponmode" );
				break;
			case NDb::SM_Careful:
				CreateButton( 502, -1,  -1, 4522, SLOT_R2C1, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_weaponmode" );
				break;
			case NDb::SM_ShortBurst:
				CreateButton( 500, -1,  -1, 4522, SLOT_R2C1, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_weaponmode" );
				break;
			case NDb::SM_LongBurst:
				CreateButton( 499, -1,  -1, 4522, SLOT_R2C1, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_weaponmode" );
				break;
			case NDb::SM_Snipe:
				CreateButton( 498, -1,  -1, 4522, SLOT_R2C1, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_weaponmode" );
				break;
			}
			// R2C2
			int nPose = GetUnitsGroupPose( GetMission() );
			switch( nPose )
			{
			case N_ICON_UNDEFAINED:
				CreateButton( 504, -1,  -1, 4523, SLOT_R2C2, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_poseselect" );
				break;
			case NAI::RUN:
				CreateButton( 393, -1,  -1, 4523, SLOT_R2C2, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_poseselect" );
				break;
			case NAI::WALK:
				CreateButton( 394, -1,  -1, 4523, SLOT_R2C2, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_poseselect" );
				break;
			case NAI::CROUCH:
				CreateButton( 392, -1,  -1, 4523, SLOT_R2C2, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_poseselect" );
				break;
			case NAI::CRAWL:
				CreateButton( 391, -1,  -1, 4523, SLOT_R2C2, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "submenu_poseselect" );
				break;
			}
			// R2C3
			int nStrafe = GetUnitsGroupStrafe( GetMission() );
			switch( nStrafe )
			{
			case 0:
				CreateButton( 505, -1, 509, 4524, SLOT_R2C3, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "pose_strafe" );
				break;
			case 1:
				CreateButton( 505, 408, 509, 4524, SLOT_R2C3, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "pose_strafe" );
				break;
			}
			// R2C4
			CreateButton( 506, -1, 508, 4525, SLOT_R2C4, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "hide" );
			break;
		}
	case NGame::N_UNITSTATE_CANNON:
		{
			CreateButton( 387, -1, 419, 4289, SLOT_R1C1, 0, NGame::UA_ATTACK, NWorld::CUnit::ST_MACHINE_GUN, "attack" );
			CreateButton( 382, -1, 421, 4283, SLOT_R2C4, 0, NGame::UA_STOP, N_ANY_VALUE, "cancel" );
			break;
		}
	case NGame::N_UNITSTATE_SNIPE:
		{
			CreateButton( 490, -1, 511, 4442, SLOT_R1C1, 0, NGame::UA_COLLECTAP_ALL, NWorld::CUnit::ST_SNIPE, "collectap_all" );
			CreateButton( 489, -1, 513, 4441, SLOT_R1C2, 0, NGame::UA_COLLECTAP_MAX, NWorld::CUnit::ST_SNIPE, "collectap_max" );
			CreateButton( 488, -1, 514, 4443, SLOT_R1C3, 0, NGame::UA_COLLECTAP_10AP, NWorld::CUnit::ST_SNIPE, "collectap_10ap" );
			CreateButton( 487, -1, 515, 4444, SLOT_R1C4, 0, NGame::UA_COLLECTAP_1AP, NWorld::CUnit::ST_SNIPE, "collectap_1ap" );
			CreateButton( 491, -1, 512, 4445, SLOT_R2C1, 1, NGame::UA_SNIPE_ATTACK, NWorld::CUnit::ST_SNIPE, "snipe_attack" );
			CreateButton( 382, -1, 421, 4283, SLOT_R2C4, 0, NGame::UA_STOP, N_ANY_VALUE, "cancel" );
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPoseIconBarSet
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPoseIconBarSet: public CIconBarSet
{
	OBJECT_BASIC_METHODS(CPoseIconBarSet)
private:
	ZDATA_(CIconBarSet)
	int nInitPose;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CIconBarSet*)this); f.Add(2,&nInitPose); return 0; }

public:
	CPoseIconBarSet() {}
	CPoseIconBarSet( NGame::IMission *pMission, const vector<CPtr<CIconButton> > &iconsSet );

	virtual void Update();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CPoseIconBarSet::CPoseIconBarSet( NGame::IMission *pMission, const vector<CPtr<CIconButton> > &iconsSet ):
	CIconBarSet( pMission, iconsSet )
{
	nInitPose = GetUnitsGroupPose( pMission );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPoseIconBarSet::Update()
{
	CIconBarSet::Update();

	int nPose = GetUnitsGroupPose( GetMission() );

	CreateButton( 393, ( nPose == NAI::RUN ? 408 : -1 ), 440, 4284, SLOT_R1C1, 0, NGame::UA_POSERUN, N_ANY_VALUE, "pose_run" );
	CreateButton( 394, ( nPose == NAI::WALK ? 408 : -1 ), 441, 4285, SLOT_R1C2, 0, NGame::UA_POSEWALK, N_ANY_VALUE, "pose_normal" );
	CreateButton( 392, ( nPose == NAI::CROUCH ? 408 : -1 ), 439, 4286, SLOT_R1C3, 0, NGame::UA_POSECROUCH, N_ANY_VALUE, "pose_crouch" );
	CreateButton( 391, ( nPose == NAI::CRAWL ? 408 : -1 ), 438, 4288, SLOT_R1C4, 0, NGame::UA_POSECRAWL, N_ANY_VALUE, "pose_crawl" );
	////
	CreateButton( 382, -1, 421, 4283, SLOT_R2C4, 0, NGame::UA_STOP, N_ANY_VALUE, "cancel" );

	if ( nInitPose != nPose )
	{
		SInterfaceState sState = GetMission()->GetMissionUI()->GetInterfaceState();
		sState.eIconsSet = SInterfaceState::IS_MAIN;
		GetMission()->GetMissionUI()->SetInterfaceState( sState );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CWeaponModeIconBarSet
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWeaponModeIconBarSet: public CIconBarSet
{
	OBJECT_BASIC_METHODS(CWeaponModeIconBarSet)
private:
	ZDATA_(CIconBarSet)
	int nInitWeaponMode;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CIconBarSet*)this); f.Add(2,&nInitWeaponMode); return 0; }

public:
	CWeaponModeIconBarSet() {}
	CWeaponModeIconBarSet( NGame::IMission *pMission, const vector<CPtr<CIconButton> > &iconsSet );

	virtual void Update();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CWeaponModeIconBarSet::CWeaponModeIconBarSet( NGame::IMission *pMission, const vector<CPtr<CIconButton> > &iconsSet ):
	CIconBarSet( pMission, iconsSet )
{
	nInitWeaponMode = GetUnitsGroupWeaponMode( pMission );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWeaponModeIconBarSet::Update()
{
	CIconBarSet::Update();

	struct SWeaponModeInfo
	{
		NDb::EShootMode eMode;

		CHAR* pszID;
		int nIcon;
		int nToolTipID;
	};
	static SWeaponModeInfo pWeaponModes[] = 
	{
		{ NDb::SM_Snap, "weapon_snapshot", 501, 4298 },
		{ NDb::SM_Aimed, "weapon_aimedshot", 503, 4299 },
		{ NDb::SM_Careful, "weapon_carefulshot", 502, 4300 },
		{ NDb::SM_ShortBurst, "weapon_shortburst", 500, 4301 },
		{ NDb::SM_LongBurst, "weapon_longburst", 499, 4302 },
		{ NDb::SM_Snipe, "weapon_snipeshot", 498, 4303 },
	};

	vector<CPtr<NGame::IUnitTracker> > unitsSet;
	GetMission()->GetSelectedUnits( &unitsSet );

	if ( unitsSet.size() != 1 )
	{
		SInterfaceState sState = GetMission()->GetMissionUI()->GetInterfaceState();
		sState.eIconsSet = SInterfaceState::IS_MAIN;
		GetMission()->GetMissionUI()->SetInterfaceState( sState );
		return;
	}

	int nWeaponMode = GetUnitsGroupWeaponMode( GetMission() );

	CPtr<NRPG::IInventoryInfo> pInventory = unitsSet[0]->GetUnit()->GetRPG()->GetInventoryInfo();
	CPtr<NRPG::IInventoryItem> pItem = pInventory->GetActive();
	CPtr<NRPG::IWeaponItemInfo> pWeapon = dynamic_cast<NRPG::IWeaponItemInfo*>( pItem.GetPtr() );
	if ( !IsValid( pWeapon ) )
	{
		SInterfaceState sState = GetMission()->GetMissionUI()->GetInterfaceState();
		sState.eIconsSet = SInterfaceState::IS_MAIN;
		GetMission()->GetMissionUI()->SetInterfaceState( sState );
		return;
	}

	int nCount = 0;
	NDb::EShootMode eMode = pWeapon->GetShootMode();
	CPtr<NDb::CRPGWeapon> pRPGWeapon = pWeapon->GetDBWeapon();
	for ( int nTemp = 0; nTemp < ARRAY_SIZE( pWeaponModes ); nTemp++ )
	{
		if ( nCount >= 4 )
			break;
		const SWeaponModeInfo &sModeInfo = pWeaponModes[nTemp];
		if ( !pRPGWeapon->shootModes[sModeInfo.eMode] )
			continue;

		CreateButton( sModeInfo.nIcon, ( sModeInfo.eMode == nWeaponMode ? 408 : -1 ), -1, sModeInfo.nToolTipID, SLOT_R1C1 + nCount, 0, NGame::UA_DEFAULT, N_ANY_VALUE, sModeInfo.pszID );
		nCount++;
	}
	////
	CreateButton( 382, -1, 421, 4283, SLOT_R2C4, 0, NGame::UA_DEFAULT, N_ANY_VALUE, "cancel" );

	if ( nInitWeaponMode != nWeaponMode )
	{
		SInterfaceState sState = GetMission()->GetMissionUI()->GetInterfaceState();
		sState.eIconsSet = SInterfaceState::IS_MAIN;
		GetMission()->GetMissionUI()->SetInterfaceState( sState );
		return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CForcedIconBarSet
////////////////////////////////////////////////////////////////////////////////////////////////////
class CForcedIconBarSet: public CIconBarSet
{
	OBJECT_BASIC_METHODS(CForcedIconBarSet)
private:
	ZDATA_(CIconBarSet)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CIconBarSet*)this); return 0; }

public:
	CForcedIconBarSet() {}
	CForcedIconBarSet( NGame::IMission *pMission, const vector<CPtr<CIconButton> > &iconsSet );

	virtual void Update();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CForcedIconBarSet::CForcedIconBarSet( NGame::IMission *pMission, const vector<CPtr<CIconButton> > &iconsSet ):
	CIconBarSet( pMission, iconsSet )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CForcedIconBarSet::Update()
{
	CIconBarSet::Update();

	CreateButton( 382, -1, 421, 4283, SLOT_R2C4, 0, NGame::UA_STOP, N_ANY_VALUE, "cancel" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitIconsBar
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitIconsBar::CUnitIconsBar( const SWindowInfo &sInfo, NGame::IMission *_pMission ):
	CWindow( sInfo ), pMission( _pMission ), iconsSet( 8 ), eLastSet( SInterfaceState::EIconsSet( -1 ) )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitIconsBar::SetIconBar( CIconBarSet *pIcons )
{
	pIconBar = pIcons;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitIconsBar::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			for ( int nTemp = 0; nTemp < iconsSet.size(); nTemp++ )
				iconsSet[nTemp] = new CIconButton( sEvent.pLoader->GetControl( NStr::Format( "icon_%d", ( nTemp + 1 ) ) ) );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pIconBar = new CMainIconBarSet( pMission, iconsSet );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitIconsBar::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	SInterfaceState sState = pMission->GetMissionUI()->GetInterfaceState();

	if ( pMission->GetState()->GetType() == NGame::IState::FORCED )
		sState.eIconsSet = SInterfaceState::IS_FORCED;
	else if ( sState.eIconsSet == SInterfaceState::IS_FORCED )
		sState.eIconsSet = SInterfaceState::IS_MAIN;

	pMission->GetMissionUI()->SetInterfaceState( sState );

	if ( eLastSet != sState.eIconsSet )
	{
		eLastSet = sState.eIconsSet;

		switch( sState.eIconsSet )
		{
		case SInterfaceState::IS_MAIN:
			{
				pIconBar = new CMainIconBarSet( pMission, iconsSet );
				break;
			}
		case SInterfaceState::IS_POSES:
			{
				pIconBar = new CPoseIconBarSet( pMission, iconsSet );
				break;
			}
		case SInterfaceState::IS_WEAPONMODES:
			{
				pIconBar = new CWeaponModeIconBarSet( pMission, iconsSet );
				break;
			}
		case SInterfaceState::IS_FORCED:
			{
				pIconBar = new CForcedIconBarSet( pMission, iconsSet );
				break;
			}
		}
	}

	if ( IsValid( pIconBar ) )
		pIconBar->Update();

	return CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
BASIC_REGISTER_CLASS(CIconBarSet)
REGISTER_SAVELOAD_CLASS( 0xB0830170, CIconButton );
REGISTER_SAVELOAD_CLASS( 0xB0830171, CMainIconBarSet );
REGISTER_SAVELOAD_CLASS( 0xB0830172, CPoseIconBarSet );
REGISTER_SAVELOAD_CLASS( 0xB0830173, CWeaponModeIconBarSet );
REGISTER_SAVELOAD_CLASS( 0xB0830174, CForcedIconBarSet );
REGISTER_SAVELOAD_CLASS( 0xB0241945, CUnitIconsBar );
