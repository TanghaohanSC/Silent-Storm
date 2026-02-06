#ifndef __AIINVENTORY_H_
#define __AIINVENTORY_H_
//
namespace NDB
{
	enum EShootMode;
}
//
namespace NRPG
{
	class IUnitMission;
	class CAttackPortion;
	class IInventoryItem;
	class CGrenadeItem;
}
//
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIUnit;
class CAIFireArmsWeapon;
class CAIFireArmsWeaponClip;
class CAIGrenadeWeapon;
class IAIInventoryItem;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIInventory
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIInventory: public CObjectBase
{
	OBJECT_BASIC_METHODS(CAIInventory);
	ZDATA
	CPtr<IAIUnit> pOwner;
	vector< CObj<CAIFireArmsWeapon> > fireArms;
	vector< CObj<CAIGrenadeWeapon> > grenades;
	vector< CObj<CAIFireArmsWeapon> > rocketLaunchers;
	CObj<IAIInventoryItem> pCurrent;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pOwner); f.Add(3,&fireArms); f.Add(4,&grenades); f.Add(5,&rocketLaunchers); f.Add(6,&pCurrent); return 0; }
	//
	void GetInventoryItems( list< CPtr<NRPG::IInventoryItem> > *pItems ) const;
	void FetchInventoryItems();
	CAIFireArmsWeapon* GetSuitableWeapon( CAIFireArmsWeaponClip *pClip ) const;
	IAIInventoryItem* GetAIInventoryItem( NRPG::IInventoryItem *pItem ) const;
	//
public:
	CAIInventory() {}
	CAIInventory( IAIUnit *_pOwner );
	//
	void AddFireArms( CAIFireArmsWeapon *pWeapon );
	void RemoveFireArms( CAIFireArmsWeapon *pWeapon );
	void AddRocketLaunchers( CAIFireArmsWeapon *pWeapon );
	void RemoveRocketLaunchers( CAIFireArmsWeapon *pWeapon );
	void AddGrenade( CAIGrenadeWeapon *pGrenade );
	void RemoveGrenade( CAIGrenadeWeapon *pGrenade );
	void AddClip( CAIFireArmsWeaponClip *pClip );
	//
	void SetCurrentItem( IAIInventoryItem *pItem );
	IAIInventoryItem* GetCurrentItem() const;
	CAIFireArmsWeapon* GetCurrentFireArms() const;
	CAIGrenadeWeapon* GetCurrentGrenade() const;
	CAIFireArmsWeapon* GetCurrentRocketLauncher() const;
	bool IsCurrentItem( IAIInventoryItem *pItem ) const;
	//
	CAIFireArmsWeapon* GetBestFireArms( const NAI::SUnitPosition &pos, IAIUnit *pTarget, 
		int nAP, int *pBestWeaponHitCover, int *pQuality,	NDb::EShootMode *shootMode, int *nMaxToHit ) const;
	CAIGrenadeWeapon* GetBestGrenade( CVec3 ptTarget ) const;
	CAIFireArmsWeapon* GetBestRocketLaunchers() const;
	bool HasAnyWeapon() const;
	//
	void DebugOutput() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIInventory* CreateAIInventory( IAIUnit *pOwner );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif