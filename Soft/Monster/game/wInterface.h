#ifndef __WINTERFACE_H_
#define __WINTERFACE_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "Geom.h"
#include "GSceneGraph.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
	class CUnitMissionBase;
	struct SUnitInfo;
}
/////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
/////////////////////////////////////////////////////////////////////////////////////
// interface part of an object
class CObject: public CObjectBase
{
	OBJECT_BASIC_METHODS(CObject);
public:
	int nModelID;
	// some other stuff required for visualisation
	CPtr<NDG::CFuncBase<SHMatrix> > pPosition;
	//CPtr<CDGFuncBase<SSkeleton> > pAnimation;
	virtual void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
class CUnit: public CObject
{
	OBJECT_BASIC_METHODS(CUnit);
	//
	CPtr<NRPG::CUnitMissionBase> pRPG;
public:
	const NRPG::SUnitInfo& GetInfo() const;
	virtual void Serialize( CStructureSaver *pFile );
	// 
	friend class CUnitServer;
};
/////////////////////////////////////////////////////////////////////////////////////
class CWorldBase: public CFundament
{
public:
	virtual const list< CPtr<CObject> >& GetActiveObjects() const = 0;
	virtual const list< CPtr<CUnit> >& GetActiveUnits() const = 0;
	//
	virtual void CreateRandom( vector<CPtr<NRPG::CUnitMissionBase> > &mercs ) = 0;
	//
	virtual void ExecMove( CUnit *pUnit, const CVec3 &ptDest ) = 0;
	virtual bool IsExecuting() const = 0;
	virtual void StepWorld() = 0;
};
/////////////////////////////////////////////////////////////////////////////////////
CWorldBase* CreateWorld( NGScene::CCTime *pTime );
void RegisterWorldClasses( int nBase );
/////////////////////////////////////////////////////////////////////////////////////
};
/////////////////////////////////////////////////////////////////////////////////////
#endif