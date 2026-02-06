#include "StdAfx.h"
#include "GDecalGeometry.h"
#include "GGeometry.h"
#include "Transform.h"
namespace NGScene
{
const float F_SHIFT = 0.004f;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSPoint
{
	CVec3 point, normal;
	float uv[2];
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SShadowPoly
{
	vector<SSPoint> points;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CShadowBuilder
{
	CObjectInfo::SData info;
	CPtr<CObjectInfo> pRes;
	// projection info
	CVec3 vVecU, vVecV, vShift, vProjectDirection;
	float fU0, fV0;
	// short cuts for sphere check
	float fUleng, fVleng;

	void AddShadowTriangle( SShadowPoly *pTri );
	void GeneratePoint( SSPoint *pRes, const CVec3 &pnt, const CVec3 &normal )
	{
		pRes->point = pnt;
		pRes->normal = normal;
		pRes->uv[0] = fU0 + pnt * vVecU;
		pRes->uv[1] = fV0 + pnt * vVecV;
	}
	WORD AddPoint( SSPoint &pnt );
public:
	CShadowBuilder( CObjectInfo *_pRes ) : pRes(_pRes)
	{
		info.geometry.polys.resize( 1 );
		info.geometry.polys[0] = 0;
	}
	~CShadowBuilder() { pRes->Assign( info ); }
	void Setup( const CVec3 &vOrigin, const CVec3 &vNormal, const CVec2 &vSize, float fRotation );
	bool CheckSphere( const CVec3 &ptCenter, float fRadius );
	void AddObject( const CObjectInfo &info, const SDiscretePos &_srcPos );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
inline float mixedMult( const CVec3 &a, const CVec3 &b, const CVec3 &c )
{
	return a.x*b.y*c.z + a.y*b.z*c.x + a.z*b.x*c.y - a.z*b.y*c.x - a.y*b.x*c.z - a.x*b.z*c.y;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CalcWeighted( CVec3 *pRes, const CVec3 &a, float fA, const CVec3 &b, float fB )
{
	pRes->x = ( a.x * fB - b.x * fA );
	pRes->y = ( a.y * fB - b.y * fA );
	pRes->z = ( a.z * fB - b.z * fA );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// мощная работа по наложению тени ( no welding yet )
// calc average point
static void CalcMiddlePoint( const SSPoint &a, const SSPoint &b, SSPoint *pRes, float fA, float fB )
{
	float f1 = 1 / ( fB - fA );
	fA *= f1;
	fB *= f1;
	CalcWeighted( &pRes->point, a.point, fA, b.point, fB );
	CalcWeighted( &pRes->normal, a.normal, fA, b.normal, fB );
	Normalize( &pRes->normal );
	pRes->uv[0] = a.uv[0] * fB - b.uv[0] * fA;
	pRes->uv[1] = a.uv[1] * fB - b.uv[1] * fA;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void Split( SShadowPoly *pRes, const SShadowPoly &src, const vector<float> &f )
{
	ASSERT( src.points.size() == f.size() );
	int nRes = 0;
	if ( src.points.size() < 2 )
	{
		pRes->points.resize( 0 );
		return;
	}
	pRes->points.resize( src.points.size() + 2 );
	int nPrev = src.points.size() - 1;
	for ( int k = 0; k < src.points.size(); nPrev = k++ )
	{
		float fPrev = f[nPrev], fCur = f[k];
		if ( fCur > 0 )
		{
			if ( fPrev < 0 )
				CalcMiddlePoint( src.points[nPrev], src.points[k], &pRes->points[nRes++], fPrev, fCur );
			pRes->points[nRes++] = src.points[k];
		}
		else
		{
			if ( fPrev > 0 )
				CalcMiddlePoint( src.points[nPrev], src.points[k], &pRes->points[nRes++], fPrev, fCur );
		}
	}
	pRes->points.resize( nRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static vector<float> fPlaneOffsets;
static bool DoSplit( SShadowPoly *pRes, const SShadowPoly &src, const CVec3 &vUV )
{
	fPlaneOffsets.resize( src.points.size() );
	for ( int k = 0; k < src.points.size(); ++k )
		fPlaneOffsets[k] = src.points[k].uv[0] * vUV.x + src.points[k].uv[1] * vUV.y + vUV.z;
	Split( pRes, src, fPlaneOffsets );
	return pRes->points.size() > 2;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void ProjectNormalized( CVec3 *pRes, const CVec3 &src, const CVec3 &vNormal )
{
	*pRes = src - vNormal * (src * vNormal);
	Normalize( pRes );
}
WORD CShadowBuilder::AddPoint( SSPoint &pnt )
{
	int nRes = info.verts.size();
	info.verts.resize( nRes + 1 );
	SVertex &v = info.verts[nRes];
	v.pos = pnt.point;
	v.tex.u = pnt.uv[0];
	v.tex.v = 1 - pnt.uv[1];
	v.normal = pnt.normal;
	ProjectNormalized( &v.texU, vVecU, v.normal );
	ProjectNormalized( &v.texV, vVecV, v.normal );
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// clip by planes
float fClipPlanes[4][3] = { {1,0,0}, {0,1,0}, {-1,0,1}, {0,-1, 1} };
static SShadowPoly tBuf;
void CShadowBuilder::AddShadowTriangle( SShadowPoly *pTri )
{
	//float fp1 = fpC0 + tri.pnts[0].point * vecC; // clip by front plane
	static SShadowPoly *pTSrc, *pTDst;
	pTSrc = pTri;
	pTDst = &tBuf;
	for ( int k = 0; k < 4; ++k )
	{
		DoSplit( pTDst, *pTSrc, CVec3( fClipPlanes[k][0], fClipPlanes[k][1], fClipPlanes[k][2] ) );
		if ( pTDst->points.size() < 3 )
			return;
		swap( pTSrc, pTDst );
	}
	for ( int k = 0; k < pTSrc->points.size(); ++k )
		info.geometry.indices.push_back( AddPoint( pTSrc->points[k] ) );
	info.geometry.polys.push_back( info.geometry.indices.size() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CShadowBuilder::AddObject( const CObjectInfo &info, const SDiscretePos &_srcPos )
{
	SShadowPoly tri;
	SFBTransform trans;
	_srcPos.MakeMatrix( &trans );
	const vector<WORD> &infoPolys = info.GetGeometry().polys;
	const vector<WORD> &infoIndices = info.GetGeometry().indices;
	const vector<CVec3> &infoPositions = info.GetPositions();
	for ( int k = 1; k < infoPolys.size(); ++k )
	{
		int nStart = infoPolys[k-1], nFinish = infoPolys[k];
		tri.points.resize( nFinish - nStart );
		for ( int i = nStart; i < nFinish; ++i )
		{
			CVec3 pos, normal;
			trans.forward.RotateHVector( &pos, infoPositions[ info.GetPositionIndices()[ infoIndices[i] ] ] );
			trans.backward.RotateVectorTransposed( &normal, NGfx::GetVector( info.GetVertices()[ infoIndices[i] ].normal ) );
			GeneratePoint( &tri.points[ i - nStart ], pos + vShift, normal );
		}
		if ( tri.points.size() < 3 )
			continue;
		// calc normal
		CVec3 vTriNormal = ( tri.points[2].point - tri.points[0].point ) ^ ( tri.points[1].point - tri.points[0].point );
		float fLeng = fabs( vTriNormal );
		if ( fLeng > 0 )
		{
			float fTest = vTriNormal * vProjectDirection;
			if ( fTest >= -0.01f * fLeng )
				continue;
		}
		AddShadowTriangle( &tri );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CShadowBuilder::CheckSphere( const CVec3 &ptCenter, float fRadius )
{
	float fV = fV0 + ptCenter *  vVecV;
	float fDV = fRadius * fVleng;
	if ( fV - fDV > 1 || fV + fDV < 0 )
		return false;
	float fU = fU0 + ptCenter *  vVecU;
	float fDU = fRadius * fUleng;
	if ( fU-fDU > 1 || fU + fDU < 0 )
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CShadowBuilder::Setup( const CVec3 &_vOrigin, const CVec3 &_vNormal, const CVec2 &vSize, float fRotation )
{
	float fCos = cos( fRotation ), fSin = sin( fRotation );
	CVec3 vU( vSize.x * fCos, vSize.x * fSin, 0 );
	CVec3 vV( -vSize.y * fSin, vSize.y * fCos, 0 );
	vVecU = CVec3( -vU.y, 0, -vU.x );
	vVecV = CVec3( -vV.y, 0, -vV.x );

	SHMatrix m;
	MakeMatrix( &m, VNULL3, _vNormal );
	m.RotateHVector( &vVecU, vVecU );
	m.RotateHVector( &vVecV, vVecV );

	fUleng = 1 / fabs( vVecU );
	vVecU = vVecU * ( fUleng * fUleng );
	fVleng = 1 / fabs( vVecV );
	vVecV = vVecV * ( fVleng * fVleng );

	vShift = _vNormal * F_SHIFT;
	vProjectDirection = _vNormal;
	fU0 = -( _vOrigin * vVecU );
	fV0 = -( _vOrigin * vVecV );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDecalGeometry
////////////////////////////////////////////////////////////////////////////////////////////////////
CDecalGeometry::CDecalGeometry( CPtrFuncBase<CObjectInfo> *p, const SDiscretePos &_srcPos,
	const CVec3 &_vOrigin, const CVec3 &_vNormal, const CVec2 &_vSize, float _fR )
	: pSource(p), srcPos(_srcPos), vOrigin(_vOrigin), vNormal(_vNormal), vSize(_vSize), fRotation(_fR)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDecalGeometry::Recalc()
{
	pValue = new CObjectInfo;
	CShadowBuilder sb( pValue );
	sb.Setup( vOrigin, vNormal, vSize, fRotation );
	sb.AddObject( *pSource->GetValue(), srcPos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x00442130, CDecalGeometry );
