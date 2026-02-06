#ifndef __WUNITATTACKEXEC_H_
#define __WUNITATTACKEXEC_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
	class CGrenadeToHitCalcer;
}
//
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_GRENADE_DELAY_STEP = 0.1f;
const float F_GRENADE_SPHERE_RADIUS = 0.25f;
//
const float F_GRENADE_CHECK_SIDE = FP_GRID_STEP;
const float F_GRENADE_CHECK_RADIUS = 0.3f;
const float F_GRENADE_CHECK_HEIGHT = 1.2f;
//
const float F_GRAVITY = 10.f;
const float F_HEAL_DISTANCE = 0.8f;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGrenadeParams
{
	CVec3 ptOriginalTarget;
	CVec3 ptStart;
	CVec3 vel;
	float fT;
	int   nSide;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsWithinHumanReach( const CVec3 &ptFrom, const CVec3 &ptTarget, float fPlaneDist );
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecQueue
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecQueue: public CCommandExecute, public IExecMove
{
	OBJECT_BASIC_METHODS(CExecQueue);
	ZDATA_(CCommandExecute)
	list<CObj<CCommandExecute> > execList;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&execList); return 0; }

	CExecQueue( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}

	void AddExecutor( CCommandExecute *pExec );
	void CheckOpenCloseOnce();
	int GetStartAP() const;
	int GetActionAP() const;
	virtual void Run();
	virtual bool TimeLabelReached();
	virtual void AnimationFinished();
	virtual void Cancel();
	virtual bool IsExecuting();
	virtual bool IsWaitingForPath( NAI::SUnitPosition *p );
	virtual NAI::CPath* GetCurrentPath() const;

	// IExecMove
	virtual void GetSearchFromPosition( NAI::SPathPlace *pRes );
	virtual void GetDesiredPlace( NAI::SPathPlace *pRes, NAI::EFindPathParams *pParams );
	virtual void GetPathPoints( list<SPathPoint> *pRes );
	virtual void FullCancel();
	void SetNewPath( NAI::CPath *pPath, NAI::EFindPathParams _eParams, ENeedActiveItem eActive = ITEM_NO_MATTER );
	void AddPath( NAI::CPath *pPath, NAI::EFindPathParams _eParams, ENeedActiveItem eActive, IExecMove *pOldFront = 0 );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecAttack
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecAttack: public CCommandExecute
{
protected:
	ZDATA_(CCommandExecute)
	bool bAttackCanceled;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&bAttackCanceled); return 0; }

protected:
	bool CreateAttack( vector<NRPG::CAttackPortion> *pAttack, CUnitServer *pUnitTarget, bool bSpendAmmo = true ) const;

public:
	CExecAttack( CUnitServer *_pUS = 0 );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const = 0;
	virtual void Start() = 0;
	virtual bool OnLabel() = 0;
	virtual void Run();
	virtual bool TimeLabelReached();
	virtual void Cancel();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecShoot
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecShoot: public CExecAttack
{
protected:
	ZDATA_(CExecAttack)
	int nExtraAP;
	int nToHit;
	bool bMissed;
	bool bComplete;
	CRay ray;
	float fRayToHit;
	CVec3 ptAnimTarget;
	vector<NRPG::CAttackPortion> Attack;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CExecAttack*)this); f.Add(2,&nExtraAP); f.Add(3,&nToHit); f.Add(4,&bMissed); f.Add(5,&bComplete); f.Add(6,&ray); f.Add(7,&fRayToHit); f.Add(8,&ptAnimTarget); f.Add(9,&Attack); return 0; }

private:
	void Scream();

protected:
	int GetExtraAP() const { return nExtraAP; }
	void CalculateExtraAP();
	void PerformShot();
	void SpendAP();

public:
	CExecShoot() {}
	CExecShoot( CUnitServer *_pUS, int _nExtraAP );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Start();
	virtual bool CheckBurst( bool bComplete );
	virtual void PerformAttack( const vector<NRPG::CAttackPortion> &attack, const CRay &ray, float fHit = 1.0f );
	virtual bool OnLabel();
	virtual void PrepareShot() {}
	virtual void CheckShotResult() {}
	virtual bool IsAttackCanceled() { return false; }
	virtual bool IsAccidental() const { return false; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecShootTile
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecShootTile: public CExecShoot
{
	OBJECT_BASIC_METHODS(CExecShootTile);
private:
	ZDATA_( CExecShoot )
	NAI::ETileHitLocation eHL;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,( CExecShoot *)this); f.Add(2,&eHL); return 0; }

public:
	CExecShootTile() {}
	CExecShootTile( CUnitServer *_pUS, const CVec3 &_ptTarget );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual void PrepareShot();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecShootUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecShootUnit: public CExecShoot
{
	OBJECT_BASIC_METHODS(CExecShootUnit);
private:
	ZDATA_(CExecShoot)
	CPtr<CUnitServer> pTarget;
	NAI::EHitLocation eHL;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CExecShoot*)this); f.Add(2,&pTarget); f.Add(3,&eHL); return 0; }

public:
	CExecShootUnit() {}
	CExecShootUnit( CUnitServer *_pUS, NWorld::CUnitServer *_pTarget, NAI::EHitLocation _eHL, int _nExtraAttackAP );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual void PrepareShot();
	virtual void CheckShotResult();
	virtual bool IsAttackCanceled();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecMelee
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecMelee: public CExecAttack
{
protected:
	ZDATA_(CExecAttack)
	CVec3 ptTarget;
	int nExtraAP;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CExecAttack*)this); f.Add(2,&ptTarget); f.Add(3,&nExtraAP); return 0; }

protected:
	int GetExtraAP() const { return nExtraAP; }

public:
	CExecMelee() {}
	CExecMelee( CUnitServer *_pUS, int _nExtraAP );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Start();
	virtual void PerformAttack( const vector<NRPG::CAttackPortion> &attack, const CRay &ray, float fHit = 1.0f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecMeleeTile
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecMeleeTile: public CExecMelee
{
	OBJECT_BASIC_METHODS(CExecMeleeTile);
public:
	CExecMeleeTile() {}
	CExecMeleeTile( CUnitServer *_pUS, const CVec3 &_ptTarget );

	virtual bool OnLabel();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecMeleeUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecMeleeUnit: public CExecMelee
{
	OBJECT_BASIC_METHODS(CExecMeleeUnit);
private:
	ZDATA_(CExecMelee)
	bool bIsHitLocationShot;
	NAI::EHitLocation eHL;
	CPtr<CUnitServer> pTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CExecMelee*)this); f.Add(2,&bIsHitLocationShot); f.Add(3,&eHL); f.Add(4,&pTarget); return 0; }

public:
	CExecMeleeUnit() {}
	CExecMeleeUnit( CUnitServer *_pUS, CUnitServer *_pTarget, NAI::EHitLocation _eHL, int _nExtraAttackAP );

	virtual void Start();
	virtual bool OnLabel();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecThrowGrenade
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecThrowGrenade: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecThrowGrenade);
	ZDATA_(CCommandExecute)
	CVec3 ptTarget;
	SGrenadeParams grenadeParams;
	CPtr<NRPG::CGrenadeToHitCalcer> pToHitCalcer;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&ptTarget); f.Add(3,&grenadeParams); f.Add(4,&pToHitCalcer); return 0; }

protected:
	void CheckToHitAndDelay( NDb::CRPGGrenade *pGrenade );
	void ThrowGrenade();

public:
	CExecThrowGrenade() {}
	CExecThrowGrenade( CUnitServer *_pUS, const CVec3 &_ptTarget );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Run();
	virtual bool TimeLabelReached();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecLaunchRocket
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecLaunchRocket: public CExecShoot
{
	OBJECT_BASIC_METHODS(CExecLaunchRocket);
private:
	ZDATA
	ZPARENT( CExecShoot );
	bool bAccidental;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CExecShoot *)this); f.Add(3,&bAccidental); return 0; }

protected:
	void LaunchRocket( );

public:
	CExecLaunchRocket() {}
	CExecLaunchRocket( CUnitServer *_pUS, const CVec3 &_ptTarget );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual void Start();
	virtual bool OnLabel();
	virtual bool IsAccidental() const { return bAccidental; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecOpenClose
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecOpenClose: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecOpenClose);
private:
	ZDATA_(CCommandExecute)
	CObj<CCmdOpenClose> pCmd;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&pCmd); return 0; }

public:
	CExecOpenClose() {}
	CExecOpenClose( CUnitServer *_pUS, CCmdOpenClose *_pCmd );
	void GetParams( bool *pBOpen, IObject **ppObject ) { *pBOpen = pCmd->bOpen; *ppObject = pCmd->pObject; }

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Run();
	virtual bool TimeLabelReached();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecCannon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecCannon: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecCannon);
private:
	ZDATA_(CCommandExecute)
	bool bEnter;
	CPtr<IObject> pCannon;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&bEnter); f.Add(3,&pCannon); return 0; }

public:
	CExecCannon() {}
	CExecCannon( CUnitServer *_pUS, IObject *_pCannon, bool _bEnter );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual void Run();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecUsePassage
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecUsePassage: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecUsePassage);
	ZDATA
	ZPARENT( CCommandExecute )
	CPtr<CCmdUsePassage> pCmd;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CCommandExecute *)this); f.Add(3,&pCmd); return 0; }
public:
	//
	CExecUsePassage() {}
	CExecUsePassage( CUnitServer *_pUS, CCmdUsePassage *_pCmd );
	//
	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual void Run();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecCorpse
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecCorpse: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecCorpse);
private:
	ZDATA_(CCommandExecute)
	bool bTake;
	CPtr<CUnitServer> pDeadUnit;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&bTake); f.Add(3,&pDeadUnit); return 0; }

public:
	CExecCorpse() {}
	CExecCorpse( CUnitServer *_pUS, CUnitServer *_pCorpse, bool _bTake );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Run();
	virtual bool TimeLabelReached();
	virtual void Cancel();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecTakeCorpseOnDeploy
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecTakeCorpseOnDeploy: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecTakeCorpseOnDeploy);
private:
	ZDATA
	ZPARENT( CCommandExecute );
	CPtr<CUnitServer> pDeadUnit;
	bool bDead;
	int n;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CCommandExecute *)this); f.Add(3,&pDeadUnit); f.Add(4,&bDead); f.Add(5,&n); return 0; }
	//
public:
	CExecTakeCorpseOnDeploy() {}
	CExecTakeCorpseOnDeploy( CUnitServer *_pUS, CUnitServer *_pCorpse, bool _bDead );
	//
	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Run();
	virtual bool TimeLabelReached();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecHeal
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecHeal: public CCommandExecute
{
	OBJECT_NOCOPY_METHODS(CExecHeal);
private:
	ZDATA_(CCommandExecute)
	CPtr<CUnitServer> pTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&pTarget); return 0; }
	//
public:
	CExecHeal() {}
	CExecHeal( CUnitServer *_pUS, CUnitServer *_pTarget );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Run();
	virtual void AnimationFinished();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecSnipeAim
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecSnipeAim: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecSnipeAim);
private:
	ZDATA_( CCommandExecute )
	CPtr<CUnitServer> pTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,( CCommandExecute *)this); f.Add(2,&pTarget); return 0; }

public:
	CExecSnipeAim() {}
	CExecSnipeAim( CUnitServer *_pUnitServer, CUnitServer *_pTarget );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Run();
	virtual void AnimationFinished();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecCollectSnipeAP
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecCollectSnipeAP: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecCollectSnipeAP );
private:
	ZDATA
	ECollectSnipeAP eAP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&eAP); return 0; }

public:
	CExecCollectSnipeAP () {}
	CExecCollectSnipeAP ( CUnitServer *pUnitServer, ECollectSnipeAP eAP );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual int GetStartAP() const;
	virtual int GetActionAP() const;
	virtual void Run();
	int GetAPToCollect() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecThrowKnife
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecThrowKnife: public CExecAttack
{
	OBJECT_BASIC_METHODS(CExecThrowKnife);
private:
	ZDATA_(CCommandExecute)
	CVec3 ptTarget;
	CPtr<CUnitServer> pTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&ptTarget); f.Add(3,&pTarget); return 0; }

protected:
	void ThrowKnife();

public:
	CExecThrowKnife() {}
	CExecThrowKnife( CUnitServer *_pUS, const CVec3 &_ptTarget, CUnitServer *_pTarget = 0 );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false ) const;
	virtual void Start();
	virtual bool OnLabel();
	virtual int GetStartAP() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecMoveItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecMoveInventoryItem: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecMoveInventoryItem);
	ZDATA_(CCommandExecute)
	int nStage;
	bool bTwoHeavy;
	CObj<CCmdMoveInventoryItem> pCmd;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&nStage); f.Add(3,&bTwoHeavy); f.Add(4,&pCmd); return 0; }
public:
	CExecMoveInventoryItem() {}
	CExecMoveInventoryItem( CUnitServer *_pUS, CCmdMoveInventoryItem *_p );

	virtual EUnitCommandResult CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget = false );
	virtual void Run();
	virtual bool TimeLabelReached();
	virtual void AnimationFinished();
	virtual void Cancel();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
