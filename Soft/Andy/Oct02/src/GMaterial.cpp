#include "StdAfx.h"
#include "GMaterial.h"
#include "GfxEffects.h"
#include "GfxRender.h"
#include "GRenderCore.h"
#include "GRenderFactor.h"
#include "GSceneUtils.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CShadowCastMaterial: public IMaterial
{
protected:
	ZDATA
	bool bDoesCastShadow;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bDoesCastShadow); return 0; }
	CShadowCastMaterial() {}
	CShadowCastMaterial( bool _bDoesCastShadow ): bDoesCastShadow(_bDoesCastShadow) {}
	bool DoesCastShadow() const { return bDoesCastShadow; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGenericMaterial: public CShadowCastMaterial
{
	OBJECT_BASIC_METHODS( CGenericMaterial );
	CVec4 vMirrorParam;
	ZDATA_(CShadowCastMaterial)
	SMaterialInfo info;
	CDGPtr<CFuncBase<CVec3> > pDiffuseColor, pSpecularColor;
	CDGPtr<CPtrFuncBase<NGfx::CTexture> > pDiffuseTex, pSpecularTex;
	CDGPtr<CPtrFuncBase<NGfx::CTexture> > pBump;
	// reflection props
	CDGPtr<CPtrFuncBase<NGfx::CCubeTexture> > pSky;
	CDGPtr<CPtrFuncBase<NGfx::CTexture> > pMirror;
	float fMetalMirror, fDielMirror;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CShadowCastMaterial*)this); f.Add(2,&info); f.Add(3,&pDiffuseColor); f.Add(4,& pSpecularColor); f.Add(5,&pSpecularColor); f.Add(6,&pDiffuseTex); f.Add(7,& pSpecularTex); f.Add(8,&pSpecularTex); f.Add(9,&pBump); f.Add(10,&pSky); f.Add(11,&pMirror); f.Add(12,&fMetalMirror); f.Add(13,& fDielMirror); f.Add(14,&fDielMirror); return 0; }
protected:
	void AddOperations( SOpGenContext *p, ESceneRenderMode rm );
	void AddATOperations( SOpGenContext *p );
	const SMaterialInfo& GetMaterialInfo();
	CVec3 GetAverageColor() const { return info.diffuse.color; }
public:
	CGenericMaterial() {}
	CGenericMaterial( bool _bDoesCastShadow )
		: CShadowCastMaterial(_bDoesCastShadow), fMetalMirror(0), fDielMirror(0) {}
	virtual bool IsDecal() const { return info.mt == SMaterialInfo::DECAL; }
	void SetDiffuseColor( CFuncBase<CVec3> *_p ) { info.diffuse.type = SMaterialInfo::T_COLOR; pDiffuseColor = _p; }
	void SetDiffuseColor( const CVec3 &_vColor ) { info.diffuse.color = _vColor; }
	void SetSpecularColor( CFuncBase<CVec3> *_p, float _fPower ) { info.specular.type = SMaterialInfo::T_COLOR; pSpecularColor = _p; info.fSpecPower =_fPower; }
	void SetDiffuseTex( CPtrFuncBase<NGfx::CTexture> *_p ) { info.diffuse.type = SMaterialInfo::T_TEXTURE; pDiffuseTex = _p; }
	void SetSpecularTex( CPtrFuncBase<NGfx::CTexture> *_p, float _fPower ) { info.specular.type = SMaterialInfo::T_TEXTURE; pSpecularTex = _p; info.fSpecPower =_fPower; }
	void SetBump( CPtrFuncBase<NGfx::CTexture> *_p ) { pBump = _p; }
	void SetDecal() { info.mt = SMaterialInfo::DECAL; bDoesCastShadow = false; }
	void SetAlphaTest( bool _b ) { info.bAlphaTest = _b; }
	void SetReflectionInfo( CPtrFuncBase<NGfx::CCubeTexture> *_pSky, CPtrFuncBase<NGfx::CTexture> *_pMirror,
		float _fDielMirror, float _fMetalMirror );
	void Check()
	{
		ASSERT( info.diffuse.type != SMaterialInfo::T_NONE );
		ASSERT( !info.bAlphaTest || info.diffuse.type != SMaterialInfo::T_COLOR );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGenericMaterial::SetReflectionInfo( CPtrFuncBase<NGfx::CCubeTexture> *_pSky, CPtrFuncBase<NGfx::CTexture> *_pMirrorTex,
	float _fDielMirror, float _fMetalMirror )
{
	fDielMirror = _fDielMirror;
	fMetalMirror = _fMetalMirror;
	if ( _pSky && ( fDielMirror > 0 || fMetalMirror > 0 ) )
	{
		pSky = _pSky;
		pMirror = _pMirrorTex;
	}
	else
		pSky = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGenericMaterial::AddOperations( SOpGenContext *p, ESceneRenderMode rm )
{
	if ( !pSky )
		return;
	pSky.Refresh();
	if ( pSky->GetValue() == 0 )
		return;
	vMirrorParam = CVec4( 0, 0, fDielMirror, fMetalMirror );
	pSky.Refresh();
	if ( NGfx::GetHardwareLevel() >= NGfx::HL_GFORCE3 && rm > RM_BEST_GF2 )
	{
		NGfx::CTexture *pRefBump = 0;
		if ( pBump )
		{
			pBump.Refresh();
			pRefBump = pBump->GetValue();
		}
		else
		{
			pRefBump = GetUniformBump();
		}
		if ( fMetalMirror == 1 && fDielMirror == 0 )
		{
			if ( pMirror )
			{
				pMirror.Refresh();
				p->AddOperation( RO_BUMPED_MIRROR, 40, DPM_EQUAL, 1, 
					pSky->GetValue(), pRefBump );
				p->AddOperation( RO_REG_MUL_TEXTURE, 45, ABM_ALPHA_BLEND|DPM_EQUAL, 0, 
					pMirror->GetValue() );
			}
			else
				p->AddOperation( RO_BUMPED_MIRROR, 45, ABM_ALPHA_BLEND|DPM_EQUAL, 0, 
					pSky->GetValue(), pRefBump );
		}
		else
		{
			if ( pMirror )
			{
				pMirror.Refresh();
				p->AddOperation( RO_BUMPED_MIRROR, 40, DPM_EQUAL, 1, 
					pSky->GetValue(), pRefBump );
				p->AddOperation( RO_TEXTURE, 41, ABM_MUL|DPM_EQUAL, 1, 
					pMirror->GetValue() );
				p->AddOperation( RO_BUMPED_FRESNEL, 45, ABM_ALPHA_BLEND|DPM_EQUAL, 0, 
					pRefBump, &vMirrorParam );
			}
			else
			{
				p->AddOperation( RO_BUMPED_MIRROR, 40, DPM_EQUAL, 1, 
					pSky->GetValue(), pRefBump );
				p->AddOperation( RO_BUMPED_FRESNEL, 45, ABM_ALPHA_BLEND|DPM_EQUAL, 0, 
					pRefBump, &vMirrorParam );
			}
		}
	}
	else
	{
		if ( pMirror )
		{
			pMirror.Refresh();
			p->AddOperation( RO_GLOSSED_MIRROR, 45, ABM_ALPHA_BLEND|DPM_EQUAL, 0, 
				pSky->GetValue(), pMirror->GetValue(), &vMirrorParam );
		}
		else
			p->AddOperation( RO_MIRROR, 45, ABM_ALPHA_BLEND|DPM_EQUAL, 0, pSky->GetValue(), &vMirrorParam );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGenericMaterial::AddATOperations( SOpGenContext *p )
{
	if ( info.bAlphaTest )
	{
		ASSERT( pDiffuseTex );
		pDiffuseTex.Refresh();
		p->AddOperation( RO_TEXTURE_AT, 0, 0, 0, pDiffuseTex->GetValue() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void Refresh( SMaterialInfo::SColorInfo *p, CDGPtr<CFuncBase<CVec3> > *pColor, 
	CDGPtr<CPtrFuncBase<NGfx::CTexture> > *pTex )
{
	switch ( p->type )
	{
		case SMaterialInfo::T_NONE: break;
		case SMaterialInfo::T_COLOR: pColor->Refresh(); p->color = (*pColor)->GetValue(); break;
		case SMaterialInfo::T_TEXTURE: pTex->Refresh(); p->pTex = (*pTex)->GetValue(); break;
		case SMaterialInfo::T_VSPEC: pTex->Refresh(); p->pTex = (*pTex)->GetValue(); break;
		default: ASSERT(0); break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SMaterialInfo& CGenericMaterial::GetMaterialInfo()
{
	if ( pBump )
	{
		pBump.Refresh();
		info.pBump = pBump->GetValue();
	}
	Refresh( &info.diffuse, &pDiffuseColor, &pDiffuseTex );
	Refresh( &info.specular, &pSpecularColor, &pSpecularTex );
	return info;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTransparentMaterial
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RenderTransparentElement( NGfx::CRenderContext *pRC, SRenderGeometryInfo *pGeometry, 
	NGfx::CTexture *pTex, NGfx::CTexture *pFog, NGfx::CCubeTexture *pSky, 
	float fMetalMirror, float fDielMirror, bool bAdditive )
{
	pGeometry->pVertices.Refresh();
	pGeometry->pTriLists[TLT_GEOM].Refresh();
	const vector<NGfx::STriangleList> &tris = pGeometry->pTriLists[TLT_GEOM]->GetValue();
	if ( tris.empty() )
		return;
	if ( bAdditive )
		pRC->SetAlphaCombine( NGfx::COMBINE_ALPHA_ADD );
	else
		pRC->SetAlphaCombine( NGfx::COMBINE_ALPHA );
	NGfx::SEffect *pEff;
	NGfx::SEffFoggedParticle efFog;
	NGfx::SEffTexture efTexture;
	NGfx::SEffFoggedTextureAndMirror efFoggedSimple;
	NGfx::SEffTextureAndMirror efSimple;
	NGfx::SEffTnLTexture efTnLTexture;
	if ( NGfx::IsTnLDevice() )
	{
		efTnLTexture.pTex = pTex;
		pEff = &efTnLTexture;
	}
	else
	{
		if ( pSky )
		{
			if ( pFog && NGfx::GetHardwareLevel() >= NGfx::HL_GFORCE3 )
			{
				efFoggedSimple.pTex = pTex;
				efFoggedSimple.pTexMirror = pSky;
				efFoggedSimple.pFog = pFog;
				efFoggedSimple.fMetalMirror = fMetalMirror;
				efFoggedSimple.fDielMirror = fDielMirror;
				pEff = &efFoggedSimple;
			}
			else
			{
				efSimple.pTex = pTex;
				efSimple.pTexMirror = pSky;
				efSimple.fMetalMirror = fMetalMirror;
				efSimple.fDielMirror = fDielMirror;
				pEff = &efSimple;
			}
		}
		else
		{
			if ( pFog )
			{
				efFog.pFog = pFog;
				efFog.pTex = pTex;
				pEff = &efFog;
			}
			else
			{
				efTexture.pTex = pTex;
				pEff = &efTexture;
			}
		}
	}
	pRC->SetEffect( pEff );
	pRC->AddPrimitive( pGeometry->pVertices->GetValue(), &tris[0], tris.size(), 0xffffffff );
	pRC->Flush();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTransparentMaterial: public IMaterial
{
	OBJECT_BASIC_METHODS( CTransparentMaterial );
	ZDATA
	CDGPtr< CPtrFuncBase<NGfx::CTexture> > pTexture;
	bool bAdditive;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pTexture); f.Add(3,&bAdditive); return 0; }
public:
	CTransparentMaterial() {}
	CTransparentMaterial( CPtrFuncBase<NGfx::CTexture> *_pTex, bool _bAdditive )
		: pTexture(_pTex), bAdditive(_bAdditive) {}
	virtual EMaterialType GetType() const { return MT_TRANSPARENT; }
	bool DoesCastShadow() const { return false; }
	void ExecTransparent( SRenderGeometryInfo *pElement, NGfx::CRenderContext *pRC, NGfx::CTexture *pFog );
};
void CTransparentMaterial::ExecTransparent( SRenderGeometryInfo *pElement, NGfx::CRenderContext *pRC, NGfx::CTexture *pFog )
{
	if ( !IsValid( pTexture ) )
		return;
	pTexture.Refresh();
	RenderTransparentElement( pRC, pElement, pTexture->GetValue(), pFog, 0, 0, 0, bAdditive );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMirroredTransparentMaterial
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMirroredTransparentMaterial: public IMaterial
{
	OBJECT_BASIC_METHODS(CMirroredTransparentMaterial);
	ZDATA
	CDGPtr<CPtrFuncBase<NGfx::CTexture> > pTexture;
	CDGPtr<CPtrFuncBase<NGfx::CCubeTexture> > pMirrorTexture;
	float fMetalMirror, fDielMirror;
	bool bAdditive;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pTexture); f.Add(3,&pMirrorTexture); f.Add(4,&fMetalMirror); f.Add(5,& fDielMirror); f.Add(6,&fDielMirror); f.Add(7,&bAdditive); return 0; }
public:
	CMirroredTransparentMaterial() {}
	CMirroredTransparentMaterial( CPtrFuncBase<NGfx::CTexture> *_pTex, CPtrFuncBase<NGfx::CCubeTexture> *_pMirror,
		float _fMetalMirror, float _fDielMirror, bool _bAdditive )
		: pTexture(_pTex), fMetalMirror(_fMetalMirror), fDielMirror(_fDielMirror), 
		pMirrorTexture(_pMirror), bAdditive(_bAdditive) {}
	virtual EMaterialType GetType() const { return MT_TRANSPARENT; }
	bool DoesCastShadow() const { return false; }
	void ExecTransparent( SRenderGeometryInfo *pElement, NGfx::CRenderContext *pRC, NGfx::CTexture *pFog );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMirroredTransparentMaterial::ExecTransparent( SRenderGeometryInfo *pElement, NGfx::CRenderContext *pRC, NGfx::CTexture *pFog )
{
	pTexture.Refresh();
	pMirrorTexture.Refresh();
	RenderTransparentElement( pRC, pElement, pTexture->GetValue(), pFog, 
		pMirrorTexture->GetValue(), fMetalMirror, fDielMirror, bAdditive );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class COccluderMaterial : public IMaterial
{
	OBJECT_NOCOPY_METHODS(COccluderMaterial);
public:
	virtual bool DoesCastShadow() const { return false; }
	virtual EMaterialType GetType() const { return MT_OCCLUDER; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
IMaterial* CreateOccluderMaterial()
{
	return new COccluderMaterial;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static IMaterial* CreateTransparentMaterial( CPtrFuncBase<NGfx::CTexture> *pTexture, 
	CPtrFuncBase<NGfx::CCubeTexture> *pSky, float fMetalMirror, float fDielMirror, 
	bool bAdditive )
{
	if ( pSky && ( fMetalMirror != 0 || fDielMirror != 0 ) )
		return new CMirroredTransparentMaterial( pTexture, pSky, fMetalMirror, fDielMirror, bAdditive );
	return new CTransparentMaterial( pTexture, bAdditive );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IMaterial* CreateMaterial( 
	const CVec3 &color,
	CPtrFuncBase<NGfx::CTexture> *pTexture, CPtrFuncBase<NGfx::CTexture> *pBump,
	float fSpecPower, const CVec3 &vGlossVolor,
	CPtrFuncBase<NGfx::CTexture> *pGlossTexture,
	CPtrFuncBase<NGfx::CTexture> *pMirrorTexture,
	CPtrFuncBase<NGfx::CCubeTexture> *pSky,
	float fMetalMirror, float fDielMirror,
	EMaterialAlpha alphaMode, bool bDoesCastShadow )
{
	CObj<CObjectBase> pHold1(pBump), pHold2(pGlossTexture), pHold3(pMirrorTexture);
	if ( alphaMode == MA_TRANSPARENT )
		return CreateTransparentMaterial( pTexture, pSky, fMetalMirror, fDielMirror, false );
	if ( alphaMode == MA_ADD_TRANSPARENT )
		return CreateTransparentMaterial( pTexture, pSky, fMetalMirror, fDielMirror, true );
	CGenericMaterial *pRes = new CGenericMaterial( bDoesCastShadow );
	pRes->SetDiffuseColor( color );
	//const_cast<CVec3&>(vGlossVolor) = CVec3(1,1,1);
	//fSpecPower = 1;
	if ( pTexture == 0 )
	{
		pRes->SetDiffuseColor( new CCVec3( color ) );
		pRes->Check();
		pRes->SetReflectionInfo( pSky, pMirrorTexture, fDielMirror, fMetalMirror );
		return pRes;
	}
	if ( alphaMode == MA_OVERLAY )
		pRes->SetDecal();
	pRes->SetDiffuseTex( pTexture );
	pRes->SetAlphaTest( alphaMode == MA_ALPHA_TEST );
	pRes->SetBump( pBump );

	if ( fabs2( vGlossVolor ) > 0 )
		pRes->SetSpecularColor( new CCVec3( vGlossVolor ), fSpecPower );
	if ( pGlossTexture )
		pRes->SetSpecularTex( pGlossTexture, fSpecPower );
	// fill reflection info
	//fDielMirror = 1;
	//fMetalMirror = 1;
	pRes->SetReflectionInfo( pSky, pMirrorTexture, fDielMirror, fMetalMirror );
	pRes->Check();
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
BASIC_REGISTER_CLASS( IMaterial )
REGISTER_SAVELOAD_CLASS( 0x02191160, CTransparentMaterial )
REGISTER_SAVELOAD_CLASS( 0x02191161, CGenericMaterial )
//REGISTER_SAVELOAD_CLASS( 0x00842120, CTreeMaterial )
REGISTER_SAVELOAD_CLASS( 0x01812170, CMirroredTransparentMaterial )
REGISTER_SAVELOAD_CLASS( 0x020A2170, COccluderMaterial )
