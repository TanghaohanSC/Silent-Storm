#ifndef __GMATERIAL_H_
#define __GMATERIAL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGfx
{
	class CTexture;
	class CCubeTexture;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
enum EMaterialAlpha
{
	MA_OPAQUE,
	MA_ALPHA_TEST,
	MA_TRANSPARENT,
	MA_ADD_TRANSPARENT,
	MA_OVERLAY
};
/*struct SMaterialLayer
{
	enum EType
	{
		DIFFUSE,
		SPECULAR
	};
	CVec3 ptColor;
	float fSpecPower;
	CObj<CPtrFuncBase<NGfx::CTexture> > pTexture;
	CObj<CPtrFuncBase<NGfx::CTexture> > pBump;
	//EMaterialAddrMode addrMode;
};*/
class IMaterial;
IMaterial* CreateMaterial( 
	const CVec3 &color,
	CPtrFuncBase<NGfx::CTexture> *pTexture = 0, CPtrFuncBase<NGfx::CTexture> *pBump = 0,
	float fSpecPower = 0, const CVec3 &glossColor = CVec3(0,0,0),
	CPtrFuncBase<NGfx::CTexture> *pGlossTexture = 0,
	CPtrFuncBase<NGfx::CTexture> *pMirrorTexture = 0,
	CPtrFuncBase<NGfx::CCubeTexture> *pSky = 0,
	float fMetalMirror = 0, float fDielMirror = 0,
	EMaterialAlpha alphaMode = MA_OPAQUE, bool bDoesCastShadow = true );
IMaterial* CreateOccluderMaterial();
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif