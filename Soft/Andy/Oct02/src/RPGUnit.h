#ifndef __RPGUNIT_H_
#define __RPGUNIT_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GSkeleton.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
	enum EPose;
}
namespace NScript
{
	class CContext;
}
namespace NDb
{
	class CModel;
	class CRPGClass;
	class CRPGPers;
	enum EWeaponType;
	enum ESkillType;
	class CAnimWeaponType;
}
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IInventory;
class CWeaponItem;
class CMeleeWeaponItem;
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_MAX_SKILL = 140;
const int N_MAX_VP = 250;
const int N_MAX_DC = 300;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Skill который может меняется, это HP, AP
class CDynamicSkill: public CObjectBase
{
	OBJECT_BASIC_METHODS(CDynamicSkill);
private:
	ZDATA
	int nBaseValue; // базовое значение вычесляемое исходя из stat
	int nMaxValue;  // максимально возможное значение скила на этот момент времени, включающие базовую величину
	int nValue;		// текущие значение скила, включающие в себя базовую величину
	float fMultiplier; // текушие критикалы
	float fProgress;	// Прогресс до следующей +1 к скиллу
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nBaseValue); f.Add(3,&nMaxValue); f.Add(4,&nValue); f.Add(5,&fMultiplier); f.Add(6,&fProgress); return 0; }
public:
	CDynamicSkill( int nSetValue = 0, int nBaseStatValue = 0 ): 
		nBaseValue(nBaseStatValue), nValue(nSetValue), nMaxValue(nSetValue), fMultiplier( 1 ), fProgress( 0 )
	{
		if ( nBaseValue > nMaxValue )
			nMaxValue = nBaseValue;
		fMultiplier = 1.0f;
	}

	void SetNewMaxValue( int nNewValue ) { nValue += nNewValue - nMaxValue; nMaxValue = nNewValue; }
	void SetNewBaseValue( int nNewValue ) { nValue += nNewValue - nBaseValue; nBaseValue = nNewValue; }
	void Modify( int nModif ) { nValue += nModif; nMaxValue += nModif; }
	bool Upgrade( float fAddToProgress );
	int  GetXPPart() const { return nMaxValue - nBaseValue;} // Значение скила зависящее только от XP
	void SetXPPart( int nNewXPPart ) { nValue += nNewXPPart - GetXPPart(); nMaxValue = nBaseValue + nNewXPPart; }
	void Multiply( float fValue );
	
	float GetProgress() const { return fProgress; }
	int GetMaxValue() const { return nMaxValue; }
	int GetCurrentMaxValue() const { return fMultiplier * nMaxValue; }
	void Reset() { nValue = fMultiplier * nMaxValue; }
	void SetValue( int n ) { nValue = n; }
	void SetProgress( float p ) { fProgress = p; }
	
	const CDynamicSkill& operator += ( int n ) { nValue += n; if ( nValue > nMaxValue ) nValue = nMaxValue; return *this; }
	const CDynamicSkill& operator -= ( int n ) { nValue -= n; return *this; }
	operator int () const { return nValue; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkillModifier: public CObjectBase
{
	OBJECT_BASIC_METHODS(CSkillModifier);
	struct SModif
	{
		CPtr<CDynamicSkill> pSkill;
		float fMultiplier; // временный модификатор

		SModif(): fMultiplier(1) {}
		SModif( CDynamicSkill *_pSkill, float fModif ) : pSkill(_pSkill), fMultiplier(fModif) { pSkill->Multiply(fModif); }
		~SModif() { pSkill->Multiply(1.0f/fMultiplier); }
	}data;
public:
	CSkillModifier() {}
	CSkillModifier( CDynamicSkill *pSkill, float fMultiplier ) : data( pSkill, fMultiplier) {}
	void Set( float fMultiplier ) 
	{ 
		data.pSkill->Multiply( 1.0f/data.fMultiplier ); 
		data.fMultiplier = fMultiplier; 
		data.pSkill->Multiply( data.fMultiplier ); 
	}
	float Get() { return data.fMultiplier; }

	int operator&( CStructureSaver &f )
	{
		f.Add( 1, &data.fMultiplier );
		f.Add( 2, &data.pSkill );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//! CUnit содердит данные которые есть у персонажа в миссии и на базе
class CUnit: public CObjectBase
{
	OBJECT_BASIC_METHODS(CUnit);
public:
	ZDATA
	int nRPGPersID;		
	int nXP;					  // Опыт данного персонажа
	vector< CObj<CDynamicSkill> > skills; // его скилы

	CObj<IInventory> pInventory;
	CDBPtr<NDb::CRPGClass> pClass;
	CPtr<NDb::CModel> pModel;
	CPtr<CWeaponItem> pCannonItem; // CRAP
	CPtr<CMeleeWeaponItem> pDefaultWeapon;
	CDBPtr<NDb::CRPGPers> pPers;
	int nHealedVP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nRPGPersID); f.Add(3,&nXP); f.Add(4,&skills); f.Add(5,&pInventory); f.Add(6,&pClass); f.Add(7,&pModel); f.Add(8,&pCannonItem); f.Add(9,&pDefaultWeapon); f.Add(10,&pPers); f.Add(11,&nHealedVP); return 0; }
	CObj<NScript::CContext> pScript; // его скрипт
	//
	CUnit();
	CUnit( NDb::CRPGPers *pPers );
	
	void AddXP( int nXPToAdd );
	CDynamicSkill& Skills( const int eSkill ) { return *skills[eSkill]; }
	bool UseSkill( const int eSkill );
	int GetSkillBaseStatValue( const int eSkill );

	NDb::CRPGPers* GetPers() const;
	NDb::EWeaponType GetWeaponType() const;
	NDb::CAnimWeaponType* GetAnimWeaponType() const;
	NDb::ESkillType  GetWeaponSkill() const;
	int GetWeaponAP() const;
	int GetWeaponBurstAP() const;
	int GetWeaponReloadAP() const;
	IInventory *GetInventory() { return pInventory; }
	void SetCannonItem( CWeaponItem *pItem ) { pCannonItem = pItem; }
	CWeaponItem* GetCannonItem() { if ( !IsValid(pCannonItem) ) pCannonItem = 0; return pCannonItem; }
	//
	CWeaponItem* GetWeaponItem() const;
	CMeleeWeaponItem* GetMeleeWeaponItem() const;

	int GetSkillCap( NDb::ESkillType eSkill, int nXP );
	int GetXPForSkill( NDb::ESkillType eSkill, int nLvl );
	bool IsDead();
	void Kill();
	void SetXPLevel( int nLevel );
};
int GetSkillByCap( int nCap, int nXP );
int GetXPBySkill( int nCap, int nLvl );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif