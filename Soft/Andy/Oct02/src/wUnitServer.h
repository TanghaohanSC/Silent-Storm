#ifndef __wUnitServer_H_
#define __wUnitServer_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wDumbUnit.h"
#include "wUnitSounds.h"
#include "wTurnBased.h"
namespace NDb
{
	class CAISound;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitServer;
class CCommandExecute: public CObjectBase
{
public:
	enum EFinishType
	{
		RUNNING,
		FINISHED,
		FAILED
	};
private:
	ZDATA
	CObj<CActionCounter> pAction;
	EFinishType state;
protected:
	CPtr<CUnitServer> pUS;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pAction); f.Add(3,&state); f.Add(4,&pUS); return 0; }
protected:
	enum EActionType
	{
		NORMAL,
		SKIPPABLE,
		NOBLOCK
	};
	void StartAction( CWorld *pWorld, EActionType actionType );
	void StopAction() { pAction = 0; }
	void Finished() { state = FINISHED; }
	void Failed() { /*ASSERT( 0 );*/ state = FAILED; }
public:
	CCommandExecute( CUnitServer *_pUS = 0 ): state(RUNNING), pUS(_pUS) {}
	EFinishType GetState() const { return state; }
	virtual int GetStartAP() const { return 0; }
	virtual int GetActionAP() const { return 0; }
	virtual void Run() { Finished(); }
	virtual bool TimeLabelReached() { return false; }
	virtual void AnimationFinished() { Finished(); }
	virtual void Cancel() {}
	virtual bool IsExecuting() { return IsValid( pAction ); }
	virtual bool IsWaitingForPath( NAI::SUnitPosition *p = 0 ) { return false; }
	virtual NAI::CPath* GetCurrentPath() const { return 0; }
	CUnitServer* GetUnitServer() const { return pUS; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPathViewer: public IPathViewer
{
	OBJECT_BASIC_METHODS(CPathViewer)
private:
	ZDATA
	CPtr<CUnitServer> pUS;
	CObj<CCommandExecute> pMove;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pUS); f.Add(3,&pMove); return 0; }

public:
	CPathViewer() {}
	CPathViewer( CUnitServer *_pUS );

	void SetPath( NAI::CPath *_pPath );

	int GetResult() const;
	void GetPoints( vector<SPathPoint> *pRes );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitState;
class CUnitServer: public CDumbUnitServer, public CUnit, public CTBSUnit<CUnitServer, CPlayer>,
	public CSoundsTracker<CUnitServer>
{
	OBJECT_NOCOPY_METHODS(CUnitServer);
	typedef CTBSUnit<CUnitServer, CPlayer> TTBSUnit;
	typedef CSoundsTracker<CUnitServer> CUSSoundTracker;
	//
	ZDATA
	ZPARENT( CDumbUnitServer )
	ZPARENT( TTBSUnit )
	ZPARENT( CUSSoundTracker )
	CObj<CCommandExecute> pExec;
	bool bCallTimeLabel;
	CObj<CCmd> pCurrentCmd;
	CObj<CUnitState> pState;
	list<NDb::ECritical> criticals; // pending criticals
	bool bIsRunningForcedAction;
	CObj<CCmd> pAutoRunCmd;
	list<CPtr<CUnitServer> > wasInterruptedList; // has had interrupt during current turn
	list< CPtr<CUnitServer> > lostUnits;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDumbUnitServer *)this); f.Add(3,(TTBSUnit *)this); f.Add(4,(CUSSoundTracker *)this); f.Add(5,&pExec); f.Add(6,&bCallTimeLabel); f.Add(7,&pCurrentCmd); f.Add(8,&pState); f.Add(9,&criticals); f.Add(10,&bIsRunningForcedAction); f.Add(11,&pAutoRunCmd); f.Add(12,&wasInterruptedList); f.Add(13,&lostUnits); return 0; }

	void RefreshExecutor();
	void CheckCmdExecState();
	// CDumbUnit callbacks
	virtual void OnUnitWasKilled();
	virtual void OnUnitMadeUnconscious();
	virtual void OnSuffersDamage( float fAP );
	virtual void ProcessCritical( NDb::ECritical eCA );
	void CancelAction();
	void FetchRPGAcks();
	void CancelHeal();
	
public:
	CUnitServer() {}
	CUnitServer( CWorld *pWorld, NRPG::IUnitMission *_pRPG, NDb::CModel *pModel, 
		CPlayer *pPlayer, const NAI::SUnitPosition &pos );
	// implement CTBSUnit
	virtual void Do( CCommand* );
	virtual bool IsMoving() const; 
	virtual bool IsDead() const { return CDumbUnitServer::IsDeadUnit(); }
	virtual bool IsUnconscious() const;
	virtual void OnTBSEvent( ETBSEvent event );
	virtual bool IsPerformingAction() const;
	// implement CUnit	
	virtual void GetVisible( vector<CPtr<CUnit> > *pTarget ) const;
	virtual bool IsStrafing() const { return animator.IsStrafing(); }
	virtual bool IsCarryingCorpse() const { return animator.IsCarryingCorpse(); }
	virtual CUnit* GetCorpseCarrier() const { return animator.GetCorpseCarrier(); }
	virtual NDb::CModel* GetModel() const { return GetUnitModel(); }
	virtual void GetInfo( NRPG::SUnitInfo *pInfo ) const;
	virtual IPlayer* GetPlayer() const;
	virtual NAI::CPath* GetCurrentPath();
	virtual IPathViewer* CreatePathViewer();
	virtual NRPG::IUnitMissionInfo* GetRPG() const;
	virtual const NAI::SUnitPosition& GetPosition() const { return GetUnitPosition(); }
	virtual void GetRealPosition( CVec3 *pRes ) { GetRealUnitPosition( pRes ); }
	virtual void AddVisitableChildren( vector<IVisObj*> *pRes ) { AddMiscObjects( pRes ); }
	virtual bool GetCurrentCommandName( string *pName ) const;
	virtual CVec3 GetAttackOrigin() const;
	virtual CVec3 GetAttackOrigin( const NAI::SUnitPosition &from ) const;
	virtual float GetMinClearDistance() const;
	virtual const CObjectBase* GetAttackIgnore() const;
	virtual EUnitCommandResult CanDo( CCmd *p, int *pnStartAP = 0, int *pnFullAP = 0 );
	virtual bool HasEnoughAP();
	virtual EState GetState();
	virtual NDb::CComplexHead* GetDBHead();
	virtual bool IsCapPresent();

	void Segment();
	void UpdateVisible( SInterruptInfo *pRes );
	bool HasCommand() const { return IsValid( pCurrentCmd ); } //pExec->IsValid(); }// && commandsQueue.empty(); }
	bool CanHearSound( const CVec3 &ptFrom, NDb::CAISound *pSound, int nSoundType, CDumbUnitServer *pWho );
	void CallTimeLabel() { bCallTimeLabel = true; }
	void RunCriticalExecutor( CCommandExecute *p );
	void PostponeCritical( NDb::ECritical critical ) { criticals.push_front( critical ); }
	void ProcessCriticalImmediately( NDb::ECritical eCA );
	void SetState( CUnitState *_pState );
	bool WasInterrupted( CUnitServer *pWho ) const { return find( wasInterruptedList.begin(), wasInterruptedList.end(), pWho ) != wasInterruptedList.end(); }
	void MarkInterrupted( CUnitServer *pWho ) { ASSERT( !WasInterrupted( pWho ) ); wasInterruptedList.push_back( pWho ); }
	bool IsCriticalsFailCommand( CCmd *pCmd ); // проверяет возможность выполнения комманды для текущих criticals. 
	void UpdateCriticalsState();
	void DynamicallyLockWay( CPtr<NAI::CPath> pPath );
	virtual bool CanSnipe() const;
	virtual void CancelSnipe();
	virtual bool IsSniping() const;
	virtual void CollectSnipeAP( int nExtraAP );
	virtual int ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor );
	virtual int GetCarefulShotExtraAP();
	bool HasLostUnits();
	bool GetBarrelDir( CRay *pRay );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif