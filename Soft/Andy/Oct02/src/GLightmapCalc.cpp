#include "StdAfx.h"
#include "GfxBuffers.h"
#include "GLightmap.h"
#include "GLightmapCalc.h"
#include "GGeometry.h"
#include "..\Misc\RandomGen.h"
#include "GRenderExecute.h"
#include "GShadowMap.h"
#include "GfxUtils.h"
#include "Transform.h"
#include "GfxEffects.h"
#include "GMaterial.h"
#include "GScene.h"
#include "Gfx.h"

// number of sky directions used for dynamic lightmaps
const int N_SKY_DIRECTIONS = 12;
const float F_SKY_SINGLE_STRENGTH_MUL = 2.0f;// / N_SKY_DIRECTIONS;
const int N_DEPTH_CHANNELS_PER_TEX = 3;
const int N_ALL_DEPTH_CHANNELS = 0xffffffff;
const float F_MAX_SCENE_HEIGHT = 20; // CRAP need to store max height in single place
static NGfx::EColorWriteMask depthChannels[3] = 
{
	NGfx::COLORWRITE_RED, NGfx::COLORWRITE_GREEN, NGfx::COLORWRITE_BLUE
};
// radius / F_POINT_LIGHT_FALLOFF - nominal brightness distance
const float F_POINT_LIGHT_FALLOFF = 6;

////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsInside( const SBound &hold, const SBound &test )
{
	float f = fabs2( test.s.ptCenter - hold.s.ptCenter );
	return f < sqr( hold.s.fRadius - test.s.fRadius );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
static void LoadShit( NGfx::CTexture *p )
{
	NGfx::CTextureLock<NGfx::SPixel8888> l( p, 0, NGfx::WRITEONLY );//INPLACE );
	for ( int y = 0; y < l.GetYSize(); ++y )
	{
		for ( int x = 0; x < l.GetXSize(); ++x )
		{
			if ( ( x + y ) & 1 )
				l[y][x] = NGfx::SPixel8888( 255, 255, 255, 255 );
			else
				l[y][x] = NGfx::SPixel8888( 0, 0, 0, 0 );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLightState
////////////////////////////////////////////////////////////////////////////////////////////////////
static float Halton( int b, int i )
{
	float x = 0, fBInv = 1.0f / b, f = fBInv;
	while ( i )
	{
		x += f * ( i % b );
		i /= b;
		f *= fBInv;
	}
	return x;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void GenerateRandomSphereVector( CVec3 *pRes )
{
	for(;;)
	{
		CVec3 v( random.GetFloat(-1,1), random.GetFloat(-1,1), random.GetFloat(-1,1) );
		float f = fabs2( v );
		if ( f == 0 || f > 1 )
			continue;
		*pRes = v / sqrt( f );
		return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_POINT_RADIUS = 16;
const float F_POINT_STRENGTH = 0.5f * 4 * 3.14f * F_POINT_RADIUS * F_POINT_RADIUS / F_POINT_LIGHT_FALLOFF / F_POINT_LIGHT_FALLOFF;
void CLightState::AddRay( const CVec3 &vFrom, const CVec3 &vDir, const CVec3 &_vColor )
{
	if ( !IsValid(pVis) )
		return;
	if ( random.GetFloat( 0, 1 ) < 0.3f ) // absorbtion
		return;
	CRay r;
	r.ptOrigin = vFrom;
	r.ptDir = vDir;
	r.ptDir *= 1000;
	CVec3 vPoint, vNormal, vReflectColor;
	float fT;
	if ( !pVis->TraceScene( r, &fT, &vNormal, &vReflectColor ) )
		return;
	vPoint = r.Get( fT ) + vNormal * 0.01f;
	float fRadius = F_POINT_RADIUS;
	//vReflectColor *= 2;
	ASSERT( vReflectColor.x <= 1 );
	ASSERT( vReflectColor.y <= 1 );
	ASSERT( vReflectColor.z <= 1 );
	//vReflectColor.Minimize( CVec3(1,1,1) );
	CVec3 vRefColor( _vColor.r * vReflectColor.r, _vColor.g * vReflectColor.g, _vColor.b * vReflectColor.b );
	CVec3 vColor(vRefColor);
	if ( fabs2(vColor) == 0 )
		return;
	while ( fabs2(vColor) < 0.01f )
	{
		vColor *= 2;
		fRadius /= 1.41f;
	}
	if ( fRadius < 1 )
		return;
	semiPoints.push_back( SSemiPointLight( vColor, vPoint, vNormal, fRadius ) );
	CVec3 vReflect;
	GenerateRandomSphereVector( &vReflect );
	if ( vReflect * vNormal < 0 )
		vReflect -= ( 2 * ( vReflect * vNormal ) ) * vNormal;
	AddRay( vPoint + vReflect * 0.01f, vReflect, vRefColor * 0.9f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightState::AddParallel( bool bDoRender, const SSphere &_bound, const CVec3 &vDir, const CVec3 &_vColor )
{
	CVec3 vColor = _vColor;
	if ( fabs2( vColor ) < 1e-6f )
		return;
	if ( bDoRender )
		parallel.push_back( SParallelLight( vColor, vDir ) );
	if ( !pVis )
		return;
	CVec3 vCenter = _bound.ptCenter;
	float fWidth = _bound.fRadius * 2;
	float fTest = random.GetFloat( 0, F_POINT_STRENGTH );
	float fStrength = fWidth * fWidth;
	while ( fabs2( vColor ) > 0.1f )
	{
		vColor = vColor * 0.5f;
		fStrength = fStrength * 2;
	}
	while ( fabs2( vColor ) < 0.02f )
	{
		vColor = vColor * 2;
		fStrength = fStrength * 0.5f;
	}
	CVec3 vRight = CVec3(0,0,1) ^ vDir;
	if ( fabs2( vRight ) < 0.001f )
		vRight = CVec3(0,1,0) ^ vDir;
	Normalize( &vRight );
	CVec3 vUp = vRight ^ vDir;
	static int nFake = 0;
	for ( ; fTest < fStrength; fTest += F_POINT_STRENGTH )
	{
		++nFake;
		CVec3 vShifted = vCenter +
			vRight * ( Halton( 5, nFake ) * fWidth - fWidth / 2 ) +
			vUp    * ( Halton( 7, nFake ) * fWidth - fWidth / 2 );
		vShifted -= vDir * 100;
		AddRay( vShifted, vDir, vColor );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightState::AddPoint( bool bDoRender, const CVec3 &vCenter, float fRadius, const CVec3 &_vColor )
{
	if ( bDoRender )
		points.push_back( SPointLight( _vColor, vCenter, fRadius ) );
	if ( !pVis )
		return;
	float fTest = random.GetFloat( 0, F_POINT_STRENGTH );
	float fStrength = sqr( fRadius ) / sqr( F_POINT_RADIUS ) * F_POINT_STRENGTH;
	CVec3 vColor(_vColor);
	while ( fabs2( vColor ) > 0.1f )
	{
		vColor = vColor * 0.5f;
		fStrength = fStrength * 2;
	}
	while ( fabs2( vColor ) < 0.02f )
	{
		vColor = vColor * 2;
		fStrength = fStrength * 0.5f;
	}
	for ( ; fTest < fStrength; fTest += F_POINT_STRENGTH )
	{
		CVec3 vDir;
		GenerateRandomSphereVector( &vDir );
		AddRay( vCenter, vDir, vColor );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightState::CreateSimple( SLightStateCalcSeed *pSeed, const SGlobalIlluminationInfo &l )
{
	CreateScattered( pSeed, l, 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 CLightState::GenerateSkyDir( SLightStateCalcSeed *pSeed )
{
	CVec3 vSky;
	int &nSeed = pSeed->nSeed;
	for (;; )
	{
		CVec3 vAmbDir;
		float fTeta = 1 - Halton( 3, nSeed );//sin( Halton( 5, nHaltonRules ) * FP_PI2 ) * FP_PI2;
		fTeta = acos( fTeta );
		float fOmega = Halton( 2, nSeed ) * FP_2PI;
		vSky = CVec3( cos(fOmega) * sin(fTeta), sin(fOmega) * sin(fTeta), -cos(fTeta) );
		++nSeed;
		if ( vSky.z < -0.3f )
			break;
		if ( nSeed == 1000 )
			nSeed = 0;
	}
	return vSky;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightState::CreateScattered( SLightStateCalcSeed *pSeed, const SGlobalIlluminationInfo &l, IGScene *_pVis )
{
	pVis = _pVis;
	vAmbientColor = l.vAmbient;
	skyDirections.resize( N_SKY_DIRECTIONS );
	for ( int i = 0; i < skyDirections.size(); ++i )
	{
		skyDirections[i] = GenerateSkyDir( pSeed );
		AddParallel( false, l.globalBounds, skyDirections[i], l.vAmbient * F_SKY_SINGLE_STRENGTH_MUL / N_SKY_DIRECTIONS );
	}
	// sun is treated as parallel light source, usually single in scene
	for ( int k = 0; k < l.parallel.size(); ++k )
	{
		const SGlobalIlluminationInfo::SDirectional &d = l.parallel[k];
		AddParallel( !d.bIsRendered, l.globalBounds, d.vDir, d.vColor );
	}
	for ( int k = 0; k < l.points.size(); ++k )
	{
		const SGlobalIlluminationInfo::SPoint &p = l.points[k];
		AddPoint( !p.bIsRendered, p.vCenter, p.fRadius, p.vColor );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightState::TraceDynamicLMPointLight( SDynamicAmbientInfo *pRes, const CVec3 &vTarget, 
	const CVec3 &vCenter, float fRadius, const CVec3 &_vColor, const CVec3 &vSemiNormal, IGScene *pVis ) const
{
	CVec3 vDir = vTarget - vCenter;
	float fDist2 = fabs2( vDir );
	if ( fDist2 > sqr( fRadius ) )
		return;
	if ( vDir * vSemiNormal < 0 )
		return;
	float fDist = sqrt( fDist2 );
	float fK = fDist / fRadius * F_POINT_LIGHT_FALLOFF;
	float fFalloff = 1 / sqr( 1 + fK );
	if ( fFalloff < 0.1f )
		return;
	float f;
	CVec3 vA, vColor;
	CRay r;
	r.ptOrigin = vCenter;
	r.ptDir = vDir;
	if ( !pVis->TraceScene( r, &f, &vA, &vColor ) || f > 1 )
	{
		CVec3 vNormalDir = vDir / fDist;
		pRes->AddLight( _vColor * fFalloff, -vNormalDir );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightState::TraceDynamicLM( SDynamicAmbientInfo *pRes, const SBound &bv, IGScene *pVis ) const
{
	pRes->Clear();
	CRay r;
	float f;
	CVec3 vA, vColor;
	for ( int k = 0; k < parallel.size(); ++k )
	{
		const SParallelLight &p = parallel[k];
		r.ptOrigin = bv.s.ptCenter;
		r.ptDir = -p.vDir * 1000;
		if ( !pVis->TraceScene( r, &f, &vA, &vColor ) )
			pRes->AddLight( p.vColor, -p.vDir );
	}
	if ( !skyDirections.empty() )
	{
		SDynamicAmbientInfo ambient;
		ambient.Clear();
		for ( int k = 0; k < skyDirections.size(); ++k )
		{
			r.ptOrigin = bv.s.ptCenter;
			r.ptDir = -skyDirections[k] * 1000;
			if ( !pVis->TraceScene( r, &f, &vA, &vColor ) )
				ambient.AddLight( CVec3( F_SKY_SINGLE_STRENGTH_MUL / N_SKY_DIRECTIONS, 0, 0 ), -skyDirections[k] );
		}
		CVec4 vAmbient( vAmbientColor, 0 );
		for ( int k = 0; k < N_DYNAMIC_AMBIENT_INFO_COMPONENTS; ++k )
			pRes->GetVec4()[k] += sqrt( ambient.GetVec4()[k].x ) * vAmbient;
	}
	for ( int k = 0; k < points.size(); ++k )
	{
		const SPointLight &p = points[k];
		TraceDynamicLMPointLight( pRes, bv.s.ptCenter, p.vCenter, p.fRadius, p.vColor, CVec3(0,0,0), pVis );
	}
	for ( int k = 0; k < semiPoints.size(); ++k )
	{
		const SSemiPointLight &p = semiPoints[k];
		TraceDynamicLMPointLight( pRes, bv.s.ptCenter, p.vCenter, p.fRadius, p.vColor, p.vNormal, pVis );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*void CLightState::TracePerspective()
{
	CVec3 vEnter, vNormal;
	if ( !TraceFromCamera( &vEnter, &vNormal ) )
		return;

	SLightSet l;
	l.points.push_back( SSemiPointLight( CVec3(0,1,0), vEnter, vNormal, 4 ) );
	UpdateLightmaps( l );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightState::TraceParallel()
{
	CVec3 vEnter, vNormal;
	if ( !TraceFromCamera( &vEnter, &vNormal ) )
		return;
	CVec3 vDir = vEnter - pCamera->GetCP(); // for sun direction will look down
	Normalize( &vDir );

	SLightSet l;
	l.parallel.push_back( SParallelLight( CVec3(0,1,0), vEnter, vDir, 11 ) );
	UpdateLightmaps( l );
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLightmapTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
static void MakeLMTS( CTransformStack *pTS, CVec4 &vZ )
{
	CTransformStack &ts = *pTS;
	SHMatrix m;
	NGfx::MakeLMToScreenMatrix( &m, N_LM_SIZE, N_LM_SIZE );
	m.z = vZ;
	ts.Init( m );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CLightmapTracker::CLightmapTracker() : nLights(1)
{
	pLMTextureCache = new CLightmapTextureCache( 0 );
	//LoadShit( pLMTextureCache->GetTexture() );
	currentBound.SphereInit( CVec3(10,10,10), 20 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CLMRegion* CLightmapTracker::AllocRegion( IPart *pSrc, int nLOD )
{
	pSrc->RefreshObjectInfo();
	const CObjectInfo &info = *pSrc->GetObjectInfo();
	const CTPoint<int> &lmSize = info.GetLMSize();
	NCache::CQuadTreeElement elem;
	elem.nXSize = GetMSB( lmSize.x - 1 ) + 1 + nLOD;
	elem.nYSize = GetMSB( lmSize.y - 1 ) + 1 + nLOD;
	CLightmapTextureCache::SCachePlace place;
	if ( !pLMTextureCache->GetPlace( elem, &place ) )
		return 0;
	CLMRegion *pRes = new CLMRegion;
	pLMTextureCache->PerformAlloc( pRes, &place );
	pRes->lmRegion.SetRect( 
		place.resPlace.nShiftX, place.resPlace.nShiftY,
		place.resPlace.nShiftX + lmSize.x, place.resPlace.nShiftY + lmSize.y
		);
	pRes->nLOD = nLOD;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NGfx::CTexture* CLightmapTracker::GetLMTexture()
{
	return pLMTextureCache->GetTexture();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::RenderLight( 
	SLightmapTargetGeom *pTarget, const CTransformStack &ts,
	const SLightInfo &lightInfo, NGfx::CTexture *pDepth,
	ERenderOperation op, CRenderCmdList::UParameter param,
	int nStencilOp )
{
	NGfx::CRenderContext &rc = pTarget->rc;
	//render;
	rc.SetCulling( NGfx::CULL_NONE );

	CRenderCmdList res;
	const vector<SRenderFragmentInfo*> &fragments = pTarget->pGeom->GetSelected();
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &f = **i;
		SOpGenContext fi( &res.ops, &f );
		fi.AddOperation( op, 100, nStencilOp, 0, param, pDepth );
	}
	Execute( 0, &rc, ts, res, lightInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//void CLightmapTracker::ProcessLight( SLightmapTargetGeom &dst, NGfx::CTexture *pDepth, 
//	const SLightInfo &lightInfo, const SDirectionalDepthInfo &depthInfo, const SBound &bTarget )
//{
//	RenderLight( dst, lightInfo, pDepth, 
//		RO_LM_DIR_LIT_SHADOWED, &depthInfo, CVec4(0,0,0,0), CVec4( 0, 0, 0.001f, 0.5f ), DPM_NONE|ABM_ADD, bTarget );
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::ProcessPerspCheck( SLightmapTargetGeom *pTarget, NGfx::CTexture *pDepth, 
	const CVec3 &vCenter, float fRadius, const SHMatrix &mWorldToLight, const CVec3 &vDir, const CVec4 &vDepth, 
	const CVec4 &vChannel )
{
	CTransformStack lmts;
	float fRadius1 = 1.0f / fRadius;
	CVec4 vZ( vDir * fRadius1, -vCenter * vDir * fRadius1 );//0, 0, 0.001f, 0.5f );
	//CVec4 vZ( 0, 0, 0.001f, 0.5f );
	MakeLMTS( &lmts, vZ );
	SPerspDirectionalDepthInfo depthInfo;
	depthInfo.m = mWorldToLight;
	depthInfo.vDepth = vDepth;
	depthInfo.vChannelSelect = vChannel;
	SLightInfo lightInfo;
	RenderLight( pTarget, lmts, lightInfo, pDepth, 
		RO_LM_SPOT_DEPTH_CHECK, &depthInfo, DPM_NONE|STM_MARK|ABM_ZERO );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::ProcessPerspLight( SLightmapTargetGeom *pTarget, const SLightInfo &lightInfo )
{
	CTransformStack lmts;
	CVec4 vZ( 0, 0, 0.001f, 0.5f );
	MakeLMTS( &lmts, vZ );
	RenderLight( pTarget, lmts, lightInfo, 0, 
		RO_LM_SPOT_LIGHT, 0.0f, STM_TEST_CLEAR_MARK|DPM_NONE|ABM_ADD );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//void CLightmapTracker::RenderParallel( IRender *pRender, SLightmapTargetGeom &dst, const CLightState::SParallelLight &p,
//	const SSphere &_bound )
//{
//	const float F_RANGE = 40;
//	const CVec3 &_vColor = p.vColor;
//	const CVec3 &vEnter = _bound.ptCenter;
//	const CVec3 &vDir = p.vDir;
//	float fWidth = 2 * _bound.fRadius;//p.fWidth;
//	CVec3 vCameraPos = vEnter - vDir * 10;
//	// setup projection for depth map
//	CTransformStack ts;
//	ts.MakeParallel( fWidth, fWidth );
//	// fill depth info
//	SHMatrix cameraPos;
//	MakeMatrix( &cameraPos, vCameraPos, vDir );
//	ts.SetCamera( cameraPos );
//	//vLightDir = ptCenter;
//	const SHMatrix &mTrans = ts.Get().forward;
//	SDirectionalDepthInfo depthInfo;
//	depthInfo.vVecU = mTrans.x *   0.5f  + mTrans.w * ( 0.5f + 0.5f/N_DEFAULT_RT_RESOLUTION );
//	depthInfo.vVecV = mTrans.y * (-0.5f) + mTrans.w * ( 0.5f + 0.5f/N_DEFAULT_RT_RESOLUTION );
//	SLightInfo lightInfo;
//	lightInfo.bNeedSet = true;
//	lightInfo.vLightColor = _vColor;
//	lightInfo.vLightPos = CVec4( -vDir, 0 );
//	float fRange1 = 1.0f / F_RANGE;
//	depthInfo.vDepth = CVec4( -vDir * fRange1, 0.5f + fRange1 * ( vDir * vEnter ) );//0, 0, 1 / 20.0f, 0 ); //_fMaxHeight
//
//	// depth map render (using ordinary scene?!)
//	NGfx::CRenderContext rcDepth;
//	NGfx::CTexture *pDepth = shadowMapsShare.GetParticleLM();
//	rcDepth.SetTextureRT( pDepth );
//	rcDepth.ClearBuffers( 0 );
//	//if ( p.bCircle )
//	//	CopyCircleToTarget( &rcDepth );
//	rcDepth.SetColorWrite( NGfx::COLORWRITE_ALPHA );
//	CSceneFragments geom;
//	CRenderCmdList res;
//	pRender->FormDepthList( &ts, &ts, &geom, IRender::DT_STATIC );
//	MakeSingleOp( &res, geom, true, RO_DIR_DEPTH, &depthInfo );
//	Execute( pRender, &rcDepth, ts, res, lightInfo );
//	// depth map border
//	DrawBorder( &rcDepth, N_DEFAULT_RT_RESOLUTION );
//	// light each light map with special shader
//	SBound bTarget;
//	bTarget.SphereInit( _bound.ptCenter, _bound.fRadius );
//	ProcessLight( dst, pDepth, lightInfo, depthInfo, bTarget );
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void DrawExtendedQuad( NGfx::CRenderContext *pRC, const CVec3 &_vCenter, const CVec3 &vNormal, float fDist )
{
	CVec3 v1( vNormal ^ CVec3(1,0,0) );
	if ( fabs2( v1 ) <= 0.01f )
		v1 = vNormal ^ CVec3(0,1,0);
	Normalize( &v1 );
	CVec3 v2( v1 ^ vNormal );
	CVec3 vCenter( _vCenter - vNormal * fDist );
	CObj<NGfx::CGeometry> pGeom;
	vector<STriangle> tris(2);
	{
		NGfx::CBufferLock<NGfx::SGeomVecFull> points( &pGeom, 4 );
		points[0].pos = vCenter + v1 * 10 + v2 * 10;
		points[1].pos = vCenter + v1 * 10 - v2 * 10;
		points[2].pos = vCenter - v1 * 10 - v2 * 10;
		points[3].pos = vCenter - v1 * 10 + v2 * 10;
		tris[0] = STriangle(0,1,2);
		tris[1] = STriangle(0,2,3);
	}
	pRC->SetCulling( NGfx::CULL_NONE );
	NGfx::SEffConstLight color;
	color.color = CVec4(1,1,1,1);
	pRC->SetEffect( &color );
	pRC->DrawPrimitive( pGeom, tris );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetPerspDepthCamera( SHMatrix *pRes, CVec3 *pDirection, const CVec3 &_vCenter, const CVec3 &_vNormal, int nDir )
{
	CVec3 vCenter( _vCenter );
	CVec3 vvAxis(0,0,1), vvAxis2, vAxis1, vAxis2;
	vvAxis = vvAxis ^ _vNormal;
	if ( fabs2( vvAxis ) < 1e-3 )
		vvAxis = CVec3(0,1,0) ^ _vNormal;
	Normalize( &vvAxis );
	vvAxis2 = vvAxis ^ _vNormal;
	Normalize( &vvAxis2 );
	switch ( nDir )
	{
	case 0: 
		vAxis1 = vvAxis;
		vAxis2 = vvAxis2;
		break;
	case 1:
		vAxis1 = vvAxis * (-0.5f)             + vvAxis2 * (FP_SQRT_3 * 0.5f);
		vAxis2 = vvAxis * (-FP_SQRT_3 * 0.5f) + vvAxis2 * (-0.5f);
		break;
	case 2:
		vAxis1 = vvAxis * (-0.5f)             + vvAxis2 * (-FP_SQRT_3 * 0.5f);
		vAxis2 = vvAxis * ( FP_SQRT_3 * 0.5f) + vvAxis2 * (-0.5f);
		break;
	}
	CVec3 vForward = vAxis1 + vAxis2, vRight, vUp;
	Normalize( &vForward );
	vRight = vForward ^ _vNormal;
	Normalize( &vRight );
	vForward = vForward + _vNormal;
	Normalize( &vForward );
	vUp = vRight ^ vForward;
	Normalize( &vUp );
	SHMatrix &cameraPos = *pRes;
	cameraPos._11 = vRight.x; cameraPos._12 = vForward.x; cameraPos._13 = vUp.x; cameraPos._14 = vCenter.x;
	cameraPos._21 = vRight.y; cameraPos._22 = vForward.y; cameraPos._23 = vUp.y; cameraPos._24 = vCenter.y;
	cameraPos._31 = vRight.z; cameraPos._32 = vForward.z; cameraPos._33 = vUp.z; cameraPos._34 = vCenter.z;
	cameraPos._41 = 0; cameraPos._42 = 0; cameraPos._43 = 0; cameraPos._44 = 1;
	*pDirection = vForward;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SPerspShadowDir
{
	CTransformStack ts;
	SHMatrix mWorldToLight;
	CVec4 vDepth;
	CVec3 vDir;
};
void CLightmapTracker::RenderSemiPointDepthCheck( SLightmapTargetGeom *pTarget, 
	const CVec3 &vCenter, float fRadius, const CVec3 &vNormal, 
	const SLightInfo &lightInfo, bool bUseSafeClear )
{
	SPerspShadowDir dir[3];

	CSceneFragments geom;
	CTransformStack tsCube;
	tsCube.MakeParallel( fRadius, fRadius, -fRadius, fRadius );
	SHMatrix mCubeCenter;
	MakeMatrix( &mCubeCenter, vCenter, CVec3(0,0,1) );
	tsCube.SetCamera( mCubeCenter );
	pRender->FormDepthList( &tsCube, &tsCube, &geom, IRender::DT_FAST );//STATIC );
	// render depth
	NGfx::CRenderContext rcDepth;
	NGfx::CTexture *pDepth = shadowMapsShare.GetParticleLM();
	if ( !bUseSafeClear )
		rcDepth.SetTextureRT( pDepth );
	for ( int k = 0; k < 3; ++k )
	{
		switch ( k )
		{
		case 0:
			if ( bUseSafeClear )
				SetAndClearRT( &rcDepth, pDepth, NGfx::COLORWRITE_RED, N_DEFAULT_RT_RESOLUTION );
			else
			{
				rcDepth.ClearBuffers( 0 );
				rcDepth.SetColorWrite( NGfx::COLORWRITE_RED ); 
			}
			break;
		case 1: 
			if ( bUseSafeClear )
				SetAndClearRT( &rcDepth, pDepth, NGfx::COLORWRITE_GREEN, N_DEFAULT_RT_RESOLUTION );
			else
			{
				rcDepth.ClearZBuffer();
				rcDepth.SetColorWrite( NGfx::COLORWRITE_GREEN ); 
			}
			break;
		case 2: 
			if ( bUseSafeClear )
				SetAndClearRT( &rcDepth, pDepth, NGfx::COLORWRITE_BLUE, N_DEFAULT_RT_RESOLUTION );
			else
			{
				rcDepth.ClearZBuffer();
				rcDepth.SetColorWrite( NGfx::COLORWRITE_BLUE ); 
			}
			break;
		}
		CTransformStack &ts = dir[k].ts;
		const float F_FOV = 138;//36;
		ts.MakeProjective( 0.41f, F_FOV, 0.005f, fRadius );
		// fill depth info
		SHMatrix cameraPos;
		CVec3 &vDir = dir[k].vDir;
		SetPerspDepthCamera( &cameraPos, &vDir, vCenter, vNormal, k );
		//MakeMatrix( &cameraPos, vCenter, vDir );
		ts.SetCamera( cameraPos );

		SHMatrix &mWorldToLight = dir[k].mWorldToLight;
		mWorldToLight = ts.Get().forward;
		NGfx::GetTexMapFromProjection( &mWorldToLight, N_DEFAULT_RT_RESOLUTION );

		float fRange = 4 + 4 + fRadius;
		float fZero = (4 + fRadius) / fRange;
		dir[k].vDepth = CVec4( -vDir / fRange, fZero + ( vDir * vCenter ) / fRange );
		SDirectionalDepthInfo depthInfo;
		depthInfo.vDepth = dir[k].vDepth;
		// depth map render (using ordinary scene?!)
		//CSceneFragments geom;
		CRenderCmdList res;
		//pRender->FormDepthList( &ts, &ts, &geom, IRender::DT_STATIC );
		MakeSingleOp( &res, geom, true, RO_DIR_DEPTH, &depthInfo );
		Execute( pRender, &rcDepth, ts, res, lightInfo );
		// cap upper hemisphere
		DrawExtendedQuad( &rcDepth, vCenter, vNormal, 0.005f );
	}
	// depth map border
	rcDepth.SetColorWrite( NGfx::COLORWRITE_ALL );
	DrawBorder( &rcDepth, N_DEFAULT_RT_RESOLUTION );
	// mark shadow / render each dir
	for ( int k = 0; k < 3; ++k )
	{
		CVec4 vChannel(0,0,0,0);
		vChannel.m[k] = 1;
		ProcessPerspCheck( pTarget, pDepth, vCenter, fRadius, dir[k].mWorldToLight, dir[k].vDir, dir[k].vDepth, vChannel );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CLightmapTracker::RenderSemiPoint( SLightmapTargetGeom *pTarget, 
	const CVec3 &_vColor, const CVec3 &_vCenter, const CVec3 &_vNormal, float _fRadius )
{
	float fRadius = _fRadius;

	SBound bTarget;
	bTarget.SphereInit( _vCenter, fRadius );
	SSelectFragments selector( pTarget->pGeom, SBoundIntersectFilter( bTarget ) );
	if ( pTarget->pGeom->GetSelected().empty() )
		return false;

	SLightInfo lightInfo;
	lightInfo.bNeedSet = true;
	lightInfo.vLightColor = _vColor;
	lightInfo.vLightPos = CVec4( _vCenter, 0 );
	NGfx::InitRadius( &lightInfo.vRadius, fRadius );
	if ( fabs2( _vNormal ) == 0 )
	{
		RenderSemiPointDepthCheck( pTarget, _vCenter, fRadius, CVec3(0,0, 1), lightInfo, false );
		RenderSemiPointDepthCheck( pTarget, _vCenter, fRadius, CVec3(0,0,-1), lightInfo ,true );
	}
	else
		RenderSemiPointDepthCheck( pTarget, _vCenter, fRadius, _vNormal, lightInfo, false );

	// render light & clear mark
	ProcessPerspLight( pTarget, lightInfo );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CLightmapTracker::RenderPointNoShadows( SLightmapTargetGeom *pTarget, 
	const CVec3 &_vColor, const CVec3 &_vCenter, const CVec3 &_vNormal, float fRadius )
{
	if ( fabs2(_vColor) == 0 || fRadius < 0.1f )
		return false;
	SBound bTarget;
	bTarget.SphereInit( _vCenter, fRadius );
	SSelectFragments selector( pTarget->pGeom, SBoundIntersectFilter( bTarget ) );
	if ( pTarget->pGeom->GetSelected().empty() )
		return false;
	SLightInfo lightInfo;
	lightInfo.bNeedSet = true;
	lightInfo.vLightColor = _vColor;
	lightInfo.vLightPos = CVec4( _vCenter, 0 );
	NGfx::InitRadius( &lightInfo.vRadius, fRadius );
	CTransformStack lmts;
	if ( fabs2( _vNormal ) > 0 )
	{
		float fRadius1 = 1 / fRadius;
		MakeLMTS( &lmts, CVec4( _vNormal * fRadius1, - (_vCenter * _vNormal) * fRadius1 ) );
	}
	else
		MakeLMTS( &lmts, CVec4( 0, 0, 0.001f, 0.5f ) );
	RenderLight( pTarget, lmts, lightInfo, 0, 
		RO_LM_SPOT_LIGHT, 0.0f, DPM_NONE|ABM_ADD );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RenderParallelDepth( IRender *pRender, NGfx::CRenderContext *pRC, 
	const SSphere &_bound, const CVec3 &vDir, 
	SDirectionalDepthInfo *pDepthInfo )
{
	//	const float F_RANGE = 40;
	const CVec3 &vEnter = _bound.ptCenter;
	float fWidth = 2 * _bound.fRadius;//p.fWidth;
	float fRange = 2 * _bound.fRadius;
	// setup projection for depth map
	CTransformStack ts;
	ts.MakeParallel( fWidth, fWidth, -1000, 1000 );
	// fill depth info
	SHMatrix cameraPos;
	MakeMatrix( &cameraPos, vEnter, vDir );
	ts.SetCamera( cameraPos );
	//vLightDir = ptCenter;
	SHMatrix mTrans = ts.Get().forward;
	SDirectionalDepthInfo &depthInfo = *pDepthInfo;
	NGfx::GetTexMapFromProjection( &mTrans, N_DEFAULT_RT_RESOLUTION );
	depthInfo.vVecU = mTrans.x;
	depthInfo.vVecV = mTrans.y;
	float fRange1 = 1.0f / fRange;
	//depthInfo.vDepth = CVec4( -vDir * fRange1, 0.5f + fRange1 * ( vDir * vEnter ) );//0, 0, 1 / 20.0f, 0 ); //_fMaxHeight
	depthInfo.vDepth = CVec4( 0, 0, 1 / F_MAX_SCENE_HEIGHT, 0 );

	SLightInfo lightInfo;
	CSceneFragments geom;
	CRenderCmdList res;
	pRender->FormDepthList( &ts, &ts, &geom, IRender::DT_FAST );//STATIC );
	MakeSingleOp( &res, geom, true, RO_DIR_DEPTH, &depthInfo );
	Execute( pRender, pRC, ts, res, lightInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::RecalcDepthChannel( int nChannel )
{
	NGfx::CRenderContext rcDepth;
	NGfx::CTexture *pDepth = shadowMapsShare.GetLMDepthBuffer();
	if ( nChannel == N_ALL_DEPTH_CHANNELS )
	{
		rcDepth.SetTextureRT( pDepth );
		rcDepth.ClearBuffers( 0 );
	}
	else
	{
		int nMask = 0;
		for ( int i = 0; i < N_DEPTH_CHANNELS_PER_TEX; ++i )
		{
			if ( nChannel & (1<<i)  )
				nMask |= depthChannels[i];
		}
		SetAndClearRT( &rcDepth, pDepth, nMask, N_DEFAULT_RT_RESOLUTION );
	}
	bool bHasRendered = false;
	for ( int i = 0; i < N_DEPTH_CHANNELS_PER_TEX; ++i )
	{
		if ( ( nChannel & (1<<i) ) == 0 )
			continue;
		const CVec3 &vDir = skyDirs[ i ];
		rcDepth.SetColorWrite( depthChannels[i] );
		if ( bHasRendered )
			rcDepth.ClearZBuffer();
		RenderParallelDepth( pRender, &rcDepth, currentBound.s, vDir, &depthInfos[ i ] );
		bHasRendered = true;
		//pRes->lightDirs[ nDir ] = vDir;
		CVec4 &vChannel = depthInfos[ i ].vChannelSelect;
		vChannel = CVec4(0,0,0,0);
		vChannel.m[i] = 1;
	}
	rcDepth.SetColorWrite( NGfx::COLORWRITE_ALL );
	// depth map border
	DrawBorder( &rcDepth, N_DEFAULT_RT_RESOLUTION );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::RenderSkyCheck( SLightmapTargetGeom *pTarget, const CTransformStack &lmts, float fStrength )
{
	NGfx::CRenderContext &rc = pTarget->rc;
	SLightInfo lightInfo;
	lightInfo.bNeedSet = true;
	float f = fStrength;
	lightInfo.vLightColor = CVec3(f,f,f);

	if ( NGfx::GetHardwareLevel() >= NGfx::HL_GFORCE3 )
	{
		SSkyDepth3Info depthInfo;
		depthInfo.channels[0] = &depthInfos[0];
		depthInfo.channels[1] = &depthInfos[1];
		depthInfo.channels[2] = &depthInfos[2];
		depthInfo.vDirs[0] = -skyDirs[0];
		depthInfo.vDirs[1] = -skyDirs[1];
		depthInfo.vDirs[2] = -skyDirs[2];
		RenderLight( pTarget, lmts, lightInfo, shadowMapsShare.GetLMDepthBuffer(), 
			RO_LM_SKY_3LIGHT, &depthInfo, DPM_NONE|ABM_ADD );
	}
	else
	{
		for ( int k = 0; k < 3; ++k )
		{
			lightInfo.vLightPos = CVec4( -skyDirs[k], 0 );
			rc.SetColorWrite( NGfx::COLORWRITE_NONE );
			RenderLight( pTarget, lmts, lightInfo, shadowMapsShare.GetLMDepthBuffer(), 
				RO_LM_SKY_DIR_CHECK, &depthInfos[k], DPM_NONE|STM_LIGHT );
			rc.SetColorWrite( NGfx::COLORWRITE_ALPHA );
			RenderLight( pTarget, lmts, lightInfo, 0, 
				RO_LM_SKY_LIGHT, 0.0f, DPM_NONE|STM_TEST_CLEAR_MARK|ABM_ADD );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::ChooseNewSkyDirection( int nTarget )
{
	skyDirs[nTarget] = lightState.GenerateSkyDir( &ambientLightSeed );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_PASSES_PER_RECALC = 4;
void CLightmapTracker::RecalcStep( CSceneFragments *pScene )
{
	CTransformStack lmts;
	MakeLMTS( &lmts, CVec4( 0, 0, 0.001f, 0.5f ) );
	switch ( rs.nState )
	{
		case RC_START:
			{
				rs.nStep = 0;
				if ( rs.bCalcSky )
					rs.nState = RC_SKY_DEPTH;
				else if ( rs.bCalcColor )
					rs.nState = RC_COLOR_POINT;
				else
				{
					ASSERT( 0 && "asked to recalc no lightmaps" );
					return; 
				}
				NGfx::CRenderContext rc;
				rc.SetTextureRT( shadowMapsShare.GetLMTemp() );
				rc.ClearTarget( 0 );
			}
			break;
		case RC_SKY_DEPTH:
			// calc sky map
			{
				int nChannel = rs.nStep % ( N_DEPTH_CHANNELS_PER_TEX + 1 );
				if ( nChannel < N_DEPTH_CHANNELS_PER_TEX )
				{
					// pick new channel & new dir
					ChooseNewSkyDirection( nChannel );
					RecalcDepthChannel( 1 << nChannel );
				}
				else
				{
					// render sky check
					SLightmapTargetGeom lmTarget( pScene );
					lmTarget.rc.SetTextureRT( shadowMapsShare.GetLMTemp() );
					lmTarget.rc.SetColorWrite( NGfx::COLORWRITE_ALPHA );
					RenderSkyCheck( &lmTarget, lmts, F_SKY_SINGLE_STRENGTH_MUL / N_PASSES_PER_RECALC / N_DEPTH_CHANNELS_PER_TEX );
				}
				++rs.nStep;
				if ( rs.nStep == (N_DEPTH_CHANNELS_PER_TEX+1) * N_PASSES_PER_RECALC )
				{
					if ( rs.bCalcColor )
					{
						rs.nStep = 0;
						rs.nState = RC_COLOR_POINT;
					}
					else
						rs.nState = RC_APPLY;
				}
			}
			break;
		case RC_COLOR_POINT:
			{
				SLightmapTargetGeom lmTarget( pScene );
				lmTarget.rc.SetTextureRT( shadowMapsShare.GetLMTemp() );
				lmTarget.rc.SetColorWrite( (NGfx::EColorWriteMask)( NGfx::COLORWRITE_RED|NGfx::COLORWRITE_GREEN|NGfx::COLORWRITE_BLUE ) );
				while ( rs.nStep < lightState.points.size() )
				{
					const CLightState::SPointLight &p = lightState.points[rs.nStep++ ];
					if ( RenderSemiPoint( &lmTarget, p.vColor, p.vCenter, CVec3(0,0,0), p.fRadius ) )
						break;
				}
				if ( rs.nStep >= lightState.points.size() )
				{
					rs.nStep = 0;
					rs.nState = RC_COLOR_SEMI;
				}
			}
			break;
		case RC_COLOR_SEMI:
			{
				SLightmapTargetGeom lmTarget( pScene );
				lmTarget.rc.SetTextureRT( shadowMapsShare.GetLMTemp() );
				lmTarget.rc.SetColorWrite( (NGfx::EColorWriteMask)( NGfx::COLORWRITE_RED|NGfx::COLORWRITE_GREEN|NGfx::COLORWRITE_BLUE ) );
				while ( rs.nStep < lightState.semiPoints.size() )
				{
					const CLightState::SSemiPointLight &p = lightState.semiPoints[ rs.nStep++ ];
					if ( !RenderSemiPoint( &lmTarget, p.vColor, p.vCenter, p.vNormal, p.fRadius ) )
						break;
				}
				if ( rs.nStep >= lightState.semiPoints.size() )
					rs.nState = RC_APPLY;
			}
			break;
		case RC_APPLY:
			// store to lightmap
			{
				SLightmapTargetGeom lmTarget( pScene );
				lmTarget.rc.SetTextureRT( shadowMapsShare.GetLM() );
				if ( !rs.bCalcSky )
				{
					lmTarget.rc.SetColorWrite( (NGfx::EColorWriteMask)( NGfx::COLORWRITE_RED|NGfx::COLORWRITE_GREEN|NGfx::COLORWRITE_BLUE ) );
					RenderLight( &lmTarget, lmts, SLightInfo(), shadowMapsShare.GetLMTemp(), RO_LM_SKY_SQRT_MODULATE, 1.0f, DPM_NONE );
				}
				else
				{
					if ( nLights > 1 )
					{
						// blend with previous result
						int nDiv = Min( 16, nLights );
						float fNewBlend = 1.0f / nDiv;
						if ( rs.bCalcColor )
						{
							CVec4 vOldBlend( 0, 0, 0, 1 - fNewBlend );
							RenderLight( &lmTarget, lmts, SLightInfo(), 0, RO_LM_MODULATE, &vOldBlend, ABM_MUL|DPM_NONE );
							RenderLight( &lmTarget, lmts, SLightInfo(), shadowMapsShare.GetLMTemp(), RO_LM_SKY_SQRT_MODULATE, fNewBlend, ABM_ADD|DPM_NONE );
						}
						else
						{
							CVec4 vOldBlend( 1, 1, 1, 1 - fNewBlend );
							RenderLight( &lmTarget, lmts, SLightInfo(), 0, RO_LM_MODULATE, &vOldBlend, ABM_MUL|DPM_NONE );
							lmTarget.rc.SetColorWrite( NGfx::COLORWRITE_ALPHA );
							RenderLight( &lmTarget, lmts, SLightInfo(), shadowMapsShare.GetLMTemp(), RO_LM_SKY_SQRT_MODULATE, fNewBlend, ABM_ADD|DPM_NONE );

						}
					}
					else
					{
						// simple store
						if ( !rs.bCalcColor )
							lmTarget.rc.SetColorWrite( NGfx::COLORWRITE_ALPHA );
						RenderLight( &lmTarget, lmts, SLightInfo(), shadowMapsShare.GetLMTemp(), RO_LM_SKY_SQRT_MODULATE, 1.0f, DPM_NONE );
					}
					++nLights;
				}
				rs.nState = RC_START; // circle on the sand round and round ( (C)Belinda Carl..)
				rs.bCalcSky = true;
				rs.bCalcColor = !rs.bCalcColor; // calc color every second attempt
			}
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::RefreshCache()
{
	pLMTextureCache->RefreshTexture();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::CatchUp( IRender *_pRender, CTransformStack *pTS, CSceneFragments *pScene, CSceneFragments *pNewLMScene, bool bHasNewLightmaps )
{
	pRender = _pRender;
	SSelectFragments filterLightmapped( pScene, SLightmappedFilter() );
	bool bRecalcAllDepth = false;
	// on first update select sky directions
	if ( depthInfos.empty() )
	{
		depthInfos.resize( N_DEPTH_CHANNELS_PER_TEX );
		skyDirs.resize( N_DEPTH_CHANNELS_PER_TEX );
		for ( int i = 0; i < N_DEPTH_CHANNELS_PER_TEX; ++i )
			ChooseNewSkyDirection( i );
		bRecalcAllDepth = true;
	}
	// calc new scene bound & check if need to recalc depths
	SBound bNew;
	MakeSceneGeometryBound( &bNew, *pTS, F_MAX_SCENE_HEIGHT );
	if ( !IsInside( currentBound, bNew ) )
	{
		currentBound.SphereInit( bNew.s.ptCenter, bNew.s.fRadius + 10 );
		bRecalcAllDepth = true;
	}
	// recalc directional depths
	if ( bRecalcAllDepth )
		RecalcDepthChannel( N_ALL_DEPTH_CHANNELS );
	// calc lightmap for new stuff
	if ( !pNewLMScene->GetSelected().empty() )
	{
		bHasNewLightmaps = true;
		CTransformStack lmts;
		MakeLMTS( &lmts, CVec4( 0, 0, 0.001f, 0.5f ) );
		SLightmapTargetGeom lmTarget( pNewLMScene );
		lmTarget.rc.SetTextureRT( shadowMapsShare.GetLMTemp() );
		// clear
		RenderLight( &lmTarget, lmts, SLightInfo(), shadowMapsShare.GetLMTemp(), RO_LM_MODULATE, &VNULL4, DPM_NONE );
		// sky map
		lmTarget.rc.SetColorWrite( NGfx::COLORWRITE_ALPHA );
		RenderSkyCheck( &lmTarget, lmts, F_SKY_SINGLE_STRENGTH_MUL / N_DEPTH_CHANNELS_PER_TEX );
		// color map
		lmTarget.rc.SetColorWrite( (NGfx::EColorWriteMask)( NGfx::COLORWRITE_RED|NGfx::COLORWRITE_GREEN|NGfx::COLORWRITE_BLUE ) );
		for ( int k = 0; k < lightState.points.size(); ++k )
		{
			const CLightState::SPointLight &p = lightState.points[ k ];
			RenderPointNoShadows( &lmTarget, p.vColor, p.vCenter, CVec3(0,0,0), p.fRadius );
		}
		for ( int k = 0; k < lightState.semiPoints.size(); ++k )
		{
			const CLightState::SSemiPointLight &p = lightState.semiPoints[ k ];
			RenderPointNoShadows( &lmTarget, p.vColor, p.vCenter, p.vNormal, p.fRadius );
		}
		// copy to origin
		lmTarget.rc.SetTextureRT( shadowMapsShare.GetLM() );
		lmTarget.rc.SetColorWrite( NGfx::COLORWRITE_ALL );
		RenderLight( &lmTarget, lmts, SLightInfo(), shadowMapsShare.GetLMTemp(), RO_LM_SKY_SQRT_MODULATE, 1.0f, DPM_NONE );
	}
	if ( bHasNewLightmaps )
	{
		rs.nState = RC_START;
		rs.bCalcSky = rs.bCalcColor = true;
		nLights = 1;
	}
	if ( !pScene->GetSelected().empty() && !bHasNewLightmaps && 1 ) // if not stress mode 
		RecalcStep( pScene );

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::SetNewIllumination( const SGlobalIlluminationInfo &gl )
{
	globalIllumination = gl;
	rs.nState = RC_START;
	rs.bCalcColor = true;
	rs.bCalcSky = false;
	lightState.Clear();
	SLightStateCalcSeed seed( ambientLightSeed );
	lightState.CreateSimple( &seed, globalIllumination );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CLightmapTracker::ResetToChecker()
{
	LoadShit( pLMTextureCache->GetTexture() );
	rs.nState = RC_START;
	rs.bCalcColor = true;
	rs.bCalcSky = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//ASSERT( N_DEFAULT_RT_RESOLUTION == N_LM_SIZE );
//char szBuf[1024];
//sprintf( szBuf, "calcing lightmap for %d nodes\n", lightmaps.size() );
//OutputDebugString( szBuf );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x02592130, CLightmapTracker )

