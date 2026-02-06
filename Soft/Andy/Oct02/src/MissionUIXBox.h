#ifndef __A5_MISSIONINTERFACE_XBOX_H__
#define __A5_MISSIONINTERFACE_XBOX_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitIconXB
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitIconXB: public CWindow
{
	OBJECT_BASIC_METHODS(CUnitIconXB);
private:
	int nDarkness;
	CPtr<CText> pHP;
	CPtr<CText> pAP;
	CPtr<CText> pName;
	CPtr<CText> pHPValue;
	CPtr<CText> pAPValue;
	CPtr<CText> pItemsValue;
	CPtr<CImage> pFaceIcon;
	CPtr<CImage> pWeaponIcon;
	CPtr<CImage> pBackground;
	CPtr<NWorld::CUnit> pUnit;

protected:
	virtual void Update( SScene *pScene, const STime &sTime );

public:
	CUnitIconXB() {}
	CUnitIconXB( IContainer* pParent, const SPoint &sPosition, const SPoint &sSize, NWorld::CUnit *pUnit, int nStyle = STYLE_VISIBLE | STYLE_ENABLED );

	void SetUnit( NWorld::CUnit *pUnit );
	void SetDarkness( int nValue );

	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitPanelXB
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitPanelXB: public CUnitPanel
{
	OBJECT_BASIC_METHODS(CUnitPanelXB);
private:
	float fAngle;
	float fTargetAngle;
	STime sUpdateTime;
	CPtr<NWorld::CUnit> pActive;
	CPtr<NWorld::IPlayer> pPlayer;
	IUnitPanel::EDirection eDir;
	vector< CPtr<CUnitIconXB> > iconsSet;
	vector< CPtr<NWorld::CUnit> > unitsSet;

protected:
	virtual void Update( SScene *pScene, const STime &sTime );

public:
	CUnitPanelXB() {}
	CUnitPanelXB( IContainer* pParent, const SPoint &sPosition, const SPoint &sSize, NWorld::IPlayer* pPlayer, int nStyle = STYLE_VISIBLE | STYLE_ENABLED );

	void SetPlayer( NWorld::IPlayer *_pPlayer ) {}
	void SetSelection( NWorld::CUnit* pUnit, IUnitPanel::EDirection eDir ) {}

	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
