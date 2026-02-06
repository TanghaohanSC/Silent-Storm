#ifndef _AICOMMANDER_H_
#define _AICOMMANDER_H_

#include "wInterface.h"

namespace NWorld
{
	enum ETBSEvent;
	class CWorld;
	class CPlayer;
	class CObjectServerBase;
}

namespace NAI
{
class IAIUnit;
class IAISignal;
class CAITaskCommander;
class CAITacticalCommander;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAICommander;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAICommander: public NWorld::CCommander
{
	OBJECT_BASIC_METHODS(CAICommander);
	ZDATA
	ZPARENT( NWorld::CCommander );
	CPtr<NWorld::CPlayer> pPlayer;
	CObj<CAITaskCommander> pTaskCommander;
	CObj<CAITacticalCommander> pTacticalCommander;
	list< CPtr<NWorld::CObjectServerBase> > LockedObjects;
	list< CObj<IAIUnit> > units;
	CPtr<NWorld::CWorld> pWorld;
	bool bAITurn;
	bool bWantTurnBased;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(NWorld::CCommander*)this); f.Add(3,&pPlayer); f.Add(4,&pTaskCommander); f.Add(5,&pTacticalCommander); f.Add(6,&LockedObjects); f.Add(7,&units); f.Add(8,&pWorld); f.Add(9,&bAITurn); f.Add(10,&bWantTurnBased); return 0; }
	//
private:
	// TBSEvents
	void OnTurnStarted();
	void OnTurnFinished();
	void OnCancelAction();
	void OnStartRealTime();
	//
public:
	CAICommander() {}
	CAICommander( NWorld::CWorld *_pWorld, NWorld::CPlayer *_pPlayer ); 
	void GenerateCommand();
	virtual void OnPassControl( NWorld::CPlayer *_pPlayer );
	virtual void OnUnitDied( NWorld::CUnitServer *pUnit );
	void RemoveUnit( NWorld::CUnitServer *pUS );
	virtual void OnSeeUnit( NWorld::CUnitServer *pWatcher, NWorld::CUnitServer *pTarget );
	virtual bool IsEndOfTurn();
	virtual bool IsRequestInterrupt() const { return false; }
	virtual void Segment(); 
	virtual CAITaskCommander *GetAITaskCommander() { return pTaskCommander; }
	virtual CAITacticalCommander *GetAITacticalCommander() { return pTacticalCommander; }
	virtual void OnUnitAdded( NWorld::CUnitServer *pUnit );
	virtual void OnTBSEvent( NWorld::ETBSEvent event );
	// Object lock
	bool IsObjectLocked( NWorld::CObjectServerBase *pObject );
	void LockObject( NWorld::CObjectServerBase *pObject );
	void UnLockObject( NWorld::CObjectServerBase *pObject );
	void UnLockAllObjects();
	//
	virtual void ProcessAISignals();
	NWorld::CWorld *GetWorld() { return pWorld; }
	IAIUnit *GetAIUnit( NWorld::CUnitServer *pUnit );
	bool IsAITurn() { return bAITurn; }
	bool HasVisibleEnemies();
	void Synchronize();
	void WantTurnBased();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif