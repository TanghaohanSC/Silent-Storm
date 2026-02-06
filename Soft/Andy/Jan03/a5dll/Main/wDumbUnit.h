#ifndef __wDumbUnit_H_
#define __wDumbUnit_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "wInterface.h"
#include "wInterfaceVisitors.h"
#include "Sync.h"
#include "wAnimation.h"
#include "wDynObject.h"
#include "RPGAttackMech.h"

namespace NDb
{
	class CRPGArmor;
	class CTSound;
	class CAISound;
	enum ECritical;
	enum ESlot;
	class CPanzerklein;
}

namespace NAI
{
	enum EHitLocation;
}
namespace NRPG
{
	enum EAction;
	//enum ECriticalAction;
	class IUnitMission;
}
struct SStepSound;
namespace NWorld
{
class CWorld;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPlayer;
struct SInterruptInfo;
class IDynamicObject;
class CTimedObject;
class CUnit;
class CWorld;
class CMine;
class CDumbUnitServer: public IVisObj, public NRPG::IAttackable
{
	ZDATA
	CObj<NRPG::IUnitMission> pRPG;
	NAI::SPathPlace nextLock;
	bool bLocksTwoPlaces;
	NAI::SUnitPosition position;
	bool bStrafe;
	NAI::EPose wishPose;
	CPtr<CWorld> pWorld;
	CSyncSrcBind<IVisObj> bindGlobal;
	bool bUndrawWeapon;
	bool bNoHeavyWeapon;
	list<CObj<IDynamicObject> > miscObjects;
protected:
	CPtr<NDb::CModel> pModel;
	bool bIsPKWhichIsWeared;
public:
	CUnitAnimator animator;
private:
	bool bJustUnhided;
	CPtr<CObjectBase> pAIMapHull;
	int nPrevFloor;
	bool bHeadless;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pRPG); f.Add(3,&nextLock); f.Add(4,&bLocksTwoPlaces); f.Add(5,&position); f.Add(6,&bStrafe); f.Add(7,&wishPose); f.Add(8,&pWorld); f.Add(9,&bindGlobal); f.Add(10,&bUndrawWeapon); f.Add(11,&bNoHeavyWeapon); f.Add(12,&miscObjects); f.Add(13,&pModel); f.Add(14,&bIsPKWhichIsWeared); f.Add(15,&animator); f.Add(16,&bJustUnhided); f.Add(17,&pAIMapHull); f.Add(18,&nPrevFloor); f.Add(19,&bHeadless); return 0; }

private:
	void ProcessSteps( const STime tCurrent );
	void PlaySound( NDb::CTSound *pSound );
	void SetPositionCore( const NAI::SUnitPosition &dst );
	NDb::CSound* GetStepSound( NDb::CRPGArmor *pArmor );
	NDb::CAISound* GetStepAISound();
	void MakeStepSound( bool bSound );
	NDb::CRPGArmor* GetArmor();
	void GetUnitPositionForVisit( NAI::SUnitPosition *pPos );
	int GetFloor();
	bool IsAddedToVisitor();
	void FallAsIfDead( const CVec3 &ptDir, bool bDropItemsFromBackPack );
	void DropItems( bool bHands, bool bBackPack );
	void BlowUp();
protected:
	bool IsLocker();
	virtual void Die( bool bRemove = false ) {}
	virtual void OnUnitMadeUnconscious( bool bFromScript = false ) {}
	virtual void OnSuffersDamage( float fAP )	{} // сколько осталось в процентах
	virtual void ProcessCritical( NDb::ECritical eCA ) {}
	virtual void TouchedMines( const vector<CPtr<CMine> > &mines ) {}
	virtual void RemoveFromWorld() {}
	CObjectBase* GetAIMapUnitHull() { return pAIMapHull; }
	NDb::CModel* GetUnitModel() const { return pModel; }
	bool IsEmptyPK() const;
	float GetMaxFallDist() const;
public:
	struct SResItem
	{
		CVec3 ptCenter;
		CQuat q;
		CPtr<NDb::CModel> pModel;
		CObj<NRPG::IInventoryItem> pItem;
	};

	CDumbUnitServer() {}
	CDumbUnitServer( CWorld *pWorld, NRPG::IUnitMission *_pRPG, NDb::CModel *pModel, const NAI::SUnitPosition &pos );
	void KillUnit( const CVec3 &ptDir );
	void MakeUnconscious( const CVec3 &ptDir, bool bFromScript = false );
	void SetPosition( const NAI::SUnitPosition &dst );
	void SetTemporaryPosition( const NAI::SUnitPosition &dst ) // to be used only in CExecQueue; in all other cases use SetPosition
	{	position = dst;	}
	void DoGameMove( const NAI::SUnitPosition &dst );
	void LockNextPlace( const NAI::SUnitPosition &dst );
	bool CanDoGameMove( const NAI::SUnitPosition &dst );
	bool CheckPassable( const NAI::SUnitPosition &dst );
	int GetActionAP( NRPG::EAction action ) const;
	int GetAP() const;
	bool CanSpendAP( int nAP ) const;
	void SpendAP( int nAP );
	void DoAction( NRPG::EAction action ); // register action & spends AP
	void GetBonePos( CVec3 *pRes, CQuat *pQuat, const char *pszBoneName );
	bool TearOffItem( SResItem *pRes, NDb::ESlot slot, bool bPlaceNextSameItem = false );
	void AttachMiscObject( CTimedObject *p );
	void SetRunning( bool bRun ) { position.bRun = bRun; }

	NRPG::IUnitMission* GetUnitRPG() const { return pRPG; }
	const NAI::SUnitPosition& GetUnitPosition() const { return position; }
	void GetRealUnitPosition( CVec3 *pRes );
	CWorld* GetWorld() const { return pWorld; }

	virtual bool IsStrafing() const { return bStrafe; }
	void SetStrafe( bool _bStrafe ) { bStrafe = _bStrafe; }
	NAI::EPose GetWishPose() const { return wishPose; }
	void SetWishPose( NAI::EPose pose ) { wishPose = pose; }
	
	void CreateFlash();
	void Update() { bindGlobal.Update(); }
	void SetUndrawItem( bool _bUndraw, bool _bNoHeavyWeapon = false ) { bUndrawWeapon = _bUndraw; bNoHeavyWeapon = _bNoHeavyWeapon; Update(); }
	bool GetUndrawItem() { return bUndrawWeapon; }

	void AddMiscObjects( vector<IVisObj*> *pRes );
	// implement IVisObj
	virtual void Visit( IRenderVisitor* );
	virtual void Visit( IAIVisitor* );
	// implement IAttackable
	virtual int ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor );

	void Segment();
	void PlaceOnPassablePlace();
	virtual CDumbUnitServer *GetCorpse() { return 0; }
	void InitAsCorpse( bool bDead );

	bool IsWearingPK() { return IsValid( GetWearingDBPK() ); }
	virtual NDb::CPanzerklein *GetWearingDBPK() { ASSERT(0); return 0; }
	bool WearAsPK( bool bWear );
	void Hide( bool bHide );
	bool IsJustUnhided() { return bJustUnhided; }
	virtual bool IsDead() const = 0;
	virtual bool IsUnconscious() const = 0;
	virtual bool CanFight() const = 0;
};
void LaunchItem( CWorld *pWorld, const CDumbUnitServer::SResItem &item, const CVec3 &vel = VNULL3 );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
