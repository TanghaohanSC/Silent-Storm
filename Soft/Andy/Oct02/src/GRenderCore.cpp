#include "StdAfx.h"
#include "GRenderCore.h"
#include "GfxBuffers.h"
#include "Transform.h"
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSceneFragments
////////////////////////////////////////////////////////////////////////////////////////////////////
int CSceneFragments::CountTris( SRenderGeometryInfo *pGeometry, TPartFlags nParts )
{
	pGeometry->pTriLists[TLT_POSITION].Refresh();
	const vector<NGfx::STriangleList> &triLists = pGeometry->pTriLists[TLT_POSITION]->GetValue();
	int nRes = 0;
	for ( int k = 0; k < triLists.size(); ++k )
	{
		if ( nParts & (1<<k) )
			nRes += triLists[k].nTris;
	}
	nSceneTris += nRes;
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSceneFragments::AddElement( CObjectBase *pHandle, SRenderGeometryInfo *pGeometry, TPartFlags nParts, 
	IMaterial *pMaterial, NGfx::CTexture *pLightmap, const SDynamicAmbientInfo *pLM, bool bSkipPerPartTests )
{
	pGeometry->pVertices.Refresh();
	if ( nParts == 0 || pGeometry->pTriLists[TLT_POSITION]->GetValue().empty() )
		return;
	SRenderStaticInfo &staticInfo = *staticInfos.Alloc();
	staticInfo.pGeometry = pGeometry;
	staticInfo.pMaterial = pMaterial;
	staticInfo.pHandle = pHandle;
	staticInfo.pLightmap = pLightmap;
	staticInfo.pLM = pLM;

	SRenderFragmentInfo &frag = *fragments.Alloc();
	frag.pStatic = &staticInfo;
	frag.nParts = nParts;
	frag.bSkipPerPartTests = bSkipPerPartTests;
	selected.push_back( &frag );
	// count tris	
	/*staticInfo.pGeometry->pVertices.Refresh();
	staticInfo.pGeometry->pTriLists[TLT_GEOM].Refresh();
	const vector<NGfx::STriangleList> &triLists = staticInfo.pGeometry->pTriLists[TLT_GEOM]->GetValue();
	for ( int k = 0; k < triLists.size(); ++k )
	{
		if ( nParts & (1<<k) )
			nSceneTris += triLists[k].nTris;
	}*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSceneFragments::AddLitParticles( IVBCombiner *pVertices, CFuncBase<vector<NGfx::STriangleList> > *pTris, 
	int nPart, IMaterial *pMaterial, bool bSkipPerPartTests )
{
	SRenderGeometryInfo &g = *geometryInfos.Alloc();
	g.pVertices = pVertices;
	g.pTriLists[TLT_POSITION] = pTris;
	g.pTriLists[TLT_GEOM] = pTris;
	SRenderStaticInfo &staticInfo = *staticInfos.Alloc();
	staticInfo.pGeometry = &g;
	staticInfo.pMaterial = pMaterial;
	staticInfo.pHandle = 0;
	staticInfo.pLightmap = 0;
	staticInfo.pLM = 0;

	SRenderFragmentInfo &frag = *fragments.Alloc();
	frag.pStatic = &staticInfo;
	frag.nParts = 1 << nPart;
	frag.bSkipPerPartTests = bSkipPerPartTests;
	litParticles.push_back( &frag );

	staticInfo.pGeometry->pVertices.Refresh();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Filter ops
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ECullAcceptLevel
{
	FULL_GET,
	GET,
	REJECT
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static ECullAcceptLevel GetIntersectLevel( const SBound &bound, const SBound &test )
{
	CVec3 ptDif = bound.s.ptCenter - test.s.ptCenter;
	// fast outside
	float fDif2 = fabs2(ptDif);
	if ( fDif2 > sqr( bound.s.fRadius + test.s.fRadius ) )
		return REJECT;
	// fast inside, fDif + test.s.fRadius < bound.s.fRadius
	float fRDif = bound.s.fRadius - test.s.fRadius; 
	if ( fDif2 < fRDif * fabs( fRDif ) )
		return FULL_GET;
	if ( 
		fabs(ptDif.x) + test.ptHalfBox.x < bound.ptHalfBox.x && 
		fabs(ptDif.y) + test.ptHalfBox.y < bound.ptHalfBox.y && 
		fabs(ptDif.z) + test.ptHalfBox.z < bound.ptHalfBox.z )
		return FULL_GET;
	return GET;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static ECullAcceptLevel GetIntersectLevel( const SSphere &bound, const SSphere &test )
{
	CVec3 ptDif = bound.ptCenter - test.ptCenter;
	// fast outside
	float fDif2 = fabs2(ptDif);
	if ( fDif2 > sqr( bound.fRadius + test.fRadius ) )
		return REJECT;
	// fast inside, fDif + test.s.fRadius < bound.s.fRadius
	float fRDif = bound.fRadius - test.fRadius; 
	if ( fDif2 < fRDif * fabs( fRDif ) )
		return FULL_GET;
	return GET;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool DoesIntersect( const SSphere &a, const SSphere &b )
{
	CVec3 ptDif = a.ptCenter - b.ptCenter;
	return fabs2(ptDif) < sqr( a.fRadius + b.fRadius );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EFragmentsSplit FilterPartsByBV( const SSphere &bound, SRenderFragmentInfo *pAccept, const SSelectFragments &selector )
{
	SRenderFragmentInfo &f = *pAccept;
	TPartFlags accepted = 0, rejected = 0;
	IVBCombiner *pVB = f.pStatic->pGeometry->pVertices;
	const vector<SSphere> &partBVs = pVB->GetBounds();
	for ( int k = 0; k < pVB->GetPartsNum(); ++k )
	{
		if ( f.nParts & (1<<k) )
		{
			if ( DoesIntersect( bound, partBVs[k] ) )
				accepted |= 1 << k;
			else
				rejected |= 1 << k;
		}
	}
	return selector.Split( f, accepted, rejected );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SBoundIntersectFilter
////////////////////////////////////////////////////////////////////////////////////////////////////
EFragmentsSplit SBoundIntersectFilter::operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const 
{
	pF->pStatic->pGeometry->pVertices.Refresh();
	ECullAcceptLevel l = GetIntersectLevel( bv, pF->pStatic->pGeometry->pVertices->GetBound() );
	if ( l == REJECT )
		return FST_REJECT;
	if ( l == FULL_GET || pF->bSkipPerPartTests )
		return FST_ACCEPT;
	return FilterPartsByBV( bv.s, pF, selector );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SFrustrumFilter
////////////////////////////////////////////////////////////////////////////////////////////////////
EFragmentsSplit SFrustrumFilter::operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const 
{
	if ( !pTS->PushClipHint( pF->pStatic->pGeometry->pVertices->GetBound() ) )
		return FST_REJECT;
	if ( pTS->IsFullGet() || pF->bSkipPerPartTests )
	{
		pTS->PopClipHint();
		return FST_ACCEPT;
	}
	TPartFlags accepted = 0, rejected = 0;
	IVBCombiner *pVB = pF->pStatic->pGeometry->pVertices;
	const vector<SSphere> &partBVs = pVB->GetBounds();
	for ( int k = 0; k < pVB->GetPartsNum(); ++k )
	{
		if ( pF->nParts & (1<<k) )
		{
			if ( pTS->IsIn( partBVs[k] ) )
				accepted |= 1<<k;
			else
				rejected |= 1<<k;
		}
	}
	EFragmentsSplit res = selector.Split( *pF, accepted, rejected );
	pTS->PopClipHint();
	return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SSphereFilter
////////////////////////////////////////////////////////////////////////////////////////////////////
EFragmentsSplit SSphereFilter::operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const
{
	ECullAcceptLevel l = GetIntersectLevel( sph, pF->pStatic->pGeometry->pVertices->GetBound().s );
	if ( l == REJECT )
		return FST_REJECT;
	if ( l == FULL_GET || pF->bSkipPerPartTests )
		return FST_ACCEPT;
	return FilterPartsByBV( sph, pF, selector );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SSphereAndIgnoredFilter
////////////////////////////////////////////////////////////////////////////////////////////////////
EFragmentsSplit SSphereAndIgnoredFilter::operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const
{
	ECullAcceptLevel l = GetIntersectLevel( sph, pF->pStatic->pGeometry->pVertices->GetBound().s );
	if ( l == REJECT )
		return FST_REJECT;
	CFilterPartsHash::const_iterator ignored = ignoreList.find( pF->pStatic->pHandle );
	if ( ( l == FULL_GET || pF->bSkipPerPartTests ) && ignored == ignoreList.end() )
		return FST_ACCEPT;
	TPartFlags accepted = 0, rejected = 0;
	IVBCombiner *pVB = pF->pStatic->pGeometry->pVertices;
	const vector<SSphere> &partBVs = pVB->GetBounds();
	if ( ignored == ignoreList.end() )
	{
		for ( int k = 0; k < pVB->GetPartsNum(); ++k )
		{
			if ( pF->nParts & (1<<k) )
			{
				if ( DoesIntersect( sph, partBVs[k] ) )
					accepted |= 1 << k;
				else
					rejected |= 1 << k;
			}
		}
	}
	else
	{
		TPartFlags check = pF->nParts & (~ignored->second);
		rejected = pF->nParts & ignored->second;
		if ( check )
		{
			for ( int k = 0; k < pVB->GetPartsNum(); ++k )
			{
				if ( check & (1<<k) )
				{
					if ( DoesIntersect( sph, partBVs[k] ) )
						accepted |= 1 << k;
					else
						rejected |= 1 << k;
				}
			}
		}
	}
	return selector.Split( *pF, accepted, rejected );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AddSingleOp( CRenderCmdList *pRes, const vector<SRenderFragmentInfo*> &fragments, 
	ERenderOperation op,
	CRenderCmdList::UParameter _p1,
	CRenderCmdList::UParameter _p2,
	CRenderCmdList::UParameter _p3,
	int nStencilOp )
{
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &f = **i;
		SOpGenContext fi( &pRes->ops, &f );
		f.pStatic->pMaterial->AddATOperations( &fi );
		if ( fi.HasAddedOps() )//bAlphaTest )
			fi.AddOperation( op, 100, nStencilOp|DPM_EQUAL, 0, _p1, _p2, _p3 );
		else
			fi.AddOperation( op, 100, nStencilOp, 0, _p1, _p2, _p3 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MakeSingleOp( CRenderCmdList *pRes, CSceneFragments &src, bool bTakeLitParticles, ERenderOperation op,
	CRenderCmdList::UParameter _p1,
	CRenderCmdList::UParameter _p2,
	CRenderCmdList::UParameter _p3,
	int nStencilOp )
{
	AddSingleOp( pRes, src.GetSelected(), op, _p1, _p2, _p3, nStencilOp );
	if ( bTakeLitParticles )
		AddSingleOp( pRes, src.GetLitParticles(), op, _p1, _p2, _p3, nStencilOp );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddFinalOps( CRenderCmdList *pRes, CSceneFragments &src, ESceneRenderMode rm, 
	ERenderOperation op, CRenderCmdList::UParameter _p1 )
{
	//fi.pRes = this;
	const vector<SRenderFragmentInfo*> &fragments = src.GetSelected();
	for ( vector<SRenderFragmentInfo*>::const_iterator i = fragments.begin(); i != fragments.end(); ++i )
	{
		SRenderFragmentInfo &f = **i;
		SOpGenContext fi( &pRes->ops, &f );
		if ( op != RO_NOP )
			fi.AddOperation( op, 50, ABM_ALPHA_FOG|DPM_EQUAL, 0, _p1 );
		//fi.pPre = 0;
		//fi.pPerLight = 0;
		//fi.pPost = &res.finalPass;
		f.pStatic->pMaterial->AddOperations( &fi, rm );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGScene;
BASIC_REGISTER_CLASS( ILight )
