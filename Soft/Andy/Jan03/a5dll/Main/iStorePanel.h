#ifndef __INTERFACE_STOREPANEL_H_
#define __INTERFACE_STOREPANEL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
	class IMissionGame;
	class IUnitTracker;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStoreSlot;
class CSlotScroll;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStorePanel
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStorePanel: public CWindow
{
	OBJECT_BASIC_METHODS(CStorePanel)
private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	CPtr<CButton> pClose;
	CObj<CStoreSlot> pStoreSlot;
	CObj<CSlotScroll> pStoreSlotView;
	////
	CObj<CComplexButton> pSMG;
	CObj<CComplexButton> pOthers;
	CObj<CComplexButton> pRifles;
	CObj<CComplexButton> pPistols;
	CObj<CComplexButton> pGrenades;
	CObj<CComplexButton> pColdSteel;
	CObj<CComplexButton> pPKWeapons;
	CObj<CComplexButton> pHeavyWeapon;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pClose); f.Add(4,&pStoreSlot); f.Add(5,&pStoreSlotView); f.Add(6,&pSMG); f.Add(7,&pOthers); f.Add(8,&pRifles); f.Add(9,&pPistols); f.Add(10,&pGrenades); f.Add(11,&pColdSteel); f.Add(12,&pPKWeapons); f.Add(13,&pHeavyWeapon); return 0; }

protected:
	void UpdateButtons();

public:
	CStorePanel() {}
	CStorePanel( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
