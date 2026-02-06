#ifndef __RPGMISSION_H_
#define __RPGMISSION_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GSkeleton.h"
#include "RPGUnitInfo.h"
#include "Grid.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_MELEE_DISTANCE = FP_GRID_STEP * SQRT_2 + 0.01f;
namespace NAI
{
	class IAIMap;
	enum EPose;
	enum EHitLocation;
}
namespace NDb
{
	class CRPGWeapon;
	class CRPGPers;
	class CAISound;
	class CModel;
	class CPanzerklein;
	struct SToHitConstants;
	struct SAISoundConstants;
	struct SInterruptsConstants;
	class CRPGMine;
	enum ECriticalLocation;
	enum ECritical;
}
namespace NRPG
{
struct SUnitInfo; // data about any unit that can be shown in interface
class CAttackPortion;
class IInventory;
class IInventoryInfo;
class IInventoryItem;
class IClipItem;
class IWeaponItem;
class CUnit;
class CCritical;
struct SCritical;
struct SDiplomacy;
struct SFirstAid;
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ECriticalState
{
	CS_ENABLED = 0,
	CS_DISABLED, // выполнение невозможно
	CS_REROLL // надо выбрать другой критикал
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSnipeAP
{
	ZDATA
	int nAP;
	CPtr<IUnitMissionInfo> pTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nAP); f.Add(3,&pTarget); return 0; }
	SSnipeAP( int _nAP = 0, IUnitMissionInfo *_pTarget = 0 ): nAP(_nAP), pTarget(_pTarget ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// mission time RPG information handler
class CDynamicSkill;
class IUnitMission: public IUnitMissionInfo
{
public:
	virtual void SpendAP( int nAP ) = 0;
	virtual void RegisterAction( EAction action ) = 0;
	virtual const SSnipeAP& GetSavedAP() const = 0;
	virtual void SaveAP( const SSnipeAP &ap ) = 0;
	virtual void StartNewTurn( const CVec3 &ptCP ) = 0;
	virtual bool CheckIC() = 0;	// Return true ecли он сумел увернуться
	virtual void Kill() = 0;
	virtual bool IsDead() const = 0;
	virtual IInventory* GetInventory() const = 0;
	virtual bool CreateAttack( vector<CAttackPortion> *pRes, bool bSpendAmmo, 
		bool bAnonymous = true, IUnitMissionInfo *pTarget = 0, bool bBackStab = false ) = 0;
	virtual int ProcessAttack( int nUserID, CAttackPortion *pAttack, NDb::CRPGArmor *pArmor ) = 0;
	virtual void Seat() = 0;
	virtual void Stand() = 0;
	virtual bool CanMove() const = 0;
	virtual void Reload() = 0;
	virtual bool LoadWeapon( IWeaponItemInfo *pWeapon, IClipItem *pClip ) = 0;
	virtual bool UnloadWeapon( IWeaponItemInfo *pWeapon ) = 0;
	virtual void StartAttack() = 0;					// Burst
	virtual void NextBullet() = 0;					// Burst
	virtual int  GetNBullets() const = 0;		// Burst
	virtual void AddLastCritical( NDb::ECritical eCA ) = 0; // for Criticals
	virtual void GetLastCriticals( vector<NDb::ECritical> *pResCritical ) = 0;
	virtual bool HasCritical( NDb::ECritical eCritical, CCritical** ppCritical = 0 ) const = 0;
	virtual void UseTwoHanded( bool bUse ) = 0;
	virtual bool CanUseTwoHanded() const = 0;
	virtual void Blind( bool bBlind ) = 0;
	virtual void Deaf( bool bDeaf ) = 0;
	virtual int GetRPGPersID() const = 0;
	virtual void SetCannonItem( IWeaponItem *pItem ) = 0;
	virtual IWeaponItem* GetCannonItem() const = 0;
	virtual IWeaponItem* GetWeaponItem() const = 0;
	virtual bool CanHearSound( const CVec3 &ptSoundPosition, const CVec3 &ptListenerPosition,
		NDb::CAISound *pSound, int nAISoundType, IUnitMission *pSource ) = 0;
	virtual NDb::SToHitConstants *GetToHitConstants() = 0;
	virtual NDb::SAISoundConstants *GetAISoundConstants() = 0;
	virtual NDb::SInterruptsConstants *GetInterruptsConstants() = 0;
	virtual int GetHealedVP() const = 0;
	virtual int GetTotalVP() const = 0;
	virtual int GetLastActionTimes() const = 0;
	virtual int GetMoveInLastTurn() const = 0;
	virtual void ApplyCritical( const NRPG::SCritical &critical ) = 0;
	virtual bool RemoveCritical( NDb::ECritical eCritical ) = 0;
	virtual void SuspendCriticals( int nTurns ) = 0;
	virtual void MakeDirectDamage( int nDmg ) = 0; // CRAP
	virtual void EnableCriticals() = 0;
	virtual void DisableCriticals() = 0;
	virtual void DisableCritical( NDb::ECritical eC, ECriticalState eState  ) = 0;
	virtual int  CheckInterrupt( const IUnitMission *pEnemy, bool bIsMutual, bool bWasShot ) = 0;
	virtual void BulletHit() = 0;
	virtual bool GetAck( int *pAckID, IUnitMissionInfo ** ppAttacker ) = 0;
	virtual float GetXP( int nHowManyPerson ) const = 0;
	virtual void StartRealTime() = 0;
	virtual const SDiplomacy& GetDiplomacy() const = 0;
	virtual void SetDiplomacy( const SDiplomacy &dip ) = 0;
	virtual bool IsUnconscious() = 0;
	virtual void InitAsCorpse( bool bDead ) = 0;
	virtual int GetFallDamage( float fHDiff ) = 0;
	virtual NDb::CPanzerklein *GetPanzerklein() = 0;
	virtual void SetPanzerklein( NDb::CPanzerklein *pPK, CDynamicSkill *_pPanzerkleinVP, IInventory *_pPKInventory ) = 0; 
	virtual void DoRegenerations() = 0;
	virtual void SetHiding( bool _bHiding ) = 0;
	virtual void HealVP( const SFirstAid &fa ) = 0;
	virtual void HealCriticals( int nDC ) = 0;
	virtual bool HasPerk( int nPerkID, 
		float *pParam1 = 0, float *pParam2 = 0, float *pParam3 = 0 ) const = 0;
	virtual int GetGrenadeTrapDC( NDb::CRPGGrenade *pGrenade ) = 0;
	virtual int GetMineDC( NDb::CRPGMine *pMine ) = 0;
	virtual bool CanSeeMine( float fDistance, int nDC ) = 0;
	virtual bool CanClear( int nDC, int nSkillModif ) = 0;
	virtual int GetUnhideProbability( IUnitMission *pTarget, float fDistance ) const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
float GetCubesArea( const CVec3 &ptPos, vector<CVec3> *pCubes );
int GetHLPenalty( NAI::EHitLocation hl );
float GetVPPenalty( int nVP, int nHealedVP, int nMaxVP );
extern void DumpCritical( CCritical *p );
extern void DumpStats( CUnit *p, int nHealedVP );
extern const string& GetCLName( NDb::ECriticalLocation cl );
extern const string& GetHLName( NAI::EHitLocation cl );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif