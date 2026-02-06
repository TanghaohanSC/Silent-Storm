#ifndef __A5_MISSIONINTERFACE_PC_H__
#define __A5_MISSIONINTERFACE_PC_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAckIcon;
class CItemText;
class CEnemyIcon;
class CHitTracker;
class CTopBar;
class CLogPanel;
class CInventoryPanel;
class CCharacterPanel;
class CMissionUnitPanel;
////////////////////////////////////////////////////////////////////////////////////////////////////
// SInterfaceState
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SInterfaceState
{
	enum EIconsSet
	{
		IS_FORCED,
		IS_MAIN,
		IS_POSES,
		IS_WEAPONMODES
	};

	ZDATA
	bool bInventory;
	bool bCharacter;
	EIconsSet eIconsSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bInventory); f.Add(3,&bCharacter); f.Add(4,&eIconsSet); return 0; }

	SInterfaceState(): bInventory( false ), bCharacter( false ), eIconsSet( IS_MAIN ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMissionUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMissionUI: public CWindow
{
	OBJECT_NOCOPY_METHODS(CMissionUI);
private:
	NInput::CBind bindCancel, bindShowItems, bindInventory, bindCharacter;
	NInput::CBind bindPoseSubMenu, bindWeaponModeSubMenu;

	struct SItem
	{
		ZDATA
		NWorld::SItem sItem;
		CObj<CItemText> pText;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&sItem); f.Add(3,&pText); return 0; }
	};

	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	CObj<CWindow> pClientWindow;
	SInterfaceState sInterfaceState;
	////
	int nAckSequenceCounter;
	vector<CObj<CAckIcon> > ackIconsSet;
	vector<CPtr<NWorld::CAckEvent> > ackEventsSet;
	////
	hash_map<DWORD, SItem> itemsMap;
	list<CObj<CEnemyIcon> > enemyIconsList;
	////
	STime sCameraScrollUpdate;
	////
	CPtr<CImage> pPause;
	CPtr<CButton> pInventory;
	CPtr<CButton> pCharacter;
	CObj<CTopBar> pTopBar;
	CObj<CLogPanel> pLogPanel;
	CObj<CInventoryPanel> pInventoryPanel;
	CObj<CCharacterPanel> pCharacterPanel;
	CObj<CMissionUnitPanel> pUnitPanel;
	list<CObj<CHitTracker> > hitsList;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pClientWindow); f.Add(4,&sInterfaceState); f.Add(5,&nAckSequenceCounter); f.Add(6,&ackIconsSet); f.Add(7,&ackEventsSet); f.Add(8,&itemsMap); f.Add(9,&enemyIconsList); f.Add(10,&sCameraScrollUpdate); f.Add(11,&pPause); f.Add(12,&pInventory); f.Add(13,&pCharacter); f.Add(14,&pTopBar); f.Add(15,&pLogPanel); f.Add(16,&pInventoryPanel); f.Add(17,&pCharacterPanel); f.Add(18,&pUnitPanel); f.Add(19,&hitsList); return 0; }

protected:
	void UpdateAck();
	void UpdateHits( const STime &sTime );
	void UpdateItems();
	void UpdateEnemies();
	void UpdateCameraScroll( const STime &sTime );

public:
	CMissionUI();
	CMissionUI( const SWindowInfo &sInfo, NGame::IMission *pMission );

	void SetCursorType( int nType, const wstring &wsText = L"" );

	CWindow* GetClientWindow() const;

	const SInterfaceState& GetInterfaceState() const;
	void SetInterfaceState( const SInterfaceState &sInterfaceState );

	bool ProcessEvent( const NInput::SEvent &sEvent );
	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
