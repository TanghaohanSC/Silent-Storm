#include "StdAfx.h"
#include "GfxBuffers.h"
#include "GfxRender.h"
#include "GTransparent.h"
#include "GParticleInfo.h"
#include "Transform.h"
#include "GfxUtils.h"
#include "GSceneParticles.h"
#include "GfxEffects.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTransparentRenderer
////////////////////////////////////////////////////////////////////////////////////////////////////
CTransparentRenderer::CTransparentRenderer( CTransformStack *pTS, const CTPoint<int> &vLightBuffersize, 
	bool _bUseFakeLM, DWORD _dwLitColor, DWORD _dwNormalColor )
	: nTotalParticles(0), nLitParticles(0), dwLitColor(_dwLitColor), dwNormalColor(_dwNormalColor)
{
	bTnLMode = NGfx::IsTnLDevice();
	SHMatrix invRoot;
	invRoot = pTS->Get().backward * pTS->GetProjection().forward;
	orientation.vBasic[0] = CVec3( invRoot._11, invRoot._21, invRoot._31 );
	orientation.vBasic[1] = CVec3( invRoot._12, invRoot._22, invRoot._32 );
	orientation.vBasic[2] = CVec3( invRoot._13, invRoot._23, invRoot._33 );
	orientation.vBasic[3] = invRoot.GetTranslation();
	orientation.vDepth = CVec3( -pTS->Get().forward.wx, -pTS->Get().forward.wy, -pTS->Get().forward.wz );;

	bUseFakeLM = _bUseFakeLM;
	if ( bUseFakeLM )
	{
		ASSERT( vLightBuffersize.x == 2 );
		ASSERT( vLightBuffersize.y == 1 );
		litParticlesAlloc.Init( 1, 1, 2, 1, false );
		//litParticlesAlloc.vLightSize = CVec2(1,1);
		//litParticlesAlloc.ptStart = CTPoint<int>(0,0);
		//litParticlesAlloc.ptSize = CTPoint<int>( 2, 1 );
		//litParticlesAlloc.bIncremental = false;
		kernel = litParticlesAlloc;
		litParticlesAlloc.ForcedInc();
		//litParticlesAlloc.ptStart = CTPoint<int>(1,0);
	}
	else
	{
		litParticlesAlloc.Init( 4, 4, Float2Int( vLightBuffersize.x ), Float2Int( vLightBuffersize.y ), true );
		//litParticlesAlloc.vLightSize = CVec2(4,4);
		//litParticlesAlloc.ptStart = CTPoint<int>(0,0);
		//litParticlesAlloc.ptSize = CTPoint<int>( vLightBuffersize.x / 4, vLightBuffersize.y / 4 );
		//litParticlesAlloc.bIncremental = true;
		kernel = litParticlesAlloc;
		litParticlesAlloc.Inc();
		kernel.StopIncrementing();
	}
	NGfx::CalcCompactVector( &vNormal, -orientation.vBasic[2] );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::AllocParticlesWriteBuffer()
{
	if ( bTnLMode )
		pParticlesGeometry = pTnLParticlesGeometry = new CTnLParticlesGeometry( orientation );
	else
		pParticlesGeometry = pShaderParticlesGeometry = new CShaderParticlesGeometry( orientation, vNormal );
	pParticlesTrilist = new CParticlesTriList;
	nTargetParticle = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::FinishParticles() 
{
	if ( IsValid(pParticlesGeometry) )
		pParticlesGeometry->FreeWriteBuffer();
	pParticlesGeometry = 0;
	pTnLParticlesGeometry = 0;
	pShaderParticlesGeometry = 0;
	pParticlesTrilist = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::FinishParticlesPiece()
{
	int nParticles = nTargetParticle - nPieceStart;
	pParticlesTrilist->AddPart( nParticles );
	SBound bv;
	pParticlesGeometry->FinishPart( &bv, pLMAlloc );
	if ( pReportParticles )
	{
		pReportParticles->AddParticles( pParticlesGeometry, pParticlesTrilist, pParticlesGeometry->GetPartsNum() - 1, 
			nParticles, bv );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::StartParticlesPiece()
{
	pWriteParticles = AddFragment();
	pWriteParticles->pGeom = pParticlesGeometry;
	pWriteParticles->fDepth = currentEffectBV.s.ptCenter * GetDepth();
	pWriteParticles->nOffset = nTargetParticle * 4;
	nPieceStart = nTargetParticle;
	int nEffectParticle = nTargetParticle - nTargetStart;
	pParticlesGeometry->Start( pCurrentEffect, nEffectParticle, currentEffectBV, *pLMAlloc, dwCurrentParticleColor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::AddParticles( IParticles *pParticles, bool bIsLit, const SBound &bv, 
	IReportParticlesGeometry *pStore ) 
{
	pReportParticles = pStore;
	currentEffectBV = bv;
	pCurrentEffect = pParticles->GetEffect();
	if ( bIsLit )
	{
		dwCurrentParticleColor = dwLitColor;
		pLMAlloc = &litParticlesAlloc;
	}
	else
	{
		dwCurrentParticleColor = dwNormalColor;
		pLMAlloc = &kernel;
	}
	if ( !IsValid(pParticlesGeometry) )
		AllocParticlesWriteBuffer();
	else if ( pParticlesGeometry->GetPartsNum() == PF_MAX_PARTS_PER_COMBINER )
	{
		pParticlesGeometry->FreeWriteBuffer();
		AllocParticlesWriteBuffer();
	}
	nTargetStart = nTargetParticle;
	StartParticlesPiece();
	
	pCurrentEffect->AddParticles( this );
	
	FinishParticlesPiece();
	nTotalParticles += nTargetParticle - nTargetStart;
	if ( bIsLit )
		nLitParticles += nTargetParticle - nTargetStart;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::AddParticleOverflow( const CVec3 vPos[4], DWORD dwColor, const STransparentTexturePlace &tPlace,
	float fDepth )
{
	// buffer is full, start a new one
	if ( bTnLMode )
		pTnLParticlesGeometry->RealAddParticle( vPos, dwColor, tPlace, fDepth );
	else
		pShaderParticlesGeometry->RealAddParticle( vPos, dwColor, tPlace, fDepth );
	FinishParticlesPiece();
	pParticlesGeometry->FreeWriteBuffer();

	int nEffectParticle = nTargetParticle - nTargetStart;
	AllocParticlesWriteBuffer();
	nTargetStart = nTargetParticle - nEffectParticle;

	StartParticlesPiece();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::AddParticle( const CVec3 vPos[4], DWORD dwColor, const STransparentTexturePlace &tPlace,
	float fDepth )
{
	pWriteParticles->particleDepths.push_back( fDepth );
	++nTargetParticle;
	if ( nTargetParticle < N_PARTICLES_BUFFER_SIZE / 4 )
	{
		if ( bTnLMode )
			pTnLParticlesGeometry->RealAddParticle( vPos, dwColor, tPlace, fDepth ); // to enable tail call optimization
		else
			pShaderParticlesGeometry->RealAddParticle( vPos, dwColor, tPlace, fDepth ); // to enable tail call optimization
	}
	else
		AddParticleOverflow( vPos, dwColor, tPlace, fDepth );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SObjectSort
{
	ITransparent *p;
	float fDepth;
	SObjectSort( ITransparent *_p, IParticleOutput *pInfo ) : p(_p)
	{
		fDepth = p->GetDepth( pInfo );
	}
};
static bool CmpObjects( const SObjectSort &a, const SObjectSort &b )
{
	return a.fDepth < b.fDepth;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool CmpFragments( const STransparentInfo &f1, const STransparentInfo &f2 ) 
{
	return f1.fDepth < f2.fDepth;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ERadixFetch
{
	RF_NORMAL,
	RF_BACK,
	RF_FLOAT
};
static void RadixSortStage( int shift, const vector<float> &depths, vector<int> *pRes, ERadixFetch fetch )
{
	int lists[256];
	memset( lists, -1, sizeof(lists) );
	int nCount = depths.size();
	vector<int> temp( nCount );
	for ( int k = 0; k < nCount; ++k )
	{
		int nIdx = (*pRes)[k];
		int n = ( (*(int*)(&depths[ nIdx ])) >> shift ) & 0xff; // determine pile to which this element is to be stored
		temp[nIdx] = lists[n];
		lists[n] = nIdx;
	}
	int *pOut = &(*pRes)[0], *pFinal = pOut + nCount - 1;
	switch ( fetch )
	{
		case RF_NORMAL:
			for ( int i = 0; i < 256; i++ )
			{
				for ( int nIdx = lists[i]; nIdx >= 0; nIdx = temp[nIdx] )
					*pOut++ = nIdx;
			}
			ASSERT( pOut == pFinal + 1 );
			break;
		case RF_BACK:
			for ( int i = 255; i >= 0; i-- )
			{
				for ( int nIdx = lists[i]; nIdx >= 0; nIdx = temp[nIdx] )
					*pOut++ = nIdx;
			}
			ASSERT( pOut == pFinal + 1 );
			break;
		case RF_FLOAT:
			for ( int i = 255; i >= 128; i-- )
			{
				for ( int nIdx = lists[i]; nIdx >= 0; nIdx = temp[nIdx] )
					*pOut++ = nIdx;
			}
			for ( int i = 127; i >= 0; i-- )
			{
				for ( int nIdx = lists[i]; nIdx >= 0; nIdx = temp[nIdx] )
					*pFinal-- = nIdx;
			}
			break;
	}
}
// lowest first
static void DoRadixSort( const vector<float> &src, vector<int> *pRes )
{
	pRes->resize( src.size() );
	if ( src.empty() )
		return;
	for ( int k = 0; k < pRes->size(); ++k )
		(*pRes)[k] = k;
	//RadixSortStage( 0, pSort, pSort1, nElem, 1 );
	RadixSortStage( 8, src, pRes, RF_BACK );
	RadixSortStage( 16, src, pRes, RF_NORMAL );
	RadixSortStage( 24, src, pRes, RF_FLOAT );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPreciseTranspRender
{
	vector<const STransparentInfo*> infos;
	vector<float> depths;
	vector<int> sourcePtrs;
	vector<STriangle> tris;
	int nCurrentSrc, nCurrentBase;
	NGfx::CRenderContext *pRC;

	void Flush()
	{
		if ( nCurrentSrc == -1 )
			return;
		NGfx::STriangleList triList;
		triList.pTri = &tris[0];
		triList.nTris = tris.size();
		triList.nBaseIndex = infos[nCurrentSrc]->nOffset;
		CDGPtr<IVBCombiner> pVertices( infos[nCurrentSrc]->pGeom );
		pVertices.Refresh();
		pRC->AddPrimitive( pVertices->GetValue(), triList );
		tris.resize( 0 );
		nCurrentSrc = -1;
	}
public:
	void Render( NGfx::CRenderContext *_pRC, const list<STransparentInfo> &fragments )
	{
		pRC = _pRC;
		for ( list<STransparentInfo>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
		{
			const STransparentInfo &info = *i;
			int nInfoIdx = infos.size() << 20;
			infos.push_back( &info );
			if ( 0 )//IsObject IsValid( info.pTris ) )
			{
				depths.push_back( info.fDepth );
				sourcePtrs.push_back( nInfoIdx );
			}
			else if ( IsValid( info.pGeom ) )
			{
				for ( int k = 0; k < info.particleDepths.size(); ++k )
				{
					depths.push_back( info.particleDepths[k] );
					sourcePtrs.push_back( nInfoIdx | (k+1) );
				}
			}
		}
		vector<int> sorted;
		DoRadixSort( depths, &sorted );
		nCurrentSrc = -1;
		for ( int k = 0; k < sorted.size(); ++k )
		{
			int nSrcPtr = sourcePtrs[ sorted[k] ];
			int nParticle = nSrcPtr & 0xfffff;
			int nSrc = nSrcPtr >> 20;
			if ( nParticle == 0 ) // IsObject
			{
				Flush();
				//const STransparentInfo *pInfo = infos[nSrc];
				//pRC->AddPrimitive( pInfo->pGeom->GetValue(), pInfo->pTris->GetValue() );
			}
			else
			{
				if ( nSrc != nCurrentSrc )
					Flush();
				nCurrentSrc = nSrc;
				int nBase = ( nParticle - 1 ) * 4;
				tris.push_back( STriangle( nBase, nBase + 1, nBase + 2 ) );
				tris.push_back( STriangle( nBase, nBase + 2, nBase + 3 ) );
			}
		}
		Flush();
		pRC->Flush();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::RealRender( NGfx::CRenderContext *pRC, NGfx::CTexture *pLight )
{
	NGfx::CRenderContext &rc = *pRC;
	rc.Use();
	rc.SetAlphaCombine( NGfx::COMBINE_SMART_ALPHA );
	rc.SetDepth( NGfx::DEPTH_TESTONLY );
	rc.SetStencil( NGfx::STENCIL_NONE );
	if ( bTnLMode )
	{
		NGfx::SEffTnLParticles eff;
		rc.SetEffect( &eff );
	}
	else
	{
		NGfx::SEffTransparentParticles eff;
		eff.pLight = pLight;
		rc.SetEffect( &eff );
	}
	if ( NGfx::CanStreamGeometry() )
	{
		CPreciseTranspRender pr;
		pr.Render( pRC, fragments );
	}
	else
	{
		fragments.sort( CmpFragments );
		for ( list<STransparentInfo>::iterator i = fragments.begin(); i != fragments.end(); ++i )
		{
			const STransparentInfo &info = *i;
			if ( 0 ) // IsObject IsValid( info.pTris ) )
			{
				//pRC->DrawPrimitive( info.pGeom->GetValue(), info.pTris->GetValue() );
			}
			else if ( IsValid( info.pGeom ) )
			{
				int nParticles = info.particleDepths.size();
				nParticles = Min( nParticles, NGfx::N_MAX_RECTANGLES ); // CRAP
				if ( nParticles == 0 )
					continue;

				vector<int> particles( nParticles );
				for ( int i = 0; i < particles.size(); ++i )
					particles[i] = i;
				DoRadixSort( info.particleDepths, &particles );
				//sort( particles.begin(), particles.end(), SSortDepths( info.particleDepths ) );
				CObj<NGfx::CTriList> pTriList;
				{
					NGfx::CBufferLock<NGfx::S3DTriangle> tris( &pTriList, nParticles * 2 );
					//int nOffset = info.nOffset;
					for ( int k = 0; k < particles.size(); ++k )
					{
						int nBase = particles[k] * 4; //+ nOffset;
						tris[ k*2 + 0 ] = NGfx::S3DTriangle( nBase, nBase + 1, nBase + 2 );
						tris[ k*2 + 1 ] = NGfx::S3DTriangle( nBase, nBase + 2, nBase + 3 );
					}
				}
				CDGPtr<IVBCombiner> pGeom( info.pGeom );
				pGeom.Refresh();
				rc.DrawPrimitive( pGeom->GetValue(), pTriList, info.nOffset, nParticles * 4 );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransparentRenderer::Render( NGfx::CRenderContext *pRC, NGfx::CTexture *pFogLookup, 
	NGfx::CTexture *pParticleLight )
{
	FinishParticles();
	// set mode for rendering objects, without them no particles are visible behind the window
	//pRC->SetDepth( NGfx::DEPTH_TESTONLY );
	//pRC->SetStencil( NGfx::STENCIL_NONE );
	pRC->SetDepth( NGfx::DEPTH_NORMAL );
	pRC->SetStencil( NGfx::STENCIL_NONE );
	// objects
	list<SObjectSort> objects;
	for ( list< CPtr<ITransparent> >::iterator i = transparent.begin(); i != transparent.end(); ++i )
		objects.push_back( SObjectSort( *i, this ) );
	objects.sort( CmpObjects );
	for ( list<SObjectSort>::iterator i = objects.begin(); i != objects.end(); ++i )
		i->p->Render( pRC, pFogLookup );
	RealRender( pRC, pParticleLight );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
