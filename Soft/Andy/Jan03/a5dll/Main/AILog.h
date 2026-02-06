#ifndef __AILOG_H_
#define __AILOG_H_

#include "aiPosition.h"

namespace NDb
{
	enum EShootMode;
}

namespace NRPG
{
	class IInventoryItem;
	class CWeaponItem;
	class CGrenadeItem;
	class CClipItem;
	class IUnitMission;
}

namespace NWorld
{
	class CCmd;
	class CCommand;
	class CCannon;
}

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EHitLocation;
class IAIUnit;
class CAIFireArmsWeapon;
class CAIFireArmsWeaponClip;
class CAIGrenadeWeapon;
class IAIInventoryItem;
////////////////////////////////////////////////////////////////////////////////////////////////////
//	IAILogRecord
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAILogRecord: public CObjectBase
{
public:
	virtual void RollBack() = 0; // откатить 
	virtual void Commit() = 0;
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands ) = 0; // выдать комманды на исполнение
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//	IAILogContainer
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAILogContainer: public IAILogRecord
{
public:
	virtual void Add( IAILogRecord *pAILogRecord, bool bCommit = false ) = 0; // добавить запись 
	virtual void Add( IAILogContainer *pAILogContainer ) = 0; // добавить все запись контейнера
	virtual void Clear() = 0; // удалить все записи без отката
	virtual list< CObj<IAILogRecord> > *GetLogRecords() = 0;
	virtual bool IsEmpty() = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogRecord
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogRecord: public IAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogRecord);
	ZDATA
public:
	CPtr<IAIUnit> pAIUnit; // над кем производится действие 
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pAIUnit); return 0; }
	CAILogRecord() {}
	CAILogRecord( IAIUnit *_pAIUnit ) : pAIUnit(_pAIUnit) {}
	// IAILogRecord
	virtual void RollBack() {}
	virtual void Commit() {}
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogPosition
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogPosition: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogPosition);
	ZDATA
	ZPARENT(CAILogRecord)
	NAI::EPose pose;
	SPosition pSourcePosition;
	SPosition pTargetPosition;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pSourcePosition); f.Add(4,&pTargetPosition); return 0; }
public:
	CAILogPosition() {}
	CAILogPosition(	IAIUnit *_pAIUnit, SPosition _pSourcePosition, SPosition _pTargetPosition, NAI::EPose _pose = NAI::RUN );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogShot - выстрел
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogShot: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogShot);
	ZDATA
	ZPARENT(CAILogRecord)
	CPtr<IAIUnit> pTarget;
	NAI::EHitLocation eHitLocation;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pTarget); f.Add(4,&eHitLocation); return 0; }
public:
	CAILogShot() {}
	CAILogShot(	IAIUnit *_pAIUnit, IAIUnit *_pTarget, NAI::EHitLocation _eHitLocation );
	// IAILogRecord
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogShotPoint
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogShotPoint: public CAILogRecord
{
	OBJECT_BASIC_METHODS( CAILogShotPoint );
	ZDATA
	ZPARENT( CAILogRecord )
	CVec3 ptTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord *)this); f.Add(3,&ptTarget); return 0; }
public:
	CAILogShotPoint() {}
	CAILogShotPoint( IAIUnit *_pAIUnit, CVec3 ptTarget );
	// IAILogRecord
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogReloadWeapon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogReloadWeapon: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogReloadWeapon);
	ZDATA
	ZPARENT(CAILogRecord)
	CPtr<CAIFireArmsWeapon> pWeapon;
	CObj<CAIFireArmsWeaponClip> pOldClip;
	CPtr<CAIFireArmsWeaponClip> pNewClip;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pWeapon); f.Add(4,&pOldClip); f.Add(5,&pNewClip); return 0; }
public:
	CAILogReloadWeapon() {}
	CAILogReloadWeapon(	IAIUnit *_pAIUnit, CAIFireArmsWeapon *_pWeapon );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogSpendAP
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogSpendAP: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogSpendAP);
	ZDATA
	ZPARENT(CAILogRecord)
	int nOldAP, nNewAP, nMaxAP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&nOldAP); f.Add(4,&nNewAP); f.Add(5,&nMaxAP); return 0; }
public:
	CAILogSpendAP() {}
	CAILogSpendAP(	IAIUnit *_pAIUnit, int nSpendAP );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogSpendHP
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogSpendHP: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogSpendHP);
	ZDATA
	ZPARENT(CAILogRecord)
	int nOldHP, nNewHP, nMaxHP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&nOldHP); f.Add(4,&nNewHP); f.Add(5,&nMaxHP); return 0; }
public:
	CAILogSpendHP() {}
	CAILogSpendHP(	IAIUnit *_pAIUnit, int nSpendHP );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogHurt
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogHurt: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogHurt);
	ZDATA
	ZPARENT(CAILogRecord)
	int nOldHurtHP, nNewHurtHP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&nOldHurtHP); f.Add(4,&nNewHurtHP); return 0; }
public:
	CAILogHurt() {}
	CAILogHurt(	IAIUnit *_pAIUnit, int nHurtHP );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogSpendAmmo
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogSpendAmmo: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogSpendAmmo);
	ZDATA
	ZPARENT(CAILogRecord)
	int nOldAmmo, nNewAmmo;
	CPtr<CAIFireArmsWeaponClip> pClip;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&nOldAmmo); f.Add(4,&nNewAmmo); f.Add(5,&pClip); return 0; }
public:
	CAILogSpendAmmo() {}
	CAILogSpendAmmo( CAIFireArmsWeaponClip *_pClip, int nSpendAmmo );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogChangeWeapon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogChangeWeapon: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogChangeWeapon);
	ZDATA
	ZPARENT(CAILogRecord)
	CObj<IAIInventoryItem> pOldWeapon;
	CObj<IAIInventoryItem> pNewWeapon;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pOldWeapon); f.Add(4,&pNewWeapon); return 0; }
	//
	void GetItemPosition( NRPG::IInventoryItem *pItem, CTPoint<int> *Position );
public:
	//
	CAILogChangeWeapon() {}
	CAILogChangeWeapon(	IAIUnit *_pAIUnit, IAIInventoryItem *_pWeapon );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogAddWeapon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogAddWeapon: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogAddWeapon);
	ZDATA
	ZPARENT(CAILogRecord)
	CPtr<CAIFireArmsWeapon> pWeapon;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pWeapon); return 0; }
public:
	//
	CAILogAddWeapon() {}
	CAILogAddWeapon(	IAIUnit *_pAIUnit, CAIFireArmsWeapon *_pWeapon );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogAddWeaponClip
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogAddWeaponClip: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogAddWeaponClip);
	ZDATA
	ZPARENT(CAILogRecord)
	CPtr<CAIFireArmsWeapon> pWeapon;
	CPtr<CAIFireArmsWeaponClip> pClip;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pWeapon); f.Add(4,&pClip); return 0; }
public:
	//
	CAILogAddWeaponClip() {}
	CAILogAddWeaponClip(	IAIUnit *_pAIUnit, CAIFireArmsWeapon *_pWeapon, CAIFireArmsWeaponClip *_pClip );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogPickUpItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogPickUpItem: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogPickUpItem);
	ZDATA
	ZPARENT(CAILogRecord)
	CPtr<NRPG::IInventoryItem> pItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pItem); return 0; }
public:
	//
	CAILogPickUpItem() {}
	CAILogPickUpItem(	IAIUnit *_pAIUnit, NRPG::IInventoryItem *_pItem );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogDropItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogDropItem: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogDropItem);
	ZDATA
	ZPARENT(CAILogRecord)
	CPtr<NRPG::IInventoryItem> pItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pItem); return 0; }
public:
	//
	CAILogDropItem() {}
	CAILogDropItem(	IAIUnit *_pAIUnit, NRPG::IInventoryItem *_pItem );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogThrowGrenade
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogThrowGrenade: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogThrowGrenade);
	ZDATA
	ZPARENT(CAILogRecord)
	CObj<CAIGrenadeWeapon> pGrenade;
	CVec3 ptTarget;
	CPtr<IAIUnit> pEnemy;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pGrenade); f.Add(4,&ptTarget); f.Add(5,&pEnemy); return 0; }
public:
	CAILogThrowGrenade() {}
	CAILogThrowGrenade(	IAIUnit *_pUnit, CVec3 _ptTarget, CAIGrenadeWeapon *_pGrenade );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogExpediency
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogExpediency: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogExpediency);
	ZDATA
	ZPARENT(CAILogRecord)
	int nOldExpediency;
	int nNewExpediency;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&nOldExpediency); f.Add(4,&nNewExpediency); return 0; }
public:
	CAILogExpediency() {}
	CAILogExpediency(	IAIUnit *_pAIUnit, int nExpediency );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogChangeShootMode: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogChangeShootMode);
	ZDATA
	ZPARENT(CAILogRecord)
	CPtr<NRPG::CWeaponItem> pWeaponItem;
	NDb::EShootMode eOldShootMode;
	NDb::EShootMode eNewShootMode;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&pWeaponItem); f.Add(4,&eOldShootMode); f.Add(5,&eNewShootMode); return 0; }
public:
	CAILogChangeShootMode() {}
	CAILogChangeShootMode( IAIUnit *_pUnit, CAIFireArmsWeapon *pWeapon, NDb::EShootMode _eShootMode );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogChangeMaxToHit: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogChangeMaxToHit);
	ZDATA
	ZPARENT(CAILogRecord)
	int nOldToHit;
	int nNewToHit;	
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord*)this); f.Add(3,&nOldToHit); f.Add(4,&nNewToHit); return 0; }
public:
	CAILogChangeMaxToHit() {}
	CAILogChangeMaxToHit( IAIUnit *_pAIUnit, int nMaxToHit );
	// IAILogRecord
	virtual void RollBack();
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogHide: public CAILogRecord
{
	OBJECT_BASIC_METHODS( CAILogHide );
	ZDATA
	ZPARENT( CAILogRecord )
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAILogRecord *)this); return 0; }
	//
public:
	CAILogHide() {}
	CAILogHide( IAIUnit *_pAIUnit );
	//
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
/*
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogUseCannon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogUseCannon: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogUseCannon);
	ZDATA_(CAILogRecord)
	CPtr<NWorld::CCannon> pCannon;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAILogRecord*)this); f.Add(2,&pCannon); return 0; }
public:
	CAILogUseCannon() {}
	CAILogUseCannon(	IAIUnit *_pAIUnit, NWorld::CCannon *_pCannon );
	// IAILogRecord
	virtual void RollBack() {}
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogExitCannon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogExitCannon: public CAILogRecord
{
	OBJECT_BASIC_METHODS(CAILogExitCannon);
	ZDATA_(CAILogRecord)
	CPtr<NWorld::CCannon> pCannon;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAILogRecord*)this); f.Add(2,&pCannon); return 0; }
public:
	CAILogExitCannon() {}
	CAILogExitCannon(	IAIUnit *_pAIUnit, NWorld::CCannon *_pCannon );
	// IAILogRecord
	virtual void RollBack() {}
	virtual void Commit();
	virtual void GetCommands( list< CPtr<NWorld::CCommand> > *Commands );
};
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
IAILogContainer *CreateAILogContainer();
////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif