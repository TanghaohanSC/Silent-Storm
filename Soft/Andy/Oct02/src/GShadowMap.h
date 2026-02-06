#ifndef __GShadowMap_H_
#define __GShadowMap_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace NGfx
{
	class CTexture;
	class CRenderContext;
}
class CTransformStack;
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_DEFAULT_RT_RESOLUTION = 512;
class CShadowMapsShare
{
	CObj<NGfx::CTexture> pDepthShadow, pParticleLM, pLMDepthBuffer, pLM, pLMTemp;
	int nDepthResolution;

	void Refresh();
public:
	CShadowMapsShare();
	NGfx::CTexture* GetDepthShadow() { Refresh(); return pDepthShadow; }
	NGfx::CTexture* GetParticleLM() { Refresh(); return pParticleLM; }
	NGfx::CTexture* GetLMDepthBuffer() { Refresh(); return pLMDepthBuffer; }
	NGfx::CTexture* GetLM() { Refresh(); return pLM; }
	NGfx::CTexture* GetLMTemp() { Refresh(); return pLMTemp; }
	int GetDepthResolution() const { return nDepthResolution; }
};
extern CShadowMapsShare shadowMapsShare;
void DrawBorder( NGfx::CRenderContext *pRC, int nSize );
void SetAndClearRT( NGfx::CRenderContext *pRC, NGfx::CTexture *pTex, int nWriteMask, int nSize );
void MakeShadowMatrix( CTransformStack *pRes, const CTransformStack &ts, const CVec3 &ptDir, float fMaxHeight );
void MakeSceneGeometryBound( SBound *pRes, const CTransformStack &ts, float fMaxHeight );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif