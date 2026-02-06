#ifndef __wOSBase_H_
#define __wOSBase_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "wInterface.h"
#include "RPGAttackMech.h"
#include "wDynObject.h"
namespace NAnimation
{
	class CSkeletonAnimator;
	class CSkeletonState;
}
namespace NDb
{
	class CObject;
	class CDebrisMaterial;
	class CContainerModel;
}
namespace NRPG
{
	class IObject;
}
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SObjectPlace
{
	CVec3 ptPos, ptScale;
	float fAngle;
	int nFloor;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorld;
class CObjectServerBase: public IObject, public NRPG::IAttackable
{
protected:
	ZDATA
	CObj<NRPG::IObject> pRPG;
	CPtr<NDb::CObject> pDbObject;
	int nDestroyStage;
	bool bLightMap;
	CSyncSrcBind<IVisObj> bindGlobal;
	SObjectPlace position;
	CPtr<CWorld> pWorld;
	STime tStageChange;
	STime tLastSound;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pRPG); f.Add(3,&pDbObject); f.Add(4,&nDestroyStage); f.Add(5,&bLightMap); f.Add(6,&bindGlobal); f.Add(7,&position); f.Add(8,&pWorld); f.Add(9,&tStageChange); f.Add(10,&tLastSound); return 0; }
protected:
	void Kill( const CVec3 &ptDir );
	void CreateTransform( SFBTransform *pRes );
	NDb::CDebrisMaterial* GetDebrisMaterial() const;
	void AddEffects( IRenderVisitor *p, NDb::CContainerModel *pCont, const SFBTransform &rv );
	void AddEffects( IAIVisitor *p, NDb::CContainerModel *pCont, const SFBTransform &rv );
	void AddObject( IRenderVisitor *p, NDb::CObject *pO, const SFBTransform &rv );
	void AddObject( IAIVisitor *p, NDb::CObject *pO, const SFBTransform &rv );
	void AddObject( ISoundVisitor *p, NDb::CObject *pO, const SFBTransform &rv );
	CWorld* GetWorld() const { return pWorld; }
public:
	CObjectServerBase() {}
	CObjectServerBase( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
		NDb::CObject *pO, NRPG::IObject *pRPG );
	
	bool CheckStability();
	
	// NRPG::IAttackable
	virtual int ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor );
	// IObject
	virtual bool IsTargetable() const;
	// IVisObj
	virtual void Visit( IRenderVisitor* );
	virtual void Visit( IAIVisitor* );
	virtual void Visit( ISoundVisitor *p );

	virtual bool NeedSegment() const;
	virtual bool Segment();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IGetApproaches
{
public:
	virtual void GetApproaches( vector<NAI::SPathPlace> *pRes, NAI::IPathNetwork *pNet ) const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAnimObjectServerBase: public CObjectServerBase, public IGetApproaches, public IDynamicObject
{
protected:
	ZDATA_(CObjectServerBase)
	CDBPtr<NDb::CSkeleton> pSkeleton;
	CObj<CFuncBase<STime> > pTime;
	CObj<NAnimation::CSkeletonAnimator> pAnimator;
	CObj<NAnimation::CSkeletonState> pState;
	CObj<CActionCounter> pAction;
	STime tEnd;
	bool bBroken;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CObjectServerBase*)this); f.Add(2,&pSkeleton); f.Add(3,&pTime); f.Add(4,&pAnimator); f.Add(5,&pState); f.Add(6,&pAction); f.Add(7,&tEnd); f.Add(8,&bBroken); return 0; }
protected:
	void IdleOn();
	void IdleOff();
	void PlayDestructAnimation();
public:
	CAnimObjectServerBase() {}
	CAnimObjectServerBase( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
		NDb::CObject *pO, NRPG::IObject *pRPG, CFuncBase<STime> *_pTime );

	// IGetApproaches
	virtual void GetApproaches( vector<NAI::SPathPlace> *pRes, NAI::IPathNetwork *pNet ) const;
	// IVisObj
	virtual void Visit( IRenderVisitor* );
	virtual void Visit( IAIVisitor* );
	// IDynamicObject
	virtual bool Segment();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetMask( NDb::CAIGeometry *pAIGeometry );
}
#endif
