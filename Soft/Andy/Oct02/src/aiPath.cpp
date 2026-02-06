#include "StdAfx.h"
#include "aiPath.h"
#include "aiMoves.h"
#include "wInterface.h"
#include "aiWayConstraints.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPath
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPath::DebugOutput()
{
	char buf[128];
	Zero( buf );
	for ( vector<SPathPlace>::iterator i = points.begin(); i != points.end(); ++i )
	{
		if ( i->IsIntegral() )
		{
			CDynamicCast<CPathNetwork> pRealNet( pNet );
			CNodesLayer *pLayer = pRealNet->GetLayer( i->GetLayer() );
			int nFloor = pRealNet->GetFloor( i->GetLayer() );
			float fZ = pLayer->GetCP( *i ).z;
			sprintf( buf, "x:%d, y:%d, l:%d, p:%d, d:%d, m:%d, floor: %d, z = %f\n", 
				i->GetX(), i->GetY(), i->GetLayer(), i->GetPose(), i->GetDirection(), i->IsMoving(), nFloor, fZ );
		}
		else
			sprintf( buf, "ladder# %d, step:%d, l:%d, d:%d\n", i->GetX(), i->GetY(), i->GetLayer(), i->GetDirection() );
		OutputDebugString( buf );
	}
	OutputDebugString( "End of path.\n" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPath::IsEqual( const CPath &a ) const
{
	return pNet == a.pNet && points == a.points;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool AreAdjacentDirections( int dir1, int dir2, int pose = CM_STAND )
{
	if (dir1 == dir2) 
		return true;
	if (( dir1 == ((dir2 + 1) & 7) ) || ( dir2 == ((dir1 + 1) & 7) )) 
		return true;
	if ( pose == CM_LAY )
		return false;
	if (( dir1 == ((dir2 + 2) & 7) ) || ( dir2 == ((dir1 + 2) & 7) )) 
		return true;
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPath::AddWithPossibleAction( const SPathPlace& point )
{
	ASSERT( !points.empty() );
	SPathPlace last = points.back();
	CDynamicCast<CPathNetwork> pRealNet( pNet );
	CPtr<CObjectBase> pFlipper;
	bool bIsNowOpen, bBlocksInOpenState, bBlocksInClosedState;
	if ( pRealNet->IsBlockedByFlipper( last, point, &pFlipper, &bIsNowOpen, &bBlocksInOpenState, &bBlocksInClosedState ) )
	{
		for ( int i = 0; i < actions.size(); ++i )
			if ( actions[i].pObject == pFlipper )
				bIsNowOpen = actions[i].action == PA_OPEN;
		bool bMustFlip = bIsNowOpen? bBlocksInOpenState : bBlocksInClosedState;
		if ( bMustFlip )
		{
			SPathAction a;
			a.action = bIsNowOpen ?  PA_CLOSE : PA_OPEN;
			a.where = last;
			a.pObject = pFlipper;
			actions.push_back( a );
		}
	}
	points.push_back( point );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPath::AddNewPoint( const SPathPlace& point )
{
//#ifdef _DEBUG
	/*char buf[128];
	if ( point.IsIntegral() )
		sprintf( buf, "x:%d, y:%d, l:%d, p:%d, d:%d, m:%d\n", point.GetX(), point.GetY(), point.GetLayer(), point.GetPose(), point.GetDirection(), point.IsMoving() );
	else
		sprintf( buf, "ladder# %d, step:%d, l:%d, d:%d\n", point.GetX(), point.GetY(), point.GetLayer(), point.GetDirection() );
	OutputDebugString( buf );*/
//#endif
	if ( points.empty() )
	{
		SPathPlace pushPoint( point );
		if ( !point.IsIntegral() )
			pushPoint.SetMoving( 0 );
		points.push_back( pushPoint );
		return true;
	}
	SPathPlace last = points.back();
//#ifdef _DEBUG
	ETransitionType tt = GetTransitionType( pNet, last, point );
//	sprintf( buf, "TT = %d\n", tt );
//	OutputDebugString( buf );
//#endif
	if ( !point.IsIntegral() )
	{
		if ( last.GetPose() == CM_STAND && last.GetDirection() != point.GetDirection() )
		{
			SPathPlace pushPoint( last );
			pushPoint.SetDirection( point.GetDirection() );
			pushPoint.SetMoving( 0 );
			if ( !AddWithPossibleAction( pushPoint ) )
				return false; 
		}
		SPathPlace pushPoint( point );
		pushPoint.SetMoving( 0 );
		if ( !AddWithPossibleAction( pushPoint ) )
			return false;
		return true;
	}
	if ( !last.IsIntegral() )
	{
		SPathPlace pushPoint( point );
		pushPoint.SetMoving( 0 );
		if ( point.GetPose() == CM_STAND )
			pushPoint.SetDirection( last.GetDirection() );
		if ( !AddWithPossibleAction( pushPoint ) )
			return false;
		return true;
	}
	// from this line, everything is integral
	if ( last.GetPose() == CM_LAY || last.GetPose() == CM_INACTIVE )
	{
		// changing pose to lay
		bool bTurn = point.GetX() == last.GetX() && point.GetY() == last.GetY() && point.GetDirection() != last.GetDirection();
		if ( ( point.GetPose() != CM_LAY && point.GetPose() != CM_INACTIVE ) || bTurn )
		{
			SPathPlace pushPoint( point );
			pushPoint.SetDirection( last.GetDirection() );
			if ( !AddWithPossibleAction( pushPoint ) )
				return false;
		}
		else
		// moving in lay pose
		if ( point.GetPose() == CM_LAY && last.GetPose() == CM_LAY )
		{
			SPathPlace pushPoint( point );
			int pointDir = point.GetDirection();
			if ( point.GetLayer() != last.GetLayer() )
			{
			}
			if ( AreAdjacentDirections( pointDir, last.GetDirection(), CM_LAY ) )
			{
				if ( point.GetX() == last.GetX() && point.GetY() == last.GetY() && point.GetLayer() == last.GetLayer() )
					return true;
				pushPoint.SetMoving( 1 );
			}
			if ( !AddWithPossibleAction( pushPoint ) )
				return false;
		}
		if ( last.GetPose() == CM_INACTIVE )
		{
			SPathPlace pushPoint( point );
			pushPoint.SetDirection( last.GetDirection() );
			if ( !AddWithPossibleAction( pushPoint ) )
				return false;
		}
		return true;
	}
#ifndef _DEBUG
//	ETransitionType tt = GetTransitionType( pNet, last, point );

#endif
	// from this point, last.GetPose() is CM_STAND or CM_CROACH
	if ( last.GetLayer() != point.GetLayer() && last.GetX() == point.GetX()
		&& last.GetY() == point.GetY() )
	{
		CDynamicCast<CPathNetwork> pCNet( pNet );
		ASSERT( pCNet );
		CNodesLayer *pL1 = pCNet->GetLayer( last.GetLayer() );
		CNodesLayer *pL2 = pCNet->GetLayer( point.GetLayer() );
		if ( pL1->pGroup == pL2->pGroup ) // intergrid
		{
			SPathPlace pushPoint( point );
			pushPoint.SetDirection( last.GetDirection() );
			pushPoint.SetMoving( last.IsMoving() );
			if ( !AddWithPossibleAction( pushPoint ) )
				return false;
			return true;
		}
	}	
	int 
		lastDir = last.GetDirection(),
		newDir = pNet->GetDir( last, point );
	bool bCanDo = true;
	if ( tt == TT_SAME )
		return true;
	if ( tt == TT_POSE )
	{
		newDir = point.GetDirection();
		SPathPlace pushPoint( last );
		// 1) switching to lay or inactive
		if ( point.GetPose() == CM_LAY || point.GetPose() == CM_INACTIVE )
		{
			if ( last.GetDirection() != newDir || pushPoint.IsMoving() )
			{
				pushPoint.SetDirection( newDir );
				pushPoint.SetMoving( 0 );
				if ( !AddWithPossibleAction( pushPoint ) )
					return false;
			}
			if ( !AddWithPossibleAction( point ) )
				return false;
			return true;
		}
		// 2) switching to other
		else
		{
			pushPoint.SetPose( point.GetPose() );
			pushPoint.SetMoving( 0 );
			if ( !AddWithPossibleAction( pushPoint ) )
				return false;
			return true;
		}
	}
	// начинаются настоящие ходы
	if (( tt >= TT_CLIMB_1 ) && ( tt <= TT_JUMP ))
		bCanDo = false;
	if ( tt == TT_TURN )
		newDir = point.GetDirection();
	if ( ( !last.IsMoving() ) && ( lastDir != newDir ) )
		bCanDo = false;
	if ( AreAdjacentDirections( lastDir, newDir ) && bCanDo )
	{
		SPathPlace pushPoint( point );
		pushPoint.SetMoving( 1 );
		if ( tt == TT_INTERGRID )
		{
			CDynamicCast<CPathNetwork> pCNet( pNet );
			SPathPlace pl( last ); 
			pl.SetDirection( newDir );
			float fDir = pCNet->GetDirection( pl );
			newDir = pNet->GetClosestDir( point.GetLayer() , fDir );
		}
		pushPoint.SetDirection( newDir );
		if ( !AddWithPossibleAction( pushPoint ) )
			return false;
		return true;
	}
	// add one more "stop-and-turn" point
	SPathPlace pushPoint( last );
	if ( tt == TT_INTERGRID )
	{
		CDynamicCast<CPathNetwork> pCNet( pNet );
		SPathPlace pl( last ); 
		pl.SetDirection( newDir );
		float fDir = pCNet->GetDirection( pl );
		newDir = pNet->GetClosestDir( point.GetLayer() , fDir );
	}
	pushPoint.SetDirection( newDir );
	pushPoint.SetMoving( 0 );
	if ( !AddWithPossibleAction( pushPoint ) )
		return false;
	pushPoint = point;
	pushPoint.SetMoving( 1 );
	pushPoint.SetDirection( newDir );
	if ( !AddWithPossibleAction( pushPoint ) )
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPath::Improve( bool bStrafe, const SPathPlace &strafe, IPathNetwork *pNet, EFindPathParams eParams )
{
	vector<SPathPlace> pts;
	/*
	// drop intergrid transits with same points
	for ( int i = 1; i < points.size(); ++i )
	{
		SPathPlace &prev = points[i-1];
		SPathPlace &cur = points[i];
		ETransitionType type = pNet->GetTransitionType( prev, cur );
		if ( type != TT_INTERGRID_SAME )
			pts.push_back( prev );
	}
	pts.push_back( points.back() );
	points = pts;
	pts.clear();
	*/
	//
	// cut final changepose or turns, if no need for it
	bStrafePath = bStrafe;
	if ( bStrafe )
	{
		float fAngle = 0;
		CDynamicCast<CPathNetwork> _pNet(pNet);
		if ( _pNet )
			fAngle = _pNet->GetDirection( strafe );
		// insert and remove rotates
		int nDirection = pNet->GetClosestDir( points[0].GetLayer(), fAngle );
		pts.push_back( points[0] );
		/*int nFinal = points.size();
		if ( eParams != PF_DEFAULT )
		{
			SPathPlace ptDst = points.back();
			for ( int k = points.size() - 1; k >= 1; ++k )
			{
				ETransitionType type = pNet->GetTransitionType( points, cur );
			}
		}*/
		for ( int i = 1; i < points.size(); ++i )
		{
			SPathPlace &prev = points[i-1];
			SPathPlace &cur = points[i];
			ETransitionType type = GetTransitionType( pNet, prev, cur );
			ASSERT( type != TT_NO_WAY );

			if ( ( prev.GetPose() == CM_CROUCH || prev.GetPose() == CM_STAND )
				&& ( cur.GetPose() == CM_CROUCH || cur.GetPose() == CM_STAND ) )
			{
				if ( type == TT_TURN || type == TT_POSE || type == TT_INTERGRID_SAME )
					continue;
				if ( type == TT_MOVE || type == TT_MOVE_DIAGONAL || type == TT_INTERGRID )
				{
					SPathPlace last = pts.back();
					last.SetMoving( 0 );
					if ( last.GetPose() != prev.GetPose() )
					{
						last.SetPose( prev.GetPose() );
						pts.push_back( last );
					}
					if ( last.GetDirection() != nDirection )
					{
						last.SetDirection( nDirection );
						pts.push_back( last );
					}
					SPathPlace p = cur;
					nDirection = pNet->GetClosestDir( cur.GetLayer(), fAngle );
					p.SetDirection( nDirection );
					pts.push_back( p );
					continue;
				}
			}

			SPathPlace last = pts.back();
			last.SetMoving( 0 );
			if ( last.GetPose() != prev.GetPose() )
			{
				last.SetPose( prev.GetPose() );
				pts.push_back( last );
			}
			if ( last.GetDirection() != prev.GetDirection() )
			{
				last.SetDirection( prev.GetDirection() );
				pts.push_back( last );
			}
			nDirection = pNet->GetClosestDir( cur.GetLayer(), fAngle );
			pts.push_back( cur );
		}
		points = pts;
///		OutputDebugString( "Improved path:\n" );
///		DebugOutput();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CPath::operator&( CStructureSaver &f )
{
	f.Add( 1, &points );
	f.Add( 2, &pNet );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef list<CObjectBase*> CUnitsContainer;
////////////////////////////////////////////////////////////////////////////////////////////////////
void Assign( CPointsContainer *pRes, const CUnitsContainer &units )
{
	pRes->clear();
	for ( CUnitsContainer::const_iterator i = units.begin(); i != units.end(); ++i )
	{
		NWorld::CUnit *pUnit = dynamic_cast<NWorld::CUnit*>( *i );
		if ( !pUnit ) 
			continue; 
		SPosition pos = pUnit->GetPosition().pos;
		pRes->push_back( pos.p );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SPathFinder2
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SPathFinder2 
{
	void AddFinal( CLayerColorConstraints &constraints, const SPathPlace &p, unsigned nDirection, EFindPathParams eParams );
public:
	CPath* FindPath( CPathNetwork *pNet, const SPathPlace &src, const vector<SPathPlace> &dst, const int *pCosts,
		int nPriceLimit, bool bCheckSuicide = false, bool bMoveOnly = false, EFindPathParams eParams = PF_DEFAULT, bool bCanFindNotExactPath = false );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void SPathFinder2::AddFinal( CLayerColorConstraints &constraints, const SPathPlace &p, 
	unsigned nDirection, EFindPathParams eParams )
{
	if ( eParams & PF_USE_POSE )
	{
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), nDirection, p.GetPose(), 1 ), p );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), nDirection, p.GetPose(), 0 ), p );
	}
	else
	{
		SPathPlace _p( p );
		_p.SetPose( CM_LAY );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), nDirection, CM_LAY, 1 ), _p );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), nDirection, CM_LAY, 0 ), _p );
		_p.SetPose( CM_CROUCH );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), 0, CM_CROUCH, 1 ), _p );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), 0, CM_CROUCH, 0 ), _p );
		_p.SetPose( CM_STAND );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), 0, CM_STAND, 1 ), _p );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), 0, CM_STAND, 0 ), _p );
		_p.SetPose( CM_INACTIVE );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), nDirection, CM_INACTIVE, 1 ), _p );
		constraints.AddFinalPoint( SPathPlace( p.GetX(), p.GetY(), p.GetLayer(), nDirection, CM_INACTIVE, 0 ), _p );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CPath* FindPath( IPathNetwork *_pNet, const SPathPlace &src, const vector<SPathPlace> &_dst, const int *pCosts, int nPriceLimit,
	bool bStrafe, const CUnitsContainer &accountUnits, bool bCheckSuicide, bool bMoveOnly, EFindPathParams eParams, bool bCanFindNotExactPath )
{
	CDynamicCast<CPathNetwork> pNet( _pNet );
	if ( !pNet )
	{
		ASSERT( 0 );	
		return 0;
	}

	_pNet->LockSelected( accountUnits );
	vector<SPathPlace> dst;
	for ( int k = 0; k < _dst.size(); ++k )
	{
		if ( _pNet->IsValidDestination( _dst[k] ) ) 
			dst.push_back( _dst[k] );
	}
	if ( dst.empty() )
	{
		if ( bCanFindNotExactPath )
			dst = _dst; // пусть пройти тут нигде и нельзя, но мы ведь умеем "подползать" к ближайшей проходимой точке...
		else
		{
			_pNet->UnlockSelected();
			return 0;
		}
	}
	
	CPath *pRes;
	CPointsContainer accountPoints;
	Assign( &accountPoints, accountUnits );
	_pNet->UpdateColouring( accountPoints );

	SPathFinder2 pth;
	SPathPlace a( src );
	a.SetMoving( 0 );
	pRes = pth.FindPath( pNet, a, dst, pCosts, nPriceLimit, bCheckSuicide, bMoveOnly, eParams, bCanFindNotExactPath );
	if ( pRes )
	{
	//	OutputDebugString("Found path:\n");
	//	pRes->DebugOutput();
	}

//	if ( eParams & PF_USE_POSE )
//		OutputDebugString( " Using pose. \n" );
//	if ( eParams & PF_USE_DIR )
//		OutputDebugString( " Using direction. \n" );
	if ( pRes )
		pRes->Improve( bStrafe, src, _pNet, eParams );
	_pNet->UnlockSelected();
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CutPath( CPath *pPath, const SPathPlace &from )
{
	if ( pPath->points[0] == from )
		return false;

	for ( int i = 1; i < pPath->points.size(); ++i )
	{
		if ( pPath->points[i] == from )
		{
			pPath->points.erase( pPath->points.begin(), pPath->points.begin() + i );
			return true;
		}
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsValidPath( const CPath &path )
{
	IPathNetwork *pNet = path.pNet;
	bool bRes = true;
	for ( int i = 1; i < path.points.size(); ++i )
	{
		const SPathPlace &p = path.points[i];
		if ( pNet->IsLocked( p ) )
		{
			bRes = false;
			break;
		}
	}
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsPathComplete( const CPath &path, const SPathPlace &sPlace )
{
	if ( path.points.back() == sPlace )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CPath* SPathFinder2::FindPath( CPathNetwork *pNet, const SPathPlace &src, const vector<SPathPlace> &dst, 
	const int *pCosts, int nPriceLimit, bool bCheckSuicide, bool bMoveOnly, EFindPathParams eParams, bool bCanFindNotExactPath )
{
#ifdef _DEBUG
	for ( int i = 0; i < N_MOVE_TYPES; ++i )
	{
		if ( pCosts[i] > N_PF_MAXWEIGHT - 1 )
		{
			ASSERT(0);
			return 0;
		}
	}
#endif
	if ( dst.empty() ) 
		return 0;

	// Simple path not found, find complex path
	CMovesEnumerator enumerator( pNet, pCosts, bCheckSuicide, bMoveOnly );
	CLayerColorConstraints constraints( pNet, bMoveOnly, pCosts );
	if ( nPriceLimit >= 40 )
	{
		// preparing constraints
		CUpperNetWay way;
		SZone srcZone( pNet, src );   
		hash_map<SZone, bool, SZoneHash> checkedZones;
		vector<SZone> zonesDst;
		vector<CVec3> dstCP;
		for ( vector<SPathPlace>::const_iterator i = dst.begin(); i != dst.end(); ++i )
		{
			SZone	dstZone( pNet, *i );
	    if ( checkedZones.find( dstZone ) != checkedZones.end() )
				continue; // this zone was already checked
			zonesDst.push_back( dstZone );
			dstCP.push_back( pNet->GetCP( *i ) );
			checkedZones[ dstZone ] = true;
		}
		pNet->GetWaysCalcer()->CalcBestWays( pNet, &constraints, srcZone, zonesDst, dstCP, bMoveOnly );
		if ( constraints.IsClear() ) 
			return 0;
	}
	else 
		constraints.DoNotCheck();

	// counting
	static CPathPlaceTable table;
	static CWaveCountCostsMajor<CMovesEnumerator, SPathPlace, WORD,
		CLayerColorConstraints, CPathPlaceTable, 16>
		counter( &table, &constraints, &enumerator, src );
	counter.SetMap( &enumerator );
	counter.SetPosition( src );
	for ( vector<SPathPlace>::const_iterator i = dst.begin(); i != dst.end() ; ++i )
	{
		if ( eParams & PF_USE_DIR )
		{
			if ( i->GetPose() == CM_STAND || i->GetPose() == CM_CROUCH )
				AddFinal( constraints, *i, 0, eParams );
			else
				AddFinal( constraints, *i, i->GetDirection(), eParams );
		}
		else
		{
			SPathPlace final( *i );
			for ( int k = 0; k < 8; ++k )
			{
				final.SetDirection( k );
				AddFinal( constraints, final, k, eParams );
			}
		}
	}
	counter.SetConstraints( &constraints );
	bool bPathFound = counter.Count( nPriceLimit );
	list<SPathPlace> points;
	SPathPlace cur;
	SPathPlace parent;
	if ( !bPathFound ) 
	{
		if ( nPriceLimit <= 40 || ( !bCanFindNotExactPath ) )
			return 0;
		// в точности нужная точка не найдена
		// найдем ближайшую "похожую"
		CPathPlaceTable::CMovesHash::iterator thi;
		vector<CVec3> dstCP;
		for ( int i = 0; i < dst.size(); ++i )
			dstCP.push_back( pNet->GetCP( dst[i] ) );
		int oneMoveAP = pCosts[ MT_CLIMB_4 ];
		float fKoeff = ( FP_GRID_STEP / oneMoveAP ); 
		float fBestEstimation = 1e4;
		for ( thi = table.data.begin(); thi != table.data.end(); ++thi )
		{
			SPathPlace p( thi->first );
			CVec3 pCP( pNet->GetCP( p ) );
			for ( int i = 0; i < dst.size(); ++i )
			{
				float fDist2 = fabs2( pCP - dstCP[ i ] );
				int nAP = thi->second.cost;
				if ( fDist2 > fBestEstimation * fBestEstimation ) // do not calc sqrt if it's not necessary
					continue;
				float fTotalEstimation = sqrt( fDist2 ) + fKoeff * nAP;
				if ( fTotalEstimation < fBestEstimation )
				{
					fBestEstimation = fTotalEstimation;
					cur = p;	
				}
			}
		}
		if ( fBestEstimation > 1e7 )
		{
			ASSERT(0);
			return 0;
		}
		parent = cur;
	}
	else // path was found
	{
		cur = counter.firstReached;
		parent = constraints.GetFinalParent( cur );
		if ( cur.GetData() != parent.GetData() )
			points.push_front( parent );
		points.push_front( cur );
	}
	while ( cur.GetData() != src.GetData() )
	{
		SPathPlace _cur( cur );
//		if ( _cur.GetPose() == CM_STAND )
//			_cur.SetDirection( 0 );
		CPathPlaceTable::CMovesHash::iterator k = table.data.find( _cur );
		if ( k == table.data.end() )
		{
			ASSERT( 0 ); // it's an error - counter returned "path found", but no path itself
			return 0;
		}
		cur = k->second.parentPos;
		points.push_front( cur );
	}
	if ( !points.empty() )
		ASSERT( points.front() == src );
	CPath *pRes = new CPath;
	pRes->pNet = pNet;
//	OutputDebugString("Adding Points to way\n");
	for ( list<SPathPlace>::iterator i = points.begin(); i != points.end(); ++i )
	{
		if ( !pRes->AddNewPoint( *i ) )
			return pRes;
	}
	// Cut final turn ( or changepose ), if any. Also cut final "TT_SAME's"
	if ( pRes->points.size() < 2 )
		return pRes;
	SPathPlace last = pRes->points.back();
	pRes->points.pop_back();
	SPathPlace preLast = pRes->points.back();
	ETransitionType tt = GetTransitionType( pNet, preLast, last );
	while ( tt == TT_SAME )
	{
		if ( pRes->points.size() < 2 )
			return pRes;
		pRes->points.pop_back();
		preLast = pRes->points.back();
		tt = GetTransitionType( pNet, preLast, last );
	}

	if ( ( tt == TT_POSE ) && (!( eParams & PF_USE_POSE )) )
		return pRes;
	if ( ( tt == TT_TURN ) && (!( eParams & PF_USE_DIR )) )
		return pRes;
	// eliminate AI bug with multiple turns when shooting and using pose
	if ( (tt == TT_POSE ) && (!( eParams & PF_USE_DIR )) && ( pRes->points.size() > 1 ) )
	{
		pRes->points.pop_back();
		SPathPlace prePreLast = pRes->points.back();
		tt = GetTransitionType( pNet, prePreLast, preLast );
		if ( tt == TT_TURN )
			last.SetDirection( prePreLast.GetDirection() );
		else
			pRes->points.push_back( preLast );
	}
	pRes->points.push_back( last );
	if ( ( eParams == PF_USE_POSEDIR ) && last.GetData() != parent.GetData() )
	{
		last.SetMoving( 0 ); parent.SetMoving( 0 );
		pRes->points.push_back( parent );
	}
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NAI;
REGISTER_SAVELOAD_CLASS( 0x00641180, CPath );
