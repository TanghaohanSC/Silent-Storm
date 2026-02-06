#ifndef __wObject_H_
#define __wObject_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "wOSBase.h"
namespace NRPG
{
	class IWeaponItem;
	class CAttackPortion;
	class CRPGArmor;
}
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CObjectServer: public CObjectServerBase
{
	OBJECT_NOCOPY_METHODS(CObjectServer);
	ZDATA_(CObjectServerBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CObjectServerBase*)this); return 0; }
public:
	CObjectServer() {}
	CObjectServer( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
		NDb::CObject *pO, NRPG::IObject *pRPG ) :
		CObjectServerBase( pWorld, pos, bLightMap, pO, pRPG ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAnimObjectServer: public CAnimObjectServerBase
{
	OBJECT_NOCOPY_METHODS(CAnimObjectServer);
	ZDATA_(CAnimObjectServerBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAnimObjectServerBase*)this); return 0; }
public:
	CAnimObjectServer() {}
	CAnimObjectServer( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
		NDb::CObject *pO, NRPG::IObject *pRPG, CFuncBase<STime> *_pTime )
		: CAnimObjectServerBase( pWorld, pos, bLightMap, pO, pRPG, _pTime ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWindowDoor: public CAnimObjectServerBase, public IWindowDoor
{
	OBJECT_NOCOPY_METHODS(CWindowDoor);
	ZDATA_(CAnimObjectServerBase)
	bool bIsOpen;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAnimObjectServerBase*)this); f.Add(2,&bIsOpen); return 0; }
public:
	CWindowDoor() {}
	CWindowDoor( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
		NDb::CObject *pO, NRPG::IObject *pRPG, CFuncBase<STime> *_pTime, bool bOpen = false );

	// IWindowDoor
	virtual bool IsBroken() const;
	void OpenClose( bool bOpen, bool bAbruptly = false );
	virtual bool IsOpen() const { return bIsOpen; }
	// IDynamicObject
	virtual bool Segment();
	virtual void Visit( IAIVisitor *p );
	// 
	int ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCannon: public CAnimObjectServerBase, public ICannon
{
	OBJECT_NOCOPY_METHODS(CCannon);
	ZDATA_(CAnimObjectServerBase)
	CPtr<CUnit> pCurUnit;
	CObj<NRPG::IWeaponItem> pItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAnimObjectServerBase*)this); f.Add(2,&pCurUnit); f.Add(3,&pItem); return 0; }
public:
	CCannon() {}
	CCannon( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
		NDb::CObject *pO, NRPG::IObject *pRPG, CFuncBase<STime> *_pTime );

	NDb::CSkeleton* GetSkeleton() { return pSkeleton; }
	NAnimation::CSkeletonAnimator* GetSkeletonAnimator() { return pAnimator; }

	void SetCurrentUnit( CUnit *pUnit ) { pCurUnit = pUnit; }
	CVec3 GetPosition();
	float GetDirection();
	NRPG::IWeaponItem* GetItem() const { return pItem; }

	// ICannon
	virtual bool IsBroken() const;
	virtual bool IsOccupied() const { return IsValid( pCurUnit ); }
	virtual CUnit* GetCurrentUnit() const { return pCurUnit; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPassageObject: public CAnimObjectServerBase, public IPassageObject
{
	OBJECT_NOCOPY_METHODS(CPassageObject);
	ZDATA
	ZPARENT( CAnimObjectServerBase );
	int nPassageZoneID;
	int nPassageObjectID;
	int nAPRadius;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAnimObjectServerBase *)this); f.Add(3,&nPassageZoneID); f.Add(4,&nPassageObjectID); f.Add(5,&nAPRadius); return 0; }
public:
	//
	CPassageObject() {}
	CPassageObject( CWorld *pWorld, const SObjectPlace &pos, 
		bool bLightMap,	NDb::CObject *pO, NRPG::IObject *pRPG, 
		CFuncBase<STime> *_pTime, int _nPassageZoneID,	int _nPassageObjectID, int _nAPRadius );
	//
	bool CanPass( CUnitServer *pUS );
	int GetAPRadius() { return nAPRadius; }
	int GetPassageZoneID() { return nPassageZoneID; }
	int GetPassageObjectID() { return nPassageObjectID; }
	virtual bool UsePassageObject( CUnitServer *pUS );
	virtual bool IsBroken() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
