#ifndef __RPGCRITICAL_H_
#define __RPGCRITICAL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RPGUnit.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
	enum EHitLocation;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SCritical
{
	NDb::ECriticalLocation eCl;
	NDb::ECritical eCritical;
	int   nDuration;
	float fValue;
	int   nDC; // Difficulty Class

	SCritical() {}
	SCritical( NDb::ECriticalLocation hl, NDb::ECritical cr, int _nDuration = -1, float _fValue = 0, int _nDC = N_MAX_DC )
		: eCl(hl), eCritical(cr), nDuration(_nDuration), fValue(_fValue), nDC(_nDC) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnit;
class CSkillModifier;
class IUnitMission;
//enum ECriticalAction;
class CCritical: public CObjectBase
{
	OBJECT_BASIC_METHODS(CCritical);

protected:
	void PushModifier( CSkillModifier *pModifier ) { modifiers.push_back( pModifier ); }

private:
	ZDATA
	int nTurn;
	vector<CPtr<CSkillModifier> > modifiers;
protected:
	SCritical critical;

public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nTurn); f.Add(3,&modifiers); f.Add(4,&critical); return 0; }

	CCritical() {}
	CCritical( const SCritical &crit );

	bool NextTurn();		// true пока время действия данного critical'а не истекло
	int  GetRemainingTime() const;
	virtual NDb::ECritical GetCriticalType() const { return critical.eCritical; }
	bool IsTemporarily() const { return critical.nDuration >= 0; }
	virtual bool SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission  ) // false, если небыло повешено никаких модификаторов
	{
		OutputDebugString( "Empty critical\n" );
		return false;
	}

	enum { WEAKER, EQUAL, STRONGER, OTHER }; // результаты функции Compare
	virtual int Compare( SCritical *pCritical ) const;
	const SCritical& GetCritical() const { return critical; }
	float GetModifier() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CCritical* CreateCritical( const SCritical &critical );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //__RPGCRITICAL_H_
