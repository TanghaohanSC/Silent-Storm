#ifndef __RPGUNITINFO_H_
#define __RPGUNITINFO_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
	enum EWeaponType;
	enum ESkillType;
	enum ECritical;
	enum ECriticalLocation;
	class CModel;
	class CRPGArmor;
	class CRPGPers;
	class CRPGGrenade;
	class CAnimWeaponType;
	class CComplexHead;
}
namespace NAI
{
	enum EPose;
	enum EHitLocation;
	enum ETileHitLocation;
	struct SPosition;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// information about unit for interface
struct SUnitInfo
{
public:
	int nHP, nHealedHP, nMaxHP;
	int nAP, nMaxAP;
	int nSightDistance;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// ƒействи€ которые умеет выполн€ть RPG unit
enum EAction // Ќе измен€ть этот enum и даже не мен€ть в нЄм строчки местами, без согласовани€ с Epik-ом! (или добавл€ть только в конец)
{
	AC_NONE,
	AC_MOVE_SIDE,
	AC_MOVE_DIAGONAL,
	AC_MOVE_CORPSE_SIDE,
	AC_MOVE_CORPSE_DIAGONAL,
	AC_ROTATE,
	AC_PREPARE_AND_SHOOT,
	AC_SHOOT,
	AC_EXPLODE,
	AC_POSE_CRAWL,
	AC_POSE_CROUCH,
	AC_POSE_WALK,
	AC_POSE_RUN,
	AC_THROW_GRENADE,
	AC_CLIMB_1,
	AC_CLIMB_2,
	AC_CLIMB_3,
	AC_CLIMB_4,
	AC_JUMP,
	AC_FIRSTAID,
	AC_MELEE,
	AC_TAKE_CORPSE,
	AC_BURST,
	AC_RELOAD,
	AC_OPEN_CLOSE,
	AC_APPROACH_CANNON,
	AC_LADDER,
	AC_LADDER_MOVE,
	AC_END_SHOOT,
	AC_THROW_KNIFE,
	AC_PREPARE,
	AC_ENTER_PK,
	AC_LEAVE_PK,
	AC_HIDE,
	AC_TRAP_OBJECT,
	AC_DISARM_TRAP,
	AC_SET_MINE,
	AC_DISARM_MINE
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// Critical info
class ICriticalInfo: public CObjectBase
{
public:
	virtual int GetDifficultyClass() const = 0;
	virtual NDb::ECritical GetCriticalType() const = 0;
	virtual NDb::ECriticalLocation GetCriticalLocation() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// mission time RPG information handler
class CUnit;
class IInventoryInfo;
class IWeaponItemInfo;
class IUnitMissionInfo: virtual public CObjectBase
{
public:
	virtual int GetActionAP( NAI::EPose pose, EAction action ) const = 0;
	virtual bool CanSpendAP( int nAP ) const = 0;
	virtual int GetMaxExtraAP() const = 0;
	virtual int GetAP() const = 0;

	virtual int GetInterrupt() const = 0;
	virtual int GetIC() const = 0;
	virtual int GetToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, const NAI::SPosition &posTarget, 
		NAI::EHitLocation eHL, int nExtraAP, IUnitMissionInfo *pTarget, const vector<int> &accessibleHLs, 
		int nHitCover, bool bFirstRound, const CVec3 &ptIllumination = CVec3(1,1,1), bool bBackstab = false ) const = 0;
	virtual int GetGrenadeToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, 
		bool bFirstRound, CVec3 ptTilePos, NDb::CRPGGrenade *pGrenade, const CVec3 &ptIllumination = CVec3(1,1,1) ) = 0;
	virtual int GetTileToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, 
		CVec3 ptTilePos, NAI::ETileHitLocation eHitLocation, int nExtraAP, int nHitCover, 
		bool bFirstRound, const CVec3 &ptIllumination = CVec3(1,1,1) ) = 0;
	virtual int GetRLauncherToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, 
		CVec3 ptTilePos, NAI::ETileHitLocation eHitLocation, int nExtraAP, 
		bool bFirstRound, const CVec3 &ptIllumination = CVec3(1,1,1) ) = 0;

	virtual int  GetBulletsQuantityInShot() const = 0;
	virtual void GetInfo( NAI::EPose pose, SUnitInfo *pInfo ) const = 0;
	virtual float GetSightDistance( NAI::EPose pose ) const = 0;
	virtual const wstring& GetName() const = 0;
	virtual NDb::CModel* GetModel() const = 0;

	virtual IInventoryInfo* GetInventoryInfo() const = 0;
	virtual NDb::EWeaponType GetWeaponType() const = 0;
	virtual IWeaponItemInfo* GetCannonItemInfo() const = 0;
	virtual NDb::CAnimWeaponType* GetDBAnimWeapon() const = 0;

	virtual NDb::CRPGArmor* GetRPGArmor() const = 0;
	virtual int GetSkillValue( NDb::ESkillType skill ) const = 0;
	virtual int GetSkillMaxValue( NDb::ESkillType skill ) const = 0;
	virtual float GetSkillProgress( NDb::ESkillType skill ) const = 0;
	virtual void DumpStats() const = 0;
	virtual void PrintLog( bool bPrint ) {};
	virtual CVec3 GetTurnStartCP() const = 0;
	virtual NDb::CRPGPers* GetRPGPers() const = 0;
	virtual NDb::CComplexHead* GetRPGPersHead() const = 0;
	virtual NRPG::CUnit* GetRPGUnit() const = 0;

	virtual bool IsHero() const = 0;
	virtual bool IsHiding() const = 0;

	virtual void GetCriticalsList( list<CPtr<ICriticalInfo> > *pListCriticals ) const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif