#ifndef __wRocket_H_
#define __wRocket_H_
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
	class IClipItem;
}
namespace NWorld
{
class CWorld;
struct IVisObj;
class CUnitServer;

////////////////////////////////////////////////////////////////////////////////////////////////////
class CRocketServer: public IDynamicObject
{
	OBJECT_NOCOPY_METHODS(CRocketServer);
	ZDATA
	STime tFinish;
	CDGPtr<NAnimation::CSkeletonAnimator> pAnimator;
	CPtr<NDb::CModel> pModel;
	CPtr<CWorld> pWorld;
	CSyncSrcBind<IVisObj> bindGlobal;
	NRPG::CAttackPortion attack;
	CObj<CActionCounter> pAction;
	CObj<NRPG::IClipItem> pRocket;
	CVec3 curPos;
	CVec3 velocity;
	CPtr<CUnitServer> pIgnored;
	CPtr<NDb::CEffect> pEffect;
	CPtr< CFuncBase<SFBTransform> > pFlame;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&tFinish); f.Add(3,&pAnimator); f.Add(4,&pModel); f.Add(5,&pWorld); f.Add(6,&bindGlobal); f.Add(7,&attack); f.Add(8,&pAction); f.Add(9,&pRocket); f.Add(10,&curPos); f.Add(11,&velocity); f.Add(12,&pIgnored); return 0; }
public:
	CRocketServer() {}
	CRocketServer( CWorld *pWorld, const CVec3 &vFrom, const CVec3 &vSpeed,
		STime tThrow, float fDistance, NDb::CModel *pModel, NRPG::CAttackPortion &_attack, 
		NRPG::IClipItem *_pRocket, CUnitServer *_pIgnored, NDb::CEffect *_pEffect = 0 );
	//
	bool Segment();
	virtual void Visit( IRenderVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
