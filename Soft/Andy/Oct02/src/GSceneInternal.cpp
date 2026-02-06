#include "StdAfx.h"
#include "Gfx.h"
#include "GfxRender.h"
#include "GfxBuffers.h"
#include "GfxEffects.h"
#include "GfxUtils.h"
#include "GPixelFormat.h"
#include "DG.h"
#include "GScene.h"
#include "Transform.h"
#include "GSceneInternal.h"
#include "GMaterial.h"
#include "GClipper.h"
#include "GCombiner.h"
#include "GRenderCore.h"
#include "GRenderLight.h"
#include "GCombiner.h"
#include "..\Misc\HPTimer.h"
#include "..\Misc\2DArray.h"
#include "GfxUtils.h"
#include "GRenderFactor.h"
#include "GParticleInfo.h"
#include "Bound.h"
#include "SuperCollider.h" // for TraceStatic(...)
#include "GShadowMap.h" // CRAP

#include "..\Misc\Commands.h" // for lm command
#include "..\Misc\LogStream.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
// размер узла octree на котором создаются кеши lightmap`ов
//const float F_LIGHTMAP_OCTREE_NODE_SIZE = 30;
const float F_SELECTION_STEP = 0.03f;
//! граничное значение для количества треугольников в узле с которого начинается отдельные проверки
//! каждого фрагмента
const int N_TRIS_PER_NODE_FOR_PER_PART_CLIP = 500;
const int N_PARTICLES_TO_START_CLIP = 20;
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
static SRenderStats lastFrameStats;
static bool bWireframe;
static ELMCalcType crapRecalcLightmaps = LM_CALC_NONE;
static bool bShow2DTextureCache = false, bShowTranspTextureCache = false, bShowParticleLMCache = false;
enum EShowLinearCache
{
	SLC_NONE,
	SLC_STATIC,
	SLC_DYNAMIC
};
static EShowLinearCache showLinearCache = SLC_NONE;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGetTranspCache : public CPtrFuncBase<NGfx::CTexture>
{
	OBJECT_NOCOPY_METHODS(CGetTranspCache);
	void Recalc() { pValue = NGfx::GetTransparentTextureCache(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLightGroup
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLightGroup: public CObjectBase
{
	OBJECT_NOCOPY_METHODS(CLightGroup);
	ZDATA
	CPtr<CGScene> pGame;
	int nGroup;
public:
	SDynamicAmbientInfo ambientData;
	CVec3 vPrevPos;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pGame); f.Add(3,&nGroup); return 0; }
	CLightGroup() {}
	CLightGroup( CGScene *p, int _n ) : pGame(p), nGroup(_n), vPrevPos(-1e6f, -1e6f, -1e6f) {}
	~CLightGroup()
	{
		if ( IsValid( pGame ) )
			pGame->FreeLightGroup( nGroup );
	}
	int GetGroup() const { return nGroup; }
};
int GetGroup( CLightGroup *p ) { return p->GetGroup(); }
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDynamicLightCache
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDynamicLightCache::CheckStatic()
{
	bool bRes = pStaticChanged.Refresh();
	if ( bRes )
		data.clear();
	if ( data.size() > 500 )
		data.clear();
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SDynamicAmbientInfo& CDynamicLightCache::Calc( const SBound &bv, CGScene *pScene )
{
	CHash::iterator i = data.find( bv.s.ptCenter );
	if ( i == data.end() )
	{
		SDynamicAmbientInfo *pRes = &data[ bv.s.ptCenter ];
		pScene->TraceDynamicAmbient( pRes, bv );
		return *pRes;
	}
	else
		return i->second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// unpretentious render of simple sets
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AttachPart( CNonePart *pRes, CVolumeNode *pNode, IMaterial *pMat,
	const SGroupInfo &_ginfo, CVersioningBase *pStaticTracker, CLightmapTracker *plmTracker, bool bIsDynamic,
	bool bAnimationOnly )
{
	bool bIsLightmapped = _ginfo.nLightGroup == 0 && plmTracker && pMat->GetType() != IMaterial::MT_OCCLUDER;
	/*if ( _ginfo.nLightGroup == 0 && plmTracker && pMat->GetType() != IMaterial::MT_OCCLUDER )
	{
		//CVolumeNode *pLMNode = pNode;
		if ( !IsValid( pRes->pLightmapInfo ) )
			pRes->pLightmapInfo = plmTracker->AllocRegion( pRes );
		// has to attach a lightmap
		//while ( pLMNode->GetSize() < F_LIGHTMAP_OCTREE_NODE_SIZE )
		//	pLMNode = pLMNode->GetUpLink();
		//if ( !IsValid( pLMNode->pLightmap ) )
		//{
		//	SSphere bound;
		//	pLMNode->GetBound( &bound );
		//	pLMNode->pLightmap = plmTracker->CreateLightmapNode( bound );
		//}
		//if ( !IsValid( pRes->pLightmapInfo ) || pRes->pLightmapInfo->GetCache()->GetLMVolumeNode() != pLMNode->pLightmap )
		//	pRes->pLightmapInfo = pLMNode->pLightmap->AllocRegion( pRes );
	}*/
	SCombinedKey key;
	//if ( IsValid( pRes->pLightmapInfo ) )
	//	key = SCombinedKey( pMat, _ginfo, pRes->pLightmapInfo->GetCache() );
	//else
	key = SCombinedKey( pMat, _ginfo );
	CCombinedPart *pCPart;
	if ( bIsDynamic )
		pCPart = pNode->dynamicParts.GetCombinerPartForAdd( key, 0, bIsLightmapped );
	else
		pCPart = pNode->staticParts.GetCombinerPartForAdd( key, pStaticTracker, bIsLightmapped );
	pRes->pOwner = pCPart;
	if ( bAnimationOnly )
		pRes->SetCombiner( pCPart->GetCombiner(), false, true );
	else
		pRes->SetCombiner( pCPart->GetCombiner(), true, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CNonePart
////////////////////////////////////////////////////////////////////////////////////////////////////
CTRect<int> CNonePart::zeroRect;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGenericDynamicPart
////////////////////////////////////////////////////////////////////////////////////////////////////
CGenericDynamicPart::CGenericDynamicPart( CPtrFuncBase<CObjectInfo> *pData, IMaterial *_pMaterial, const SGroupInfo &_gInfo )
	: CNonePart( pData ), pMaterial(_pMaterial), groupInfo(_gInfo), nStillCounter(0)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDynamicPart
////////////////////////////////////////////////////////////////////////////////////////////////////
CDynamicPart::CDynamicPart( CPtrFuncBase<CObjectInfo> *pData, CFuncBase<SFBTransform> *pPos, 
	IMaterial *_pMaterial, const SGroupInfo &_gInfo )
	: CGenericDynamicPart( pData, _pMaterial, _gInfo ), pTransform(pPos)
{
	RefreshObjectInfo();
	GetObjectInfo()->CalcBound( &bound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDynamicPart::Update( CVolumeNode *pVolume, CVersioningBase *pStaticTracker, CLightmapTracker *plmTracker )
{
	bool bObjectInfoChanged = RefreshObjectInfo();
	if ( bObjectInfoChanged )
		GetObjectInfo()->CalcBound( &bound );
	if ( pTransform.Refresh() | bObjectInfoChanged )
	{
		nStillCounter = 0;
		CVolumeNode *pNode = pVolume->SelectNode( pTransform, bound );
		ASSERT( pNode );
		AttachPart( this, pNode, pMaterial, groupInfo, pStaticTracker, plmTracker, true, true );
	}
	else
	{
		++nStillCounter;
		if ( nStillCounter == 3 && groupInfo.nLightGroup == 0 )
		{
			CVolumeNode *pNode = pVolume->SelectNode( pTransform, bound );
			ASSERT( pNode );
			AttachPart( this, pNode, pMaterial, groupInfo, pStaticTracker, plmTracker, false, false );
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAnimatedPart
////////////////////////////////////////////////////////////////////////////////////////////////////
CAnimatedPart::CAnimatedPart( CPtrFuncBase<CObjectInfo> *pData, CFuncBase< vector<SHMatrix> > *_pAnim, 
	IMaterial *_pMaterial, const SGroupInfo &_gInfo )
	: CGenericDynamicPart( pData, _pMaterial, _gInfo ), pAnimation(_pAnim)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAnimatedPart::EstimateBound( SBound *pRes )
{
	// estimate bound
	SBoundCalcer bc;
	const vector<SHMatrix> &anim = pAnimation->GetValue();
	for ( int i=0; i<anim.size(); ++i )
		bc.Add( anim[i].GetTranslation(), 0 );
	bc.Make( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_MAX_DISTANCE_TO_BONE = 0.5f;
bool CAnimatedPart::Update( CVolumeNode *pVolume, CVersioningBase *pStaticTracker, CLightmapTracker *plmTracker )
{
	bool bObjectInfoChanged = RefreshObjectInfo();
	if ( pAnimation.Refresh() | bObjectInfoChanged )
	{
		SBound bv;
		EstimateBound( &bv );
		// update position
		nStillCounter = 0;
		CVolumeNode *pNode = pVolume->GetNode( bv.s.ptCenter, bv.s.fRadius );
		ASSERT( pNode );
		AttachPart( this, pNode, pMaterial, groupInfo, pStaticTracker, plmTracker, true, true );
	}
	else
	{
		++nStillCounter;
		if ( nStillCounter == 3 && groupInfo.nLightGroup == 0 )
		{
			SBound bv;
			EstimateBound( &bv );
			CVolumeNode *pNode = pVolume->GetNode( bv.s.ptCenter, bv.s.fRadius );
			ASSERT( pNode );
			AttachPart( this, pNode, pMaterial, groupInfo, pStaticTracker, plmTracker, false, false );
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDynamicGeometryPart
////////////////////////////////////////////////////////////////////////////////////////////////////
CDynamicGeometryPart::CDynamicGeometryPart( CPtrFuncBase<CObjectInfo> *pData, CFuncBase<SBound> *_pBound, 
	IMaterial *_pMaterial, const SGroupInfo &_gInfo )
	: CGenericDynamicPart( pData, _pMaterial, _gInfo ), pBound(_pBound)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDynamicGeometryPart::Update( CVolumeNode *pVolume, CVersioningBase *pStaticTracker, CLightmapTracker *plmTracker )
{
	bool bObjectInfoChanged = RefreshObjectInfo();
	pBound.Refresh();
	if ( bObjectInfoChanged )
	{
		// update position
		nStillCounter = 0;
		const SBound &bv = pBound->GetValue();
		CVolumeNode *pNode = pVolume->GetNode( bv.s.ptCenter, bv.s.fRadius );
		ASSERT( pNode );
		AttachPart( this, pNode, pMaterial, groupInfo, pStaticTracker, plmTracker, true, false );
	}
	else
	{
		++nStillCounter;
		if ( nStillCounter == 3 && groupInfo.nLightGroup == 0 )
		{
			const SBound &bv = pBound->GetValue();
			CVolumeNode *pNode = pVolume->GetNode( bv.s.ptCenter, bv.s.fRadius );
			ASSERT( pNode );
			AttachPart( this, pNode, pMaterial, groupInfo, pStaticTracker, plmTracker, false, false );
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//	CParticles
////////////////////////////////////////////////////////////////////////////////////////////////////
CParticleEffect* CParticles::GetEffect()
{
	pParticles.Refresh();
	return pParticles->GetValue();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CParticles::Update( CVolumeNode *pVolume )
{
	if ( pPlacement.Refresh() )
	{
		TransformBound( &transformedBound, bound, pPlacement->GetValue().forward );
		CVolumeNode *pNewNode = pVolume->SelectNode( pPlacement, bound );
		if ( pNode != pNewNode )
		{
			pNewNode->particles.push_back( this );
			if ( pNode )
				pNode->particles.remove( this );
			pNode = pNewNode;
		}
	}
	pParticles.Refresh();
	return !pParticles->GetValue()->bEnd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSelection
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSelection::Initialize( CObjectBase *pObject, const CVec4 &_vColor )
{
	vColor = _vColor;
	if ( CDynamicCast<CNonePart> pPart(pObject) )
	{
		pTarget = pPart;
		if ( NGfx::IsTnLDevice() )
		{
			pAnimation = new CVersioningBase;
			pCombo = new CAutomaticCombiner;
			pCombo->AddPart( pTarget );
			pOffsetVertices = new CVBCombiner( pCombo, LT_TNL_SELECTION, CT_DYNAMIC, pAnimation );
			pTriList = new CIBCombiner( pCombo, LT_NONE, IBTT_VERTICES );
			pOffsetVertices->SetOffset( F_SELECTION_STEP );
		}
		return true;
	}
	ASSERT(0);
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSelection::Render( CTransformStack *pTS, NGfx::CRenderContext *pRC, bool bOffset )
{
	if ( !IsValid( pTarget ) || !IsValid( pTarget->pOwner ) )
		return;
	SRenderGeometryInfo *pGeom = pTarget->pOwner->GetGeometryInfo();
	// skip animating not visible selection
	// requires that selection was rendered after normal scene without marking new DG frame
	if ( !pGeom->pVertices->WasRefreshed() )
		return;
	const vector<CPtr<IPart> > &parts = pTarget->pOwner->GetCombiner()->GetValue();
	int nIdx = -1;
	for ( int k = 0; k < parts.size(); ++k )
		if ( parts[k].GetPtr() == pTarget.GetPtr() )
			nIdx = k;
	if ( nIdx < 0 )
		return;
	if ( !pTS->IsIn( pGeom->pVertices->GetBounds()[ nIdx ] ) )
		return;
	if ( bOffset && NGfx::IsTnLDevice() )
	{
		pAnimation->Updated();
		pOffsetVertices.Refresh();
		pTriList.Refresh();
		pRC->AddPrimitive( pOffsetVertices->GetValue(), pTriList->GetValue()[0] );
	}
	else if ( pGeom->pTriLists[TLT_GEOM] )
	{
		pGeom->pTriLists[TLT_GEOM].Refresh();
		pRC->AddPrimitive( pGeom->pVertices->GetValue(), pGeom->pTriLists[TLT_GEOM]->GetValue()[nIdx] );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSelection::Update( IGScene *pScene )
{
	return IsValid( pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCombinedPart
////////////////////////////////////////////////////////////////////////////////////////////////////
CCombinedPart::CCombinedPart( const SCombinedKey &k, CVersioningBase *pTracker, bool bIsLightmapped )
	: nIgnoreMark(0), pMaterial( k.pMaterial ), groupInfo(k.groupInfo), pCombiner( new CPerMaterialCombiner(pTracker ) )
{
	SRenderGeometryInfo &gi = geometryInfo;
	if ( pMaterial->GetType() == IMaterial::MT_OCCLUDER )
	{
		gi.pVertices = new CVBCombiner( pCombiner, LT_POSITION, pTracker ? CT_STATIC : CT_DYNAMIC, pCombiner->GetAnimation() );
		gi.pTriLists[TLT_POSITION] = new CIBCombiner( pCombiner, LT_POSITION, IBTT_POSITIONS );
	}
	else if ( bIsLightmapped )
	{
		pLMMapping = new CVersioningBase;
		CVBCombiner *pVB = new CVBCombiner( pCombiner, LT_NORMAL, pTracker ? CT_STATIC : CT_DYNAMIC, pCombiner->GetAnimation() );
		pVB->SetLMMappingTracker( pLMMapping );
		gi.pVertices = pVB;
		gi.pTriLists[TLT_POSITION] = new CIBCombiner( pCombiner, LT_NORMAL, IBTT_POSITIONS );
		gi.pTriLists[TLT_GEOM] = new CIBCombiner( pCombiner, LT_NORMAL, IBTT_VERTICES );
		gi.pTriLists[TLT_LM] = new CIBCombiner( pCombiner, LT_NORMAL, IBTT_LM );
		gi.pTriLists[TLT_LMCALC] = new CIBCombiner( pCombiner, LT_NORMAL, IBTT_LMCALC );
	}
	else
	{
		gi.pVertices = new CVBCombiner( 
			pCombiner, 
			NGfx::IsTnLDevice() ? LT_TNL : LT_NONE, 
			pTracker ? CT_STATIC : CT_DYNAMIC, 
			pCombiner->GetAnimation() );
		gi.pTriLists[TLT_POSITION] = new CIBCombiner( pCombiner, LT_NONE, IBTT_POSITIONS );
		gi.pTriLists[TLT_GEOM] = new CIBCombiner( pCombiner, LT_NONE, IBTT_VERTICES );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCombinedPart::AddForLightmapRender( CSceneFragments *pRes )
{
	ASSERT( pMaterial->GetType() != IMaterial::MT_OCCLUDER );
	pRes->CountTris( &geometryInfo, PF_ALL_PARTS );
	pRes->AddElement( this, &geometryInfo, PF_ALL_PARTS, pMaterial, 0, 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCombinedPart::SetIgnored( int _nIgnoreMark, TPartFlags nParts )
{ 
	pCombiner.Refresh();
	nIgnoreMark = _nIgnoreMark;
	ignoredParts = nParts;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTransElementWrapper
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransElementWrapper::Render( NGfx::CRenderContext *pRC, NGfx::CTexture *pFog )
{
	pElement->GetMaterial()->ExecTransparent( pElement->GetGeometryInfo(), pRC, pFog );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CTransElementWrapper::GetDepth( IParticleOutput *pInfo )
{
	return ptCenter * pInfo->GetDepth();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVolumeNode
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVolumeNode::IsEmpty()
{
	staticParts.Walk();
	dynamicParts.Walk();
	EraseInvalidRefs( &particles );
	return staticParts.IsEmpty() && dynamicParts.IsEmpty() && particles.empty();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPolyline
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPolyline::Render( NGfx::CRenderContext *pRC )
{
	pGeometry.Refresh();
	pRC->DrawLineStrip( pGeometry->GetValue() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CPolyline::operator&( CStructureSaver &f )
{
	f.Add( 1, &pGeometry );
	f.Add( 2, &color );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFakeParticleLMTexture
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_FAKE_LM_SIZEX = 2;
const int N_FAKE_LM_SIZEY = 1;
void CFakeParticleLMTexture::Recalc()
{
	pValue = NGfx::MakeTexture( N_FAKE_LM_SIZEX, N_FAKE_LM_SIZEY, 1, NGfx::SPixel8888::ID, NGfx::REGULAR, NGfx::CLAMP );
	NGfx::CTextureLock<NGfx::SPixel8888> lock( pValue, 0, NGfx::INPLACE );
	dwNormalColor = NGfx::GetDWORDColor( CVec4( 0.25f, 0.25f, 0.25f, 1 ) );
	dwParticleColor = NGfx::GetDWORDColor( CVec4( ( vAmbient + pColor->GetValue() ), 1 ) );
	lock[0][0].color = dwNormalColor;
	lock[0][1].color = dwParticleColor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGScene
////////////////////////////////////////////////////////////////////////////////////////////////////
CGScene::CGScene(): holdMask(0,0), 
	pStaticTracker( new CVersioningBase ), dynamicLightCache(pStaticTracker), nFrameCounter(100)
{
	bLightmapsEnable = !NGfx::IsTnLDevice();
	pVolume = new CVolumeNode;
	pVolume->SetSize( CVec3( -128, -128, -128 ), 1024 );
#ifdef _DEBUG
	renderMode = RM_FASTEST;
#else
	renderMode = RM_BEST_GF3;
#endif
	if ( NGfx::IsTnLDevice() )
		renderMode = RM_TNL;
	pCamera = new CCVec3;
	nSlowVolumeWalk = 30;
	pAmbient = new CCVec3;
	pAmbient->Set( CVec3( 0.25f, 0.25f, 0.25f ) );
	//pAmbient->AddLight( new CAmbientLight( pAmbientColor, 0 ) );
	//AddLightGroup( pAmbient );
	nCurrentIgnoreMark = 1;
	pIgnoreStaticTrack = pStaticTracker;
	lightGroups.push_back( 0 );
	pTransparentMaterial = CreateMaterial( CVec3(1,1,1), new CGetTranspCache, 0, 0, CVec3(0,0,0), 0, 0, 0, 0, 0, MA_ALPHA_TEST );
	pFakeParticleLM = new CFakeParticleLMTexture;
	pFakeParticleLM->SetAmbient( pAmbient->GetValue() );
	pFakeParticleLM->SetColor( new CCVec3( CVec3(0,0,0) ) );
	nWantFirstLM = 1;
	bLightStateCalced = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::SetLightmaps( bool bEnable )
{ 
	if ( !NGfx::IsTnLDevice() )
		bLightmapsEnable = bEnable; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CLightmapTracker* CGScene::GetLMTracker()
{
	if ( !bLightmapsEnable )
		return 0;
	if ( !pLMTracker )
		pLMTracker = new CLightmapTracker;
	return pLMTracker;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CLightGroup* CGScene::CreateLightGroup()
{
	int n;
	if ( !freeLightGroups.empty() )
	{
		n = freeLightGroups.back();
		freeLightGroups.resize( freeLightGroups.size() - 1 );
	}
	else
	{
		n = lightGroups.size();
		lightGroups.resize( n + 1 );
	}
	CLightGroup *pRes = new CLightGroup( this, n );
	lightGroups[n] = pRes;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// only positive collisions qualify, closest is searched for
static bool Collide( const SRayInfo &r, vector<CVec3> points, const vector<STriangle> &tris, float *pfT, CVec3 *pNormal )
{
	const CVec3 &vRayOrigin = r.vOrigin;
	const CVec3 &vRayDir = r.vDir;
	NCollider::SSegment ray( vRayOrigin, vRayOrigin + vRayDir );
	bool bRes = false;
	for ( int k = 0; k < tris.size(); ++k )
	{
		const STriangle &t = tris[k];
		NCollider::SSegment seg1( points[t.i3], points[t.i1] );//points[nPrev], points[ indices[i] ] );
		if ( NCollider::SegmentDotProduct( ray, seg1 ) < 0 )
			continue;
		NCollider::SSegment seg2( points[t.i1], points[t.i2] );//points[nPrev], points[ indices[i] ] );
		if ( NCollider::SegmentDotProduct( ray, seg2 ) < 0 )
			continue;
		NCollider::SSegment seg3( points[t.i2], points[t.i3] );//points[nPrev], points[ indices[i] ] );
		if ( NCollider::SegmentDotProduct( ray, seg3 ) < 0 )
			continue;
		// collides - calc exact place
		CVec3 vNormal;
		vNormal = ( points[t.i3] - points[t.i2] ) ^ ( points[t.i1] - points[t.i2] );
		if ( fabs2( vNormal ) == 0 )
		{
			ASSERT(0);
			continue;
		}
		Normalize( &vNormal );

		float fT = ( ( points[ t.i1 ] - vRayOrigin ) * vNormal ) / ( vRayDir * vNormal );
		if ( fT > 0 && fT < *pfT )
		{
			*pfT = fT;
			//trans.backward.RotateVectorTransposed( pNormal, vNormal );
			*pNormal = vNormal;
			//Normalize( pNormal );
			bRes = true;
		}
	}
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool DoesIntersect( const SSphere &s, const SRayInfo &r )
{
	CVec3 vToCenter( s.ptCenter - r.vOrigin );
	float fDist = r.vDirOrt * vToCenter;
	float fRo = ( vToCenter * vToCenter ) - sqr( fDist );
	if ( fRo > sqr( s.fRadius ) )
		return false;
	float fSide = sqrt( sqr( s.fRadius ) - fRo );
	if ( fDist - fSide > r.fLength )
		return false;
	if ( fDist + fSide < 0 )
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGScene::TraceStatic( CVolumeNode *pNode, const SRayInfo &r, float *pfT, CVec3 *pNormal, CVec3 *pColor )
{
	if ( !IsValid( pNode ) )
		return false;
	// could be optimized by rejecting volume node by distance
	SSphere nodeBound;
	pNode->GetBound( &nodeBound );
	if ( !DoesIntersect( nodeBound, r ) )
		return false;
	// CRAP - account lit particles in trace
	bool bRes = false;
	for ( list<CPtr<CCombinedPart> >::const_iterator i = pNode->staticParts.elements.begin(); i != pNode->staticParts.elements.end(); ++i )
	{
		CCombinedPart *pCPart = *i;
		IVBCombiner *pVB = pCPart->GetVBCombiner();
		if ( !DoesIntersect( pVB->GetBound().s, r ) )
			continue;
		CDGPtr<CFuncBase<vector< CPtr<IPart> > > > pC = pCPart->GetCombiner();
		pC.Refresh();
		const vector< CPtr<IPart> > &parts = pC->GetValue();
		const vector<SSphere> &partBVs = pVB->GetBounds();
		for ( int k = 0; k < parts.size(); ++k )
		{
			if ( !DoesIntersect( partBVs[k], r ) )
				continue;
			IPart *pPart = parts[k];
			vector<CVec3> points;
			vector<STriangle> tris;
			TransformPart( pPart, &points, &tris );
			if ( Collide( r, points, tris, pfT, pNormal ) )
			{
				*pColor = pCPart->GetMaterial()->GetAverageColor();
				bRes = true;
			}
		}
	}
	for ( int k = 0; k < 8; ++k )
		bRes |= TraceStatic( pNode->GetNode( k ), r, pfT, pNormal, pColor );
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGScene::TraceScene( const CRay &r, float *pfT, CVec3 *pNormal, CVec3 *pColor )
{
	*pfT = 1;
	SRayInfo rayInfo( r );
	return TraceStatic( pVolume, rayInfo, pfT, pNormal, pColor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool CheckMaterial( IMaterial *pMat )
{
	ASSERT( IsValid( pMat ) );
	if ( !IsValid( pMat ) )
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool CheckOI( CPtrFuncBase<CObjectInfo> *pInfo, SBound *pBV )
{
	CDGPtr<CPtrFuncBase<CObjectInfo> > pOI( pInfo );
	pOI.Refresh();
	if ( !IsValid( pOI->GetValue() ) || pOI->GetValue()->IsEmpty() )
	{
		pOI.Extract();
		return false;
	}
	pOI->GetValue()->CalcBound( pBV );
	pOI.Extract();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void PlaceToOctree( CNonePart *pPart, CVolumeNode *pRoot, const CVec3 &vPos, float fR, 
	IMaterial *pMat, const SGroupInfo &_ginfo, CVersioningBase *pStaticTracker, CLightmapTracker *plmTracker )
{
	CVolumeNode *pNode = pRoot->GetNode( vPos, fR );
	ASSERT( pNode );
	pPart->RefreshObjectInfo();
	AttachPart( pPart, pNode, pMat, _ginfo, pStaticTracker, plmTracker, false, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
	const SGroupInfo &_ginfo )
{
	CPtr< CPtrFuncBase<CObjectInfo> > pInfoHolder = pInfo;
	CPtr<IMaterial> pMaterialHolder = pMat;
	//
	if ( !CheckMaterial( pMat ) )
		return 0;
	SBound bv;
	if ( !CheckOI( pInfo, &bv ) )
		return 0;
	CNonePart *pRes = new CNonePart( pInfo );
	PlaceToOctree( pRes, pVolume, bv.s.ptCenter, bv.s.fRadius, pMat, _ginfo, pStaticTracker, GetLMTracker() );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
	const SFBTransform &trans, const SGroupInfo &_ginfo )
{
	CPtr< CPtrFuncBase<CObjectInfo> > pInfoHolder = pInfo;
	CPtr<IMaterial> pMaterialHolder = pMat;
	//
	if ( !CheckMaterial( pMat ) )
		return 0;
	SBound bv;
	if ( !CheckOI( pInfo, &bv ) )
		return 0;
	CVec3 pos;
	trans.forward.RotateHVector( &pos, bv.s.ptCenter );
	float fR = sqrt( CalcRadius2( bv, trans.forward ) );
	CSimplePart *pRes = new CSimplePart( pInfo, trans );
	PlaceToOctree( pRes, pVolume, pos, fR, pMat, _ginfo, pStaticTracker, GetLMTracker() );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
	const SDiscretePos &trans, const SGroupInfo &_ginfo )
{
	CPtr< CPtrFuncBase<CObjectInfo> > pInfoHolder = pInfo;
	CPtr<IMaterial> pMaterialHolder = pMat;
	//
	if ( !CheckMaterial( pMat ) )
		return 0;
	SBound bv;
	if ( !CheckOI( pInfo, &bv ) )
		return 0;
	CVec3 pos;
	SFBTransform fbTrans;
	trans.MakeMatrix( &fbTrans );
	fbTrans.forward.RotateHVector( &pos, bv.s.ptCenter );
	float fR = sqrt( CalcRadius2( bv, fbTrans.forward ) );
	CDiscretePart *pRes = new CDiscretePart( pInfo, trans );
	PlaceToOctree( pRes, pVolume, pos, fR, pMat, _ginfo, pStaticTracker, GetLMTracker() );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
	CFuncBase<SFBTransform> *pPlacement, const SGroupInfo &_ginfo )
{
	CPtr< CPtrFuncBase<CObjectInfo> > pInfoHolder = pInfo;
	CPtr<IMaterial> pMaterialHolder = pMat;
	//
	if ( !CheckMaterial( pMat ) )
		return 0;
	CDynamicPart *pRes = new CDynamicPart( pInfo, pPlacement, pMat, _ginfo );
	movingParts.push_back( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
	CFuncBase<vector<SHMatrix> > *pPlacement, const SGroupInfo &_ginfo )
{
	CPtr< CPtrFuncBase<CObjectInfo> > pInfoHolder = pInfo;
	CPtr<IMaterial> pMaterialHolder = pMat;
	//
	if ( !CheckMaterial( pMat ) )
		return 0;
	CAnimatedPart *pRes = new CAnimatedPart( pInfo, pPlacement, pMat, _ginfo );
	animatedParts.push_back( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateDynamicGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
	CFuncBase<SBound> *pBound, const SGroupInfo &_ginfo )
{
	CPtr< CPtrFuncBase<CObjectInfo> > pInfoHolder = pInfo;
	CPtr<IMaterial> pMaterialHolder = pMat;
	//
	if ( !CheckMaterial( pMat ) )
		return 0;
	CDynamicGeometryPart *pRes = new CDynamicGeometryPart( pInfo, pBound, pMat, _ginfo );
	dynamicFrags.push_back( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateParticles( CPtrFuncBase<CParticleEffect> *pInfo, 
	CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SGroupInfo &_ginfo, bool bIsLit )
{
	CParticles* pRes = new CParticles( pInfo, pPlacement, bound, _ginfo, bIsLit );
	particles.push_back( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateLitParticles( CPtrFuncBase<CParticleEffect> *pInfo, 
	IMaterial *pMat, CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SGroupInfo &_ginfo )
{
	CPtr<IMaterial> pHoldMaterial(pMat);
	return CreateParticles( pInfo, pPlacement, bound, _ginfo, true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CPolyline* CGScene::CreatePolyline( CPtrFuncBase<NGfx::CGeometry> *pGeometry, const CVec3 &color )
{
	CPolyline *pR = new CPolyline;
	pR->pGeometry = pGeometry;
	pR->color = color;
	lines.push_back( pR );
	return pR;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGScene::CreateSelection( CObjectBase *pRenderNode, const CVec4 &vColor )
{
	CPtr<CSelection> pSelection = new CSelection;
	if ( pSelection->Initialize( pRenderNode, vColor ) )
	{
		selections.push_back( pSelection );
		return pSelection;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::SetAmbient( const CVec3 &_ambientColor )
{
	pAmbient->Set( _ambientColor );
	pFakeParticleLM->SetAmbient( _ambientColor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSortLightGroups
{
	bool operator()( const ILight *p1, const ILight *p2 ) const 
	{ 
		return p1->GetPriority() < p2->GetPriority();
	}
};
void CGScene::AddLight( ILight *pGroup )
{
	lights.push_back( pGroup );
	lights.sort( SSortLightGroups() );
	bLightStateCalced = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ILight* CGScene::AddPointLight( const CVec3 &_vColor, const CVec3 &ptOrigin, float fR, bool bLightmapOnly )
{
	if ( fR <= 0 )
		return 0;
	ILight *pRes = new CPointLight( _vColor, ptOrigin, fR, pStaticTracker, bLightmapOnly );
	AddLight( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ILight* CGScene::AddPointLight( CPtrFuncBase<CAnimLight> *pLight )
{
	ILight *pRes = new CDynamicPointLight( pLight );
	AddLight( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ILight* CGScene::AddSpotLight( CFuncBase<CVec3> *pColor, const CVec3 &ptOrigin, const CVec3 &ptDir, 
	float fFOV, float fRadius, CPtrFuncBase<NGfx::CTexture> *pMask, bool bLightmapOnly )
{
	CPtr<CFuncBase<CVec3> > pHold(pColor);
	CPtr<CPtrFuncBase<NGfx::CTexture> > pHold1(pMask);
	if ( fRadius <= 0 )
		return 0;
	return 0;
/*
	CSpotLight *pLight = new CSpotLight( pColor, ptOrigin, ptDir, fFOV, fRadius, pMask, nTargetGroupID );
	pRes->AddLight( pLight );
	AddLight( pRes );
	return pRes;*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ILight* CGScene::AddDirectionalLight( CFuncBase<CVec3> *pColor, CFuncBase<CVec3> *pGlossColor, 
	const CVec3 &vShadowColor, const CVec3 &ptLight, 
	const CVec3 &ptOrigin, const CVec2 &ptSize, float fMaxHeight, bool bLightmapOnly ) 
{
	pFakeParticleLM->SetColor( pColor );
	ILight *pRes = new CDirectionalLight( pColor, pGlossColor, vShadowColor,
		ptLight, ptOrigin, ptSize, fMaxHeight, pStaticTracker, pAmbient, bLightmapOnly );
	AddLight( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::RecalcCullingInfo()
{
	UpdateSet( &animatedParts, pVolume.GetPtr(), pStaticTracker.GetPtr(), GetLMTracker() );
	UpdateSet( &movingParts, pVolume.GetPtr(), pStaticTracker.GetPtr(), GetLMTracker() );
	UpdateSet( &dynamicFrags, pVolume.GetPtr(), pStaticTracker.GetPtr(), GetLMTracker() );
	//UpdateSet( &litParticles, pVolume.GetPtr() );
	UpdateSet( &particles, pVolume.GetPtr() );
	if ( --nSlowVolumeWalk < 0 )
	{
		nSlowVolumeWalk = 30;
		pVolume->Walk();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static TPartFlags SelectParts( CTransformStack *pTS, int nParts, IVBCombiner *pVB )
{
	if ( nParts == 1 || pTS->IsFullGet() )
		return PF_ALL_PARTS;
	TPartFlags nRes = 0;
	const vector<SSphere> &partBVs = pVB->GetBounds();
	for ( int i = 0; i < nParts; ++i )
	{
		if ( pTS->IsIn( partBVs[i] ) )
			nRes |= 1<<i;
	}
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AddParts( CTransformStack *pTS, list<SRenderPartSet> *pRes, 
	const list<CPtr<CCombinedPart> > &elems, const SGroupSelect &mask )
{
	for ( list<CPtr<CCombinedPart> >::const_iterator i = elems.begin(); i != elems.end(); ++i )
	{
		CCombinedPart *pElement = *i;
		if ( !IsValid( pElement ) )
			continue;
		if ( !pElement->GetGroup().IsMaskMatch( mask ) )
			continue;
		
		IVBCombiner *pVB = pElement->GetVBCombiner();
		if ( !pTS->PushClipHint( pVB->GetBound() ) )
			continue;
		
		CDGPtr<CPerMaterialCombiner> pCombiner = pElement->GetCombiner();
		pCombiner.Refresh();
		const vector< CPtr<IPart> > &listParts = pCombiner->GetValue();
		SRenderPartSet &res = *pRes->insert( pRes->end(), SRenderPartSet( pElement, &listParts ) );
		res.nParts = SelectParts( pTS, listParts.size(), pVB );
		pTS->PopClipHint();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::AddNodeParts( CTransformStack *pTS, list<SRenderPartSet> *pRes, CVolumeNode *pNode, ERLRequest eReq, const SGroupSelect &mask )
{
	if ( pNode == 0 )
		return;

	SSphere sClipTest;
	pNode->GetBound( &sClipTest );
	if ( !pTS->PushClipHint( sClipTest ) )
		return;

	if ( eReq & RN_DYNAMIC )
		AddParts( pTS, pRes, pNode->dynamicParts.elements, mask );
	if ( eReq & RN_STATIC )
		AddParts( pTS, pRes, pNode->staticParts.elements, mask );

	for ( int i = 0; i < 8; ++i )
		AddNodeParts( pTS, pRes, pNode->GetNode(i), eReq, mask );

	pTS->PopClipHint();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::MakePartList( CTransformStack *pTS, list<SRenderPartSet> *pRes, ERLRequest req, const SGroupSelect &mask )
{
	AddNodeParts( pTS, pRes, pVolume, req, mask );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::SSceneFragmentGroupInfo::AddElement( CTransformStack *pTS, CCombinedPart *p, ERLRequest req, 
	int _nIgnoreMark )
{
	switch ( p->GetMaterial()->GetType() )
	{
		case IMaterial::MT_TRANSPARENT:
			if ( pTransp )
			{
				const SBound &bv = p->GetVBCombiner()->GetBound();
				if ( pTS->IsIn( bv ) )
					pTransp->AddTransparent( new CTransElementWrapper( p, bv.s.ptCenter ) );
			}
			return;
		case IMaterial::MT_OCCLUDER:
			if ( req & RN_OCCLUDERS )
			{
				pList->CountTris( p->GetGeometryInfo(), PF_ALL_PARTS );
				pList->AddElement( p, p->GetGeometryInfo(), PF_ALL_PARTS, p->GetMaterial(), 0, 0, true );
			}
			return;
		case IMaterial::MT_NORMAL:
			if ( req & RN_OCCLUDERS )
				return;
			break;
		default: ASSERT(0); break;
	}
	const SBound &bv = p->GetVBCombiner()->GetBound();
	if ( !pTS->PushClipHint( bv ) )
		return;

	// select parts
	TPartFlags nParts;
	int nOriginalPartsNum = p->GetCombiner()->GetValue().size();
	bool bSkipPerPartTests = nOriginalPartsNum == 1;
	if ( p->GetIgnoreMark() == _nIgnoreMark )
	{
		nParts = ~p->GetIgnoreFlags();
		int nTris = pList->CountTris( p->GetGeometryInfo(), nParts );
		bSkipPerPartTests |= nTris < N_TRIS_PER_NODE_FOR_PER_PART_CLIP;
	}
	else
	{
		nParts = PF_ALL_PARTS;
		int nTris = pList->CountTris( p->GetGeometryInfo(), nParts );
		if ( nTris > N_TRIS_PER_NODE_FOR_PER_PART_CLIP && !bSkipPerPartTests )
			nParts = SelectParts( pTS, nOriginalPartsNum, p->GetVBCombiner() );
		else
			bSkipPerPartTests = true;
	}
	pTS->PopClipHint();
	// shortcut when no lightmap info is required
	if ( (req & RN_LIGHTMAPS) == 0 )
	{
		pList->AddElement( p, p->GetGeometryInfo(), nParts, p->GetMaterial(), 0, 0, bSkipPerPartTests );
		return;
	}
	int nGroup = p->GetGroup().nLightGroup;
	if ( nGroup == 0 )
	{
		// static lightmaps are used
		CDGPtr<CFuncBase<vector< CPtr<IPart> > > > pCombiner( p->GetCombiner() );
		pCombiner.Refresh();
		const vector< CPtr<IPart> > &parts = pCombiner->GetValue();
		TPartFlags nNewParts = 0;
		int nElementSize = 70 / ( fabs( bv.s.ptCenter - vCamera ) + 10 );
		int nLOD = 0;
		while ( nElementSize > 1 )
		{
			++nLOD;
			nElementSize >>= 1;
		}
		for ( int i = 0; i < parts.size(); ++i )
		{
			if ( ( nParts & (1<<i) ) == 0 )
				continue;
			CNonePart *pRes = SafeCast<CNonePart>( parts[i].GetPtr() );
			if ( !IsValid( pRes->pLightmapInfo ) || pRes->pLightmapInfo->nLOD != nLOD )
			{
				pRes->pLightmapInfo = pLMTracker->AllocRegion( pRes, nLOD );
				p->UpdateLMMapping(); // recalc VB combiner stuff
			}
			CLMRegion *pLMRegion = pRes->pLightmapInfo;
			if ( pLMRegion->nLastFrame < nCurrentFrame - 1 )
			{
				bHasNewLightmaps = true;
				nNewParts |= 1 << i;
			}
			pLMRegion->Touch();
			pLMRegion->nLastFrame = nCurrentFrame;
		}
		if ( nNewParts != 0 )
			pNewLMList->AddElement( p, p->GetGeometryInfo(), nNewParts, p->GetMaterial(), pLMTracker->GetLMTexture(), 0, true );
		pList->AddElement( p, p->GetGeometryInfo(), nParts, p->GetMaterial(), pLMTracker->GetLMTexture(), 0, bSkipPerPartTests );
		return;
	}
	// dynamic lightmap
	if ( nGroup != 0 )
	{
		for ( vector<SDynamicLightGroup>::iterator i = groups.begin(); i != groups.end(); ++i )
		{
			if ( i->nGroup == nGroup )
			{
				if ( i->bv.s.fRadius < bv.s.fRadius )
					i->bv = bv;
				pList->AddElement( p, p->GetGeometryInfo(), nParts, p->GetMaterial(), 0, i->pAmbient, bSkipPerPartTests );
				return;
			}
		}
	}
	SDynamicAmbientInfo *pLM = pList->AllocDynamicAmbient();
	groups.push_back( SDynamicLightGroup( pLM, bv, nGroup ) );
	pList->AddElement( p, p->GetGeometryInfo(), nParts, p->GetMaterial(), 0, pLM, bSkipPerPartTests );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::SSceneFragmentGroupInfo::CalcLightmaps( CGScene *pScene )
{
	for ( vector<SDynamicLightGroup>::iterator i = groups.begin(); i != groups.end(); ++i )
	{
		const SDynamicLightGroup &g = *i;
		*g.pAmbient = pScene->GetDynamicAmbient( g.nGroup, g.bv );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SLitParticlesAdder : public IReportParticlesGeometry
{
	CSceneFragments *pList;
	IMaterial *pMaterial;
	CTransformStack *pTS;
	SLitParticlesAdder( CSceneFragments *_pList, IMaterial *_pMaterial, CTransformStack *_pTS )
		: pList(_pList), pMaterial(_pMaterial), pTS(_pTS) {}
	virtual void AddParticles( IVBCombiner *pVertices, CFuncBase<vector<NGfx::STriangleList> > *pTrilists, 
		int nPart, int nParticles, const SBound &bv )
	{
		if ( nParticles )
		{
			if ( pTS->IsIn( bv ) )
				pList->AddLitParticles( pVertices, pTrilists, nPart, pMaterial, nParticles < N_PARTICLES_TO_START_CLIP );
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::AddNode( CTransformStack *pTS, SSceneFragmentGroupInfo *pFragmentsInfo,
	CVolumeNode *pNode, ERLRequest req, const SGroupSelect &mask, int nIgnoreMark )
{
	if ( pNode == 0 )
		return;
	SSphere stest;
	pNode->GetBound( &stest );
	if ( !pTS->PushClipHint( stest ) )
		return;
	bool bDoLightmpasCalcDynamicAmbient = ( req & RN_LIGHTMAPS ) != 0;
	if ( req & RN_DYNAMIC )
	{
		list<CPtr<CCombinedPart> > &elems = pNode->dynamicParts.elements;
		for ( list<CPtr<CCombinedPart> >::iterator i = elems.begin(); i != elems.end(); ++i )
		{
			CCombinedPart *pElement = *i;
			if ( !IsValid( pElement ) )
				continue;
			if ( !pElement->GetGroup().IsMaskMatch( mask ) )
				continue;
			pFragmentsInfo->AddElement( pTS, pElement, req /*pVB->GetBound()*/, 1 );
		}
	}
	if ( req & RN_STATIC )
	{
		list<CPtr<CCombinedPart> > &elems = pNode->staticParts.elements;
		for ( list<CPtr<CCombinedPart> >::iterator i = elems.begin(); i != elems.end(); ++i )
		{
			CCombinedPart *pElement = *i;
			if ( !IsValid( pElement ) )
				continue;
			if ( !pElement->GetGroup().IsMaskMatch( mask ) )
				continue;
			pFragmentsInfo->AddElement( pTS, pElement, req/*, pVB->GetBound()*/, nIgnoreMark );
		}
	}
	if ( ( req & RN_OCCLUDERS ) == 0 )
	{
		for ( list<CPtr<CParticles> >::iterator i = pNode->particles.begin(); i != pNode->particles.end(); ++i )
		{
			CParticles *pPart =*i;
			if ( !IsValid( pPart ) )
				continue;
			if ( !pPart->GetGroup().IsMaskMatch( mask ) )
				continue;
			if ( (req & RN_DEPTH) != 0 )
			{
				if ( !pPart->IsLit() )
					continue;
				if ( req & RN_DYNAMIC )
					continue;
			}
			const SBound &bv = pPart->GetBound();
			if ( pTS->IsIn( bv.s ) )
			{
				CTransparentRenderer *pTransp = pFragmentsInfo->pTransp;
				ASSERT( pTransp );
				if ( pTransp )
				{
					if ( pPart->IsLit() )
					{
						if ( ( req & (RN_DEPTH|RN_LIT_PARTICLES) ) != 0 )
						{
							SLitParticlesAdder adder( pFragmentsInfo->pList, pTransparentMaterial, pTS );
							pTransp->AddParticles( pPart, true, bv, &adder );
						}
						else
							pTransp->AddParticles( pPart, true, bv, 0 );
					}
					else
					{
						if ( ( req & RN_DEPTH ) == 0 )
							pTransp->AddParticles( pPart, false, bv, 0 );
					}
				}
			}
		}
	}
	for ( int i = 0; i < 8; ++i )
		AddNode( pTS, pFragmentsInfo, pNode->GetNode(i), req, mask, nIgnoreMark );
	pTS->PopClipHint();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::MakeRenderList( CTransformStack *pTS, SSceneFragmentGroupInfo *pTarget, 
	ERLRequest req, const SGroupSelect &mask, int nIgnoreMark )
{
	AddNode( pTS, pTarget, pVolume, req, mask, nIgnoreMark );
	if ( req & RN_LIGHTMAPS )
		pTarget->CalcLightmaps( this );
	if ( pTarget->pTransp )
		pTarget->pTransp->FinishParticles();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::RecalcRenderStats( int nSceneTris, int nParticles, int nLitParticles )
{
	lastFrameStats.nSceneTris += nSceneTris;
	lastFrameStats.nParticles += nParticles;
	lastFrameStats.nLitParticles += nLitParticles;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool operator!=( const SHMatrix &a, const SHMatrix &b )
{
	return memcmp( &a, &b, sizeof(a) ) != 0;
}
void CGScene::UpdateIgnoreMark( IRender *pRender, CTransformStack *pTS, const SGroupSelect &mask )
{
	bool bStaticUpdated = pIgnoreStaticTrack.Refresh();
	if ( bStaticUpdated || pTS->Get().forward != mHoldTransform || holdMask != mask )
	{
		if ( bStaticUpdated ) // if this always happens no glitches will be visible but perfomance will drop
			++nCurrentIgnoreMark;  
		nIgnoreListWasCalced = 0;//false;
	}
	else
	{
		if ( nIgnoreListWasCalced == 2 )//!bIgnoreListWasCalced )
		{
			++nCurrentIgnoreMark;
			CIgnorePartsHash res;
			MakeInvisibleElementsList( pRender, pTS, mask, GetScreenRect(), &res );
			for ( CIgnorePartsHash::iterator i = res.begin(); i != res.end(); ++i )
			{
				CDynamicCast<CCombinedPart> pC( i->first );
				pC->SetIgnored( nCurrentIgnoreMark, i->second );
			}
			//bIgnoreListWasCalced = true;
		}
		++nIgnoreListWasCalced;
	}
	mHoldTransform = pTS->Get().forward;
	holdMask = mask;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::TraceDynamicAmbient( SDynamicAmbientInfo *pRes, const SBound &bv )
{
	if ( pLMTracker )
		pLMTracker->GetLightState().TraceDynamicLM( pRes, bv, this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::CheckDynamicLightmapCache()
{
	if ( dynamicLightCache.CheckStatic() )
	{
		for ( int k = 0; k < lightGroups.size(); ++k )
		{
			CLightGroup *p = lightGroups[k];
			if ( IsValid( p ) )
				p->vPrevPos = CVec3( -1e4f, -1e4f, -1e4f );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SDynamicAmbientInfo& CGScene::GetDynamicAmbient( int nGroup, const SBound &bv )
{
	//ASSERT( nGroup >= 0 && nGroup < lightGroups.size() && ( nGroup == 0 || IsValid( lightGroups[nGroup] ) ) );
	if ( nGroup < 1 || nGroup >= lightGroups.size() || !IsValid( lightGroups[nGroup] ) )
		return dynamicLightCache.Calc( bv, this );
	CLightGroup *p = lightGroups[ nGroup ];
	float fDist = fabs2( p->vPrevPos - bv.s.ptCenter );
	if ( fDist < 0.5f )
		return p->ambientData;
	p->vPrevPos = bv.s.ptCenter;
	if ( fDist < 1 )
	{
		SDynamicAmbientInfo lm;
		TraceDynamicAmbient( &lm, bv );
		p->ambientData.Blend( lm, 0.5f );
	}
	else
		TraceDynamicAmbient( &p->ambientData, bv );
	return p->ambientData;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::ResetDynamicLightmapsCache()
{
	dynamicLightCache.Reset();
	for ( int k = 0; k < lightGroups.size(); ++k )
	{
		if ( IsValid( lightGroups[k] ) )
			lightGroups[k]->vPrevPos = CVec3(1e30f,1e30f,1e30f);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::CalcNewLightState()
{
	SGlobalIlluminationInfo lsGlobal;
	lsGlobal.globalBounds = SSphere( CVec3( 10, 10, 10 ), 30 ); // CRAP, currently is not used
	for ( list< CPtr<ILight> >::iterator i = lights.begin(); i != lights.end(); ++i )
	{
		if ( !IsValid( *i) )
			continue;
		if ( CDynamicCast<CDirectionalLight> pDir(*i) )
		{
			CDirectionalLight::SRadianceInfo info;
			pDir->GetRadianceInfo( &info, RM_BEST_GF3 );
			lsGlobal.vAmbient = info.vAmbientColor;
			lsGlobal.parallel.push_back( 
				SGlobalIlluminationInfo::SDirectional( info.vColor, info.vDirection, info.bIsRendered ) 
				);
		}
		else if ( CDynamicCast<CPointLight> pPoint(*i) )
		{
			CPointLight::SRadianceInfo info;
			pPoint->GetRadianceInfo( &info, RM_BEST_GF3 );
			lsGlobal.points.push_back( 
				SGlobalIlluminationInfo::SPoint( info.vColor, info.vCenter, info.fRadius, info.bIsRendered )
				);
		}
	}
	pLMTracker->SetNewIllumination( lsGlobal );
	ResetDynamicLightmapsCache();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::UpdateLightmaps( CTransformStack *pTS, CSceneFragments *pScene, CSceneFragments *pNewLMScene, bool bHasNewLightmaps )
{
	if ( !bLightStateCalced )
	{
		CalcNewLightState();
		bLightStateCalced = true;
		// CRAP somehow recalc everything dynamically
		//crapRecalcLightmaps = LM_CALC_ALL;//LM_CALC_COLOR;
	}
	if ( crapRecalcLightmaps == LM_CALC_CLEAR )
	{
		pLMTracker->ResetToChecker();
		crapRecalcLightmaps = LM_CALC_NONE;
		return;
	}
	if ( NGlobal::GetVar( "gfx_lm_recalc", 1 ).GetFloat() == 0 )
		return;
	CRenderWrapper renderWrapper( this );
	CLightmapTracker &lmTracker = *GetLMTracker();
	lmTracker.CatchUp( &renderWrapper, pTS, pScene, pNewLMScene, bHasNewLightmaps );
	//if ( calc == LM_CALC_SCATTER ) currentLightState.CreateScattered( lsGlobal, this ); // scatter
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SVec4Hash
{
	int operator()( const CVec4 &a ) const { int *p = (int*)&a.x; return p[0] ^ p[1] ^ p[2] ^ p[3]; }
};
void CGScene::DrawSelection( CTransformStack *pTS, NGfx::CRenderContext *pRC )
{
	UpdateSet( &selections, this );
	NGfx::CRenderContext &rc = *pRC;

	rc.SetCulling( NGfx::CULL_CW );
	rc.SetAlphaCombine( NGfx::COMBINE_ZERO_ONE );
	rc.SetColorWrite( NGfx::COLORWRITE_NONE );
	rc.SetDepth( NGfx::DEPTH_NONE );
	rc.SetStencil( NGfx::STENCIL_WRITE, 1 );
	NGfx::SEffGlow sGlow;
	NGfx::SEffConstLight tnlGlow;
	if ( NGfx::IsTnLDevice() )
	{
		tnlGlow.color = CVec4(0,0,0,0);
		rc.SetEffect( &tnlGlow );
	}
	else
	{
		sGlow.fScale = 0;
		sGlow.vColor = CVec4(0,0,0,0);
		rc.SetEffect( &sGlow );
	}
	for ( list< CPtr<CSelection> >::iterator i = selections.begin(); i != selections.end(); ++i )
		(*i)->Render( pTS, pRC, false );
	rc.Flush();

	rc.SetCulling( NGfx::CULL_NONE );
	rc.SetAlphaCombine( NGfx::COMBINE_ADD );
	rc.SetColorWrite( NGfx::COLORWRITE_ALL );
	rc.SetDepth( NGfx::DEPTH_NORMAL );
	rc.SetStencil( NGfx::STENCIL_TESTINCR, 0 );//STENCIL_TESTDECR, 1 );
	typedef hash_map<CVec4, list< CPtr<CSelection> >, SVec4Hash> CColorHash;
	CColorHash hashSel;
	for ( list< CPtr<CSelection> >::iterator i = selections.begin(); i != selections.end(); ++i )
		hashSel[ (*i)->GetColor() ].push_back( *i );

	for ( CColorHash::iterator i = hashSel.begin(); i != hashSel.end(); ++i )
	{
		if ( NGfx::IsTnLDevice() )
		{
			tnlGlow.color = i->first;
			rc.SetEffect( &tnlGlow );
		}
		else
		{
			sGlow.fScale = F_SELECTION_STEP;
			sGlow.vColor = i->first;
			rc.SetEffect( &sGlow );
		}
		for ( list< CPtr<CSelection> >::iterator k = i->second.begin(); k != i->second.end(); ++k )
			(*k)->Render( pTS, pRC, true );
		rc.Flush();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::DrawLines( NGfx::CRenderContext *pRC )
{
	pRC->SetAlphaCombine( NGfx::COMBINE_NONE );
	pRC->SetDepth( NGfx::DEPTH_NORMAL );
	pRC->SetStencil( NGfx::STENCIL_NONE );
	typedef hash_map<CVec3, list<CPolyline*>, SVec3Hash> CColorHash;
	CColorHash hashSel;
	for ( list< CPtr<CPolyline> >::iterator i = lines.begin(); i != lines.end(); )
	{
		if ( IsValid(*i) )
		{
			hashSel[ (*i)->color ].push_back( *i );
			++i;
		}
		else
			i = lines.erase( i );
	}

	for ( CColorHash::iterator k = hashSel.begin(); k != hashSel.end(); ++k )
	{
		NGfx::SEffConstLight d;
		d.color = k->first;
		pRC->SetEffect( &d );
		const list<CPolyline*> &l = k->second;
		for ( list<CPolyline*>::const_iterator i = l.begin(); i != l.end(); ++i )
			(*i)->Render( pRC );
		pRC->Flush();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::RefreshParticleLMTarget()
{
	if ( IsValid( particleLM.pParticleLMs ) )
		return;
	particleLM.pParticleLMs = shadowMapsShare.GetParticleLM();
	SHMatrix &m = particleLM.rootTransform;
	NGfx::MakeLMToScreenMatrix( &m, N_DEFAULT_RT_RESOLUTION, N_DEFAULT_RT_RESOLUTION );
	particleLM.vParticleLMSize = CVec2( N_DEFAULT_RT_RESOLUTION, N_DEFAULT_RT_RESOLUTION );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::Draw( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, 
	const SGroupSelect &_mask, EFogMode fogMode, const SFogParams &_fog, EHSRMode hsrMode )
{
	ESceneRenderMode rm = GetRenderMode();
	SGroupSelect mask(_mask);
	bool bUseFakeParticleLM = true;
	NGfx::EHardwareLevel hl = NGfx::GetHardwareLevel();
	// lit particles mode
	if ( hl >= NGfx::HL_GFORCE3 )
	{
		if ( rm >= RM_BEST_GF2 )
			bUseFakeParticleLM = false;
		if ( !bUseFakeParticleLM )
			RefreshParticleLMTarget();
	}
	if ( hl < NGfx::HL_GFORCE3 && rm > RM_BEST_GF2 )
		rm = RM_BEST_GF2;
	if ( !pRC->HasRegisters() && IsUsingRegister( rm ) )
	{
		ASSERT(0);
		rm = RM_FASTEST;
	}
//	CObj<IRenderFactor> pFog;
	CPtr<NGfx::CTexture> pFogLookup;
	SFogParams fog( _fog );
	// recalc camera pos
	{
		CVec4 ptCenter;
		pTS->Get().backward.RotateHVector( &ptCenter, CVec4(0,0,1,0) );
		ptCenter.x /= ptCenter.w; ptCenter.y /= ptCenter.w; ptCenter.z /= ptCenter.w;
		pCamera->Set( CVec3( ptCenter.x, ptCenter.y, ptCenter.z ) );
		fog.fCameraHeight = ptCenter.z;
	}
	//
	MarkNewDGFrame();
	RecalcCullingInfo();
	// update lightmaps
	CheckDynamicLightmapCache();

	CSceneFragments geom, newLMgeom;
	CObj<CTransparentRenderer> pTransp;
	pFakeParticleLM.Refresh();
	if ( bUseFakeParticleLM )
	{
		CTPoint<int> ptFakeRegisterSize( N_FAKE_LM_SIZEX, N_FAKE_LM_SIZEY );
		pTransp = new CTransparentRenderer( pTS, ptFakeRegisterSize, true, 
			pFakeParticleLM->GetParticleColor(), pFakeParticleLM->GetNormalColor() );
	}
	else
		pTransp = new CTransparentRenderer( pTS, CTPoint<int>(N_DEFAULT_RT_RESOLUTION,N_DEFAULT_RT_RESOLUTION), false,
			pFakeParticleLM->GetParticleColor(), pFakeParticleLM->GetNormalColor() );
	CRenderWrapper renderWrapper( this );
	int nUseIgnoreMark = 1;
	if ( hsrMode != HSR_NONE )
	{
		UpdateIgnoreMark( &renderWrapper, pClipTS, mask );
		nUseIgnoreMark = nCurrentIgnoreMark;
	}
	ERLRequest rlReq = RN_ALL;
	if ( rm != RM_FASTEST )
		rlReq = (ERLRequest)( ((int)rlReq) | RN_LIGHTMAPS );
	if ( rm >= RM_BEST_GF2 )
		rlReq = (ERLRequest)( ((int)rlReq) | RN_LIT_PARTICLES );	
	if ( rm == RM_SHOWOCCLUDERS )
	{
		rlReq = (ERLRequest)( RN_OCCLUDERS|RN_STATIC );//|RN_DYNAMIC );
		mask.nMaskEvery = mask.nMaskAny = N_OCCLUDER_FLAG;
	}
	bool bDoUseLightmaps = bLightmapsEnable && rm > RM_FASTEST;
	if ( bDoUseLightmaps )
		GetLMTracker()->RefreshCache();
	SSceneFragmentGroupInfo renderList( pCamera->GetValue(), &geom, pTransp, GetLMTracker(), ++nFrameCounter, &newLMgeom );
	MakeRenderList( pClipTS, &renderList, rlReq, mask, nUseIgnoreMark );
	if ( bDoUseLightmaps )
		UpdateLightmaps( pTS, &geom, &newLMgeom, renderList.bHasNewLightmaps );
	// add lights to geom
	EraseInvalidRefs( &lights );
	// add all lights to scene
	NGfx::CRenderContext &rc = *pRC;
	if ( bWireframe )
		NGfx::SetWireframe( NGfx::WIREFRAME_ON );
	else
		NGfx::SetWireframe( NGfx::WIREFRAME_OFF );

	// render all lights
	SParticleLMRenderTargetInfo useParticleTarget;
	if ( !bUseFakeParticleLM )
		useParticleTarget = particleLM;
	useParticleTarget.vKernelSize = pTransp->GetKernelLightInfo().vLightSize;
	for ( list< CPtr<ILight> >::iterator k = lights.begin(); k != lights.end(); ++k )
	{
		ILight *pLight = *k;
		pLight->Render( pTS, pClipTS, &rc, rm, &renderWrapper, geom, useParticleTarget );
	}
	// render fog & reflection
	if ( rm != RM_TNL )
	{
		CRenderCmdList finalOps;
		switch ( fogMode )
		{
			case FOG_PERVERTEX:
				AddFinalOps( &finalOps, geom, rm, RO_FOG_STATIC, &fog );
				pFogLookup = GetFogLookupTexture( fog );
				break;
			case FOG_DYNAMIC:
				AddFinalOps( &finalOps, geom, rm, RO_FOG_DYNAMIC, &fog );
				pFogLookup = GetFogLookupTexture( fog );
				break;
			default:
				AddFinalOps( &finalOps, geom, rm, RO_NOP, 0.0f );
				break;
		}
		SLightInfo lightInfo;
		Execute( &renderWrapper, &rc, *pTS, finalOps, lightInfo );
	}
	//
	NGfx::SetWireframe( NGfx::WIREFRAME_OFF );

	if ( rc.HasRegisters() )
		rc.SetRegister( 0 );
	// draw lines
	rc.SetTransform( pTS->Get() );
	DrawLines( &rc );
	// draw transparent stuff
	if ( bUseFakeParticleLM )
		pTransp->Render( &rc, pFogLookup, pFakeParticleLM->GetValue() );
	else
		pTransp->Render( &rc, pFogLookup, particleLM.pParticleLMs );
	DrawSelection( pTS, &rc );
	//
	RecalcRenderStats( geom.GetSceneTris(), pTransp->GetTotalParticles(), pTransp->GetLitParticles() );
	rc.SetStencil( NGfx::STENCIL_NONE );
	if ( bShow2DTextureCache )
		NGfx::ShowTexture( &rc, NGfx::GetTextureCache(), CVec2(1600,1200) );
	if ( bShowTranspTextureCache )
		NGfx::ShowTexture( &rc, NGfx::GetTransparentTextureCache(), CVec2(1600,1200) );
	if ( bShowParticleLMCache )
	{
		if ( bUseFakeParticleLM )
			;//NGfx::ShowTexture( &rc, pFakeParticleLM->GetValue(), CVec2(10, 10) );
		else
			NGfx::ShowTexture( &rc, particleLM.pParticleLMs, CVec2(800,600) );
	}
	switch ( showLinearCache )
	{
		case SLC_NONE: break;
		case SLC_DYNAMIC: NGfx::ShowTexture( &rc, NGfx::GetLinearBufferMRU( NGfx::DYNAMIC ), CVec2(1024, 768) ); break;
		case SLC_STATIC:  NGfx::ShowTexture( &rc, NGfx::GetLinearBufferMRU( NGfx::STATIC ), CVec2(1024, 768) ); break;
	}
	//
	//CopyRenderResult( &rc, 0, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec2 CGScene::GetScreenRect()
{
	return NGfx::GetScreenRect();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ESceneRenderMode CGScene::GetRenderMode()
{
	if ( NGfx::IsTnLDevice() )
		renderMode = RM_TNL;
	return renderMode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGScene::SetRenderMode( ESceneRenderMode mode )
{
	if ( !NGfx::IsTnLDevice() )
	{
		if ( mode == RM_TNL	)
			mode = RM_FASTEST;
		renderMode = mode;
	}
	else
		renderMode = RM_TNL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CFuncBase<CVec3>* CGScene::GetCamera()
{
	return pCamera;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CRenderWrapper
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderWrapper::FormPartList( CTransformStack *pTS, list<SRenderPartSet> *pRes, EDepthType dt )
{
	SGroupSelect mask(0x8000, 0);
	switch ( dt )
	{
		case DT_STATIC:
			pScene->MakePartList( pTS, pRes, CGScene::RN_STATIC, mask );
			break;
		case DT_DYNAMIC:
			pScene->MakePartList( pTS, pRes, CGScene::RN_DYNAMIC, mask );
			break;
		case DT_ALL:
			pScene->MakePartList( pTS, pRes, CGScene::RN_ALL, mask );
			break;
		default:
			ASSERT( 0 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderWrapper::FormPartList( CTransformStack *pTS, list<SRenderPartSet> *pRes, const SGroupSelect &mask )
{
	SGroupSelect maskM( mask );
	maskM.nMaskEvery |= 0x8000;
	pScene->MakePartList( pTS, pRes, CGScene::RN_STATIC, maskM );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderWrapper::FormDepthList( CTransformStack *pTS, CTransformStack *pParticleTS, CSceneFragments *pRes, IRender::EDepthType dt )
{
	CTPoint<int> ptFakeRegisterSize( N_FAKE_LM_SIZEX, N_FAKE_LM_SIZEY );
	CObj<CTransparentRenderer> pTransp( new CTransparentRenderer( pParticleTS, ptFakeRegisterSize, true, 0, 0 ) );
	SGroupSelect mask(0x8000, 0);
	CGScene::SSceneFragmentGroupInfo target( VNULL3, pRes, pTransp );
	switch ( dt )
	{
		case DT_STATIC:
			pScene->MakeRenderList( pTS, &target, (CGScene::ERLRequest)(CGScene::RN_STATIC | CGScene::RN_DEPTH), mask, -1 );
			break;
		case DT_DYNAMIC:
			pScene->MakeRenderList( pTS, &target, (CGScene::ERLRequest)(CGScene::RN_DYNAMIC | CGScene::RN_DEPTH), mask, -1 );
			break;
		case DT_ALL:
			pScene->MakeRenderList( pTS, &target, (CGScene::ERLRequest)(CGScene::RN_ALL | CGScene::RN_DEPTH), mask, -1 );
			break;
		case DT_FAST:
			mask = SGroupSelect( N_OCCLUDER_FLAG, N_OCCLUDER_FLAG );
			pScene->MakeRenderList( pTS, &target, (CGScene::ERLRequest)(CGScene::RN_OCCLUDERS|CGScene::RN_STATIC), mask, -1 );
			break;
		default:
			ASSERT( 0 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
IGScene* CreateScene()
{
	return new CGScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Is3DActive()
{
	return NGfx::Is3DActive();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MakeScreenShot()
{
	char szName[1024];
	SYSTEMTIME sTime;
	GetLocalTime( &sTime );
	sprintf( szName, ".\\screenshots\\ScrnShot_%2.2d%2.2d%2.2d_%2.2d%2.2d%2.2d.bmp", sTime.wDay, sTime.wMonth, sTime.wYear % 100, sTime.wHour, sTime.wMinute, sTime.wSecond );

	CFileStream f;
	try
	{
		f.OpenWrite( szName );
	}
	catch(...)
	{
		return;
	}

	CArray2D<NGfx::SPixel8888> data;
	NGfx::MakeScreenShot( &data, true );

	BITMAPFILEHEADER head;
	BITMAPINFOHEADER info;
	Zero( head );
	Zero( info );
	head.bfType = 0x4d42;
	head.bfSize = sizeof( head ) + sizeof( info ) + data.GetXSize() * data.GetYSize() * 4;
	head.bfOffBits = sizeof( head ) + sizeof( info );
	info.biSize = sizeof( info );
	info.biWidth = data.GetXSize();
	info.biHeight = data.GetYSize();
	info.biPlanes = 1;
	info.biBitCount = 32;
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 1;
	info.biYPelsPerMeter = 1;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

	f.Write( &head, sizeof( head ) );
	f.Write( &info, sizeof( info ) );
	for ( int y = data.GetYSize() - 1; y >=0; --y )
		f.Write( &data[y][0], data.GetXSize() * 4 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Clear( NGfx::CRenderContext *pRC, const CVec3 &vColor )
{
	NGfx::SPixel8888 clearColor( Float2Int( vColor.x * 255 ), Float2Int( vColor.y * 255 ), Float2Int( vColor.z * 255 ), 0 );
	pRC->ClearBuffers( clearColor.color );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClearScreen( const CVec3 &vColor )
{
	NGfx::CRenderContext rc;
	NGfx::SPixel8888 clearColor( Float2Int( vColor.x * 255 ), Float2Int( vColor.y * 255 ), Float2Int( vColor.z * 255 ), 0 );
	rc.ClearBuffers( clearColor.color );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CopyRegisterOnScreen( const CTRect<float> &rScreenRect, bool bAlphaTest )
{
	NGfx::CRenderContext rcScreen;
	CTRect<float> regSize;
	NGfx::GetRegisterSize( &regSize );

	//ShowTexture( NGfx::GetRegisterTexture(0) );
	if ( bAlphaTest )//pTarget && pTarget->bUseAlphaTest )
		rcScreen.SetAlphaCombine( NGfx::COMBINE_ALPHA );
	else
		rcScreen.SetAlphaCombine( NGfx::COMBINE_NONE );
	rcScreen.SetStencil( NGfx::STENCIL_NONE );
	NGfx::CopyTexture( rcScreen, NGfx::GetScreenRect(), rScreenRect, NGfx::GetRegisterTexture(0), regSize );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Flip()
{
	NGfx::Flip();

	lastFrameStats.nSceneTris = 0;
	lastFrameStats.nParticles = 0;
	lastFrameStats.nLitParticles = 0;
	static NHPTimer::STime timeFrameStart = 0;
	lastFrameStats.fFrameTime = NHPTimer::GetTimePassed( &timeFrameStart );
	lastFrameStats.bStaticShadowDepthRendered = bStaticShadowDepthRendered;
	bStaticShadowDepthRendered = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CalcTouchedTextureSize()
{
	return NGfx::CalcTouchedTextureSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SetWireframe( bool bWire )
{
	bWireframe = bWire;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetRenderStats( SRenderStats *pStats )
{
	lastFrameStats.nTris = NGfx::renderStats.nTris;
	lastFrameStats.nVertices = NGfx::renderStats.nVertices;
	lastFrameStats.bGeometryThrashing = NGfx::IsGeometryThrashing();
	lastFrameStats.b2DTexturesThrashing = NGfx::Is2DTextureThrashing();
	lastFrameStats.bTransparentThrashing = NGfx::IsTransparentThrashing();
	*pStats = lastFrameStats;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float GetFrameTime()
{
	return lastFrameStats.fFrameTime;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands/Vars
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CmdClearLightmaps( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	crapRecalcLightmaps = LM_CALC_CLEAR;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSwitchTexCache( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	bShow2DTextureCache = false;
	if ( sValue.GetFloat() != 0 )
		bShow2DTextureCache = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSwitchTranspCache( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	bShowTranspTextureCache = false;
	if ( sValue.GetFloat() != 0 )
		bShowTranspTextureCache = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSwitchTranspLMCache( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	bShowParticleLMCache = false;
	if ( sValue.GetFloat() != 0 )
		bShowParticleLMCache = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSwitchLinearCache( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	showLinearCache = SLC_NONE;
	if ( sValue.GetFloat() != 0 )
	{
		if ( sValue.GetFloat() != 1 )
			showLinearCache = SLC_STATIC;
		else
			showLinearCache = SLC_DYNAMIC;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(GSceneInternal)
	REGISTER_CMD( "lmclear", CmdClearLightmaps )
	REGISTER_VAR( "gfx_showcache_2d", VarSwitchTexCache, 0.0f, false )
	REGISTER_VAR( "gfx_showcache_transp", VarSwitchTranspCache, 0.0f, false )
	REGISTER_VAR( "gfx_showcache_transplm", VarSwitchTranspLMCache, 0.0f, false )
	REGISTER_VAR( "gfx_showcache_linear", VarSwitchLinearCache, 0.0f, false )
	REGISTER_VAR( "gfx_lm_recalc", 0, 1, true )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x0251100b, CGScene )
REGISTER_SAVELOAD_CLASS( 0x03111000, CPolyline )
REGISTER_SAVELOAD_CLASS( 0x00661130, CVolumeNode )
REGISTER_SAVELOAD_CLASS( 0x00861121, CSelection )
REGISTER_SAVELOAD_CLASS( 0x11791170, CCombinedPart )
REGISTER_SAVELOAD_CLASS( 0x27041130, CParticles )
REGISTER_SAVELOAD_CLASS( 0x01362120, CLightGroup )
REGISTER_SAVELOAD_CLASS( 0x02662160, CNonePart )
REGISTER_SAVELOAD_CLASS( 0x02662161, CSimplePart )
REGISTER_SAVELOAD_CLASS( 0x02662162, CDiscretePart )
REGISTER_SAVELOAD_CLASS( 0x02662163, CDynamicPart )
REGISTER_SAVELOAD_CLASS( 0x02662164, CAnimatedPart )
REGISTER_SAVELOAD_CLASS( 0x02662165, CDynamicGeometryPart )
REGISTER_SAVELOAD_CLASS( 0x00372110, CGetTranspCache )
REGISTER_SAVELOAD_CLASS( 0x00372140, CFakeParticleLMTexture )