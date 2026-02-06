#ifndef __AICOMPOUNDACTION_H_
#define __AICOMPOUNDACTION_H_

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIAction *CreateAIMoveAndShootCompoundAction( IAIState *pAIState, IAIJob *pParentJob = 0 );
CAIAction *CreateAIShootAndHideCompoundAction( IAIState *pAIState, IAIJob *pParentJob = 0 );
CAIAction *CreateAILootAndShootAction( IAIState *pAIState, IAIJob *pParentJob = 0 );
////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif