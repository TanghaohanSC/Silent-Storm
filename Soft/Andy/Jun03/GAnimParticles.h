#ifndef __GANIMPARTICLES_H_
#define __GANIMPARTICLES_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GAnimBase.h"
#include "..\DBFormat\DataPhys.h"
#include "Physics.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
	class IAIMap;
}
namespace NGScene
{
	class CCInt;
}
namespace NWorld
{
	class CUnitServer;
}
namespace NRPG
{
	class IInventoryItem;
}
namespace NAnimation
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EPhysCase
{
	PH_GRENADE_ATTACK,
	PH_THROW_OUT,
	PH_FALL_FROM_BODY
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAParticle : public CAnimator
{
public:
	struct SStick
	{
		int nP1;
		int nP2;
		float fRest;
		float fSumInvMasses;
		int nMax;
	};
protected:
	struct SRestoreBone
	{
		int nP1;
		int nP2;
		int nP3;
		SRestoreBone( int a = -1, int b = -1, int c = -1 ) { nP1 = a; nP2 = b; nP3 = c; }
	};
	ZDATA_(CAnimator)
	vector<int> nParticleBones; // correspondence nParticle -> nSkeletonBone
	vector<CVec3> particles; // current particle positions
	vector<SRestoreBone> restore; // for restoring bones from particles
	vector<SStick> sticks; // stick pairs
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAnimator*)this); f.Add(2,&nParticleBones); f.Add(3,&particles); f.Add(4,&restore); f.Add(5,&sticks); return 0; }
protected:
	void InitSticks( const int *pPairs, int nPairs );
	void IterateSticks();
	void RestoreTriangle( vector<CVec3> parts, int nP1, int nP2, int nP3, int nTargetBone, SSkeletonPose *pPose );
	void RestoreLine( vector<CVec3> parts, int nP1, int nP2, int nTargetBone, SSkeletonPose *pPose );
	void RestoreAll( vector<CVec3> parts, SSkeletonPose *pPose );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CParticleSkeleton : public CAParticle
{
	OBJECT_BASIC_METHODS(CParticleSkeleton);
	ZDATA_(CAParticle)
	STime tCurrent;
	vector<CVec3> last;
	vector<SStick> softs;
	vector<float> fInvMasses;
	vector<CVec3> spine;
	int nStep;
	bool bStopped;
	bool bStartPhys;
	SSkeletonPose defaultPose;
	CPtr<NWorld::CUnitServer> pUnit;
	vector<CQuat> addRandom;

public:
	CPtr<NAI::IAIMap> pMap;
	CPtr<CAnimator> pInput;
	STime tActive, tMax;
	CVec3 impact;

	CPtr<CAnimator> pEffector;
private:
	CObj<NGScene::CCInt> pFloor;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAParticle*)this); f.Add(2,&tCurrent); f.Add(3,&last); f.Add(4,&softs); f.Add(5,&fInvMasses); f.Add(6,&spine); f.Add(7,&nStep); f.Add(8,&bStopped); f.Add(9,&bStartPhys); f.Add(10,&defaultPose); f.Add(11,&pUnit); f.Add(12,&addRandom); f.Add(13,&pMap); f.Add(14,&pInput); f.Add(15,&tActive); f.Add(16,&tMax); f.Add(17,&impact); f.Add(18,&pEffector); f.Add(19,&pFloor); return 0; }

	void Step();
	void Init( STime t, SSkeletonPose &pose, SSkeletonPose &nextPose, const CVec3 &impact );
	void CalcParticles( vector<CVec3> *pParticles, SSkeletonPose &pose );
	void CheckIfCollided( STime t, SSkeletonPose *pPose, SSkeletonPose *pNextPose );
	void BeStopped();
	void GetInputFrame( STime t, SSkeletonPose *pPose );
public:
	CParticleSkeleton() {}
	CParticleSkeleton( NGScene::CCInt *_pFloor, CPtrFuncBase<CFileSkeletonInfo> *_pSkeleton, NWorld::CUnitServer *pUnit );
	virtual bool NeedUpdate( STime t );
	virtual void GetFrame( STime t, SSkeletonPose *pPose );
	virtual string GetDebugInfo() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CASphereSet : public CAnimator
{
	OBJECT_BASIC_METHODS(CASphereSet);
	ZDATA_(CAnimator)
	NPhysics::CAnimator animator;
	//vector<SMassSphere> spheres;
	//CVec3 boundSize;
	
	//float fMass;
	CVec3 massCenter;
	//SHMatrix inertiaInvBody;
	//float fBoundSize;

	//CVec3 pos;
	//CQuat rot;
	//CVec3 p; // linear moment
	//CVec3 l; // angular moment

	//CVec3 lastPos; // for interpolation
	//CQuat lastRot;
	STime tCurrent;
	//STime tLast; // for case - finish time

	//bool bStopped; // is resting
	//bool bCollided;
	//CDBPtr<NDb::CDBPhysParams> pPhys;
	//CPtr<CObjectBase> pIgnore;
public:
	CDGPtr< CFuncBase<STime> > pTime;
	CPtr<NAI::IAIMap> pMap;

private:
	int nFloor;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAnimator*)this); f.Add(2,&animator); f.Add(3,&massCenter); f.Add(4,&tCurrent); f.Add(5,&pTime); f.Add(6,&pMap); f.Add(7,&nFloor); return 0; }

	//CVec3 posVel; // from p
	//CVec3 rotVel; // from l
	//SHMatrix inertiaInv; //from l

	//void CalcVelocities();
	//void CalcPosVel();
	//void CalcRotVel();
	//void AdvanceIntegrator( float fTime, bool bMidPoint = false );
	//CVec3 PredictPosition( float fTime, bool bMidPoint = false );
	void Step();
	//float GetEnergy();
	//void ApplyCollision( CVec3 ptColl, CVec3 vel );
	void AddSphere( const SSphere &sphere, float fMass );
	//void BeStopped();
public:
	CASphereSet() {}
	CASphereSet( const vector<SMassSphere> &_spheres, const CVec3 &boundCenter, const CVec3 &_boundSize, CObjectBase *pIgnore, int _nFloor );
	
	//void InitSpheres( );
	//void InitBound( );
	void Init( STime t, const CVec3 &pos, const CQuat &rot, const CVec3 &vel, bool bMassCenter, EPhysCase physCase, NRPG::IInventoryItem *pItem = 0 );
	bool HasStopped();
	virtual void GetFrame( STime t, SSkeletonPose *pPose );
	bool DidCollide() { return false; }//bCollided; } 
	void Calc( STime t );
	int GetFloor() const { return nFloor; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CATrailPath: public CAnimator
{
	OBJECT_BASIC_METHODS(CATrailPath);
public:
	struct STrailPoint
	{
		ZDATA
		CVec3 vDir;
		CVec3 vPosition;
		STime sPassTime;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&vDir); f.Add(3,&vPosition); f.Add(4,&sPassTime); return 0; }
	};
private:
	ZDATA_(CAnimator)
	int nTrailCount;
	STime sCast;
	vector<STrailPoint> trailpointsSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CAnimator*)this); f.Add(2,&nTrailCount); f.Add(3,&sCast); f.Add(4,&trailpointsSet); return 0; }

public:
	CATrailPath() {}

	void Init( STime sCast, const vector<STrailPoint> &trail );
	void GetFrame( STime sFrame, SSkeletonPose *pPose );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif