#ifndef __RPGALLCRITICALS_H_
#define __RPGALLCRITICALS_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "RPGCritical.h"
#include "RPGUnitMission.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// модификация статов в зависимости от VP
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVPCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CVPCritical);
public:
	CVPCritical() {}
	CVPCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAPCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CAPCritical);
public:
	CAPCritical() {}
	CAPCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWeaponSkillCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CWeaponSkillCritical);
public:
	CWeaponSkillCritical() {}
	CWeaponSkillCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDeathCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CDeathCritical);
public:
	CDeathCritical() {}
	CDeathCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMotionlessCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CMotionlessCritical);
	struct SModif
	{
		ZDATA
		CPtr<IUnitMission> pRPGMission;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pRPGMission); return 0; }
		SModif(): pRPGMission(0) {}
		~SModif();
	};
	ZDATA_(CCritical)
	SModif data;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCritical*)this); f.Add(2,&data); return 0; }
	
	CMotionlessCritical() {}
	CMotionlessCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLostWeaponCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CLostWeaponCritical);
public:
	CLostWeaponCritical() {}
	CLostWeaponCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CIdleHandCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CIdleHandCritical);
	struct SModif
	{
		ZDATA
		CPtr<IUnitMission> pRPGMission;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pRPGMission); return 0; }
		SModif(): pRPGMission(0) {}
		~SModif();
	};
	ZDATA_(CCritical)
	SModif data;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCritical*)this); f.Add(2,&data); return 0; }

	CIdleHandCritical() {}
	CIdleHandCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAccidentalShotCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CAccidentalShotCritical);
public:
	CAccidentalShotCritical() {}
	CAccidentalShotCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDamageWeaponCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CDamageWeaponCritical);
public:
	CDamageWeaponCritical() {}
	CDamageWeaponCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CBlindCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CBlindCritical);
	struct SModif
	{
		ZDATA
		CPtr<IUnitMission> pRPGMission;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pRPGMission); return 0; }
		SModif(): pRPGMission(0) {}
		~SModif();
	};
	ZDATA_(CCritical)
	SModif data;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCritical*)this); f.Add(2,&data); return 0; }

	CBlindCritical() {}
	CBlindCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStunCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CStunCritical);
	ZDATA_(CCritical)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCritical*)this); return 0; }

	CStunCritical() {}
	CStunCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPatientCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CPatientCritical);
	ZDATA_(CCritical)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCritical*)this); return 0; }

	CPatientCritical() {}
	CPatientCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission ) { return true; }
	virtual int Compare( SCritical *pCritical ) const { return OTHER; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDeafCritical: public CCritical
{
	OBJECT_BASIC_METHODS(CDeafCritical);
	struct SModif
	{
		ZDATA
		CPtr<IUnitMission> pRPGMission;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pRPGMission); return 0; }
		SModif(): pRPGMission(0) {}
		~SModif();
	};
	ZDATA_(CCritical)
	SModif data;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCritical*)this); f.Add(2,&data); return 0; }

	CDeafCritical() {}
	CDeafCritical( const SCritical &crit ): CCritical( crit ) {}

	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __RPGALLCRITICALS_H_