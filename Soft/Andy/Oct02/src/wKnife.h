#ifndef __wKnife_H_
#define __wKnife_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wDynObject.h"
#include "RPGAttackMech.h"

namespace NAnimation
{
	class CSkeletonAnimator;
}
namespace NRPG
{
	class IInventoryItem;
}
namespace NWorld
{
class CWorld;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CKnifeServer: public IDynamicObject
{
	OBJECT_NOCOPY_METHODS(CKnifeServer);
	ZDATA
	STime tFinish;
	CDGPtr<NAnimation::CSkeletonAnimator> pAnimator;
	CPtr<NDb::CModel> pModel;
	CPtr<CWorld> pWorld;
	CSyncSrcBind<IVisObj> bindGlobal;
	NRPG::CAttackPortion attack;
	CObj<CActionCounter> pAction;
	CObj<NRPG::IInventoryItem> pIItem;
	CVec3 curPos;
	CVec3 velocity;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&tFinish); f.Add(3,&pAnimator); f.Add(4,&pModel); f.Add(5,&pWorld); f.Add(6,&bindGlobal); f.Add(7,&attack); f.Add(8,&pAction); f.Add(9,&pIItem); f.Add(10,&curPos); f.Add(11,&velocity); return 0; }
public:
	CKnifeServer() {}
	CKnifeServer( CWorld *pWorld, const CVec3 &vFrom, const CVec3 &vSpeed,
		STime tThrow, float fDistance, NDb::CModel *pModel, NRPG::CAttackPortion &_attack, NRPG::IInventoryItem *_pIItem );
	//
	bool Segment();
	virtual void Visit( IRenderVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
