#ifndef __wUnitStates_H_
#define __wUnitStates_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
namespace NRPG
{
	//enum ECriticalAction;
	enum ECritical;
}		
namespace NAI
{
	struct SUnitPosition;
}
namespace NWorld
{
class CCmd;
class CCommandExecute;
class CUnitServer;
class CCannon;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitState: public CObjectBase
{
protected:
	ZDATA
	CPtr<CUnitServer> pUS;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pUS); return 0; }
	CUnitState( CUnitServer *_pUS = 0 ): pUS(_pUS) {}
	virtual CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult ) = 0;
	virtual void ProcessCritical( NDb::ECritical eCA ) = 0;
	virtual void OnActionFinish() {}
	virtual void OnStartNewTurn() {}
	virtual void OnFinishOwnTurn() {}
	virtual void OnStartRealTime() {}
	virtual void OnDeath() {}
	virtual void DisableCriticals();
	virtual void Segment() {}
	virtual void OnStateStarted() {}
	virtual void OnStateFinished() {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateNormal: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateNormal);
	bool IsInactive() const;
public:
	CUnitStateNormal( CUnitServer *_pUS = 0 ): CUnitState(_pUS) {}
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );
	virtual void DisableCriticals();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateSniping: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateSniping);
	ZDATA_(CUnitState)
	int nBaseAP; // AP до snipe
	int nSnipeAP; // AP набранное во время snipe
	CPtr<CUnitServer> pTarget;
	NAI::SUnitPosition InitialTargetPosition;
	NAI::SUnitPosition TargetPosition;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUnitState*)this); f.Add(2,&nBaseAP); f.Add(3,&nSnipeAP); f.Add(4,&pTarget); f.Add(5,&InitialTargetPosition); f.Add(6,&TargetPosition); return 0; }

	bool CheckTarget();

public:
	CUnitStateSniping() {}
	CUnitStateSniping( CUnitServer *_pUS, CUnitServer *_pTarget, int _nBaseAP );

	void CancelSnipe();

	virtual void Segment();
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );

	virtual CUnitServer *GetTarget() { return pTarget; }
	virtual void CollectSnipeAP( int _nAP );
	virtual int GetCollectedSnipeAP() { return nSnipeAP; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateUsingCannon: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateUsingCannon);
	ZDATA_(CUnitState)
	CPtr<CCannon> pCannon;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUnitState*)this); f.Add(2,&pCannon); return 0; }
public:
	CUnitStateUsingCannon() {}
	CUnitStateUsingCannon( CUnitServer *_pUS, CCannon *_pCannon ): CUnitState(_pUS), pCannon(_pCannon) {}
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );
	virtual void OnActionFinish();
	virtual void OnDeath();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateCorpseCarrier: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateCorpseCarrier);
	ZDATA_(CUnitState)
	CPtr<CUnitServer> pDeadUnit;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUnitState*)this); f.Add(2,&pDeadUnit); return 0; }
	CUnitStateCorpseCarrier() {}
	CUnitStateCorpseCarrier( CUnitServer *_pUS, CUnitServer *_pDead );
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );
	virtual void OnStateStarted();
	virtual void OnStateFinished();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateHealer: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateHealer);
	ZDATA_(CUnitState)
	CPtr<CUnitServer> pTarget;
	int nRequiredAP;
	bool bNewSegment;
	int nPrevTime;
	float fdVPFraction;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUnitState*)this); f.Add(2,&pTarget); f.Add(3,&nRequiredAP); f.Add(4,&bNewSegment); f.Add(5,&nPrevTime); f.Add(6,&fdVPFraction); return 0; }
	
	void SayAck();
	void DoHealing( int nSpentAP );
	void FinishHealing();
public:
	CUnitStateHealer( CUnitServer *_pUS = 0 ): CUnitState(_pUS), nRequiredAP(2e9), bNewSegment(false) {}
	bool StartHeal( CUnitServer *_pTarget ); // return true if finished
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );
	void Cancel();
	virtual void OnFinishOwnTurn();
	virtual void OnStartRealTime();
	virtual void OnActionFinish();
	virtual void OnDeath();
	virtual void Segment();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateEngineering: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateEngineering);
public:
	CUnitStateEngineering( CUnitServer *_pUS = 0 ): CUnitState(_pUS) {}
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateStun: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateStun);
public:
	CUnitStateStun( CUnitServer *_pUS = 0 ): CUnitState(_pUS) {}
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );
	void OnActionFinish();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateDeath: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateDeath);
public:
	CUnitStateDeath( CUnitServer *_pUS = 0 ): CUnitState(_pUS) {}
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );
	virtual void DisableCriticals();
	virtual void OnStateStarted();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitStateUnconscious: public CUnitState
{
	OBJECT_BASIC_METHODS(CUnitStateUnconscious);
public:
	//
	CUnitStateUnconscious( CUnitServer *_pUS = 0 );
	CCommandExecute* CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult );
	void ProcessCritical( NDb::ECritical eCA );
	virtual void DisableCriticals();
	virtual void OnStateStarted();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
