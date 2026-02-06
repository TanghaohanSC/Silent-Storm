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
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&pLevel); f.Add(4,&pClass); f.Add(5,&pEvasion); f.Add(6,&pStrength); f.Add(7,&pDexterity); f.Add(8,&pIntelligence); f.Add(9,&pActionPoints); f.Add(10,&pVitalityPoints); f.Add(11,&pEvasionBar); f.Add(12,&pStrengthBar); f.Add(13,&pDexterityBar); f.Add(14,&pIntelligenceBar); f.Add(15,&pActionPointsBar); f.Add(16,&pVitalityPointsBar); f.Add(17,&pHide); f.Add(18,&pSpot); f.Add(19,&pBurst); f.Add(20,&pMelee); f.Add(21,&pSnipe); f.Add(22,&pMedicine); f.Add(23,&pShooting); f.Add(24,&pThrowing); f.Add(25,&pInterrupt); f.Add(26,&pEngineering); f.Add(27,&pHideBar); f.Add(28,&pSpotBar); f.Add(29,&pBurstBar); f.Add(30,&pMeleeBar); f.Add(31,&pSnipeBar); f.Add(32,&pMedicineBar); f.Add(33,&pShootingBar); f.Add(34,&pThrowingBar); f.Add(35,&pInterruptBar); f.Add(36,&pEngineeringBar); f.Add(37,&pClose); f.Add(38,&pPerks); f.Add(39,&pMedals); f.Add(40,&pBackground); return 0; }

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
