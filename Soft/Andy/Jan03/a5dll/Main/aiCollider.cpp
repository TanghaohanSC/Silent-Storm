#include "StdAfx.h"
#include "aiCollider.h"
#include "Bound.h"
#include "aiObject.h"
#include "aiMap.h"
#include "../dbformat/DataRPG.h"
#include "wTSFlags.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCollider
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollider::CalcEnclosingBound( SBound *pRes, const vector<SSphere> &spheres, const vector<CVec3> &velocities )
{
	ASSERT( spheres.size() == velocities.size() );
	SBoundCalcer bc;
	for ( int k = 0; k < spheres.size(); ++k )
	{
		const SSphere &s = spheres[k];
		bc.Add( s.ptCenter, s.fRadius );
		bc.Add( s.ptCenter + velocities[k], s.fRadius );
	}
	bc.Make( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollider::SetBoundAndResolution( const SBound &b, float fLeng )
{
	PrepareCollider( b, fLeng );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollider::AddConvexHull( const SConvexHull &h )
{
	if ( h.points.empty() )
		return;
	if ( !ignoredObjects.empty() && ignoredObjects.find( h.src.pUserData.GetPtr() ) != ignoredObjects.end() )
		return;
	SColliderUserInfo user( &h.src, h.nUserID );
	if ( h.trees.empty() )
	//if ( h.src.pUserData != 0 ) - to create a new tree always
	{
		//ASSERT( 0 ); - no BSPtree generated for this geometry?
		vector<STriangle> tris;
		h.tris.BuildTriangleList( &tris );
		CPtr<CBSPTree> pTree = CreateBSPTree( h.points, tris );
		AddEntity( h.trans.backward, h.trans.forward, pTree, h.points, h.src.pUserData == 0, user );
	}
	for ( int i = 0; i < h.trees.size(); ++i )
		AddEntity( h.trans.backward, h.trans.forward, h.trees[i], h.points, h.src.pUserData == 0, user );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int nCollidedTris;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SInfoCollider
{
	bool bCollision;
	float fMinDist;
	CVec3 ptClosest;
	const SColliderUserInfo *pInfo;
	SPlane plane;

	SInfoCollider(): bCollision(false), fMinDist(1e10f) {}
	bool operator()( float fDist, const CVec3 &ptCollision, const SPlane &_plane, const SColliderUserInfo &info )
	{
		if ( fDist < fMinDist )
		{
			fMinDist = fMinDist;// keep compiler happy (skipping this line leads to odd internal compiler error on msvc6sp5
			bCollision = true;
			fMinDist = fDist;
			pInfo = &info;
			plane = _plane;
			ptClosest = ptCollision;
		}
		return false; 
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollider::CollideInfo( const vector<SSphere> &spheres, 
	const vector<CVec3> &velocities, vector<SCollisionPoint> *pRes )
{
	if ( spheres.empty() )
		return;
	ASSERT( spheres.size() == velocities.size() );
	pRes->resize( spheres.size() );
	nCollidedTris = 0;
	for ( int i=0; i<spheres.size(); ++i )
	{
		const SSphere &sphere = spheres[i];
		const CVec3 &vel = velocities[i];
		SInfoCollider ic;
		Collide( sphere, vel, &ic );
		if ( !ic.bCollision )
		{
			(*pRes)[i].fDist = NCollider::F_NO_COLLISION;
		}
		else
			(*pRes)[i] = SCollisionPoint( ic.fMinDist, ic.ptClosest, ic.pInfo->pSrc, ic.pInfo->nUserID, ic.plane );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollider::CollideInfo( const CVec3 &p, const CVec3 &v, SCollisionPoint *pRes )
{
	SInfoCollider ic;
	Collide( p, p + v, &ic );
	if ( !ic.bCollision )
		pRes->fDist = NCollider::F_NO_COLLISION;
	else
		*pRes = SCollisionPoint( ic.fMinDist, ic.ptClosest, ic.pInfo->pSrc, ic.pInfo->nUserID, ic.plane );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollider::CollideInfo( const vector<CVec3> &points, 
	const vector<CVec3> &velocities, vector<SCollisionPoint> *pRes )
{
	if ( points.empty() )
		return;
	ASSERT( points.size() == velocities.size() );
	pRes->resize( points.size() );
	nCollidedTris = 0;
	for ( int i=0; i<points.size(); ++i )
	{
		CollideInfo( points[i], velocities[i], &(*pRes)[i] );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SBoolCollider
{
	bool bCollision;
	
	SBoolCollider(): bCollision(false) {}
	bool operator()( const SColliderUserInfo &info )
	{ 
		bCollision = true;
		return true;
	}
};
void CCollider::CollideCheck( const vector<SSphere> &spheres, 
	const vector<CVec3> &velocities, vector<char> *pRes )
{
	if ( spheres.empty() )
		return;
	ASSERT( spheres.size() == velocities.size() );
	nCollidedTris = 0;
	pRes->resize( spheres.size() );
	SBoolCollider bc;
	for ( int i = 0; i < spheres.size(); ++i )
		(*pRes)[i] = CollideBool( spheres[i], velocities[i], &bc );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
static float CalcMax( const vector<SSphere> &spheres, const vector<CVec3> &velocities )
{
	float fMax = 0;
	for ( int i = 0; i < spheres.size(); ++i )
		fMax = Max( Max( sqr( spheres[i].fRadius ), fMax ), fabs2( velocities[i] ) );
	return sqrt( fMax );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CollideInfo( IAIMap *pMap, const vector<SSphere> &spheres, const vector<CVec3> &velocities, 
	vector<SCollisionPoint> *pRes, int nFlags )
{
	if ( spheres.empty() )
		return;
	CPtr<NAI::CCollider> pCollider = new NAI::CCollider;
	NAI::CCollider &collider = *pCollider;
	float fMax = CalcMax( spheres, velocities );
	
	SBound bound;
	collider.CalcEnclosingBound( &bound, spheres, velocities );
	pMap->PrepareCollider( &collider, bound, fMax * 2, nFlags );
	collider.CollideInfo( spheres, velocities, pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NAI;
BASIC_REGISTER_CLASS(CCollider);
