#ifndef __AIWEAPON_H_
#define __AIWEAPON_H_
//
namespace NRPG
{
	class CClipItem;
	class CWeaponItem;
	class CGrenadeItem;
	class IInventoryItem;
}
//
namespace NDb
{
	enum EShootMode;
}
//
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EPose;
class IAIUnit;
class CAILogRecord;
struct SUnitPosition;
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIInventoryItem: public CObjectBase
{
public:
	virtual NRPG::IInventoryItem* GetInventoryItem() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIFireArmsWeaponClip: public CObjectBase
{
	OBJECT_BASIC_METHODS( CAIFireArmsWeaponClip );
	ZDATA
	CPtr<NRPG::CClipItem> pClipItem;
	int nAmmoCount;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pClipItem); f.Add(3,&nAmmoCount); return 0; }
	//
public:
	CAIFireArmsWeaponClip() {}
	CAIFireArmsWeaponClip( NRPG::CClipItem *_pClipItem );
	//
	int GetAmmoCount() const;
	void SetAmmoCount( int nCount );
	void SpendAmmo( int nCount );
	bool IsEmpty() const;
	NRPG::CClipItem* GetItem() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIFireArmsWeapon: public IAIInventoryItem
{
	OBJECT_BASIC_METHODS( CAIFireArmsWeapon );
	ZDATA
	CPtr<NRPG::CWeaponItem> pWeaponItem;
	CObj<CAIFireArmsWeaponClip> pCurrentClip; // обойма заряженная в оружие
	vector< CObj<CAIFireArmsWeaponClip> > clips; // имеющиеся обоймы ( заряженная не входит )
	CPtr<IAIUnit> pOwner; // у кого в inventory лежит
	ZEND int operator&( CStructureSaver &f ) { return 0; }
	//
	bool IsBurstMode( NDb::EShootMode eShotMode ) const;
	int GetMeanDamage() const;
	int GetMinAPToShoot( int nUnitAP ) const;
	int GetLongBurstAmmoCountPerShot( int nUnitAP ) const;
	int GetAmmoCountPerShot( int nUnitAP ) const;
	int GetAmmoCountPerAP( int nAP ) const;
	//
public:
	CAIFireArmsWeapon() {}
	CAIFireArmsWeapon( IAIUnit *_pOwner, NRPG::CWeaponItem *_pWeaponItem );
	//
	int GetShotAP() const;
	int GetBurstAP() const;
	int GetReloadAP() const;
	//
	NRPG::CWeaponItem* GetItem() const { return pWeaponItem; }
	virtual NRPG::IInventoryItem* GetInventoryItem() const;
	bool IsRocketLauncher() const;
	// clip management
	int GetClipCount() const;
	CAIFireArmsWeaponClip* GetCurrentClip() const;
	void SetCurrentClip( CAIFireArmsWeaponClip *pClip );
	CAIFireArmsWeaponClip* GetNextClip() const;
	void RemoveClip( CAIFireArmsWeaponClip *pClip );
	void AddClip( CAIFireArmsWeaponClip *pClip );
	bool IsSuitableClip( CAIFireArmsWeaponClip *pClip ) const;
	//
	bool IsSameWeapon( CAIFireArmsWeapon *pAIWeapon ) const;
	void GetShotParameters( const NAI::SUnitPosition &pos, IAIUnit *pTarget, 
		int nHitCover, int nAvailableAP, int *nAP, int *nAmmo, int *nDamage, bool *bNeedReload ) const;
	int GetDamage( const NAI::SUnitPosition &pos, IAIUnit *pTarget, 
		int nHitCover, NAI::EPose ePose, int nAP, NDb::EShootMode eShootMode, int *nMaxToHit ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIGrenadeWeapon: public IAIInventoryItem
{
	OBJECT_BASIC_METHODS( CAIGrenadeWeapon );
	ZDATA
	CPtr<NRPG::CGrenadeItem> pGrenade;
	ZEND
	//
public:
	CAIGrenadeWeapon() {}
	CAIGrenadeWeapon( NRPG::CGrenadeItem *_pGrenade ): pGrenade( _pGrenade ) {}
	//
	NRPG::CGrenadeItem* GetItem() const { return pGrenade; }
	virtual NRPG::IInventoryItem* GetInventoryItem() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIFireArmsWeaponClip* CreateAIFireArmsWeaponClip( NRPG::CClipItem *pItem );
CAIFireArmsWeapon* CreateAIFireArmsWeapon( IAIUnit *pOwner, NRPG::CWeaponItem *pItem );
CAIGrenadeWeapon* CreateAIGrenadeWeapon( NRPG::CGrenadeItem *pItem );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif