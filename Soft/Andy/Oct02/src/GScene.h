#ifndef __GSCENE_H_
#define __GSCENE_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
#include "GGeometry.h"
#include "DiscretePos.h"
#include "GRenderModes.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTransformStack;
class CRectLayout;
class CFontFormatInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGfx
{
	class CTexture;
	class CGeometry;
	class CRenderContext;
}
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRenderPart;
class CPolyline;
class CRects;
class ILight;
class IMaterial;
class CParticles;
class CParticleEffect;
struct SRenderStats;
class ITextureLoader;
class CLightGroup;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGroupSelect
{
	unsigned short nMaskAny, nMaskEvery;

	SGroupSelect( unsigned short _nMaskAny, unsigned short _nMaskEvery ): nMaskAny(_nMaskAny), nMaskEvery(_nMaskEvery) {}
};
inline bool operator!=( const SGroupSelect &a, const SGroupSelect &b ) { return a.nMaskAny != b.nMaskAny || a.nMaskEvery != b.nMaskEvery; }
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGroupInfo
{
	unsigned short nLightGroup, nObjectGroup;
	//
	SGroupInfo(): nLightGroup(0), nObjectGroup(0xffff) {}
	SGroupInfo( int _nLG, int _nOG ): nLightGroup(_nLG), nObjectGroup(_nOG) {}
	bool operator==( const SGroupInfo &a ) const { return a.nLightGroup == nLightGroup && a.nObjectGroup == nObjectGroup; }
	bool IsMaskMatch( const SGroupSelect &m ) const { return ( nObjectGroup & m.nMaskAny ) != 0 && ( nObjectGroup & m.nMaskEvery ) == m.nMaskEvery; }
};
int GetGroup( CLightGroup *p );
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAnimLight: public CObjectBase
{
	OBJECT_BASIC_METHODS(CAnimLight);
public:
	CVec3 position;
	CVec3 color;
	float fRadius;
	bool bActive;
	bool bEnd;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IGScene: virtual public CObjectBase
{
public:
	virtual CLightGroup* CreateLightGroup() = 0;
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		const SGroupInfo &_ginfo ) = 0;
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		const SFBTransform &trans, const SGroupInfo &_ginfo ) = 0;
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		const SDiscretePos &trans, const SGroupInfo &_ginfo ) = 0;
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		CFuncBase<SFBTransform> *pPlacement, const SGroupInfo &_ginfo ) = 0;
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		CFuncBase<vector<SHMatrix> > *pPlacement, const SGroupInfo &_ginfo ) = 0;
	virtual CObjectBase* CreateDynamicGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		CFuncBase<SBound> *pBound, const SGroupInfo &_ginfo ) = 0;
	virtual CObjectBase* CreateParticles( CPtrFuncBase<CParticleEffect> *pInfo,
		CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SGroupInfo &_ginfo, bool bIsLit = false ) = 0;
	virtual CObjectBase* CreateLitParticles( CPtrFuncBase<CParticleEffect> *pInfo, 
		IMaterial *pMat, CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SGroupInfo &_ginfo ) = 0;
	virtual CPolyline* CreatePolyline( CPtrFuncBase<NGfx::CGeometry> *pGeometry, const CVec3 &color ) = 0;
	virtual CObjectBase* CreateSelection( CObjectBase *pRenderNode, const CVec4 &vColor ) = 0;
	virtual void SetAmbient( const CVec3 &ambientColor ) = 0;
	virtual ILight* AddDirectionalLight( CFuncBase<CVec3> *pColor, CFuncBase<CVec3> *pGlossColor, const CVec3 &vShadowColor, 
		const CVec3 &ptLight, const CVec3 &ptOrigin, 
		const CVec2 &ptSize, float fMaxHeight, bool bLightmapOnly ) = 0; 
	virtual ILight* AddPointLight( const CVec3 &_vColor, const CVec3 &ptOrigin, float fR, bool bLightmapOnly ) = 0;
	virtual ILight* AddPointLight( CPtrFuncBase<CAnimLight> *pLight ) = 0;
	virtual ILight* AddSpotLight( CFuncBase<CVec3> *pColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, 
		float fRadius, CPtrFuncBase<NGfx::CTexture> *pMask, bool bLightmapOnly ) = 0;
	virtual void Draw( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, const SGroupSelect &mask, EFogMode fogMode, const SFogParams &fog, EHSRMode hsrMode ) = 0;
	virtual ESceneRenderMode GetRenderMode() = 0;
	virtual void SetRenderMode( ESceneRenderMode mode ) = 0;
	virtual CFuncBase<CVec3>* GetCamera() = 0;
	virtual CVec2 GetScreenRect() = 0;
	virtual bool TraceScene( const CRay &r, float *pfT, CVec3 *pNormal, CVec3 *pColor ) = 0;
	virtual void SetLightmaps( bool bEnable ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IGScene* CreateScene();
// wrappers to Gfx to exclude interface on Gfx dependency
bool Is3DActive();
void SetWireframe( bool bWire );
void MakeScreenShot();
void Clear( NGfx::CRenderContext *pRC, const CVec3 &vColor );
void ClearScreen( const CVec3 &vColor );
void CopyRegisterOnScreen( const CTRect<float> &rScreenRect, bool bAlphaTest );
void GetRenderStats( SRenderStats *pStats );
float GetFrameTime();
void Flip();
int CalcTouchedTextureSize();
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif




















