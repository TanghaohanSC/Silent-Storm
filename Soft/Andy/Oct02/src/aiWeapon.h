#ifndef __AIWEAPON_H_
#define __AIWEAPON_H_

namespace NRPG
{
	class CClipItem;
	class CWeaponItem;
	class CGrenadeItem;
	class IUnitMission;
	class IInventoryItem;
}

namespace NDb
{
	class CRPGClip;
	class CRPGWeapon;
	enum EShootMode;
}

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIUnit;
class CAILogRecord;
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIWeaponClip
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIWeaponClip: public CObjectBase
{
public:
	virtual int GetAmmoCount() = 0; // количество зарядов в обойме
	virtual void SetAmmoCount( int nCount ) = 0;
	virtual void SpendAmmo( int nCount ) = 0; // выстрелить nCount патронов
	virtual bool IsEmpty() = 0; // пустая обойма
	virtual NRPG::IInventoryItem *GetInventoryItem() = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIWeapon
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIWeapon: public CObjectBase
{
public:
	virtual int GetDamage() = 0; // повреждения от одной пули
	virtual int GetClipCount() = 0; // количество имеющихся обойм к оружию 
	virtual int GetAmmoCountPerShot( int nUnitAP, int nUnitMaxAP ) = 0;
	virtual int GetAmmoCountPerAP( int _nAP ) = 0;
	virtual void AddClip( IAIWeaponClip *pClip ) = 0;
	virtual void RemoveClip( IAIWeaponClip *pClip ) = 0;
	virtual void RemoveClips() = 0;
	virtual int GetDamage( IAIUnit *pTarget, int nHitCover, 
		NAI::EPose ePose, int nAP, NDb::EShootMode eShootMode, int *nMaxToHit ) = 0;
	virtual bool IsSuitableClip( IAIWeaponClip *pClip ) = 0;
	virtual bool IsInHands() = 0; // взято-ли в руки
	virtual void TakeInHands( bool _bInHands ) = 0; // взять в руки
	virtual int GetReloadAP() = 0; // количество AP для перезарядки
	virtual int GetShotAP() = 0; // количество AP для выстрела
	virtual NRPG::IInventoryItem *GetInventoryItem() = 0;
	virtual IAIWeaponClip *GetCurrentClip() = 0;
	virtual void SetCurrentClip( IAIWeaponClip *pClip ) = 0;
	virtual IAIWeaponClip *GetNextClip() = 0;
	virtual void GetShotParameters( int nMaxAP, int nHitCover,
		int *nAP, int *nAmmo, IAIUnit *pTarget, int *nDamage, bool *bNeedReload ) = 0;
	virtual bool IsSameWeapon( IAIWeapon *pAIWeapon ) = 0;
	virtual CAILogRecord *GetAttackCommand( IAIUnit *pAIUnit, IAIUnit *pAITarget ) = 0;
	virtual void DebugOutput() = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeaponClip *CreateAIFireArmsWeaponClip( NRPG::CClipItem *pClip );
IAIWeaponClip *CreateAIGrenadeWeaponClip( NRPG::CGrenadeItem *_pGrenade );
IAIWeapon *CreateFireArmsAIWeapon( IAIUnit *_pOwner, NRPG::CWeaponItem *_pWeapon );
IAIWeapon *CreateGrenadeAIWeapon( IAIUnit *_pOwner, NRPG::CGrenadeItem *_pGrenade );
////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif