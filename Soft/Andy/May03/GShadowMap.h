#ifndef __GShadowMap_H_
#define __GShadowMap_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GInit.h"
namespace NGfx
{
	class CTexture;
	class CCubeTexture;
	class CRenderContext;
}
class CTransformStack;
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_DEFAULT_RT_RESOLUTION = 512;
const int N_CUBE_TEXTURE_CHANNELS = 100;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCubeTextureChannel: public CObjectBase
{
	OBJECT_BASIC_METHODS( CCubeTextureChannel );
public:
	enum EChannel
	{
		RED = 1,
		GREEN = 2,
		BLUE = 3,
		ALPHA = 0
	};
private:
	CObj<NGfx::CCubeTexture> p;
	EChannel channel;
	int nLRU;
public:
	CCubeTextureChannel() {}
	CCubeTextureChannel( NGfx::CCubeTexture *_p, EChannel _c ): p(_p), channel(_c), nLRU(0) {}
	void Touch();
	bool IsAlpha() const { return channel == ALPHA; }
	NGfx::CCubeTexture* GetTexture() { Touch(); return p; }
	EChannel GetChannel() const { return channel; }
	int GetLRU() const { return nLRU; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CShadowMapsShare
{
	CObj<NGfx::CTexture> pDepthShadow, pParticleLM, pLMDepthBuffers[ N_MAX_SKY_TEXTURES ];
	CObj<NGfx::CCubeTexture> pCubeDepth;
	int nDepthResolution, nCLSkyTextures;

	void Refresh();
public:
	CShadowMapsShare();
	NGfx::CTexture* GetDepthShadow() { Refresh(); return pDepthShadow; }
	NGfx::CTexture* GetParticleLM() { Refresh(); return pParticleLM; }
	NGfx::CTexture* GetLMDepthBuffer( int nBuffer ) 
	{ 
		ASSERT( nBuffer >= 0 && nBuffer < ARRAY_SIZE(pLMDepthBuffers) && nBuffer < nCLSkyTextures ); 
		Refresh(); 
		return pLMDepthBuffers[ nBuffer ]; 
	}
	NGfx::CCubeTexture* GetCubeDepth() { Refresh(); return pCubeDepth; }
	int GetDepthResolution() { Refresh(); return nDepthResolution; }
	int GetCLSkyTexturesNumber() { Refresh(); return nCLSkyTextures; }
	int GetCubeDepthResolution() const { return 256; }
};
extern CShadowMapsShare shadowMapsShare;
void DrawBorder( NGfx::CRenderContext *pRC, int nXSize, int nYSize );
void SetAndClearRT( NGfx::CRenderContext *pRC, NGfx::CTexture *pTex, int nWriteMask, int nSize );
void ClearRT( NGfx::CRenderContext *pRC, int nWriteMask, DWORD dwColor = 0 );
void MakeShadowMatrix( CTransformStack *pRes, const CTransformStack &ts, const CVec3 &ptDir, float fMaxHeight );
void MakeSceneGeometryBound( SBound *pRes, const CTransformStack &ts, float fMaxHeight );
void UpdateCubeTextureShare();
bool IsCubeTextureShareThrashing();
CCubeTextureChannel* MakeNewCubeChannel();
CCubeTextureChannel* GetSharedCubeChannel();
int GetCubeTextureBufferNumber();
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif