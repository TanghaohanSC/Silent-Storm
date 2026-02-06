#ifndef __AITACTICALCOMMANDER_H_
#define __AITACTICALCOMMANDER_H_

#include "aiJob.h"

namespace NWorld
{
	class CCmd;
	class CWorld;
	class CPlayer;
	class CCommand;
	class CUnitServer;
	class CCannon;
}

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIUnit;
class IAIState;
class IAIControl;
class IAIIterator;
class IAICriterion;
class IAILogContainer;
class CScaleConverter;
class CAICommander;
class CAITacticalCommander;
class CAIAction;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAITacticalCommander
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAITacticalCommander: public CAIJob
{
	typedef list< CPtr<NWorld::CUnitServer> > UnitSet;
	typedef list< CPtr<NWorld::CCommand> > CommandSet;
	//
	OBJECT_BASIC_METHODS(CAITacticalCommander);
	ZDATA
	ZPARENT(CAIJob);
	bool bEndOfTurn;
	int nIterationID;
	bool bBestTurnFound;
	CommandSet Commands;
	bool bNeedContinueAction;
	CPtr<NWorld::CUnitServer> pLastCommandedUnit;
	CObj<IAIState> pAIState;
	CObj<IAILogContainer> pAILog; 
	CPtr<CAICommander> pAICommander;
	CObj<IAIIterator> pAIUnitIterator;
	CObj<IAIIterator> pAIActionIterator;
	vector< CObj<CAIAction> > actions;
	float fBestExpediency;
	CObj<IAICriterion> pAICriterion;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIJob*)this); f.Add(3,&bEndOfTurn); f.Add(4,&nIterationID); f.Add(5,&bBestTurnFound); f.Add(6,&Commands); f.Add(7,&bNeedContinueAction); f.Add(8,&pLastCommandedUnit); f.Add(9,&pAIState); f.Add(10,&pAILog); f.Add(11,&pAICommander); f.Add(12,&pAIUnitIterator); f.Add(13,&pAIActionIterator); f.Add(14,&actions); f.Add(15,&fBestExpediency); f.Add(16,&pAICriterion); return 0; }
	//
	NWorld::CCannon *FindNearestCannon( IAIUnit *pUnit );
	bool CannonCanDamageEnemy( NWorld::CCannon *pCannon );
	void UseCannon( IAIUnit *pAIUnit );
	void CheckForReleasableUnits();
	void CreateActions();
	void StartCurrentAction();
	void ResetActions();
	//
public:
	CAITacticalCommander() {}
	CAITacticalCommander( CAICommander *_pAICommander );
	//
	virtual NWorld::CCommand* GetCommand();
	virtual void Think();
	virtual bool IsEndOfTurn();
	virtual bool IsPerformingAction();
	virtual bool IsUnderControl( IAIUnit *_pAIUnit );
	virtual void OnPassControl( NWorld::CPlayer *pPlayer );
	virtual void OnUnitWasKilled( NWorld::CUnitServer *pUnit );
	virtual void OnTurnStarted();
	virtual void OnTurnFinished();
	virtual void OnStartRealTime();
	virtual void OnCancelAction();
	virtual void Segment();
	virtual void AddAllyUnit( IAIUnit *_pAIUnit );
	virtual void AddEnemyUnit( IAIUnit *_pAIUnit );
	virtual void OnSeeUnit( NWorld::CUnitServer *pWatcher, NWorld::CUnitServer *pTarget );
	// AIJob
	virtual void DoJob();
	virtual bool IsIdleJob();
	virtual bool IsJobFinished();
	virtual void OnJobFinished();
	//
	NWorld::CWorld *GetWorld();
	IAIUnit *GetDangerousAttackableEnemy( IAIUnit *pAIUnit );
	IAIUnit *GetNearestAttackableEnemy( IAIUnit *pAIUnit );
	IAIUnit *GetNearestEnemy( IAIUnit *pAIUnit );
	IAIUnit *GetNearestAlly( IAIUnit *pAIUnit );
	void DismissUnit( IAIUnit *_pAIUnit );
	void Synchronize();
	bool IsAITurn();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAITacticalCommander *CreateAITacticalCommander( CAICommander *pAICommander );
////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif