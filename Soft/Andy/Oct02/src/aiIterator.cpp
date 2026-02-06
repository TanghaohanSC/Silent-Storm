#include "StdAfx.h"

#include "wMain.h"
#include "wUnitServer.h"

#include "aiLog.h"
#include "aiUnit.h"
#include "aiState.h"
#include "aiPlayer.h"
#include "aiAction.h"
#include "aiWeapon.h"
#include "aiInventory.h"
#include "aiMultiMoves.h"
#include "aiTacticalCommander.h"

#include "aiIterator.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
int N_AI_INACTIVE_EXPEDIENCY = 10000;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIIterator: public IAIIterator
{
	ZDATA
public:
	int nMaxAP;
	CPtr<IAIState> pAIState;
	CObj<IAILogContainer> pAILog;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nMaxAP); f.Add(3,&pAIState); f.Add(4,&pAILog); return 0; }
	//
	CAIIterator() {}
	CAIIterator( IAIState *_pAIState ): 
		pAIState(_pAIState), pAILog( CreateAILogContainer() ) {}
	//
	virtual IAILogContainer *GetAILog() { return pAILog; }
	virtual void SetMaxAP( int _nMaxAP ) { nMaxAP = _nMaxAP; }
	virtual void Reset() {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIPositionIterator
///////////////////////////////////////////////////////////////////////////////////////////////////
class CAIPositionIterator: public CAIIterator
{
	typedef vector<NAI::SPathPlace> PlaceSet;
	//
	OBJECT_BASIC_METHODS(CAIPositionIterator);
	//
	ZDATA
	ZPARENT(CAIIterator)
	PlaceSet Places;
	NAI::CMultiMovesTable MovesTable;
	int nCurrentPlace;
	bool bInactivePose;
	CPtr<IAIUnit> pPrevAIUnit;
	int nPrevMaxAP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIIterator*)this); f.Add(3,&Places); f.Add(4,&MovesTable); f.Add(5,&nCurrentPlace); f.Add(6,&bInactivePose); f.Add(7,&pPrevAIUnit); f.Add(8,&nPrevMaxAP); return 0; }
	//
	void GetPossiblePlaces();
	bool FindNextPlace();
	void SetCurrentUnitPosition();
public:
	//
	CAIPositionIterator() {}
	CAIPositionIterator( IAIState *_pAIState ): 
		CAIIterator( _pAIState ), pPrevAIUnit( 0 ), nPrevMaxAP( -1 ) {}
	// IAIIterator
	virtual void First();
	virtual void Next();
	virtual bool IsEnd() { return Places.empty() || nCurrentPlace == Places.size(); }
	virtual void Reset() { pPrevAIUnit = 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIPositionIterator::GetPossiblePlaces()
{
	Places.clear();
	nCurrentPlace = 0;
	IAIUnit *pAIUnit = pAIState->GetCurrentAIUnit();
	if ( !IsValid( pAIUnit ) )
		return;
	//
	bInactivePose = pAIUnit->GetPosition().p.GetPose() == NAI::CM_INACTIVE;
	NWorld::CUnitServer *pUS = pAIUnit->GetUnitServer();
	pUS->SetWishPose( NAI::RUN );
	list<NAI::SPathPlace> TmpPlaces;
	pAIState->GetWorld()->PrepareAllPaths( &MovesTable, 
		&TmpPlaces, pUS, pAIUnit->GetPosition().p, pAIUnit->GetMaxAP(), pUS, true );
	//
	int n = 0;
	Places.resize( TmpPlaces.size() );
	for ( list<NAI::SPathPlace>::iterator i = TmpPlaces.begin(); i != TmpPlaces.end(); ++i, ++n )
		Places[n] = *i;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIPositionIterator::FindNextPlace()
{
	IAIUnit *pAIUnit = pAIState->GetCurrentAIUnit();
	if ( !IsValid( pAIUnit ) )
		return false;
	//
	while ( !IsEnd() )
	{
		NAI::SPathPlace &p = Places[nCurrentPlace];
		int nCost = MovesTable.GetCost( p );
		if ( !pAIState->IsPositionLocked( p, pAIUnit ) && 
			p.GetPose() != NAI::CM_LAY && p.GetPose() != NAI::CM_INACTIVE && 
			 nCost > nPrevMaxAP && nCost <= nMaxAP )
				return true;
		//
		++nCurrentPlace;
	}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIPositionIterator::SetCurrentUnitPosition()
{
	IAIUnit *pAIUnit = pAIState->GetCurrentAIUnit();
	//
	if ( FindNextPlace() )
	{
		SPosition pos;
		pos.p = Places[nCurrentPlace];
		pos.SetNetwork( pAIUnit->GetUnitServer()->GetWorld()->GetPathNetwork() );
		if ( bInactivePose )
			GetAILog()->Add( new CAILogExpediency( pAIUnit, N_AI_INACTIVE_EXPEDIENCY ), true );
		GetAILog()->Add( new CAILogPosition( pAIUnit, pAIUnit->GetPosition(), pos ), true );
		GetAILog()->Add( new CAILogSpendAP( pAIUnit, MovesTable.GetCost( Places[nCurrentPlace] ) ), true );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIPositionIterator::First()
{
	if ( nPrevMaxAP > nMaxAP )
		nPrevMaxAP = -1;
	//
	CPtr<IAIUnit> pAIUnit = pAIState->GetCurrentAIUnit();
	pAIUnit->SavePrevPosition();
	//
	if ( pPrevAIUnit != pAIUnit )
	{
		GetPossiblePlaces();
		pPrevAIUnit = pAIUnit;
	}
	//
	nCurrentPlace = 0;
	if ( !IsEnd() )
		SetCurrentUnitPosition();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIPositionIterator::Next()
{
	if ( !IsEnd() )
	{
		++nCurrentPlace;
		if ( !IsEnd() )
			SetCurrentUnitPosition();
		else
			nPrevMaxAP = nMaxAP;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAINumberIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAINumberIterator: public CAIIterator
{
	ZDATA
	ZPARENT(CAIIterator);
public:
	int nCount;
	int nCurrent;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIIterator*)this); f.Add(3,&nCount); f.Add(4,&nCurrent); return 0; }
	//
	CAINumberIterator() {}
	CAINumberIterator( IAIState *_pAIState, int _nCount );
	//
	virtual void ModifyState() = 0;
	virtual int GetCount();
	// IAIIterator
	virtual void First();
	virtual void Next();
	virtual bool IsEnd() 
	{
		return nCurrent >= nCount; 
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAINumberIterator::CAINumberIterator( IAIState *_pAIState, int _nCount ):
	CAIIterator( _pAIState ), nCount( _nCount )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAINumberIterator::GetCount()
{
	return nCount;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAINumberIterator::First()
{
	nCurrent = 0;
	nCount = GetCount();
  ModifyState();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAINumberIterator::Next()
{
	if ( !IsEnd() )
	{
		++nCurrent;
		if ( !IsEnd() )
			ModifyState();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIActionIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIActionIterator: public CAINumberIterator
{
	OBJECT_BASIC_METHODS(CAIActionIterator);
	//
	ZDATA
	ZPARENT( CAINumberIterator );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAINumberIterator *)this); return 0; }
public:
	//
	CAIActionIterator() {}
	CAIActionIterator( IAIState *_pAIState, int _nCount ):
		CAINumberIterator( _pAIState, _nCount ) {}
	//
	virtual void ModifyState()
	{
		pAIState->SetCurrentAction( nCurrent );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIUnitIterator
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIUnitIterator: public CAINumberIterator
{
	OBJECT_BASIC_METHODS(CAIUnitIterator)
	//
	ZDATA
	ZPARENT( CAINumberIterator );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAINumberIterator *)this); return 0; }
public:
	CAIUnitIterator() {}
	CAIUnitIterator( IAIState *_pAIState ):
		CAINumberIterator( _pAIState, 0 ) {}
	//
	virtual void ModifyState()
	{
		IAIUnit *pAIUnit = (*pAIState->GetAllyAIPlayer()->GetUnits())[nCurrent];
		pAIState->SetCurrentAIUnit( pAIUnit );
		pAIState->SetCurrentAIEnemy( pAIState->GetAITacticalCommander()->GetDangerousAttackableEnemy( pAIUnit ) );
	}
	virtual int GetCount()
	{
		return pAIState->GetAllyAIPlayer()->GetUnits()->size();
	}
};
//////////////////////////////////////////////////////////////////////////////////////
IAIIterator *CreateAIPositionIterator( IAIState *pAIState )
{
	return new CAIPositionIterator( pAIState );
}
//////////////////////////////////////////////////////////////////////////////////////	
IAIIterator *CreateAIUnitIterator( IAIState *pAIState )
{
	return new CAIUnitIterator( pAIState );
}
//////////////////////////////////////////////////////////////////////////////////////	
IAIIterator *CreateAIActionIterator( IAIState *pAIState, int nCount )
{
	return new CAIActionIterator( pAIState, nCount );
}
//////////////////////////////////////////////////////////////////////////////////////	
}
//
using namespace NAI;
//
REGISTER_SAVELOAD_CLASS( 0x52822131, CAIUnitIterator );
REGISTER_SAVELOAD_CLASS( 0x52822132, CAIPositionIterator );
REGISTER_SAVELOAD_CLASS( 0x50662180, CAIActionIterator );