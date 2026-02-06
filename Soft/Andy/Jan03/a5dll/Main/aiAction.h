#ifndef __AIACTION_H_
#define __AIACTION_H_
//
#include "aiPosition.h"
//
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIState;
class IAILogContainer;
class IAIUnit;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SPlaceWithAP
{
	ZDATA
	SUnitPosition place;
	int nUnitAP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&place); f.Add(3,&nUnitAP); return 0; }
	SPlaceWithAP() {}
	SPlaceWithAP( const SUnitPosition &_place, int _nUnitAP ): place( _place ), nUnitAP( _nUnitAP ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIAction: public CObjectBase
{
	ZDATA
	CPtr<IAIState> pState;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pState); return 0; }
	//
protected:
	IAIState* GetState() const;
	IAIUnit* GetUnit() const;
	IAIUnit* GetEnemy() const;
	SPlaceWithAP GetCurrentPlace() const;
	//
public:
	CAIAction() {}
	CAIAction( IAIState *_pState );
	//
	virtual bool CanDo( const SPlaceWithAP &place ) const = 0;
	virtual void Do( IAILogContainer *pLog ) const = 0;
	virtual bool ComparePlaces( const SPlaceWithAP &p1, const SPlaceWithAP &p2 ) const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif
