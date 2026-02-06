#ifndef __GSCENEGRAPH_H_
#define __GSCENEGRAPH_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
#include "geom.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
	using namespace NDG;
/////////////////////////////////////////////////////////////////////////////////////
typedef DWORD STime;
DEFINE_DG_CONSTANT_NODE( CCTime, STime );
DEFINE_DG_CONSTANT_NODE( CCMSR, SHMatrix );
DEFINE_DG_CONSTANT_NODE( CCFBTransform, SFBTransform );
DEFINE_DG_CONSTANT_NODE( CCVec3, CVec3 );
DEFINE_DG_CONSTANT_NODE( CCFloat, float );
/////////////////////////////////////////////////////////////////////////////////////
// parameters 2 MSR converter
class CMSRConvert: public CFuncBase<SHMatrix>
{
	OBJECT_BASIC_METHODS(CMSRConvert);
protected:
	virtual void Update();
public:
	CDGPtr< CFuncBase<CVec3> > pMove;
	CDGPtr< CFuncBase<CVec3> > pRotate;
	CDGPtr< CFuncBase<CVec3> > pScale;
	//
	void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
class CVec3Convert: public CFuncBase<CVec3>
{
	OBJECT_BASIC_METHODS(CVec3Convert);
protected:
	virtual void Update();
public:
	CDGPtr< CFuncBase<float> > pX;
	CDGPtr< CFuncBase<float> > pY;
	CDGPtr< CFuncBase<float> > pZ;
	//
	void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
// testing dynamics node only
class CSinus: public CFuncBase<float>
{
	OBJECT_BASIC_METHODS(CSinus);
protected:
	virtual void Update();
public:
	float fpMn, fpFreq, fpAdd;
	CDGPtr< CFuncBase<STime> > pTime;
	//
	CSinus() { fpMn = 1; fpFreq = 1; fpAdd = 0; }
	void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
class CGSceneGraph: public CBaseScene
{
	CObj<CCTime> pTime;
public:
	CGSceneGraph() { pTime = new CCTime; pTime->Set(0); }
	CCTime* GetTime() const { return pTime; }
	virtual void Serialize( CStructureSaver *pFile ) { pFile->AddObject( 1, &pTime ); }
};
/////////////////////////////////////////////////////////////////////////////////////
}
void RegisterSceneGraphClasses( int nBase );
/////////////////////////////////////////////////////////////////////////////////////
#endif