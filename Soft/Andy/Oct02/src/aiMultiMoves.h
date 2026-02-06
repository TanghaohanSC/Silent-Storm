#ifndef __aiMultiMoves_H_
#define __aiMultiMoves_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "aiPosition.h"
#include "aiMoves.h"
#include "aiPath.h"
#include "aiWaveSearch.h"
#include "aiWayConstraints.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
	class CWorld;
}
namespace NAI
{
struct SAlways;
class CMultiMovesTable : public CPathPlaceTable
{
	typedef SMoveInfo<SPathPlace, WORD> SMove;
	typedef hash_map<SPathPlace, SMove, SPathPlaceHash> CMovesHash;
	friend class NWorld::CWorld;
	void PrepareAllPaths( list<SPathPlace> *pRes,
		IPathNetwork *_pNet, const SPathPlace &_src, const int *pCosts, int nPriceLimit, 
		const list<CObjectBase*> &accountUnits, 
		bool bCheckSuicide = false, bool bMoveOnly = false );
	ZDATA_(CPathPlaceTable)
	SPathPlace src;
	CPtr<IPathNetwork> pNet;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CPathPlaceTable*)this); f.Add(2,&src); f.Add(3,&pNet); return 0; }
////////////////////////////////////////////////////////////////////////////////////////////////////
	CPath *ConstructPath( const SPathPlace &cur );
////////////////////////////////////////////////////////////////////////////////////////////////////
};
////////////////////////////////////////////////////////////////////////////////////////////////////

}
#endif
