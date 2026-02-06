#ifndef __wGrenade_H_
#define __wGrenade_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wDynObject.h"

namespace NAnimation
{
	class CSkeletonAnimator;
	class CASphereSet;
}
namespace NDb
{
	class CRPGGrenade;
}
namespace NWorld
{
class CWorld;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGrenadeServer : public IDynamicObject
{
	OBJECT_NOCOPY_METHODS(CGrenadeServer);
	ZDATA
	STime tExplode;
	CDGPtr<NAnimation::CSkeletonAnimator> pAnimator;
	CPtr<NDb::CModel> pModel;
	CPtr<CWorld> pWorld;
	CSyncSrcBind<IVisObj> bindGlobal;
	CDBPtr<NDb::CRPGGrenade> pRPGGrenade;
	CObj<CActionCounter> pAction;
	CPtr<CUnitServer> pUnitServer; // кто кинул гранату
	CObj<NAnimation::CASphereSet> pSphere;
	bool bTimeDelayGrenade;
	STime tErase; // скорее всего граната уже вылетела за пределы зоны
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&tExplode); f.Add(3,&pAnimator); f.Add(4,&pModel); f.Add(5,&pWorld); f.Add(6,&bindGlobal); f.Add(7,&pRPGGrenade); f.Add(8,&pAction); f.Add(9,&pUnitServer); f.Add(10,&pSphere); f.Add(11,&bTimeDelayGrenade); f.Add(12,&tErase); return 0; }

	bool IsTimeToExplode( STime tCur ) { return tCur >= tExplode; }
	const CVec3 GetPosition();
public:
	CGrenadeServer() {}
	CGrenadeServer( CWorld *pWorld, const CVec3 &vFrom, const CVec3 &vSpeed,
		STime tThrow, float fTFly, NDb::CModel *pModel, NDb::CRPGGrenade *_pRPGGrenade,
		CUnitServer *_pUnitServer = 0 );
	//
	bool Segment();
	virtual void Visit( IRenderVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CClickOfDeath : public IDynamicObject
{
	OBJECT_NOCOPY_METHODS(CClickOfDeath);
	ZDATA
	CObj<CActionCounter> pAction;
	CObj<CObjectBase> pTarget;
	int nUserID;
	CRay ray;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pAction); f.Add(3,&pTarget); f.Add(4,&nUserID); f.Add(5,&ray); return 0; }
public:
	CClickOfDeath() {}
	CClickOfDeath( CActionCounter *pC, CObjectBase *pTarget, int _nUserID, const CRay &ray );
	bool Segment();
	virtual void Visit( IRenderVisitor *p ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
