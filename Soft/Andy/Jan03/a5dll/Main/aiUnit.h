#ifndef __AIUnit_H_
#define __AIUnit_H_

#include "aiPosition.h"

namespace NRPG
{
	class CWeaponItem;
	class CUnit;
	class IUnitMission;
}

namespace NDb
{
	class CRPGWeapon;
	class IUnitMission;
	class CUnit;
}

namespace NWorld
{
	class CUnitServer;
	class CCannon;
}

namespace NAI
{
class IAIState;
class CAIInventory;
class IAILogRecord;
class	IAICriterionData;
class IAIControl;
class CTask;
class CAILogic;
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EHitLocation;
struct SPathPlace;
struct SPosition;
struct SUnitPosition;
enum EAIManager;
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIUnit: public CObjectBase
{
public:	
	virtual NWorld::CUnitServer* GetUnitServer() const = 0;
	virtual NRPG::IUnitMission* GetUnitMission() const = 0;
	virtual NRPG::CUnit* GetRPGUnit() const = 0;
	virtual void Synchronize() = 0;
	virtual SPosition GetPosition() = 0;
	virtual SUnitPosition GetUnitPosition() = 0;
	virtual void SetPosition( SPosition _pPosition ) = 0;
	virtual void SetPosition( SPathPlace _pPosition ) = 0;
	virtual SPosition GetPrevPosition() = 0;
	virtual void SavePrevPosition() = 0;
	virtual int GetTurnStartHP() = 0;
	virtual void GetHP( int *_nHP, int *_nMaxHP ) = 0;
	virtual int GetHP() = 0;
	virtual void SetHP( int _nHP, int _nMaxHP ) = 0;
	virtual void GetAP( int *_nAP, int *_nMaxAP ) = 0;
	virtual int GetAP() = 0;
	virtual void SetAP( int _nAP, int _nMaxAP ) = 0;
	virtual int GetMaxAP() = 0;
	virtual int GetMaxHP() = 0;
	virtual void SpendHP( int _nHP ) = 0;
	virtual void SpendAP( int _nAP ) = 0;
	virtual void SetPose( int pose ) = 0;
	virtual bool IsUsingCannon() = 0;
	virtual NWorld::CCannon *GetCannon() = 0;
	virtual void SetCannon( NWorld::CCannon *_pCannon ) = 0;
	virtual bool IsMovedThisTurn() = 0;
	virtual bool IsDead() = 0;
	virtual int GetRemainAP() = 0;
	virtual int GetToHit( IAIUnit *pTarget, const NAI::SUnitPosition &pos, NAI::EHitLocation hl = NAI::HL_ANY ) = 0;
	virtual bool IsPerformingAction() = 0;
	virtual void OnTurnStarted() = 0;
	virtual void GetLastSeenEnemy( SPosition *Position, IAIUnit **ppAIUnit ) = 0;
	virtual void SetLastSeenEnemy( IAIUnit *pAIUnit ) = 0;
	virtual CAIInventory* GetAIInventory() = 0;
	virtual int GetHurtHP() = 0;
	virtual void SetHurtHP( int _nHurtHP ) = 0;
	virtual int GetCoverForFixedUnit( const NAI::SUnitPosition &pos,
		NWorld::CUnitServer *pTarget, NRPG::CWeaponItem *pWeaponItem, NAI::EHitLocation HitLocation ) = 0;
	virtual bool HasInactivePose() = 0;
	virtual void AssignControl( IAIControl *pAIControl ) = 0;
	virtual CTask* GetRoute() const = 0;
	virtual void ActivateCurrentControl() = 0;
	virtual void DeactivateCurrentControl() = 0;
	virtual void OnControlFinished() = 0;
	virtual bool IsUnderAIControl() = 0;
	virtual int GetAdditionalExpediency() = 0;
	virtual void SetAdditionalExpediency( int nExpediency ) = 0;
	virtual void SetMaxToHit( int _nMaxToHit ) = 0;
	virtual int GetMaxToHit() = 0;
	virtual bool HasVisibleEnemies() = 0;
	virtual void DebugOutput() = 0;
	virtual void OnDied() = 0;

	virtual CAILogic* GetLogic() const = 0;
	virtual void SetLogic( CAILogic *_pLogic ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIUnit *CreateAIUnit( NWorld::CUnitServer *pUnitServer, bool bUnderAIControl );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif
