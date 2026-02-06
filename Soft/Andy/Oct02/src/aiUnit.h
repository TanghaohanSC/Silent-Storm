#ifndef __AIUnit_H_
#define __AIUnit_H_

namespace NRPG
{
	class CWeaponItem;
}

namespace NDb
{
	class CRPGWeapon;
}

namespace NWorld
{
	class CUnitServer;
	class CCannon;
}

namespace NAI
{
class IAIState;
class IAIWeapon;
class IAIInventory;
class IAILogRecord;
class	IAICriterionData;
class IAIControl;
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EHitLocation;
struct SPathPlace;
struct SPosition;
struct SUnitPosition;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAICriterionData: public CObjectBase
{
	OBJECT_BASIC_METHODS(CAICriterionData)
	ZDATA
	bool bNeedRecalc;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bNeedRecalc); return 0; }

	CAICriterionData(): bNeedRecalc(true) {}
	virtual bool IsNeedRecalc() { return bNeedRecalc; }
	virtual void SetNeedRecalc( bool _bNeedRecalc = true ) { bNeedRecalc = _bNeedRecalc; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIEnemyDamageCriterionData: public CAICriterionData
{
	OBJECT_BASIC_METHODS(CAIEnemyDamageCriterionData)
	ZDATA_(CAICriterionData)
	float fDamage;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAICriterionData*)this); f.Add(2,&fDamage); return 0; }
public:
	CAIEnemyDamageCriterionData(): CAICriterionData(), fDamage(0) {}
	virtual float GetDamage() { return fDamage; }
	virtual void SetDamage( float _fDamage ) { fDamage = _fDamage; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIAllyDamageCriterionData: public CAICriterionData
{
	OBJECT_BASIC_METHODS(CAIAllyDamageCriterionData)
	ZDATA_(CAICriterionData)
	float fDamage;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAICriterionData*)this); f.Add(2,&fDamage); return 0; }
public:
	CAIAllyDamageCriterionData(): CAICriterionData(), fDamage(0) {}
	virtual float GetDamage() { return fDamage; }
	virtual void SetDamage( float _fDamage ) { fDamage = _fDamage; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// по смыслу ближе к структуре, нежели к классу, поэтому SAICriterionData
struct SAICriterionData: public CObjectBase
{
	OBJECT_BASIC_METHODS(SAICriterionData)
public:
	ZDATA
	CObj<CAIEnemyDamageCriterionData> pEnemyDamage;
	CObj<CAIAllyDamageCriterionData> pAllyDamage;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pEnemyDamage); f.Add(3,&pAllyDamage); return 0; }
	SAICriterionData()
	{
		pEnemyDamage = new CAIEnemyDamageCriterionData();
		pAllyDamage = new CAIAllyDamageCriterionData();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIUnit: public CObjectBase
{
public:	
	virtual NWorld::CUnitServer *GetUnitServer() = 0;
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
	virtual IAIUnit *GetCurrentEnemy() = 0;
	virtual void SetCurrentEnemy( IAIUnit *_pCurrentEnemy ) = 0;
	virtual int GetToHit( IAIUnit *pTarget ) = 0;
	virtual bool IsPerformingAction() = 0;
	virtual void OnTurnStarted() = 0;
	virtual void GetLastSeenEnemy( SPosition *Position, IAIUnit **ppAIUnit ) = 0;
	virtual void SetLastSeenEnemy( IAIUnit *pAIUnit ) = 0;
	virtual IAIInventory *GetAIInventory() = 0;
	virtual int GetHurtHP() = 0;
	virtual void SetHurtHP( int _nHurtHP ) = 0;
	virtual int GetCoverForFixedUnit( NWorld::CUnitServer *pTarget, 
		NRPG::CWeaponItem *pWeaponItem, NAI::EHitLocation HitLocation ) = 0;
	virtual bool HasInactivePose() = 0;
	virtual void AssignControl( IAIControl *pAIControl ) = 0;
	virtual void OnControlFinished() = 0;
	virtual bool IsUnderAIControl() = 0;
	virtual int GetAdditionalExpediency() = 0;
	virtual void SetAdditionalExpediency( int nExpediency ) = 0;
	virtual void SetMaxToHit( int _nMaxToHit ) = 0;
	virtual int GetMaxToHit() = 0;
	virtual bool HasVisibleEnemies() = 0;
	virtual void DebugOutput() = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIUnit *CreateAIUnit( NWorld::CUnitServer *pUnitServer, bool bUnderAIControl );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif
