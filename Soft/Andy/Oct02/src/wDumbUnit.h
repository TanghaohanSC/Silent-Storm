#ifndef __wDumbUnit_H_
#define __wDumbUnit_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wInterface.h"
#include "wAnimation.h"
#include "wDynObject.h"
#include "RPGAttackMech.h"

namespace NDb
{
	class CTerrainTile;
	class CRPGArmor;
	class CTSound;
	class CAISound;
	enum ECritical;
	enum ESlot;
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
namespace NWorld
{
class CWorld;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPlayer;
struct SInterruptInfo;
class IDynamicObject;
class CTimedObject;
class CUnitServer;
class CDumbUnitServer: virtual public IVisObj,
	public NRPG::IAttackable
{
	ZDATA
	CObj<NRPG::IUnitMission> pRPG;
	NAI::SPathPlace nextLock;
	bool bLocksTwoPlaces;
	NAI::SUnitPosition position;
	bool bAlive;
	NAI::EPose wishPose;
	CPtr<CWorld> pWorld;
	CPtr<NDb::CModel> pModel;
	CSyncSrcBind<IVisObj> bindGlobal;
	bool bUndrawWeapon;
	bool bNoHeavyWeapon;
	list<CObj<IDynamicObject> > miscObjects;
public:
	CUnitAnimator animator;
private:
	bool bUnconscious;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pRPG); f.Add(3,&nextLock); f.Add(4,&bLocksTwoPlaces); f.Add(5,&position); f.Add(6,&bAlive); f.Add(7,&wishPose); f.Add(8,&pWorld); f.Add(9,&pModel); f.Add(10,&bindGlobal); f.Add(11,&bUndrawWeapon); f.Add(12,&bNoHeavyWeapon); f.Add(13,&miscObjects); f.Add(14,&animator); f.Add(15,&bUnconscious); return 0; }

private:
	void ProcessSteps( const STime tCurrent );
	void PlaySound( NDb::CTSound *pSound );
	void SetPositionCore( const NAI::SUnitPosition &dst );
	NDb::CSound* GetStepSound( NDb::CRPGArmor *pArmor, NDb::CTerrainTile *pTile );
	int GetAISoundType( NDb::CRPGArmor *pArmor, NDb::CTerrainTile *pTile ) const;
	NDb::CAISound* GetStepAISound();
	void MakeStepSound( bool bSound );
	void GetTileOrArmor( NDb::CTerrainTile **pTile, NDb::CRPGArmor **pArmor );
protected:
	virtual void OnUnitWasKilled() {}
	virtual void OnUnitMadeUnconscious() {}
	virtual void OnSuffersDamage( float fAP )	{} // сколько осталось в процентах
	virtual void ProcessCritical( NDb::ECritical eCA ) {}
	NDb::CModel* GetUnitModel() const { return pModel; }
	void FallAsIfDead( const CVec3 &ptDir, bool bDropItemsFromBackPack );
	void KillUnit( const CVec3 &ptDir );
	void MakeUnconscious( const CVec3 &ptDir );
	void DropItems( bool bHands, bool bBackPack );
public:
	struct SResItem
	{
		CVec3 ptCenter;
		CQuat q;
		CPtr<NDb::CModel> pModel;
		CObj<NRPG::IInventoryItem> pItem;
	};

	CDumbUnitServer() {}
	CDumbUnitServer( CWorld *pWorld, NRPG::IUnitMission *_pRPG, NDb::CModel *pModel, 
		const NAI::SUnitPosition &pos ); //CPlayer *pPlayer, 
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
	void LaunchItem( const SResItem &item, const CVec3 &vel = VNULL3 );
	void AttachMiscObject( CTimedObject *p );
	void SetRunning( bool bRun ) { position.bRun = bRun; }

	NRPG::IUnitMission* GetUnitRPG() const { return pRPG; }
	const NAI::SUnitPosition& GetUnitPosition() const { return position; }
	void GetRealUnitPosition( CVec3 *pRes );
	CWorld* GetWorld() const { return pWorld; }
	NAI::EPose GetWishPose() const { return wishPose; }
	void SetWishPose( NAI::EPose pose ) { wishPose = pose; }
	
	void CreateFlash();
	void Update() { bindGlobal.Update(); }
	void SetUndrawItem( bool _bUndraw, bool _bNoHeavyWeapon = false ) { bUndrawWeapon = _bUndraw; bNoHeavyWeapon = _bNoHeavyWeapon; Update(); }
	bool GetUndrawItem() { return bUndrawWeapon; }

	bool IsDeadUnit() const { return !bAlive; }
	void AddMiscObjects( vector<IVisObj*> *pRes );
	// implement IVisObj
	virtual void Visit( IRenderVisitor* );
	virtual void Visit( IAIVisitor* );
	// implement IAttackable
	virtual int ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor );

	void Segment();
	void PlaceOnPassablePlace();
	virtual bool IsStrafing() { return animator.IsStrafing(); }
	void InitAsCorpse( bool bDead );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
