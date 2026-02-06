#ifndef __AIPATH_H_
#define __AIPATH_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "aiPosition.h"
#include "Grid.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
	class CUnit;
}
namespace NAI
{
class IAIMap;
class IPathNetwork;
const int N_PF_MAXWEIGHT = 16;
enum EPathAction
{
	PA_OPEN,
	PA_CLOSE
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPath: public CObjectBase
{
	OBJECT_BASIC_METHODS(CPath);
	bool AddWithPossibleAction( const SPathPlace& point );
public:
	struct SPathAction
	{
		SPathPlace where;
		EPathAction action;
		CPtr<CObjectBase> pObject;
	};
	vector<SPathPlace> points;
	CPtr<IPathNetwork> pNet;
	vector<SPathAction> actions;
	bool bStrafePath;
	
	void DebugOutput();
	void Improve( bool bStrafe, const SPathPlace &strafe, IPathNetwork *pNet, EFindPathParams eParams );
	bool AddNewPoint( const SPathPlace& point );
	bool IsEqual( const CPath &a ) const;
	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CPath* FindPath( IPathNetwork *pNet, const SPathPlace &src, const vector<SPathPlace> &dst, const int *pCosts, int nPriceLimit, 
	bool bStrafe, const list<CObjectBase*> &accountUnits, bool bCheckSuicide = false,
	bool bMoveOnly = false, EFindPathParams eParams = PF_DEFAULT, bool bCanFindNotExactPath = false );
bool CutPath( CPath *pPath, const SPathPlace &from );
bool IsValidPath( const CPath &path );
bool IsPathComplete( const CPath &path, const SPathPlace &sPlace );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif




















