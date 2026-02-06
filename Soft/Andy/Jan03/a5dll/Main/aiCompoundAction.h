#ifndef __AICOMPOUNDACTION_H_
#define __AICOMPOUNDACTION_H_

#include "aiJob.h"

namespace NAI
{
class IAIState;
class CAIAction;
struct SPlaceWithAP;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILogic
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILogic: public CAIJob
{
	ZDATA
	ZPARENT( CAIJob );
	CPtr<IAIState> pState;
	CPtr<IAILogContainer> pLog;
	int nCurrentJob;
protected:
	vector< CPtr<CAIJob> > jobs;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIJob *)this); f.Add(3,&pState); f.Add(4,&jobs); return 0; }
	//
protected:
	IAIState* GetState() const { return pState; }
	void DoAction( CAIAction *pAction );
	IAILogContainer* GetLog() const { return pLog; }
	//
public:
	CAILogic() {}
	CAILogic( IAIState *_pState, IAILogContainer *_pLog, CAIJob *_pParentJob ): 
		CAIJob( _pParentJob ), pLog( _pLog ), pState( _pState ), nCurrentJob( 0 ) {}
	//
	virtual void DoJob();
	bool Think();
	//
	virtual bool CanSkip() const = 0;
	virtual bool GetPlaceForAction( CAIAction *pAction, SPlaceWithAP *pPlace ) = 0;
	virtual void MakeDecision() = 0;
	virtual void OnPrepare( CAIJob *pJob ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAILogic* CreateAttackAILogic( IAIState *pState, IAILogContainer *pLog, CAIJob *pParentJob );
CAILogic* CreateDefendAILogic( IAIState *pState, IAILogContainer *pLog, CAIJob *pParentJob );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif