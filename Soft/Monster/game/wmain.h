#ifndef __WMAIN_H_
#define __WMAIN_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "wInterface.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
/////////////////////////////////////////////////////////////////////////////////////
class CWorld;
/////////////////////////////////////////////////////////////////////////////////////
// walk along a line / stand
class CUnitPath: public NDG::CFuncBase<SHMatrix>
{
	OBJECT_BASIC_METHODS(CUnitPath);
protected:
	virtual void Update();
public:
	enum EMode
	{
		E_STAND,
		E_MOVE
	};
	NGScene::STime tMoveStart;
	EMode mode;
	CVec3 ptPos, ptDest;
	float fpAngle;
	CDGPtr< NDG::CFuncBase<NGScene::STime> > pTime;
	//
	void Init( const CVec3 &pos );
	void Move( const CVec3 &ptDest );
	//
	bool IsStanding() const { return mode == E_STAND; }
	//
	virtual void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
class CUnitServer: public CObjectBase
{
	OBJECT_BASIC_METHODS(CUnitServer);
	CObj<CUnit> pUnit;
	CObj<CUnitPath> pPath;
	CObj<NRPG::CUnitMissionBase> pRPG;
public:
	bool bIsDynamic; // true if this unit adds to CWorld::nExecCounter
	//
	CUnitServer();
	void Init( CWorld *pWorld, NRPG::CUnitMissionBase *_pRPG );
	void Segment();
	void Stand( const CVec3 &ptDest );
	void Move( const CVec3 &ptDest );
	void ShowUnit( CWorld *pWorld );
	bool HasUnit( CUnit *_pUnit ) const { return pUnit == _pUnit; }
	//
	virtual void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
class CWorld: public CWorldBase
{
	FUNDAMENT_BASIC_METHODS(CWorld);
	//
	list< CPtr<CObject> > showObjects; // sorted list
	list< CPtr<CUnit> > showUnits; // sorted list
	int nExecCounter;          // non zero when command is executed
	CPtr<NGScene::CCTime> pTime;
	list< CObj<CUnitServer> > units;
	//
	void AddObject( const CVec3 &pos, const CVec3 &size, int nModelID );
	void AddUnit( const CVec3 &pos, NRPG::CUnitMissionBase *_pRPG, int nModelID );
	CUnitServer* GetUnitServer( CUnit *pUnit );
public:
	NGScene::CCTime* GetTimeNode() const { return pTime; }
	//
	CWorld( NGScene::CCTime *_pTime = 0 ): pTime(_pTime) { nExecCounter = 0; }
	//
	void ShowUnit( CUnit *pUnit ) { showUnits.push_back( pUnit ); showUnits.sort(); }
	//
	void DoneDynamic() { --nExecCounter; }
	void StartDynamic() { ++nExecCounter; }
	//
	virtual const list< CPtr<CObject> >& GetActiveObjects() const { return showObjects; }
	virtual const list< CPtr<CUnit> >& GetActiveUnits() const { return showUnits; }
	virtual void CreateRandom( vector<CPtr<NRPG::CUnitMissionBase> > &mercs );
	virtual void ExecMove( CUnit *pUnit, const CVec3 &ptDest );
	virtual bool IsExecuting() const { return nExecCounter > 0; }
	virtual void StepWorld();
	//
	virtual void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
void RegisterWorldMainClasses( int nBase );
/////////////////////////////////////////////////////////////////////////////////////
};
/////////////////////////////////////////////////////////////////////////////////////
#endif