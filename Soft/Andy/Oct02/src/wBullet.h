#ifndef __WBULLET_H_
#define __WBULLET_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RPGGame.h"
#include "wDynObject.h"

namespace NAnimation
{
	class CSkeletonAnimator;
}
namespace NWorld
{
class CWorld;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CBulletServer: public IDynamicObject
{
	OBJECT_NOCOPY_METHODS(CBulletServer);
private:
	ZDATA
	int nTrailCount;
	float fTrailSpeed;
	STime sCast, sLastTrailTime;
	CPtr<CWorld> pWorld;
	vector<STime> sPassTime;
	CPtr<NDb::CModel> pModel;
	CObj<CActionCounter> pAction;
	CSyncSrcBind<IVisObj> bindGlobal;
	vector<NRPG::STrailPoint> trailpointsSet;
	CDGPtr<NAnimation::CSkeletonAnimator> pAnimator;
	CPtr<CUnitServer> pShooter;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nTrailCount); f.Add(3,&fTrailSpeed); f.Add(4,&sCast); f.Add(5,&sLastTrailTime); f.Add(6,&pWorld); f.Add(7,&sPassTime); f.Add(8,&pModel); f.Add(9,&pAction); f.Add(10,&bindGlobal); f.Add(11,&trailpointsSet); f.Add(12,&pAnimator); f.Add(13,&pShooter); return 0; }

protected:
	const CVec3 GetPosition();

public:
	CBulletServer() {}
	CBulletServer( CWorld *pWorld, const vector<NRPG::STrailPoint> &trail, STime sCast, NDb::CModel *pTrailModel, float fTrailSpeed );
	//
	bool Segment();
	void Visit( IRenderVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
