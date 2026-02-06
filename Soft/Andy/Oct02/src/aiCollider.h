#ifndef __aiCollider_H_
#define __aiCollider_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BSPCollider.h"
namespace NDb
{
	class CRPGArmor;
}
namespace NWorld
{
	struct IVisObj;
}
namespace NAI
{
struct SConvexHull;
const float FP_NO_COLLISION = NCollider::F_NO_COLLISION;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSourceInfo;
struct SColliderUserInfo
{
	const SSourceInfo *pSrc;
	int nUserID;

	SColliderUserInfo( const SSourceInfo *_pSrc, int _nUserID ): pSrc(_pSrc), nUserID(_nUserID) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCollider: public NCollider::CBSPCollider<SColliderUserInfo>, public CObjectBase
{
	OBJECT_BASIC_METHODS(CCollider);
	void CollideSphere( SSphere *pSphere, const CVec3 &vel );
public:
	struct SCollisionPoint
	{
		float fDist;
		CVec3 pt;
		const SSourceInfo *pSrc;
		int nUserID;
		SPlane plane;

		SCollisionPoint() {}
		SCollisionPoint( float _fDist, const CVec3 &_pt, const SSourceInfo *_pSrc, int _nUserID, const SPlane &_plane )
			: fDist(_fDist), pt(_pt), pSrc(_pSrc), nUserID(_nUserID), plane(_plane) {}
		int operator&( CStructureSaver &f ) { ASSERT(0&&"This struct could not be serialized!"); }
	};
	void AddConvexHull( const SConvexHull &h );
	void CalcEnclosingBound( SBound *pRes, const vector<SSphere> &spheres, const vector<CVec3> &velocities );
	void CollideSliding( const vector<SSphere> &spheres, const vector<CVec3> &velocities, vector<CVec3> *pNewPositions );
	void CollideCheck( const vector<SSphere> &spheres, const vector<CVec3> &velocities, vector<bool> *pRes );
	void CollideInfo( const vector<SSphere> &spheres, const vector<CVec3> &velocities, vector<SCollisionPoint> *pRes );
	void CollideInfo( const vector<CVec3> &points, const vector<CVec3> &velocities, vector<SCollisionPoint> *pRes );
	void CollideInfo( const CVec3 &p, const CVec3 &v, SCollisionPoint *pRes );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIMap;
//bool DoesTriSphereIntersect( const CVec3 &a1, const CVec3 &b1, const CVec3 &c1,
//	const CVec3 &ptCenter, float fR );
void CollideSliding( IAIMap *pMap, const vector<SSphere> &spheres, const vector<CVec3> &velocities, 
	vector<CVec3> *pNewPositions, int nFlags );
void CollideInfo( IAIMap *pMap, const vector<SSphere> &spheres, const vector<CVec3> &velocities, 
	vector<CCollider::SCollisionPoint> *pRes, int nFlags );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
