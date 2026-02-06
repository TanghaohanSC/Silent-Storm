#include "StdAfx.h"

#include "aiLog.h"
#include "aiUnit.h"
#include "aiState.h"
#include "aiAction.h"
#include "aiIterator.h"
#include "aiCriterion.h"

#include "aiMoveAction.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIMoveToShootAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIMoveToShootAction: public CAIIteratorBasedAction
{
	OBJECT_BASIC_METHODS(CAIMoveToShootAction)
	ZDATA
	ZPARENT( CAIIteratorBasedAction );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIIteratorBasedAction *)this); return 0; }
public:
	//
	CAIMoveToShootAction() {}
	CAIMoveToShootAction( IAIState *_pAIState, IAIJob *_pParentJob );
	//
	virtual bool IsUsable() { return IsValid( pAIState->GetCurrentAIUnit() ); } 
	virtual void Think();
	virtual void SetMaxAP( int nAP ) { CAIIteratorBasedAction::SetMaxAP( max( 0, nAP - 20 ) ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIMoveToShootAction::CAIMoveToShootAction( IAIState *_pAIState, IAIJob *_pParentJob ): 
	CAIIteratorBasedAction(_pAIState, _pParentJob)
{
	nMinAP = 0;
	nAPStep = 10;
	//
	pAIIterator = CreateAIPositionIterator( pAIState );
	pAICriterion = CreateAIToHitCriterion( pAIState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMoveToShootAction::Think()
{
	OutputDebugString( "[MOVE TO SHOOT ACTION] Think\n" );
	CAIIteratorBasedAction::Think();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIMoveToHideAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIMoveToHideAction: public CAIIteratorBasedAction
{
	OBJECT_BASIC_METHODS(CAIMoveToHideAction)
	ZDATA
	ZPARENT( CAIIteratorBasedAction );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIIteratorBasedAction *)this); return 0; }
public:
	//
	CAIMoveToHideAction() {}
	CAIMoveToHideAction( IAIState *_pAIState, IAIJob *_pParentJob );
	//
	bool IsUsable() { return IsValid( pAIState->GetCurrentAIUnit() ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIMoveToHideAction::CAIMoveToHideAction( IAIState *_pAIState, IAIJob *_pParentJob ): 
	CAIIteratorBasedAction(_pAIState, _pParentJob)
{
	nMinAP = 10;
	nAPStep = 10;
	//
	pAIIterator = CreateAIPositionIterator( pAIState );
	pAICriterion = CreateAIHideCriterion( pAIState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIAction *CreateAIMoveToShootAction( IAIState *pAIState, IAIJob *pParentJob )
{
	return new CAIMoveToShootAction( pAIState, pParentJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIAction *CreateAIMoveToHideAction( IAIState *pAIState, IAIJob *pParentJob )
{
	return new CAIMoveToHideAction( pAIState, pParentJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NAI;
//
REGISTER_SAVELOAD_CLASS( 0x53152100, CAIMoveToShootAction );
REGISTER_SAVELOAD_CLASS( 0x53152101, CAIMoveToHideAction );