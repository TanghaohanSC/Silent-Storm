#ifndef __AIATTACKACTION_H_
#define __AIATTACKACTION_H_
//
#include "aiAction.h"
//
namespace NDb
{
	enum EShootMode;
}
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAILogContainer;
class CAIFireArmsWeapon;
class CAIGrenadeWeapon;
struct SUnitPosition;
struct SAIUnitGroup;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIShootAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIShootAction: public CAIAction
{
public:
	struct SAIShootActionInfo
	{
		bool bCanDo;
		int nDamage;
		int nToHit;
		int nCover;
		float fDistance;
		bool bKillTargetCertainly;
		NDb::EShootMode shootMode;
		CPtr<CAIFireArmsWeapon> pWeapon;
		bool bNeedReload;
		SAIShootActionInfo(): bCanDo( false ) {}
	};
	//
private:
	OBJECT_BASIC_METHODS( CAIShootAction );
	ZDATA
	ZPARENT( CAIAction );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIAction *)this); return 0; }
	//
public:
	CAIShootAction() {}
	CAIShootAction( IAIState *_pState ): CAIAction( _pState ) {}
	//
	virtual bool CanDo( const SPlaceWithAP &place ) const;
	virtual void Do( IAILogContainer *pLog ) const;
	virtual bool ComparePlaces( const SPlaceWithAP &p1, const SPlaceWithAP &p2 ) const;
	//
	void GetInfo( const SPlaceWithAP &place, SAIShootActionInfo *pInfo ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIThrowGrenadeAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIThrowGrenadeAction: public CAIAction
{
public:
	struct SAIThrowGrenadeActionInfo
	{
		bool bCanDo;
		float fDistance;
		int nTargetGroup;
		bool bBadGroupHealth;
		int nTargetSize;
		CPtr<CAIGrenadeWeapon> pGrenade;
		SAIThrowGrenadeActionInfo(): bCanDo( false ) {}
	};
	//
private:
	OBJECT_BASIC_METHODS( CAIThrowGrenadeAction );
	ZDATA
	ZPARENT( CAIAction );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIAction *)this); return 0; }
	//
public:
	CAIThrowGrenadeAction() {}
	CAIThrowGrenadeAction( IAIState *_pState ): CAIAction( _pState ) {}
	//
	virtual bool CanDo( const SPlaceWithAP &place ) const;
	virtual void Do( IAILogContainer *pLog ) const;
	virtual bool ComparePlaces( const SPlaceWithAP &p1, const SPlaceWithAP &p2 ) const;
	//
	void GetInfo( const SPlaceWithAP &place, SAIThrowGrenadeActionInfo *pInfo ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILaunchRocketAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILaunchRocketAction: public CAIAction
{
public:
	struct SAILaunchRocketActionInfo
	{
		bool bCanDo;
		CVec3 ptTarget;
		int nTargetSize;
		bool bNeedReload;
		float fDistance;
		CPtr<CAIFireArmsWeapon> pWeapon;
		SAILaunchRocketActionInfo(): bCanDo( false ), bNeedReload( false ) {}
	};
	//
private:
	OBJECT_BASIC_METHODS( CAILaunchRocketAction );
	ZDATA
	ZPARENT( CAIAction );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIAction *)this); return 0; }
	//
public:
	CAILaunchRocketAction() {}
	CAILaunchRocketAction( IAIState *_pState ): CAIAction( _pState ) {}
	//
	virtual bool CanDo( const SPlaceWithAP &place ) const;
	virtual void Do( IAILogContainer *pLog ) const;
	virtual bool ComparePlaces( const SPlaceWithAP &p1, const SPlaceWithAP &p2 ) const;
	//
	void GetInfo( const SPlaceWithAP &place, SAILaunchRocketActionInfo *pInfo ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif
