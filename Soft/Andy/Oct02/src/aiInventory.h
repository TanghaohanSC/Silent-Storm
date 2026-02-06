#ifndef __AIINVENTORY_H_
#define __AIINVENTORY_H_

namespace NDB
{
	enum EShootMode;
}

namespace NRPG
{
	class IUnitMission;
	class IInventoryItem;
	class CAttackPortion;
}

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIUnit;
class IAIClip;
class IAIWeapon;
class IAIWeaponClip;
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIInventory
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIInventory: public CObjectBase
{
public:
	virtual void AddWeapon( IAIWeapon *pWeapon ) = 0;
	virtual void RemoveWeapon( IAIWeapon *pWeapon, bool bRemoveClips = false ) = 0;
	virtual void AddClip( IAIWeaponClip *pClip ) = 0;
	virtual IAIWeapon *GetBestWeapon( IAIUnit *pTarget, int nAP, int *nBestWeaponHitCover, 
		int *nQuality, NDb::EShootMode *eShootMode, int *nMaxToHit, bool bLog = false ) = 0;
	virtual NRPG::CAttackPortion GetAttackPortion() = 0;
 	virtual void SetCurrentWeapon( IAIWeapon *pWeapon ) = 0;
 	virtual IAIWeapon *GetCurrentWeapon() = 0;
	virtual int GetWeaponCount() = 0;
	virtual IAIWeapon* GetSuitableWeapon( IAIWeaponClip *pClip ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIInventory *CreateAIInventory( IAIUnit *_pOwner );
////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif