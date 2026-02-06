#ifndef __INTERFACE_CHARACTERPANEL_H_
#define __INTERFACE_CHARACTERPANEL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
	class IGlobalGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCharacterPanel
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCharacterPanel: public CWindow
{
	OBJECT_BASIC_METHODS(CCharacterPanel)
private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	CPtr<CText> pLevel;
	CPtr<CImage> pClass;
	CPtr<CProgressBar> pLevelBar;
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
	//// Controls
	CPtr<CButton> pClose;
	CPtr<CButton> pPerks;
	CPtr<CButton> pMedals;
	CPtr<CButton> pBackground;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pLevel); f.Add(4,&pClass); f.Add(5,&pLevelBar); f.Add(6,&pEvasion); f.Add(7,&pStrength); f.Add(8,&pDexterity); f.Add(9,&pIntelligence); f.Add(10,&pActionPoints); f.Add(11,&pVitalityPoints); f.Add(12,&pEvasionBar); f.Add(13,&pStrengthBar); f.Add(14,&pDexterityBar); f.Add(15,&pIntelligenceBar); f.Add(16,&pActionPointsBar); f.Add(17,&pVitalityPointsBar); f.Add(18,&pHide); f.Add(19,&pSpot); f.Add(20,&pBurst); f.Add(21,&pMelee); f.Add(22,&pSnipe); f.Add(23,&pMedicine); f.Add(24,&pShooting); f.Add(25,&pThrowing); f.Add(26,&pInterrupt); f.Add(27,&pEngineering); f.Add(28,&pHideBar); f.Add(29,&pSpotBar); f.Add(30,&pBurstBar); f.Add(31,&pMeleeBar); f.Add(32,&pSnipeBar); f.Add(33,&pMedicineBar); f.Add(34,&pShootingBar); f.Add(35,&pThrowingBar); f.Add(36,&pInterruptBar); f.Add(37,&pEngineeringBar); f.Add(38,&pClose); f.Add(39,&pPerks); f.Add(40,&pMedals); f.Add(41,&pBackground); return 0; }

public:
	CCharacterPanel() {}
	CCharacterPanel( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
