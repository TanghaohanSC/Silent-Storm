#ifndef __AIACTION_H_
#define __AIACTION_H_

#include "aiJob.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIUnit;
class IAIState;
class IAIIterator;
class IAICriterion;
class IAILogContainer;
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIAction
{
public:
	virtual int GetMinAP() = 0; // минимальное AP необходимое для выполнения действия
	virtual int GetMaxAP() = 0; // максимально AP имеющееся для выполнения действия
	virtual void SetMaxAP( int nAP ) = 0;
	virtual int GetAPStep() = 0; // шаг изменения AP
	virtual bool IsNeedRemainAP() = 0; // если true, то действию выделяется все оставшееся AP
	virtual bool IsThinking() = 0; // идет процесс поиска наилучшего варианта действия
	virtual void Think() = 0; // нахождение лучшего варианта действия за nAP
	virtual IAILogContainer *GetAILog() = 0; // log используемый действием
	virtual IAIState *GetAIState() = 0; // state используемый действием
	virtual bool IsUsable() = 0; // возможность использования в данном состоянии
	virtual void Reset() = 0; // сбросить оптимизационные данные
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIAction: public CAIJob, public IAIAction
{
	ZDATA
	ZPARENT( CAIJob );
public:
	CObj<IAILogContainer> pAILog;
	CPtr<IAIState> pAIState;
	int nMinAP, nMaxAP, nAPStep;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIJob *)this); f.Add(3,&pAILog); f.Add(4,&pAIState); f.Add(5,&nMinAP); f.Add(6,&nMaxAP); f.Add(7,&nAPStep); return 0; }
	//
	CAIAction() {}
	CAIAction( IAIState *_pAIState, IAIJob *_pParentJob = 0 );
	// IAIAction
	virtual int GetMinAP();
	virtual int GetMaxAP();
	virtual void SetMaxAP( int nAP );
	virtual int GetAPStep();
	virtual bool IsNeedRemainAP() { return false; }
	virtual bool IsThinking();
	virtual IAILogContainer *GetAILog();
	virtual IAIState *GetAIState();
	virtual bool IsUsable();
	virtual void Reset() {}
	// IAIJob
	virtual bool IsIdleJob();
	virtual void OnJobFinished();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIIteratorBasedAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIIteratorBasedAction: public CAIAction
{
	ZDATA
	ZPARENT( CAIAction );
public:
	CObj<IAIIterator> pAIIterator;
	CObj<IAICriterion> pAICriterion;
	bool bBestVersionFound;
	float fExpediency;
	bool bFirstIterationFinished;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIAction *)this); f.Add(3,&pAIIterator); f.Add(4,&pAICriterion); f.Add(5,&bBestVersionFound); f.Add(6,&fExpediency); f.Add(7,&bFirstIterationFinished); return 0; }
	//
	CAIIteratorBasedAction() {}
	CAIIteratorBasedAction( IAIState *_pAIState, IAIJob *_pParentJob = 0 );
	// IAIAction
	virtual void Think();
	virtual void Reset();
	// IAIJob
	virtual void DoJob();
	virtual bool IsJobFinished();
	virtual void OnJobFinished();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif
