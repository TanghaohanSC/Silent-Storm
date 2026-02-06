#include "StdAfx.h"
#include "GScene.h"
#include "GRenderLight.h"
#include "GSceneUtils.h"
#include "GfxEffects.h"
#include "GfxRender.h"
#include "GfxBuffers.h"
#include "GfxUtils.h"
#include "GRenderFactor.h"
#include "GRenderExecute.h"
#include "GShadowMap.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Misc\LogStream.h"

static bool bStencilShadows = false;
namespace NGScene
{
bool bStaticShadowDepthRendered;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDirectionalLight
////////////////////////////////////////////////////////////////////////////////////////////////////
CDirectionalLight::CDirectionalLight( CFuncBase<CVec3> *_pColor, CFuncBase<CVec3> *_pGlossColor,
	const CVec3 &_vShadowColor, const CVec3 &ptLight, 
	const CVec3 &ptOrigin, const CVec2 &ptSize, float _fMaxHeight,
	CVersioningBase *_pStaticTracker, CFuncBase<CVec3> *_pAmbient, bool _bLightmapOnly )
	: pStaticTracker(_pStaticTracker), pColor(_pColor), pGlossColor(_pGlossColor),
	vShadowColor(_vShadowColor), pAmbient(_pAmbient), vDepth( 0, 0, 1 / _fMaxHeight, 0 ), bLightmapOnly(_bLightmapOnly),
	fMaxHeight(_fMaxHeight)
{
	vLightDir = ptLight;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDirectionalLight::FinalPass( NGfx::CRenderContext *pRC, ESceneRenderMode renderMode )
{
	CTRect<float> rDest;
	NGfx::GetRegisterSize( &rDest );
	NGfx::CRenderContext rc( *pRC );
	rc.SetRegister( 0 );
	rc.SetAlphaCombine( NGfx::COMBINE_ADD );

	if ( renderMode == RM_BEST_GF3 )
	{
		rc.SetStencil( NGfx::STENCIL_NONE );
		NGfx::CopyTexture( rc, CVec2(rDest.Width(), rDest.Height() ), rDest, NGfx::GetRegisterTexture(1), rDest );
	}
	else
	{	
		rc.SetStencil( NGfx::STENCIL_TEST, 0x80, 0x80 );
		NGfx::CopyTexture( rc, CVec2(rDest.Width(), rDest.Height() ), rDest, NGfx::GetRegisterTexture(1), rDest );

		rc.SetStencil( NGfx::STENCIL_TEST_CLEAR, 0, 0x80 );
		NGfx::CopyTexture( rc, CVec2(rDest.Width(), rDest.Height() ), rDest, NGfx::GetRegisterTexture(1), rDest, CVec4( vShadowColor, 0 ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDirectionalLight::PrepareLightInfo( SLightInfo *pLightInfo )
{
	SLightInfo &lightInfo = *pLightInfo;
	pColor.Refresh();
	pGlossColor.Refresh();
	lightInfo.bNeedSet = true;
	lightInfo.vGlossColor = pGlossColor->GetValue();
	lightInfo.vLightColor = pColor->GetValue();
	lightInfo.vLightPos = CVec4(-vLightDir, 0);
	lightInfo.vShadowColor = vShadowColor;
	lightInfo.vAmbientColor = pAmbient->GetValue();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RenderTnL( SOpGenContext &op, const SMaterialInfo &info, const CVec3 &ambient,
	ESceneRenderMode renderMode )
{
	if ( info.diffuse.type == SMaterialInfo::T_COLOR )
		op.AddOperation( RO_TNL_DIR_AMB_LIT_DIFFUSE_SOLID, 10, 0, 0, &info.diffuse.color );
	else
	{
		ASSERT( info.diffuse.type == SMaterialInfo::T_TEXTURE );
		switch ( info.mt )
		{
		case SMaterialInfo::NORMAL:
			if ( info.bAlphaTest )
				op.AddOperation( RO_TNL_DIR_AMB_LIT_DIFFUSE_TEXTURE_AT, 10, 0, 0, info.diffuse.pTex );
			else
				op.AddOperation( RO_TNL_DIR_AMB_LIT_DIFFUSE_TEXTURE, 10, 0, 0, info.diffuse.pTex );
			break;
		case SMaterialInfo::DECAL:
			op.AddOperation( RO_TNL_DIR_AMB_LIT_DIFFUSE_TEXTURE_DECAL, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 0, info.diffuse.pTex );
			break;
		default:
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RenderFast( SOpGenContext &op, const SMaterialInfo &info, const CVec3 &ambient,
	ESceneRenderMode renderMode )
{
	if ( info.diffuse.type == SMaterialInfo::T_COLOR )
		op.AddOperation( RO_DIR_AMB_LIT_SOLID, 10, 0, 0, &info.diffuse.color, &ambient );
	else
	{
		ASSERT( info.diffuse.type == SMaterialInfo::T_TEXTURE );
		switch ( info.mt )
		{
			case SMaterialInfo::NORMAL:
				if ( info.bAlphaTest )
					op.AddOperation( RO_DIR_AMB_LIT_DIFFUSE_TEXTURE_AT, 10, 0, 0, info.diffuse.pTex, &ambient );
				else
					op.AddOperation( RO_DIR_AMB_LIT_DIFFUSE_TEXTURE, 10, 0, 0, info.diffuse.pTex, &ambient );
				break;
			case SMaterialInfo::DECAL:
				op.AddOperation( RO_DIR_AMB_LIT_DIFFUSE_TEXTURE_DECAL, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 0, info.diffuse.pTex, &ambient );
				break;
			default:
				break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RenderLight( SOpGenContext &op, const SMaterialInfo &info, const CVec3 &ambient,
	ESceneRenderMode renderMode )
{
	if ( renderMode == RM_BEST_GF2 )
	{
		if ( info.diffuse.type == SMaterialInfo::T_COLOR )
			op.AddOperation( RO_DIR_LIT_SOLID, 10, DPM_EQUAL, 1, &info.diffuse.color );
		else
		{
			ASSERT( info.diffuse.type == SMaterialInfo::T_TEXTURE );
			switch ( info.mt )
			{
			case SMaterialInfo::NORMAL:
				if ( info.pBump )
					op.AddOperation( RO_DIFFUSE_BUMP_DIR_LIT_TEXTURE, 10, ABM_SRC_AMUL|DPM_EQUAL, 1, info.diffuse.pTex, info.pBump );
				else
					op.AddOperation( RO_DIFFUSE_DIR_LIT_TEXTURE, 10, DPM_EQUAL, 1, info.diffuse.pTex );
				// add gloss ops
				if ( info.specular.type == SMaterialInfo::T_TEXTURE )
					op.AddOperation( RO_SPECULAR_DIR_TEXTURE, 11, ABM_ADD|DPM_EQUAL, 1, info.specular.pTex, info.fSpecPower );
				else if ( info.specular.type == SMaterialInfo::T_COLOR )
					op.AddOperation( RO_SPECULAR_DIR, 11, ABM_ADD|DPM_EQUAL, 1, &info.specular.color, info.fSpecPower );
				break;
			case SMaterialInfo::DECAL:
				op.AddOperation( RO_DIFFUSE_DIR_LIT_TEXTURE, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 1, info.diffuse.pTex );
				break;
			default:
				break;
			}
		}
	}
	else if ( renderMode == RM_BEST_GF3 )
	{
		if ( info.diffuse.type == SMaterialInfo::T_COLOR )
			op.AddOperation( RO_DIR_LIT_SOLID_PP, 10, DPM_EQUAL, 1, &info.diffuse.color );
		else
		{
			ASSERT( info.diffuse.type == SMaterialInfo::T_TEXTURE );
			switch ( info.mt )
			{
			case SMaterialInfo::NORMAL:
				{
					// add gloss ops
					bool bCalcNH = false;
					if ( info.specular.type == SMaterialInfo::T_TEXTURE )
					{
						bCalcNH = true;
						op.AddOperation( RO_PP_SPECULAR_TEXTURE_DIR, 7, DPM_EQUAL, 1, 2, info.specular.pTex );
					}
					else if ( info.specular.type == SMaterialInfo::T_COLOR )
					{
						bCalcNH = true;
						op.AddOperation( RO_PP_SPECULAR_COLOR_DIR, 7, DPM_EQUAL, 1, 2, &info.specular.color );
					}
					if ( bCalcNH )
					{
						if ( info.pBump )
							op.AddOperation( RO_NHCALC_BUMP, 3, DPM_EQUAL, 2, info.pBump );
						else
							op.AddOperation( RO_NHCALC, 3, DPM_EQUAL, 2, info.fSpecPower );
					}
					// diffuse
					int nAddFlag = bCalcNH ? ABM_ADD : 0;
					if ( info.pBump )
						op.AddOperation( RO_DIFFUSE_BUMP_DIR_LIT_TEXTURE_PP, 10, nAddFlag|DPM_EQUAL, 1, info.diffuse.pTex, info.pBump );
					else
						op.AddOperation( RO_DIFFUSE_DIR_LIT_TEXTURE_PP, 10, nAddFlag|DPM_EQUAL, 1, info.diffuse.pTex );
				}
				break;
			case SMaterialInfo::DECAL:
				if ( info.pBump )
					op.AddOperation( RO_DIFFUSE_BUMP_DIR_LIT_TEXTURE_PP, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 1, info.diffuse.pTex, info.pBump );
				else
					op.AddOperation( RO_DIFFUSE_DIR_LIT_TEXTURE_PP, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 1, info.diffuse.pTex );
				break;
			default:
				break;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RenderShadowTest( SOpGenContext &op, const SMaterialInfo &info, const SPerspDirectionalDepthInfo &depthInfo,
	ESceneRenderMode renderMode )
{
	if ( info.IsDecal() )
		return;
	if ( renderMode == RM_BEST_GF2 )
	{
		op.AddOperation( RO_DIR_SHADOW_TEST, 10, ABM_ZERO|STM_LIGHT|DPM_EQUAL, 0,//, STM_LIGHT|ABM_ADD|DPM_EQUAL, 0, 
			&depthInfo, shadowMapsShare.GetDepthShadow() );
	}
	else if ( renderMode == RM_BEST_GF3 )
	{
		op.AddOperation( RO_DIR_SHADOW_TEST_SMOOTHED, 10, info.bAlphaTest ? DPM_EQUAL : 0, 2,
			&depthInfo, shadowMapsShare.GetDepthShadow(), 0.0f );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static CVec3 vParticleLMShadowTestFullLight;
static bool bUseSoftLMShadows;
static void RenderParticleShadowTest( SOpGenContext &op, const SMaterialInfo &info, const SPerspDirectionalDepthInfo &depthInfo,
	ESceneRenderMode renderMode )
{
	if ( bUseSoftLMShadows )
		op.AddOperation( RO_DIR_PARTICLE_LM_SOFT_SHADOW_TEST, 10, ABM_ALPHA_BLEND|DPM_NONE, 0,
			&depthInfo, shadowMapsShare.GetDepthShadow(), &vParticleLMShadowTestFullLight );
	else
		op.AddOperation( RO_DIR_PARTICLE_LM_SHADOW_TEST, 10, ABM_ZERO|STM_LIGHT|DPM_NONE, 0,//, STM_LIGHT|ABM_ADD|DPM_EQUAL, 0, 
			&depthInfo, shadowMapsShare.GetDepthShadow() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDirectionalLight::RenderPPShadowOps( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, ESceneRenderMode renderMode, 
	IRender *pRender, CSceneFragments &scene, const SLightInfo &lightInfo, const SParticleLMRenderTargetInfo &particleLM )
{
	bool bStaticUpdated = pStaticTracker.Refresh();
	if ( memcmp( &pTS->Get().forward, &mPrevCamera, sizeof(mPrevCamera) ) != 0 )
	{
		mPrevCamera = pTS->Get().forward;
		bStaticUpdated = true;
	}

	CTransformStack sts;
	MakeShadowMatrix( &sts, *pClipTS, vLightDir, fMaxHeight );

	int nDepthTexResolution = shadowMapsShare.GetDepthResolution();

	SPerspDirectionalDepthInfo depthInfo;
	depthInfo.vChannelSelect = CVec4(1,1,0,0);
	depthInfo.vDepth = vDepth;
	depthInfo.m = sts.Get().forward;
	NGfx::GetTexMapFromProjection( &depthInfo.m, nDepthTexResolution );
	{
		CTransformStack particleTS;
		particleTS.MakeParallel( 1, 1,-1000, 1000 );
		SHMatrix particleCam;
		MakeMatrix( &particleCam, CVec3(0,0,0), vLightDir );
		particleTS.SetCamera( particleCam );

		NGfx::CRenderContext rc;
		rc.SetTextureRT( shadowMapsShare.GetDepthShadow() );

		if ( bStaticUpdated )
		{
			bStaticShadowDepthRendered = true;
			rc.ClearBuffers(0);

			CSceneFragments geom;
			CRenderCmdList res;
			rc.SetColorWrite( NGfx::COLORWRITE_RED );
			pRender->FormDepthList( &sts, &particleTS, &geom, IRender::DT_STATIC );//dt );
			MakeSingleOp( &res, geom, true, RO_DIR_DEPTH, &depthInfo );
			Execute( pRender, &rc, sts, res, lightInfo );
		}
		else
		{
			float fExtent = nDepthTexResolution;
			rc.SetColorWrite( NGfx::COLORWRITE_GREEN );
			NGfx::C2DQuadsRenderer qr( rc, CVec2( fExtent, fExtent ), NGfx::QRM_DEPTH_NONE|NGfx::QRM_SOLID );
			CTRect<float> rKernel( 0, 0, fExtent, fExtent );
			qr.AddRect( rKernel, 0, rKernel, NGfx::Get8888Color( CVec4(0,0,0,0) ) );
			rc.ClearZBuffer();
		}

		{
			CSceneFragments geom;
			CRenderCmdList res;
			rc.SetColorWrite( NGfx::COLORWRITE_GREEN );
			pRender->FormDepthList( &sts, &particleTS, &geom, IRender::DT_DYNAMIC );//dt );
			MakeSingleOp( &res, geom, true, RO_DIR_DEPTH, &depthInfo );
			Execute( pRender, &rc, sts, res, lightInfo );
		}

		rc.SetColorWrite( NGfx::COLORWRITE_ALL );
		DrawBorder( &rc, nDepthTexResolution );
	}
	//NGfx::ShowTexture( shadowMapsShare.GetDepthShadow() );

	{
		CRenderCmdList cmds;
		PrepareRender( &cmds, renderMode, scene.GetSelected(), RenderShadowTest, depthInfo );
		Execute( pRender, pRC, *pTS, cmds, lightInfo );
	}

	if ( particleLM.pParticleLMs )
	{
		CVec3 vFullAmbient = pAmbient->GetValue() + MulPerComp( pColor->GetValue(), vShadowColor );
		CVec3 vFullLight = pAmbient->GetValue() + pColor->GetValue();
		vParticleLMShadowTestFullLight = vFullLight;
		bUseSoftLMShadows = NGfx::GetHardwareLevel() >= NGfx::HL_GFORCE3;//false;//

		NGfx::CRenderContext particleRC;
		particleRC.SetTextureRT( particleLM.pParticleLMs );
		if ( bUseSoftLMShadows )
			particleRC.ClearTarget( NGfx::GetDWORDColor( CVec4( vFullAmbient, 1 ) ) );
		else
			particleRC.ClearBuffers( NGfx::GetDWORDColor( CVec4( vFullAmbient, 1 ) ) );
		
		// calc shadows on particles
		CRenderCmdList cmds;
		PrepareRender( &cmds, renderMode, scene.GetLitParticles(), RenderParticleShadowTest, depthInfo );
		CTransformStack ts;
		ts.Init( particleLM.rootTransform );
		Execute( pRender, &particleRC, ts, cmds, lightInfo );

		particleRC.SetAlphaCombine( NGfx::COMBINE_NONE );
		{
			CTRect<float> rDest( 0, 0, particleLM.vParticleLMSize.x, particleLM.vParticleLMSize.y );
			if ( !bUseSoftLMShadows )
			{
				NGfx::C2DQuadsRenderer qr( particleRC, CVec2( rDest.x2, rDest.y2 ), NGfx::QRM_DEPTH_NONE|NGfx::QRM_SOLID|NGfx::QRM_TEST_STENCIL );
				qr.AddRect( rDest, 0, rDest, NGfx::Get8888Color( CVec4( vFullLight, 1 ) ) );
			}
			{
				// render kernel
				NGfx::C2DQuadsRenderer qr( particleRC, CVec2( rDest.x2, rDest.y2 ), NGfx::QRM_DEPTH_NONE|NGfx::QRM_SOLID );
				CTRect<float> rKernel( 0, 0, particleLM.vKernelSize.x, particleLM.vKernelSize.y );
				qr.AddRect( rKernel, 0, rKernel, NGfx::Get8888Color( CVec4( 0.25f, 0.25f, 0.25f, 1 ) ) );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static CVec4 vOccluderColor(0.3f, 0.6f, 0.3f, 1 );
void CDirectionalLight::RenderOccluders( CTransformStack *pTS, NGfx::CRenderContext *pRC, IRender *pRender, 
	CSceneFragments &scene )
{
	CRenderCmdList lightOps;
	const vector<SRenderFragmentInfo*> &fragments = scene.GetSelected();
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &frag = **i;
		SOpGenContext op( &lightOps.ops, &frag );
		op.AddOperation( RO_SOLID_COLOR, 10, 0, 0, &vOccluderColor );
	}
	SLightInfo lightInfo;
	Execute( pRender, pRC, *pTS, lightOps, lightInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDirectionalLight::RenderLightmaps( CTransformStack *pTS, NGfx::CRenderContext *pRC, 
	IRender *pRender, CSceneFragments &scene, ESceneRenderMode renderMode )
{
	CRenderCmdList lightOps;
	const vector<SRenderFragmentInfo*> &fragments = scene.GetSelected();
	ERenderOperation rop = renderMode == RM_SHOWLIGHTMAP ? RO_LIGHTMAP : RO_SKYMAP;
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &frag = **i;
		if ( frag.pStatic->pLightmap )
		{
			SOpGenContext op( &lightOps.ops, &frag );
			op.AddOperation( rop, 10, 0, 0, frag.pStatic->pLightmap.GetPtr() );
		}
		if ( frag.pStatic->pLM )
		{
			SOpGenContext op( &lightOps.ops, &frag );
			op.AddOperation( RO_DYNAMIC_LIGHTMAP, 10, 0, 0, frag.pStatic->pLM );
		}
	}
	SLightInfo lightInfo;
	Execute( pRender, pRC, *pTS, lightOps, lightInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDirectionalLight::RenderAmbientLightmaps( CTransformStack *pTS, NGfx::CRenderContext *pRC, IRender *pRender, 
	CSceneFragments &scene, const SLightInfo &lightInfo )
{
	// render ambient light with lightmaps
	CRenderCmdList lightOps;
	const vector<SRenderFragmentInfo*> &fragments = scene.GetSelected();
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &frag = **i;
		const SMaterialInfo &info = frag.pStatic->pMaterial->GetMaterialInfo();
		if ( frag.pStatic->pLightmap )
		{
			NGfx::CTexture *pLM = frag.pStatic->pLightmap;
			SOpGenContext op( &lightOps.ops, &frag );

			if ( info.diffuse.type == SMaterialInfo::T_COLOR )
				op.AddOperation( RO_LMPD_SOLID, 10, 0, 0, pLM, &info.diffuse.color );
			else
			{
				NGfx::CTexture *pTex = info.diffuse.pTex;
				if ( info.IsDecal() )
				{
					ASSERT( !info.bAlphaTest ); // this is ridiculous - depth is not tested at all in this case
					op.AddOperation( RO_LMPD_TEXTURE_DECAL, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 0, pLM, pTex );
				}
				else
				{
					if ( info.bAlphaTest )
						op.AddOperation( RO_LMPD_TEXTURE_AT, 10, 0, 0, pLM, pTex );
					else
						op.AddOperation( RO_LMPD_TEXTURE, 10, 0, 0, pLM, pTex );
				}
			}
		}
		if ( frag.pStatic->pLM )
		{
			const SDynamicAmbientInfo *pLM = frag.pStatic->pLM;
			SOpGenContext op( &lightOps.ops, &frag );
			if ( info.diffuse.type == SMaterialInfo::T_COLOR )
				op.AddOperation( RO_DYN_LMPD_SOLID, 10, 0, 0, pLM, &info.diffuse.color );
			else
			{
				NGfx::CTexture *pTex = info.diffuse.pTex;
				if ( info.IsDecal() )
				{
					ASSERT( !info.bAlphaTest ); // this is ridiculous - depth is not tested at all in this case
					op.AddOperation( RO_DYN_LMPD_TEXTURE_DECAL, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 0, pLM, pTex );
				}
				else
				{
					if ( info.bAlphaTest )
						op.AddOperation( RO_DYN_LMPD_TEXTURE_AT, 10, 0, 0, pLM, pTex );
					else
						op.AddOperation( RO_DYN_LMPD_TEXTURE, 10, 0, 0, pLM, pTex );
				}
			}
		}
	}
	Execute( pRender, pRC, *pTS, lightOps, lightInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDirectionalLight::RenderInSinglePass( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, ESceneRenderMode renderMode, 
	IRender *pRender, CSceneFragments &scene, const SParticleLMRenderTargetInfo &particleLM, const SLightInfo &lightInfo )
{
	ASSERT( NGfx::GetHardwareLevel() >= NGfx::HL_GFORCE3 );
	CRenderCmdList alphaTestOps;
	const vector<SRenderFragmentInfo*> &fragments = scene.GetSelected();
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &frag = **i;
		SOpGenContext op( &alphaTestOps.ops, &frag );
		const SMaterialInfo &info = frag.pStatic->pMaterial->GetMaterialInfo();
		if ( info.bAlphaTest )
			op.AddOperation( RO_TEXTURE_AT, 10, 0, 2, info.diffuse.pTex );
	}
	Execute( pRender, pRC, *pTS, alphaTestOps, lightInfo );

	RenderPPShadowOps( pTS, pClipTS, pRC, renderMode, pRender, scene, lightInfo, particleLM );
	pRC->SetAlphaCombine( NGfx::COMBINE_NONE );
	NGfx::BlurLight( pRC, 2, 1 );

	CRenderCmdList lightOps;
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &frag = **i;
		if ( frag.pStatic->pMaterial->GetType() != IMaterial::MT_NORMAL )
		{
			ASSERT(0);
			continue;
		}
		SOpGenContext op( &lightOps.ops, &frag );
		const SMaterialInfo &info = frag.pStatic->pMaterial->GetMaterialInfo();
		NGfx::CTexture *pLightmap = frag.pStatic->pLightmap;

		if ( frag.pStatic->pLightmap )
		{
			if ( info.diffuse.type == SMaterialInfo::T_COLOR )
				op.AddOperation( RO_FULL_LIT_SOLID_PP, 10, DPM_EQUAL, 0, &info.diffuse.color, pLightmap );
			else
			{
				ASSERT( info.diffuse.type == SMaterialInfo::T_TEXTURE );
				switch ( info.mt )
				{
				case SMaterialInfo::NORMAL:
					{
						// add gloss ops
						bool bCalcNH = false;
						if ( info.specular.type == SMaterialInfo::T_TEXTURE )
						{
							bCalcNH = true;
							op.AddOperation( RO_PP_SPECULAR_FULL_TEXTURE_DIR, 7, DPM_EQUAL, 0, 2, info.specular.pTex );
						}
						else if ( info.specular.type == SMaterialInfo::T_COLOR )
						{
							bCalcNH = true;
							op.AddOperation( RO_PP_SPECULAR_FULL_COLOR_DIR, 7, DPM_EQUAL, 0, 2, &info.specular.color );
						}
						if ( bCalcNH )
						{
							if ( info.pBump )
								op.AddOperation( RO_NHCALC_BUMP, 3, DPM_EQUAL, 2, info.pBump );
							else
								op.AddOperation( RO_NHCALC, 3, DPM_EQUAL, 2, info.fSpecPower );
						}
						// diffuse
						int nAddFlag = bCalcNH ? ABM_ADD : 0;
						if ( info.pBump )
							op.AddOperation( RO_DIFFUSE_FULL_LIT_BUMP_TEXTURE_PP, 10, nAddFlag|DPM_EQUAL, 0, info.diffuse.pTex, info.pBump, pLightmap );
						else
							op.AddOperation( RO_DIFFUSE_FULL_LIT_TEXTURE_PP, 10, nAddFlag|DPM_EQUAL, 0, info.diffuse.pTex, pLightmap );
					}
					break;
				case SMaterialInfo::DECAL:
					if ( info.pBump )
						op.AddOperation( RO_DIFFUSE_FULL_LIT_BUMP_TEXTURE_PP, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 0, info.diffuse.pTex, info.pBump, pLightmap );
					else
						op.AddOperation( RO_DIFFUSE_FULL_LIT_TEXTURE_PP, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 0, info.diffuse.pTex, pLightmap );
					break;
				default:
					break;
				}
			}
		}
		else if ( frag.pStatic->pLM )
		{
			const SDynamicAmbientInfo *pLM = frag.pStatic->pLM;
			if ( info.diffuse.type == SMaterialInfo::T_COLOR )
				op.AddOperation( RO_DYNLM_LIT_SOLID_PP, 10, DPM_EQUAL, 0, pLM, &info.diffuse.color );
			else
			{
				ASSERT( info.diffuse.type == SMaterialInfo::T_TEXTURE );
				switch ( info.mt )
				{
				case SMaterialInfo::NORMAL:
					{
						// add gloss ops
						bool bCalcNH = false;
						if ( info.specular.type == SMaterialInfo::T_TEXTURE )
						{
							bCalcNH = true;
							op.AddOperation( RO_PP_SPECULAR_FULL_TEXTURE_DIR, 7, DPM_EQUAL, 0, 2, info.specular.pTex );
						}
						else if ( info.specular.type == SMaterialInfo::T_COLOR )
						{
							bCalcNH = true;
							op.AddOperation( RO_PP_SPECULAR_FULL_COLOR_DIR, 7, DPM_EQUAL, 0, 2, &info.specular.color );
						}
						if ( bCalcNH )
						{
							if ( info.pBump )
								op.AddOperation( RO_NHCALC_BUMP, 3, DPM_EQUAL, 2, info.pBump );
							else
								op.AddOperation( RO_NHCALC, 3, DPM_EQUAL, 2, info.fSpecPower );
						}
						// diffuse
						int nAddFlag = bCalcNH ? ABM_ADD : 0;
						if ( info.pBump )
							op.AddOperation( RO_DIFFUSE_DYNLM_LIT_BUMP_TEXTURE_PP, 10, nAddFlag|DPM_EQUAL, 0, pLM, info.diffuse.pTex, info.pBump );
						else
							op.AddOperation( RO_DIFFUSE_DYNLM_LIT_TEXTURE_PP, 10, nAddFlag|DPM_EQUAL, 0, pLM, info.diffuse.pTex );
					}
					break;
				case SMaterialInfo::DECAL:
					if ( info.pBump )
						op.AddOperation( RO_DIFFUSE_DYNLM_LIT_BUMP_TEXTURE_PP, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 0, pLM, info.diffuse.pTex, info.pBump );
					else
						op.AddOperation( RO_DIFFUSE_DYNLM_LIT_TEXTURE_PP, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, 0, pLM, info.diffuse.pTex );
					break;
				default:
					break;
				}
			}
		}
	}
	Execute( pRender, pRC, *pTS, lightOps, lightInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDirectionalLight::Render( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, ESceneRenderMode renderMode, 
	IRender *pRender, CSceneFragments &scene, const SParticleLMRenderTargetInfo &particleLM )
{
//	if ( lights.empty() )
//		return;
	pAmbient.Refresh();
	SLightInfo lightInfo;
	PrepareLightInfo( &lightInfo );
	if ( renderMode < RM_BEST_GF2 )
	{
//		if ( renderMode == RM_FASTEST )
//			scene.ResetLightmaps();
		//lightInfo.vLightColor *= 0.5f;
		switch ( renderMode )
		{
			case RM_TNL:
				Render( pTS, pRC, renderMode, pRender, scene, lightInfo, RenderTnL, pAmbient->GetValue() );
				break;
			case RM_FASTEST:
				Render( pTS, pRC, renderMode, pRender, scene, lightInfo, RenderFast, pAmbient->GetValue() );
				break;
			case RM_SHOWOCCLUDERS:
				RenderOccluders( pTS, pRC, pRender, scene );
				break;
			case RM_SHOWLIGHTMAP:
			case RM_SHOWSKYMAP:
				RenderLightmaps( pTS, pRC, pRender, scene, renderMode );
				break;
			case RM_LIGHTMAPPED:
				RenderAmbientLightmaps( pTS, pRC, pRender, scene, lightInfo );
				break;
			default:
				ASSERT(0);
				break;
		}
		return;
	}
	if ( renderMode == RM_BEST_GF3 )
	{
		RenderInSinglePass( pTS, pClipTS, pRC, renderMode, pRender, scene, particleLM, lightInfo );
		return;
	}
	// render ambient
	RenderAmbientLightmaps( pTS, pRC, pRender, scene, lightInfo );
	// render light into first register
	Render( pTS, pRC, renderMode, pRender, scene, lightInfo, RenderLight, pAmbient->GetValue() );
	RenderPPShadowOps( pTS, pClipTS, pRC, renderMode, pRender, scene, lightInfo, particleLM );
	FinalPass( pRC, renderMode );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDirectionalLight::GetRadianceInfo( SRadianceInfo *pRes, ESceneRenderMode rm )
{
	pRes->bIsRendered = true;//rm >= RM_BEST_GF2;
	pColor.Refresh();
	pRes->vColor = pColor->GetValue();
	pAmbient.Refresh();
	pRes->vAmbientColor = pAmbient->GetValue();
	//CVec4 &vDir = lights.front().vLightDir;
	pRes->vDirection = vLightDir;//CVec3( -vDir.x, -vDir.y, -vDir.z );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPointLight
////////////////////////////////////////////////////////////////////////////////////////////////////
CPointLight::CPointLight( const CVec3 &_vColor, const CVec3 &_ptCenter, float _fRadius, CVersioningBase *_pStaticTracker, bool _bLightmapOnly )
	: vColor(_vColor), ptCenter(_ptCenter, 1),
	bLightmapOnly(_bLightmapOnly)
{
	NGfx::InitRadius( &vRadius, _fRadius );
	sTransform.MakeParallel( _fRadius, _fRadius, -_fRadius, _fRadius );
	SHMatrix cameraPos;
	MakeMatrix( &cameraPos, _ptCenter, CVec3( 0, 0, 1 ) );
	sTransform.SetCamera( cameraPos );

	sBound.SphereInit( _ptCenter, _fRadius );
	pStaticTracker = _pStaticTracker;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPointLight::CheckCulling( CTransformStack *pTS )
{
	return pTS->IsIn( sBound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPointLight::CreateShadowVolumes( IRender *pRender, float *pHullRadius )
{
	CVec3 ptCenter3( ptCenter.x, ptCenter.y, ptCenter.z );
	bool bStaticUpdated = pStaticTracker.Refresh();

	if ( bStaticUpdated || !IsValid( pStaticGeom ) || !IsValid( pStaticTris ) )
	{
		vector<CVec3> staticVertices;
		vector<STriangle> staticTris;

		pStaticTris = 0;
		pStaticGeom = 0;

		MakeShadowVolumes( pRender, &sTransform, ptCenter3, GetFRadius(), &staticTris, &staticVertices, 
			IRender::DT_STATIC, &fStaticHullRadius, &ignoreSet );
		if ( staticTris.size() > 0 && staticTris.size() < 40000 && staticVertices.size() < 40000 ) // CRAP limits
			CalcShadowVolumes( staticVertices, staticTris, &pStaticGeom, &pStaticTris, false );
	}

	//	{
	//		vector<CVec3> dynamicVertices;
	//		vector<STriangle> dynamicTris;
	//		pDynamicTris = 0;
	//		pDynamicGeom = 0;

	//		MakeShadowVolumes( pRender, &sTransform, ptCenter3, GetFRadius(), &dynamicTris, &dynamicVertices, IRender::DT_DYNAMIC );
	//		if ( dynamicTris.size() > 0 )
	//		{
	//			pDynamicTris = NGfx::MakeTriList( dynamicTris.size(), NGfx::DYNAMIC );
	//			pDynamicGeom = NGfx::MakeGeometry( dynamicVertices.size(), NGfx::SGeomVec::ID, NGfx::DYNAMIC );
	//			if ( IsValid( pDynamicGeom ) && IsValid( pDynamicTris ) )
	//				CalcShadowVolumes( dynamicVertices, dynamicTris, &pDynamicGeom, &pDynamicTris, true );
	//		}
	//	}
	*pHullRadius = fStaticHullRadius;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPointLight::CalcShadowVolumes( const vector<CVec3> &_points, const vector<STriangle> &_tris, 
	CObj<NGfx::CGeometry> *pGeom, CObj<NGfx::CTriList> *pTris, bool bDynamic )
{
	NGfx::CBufferLock<NGfx::SGeomVecFull> points( pGeom, _points.size(), bDynamic ? NGfx::DYNAMIC : NGfx::STATIC );
	NGfx::CBufferLock<NGfx::S3DTriangle> tris( pTris, _tris.size(), bDynamic ? NGfx::DYNAMIC : NGfx::STATIC );

	for ( int k = 0; k < _points.size(); ++k )
	{
		NGfx::SGeomVecFull res;
		res.pos = _points[k];
		points[k] = res;
	}
	for ( int k = 0; k < _tris.size(); ++k )
		tris[k] = NGfx::S3DTriangle( _tris[k].i1, _tris[k].i2, _tris[k].i3 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPointLight::RenderShadowVolumes( NGfx::CRenderContext *pRC )
{
	NGfx::CRenderContext rc( *pRC );
	NGfx::SEffPureGeometry l;
	//NGfx::SEffConstLight l;
	//l.color = CVec3(0.7f, 0.7f, 1 );
	//rc.SetRegister(0);
	//rc.SetAlphaCombine( NGfx::COMBINE_NONE );
	//rc.SetDepth( NGfx::DEPTH_NORMAL );
	//rc.SetAlphaCombine( NGfx::COMBINE_NONE );
	//rc.SetRegister(1);

	rc.SetDepth( NGfx::DEPTH_GREATEEQTEST );
	rc.SetStencil( NGfx::STENCIL_INCR, 0, 0x7f );
	rc.SetCulling( NGfx::CULL_CCW );
	rc.SetColorWrite( NGfx::COLORWRITE_NONE );
	rc.SetAlphaCombine( NGfx::COMBINE_ZERO_ONE );
	rc.SetEffect( &l );
	if ( IsValid( pStaticGeom ) && IsValid( pStaticTris ) )
		rc.DrawPrimitive( pStaticGeom, pStaticTris );
	if ( IsValid( pDynamicTris ) && IsValid( pDynamicGeom ) )
		rc.DrawPrimitive( pDynamicGeom, pDynamicTris );

	rc.SetCulling( NGfx::CULL_CW );
	rc.SetStencil( NGfx::STENCIL_DECR, 0, 0x7f );
	if ( IsValid( pStaticGeom ) && IsValid( pStaticTris ) )
		rc.DrawPrimitive( pStaticGeom, pStaticTris );
	if ( IsValid( pDynamicTris ) && IsValid( pDynamicGeom ) )
		rc.DrawPrimitive( pDynamicGeom, pDynamicTris );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPointLight::FinalPass( NGfx::CRenderContext *pRC, float fHullRadius )
{
	// render octahedron transering data from one register to another
	CObj<NGfx::CGeometry> pGeom;
	vector<STriangle> tris(8);
	{
		NGfx::CBufferLock<NGfx::SGeomVecFull> points( &pGeom, 6 );
		float fOffset = fHullRadius;//GetFRadius() * 1.74f;
		for ( int i = 0; i < 6; ++i )
		{
			CVec3 ptRes( ptCenter.x, ptCenter.y, ptCenter.z );
			ptRes.m[i/2] += ( (i&1) != 0 ) ? -fOffset : fOffset;
			NGfx::SGeomVecFull res;
			res.pos = ptRes;
			points[i] = res;
		}
		tris[0] = STriangle(0,2,5);
		tris[1] = STriangle(2,1,5);
		tris[2] = STriangle(1,3,5);
		tris[3] = STriangle(3,0,5);
		tris[4] = STriangle(2,0,4);
		tris[5] = STriangle(1,2,4);
		tris[6] = STriangle(3,1,4);
		tris[7] = STriangle(0,3,4);
	}
	pRC->SetRegister(0);
	NGfx::CRenderContext rc(*pRC);
	rc.SetStencil( NGfx::STENCIL_TESTNE_CLEAR, 0, 0x80 );
	rc.SetAlphaCombine( NGfx::COMBINE_ADD );
	rc.SetDepth( NGfx::DEPTH_NONE );
	NGfx::SEffRegister reg;
	reg.pTex = NGfx::GetRegisterTexture( 1 );
	rc.SetEffect( &reg );
	//NGfx::SEffConstLight l;
	//l.color = CVec3( 1,1,1 );
	//rc.SetEffect( &l );
	rc.DrawPrimitive( pGeom, tris );
	//NGfx::SEffPureGeometry fill;// SEffConstLight fill;//
	//fill.color = CVec4( 1,1,1,1 );
	//rc.SetStencil( NGfx::STENCIL_WRITE, 0 );
	//rc.SetAlphaCombine( NGfx::COMBINE_ZERO_ONE );
	//rc.SetEffect( &fill );
	//rc.DrawPrimitive( pGeom, pTris );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPointLight::Add( SOpGenContext &op, ESceneRenderMode rm, const SMaterialInfo &info )
{
	switch ( rm )
	{
	case RM_TNL:
	case RM_FASTEST:
	case RM_SHOWOCCLUDERS:
	case RM_LIGHTMAPPED:
	case RM_SHOWLIGHTMAP:
	case RM_SHOWSKYMAP:
		ASSERT(0);
		break;
	case RM_BEST_GF2:
		// add diffuse ops
		if ( info.diffuse.type == SMaterialInfo::T_COLOR )
			op.AddOperation( RO_PNT_LIT_SOLID, 10, STM_STENCIL_LIGHT|DPM_EQUAL, 1, &info.diffuse.color );
		else
		{
			if ( info.IsDecal() )
				op.AddOperation( RO_PNT_LIT_TEXTURE, 20, STM_TEST_STENCIL_LIGHT|ABM_ALPHA_BLEND|DPM_TESTONLY, 1, info.diffuse.pTex );
			else
				op.AddOperation( RO_PNT_LIT_TEXTURE, 10, STM_STENCIL_LIGHT|DPM_EQUAL, 1, info.diffuse.pTex );
		}
		// add gloss ops
		if ( info.specular.type == SMaterialInfo::T_TEXTURE )
			op.AddOperation( RO_PNT_SPECULAR_TEXTURE, 11, ABM_ADD|DPM_EQUAL, 1, info.specular.pTex, info.fSpecPower );
		else if ( info.specular.type == SMaterialInfo::T_COLOR )
			op.AddOperation( RO_PNT_SPECULAR_COLOR, 11, ABM_ADD|DPM_EQUAL, 1, &info.specular.color, info.fSpecPower );
		break;

	case RM_BEST_GF3:
		// add diffuse ops
		if ( info.diffuse.type == SMaterialInfo::T_COLOR )
			op.AddOperation( RO_PNT_LIT_SOLID, 10, STM_STENCIL_LIGHT|DPM_EQUAL, 1, &info.diffuse.color );
		else
		{
			if ( info.pBump )
			{
				if ( info.IsDecal() )
					op.AddOperation( RO_PNT_LIT_TEXTURE_PBUMP, 20, STM_TEST_STENCIL_LIGHT|ABM_ALPHA_BLEND|DPM_TESTONLY, 1, info.pBump, info.diffuse.pTex );
				else
					op.AddOperation( RO_PNT_LIT_TEXTURE_PBUMP, 10, STM_STENCIL_LIGHT|DPM_EQUAL, 1, info.pBump, info.diffuse.pTex );
			}
			else
			{
				if ( info.IsDecal() )
					op.AddOperation( RO_PNT_LIT_TEXTURE, 20, STM_TEST_STENCIL_LIGHT|ABM_ALPHA_BLEND|DPM_TESTONLY, 1, info.diffuse.pTex );
				else
					op.AddOperation( RO_PNT_LIT_TEXTURE, 10, STM_STENCIL_LIGHT|DPM_EQUAL, 1, info.diffuse.pTex );
			}
		}
		// add gloss ops
		{
			bool bCalcNH = false;
			if ( info.specular.type == SMaterialInfo::T_TEXTURE )
			{
				bCalcNH = true;
				op.AddOperation( RO_PP_SPECULAR_TEXTURE_PNT, 11, ABM_ADD|DPM_EQUAL, 1, 2, info.specular.pTex );
			}
			else if ( info.specular.type == SMaterialInfo::T_COLOR )
			{
				bCalcNH = true;
				op.AddOperation( RO_PP_SPECULAR_COLOR_PNT, 11, ABM_ADD|DPM_EQUAL, 1, 2, &info.specular.color );
			}
			if ( bCalcNH )
			{
				if ( info.pBump )
					op.AddOperation( RO_NHCALC_BUMP, 3, DPM_EQUAL, 2, info.pBump );
				else
					op.AddOperation( RO_NHCALC, 3, DPM_EQUAL, 2, info.fSpecPower );
			}
		}
		break;
	default:
		ASSERT(0); 
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPointLight::Render( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, ESceneRenderMode renderMode, 
	IRender *pRender, CSceneFragments &scene, const SParticleLMRenderTargetInfo &particleLM )
{
	if ( renderMode < RM_BEST_GF2 )
		return;
	if ( bLightmapOnly )
		return;
	if ( !bStencilShadows )
		return;

	float fHullRadius;
	CreateShadowVolumes( pRender, &fHullRadius );
	RenderShadowVolumes( pRC );
	// render lit stuff
	CRenderCmdList lightOps;
	//STestSpheres test( sBound, ignoreSet );
	SSelectFragments selector( &scene, SSphereAndIgnoredFilter( sBound.s, ignoreSet ) );
	const vector<SRenderFragmentInfo*> &fragments = scene.GetSelected();
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &f = **i;
		//if ( !test( f.bound, f.pElement.GetPtr() ) )
		//	continue;
		SOpGenContext op( &lightOps.ops, &f );
		const SMaterialInfo &info = f.pStatic->pMaterial->GetMaterialInfo();
		Add( op, renderMode, info );
	}
	SLightInfo lightInfo;
	lightInfo.bNeedSet = true;
	lightInfo.vLightColor = vColor;
	lightInfo.vLightPos = ptCenter;
	lightInfo.vRadius = vRadius;
	Execute( pRender, pRC, *pTS, lightOps, lightInfo );
	// copy to first register
	FinalPass( pRC, fHullRadius );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPointLight::GetRadianceInfo( SRadianceInfo *pRes, ESceneRenderMode rm )
{
	pRes->bIsRendered = ( rm >= RM_BEST_GF2 ) && !bLightmapOnly && bStencilShadows;
	pRes->vColor = vColor;
	pRes->vCenter = CVec3( ptCenter.x, ptCenter.y, ptCenter.z );
	pRes->fRadius = vRadius.x;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDynamicPointLight
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDynamicPointLight::Add( SOpGenContext &op, ESceneRenderMode rm, const SMaterialInfo &info )
{
	int nDFlags = nTargetReg ? STM_MARK : ABM_ADD;
	switch ( rm )
	{
	case RM_TNL:
	case RM_FASTEST:
	case RM_SHOWOCCLUDERS:
	case RM_LIGHTMAPPED:
	case RM_SHOWLIGHTMAP:
	case RM_SHOWSKYMAP:
		ASSERT(0);
		break;
	case RM_BEST_GF2:
		// add diffuse ops
		if ( info.diffuse.type == SMaterialInfo::T_COLOR )
			op.AddOperation( RO_PNT_LIT_SOLID16, 10, nDFlags|DPM_EQUAL, nTargetReg, &info.diffuse.color );
		else
		{
			if ( info.IsDecal() )
				op.AddOperation( RO_PNT_LIT_TEXTURE16, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, nTargetReg, info.diffuse.pTex );
			else
				op.AddOperation( RO_PNT_LIT_TEXTURE16, 10, nDFlags|DPM_EQUAL, nTargetReg, info.diffuse.pTex );
		}
		break;

	case RM_BEST_GF3:
		// add diffuse ops
		if ( info.diffuse.type == SMaterialInfo::T_COLOR )
			op.AddOperation( RO_PNT_LIT_SOLID16, 10, nDFlags|DPM_EQUAL, nTargetReg, &info.diffuse.color );
		else
		{
			if ( info.pBump )
			{
				if ( info.IsDecal() )
					op.AddOperation( RO_PNT_LIT_TEXTURE_PBUMP16, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, nTargetReg, info.pBump, info.diffuse.pTex );
				else
					op.AddOperation( RO_PNT_LIT_TEXTURE_PBUMP16, 10, nDFlags|DPM_EQUAL, nTargetReg, info.pBump, info.diffuse.pTex );
			}
			else
			{
				if ( info.IsDecal() )
					op.AddOperation( RO_PNT_LIT_TEXTURE16, 20, ABM_ALPHA_BLEND|DPM_TESTONLY, nTargetReg, info.diffuse.pTex );
				else
					op.AddOperation( RO_PNT_LIT_TEXTURE16, 10, nDFlags|DPM_EQUAL, nTargetReg, info.diffuse.pTex );
			}
		}
		// add gloss ops
		{
			bool bCalcNH = false;
			if ( info.specular.type == SMaterialInfo::T_TEXTURE )
			{
				bCalcNH = true;
				op.AddOperation( RO_PP_SPECULAR_TEXTURE_PNT, 11, ABM_ADD|DPM_EQUAL, nTargetReg, 2, info.specular.pTex );
			}
			else if ( info.specular.type == SMaterialInfo::T_COLOR )
			{
				bCalcNH = true;
				op.AddOperation( RO_PP_SPECULAR_COLOR_PNT, 11, ABM_ADD|DPM_EQUAL, nTargetReg, 2, &info.specular.color );
			}
			if ( bCalcNH )
			{
				if ( info.pBump )
					op.AddOperation( RO_NHCALC_BUMP, 3, DPM_EQUAL, 2, info.pBump );
				else
					op.AddOperation( RO_NHCALC, 3, DPM_EQUAL, 2, info.fSpecPower );
			}
		}
		break;
	default:
		ASSERT(0); 
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDynamicPointLight::FinalPass( NGfx::CRenderContext *pRC )
{
	pLight.Refresh();
	const CAnimLight &l = *pLight->GetValue();
	// render octahedron transering data from one register to another
	CObj<NGfx::CGeometry> pGeom;
	vector<STriangle> tris(12);
	{
		NGfx::CBufferLock<NGfx::SGeomVecFull> points( &pGeom, 8 );
		float fOffset = l.fRadius;
		for ( int i = 0; i < 8; ++i )
		{
			CVec3 ptRes( l.position );
			ptRes.x += ((i&1) != 0) ? -fOffset : fOffset;
			ptRes.y += ((i&2) != 0) ? -fOffset : fOffset;
			ptRes.z += ((i&4) != 0) ? -fOffset : fOffset;
			NGfx::SGeomVecFull res;
			res.pos = ptRes;
			points[i] = res;
		}
		tris[0 ] = STriangle(0,1,3);
		tris[1 ] = STriangle(0,3,2);
		tris[2 ] = STriangle(2,3,7);
		tris[3 ] = STriangle(2,7,6);
		tris[4 ] = STriangle(0,2,6);
		tris[5 ] = STriangle(0,6,4);
		tris[6 ] = STriangle(1,0,4);
		tris[7 ] = STriangle(1,4,5);
		tris[8 ] = STriangle(3,1,5);
		tris[9 ] = STriangle(3,5,7);
		tris[10] = STriangle(6,7,5);
		tris[11] = STriangle(6,5,4);
	}
	pRC->SetRegister(0);
	NGfx::CRenderContext rc(*pRC);
	rc.SetStencil( NGfx::STENCIL_TESTNE_CLEAR, 0, 0x80 );
	rc.SetAlphaCombine( NGfx::COMBINE_ADD );
	rc.SetDepth( NGfx::DEPTH_NONE );
	NGfx::SEffRegister reg;
	reg.pTex = NGfx::GetRegisterTexture( 1 );
	rc.SetEffect( &reg );
	//NGfx::SEffConstLight cl;
	//cl.color = CVec3( 1,1,1 );
	//rc.SetEffect( &cl );
	rc.DrawPrimitive( pGeom, tris );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDynamicPointLight::Render( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, ESceneRenderMode renderMode, 
	IRender *pRender, CSceneFragments &scene, const SParticleLMRenderTargetInfo &particleLM )
{
	if ( renderMode < RM_BEST_GF2 )
		return;

	pLight.Refresh();
	const CAnimLight &l = *pLight->GetValue();
	float fScale = 0.5f;
	CVec3 lColor = CVec3( fScale * sqrt(l.color.x), fScale * sqrt(l.color.y), fScale * sqrt(l.color.z) );

	if ( fabs2( lColor ) < 0.001f || l.bEnd )
		return;

	SBound sBound;
	sBound.SphereInit( l.position, l.fRadius );

	// render lit stuff
	CRenderCmdList lightOps;
	SSelectFragments selector( &scene, SSphereFilter( sBound.s ) );
	const vector<SRenderFragmentInfo*> &fragments = scene.GetSelected();
	bool bHasDecal = false;
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &f = **i;
		const SMaterialInfo &info = f.pStatic->pMaterial->GetMaterialInfo();
		if ( info.IsDecal() )
			bHasDecal = true;
	}
	nTargetReg = bHasDecal ? 1 : 0;
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &f = **i;
		SOpGenContext op( &lightOps.ops, &f );
		const SMaterialInfo &info = f.pStatic->pMaterial->GetMaterialInfo();
		Add( op, renderMode, info );
	}
	SLightInfo lightInfo;
	lightInfo.bNeedSet = true;
	lightInfo.vLightColor = lColor;
	lightInfo.vLightPos = l.position;
	NGfx::InitRadius( &lightInfo.vRadius, l.fRadius );
	Execute( pRender, pRC, *pTS, lightOps, lightInfo );
	if ( bHasDecal )
		FinalPass( pRC );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDynamicPointLight::CheckCulling( CTransformStack *pTS )
{
	pLight.Refresh();
	const CAnimLight &l = *pLight->GetValue();
	if ( !l.bActive || l.bEnd )
		return false;
	return pTS->IsIn( SSphere( l.position, l.fRadius ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSpotLight
////////////////////////////////////////////////////////////////////////////////////////////////////
/*CSpotLight::CSpotLight( CFuncBase<CVec3> *_pColor, const CVec3 &_ptCenter, const CVec3 &ptDir, 
	float fFov, float _fRadius, CPtrFuncBase<NGfx::CTexture> *_pMask )
	: CLightBase(_pColor), pMask(_pMask), ptCenter(_ptCenter, 1), fRadius(_fRadius)
{
	ts.MakeProjective( 1, fFov, fRadius * 0.15f, fRadius );
	SHMatrix cameraPos;
	MakeMatrix( &cameraPos, _ptCenter, ptDir );
	ts.SetCamera( cameraPos );
	bv.SphereInit( _ptCenter, fRadius );
	//pShadow = new CFacSpotShadow;
	//CalcWorldToLight( &pShadow->mWorldToLight );
	if ( _pMask )
	{
		//CFacProjectedTexture *pProj = new CFacProjectedTexture( _pMask );
		//pProj->mWorldToLight = pShadow->mWorldToLight;
		//pDiffuse = Mul( new CFacPointDiffuseLight( _pColor, _ptCenter, fRadius ), pProj );
		//pProject = pProj;
	}
	//else
		//pDiffuse = new CFacPointDiffuseLight( _pColor, _ptCenter, fRadius );
	//pDepthFactor = new CFacPerspDepth;
	//pParticleLight = 0;//new CFacConstColor( _pColor );
	//pFalloff = new CFacPointFalloff( _ptCenter, fRadius, _pColor );
	//pWhite = new CCVec3( CVec3(1,1,1) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_SPOT_TEXTURE_RES = 256;
bool CSpotLight::Update( NGfx::CRenderContext *pRC, ESceneRenderMode renderMode, IRender *pRender )
{
//	if ( !IsValid( pDepth ) )
//		pDepth = NGfx::MakeTexture( N_SPOT_TEXTURE_RES, N_SPOT_TEXTURE_RES, 1, NGfx::SPixel8888::ID, NGfx::TARGET );
//	ASSERT( IsValid( pDepth ) );
//	if ( !IsValid( pDepth ) )
//		return false;
//	CRenderList geom;//( CRenderList::MODE_GEOMETRY );
//	pRender->FormDepthList( &ts, &geom, IRender::DT_ALL );
//	NGfx::CRenderContext rc;
//	rc.SetRenderTarget( pDepth, 0 );
//	rc.SetAlphaCombine( NGfx::COMBINE_NONE );
//	rc.SetStencil( NGfx::STENCIL_NONE );
//	rc.SetDepth( NGfx::DEPTH_NORMAL );
//	rc.ClearBuffers( 0 );
//	geom.MakeSingleOp( pDepthFactor );
//	CRenderCommandsList cmds;
//	Compile( &cmds, geom.GetFactoredRep(), 0 );
//	Execute( &cmds, &rc, ts, pRender );
	//ShowTexture( pDepth );
//	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSpotLight::CheckCulling( CTransformStack *pTS )
{
	return pTS->IsIn( bv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSpotLight::Select( CRenderList *pList )
{
	pList->AddLight( STestFrustrum( &ts ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSpotLight::CalcWorldToLight( SHMatrix *pRes )
{
	*pRes = ts.Get().forward;
	GetTexMapFromProjection( pRes, N_SPOT_TEXTURE_RES );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float fSFPoints[5][4] = 
{
	{-1,-1,1,1},
	{-1, 1,1,1},
	{ 1, 1,1,1},
	{ 1,-1,1,1},
	{ 0, 0,0,1},
};
void CSpotLight::RenderFrustrum( NGfx::SEffect *pEffect, NGfx::CRenderContext *pRC )
{
//	CObj<NGfx::CGeometry> pGeom = NGfx::MakeGeometry( 5, NGfx::SDynamicVector::ID, NGfx::DYNAMIC );
//	vector<STriangle> tris(12)
//	{
//		NGfx::CBufferLock<NGfx::SDynamicVector> points(pGeom);
//		NGfx::CBufferLock<NGfx::S3DTriangle> tris(pTris);
//		for ( int i = 0; i < 5; ++i )
//		{
//			CVec4 ptRes;
//			ts.Get().backward.RotateHVector( &ptRes, CVec4( fSFPoints[i][0], fSFPoints[i][1], fSFPoints[i][2], fSFPoints[i][3] ) );
//			float fW1 = 1 / ptRes.w;
//			points[i].pos = CVec3( ptRes.x * fW1, ptRes.y * fW1, ptRes.z * fW1 );
//		}
//		tris[0] = NGfx::S3DTriangle(4,0,1);
//		tris[1] = NGfx::S3DTriangle(4,1,2);
//		tris[2] = NGfx::S3DTriangle(4,2,3);
//		tris[3] = NGfx::S3DTriangle(4,3,0);
//		tris[4] = NGfx::S3DTriangle(1,2,3);
//		tris[5] = NGfx::S3DTriangle(1,3,0);
//		tris[6] = NGfx::S3DTriangle(4,1,0);
//		tris[7] = NGfx::S3DTriangle(4,2,1);
//		tris[8] = NGfx::S3DTriangle(4,3,2);
//		tris[9] = NGfx::S3DTriangle(4,0,3);
//		tris[10] = NGfx::S3DTriangle(1,3,2);
//		tris[11] = NGfx::S3DTriangle(1,0,3);
//	}
//	pRC->SetEffect( pEffect );
//	pRC->DrawPrimitive( pGeom, pTris );
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
/*void CSpotLight::Prepare( NGfx::CRenderContext *pRC )
{
	NGfx::SEffConstLight l;
	l.color = CVec4(0,0,0,0);
	pRC->SetDepth( NGfx::DEPTH_INVERSETEST );
	pRC->SetAlphaCombine( NGfx::COMBINE_ZERO_ONE );
	pRC->SetStencil( NGfx::STENCIL_INVERT, 0, 1 );
	RenderFrustrum( &l, pRC );
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
/*void CSpotLight::Cleanup( NGfx::CRenderContext *pRC )
{
	NGfx::SEffConstLight l;
	l.color = CVec4(0,0,0,0);
	pRC->SetDepth( NGfx::DEPTH_INVERSETEST );
	pRC->SetAlphaCombine( NGfx::COMBINE_ZERO_ONE );
	pRC->SetStencil( NGfx::STENCIL_INVERT, 0, 1 );
	RenderFrustrum( &l, pRC );
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
/*IRenderFactor* CSpotLight::GetShadowFactor()
{
	pShadow->pDepth = &pDepth;
	return pShadow;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IRenderFactor* CSpotLight::GetDiffuseFactor()
{
	return pDiffuse;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IRenderFactor* CSpotLight::GetParticleDiffuseFactor()
{
	return pParticleLight;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IRenderFactor* CSpotLight::CreateSpecularFactor( float fRo )
{
	IRenderFactor *pRes;
	if ( bUsePerPixelSpecular && NGfx::GetHardwareLevel() >= NGfx::HL_GFORCE3 )
		pRes = Mul( Chain( 
		new CFacNHCalcer( ptCenter, fRo ),
		new CFacPerPixelSpecularLight( pWhite ) ),
		pFalloff );
	else
		pRes = new CFacPointSpecularLight( pColor, ptCenter, fRadius, fRo );
	if ( pProject )
		return Mul( pRes, pProject );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IRenderFactor* CSpotLight::CreateSpecularFactor( float fRo, CPtrFuncBase<NGfx::CTexture> *_pBump, bool bWrap )
{
	IRenderFactor *pRes;
	if ( NGfx::GetHardwareLevel() >= NGfx::HL_GFORCE3 )
		pRes = Mul( Chain( 
		new CFacNHBumpedCalcer( ptCenter, _pBump, bWrap ),
		new CFacPerPixelSpecularLight( pWhite ) ),
		pFalloff );
	else
		return CreateSpecularFactor( fRo );
	if ( pProject )
		return Mul( pRes, pProject );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IRenderFactor* CSpotLight::CreateDiffuseFactor( CPtrFuncBase<NGfx::CTexture> *_pBump, bool bWrap, float fTransp )
{
	IRenderFactor *pRes;
	if ( bUseFastBump )
		pRes = new CFacFastBumpedPointDiffuseLight( pColor, ptCenter, fRadius, _pBump, bWrap );
	else
		pRes = Mul( new CFacPreciseBumpedDiffuseLight( ptCenter, _pBump, bWrap ), pFalloff );//pDiffuse );
	if ( pProject )
		pRes = Mul( pRes, pProject );
	return pRes;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/*static void SetFastBump( const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() != 1 )
	{
		csSystem << "gfx fastbump on|off" << endl;
		return;
	}
	if ( szParams[0] == L"on" )
		bUseFastBump = true;
	else
		bUseFastBump = false;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(GRenderLight)
	REGISTER_VAR_EX( "gfx_stencil_shadows", NGlobal::VarBoolHandler, &bStencilShadows, 0, true )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x02511009, CDirectionalLight )
REGISTER_SAVELOAD_CLASS( 0x01961170, CPointLight )
//REGISTER_SAVELOAD_CLASS( 0x02161170, CSpotLight )
REGISTER_SAVELOAD_CLASS( 0x02682130, CDynamicPointLight )
