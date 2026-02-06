#ifndef __RPGITEM_H_
#define __RPGITEM_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "RPGItemInfo.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
	class CRPGClip;
}
namespace NRPG
{
class CUnit;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Базовый интерфейс для перекладывания
class IJoinSplit: virtual public CObjectBase
{
public:
	virtual bool Join( IJoinSplit *pItem ) = 0;
	virtual IJoinSplit *Split( int nQuantityToGet = 1 ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IItemContainer: virtual public IItemContainerInfo
{
public:
	virtual IJoinSplit* SplitItem( int nQ ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// Для хранения предметов
class IInventory: public IInventoryInfo
{
public:
	virtual void Take( IInventoryItem *pItem ) = 0;
	virtual bool Place( const CTPoint<int> &sPos, IInventoryItem *pItem ) = 0;

	virtual bool Wear( IInventoryItem *pWhat ) = 0;
	virtual bool Equip( NDb::ESlot where, IInventoryItem *pWhat ) = 0;
	virtual IInventoryItem *UnWear() = 0;
	virtual IInventoryItem *TakeOff( NDb::ESlot where ) = 0;

	virtual bool Activate( NDb::ESlot where ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class IWeaponItem: virtual public IWeaponItemInfo
{
public:
	virtual bool HasAmmo() const = 0;
	virtual void CreateNewAttackPortion( vector<CAttackPortion> *pRes, bool bSpendAmmo ) = 0;
	virtual bool IsWorking() const = 0;
	virtual bool CanReload( IInventoryInfo *pInventory ) const = 0;

	virtual NDb::EShootMode GetShootMode() const = 0;
	virtual bool SetShootMode( NDb::EShootMode eShootMode ) = 0;
	virtual bool IsShootModeSupported( NDb::EShootMode eMode ) const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IInventory *CreateInventory( CUnit *pOwner );
////////////////////////////////////////////////////////////////////////////////////////////////////
IInventoryItem *CreateItem( const CDBRecord *pItem );
IWeaponItem *CreateWeaponItem( NDb::CRPGWeapon *pDBWeapon );
IInventoryItem *CreateMeleeWeaponItem( NDb::CRPGMeleeWeapon *pDBMeleeWeapon );
IInventoryItem *CreateClipItem( NDb::CRPGClip *pDBClip, NDb::CRPGAmmo *pDBAmmo = 0, int nAmmoQuantity = - 1 );
IInventoryItem *CreateGrenadeItem( NDb::CRPGGrenade *pDBGrenade );
IInventoryItem *CreateUniformItem( NDb::CRPGUniform *pDBUniform );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif