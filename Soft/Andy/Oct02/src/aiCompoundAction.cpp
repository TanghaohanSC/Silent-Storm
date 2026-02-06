#include "StdAfx.h"

#include "aiLog.h"
#include "aiJob.h"
#include "aiUnit.h"
#include "aiState.h"
#include "aiAction.h"
#include "aiIterator.h"
#include "aiCriterion.h"
#include "aiMoveAction.h"
#include "aiAttackAction.h"

#include "wMain.h"

#include "aiCompoundAction.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAICompoundAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAICompoundAction: public CAIIteratorBasedAction
{
	ZDATA
	ZPARENT( CAIIteratorBasedAction );
	int nCurrentAction;
	vector< CObj<CAIAction> > Actions;
	vector<int> ActionsAP;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIIteratorBasedAction *)this); f.Add(3,&nCurrentAction); f.Add(4,&Actions); f.Add(5,&ActionsAP); return 0; }
private:
	//
	void BeginToThink();
	bool CheckHaveEnoughAP();
	void SetActionsAP();
	void PrepareActionsAP();
	bool IsJobFinished();
	void DebugOutput();
	int GetAPSum();
	void SetRemainActionAP();
public:
	//
	CAICompoundAction() {}
	CAICompoundAction( IAIState *_pAIState, IAIJob *_pParentJob );
	//
	virtual void Add( CAIAction *pAIAction );
	// CAIAction
	virtual int GetMinAP();
	virtual int GetAPStep();
	virtual void Think();
	virtual bool IsUsable();
	virtual void Reset();
	// IAIJob
	virtual void DoJob();
	virtual bool IsIdleJob();
	//
	virtual void First();
	virtual void Next();
	virtual bool IsEnd();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAICompoundAction::CAICompoundAction( IAIState *_pAIState, IAIJob *_pParentJob ):
	CAIIteratorBasedAction( _pAIState, _pParentJob ), nCurrentAction( 0 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICompoundAction::Add( CAIAction *pAIAction )
{
	Actions.push_back( pAIAction );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAICompoundAction::GetMinAP()
{
	int nRes = 0;
	for ( vector< CObj<CAIAction> >::iterator i = Actions.begin(); i != Actions.end(); ++i )
		nRes += (*i)->GetMinAP();
	//
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAICompoundAction::Reset()
{
	for ( vector< CObj<CAIAction> >::iterator i = Actions.begin(); i != Actions.end(); ++i )
		(*i)->Reset();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAICompoundAction::GetAPSum()
{
	int nRes = 0;
	for ( vector< CObj<CAIAction> >::iterator i = Actions.begin(); i != Actions.end(); ++i )
		if ( !(*i)->IsNeedRemainAP() )
			nRes += (*i)->GetMaxAP();
	//
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAICompoundAction::GetAPStep()
{
	int nRes = 0xFFFF;
	for ( vector< CObj<CAIAction> >::iterator i = Actions.begin(); i != Actions.end(); ++i )
		if ( !(*i)->IsNeedRemainAP() )
			nRes = min( nRes, (*i)->GetAPStep() );
	//
	return nRes;
}
//////////////////////////////////////////////////////////////////////////////////////	
bool CAICompoundAction::CheckHaveEnoughAP()
{
	return GetAPSum() <= nMaxAP;
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAICompoundAction::SetActionsAP()
{
	for ( int n = 0; n < Actions.size(); ++n )
		Actions[n]->SetMaxAP( ActionsAP[n] );
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAICompoundAction::PrepareActionsAP()
{
	ActionsAP.resize( Actions.size() );
	for ( int n = 0; n < Actions.size(); ++n )
		if ( !Actions[n]->IsNeedRemainAP() )
			ActionsAP[n] = Actions[n]->GetMinAP();
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAICompoundAction::SetRemainActionAP()
{
	for ( int n = 0; n < Actions.size(); ++n )
		if ( Actions[n]->IsNeedRemainAP() )
		{
			Actions[n]->SetMaxAP( max( 0, GetMaxAP() - GetAPSum() ) );
			return;
		}
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAICompoundAction::First()
{
	PrepareActionsAP();
	SetActionsAP();
	SetRemainActionAP();
	//
	DebugOutput();
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAICompoundAction::Next()
{
	if ( !IsEnd() )
	{
		for ( int n = 0; n < Actions.size(); ++n )
		{
			if ( !Actions[n]->IsNeedRemainAP() )
			{
				ActionsAP[n] += Actions[n]->GetAPStep();
				if ( CheckHaveEnoughAP() )
					break;
				if ( n + 1 < Actions.size() )
					ActionsAP[n] = Actions[n]->GetMinAP();
			}
		}
	}
	//
	if ( !IsEnd() )
		SetActionsAP();
	SetRemainActionAP();
	DebugOutput();
}
//////////////////////////////////////////////////////////////////////////////////////	
void CAICompoundAction::DebugOutput()
{
	char szStr[128];
	OutputDebugString( "[ACTIONS AP ITERATOR] " );
	for ( int n = 0; n < Actions.size(); ++n )
	{
		sprintf( szStr, " <%d> ", Actions[n]->GetMaxAP() );
		OutputDebugString( szStr );
	}
	OutputDebugString( "\n" );
}
//////////////////////////////////////////////////////////////////////////////////////	
bool CAICompoundAction::IsEnd()
{
	return !CheckHaveEnoughAP() || ( Actions.size() == 1 ) && Actions[0]->IsNeedRemainAP();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CAICompoundAction::BeginToThink()
{
	IAIJobManager *pAIJobManager = pAIState->GetWorld()->GetAIJobManager();
	// следующие два цикла совмещать нельзя
	for ( vector< CObj<CAIAction> >::iterator i = Actions.begin(); i != Actions.end(); ++i )
		(*i)->Think();
	//
	for ( vector< CObj<CAIAction> >::iterator i = Actions.begin(); i != Actions.end(); ++i )
	{
		pAIJobManager->WaitForJob( this, *i );
		vector< CObj<CAIAction> >::iterator j = i;
		for ( ++j; j != Actions.end(); ++j )
			pAIJobManager->WaitForJob( *j, *i );
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CAICompoundAction::Think()
{
	IAIJobManager *pAIJobManager = pAIState->GetWorld()->GetAIJobManager();
	ASSERT( IsValid( pAIJobManager ) );
	//
	pAIJobManager->Remove( this );
	pAILog->Clear();
	bBestVersionFound = false;
	First();
	fExpediency = 0;
	pAIJobManager->Add( this );
	BeginToThink();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CAICompoundAction::DoJob()
{
	float fTmpExpediency = pAICriterion->GetExpediency();
	// DEBUG{
	char szStr[128];
	sprintf( szStr,"[COMPOUND ACTION] Expediency = %f\n", fTmpExpediency );
	OutputDebugString( szStr );
	// DEBUG}
	if ( fTmpExpediency > fExpediency )
	{
		pAILog->Clear();
		for ( vector< CObj<CAIAction> >::iterator i = Actions.begin(); i != Actions.end(); ++i )
			pAILog->Add( (*i)->GetAILog() );
		fExpediency = fTmpExpediency;
	}
	//
	for ( vector< CObj<CAIAction> >::reverse_iterator i = Actions.rbegin(); i != Actions.rend(); ++i )
	{
		(*i)->GetAILog()->RollBack();
		(*i)->GetAILog()->Clear();
	}
	//
	Next();
	if ( !IsEnd() )
		BeginToThink();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAICompoundAction::IsIdleJob()
{
	for ( vector< CObj<CAIAction> >::iterator i = Actions.begin(); i != Actions.end(); ++i )
		if ( !(*i)->IsIdleJob() )
			return false;
	//
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAICompoundAction::IsJobFinished()
{
	return IsEnd() || bBestVersionFound;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAICompoundAction::IsUsable()
{
	if ( Actions.empty() )
		return false;
	else
		return Actions.front()->IsUsable();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIMoveAndShootCompoundAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIMoveAndShootCompoundAction: public CAICompoundAction
{
	OBJECT_BASIC_METHODS(CAIMoveAndShootCompoundAction);
	//
	ZDATA
	ZPARENT(CAICompoundAction)
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAICompoundAction*)this); return 0; }
public:
	//
	CAIMoveAndShootCompoundAction() {}
	CAIMoveAndShootCompoundAction( IAIState *_pAIState , IAIJob *_pParentJob );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIMoveAndShootCompoundAction::CAIMoveAndShootCompoundAction( IAIState *_pAIState, IAIJob *_pParentJob ):
	CAICompoundAction( _pAIState, _pParentJob )
{
	pAICriterion = CreateAIDamageCriterion( _pAIState );
	Add( CreateAIMoveToShootAction( _pAIState, this ) );
	Add( CreateAIShootAction( _pAIState, this ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIShootAndHideCompoundAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIShootAndHideCompoundAction: public CAICompoundAction
{
	OBJECT_BASIC_METHODS(CAIShootAndHideCompoundAction);
	//
	ZDATA
	ZPARENT(CAICompoundAction)
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAICompoundAction*)this); return 0; }
public:
	//
	CAIShootAndHideCompoundAction() {}
	CAIShootAndHideCompoundAction( IAIState *_pAIState , IAIJob *_pParentJob );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIShootAndHideCompoundAction::CAIShootAndHideCompoundAction( IAIState *_pAIState, IAIJob *_pParentJob ):
	CAICompoundAction( _pAIState, _pParentJob )
{
	pAICriterion = CreateAIDamageCriterion( _pAIState );
	Add( CreateAIShootAction( _pAIState, this ) );
	Add( CreateAIMoveToHideAction( _pAIState, this ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILootAndShootCompoundAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILootAndShootCompoundAction: public CAICompoundAction
{
	OBJECT_BASIC_METHODS(CAILootAndShootCompoundAction);
	//
	ZDATA
	ZPARENT(CAICompoundAction)
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAICompoundAction*)this); return 0; }
public:
	//
	CAILootAndShootCompoundAction() {}
	CAILootAndShootCompoundAction( IAIState *_pAIState , IAIJob *_pParentJob );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAILootAndShootCompoundAction::CAILootAndShootCompoundAction( IAIState *_pAIState, IAIJob *_pParentJob ):
	CAICompoundAction( _pAIState, _pParentJob )
{
	pAICriterion = CreateAIDamageCriterion( _pAIState );
	Add( CreateAILootAction( _pAIState, this ) );
	Add( CreateAIShootAction( _pAIState, this ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIAction *CreateAIMoveAndShootCompoundAction( IAIState *pAIState, IAIJob *pParentJob )
{
	return new CAIMoveAndShootCompoundAction( pAIState, pParentJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIAction *CreateAIShootAndHideCompoundAction( IAIState *pAIState, IAIJob *pParentJob )
{
	return new CAIShootAndHideCompoundAction( pAIState, pParentJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIAction *CreateAILootAndShootAction( IAIState *pAIState, IAIJob *pParentJob )
{
	return new CAILootAndShootCompoundAction( pAIState, pParentJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NAI;
//
REGISTER_SAVELOAD_CLASS( 0x50762140, CAIMoveAndShootCompoundAction );
REGISTER_SAVELOAD_CLASS( 0x50762141, CAIShootAndHideCompoundAction );
REGISTER_SAVELOAD_CLASS( 0x51362130, CAILootAndShootCompoundAction );