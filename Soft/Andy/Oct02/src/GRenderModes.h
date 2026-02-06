#ifndef __GRenderModes_H_
#define __GRenderModes_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace NGfx
{
	class CTexture;
}
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ESceneRenderMode
{
	RM_TNL,
	RM_FASTEST,
	RM_SHOWOCCLUDERS,
	RM_SHOWLIGHTMAP,
	RM_SHOWSKYMAP,
	RM_LIGHTMAPPED,
	RM_BEST_GF2,
	RM_BEST_GF3,
	RM_LAST
};
inline bool IsUsingRegister( ESceneRenderMode rm ) { return rm > RM_LIGHTMAPPED; }
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EFogMode
{
	FOG_NONE,
	FOG_PERVERTEX,
	FOG_DYNAMIC,
	FOG_LAST
};
enum EHSRMode
{
	HSR_NONE,
	HSR_FAST,
	HSR_LAST
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFogParams
{
	float fDist;
	CVec3 vFogColor;
	float fHeight, fDensity;
	CVec3 vWaterColor;
	float fCameraHeight;
	float fVapourNoiseParam, fVapourSpeed, fVapourSwitchTime;
	float fTime;
	float fDistStart;
	
	void SetDefaults()
	{
		memset( this, 0, sizeof(SFogParams) );
		fDist = 10000; fDistStart = 100;
	}
	bool IsSameParams( const SFogParams &a ) const 
	{ 
		return 
			fDist == a.fDist && 
			vFogColor == a.vFogColor && 
			fHeight == a.fHeight && 
			fDensity == a.fDensity && 
			vWaterColor == a.vWaterColor &&
			fabs( fCameraHeight - a.fCameraHeight ) < 0.1f &&
			fDistStart == a.fDistStart;
	}
	bool operator==( const SFogParams &a ) const { return memcmp( &a, this, sizeof(SFogParams) ) == 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
