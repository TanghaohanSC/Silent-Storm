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
/*	CalcBound( pRes, spheres, SGetSphereCenter() );
	float fMaxVel = 0, fMaxRadius = 0;
	ASSERT( spheres.size() == velocities.size() );
	for ( int i=0; i<velocities.size(); ++i )
	{
		float fVel = fabs(velocities[i]);
		if ( fVel > fMaxVel )
			fMaxVel = fVel;
		if ( spheres[i].fRadius > fMaxRadius )
			fMaxRadius = spheres[i].fRadius;
	}
	pRes->fRadius += fMaxVel + fMaxRadius;*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollider::AddConvexHull( const SConvexHull &h )
{
	if ( h.points.empty() )
		return;
	SColliderUserInfo user( &h.src, h.nUserID );
	CPtr<CBSPTree> pTree( h.pBSPTree );
	if ( pTree == 0 )
	//if ( h.src.pUserData != 0 )
	{
		//ASSERT( 0 );
		vector<STriangle> tris;
		h.tris.BuildTriangleList( &tris );
		pTree = CreateBSPTree( h.points, tris );
	}
	int nDepth = pTree->CalcDepth();
	int nNodes = pTree->CalcNodes();
	AddEntity( h.trans.backward, h.trans.forward, pTree, h.points, user );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int nCollidedTris;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFrictionCollider
{
	bool bCollision, bTerrain;
	CVec3 ptClosest;
	float fMin;
	
	SFrictionCollider(): bCollision(false), fMin(1e30f) {}
	bool operator()( float fDist, const CVec3 &ptCollision, const SPlane &plane, const SColliderUserInfo &info )
	{ 
		bCollision = true;
		if ( fDist < fMin )
		{
			fMin = fDist;
			ptClosest = ptCollision;
			bTerrain = info.pSrc->pUserData == 0 && fabs( plane.n.z ) > 0.5f;
		}
		return false; 
	}
};
void CCollider::CollideSphere( SSphere *pSphere, const CVec3 &_vel )
{
	SSphere &sphere = *pSphere;
	CVec3 vel(_vel);
	int nIteration = 0; // just for case :)
	while ( fabs2(vel) > 1e-8f && ++nIteration < 10 )
	{
		SFrictionCollider fc;
		Collide( sphere, vel, &fc ); 		
		if ( fc.bCollision )
		{
			CVec3 realVel(vel);
			Normalize(&realVel);
			realVel = fc.fMin * realVel;

			sphere.ptCenter += realVel; // move sphere
			vel -= realVel;
			CVec3 normPlane = fc.ptClosest - sphere.ptCenter;
			Normalize(&normPlane);
			float scal = vel * normPlane;
			vel = vel - (vel * normPlane) * normPlane; // project on sliding plane
			if ( fc.bTerrain )
				vel = VNULL3;
			else
			{
				// friction
				float fSlow = Min( 0.4f * fabs(scal), fabs(vel) );
				//fSlow = 0;
				realVel = vel;
				Normalize(&realVel);
				vel -= fSlow * realVel;
			}
		}
		else
		{
			sphere.ptCenter += vel;
			vel = VNULL3;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollider::CollideSliding( const vector<SSphere> &spheres, 
	const vector<CVec3> &velocities, vector<CVec3> *pNewPositions )
{
	if ( spheres.empty() )
		return;
	ASSERT( spheres.size() == velocities.size() );
	pNewPositions->clear();
	nCollidedTris = 0;
	for ( int i=0; i<spheres.size(); ++i )
	{
		SSphere sphere( spheres[i] );
		CollideSphere( &sphere, velocities[i] );
		pNewPositions->push_back( sphere.ptCenter );
	}
/*	char buf[1024];
	sprintf( buf, "avrg tris/sphere %f\n", ((float)nCollidedTris) / spheres.size() );
	OutputDebugString( buf );*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SInfoCollider
{
	bool bCollision;
	float fMin;
	CVec3 ptClosest;
	const SColliderUserInfo *pInfo;
	SPlane plane;

	SInfoCollider(): bCollision(false), fMin(1e30f) {}
	bool operator()( float fDist, const CVec3 &ptCollision, const SPlane &_plane, const SColliderUserInfo &info )
	{
		fMin = fMin; // keep compiler happy (skipping this line leads to odd internal compiler error on msvc6sp5
		bCollision = true;
		if ( fDist < fMin )
		{
			fMin = fDist;
			pInfo = &info;
			ptClosest = ptCollision;
			plane = _plane;
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
			(*pRes)[i].fDist = NCollider::F_NO_COLLISION;
		else
			(*pRes)[i] = SCollisionPoint( ic.fMin, ic.ptClosest, ic.pInfo->pSrc, ic.pInfo->nUserID, ic.plane );
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
		*pRes = SCollisionPoint( ic.fMin, ic.ptClosest, ic.pInfo->pSrc, ic.pInfo->nUserID, ic.plane );
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
	const vector<CVec3> &velocities, vector<bool> *pRes )
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
void CollideSliding( IAIMap *pMap, const vector<SSphere> &spheres, const vector<CVec3> &velocities, 
	vector<CVec3> *pNewPositions, int nFlags )
{
	if ( spheres.empty() )
		return;
	CPtr<NAI::CCollider> pCollider = new NAI::CCollider;
	NAI::CCollider &collider = *pCollider;
	float fMax = CalcMax( spheres, velocities );

	SBound bound;
	collider.CalcEnclosingBound( &bound, spheres, velocities );
	bound.Extend( fMax * 2 );
	pMap->PrepareCollider( &collider, bound, fMax * 2, nFlags );
	collider.CollideSliding( spheres, velocities, pNewPositions );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CollideInfo( IAIMap *pMap, const vector<SSphere> &spheres, const vector<CVec3> &velocities, 
	vector<CCollider::SCollisionPoint> *pRes, int nFlags )
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
}
using namespace NAI;
BASIC_REGISTER_CLASS(CCollider);
