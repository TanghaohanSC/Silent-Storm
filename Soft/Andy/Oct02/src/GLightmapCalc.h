#ifndef __GLightmapCalc_H_
#define __GLightmapCalc_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GLightmap.h"
#include "GfxRender.h"
#include "GRenderCore.h"

namespace NGfx
{
	class CTexture;
}
namespace NGScene
{
class CMaterial;
class CPerMaterialCombiner;
struct SLightInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGlobalIlluminationInfo
{
	struct SDirectional
	{
		CVec3 vColor, vDir;
		bool bIsRendered;

		SDirectional() {}
		SDirectional( const CVec3 &_vColor, const CVec3 &_vDir, bool _bIsRendered )
			: vColor(_vColor), vDir(_vDir), bIsRendered(_bIsRendered) {}
	};
	struct SPoint
	{
		CVec3 vColor, vCenter;
		float fRadius;
		bool bIsRendered;
		
		SPoint() {}
		SPoint( const CVec3 &_vColor, const CVec3 &_vCenter, float _fRadius, bool _bIsRendered )
			: vColor(_vColor), vCenter(_vCenter), fRadius(_fRadius), bIsRendered(_bIsRendered) {}
	};
	ZDATA
	SSphere globalBounds;
	CVec3 vAmbient;
	vector<SDirectional> parallel;
	vector<SPoint> points;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&globalBounds); f.Add(3,&vAmbient); f.Add(4,&parallel); f.Add(5,&points); return 0; }
	
	SGlobalIlluminationInfo(): vAmbient(0,0,0), globalBounds(CVec3(0,0,0), 10) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// для расчета динамических lightmap`ов
class IGScene;
struct SLightStateCalcSeed
{
	int nSeed;

	SLightStateCalcSeed() : nSeed(0) {}
};
class CLightState
{
	void AddRay( const CVec3 &vFrom, const CVec3 &vDir, const CVec3 &vColor );
	void AddParallel( bool bDoRender, const SSphere &_bound, const CVec3 &vDir, const CVec3 &vColor );
	void AddPoint( bool bDoRender, const CVec3 &vCenter, float fRadius, const CVec3 &_vColor );
	void TraceDynamicLMPointLight( SDynamicAmbientInfo *pRes, const CVec3 &vTarget, 
		const CVec3 &vCenter, float fRadius, const CVec3 &vColor, const CVec3 &vSemiNormal, IGScene *pVis ) const;
	CPtr<IGScene> pVis;
public:
	struct SParallelLight
	{
		CVec3 vColor, vDir;

		SParallelLight() {}
		SParallelLight( const CVec3 &_vColor, const CVec3 &_vDir )
			: vColor(_vColor), vDir(_vDir) {}
	};
	struct SSemiPointLight
	{
		// if vNormal is zero then its not semi point light, its full circle light
		CVec3 vColor, vCenter, vNormal;
		float fRadius;

		SSemiPointLight() {}
		SSemiPointLight( const CVec3 &_vColor, const CVec3 &_vCenter, const CVec3 &_vNormal, float _fRadius )
			: vColor(_vColor), vCenter(_vCenter), vNormal(_vNormal), fRadius(_fRadius) {}
	};
	struct SPointLight
	{
		CVec3 vColor, vCenter;
		float fRadius;
		
		SPointLight() {}
		SPointLight( const CVec3 _vColor, const CVec3 &_vCenter, float _fR )
			: vColor(_vColor), vCenter(_vCenter), fRadius(_fR) {}
	};
	ZDATA
	vector<SParallelLight> parallel;
	vector<SSemiPointLight> semiPoints;
	vector<SPointLight> points;
	vector<CVec3> skyDirections;
	CVec3 vAmbientColor;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&parallel); f.Add(3,&semiPoints); f.Add(4,&points); f.Add(5,&skyDirections); f.Add(6,&vAmbientColor); return 0; }

	void Clear() { parallel.clear(); semiPoints.clear(); points.clear(); skyDirections.clear(); }
	CVec3 GenerateSkyDir( SLightStateCalcSeed *pSeed );
	void CreateSimple( SLightStateCalcSeed *pSeed, const SGlobalIlluminationInfo &l );
	void CreateScattered( SLightStateCalcSeed *pSeed, const SGlobalIlluminationInfo &l, IGScene *pVis );
	void TraceDynamicLM( SDynamicAmbientInfo *pRes, const SBound &bv, IGScene *pVis ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// recalcs lightmaps, has list of all lightmap textures, information about lights etc
class CLightmapTracker : public CObjectBase
{
	OBJECT_NOCOPY_METHODS(CLightmapTracker);
	enum EState
	{
		RC_START,
		RC_SKY_DEPTH,
		RC_COLOR_POINT,
		RC_COLOR_SEMI,
		RC_APPLY
	};
	struct SRecalcState
	{
		EState nState;
		int nStep;
		bool bCalcSky;
		bool bCalcColor;
	};
	ZDATA
	SGlobalIlluminationInfo globalIllumination;
	SLightStateCalcSeed ambientLightSeed;
	CLightState lightState;
	CObj<CLightmapTextureCache> pLMTextureCache;
	vector<SDirectionalDepthInfo> depthInfos;
	vector<CVec3> skyDirs;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&globalIllumination); f.Add(3,&ambientLightSeed); f.Add(4,&lightState); f.Add(5,&pLMTextureCache); f.Add(6,&depthInfos); f.Add(7,&skyDirs); return 0; }
	int nLights;
	SRecalcState rs;
	IRender *pRender;
	SBound currentBound;
private:
	struct SLightmapTargetGeom
	{
		NGfx::CRenderContext rc;
		CSceneFragments *pGeom;

		SLightmapTargetGeom( CSceneFragments *_pGeom ) : pGeom(_pGeom) {}
	};
	//void ProcessLight( SLightmapTargetGeom &dst, NGfx::CTexture *pDepth, 
	//	const SLightInfo &lightInfo, const SDirectionalDepthInfo &depthInfo, const SBound &bTarget );
	//bool CheckTarget( SLightmapTargetGeom &dst, const SBound &bTarget );
	//void RenderParallel( IRender *pRender, SLightmapTargetGeom &dst, const CLightState::SParallelLight &p,
	//	const SSphere &_bound );


	void RenderLight( SLightmapTargetGeom *pTarget, const CTransformStack &ts, const SLightInfo &lightInfo, NGfx::CTexture *pDepth,
		ERenderOperation op, CRenderCmdList::UParameter param, int nStencilOp );
	void ProcessPerspCheck( SLightmapTargetGeom *pTarget, NGfx::CTexture *pDepth, const CVec3 &vCenter, float fRadius, 
		const SHMatrix &mWorldToLight, const CVec3 &vDir, const CVec4 &vDepth, const CVec4 &vChannel );
	void ProcessPerspLight( SLightmapTargetGeom *pTarget, const SLightInfo &lightInfo );
	void RenderSemiPointDepthCheck( SLightmapTargetGeom *pTarget, const CVec3 &ptFrom, float fRadius, const CVec3 &vNormal, 
		const SLightInfo &lightInfo, bool bUseSafeClear );
	bool RenderSemiPoint( SLightmapTargetGeom *pTarget, 
		const CVec3 &_vColor, const CVec3 &_vCenter, const CVec3 &_vNormal, float fRadius );
	bool RenderPointNoShadows( SLightmapTargetGeom *pTarget, 
		const CVec3 &_vColor, const CVec3 &_vCenter, const CVec3 &_vNormal, float fRadius );
	void RenderSkyCheck( SLightmapTargetGeom *pTarget, const CTransformStack &lmts, float fStrength );
	void ChooseNewSkyDirection( int nTarget );
	void RecalcStep( CSceneFragments *pScene );
	void RecalcDepthChannel( int nChannel );
public:
	CLightmapTracker();
	void RefreshCache();
	void CatchUp( IRender *_pRender, CTransformStack *pTS, CSceneFragments *pScene, CSceneFragments *pNewLMScene, bool bHasNewLightmaps );
	void SetNewIllumination( const SGlobalIlluminationInfo &gl );
	const CLightState& GetLightState() const { return lightState; }
	void ResetToChecker();
	CLMRegion* AllocRegion( IPart *pSrc, int nLOD );
	NGfx::CTexture* GetLMTexture();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif