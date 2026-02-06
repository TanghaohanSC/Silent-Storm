#ifndef __wUnitCommands_H_
#define __wUnitCommands_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "aiPosition.h"
namespace NDb
{
	enum EShootMode;
}
namespace NRPG
{
	class IInventoryItem;
}
namespace NWorld
{
class IItem;
class IObject;
class CUnit;
class CUnitServer;
struct IVisObj;
class CPassageObject;
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EUnitCommandResult
{
	//// General
	UCR_OK,
	UCR_UNAVAILABLE,
	UCR_GENERAL_FAILURE,
	UCR_INVALID_COMMAND,
	//// Path
	UCR_NO_TARGET,
	UCR_NOT_ENOUGH_AP,
	UCR_PATH_NOT_FOUND,
	//// Condition
	UCR_NEED_RELOAD,
	UCR_NO_EQUIPMENT,
	UCR_WEAPON_JAMMED,
	UCR_CRITICALS_BAN,
	UCR_TARGET_OUT_OF_RANGE,
	//// Inventory
	UCR_INVENTORY_NO_PLACE
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmd: public CObjectBase
{
public:
	virtual bool IsSkippable() const { return true; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdStartCombat: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdStartCombat);
public:
	ZDATA
	ZEND int operator&( CStructureSaver &f ) { return 0; }
	//
	CCmdStartCombat() {}
	virtual bool IsSkippable() const { return false; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdPath: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdPath);
public:
	ZDATA
	NAI::EFindPathParams eParams;
	NAI::SPosition ptDst;
	bool bStrafe;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&eParams); f.Add(3,&ptDst); f.Add(4,&bStrafe); return 0; }
	//
	CCmdPath() {}
	CCmdPath( const NAI::SPosition &_ptDst, NAI::EFindPathParams _eParams = NAI::PF_DEFAULT, bool _bStrafe = false ):
		ptDst(_ptDst), eParams( _eParams ), bStrafe(_bStrafe) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdHeal: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdHeal);
public:
	ZDATA
	CPtr<CUnit> pTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pTarget); return 0; }
	//
	CCmdHeal() {}
	CCmdHeal( CUnit *_pTarget ):
		pTarget( _pTarget ) {}
	virtual bool IsSkippable() const { return true; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdShootMode: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdShootMode);
public:
	ZDATA
	NDb::EShootMode eMode;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&eMode); return 0; }
	//
	CCmdShootMode() {}
	CCmdShootMode( NDb::EShootMode _eMode ): eMode( _eMode ) {}
	virtual bool IsSkippable() const { return true; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdShootObject: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdShootObject);
public:
	ZDATA
	CPtr<NWorld::IVisObj> pTarget;
	NAI::EHitLocation eHL;
	int nExtraAttackAP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pTarget); f.Add(3,&eHL); f.Add(4,&nExtraAttackAP); return 0; }
	//
	CCmdShootObject() {}
	CCmdShootObject( NWorld::IVisObj *_pTarget, int nExtraAP, NAI::EHitLocation _eHL = NAI::HL_ANY ):
		pTarget( _pTarget ), nExtraAttackAP(nExtraAP), eHL(_eHL) {}
	virtual bool IsSkippable() const { return false; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdShootTile: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdShootTile);
public:
	ZDATA
	CVec3 ptTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&ptTarget); return 0; }
	//
	CCmdShootTile() {}
	CCmdShootTile( const CVec3 &_ptTarget ):
		ptTarget( _ptTarget ) {}
	virtual bool IsSkippable() const { return false; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdOpenClose: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdOpenClose);
public:
	ZDATA
	CPtr<IObject> pObject;
	bool bOpen;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pObject); f.Add(3,&bOpen); return 0; }
	//
	CCmdOpenClose() {}
	CCmdOpenClose( IObject *_pObject, bool _bOpen ): pObject(_pObject), bOpen(_bOpen) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdCannon: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdCannon);
public:
	ZDATA
	CPtr<IObject> pObject;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pObject); return 0; }
	//
	CCmdCannon() {}
	CCmdCannon( IObject *_pObject ): pObject(_pObject) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdExitCannon: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdExitCannon);
public:
	ZDATA
	CPtr<IObject> pCannon; // for internal use, set in UsingCannonState
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCannon); return 0; }
	//
	CCmdExitCannon() {}
	CCmdExitCannon( IObject *_pObject ): pCannon(_pObject) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdUsePassage: public CCmd
{
	OBJECT_BASIC_METHODS( CCmdUsePassage );
	ZDATA
public:
	CPtr<IObject> pPassageObject;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pPassageObject); return 0; }
	//
	CCmdUsePassage() {}
	CCmdUsePassage( IObject *_pPassageObject ): pPassageObject( _pPassageObject ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdTakeCorpse: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdTakeCorpse);
public:
	ZDATA
	CPtr<CUnit> pCorpse;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCorpse); return 0; }
	//
	CCmdTakeCorpse() {}
	CCmdTakeCorpse( CUnit *_pCorpse ): pCorpse(_pCorpse) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdTakeCorpseOnDeploy: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdTakeCorpseOnDeploy);
public:
	ZDATA
	CPtr<CUnitServer> pCarrier;
	CPtr<CUnitServer> pCorpse;
	bool bDead;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCarrier); f.Add(3,&pCorpse); f.Add(4,&bDead); return 0; }
	//
	CCmdTakeCorpseOnDeploy() {}
	CCmdTakeCorpseOnDeploy( CUnitServer *_pCarrier, CUnitServer *_pCorpse, bool _bDead ): 
		pCarrier( _pCarrier ), pCorpse( _pCorpse ), bDead( _bDead ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdDropCorpse: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdDropCorpse);
public:
	ZDATA
	CPtr<CUnitServer> pCorpse; // for internal use, set in CorpseCarrierState
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCorpse); return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdReload: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdReload);
public:
	ZDATA
	CPtr<NRPG::IInventoryItem> pItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pItem); return 0; }
		
	CCmdReload( NRPG::IInventoryItem *_pItem = 0 ): pItem(_pItem) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdWishPose: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdWishPose);
public:
	ZDATA
	NAI::EPose pose;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pose); return 0; }
		
	CCmdWishPose() {}
	CCmdWishPose( NAI::EPose _pose ): pose(_pose) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdExplode: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdExplode);
public:
	virtual bool IsSkippable() const { return false; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdTeleport: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdTeleport);
public:
	ZDATA
	NAI::SUnitPosition pos;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pos); return 0; }
	//
	CCmdTeleport() {}
	CCmdTeleport( NAI::SUnitPosition &_pos ): pos(_pos) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// Inventory
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SItem
{
	enum EPlacement
	{
		HAND,
		SLOT,
		GROUND,
		BACKPACK,
		UNIT_ANYPLACE
	};

	ZDATA
	int nSlot;
	EPlacement eType;
	CTPoint<int> sPosition;
	CPtr<CUnit> pUnit;
	CPtr<IItem> pWorldItem;
	CPtr<NRPG::IInventoryItem> pItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nSlot); f.Add(3,&eType); f.Add(4,&sPosition); f.Add(5,&pUnit); f.Add(6,&pWorldItem); f.Add(7,&pItem); return 0; }

	SItem() {}
	SItem( CUnit *_pUnit, EPlacement _eType ): pUnit( _pUnit ), eType( _eType ) {}
	SItem( CUnit *_pUnit, EPlacement _eType, int _nSlot, NRPG::IInventoryItem* _pItem = 0 ): pUnit( _pUnit ), eType( _eType ), nSlot( _nSlot ), pItem( _pItem ) {}
	SItem( CUnit *_pUnit, EPlacement _eType, const CTPoint<int> &_sPosition, NRPG::IInventoryItem* _pItem = 0 ): pUnit( _pUnit ), eType( _eType ), sPosition( _sPosition ), pItem( _pItem ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdInventoryEnter: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdInventoryEnter);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdInventoryLeave: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdInventoryLeave);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdMoveInventoryItem: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdMoveInventoryItem);
private:
	ZDATA
	SItem sSource;
	SItem sTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&sSource); f.Add(3,&sTarget); return 0; }

public:
	CCmdMoveInventoryItem() {}
	CCmdMoveInventoryItem( const SItem &_sSource, const SItem &_sTarget ): sSource( _sSource ), sTarget( _sTarget ) {}

	const SItem& GetSource() { return sSource; }
	const SItem& GetTarget() { return sTarget; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdUnloadInventoryItem: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdUnloadInventoryItem);
private:
	ZDATA
	CPtr<NRPG::IInventoryItem> pItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pItem); return 0; }

public:
	CCmdUnloadInventoryItem() {}
	CCmdUnloadInventoryItem( NRPG::IInventoryItem *_pItem ): pItem( _pItem ) {}

	NRPG::IInventoryItem* GetItem() { return pItem; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdSetActiveItem: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdSetActiveItem);
public:
	ZDATA
	int nSlot;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nSlot); return 0; }
	//
	CCmdSetActiveItem() {}
	CCmdSetActiveItem( int _nSlot ): nSlot(_nSlot) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ECollectSnipeAP
{
	CSAP_1AP,
	CSAP_10AP,
	CSAP_MAX,
	CSAP_ALL
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdCollectSnipeAP: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdCollectSnipeAP);
public:
	ZDATA
	ECollectSnipeAP eAP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&eAP); return 0; }
	//
	CCmdCollectSnipeAP() {}
	CCmdCollectSnipeAP( ECollectSnipeAP _eAP ): eAP( _eAP ) {}
	virtual bool IsSkippable() const { return false; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdSnipeAttack: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdSnipeAttack);
public:
	ZDATA
	ZEND int operator&( CStructureSaver &f ) { return 0; }
	//
	CCmdSnipeAttack() {}
	virtual bool IsSkippable() const { return false; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdEmpty: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdEmpty);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdContinue: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdContinue);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdNeedReload: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdNeedReload);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdWeaponJammed: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdWeaponJammed);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdOrderConfirmation: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdOrderConfirmation);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdImpossibleToPerformAction: public CCmd
{
	OBJECT_BASIC_METHODS(CCmdImpossibleToPerformAction);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
