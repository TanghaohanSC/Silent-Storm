#ifndef __A5_MISSIONINTERFACE_PC_H__
#define __A5_MISSIONINTERFACE_PC_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitIconPC
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitIconPC: public CControl
{
	OBJECT_BASIC_METHODS(CUnitIconPC);
private:
	int nCheckState;
	bool bMode;
	bool bSelected;
	string szID;
	CPtr<CText> pName;
	CPtr<CText> pHPValue;
	CPtr<CText> pAPValue;
	CPtr<CText> pItemsValue;
	CPtr<CModel> pFaceIcon;
	CPtr<CModel> pWeaponIcon;
	CPtr<CModel> pBackground;
	CPtr<CModel> pSmallPanel;
	CPtr<CModel> pUnitTypeIcon;
	CPtr<CModel> pBigSelection;
	CPtr<CModel> pSmallSelection;
	CPtr<NWorld::CUnit> pUnit;

protected:
	virtual bool OnFocus( bool bFocus, CControl *pControl );
	virtual bool OnPointerButtonDown();
	virtual bool OnPointerButtonUp();
	virtual void Update( SScene *pScene, const STime &sTime );
	
public:
	CUnitIconPC() {}
	CUnitIconPC( CControl* pParent, const SPoint &sPosition, const SPoint &sSize, NWorld::CUnit *pUnit, const string &szID, int nStyle = STYLE_VISIBLE | STYLE_ENABLED );

	void SetMode( bool bFull );
	void SetUnit( NWorld::CUnit *pUnit );
	void SetSelection( bool bSelection );

	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitPanelPC
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitPanelPC: public CUnitPanel
{
	OBJECT_BASIC_METHODS(CUnitPanelPC);
private:
	bool bMode;
	CPtr<CModelButton> pEndOfTurn;
	CPtr<CModelButton> pPanelMode;
	CPtr<NWorld::CUnit> pActive;
	CPtr<NWorld::IPlayer> pPlayer;
	vector< CPtr<CUnitIconPC> > iconsSet;
	vector< CPtr<NWorld::CUnit> > unitsSet;

protected:
	virtual bool OnNotify( const string &szID, CControl *pControl );
	virtual void Update( SScene *pScene, const STime &sTime );

public:
	CUnitPanelPC() {}
	CUnitPanelPC( CControl* pParent, const SPoint &sPosition, const SPoint &sSize, NWorld::IPlayer* pPlayer, int nStyle = STYLE_VISIBLE | STYLE_ENABLED );

	void Set( NWorld::CUnit* pUnit, IUnitPanel::EDirection eDir );
	NWorld::CUnit* Get() { return pActive; }

	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CItemSwitchPC
////////////////////////////////////////////////////////////////////////////////////////////////////
class CItemSwitchPC: public CItemSwitch
{
	OBJECT_BASIC_METHODS(CItemSwitchPC);
private:
	int nActive;
	float fRadius;
	CPtr<CImage> pSelected;
	vector< CPtr<CImage> > imageSet;
	vector< CDBPtr<NDb::CTexture> > iconSet;

protected:
	virtual void Update( SScene *pScene, const STime &sTime );

public:
	CItemSwitchPC() {}
	CItemSwitchPC( CControl* pParent, const SPoint &sPosition, const SPoint &sSize, const vector< CDBPtr<NDb::CTexture> > &iconSet, int nStyle = STYLE_VISIBLE | STYLE_ENABLED );

	int GetSelected();
	void SetSelected( int nSelected );

	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
