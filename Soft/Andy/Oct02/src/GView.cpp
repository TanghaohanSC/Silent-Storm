#include "StdAfx.h"
#include "GSceneUtils.h"
#include "GScene.h"
#include "GView.h"
#include "GMaterial.h"
#include "GFileSkin.h"
#include "GMesh.h"
#include "GBuilding.h"
#include "GObjectInfo.h"
#include "GTexture.h"
#include "GMemFormat.h"
//#include "MemObject.h"
#include "GMemBuilder.h"
#include "GBind.h"
#include "..\Misc\BasicShare.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataAnimation.h"
#include "GAnimFormat.h"
#include "TerrainInfo.h"
#include "grid.h"
#include "GTerrain.h"
#include "GTerrainTexture.h"
#include "GParticleFormat.h"
#include "GParticles.h"
#include "GGrass.h"
#include "2DScene.h"
#include "Transform.h"
#include "MapBuildingInfo.h"
#include "GfxRender.h" // for GetHardwareOnly()
#include "LSHead.h"
#include "GAnimLight.h"
#include "..\Misc\Commands.h"
#include "..\Misc\LogStream.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAnimation
{
	extern CBasicShare<int, CFileSkeleton> shareSkeletons;
}
namespace NLSHead
{
	extern CBasicShare<int, CHeadMeshLoader> shareHeads;
}
namespace NAI
{
	extern CBasicShare<int, NGScene::CFileAIBind> shareAIBinds;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
static bool bShowFPSStats = false;
////////////////////////////////////////////////////////////////////////////////////////////////////
// share objects
CBasicShare<STextureKey, CFileTexture, STextureKeyHash> shareTextures(103);
//CBasicShare<SPartKey, CFileTriList, SPartHash> shareTrilists(102);
static CBasicShare<int, CFileBind> shareBinds(117);
static CBasicShare<int, CParticlesLoader> shareParticles(119);
//static CBasicShare<SPartKey, CFileSkinScene, SPartHash> shareSkins(105);
static CBasicShare<SPartKey, CObjectInfoLoader, SPartHash> shareObjInfo(121);
//static CBasicShare<SVectorKey, CFileTextureComplex, SVectorKeyHash> shareComplexTextures(130);
static CBasicShare<int, CFileCubeTexture> shareCubeTextures(131);
static CBasicShare<int, CLightLoader> shareAnimLights(139);
static CBasicShare<int, CAIGeometryConverter > shareAIObjInfoConverters(142);
static CBasicShare<int, CAISkinObjectConverter > shareAISkinConverters(143);
#ifdef _DEBUG
static EFogMode defaultFogMode = FOG_NONE;
static EHSRMode defaultHSRMode = HSR_NONE;
static ESceneRenderMode defaultRenderMode = RM_FASTEST;
#else
static EFogMode defaultFogMode = FOG_DYNAMIC;
static EHSRMode defaultHSRMode = HSR_FAST;
static ESceneRenderMode defaultRenderMode = RM_BEST_GF3;
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGameView: public IGameView
{
	OBJECT_BASIC_METHODS(CGameView);
	ZDATA
	CObj<IGScene> pScene;
	list<CPtr<CRenderNode> > nodes; // if object is just created and no refs are stored it should not become leak
	list<CPtr<CBuilding> > buildings;
	list<CPtr<CGrassTracker> > grassTrackers;
	CObj<ILight> pAmbientDirectional;
	CObj<CMaterialShare> pMaterials;
	CColorMaterialShare colorMaterials;
	CTransparentMaterialShare transparentMaterials;
	CObj<CCVec3> pAmbientColor;
	int nCutFloor;
	SFogParams fog;
	EFogMode fogMode;
	bool bShowParticles;
	EHSRMode hsrMode;
	CVec3 vCurrentFogColor;
	CVec3 vDefaultClearColor;
	SHMatrix mPrevView;
	int nSameViewCount;
	bool bHasTerrain;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pScene); f.Add(3,&nodes); f.Add(4,&buildings); f.Add(5,&grassTrackers); f.Add(6,&pAmbientDirectional); f.Add(7,&pMaterials); f.Add(8,&colorMaterials); f.Add(9,&transparentMaterials); f.Add(10,&pAmbientColor); f.Add(11,&nCutFloor); f.Add(12,&fog); f.Add(13,&fogMode); f.Add(14,&bShowParticles); f.Add(15,&hsrMode); f.Add(16,&vCurrentFogColor); f.Add(17,&vDefaultClearColor); f.Add(18,&mPrevView); f.Add(19,&nSameViewCount); f.Add(20,&bHasTerrain); return 0; }
	
	void AddModelPart( CRenderNode *pRes, int nPart, NDb::CGeometry *pGeometry, 
		NDb::CMaterial *pMaterial, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_r );
	void AddModelPart( CRenderNode *pRes, int nPart, NDb::CGeometry *pGeometry, 
		NDb::CMaterial *pMaterial, const SFBTransform &placement, const SRoomInfo &_r );
	void AddSkinModelPart( CRenderNode *pRes, int nPart, NDb::CGeometry *pGeometry, 
		NDb::CMaterial *pMaterial, CFuncBase<SSkeletonMatrices> *pAnimation, const SRoomInfo &_r );
	CRenderNode* NewRenderNode();

	CObjectBase* CreateParticles( bool bLit, CPtrFuncBase<CParticleEffect> *pEffect,
		CFuncBase<SFBTransform> *pPlacement,
		const SBound &bound, const SRoomInfo &_g );
	void Draw( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC );
	void MakeTargetRect( CTRect<float> *pRes, const SDrawInfo &drawInfo );
public:
	CGameView();
	
	virtual IGScene* GetGScene() { return pScene; }
	virtual CLightGroup* CreateLightGroup();
	virtual CRenderNode* CreateMesh( NDb::CModel *pModel, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_g );
	virtual CRenderNode* CreateMesh( CMemObject *pModel, const CVec4 &color, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_g );
	virtual CRenderNode* CreateMesh( NDb::CModel *pModel, const SFBTransform &placement, const SRoomInfo &_g );
	virtual CRenderNode* CreateMesh( CMemObject *pModel, const CVec4 &color, const SFBTransform &placement, const SRoomInfo &_g );
	virtual CRenderNode* CreateSkin( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation, const SRoomInfo &_g );
	virtual CRenderNode* CreateOccluder( NDb::CAIGeometry *pGeom, const SFBTransform &placement );
	virtual CRenderNode* CreateOccluder( NDb::CAIGeometry *pGeom, NDb::CSkeleton *pSkeleton, CFuncBase<NAnimation::SSkeletonPose> *pAnimation );
	virtual CRenderNode* CreateTerrainWall( CPtrFuncBase<CTerrainPart> *pPart, NDb::CTexture *pTexture );
	virtual CRenderNode* CreateTerrainRegion( CFuncBase<STerrainInfo> *pInfo, CVersioningBase *pUpdateRegion, const SRandomSeed &sSeed, const CTRect<int> &sRegion, const list<CObj<CPtrFuncBase<CTerrainPart> > > &partsList, CGrassTracker *pGrass, const SRoomInfo &_g = SRoomInfo() );
	virtual CObjectBase* CreateGrassSector( CGrassAnimator *pEffect, NDb::CTexture *pTexture, CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SRoomInfo &_g = SRoomInfo() );
	virtual CSelectionNode* CreateSelection( const vector<CObjectBase*> &target, const CVec4 &vColor );
	virtual CObjectBase* CreateParticles( NDb::CEffect *pEffect, STime stBeginTime, CFuncBase<STime> *pTime, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_g );
	virtual CBuilding* CreateBuildingPart( int nPartID, const SMapBuilding &info, NBuilding::CBuildingInfoHold *pBI );
	virtual CPolyline* CreatePolyline( const vector<CVec3> &points, const CVec3 &color );
	virtual CObjectBase* CreateExplosion( CFuncBase<STime> *pTime, NDb::CEffect *pEffect, CFuncBase<CExplosionInfo> *pExplosion, const CVec3 &pos, const SRoomInfo &_g );
	virtual CRenderNode* CreateLSHead( NDb::CComplexHead *pHead, CFuncBase<NLSHead::SHeadFrame> *pAnimation, CFuncBase<STime> *pTime, CFuncBase<SFBTransform> *pPlacement, bool bInterface, bool bHasCap, const SRoomInfo &_g );
	virtual void Draw( const SDrawInfo &drawInfo );
	virtual CVec2 GetScreenRect();
	virtual int  GetCutFloor();
	virtual void SetCutFloor( int nFloor );
	virtual bool GetParticleShow() { return bShowParticles; }
	virtual void SetParticleShow( bool bNewState ) { bShowParticles = bNewState; }
	virtual void SetAmbient( const CVec3 &ambientColor );
	//virtual ILight* AddAmbientLight( const CVec3 &ptColor, CRenderNode *pDst );
	virtual ILight* AddDirectionalLight( const CVec3 &ptColor, const CVec3 &ptLight, const CVec3 &ptOrigin, const CVec2 &ptSize, float fMaxHeight, bool bLightmapOnly );
	virtual ILight* AddPointLight( const CVec3 &ptColor, const CVec3 &ptOrigin, float fR, bool bLightmapOnly );
	virtual ILight* AddSpotLight( const CVec3 &ptColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, float fRadius, NDb::CTexture *pMask, bool bLightmapOnly );
	virtual void SetAmbient( NDb::CAmbientLight *pLight, ELightMode lm );
	virtual ESceneRenderMode GetRenderMode() const;
	virtual void SetRenderMode( ESceneRenderMode mode );
	virtual EFogMode GetFogMode() const;
	virtual void SetFogMode( EFogMode mode );
	virtual bool TraceScene( const CRay &r, float *pfT, CVec3 *pNormal, CVec3 *pColor );
	void SetHSRMode( EHSRMode m ) { hsrMode = m; }
	EHSRMode GetHSRMode() const { return hsrMode; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CObjectSet: public CObjectBase
{
public:
	vector< CObj<CObjectBase> > parts;
	//
	void AddPart( CObjectBase *pPart ) { parts.push_back( pPart ); }
	int operator&( CStructureSaver &f ) { f.Add( 1, &parts );	return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRenderNode: public CObjectSet
{
	OBJECT_BASIC_METHODS(CRenderNode);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSelectionNode: public CObjectBase
{
	OBJECT_BASIC_METHODS(CSelectionNode);
private:
	vector< CObj<CObjectBase> > selections;

public:
	CSelectionNode() {}
	CSelectionNode( IGScene* pScene, const vector<CObjectBase*> &target, const CVec4 &vColor );

	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static SGroupInfo GetGroupInfo( const SRoomInfo &r, bool bParticles = false )
{
	if ( IsValid( r.pGroup ) )
		return SGroupInfo( GetGroup( r.pGroup ), GetFloorBit( r.nFloor, r.bShadowCast, bParticles ) );
	return SGroupInfo( 0, GetFloorBit( r.nFloor, r.bShadowCast, bParticles ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static SGroupInfo GetTerrainGroup()
{
	return SGroupInfo( 0, GetFloorBit( 0, true, false ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// IGameView
////////////////////////////////////////////////////////////////////////////////////////////////////
void IGameView::SetNextShadowsMode() 
{
	ESceneRenderMode r = (ESceneRenderMode) (GetRenderMode() + 1);
	if ( r == RM_LAST ) 
		r = RM_FASTEST;
	SetRenderMode( r );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void IGameView::SetNextFogMode()
{
	EFogMode r = (EFogMode) (GetFogMode() + 1);
	if ( r == FOG_LAST ) 
		r = FOG_NONE;
	SetFogMode( r );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void IGameView::SetFastMode()
{
	SetFogMode( FOG_NONE );
	SetRenderMode( RM_FASTEST );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void IGameView::SetNextHSRMode()
{
	EHSRMode r = (EHSRMode) (GetHSRMode() + 1);
	if ( r == HSR_LAST ) 
		r = HSR_NONE;
	SetHSRMode( r );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSelectionNode
////////////////////////////////////////////////////////////////////////////////////////////////////
CSelectionNode::CSelectionNode( IGScene* pScene, const vector<CObjectBase*> &target, const CVec4 &vColor )
{
	for ( int k = 0; k < target.size(); ++k )
	{
		CDynamicCast<CRenderNode> pNode( target[k] );
		if ( !IsValid( pNode ) )
			continue;
		
		for( int nTemp = 0; nTemp < pNode->parts.size(); nTemp++ )
		{
			CObjectBase* pSelection = pScene->CreateSelection( pNode->parts[nTemp], vColor );
			if( pSelection )
				selections.push_back( pSelection );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CSelectionNode::operator&( CStructureSaver &f )
{
	f.Add( 1, &selections );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGameView
////////////////////////////////////////////////////////////////////////////////////////////////////
CGameView::CGameView()
{
	pScene = CreateScene();
	nCutFloor = 5;
	pMaterials = new CMaterialShare;
	SetHSRMode( defaultHSRMode );
	pScene->SetRenderMode( defaultRenderMode );
	SetFogMode( defaultFogMode );
	fog.SetDefaults();
	pAmbientColor = new CCVec3( VNULL3 );
	bShowParticles = true;
	vCurrentFogColor = CVec3( 0.25f, 0.25f, 0.25f );
	vDefaultClearColor = CVec3( 0.25f, 0.25f, 0.25f );
	Zero( mPrevView );
	bHasTerrain = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CLightGroup* CGameView::CreateLightGroup()
{
	return pScene->CreateLightGroup();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::NewRenderNode()
{
	CRenderNode *pRes = new CRenderNode;
	nodes.push_back( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::AddModelPart( CRenderNode *pRes, int nPart, NDb::CGeometry *pGeometry, 
	NDb::CMaterial *pMaterial, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_r )
{
	if ( !IsValid( pMaterial ) )
		return;
	SPartKey key;
	key.nID = pGeometry->GetRecordID();
	key.nPart = nPart;
	if( CResourceFileOpener::DoesExist( "Geometries", key ) )
	{
		CObjectBase *pPart = pScene->CreateGeometry( 
			shareObjInfo.Get( key ),
			pMaterials->CreateMaterial( pMaterial ),
			pPlacement, GetGroupInfo(_r) );
		pRes->AddPart( pPart );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::AddModelPart( CRenderNode *pRes, int nPart, NDb::CGeometry *pGeometry, 
	NDb::CMaterial *pMaterial, const SFBTransform &placement, const SRoomInfo &_r )
{
	if ( !IsValid( pMaterial ) )
		return;
	SPartKey key;
	key.nID = pGeometry->GetRecordID();
	key.nPart = nPart;
	if( CResourceFileOpener::DoesExist( "Geometries", key ) )
	{
		CObjectBase *pPart = pScene->CreateGeometry( 
			shareObjInfo.Get( key ),
			pMaterials->CreateMaterial( pMaterial ),
			placement, GetGroupInfo(_r) );
		pRes->AddPart( pPart );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::AddSkinModelPart( CRenderNode *pRes, int nPart, NDb::CGeometry *pGeometry, 
	NDb::CMaterial *pMaterial, CFuncBase<SSkeletonMatrices> *pAnimation, const SRoomInfo &_r )
{
	if ( !IsValid( pMaterial ) )
		return;
	SPartKey key;
	key.nID = pGeometry->GetRecordID();
	key.nPart = nPart;
	if( CResourceFileOpener::DoesExist( "Geometries", key ) )
	{
		CObjectBase *pPart = pScene->CreateGeometry(
			shareObjInfo.Get( key ), pMaterials->CreateMaterial( pMaterial ), pAnimation, GetGroupInfo(_r) );
		pRes->AddPart( pPart );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateTerrainWall( CPtrFuncBase<CTerrainPart> *pPart, NDb::CTexture *pTexture )
{
	IMaterial *pMaterial;
	if ( pTexture )
		pMaterial = CreateMaterial( CVec3(0.4f,0.4f,0.1f), shareTextures.Get( STextureKey( pTexture->GetRecordID(), false ) ) );
	else
		pMaterial = CreateMaterial( CVec3(0.4f,0.4f,0.1f) );

	NGScene::CTerrainModelWallPart *pTerrainPart = new NGScene::CTerrainModelWallPart;
	pTerrainPart->pPart = pPart;
	CRenderNode *pRes = NewRenderNode();
	pRes->AddPart( pScene->CreateGeometry( pTerrainPart, pMaterial, GetTerrainGroup() ) );
	pRes->AddPart( pScene->CreateGeometry( pTerrainPart, pMaterials->CreateOccluderMaterial(), SGroupInfo() ) );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateTerrainRegion( CFuncBase<STerrainInfo> *pInfo, CVersioningBase *pUpdateRegion, const SRandomSeed &sSeed, const CTRect<int> &sRegion, const list<CObj<CPtrFuncBase<CTerrainPart> > > &partsList, CGrassTracker *pGrass, const SRoomInfo &_g )
{
	bHasTerrain = true;
	CVec3 ptCenter( ( sRegion.x1 + sRegion.x2 ) * 0.5f * FP_GRID_STEP, ( sRegion.y1 + sRegion.y2 ) * 0.5f * FP_GRID_STEP,	10 );
	CTerrainTexture *pTerrainBump = new CTerrainTexture( true, sSeed, sRegion, new CLODCalcer( ptCenter, pScene->GetCamera() ), pInfo, pUpdateRegion, pGrass );
	CTerrainTexture *pTerrainTexture = new CTerrainTexture( false, sSeed, sRegion, new CLODCalcer( ptCenter, pScene->GetCamera() ), pInfo, pUpdateRegion, pGrass );

	CPtr<IMaterial> pMaterial = CreateMaterial( CalcAverageColor( pInfo, sRegion ), pTerrainTexture, pTerrainBump );
	//pMaterial->light = CMaterial::SC_NORMAL; 

	CRenderNode *pRes = NewRenderNode();
	for ( list<CObj<CPtrFuncBase<CTerrainPart> > >::const_iterator iTemp = partsList.begin(); iTemp != partsList.end(); iTemp++ )
	{
		CObj<NGScene::CTerrainModelPart> pTerrainPart = new NGScene::CTerrainModelPart;
		pTerrainPart->pInfo = pInfo;
		pTerrainPart->pPart = (*iTemp);
		pTerrainPart->nrRegion = sRegion;

		pRes->AddPart( pScene->CreateGeometry( pTerrainPart, pMaterial, GetTerrainGroup() ) );
		pRes->AddPart( pScene->CreateGeometry( pTerrainPart, pMaterials->CreateOccluderMaterial(), SGroupInfo() ) );
	}
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGameView::CreateGrassSector( CGrassAnimator *pEffect, NDb::CTexture *pTexture, CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SRoomInfo &_g )
{
	SRoomInfo r(_g);
	r.bShadowCast = false;
	pEffect->SetTexture( shareTextures.Get( pTexture->GetRecordID() ) );
	return CreateParticles( true, pEffect, pPlacement, bound, r );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateMesh( NDb::CModel *pModel, CFuncBase<SFBTransform> *pPlacement, 
	const SRoomInfo &_r )
{
	CPtr<NDb::CModel> pHoldModel( pModel );
	if ( pPlacement == 0 )
	{
		SFBTransform identity;
		Identity( &identity.forward );
		Identity( &identity.backward );
		return CreateMesh( pModel, identity, _r );
	}
	CRenderNode *pRes = NewRenderNode();
	for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
		AddModelPart( pRes, i, pModel->pGeometry, pModel->pMaterials[i], pPlacement, _r );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateMesh( CMemObject *pModel, const CVec4 &color, 
	CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_r )
{
	CRenderNode *pRes = NewRenderNode();
	IMaterial *pMat = color.w < 1 ? transparentMaterials.CreateMaterial( color ) 
		: colorMaterials.CreateMaterial( CVec3( color.r, color.g, color.b) );
	CObjectBase *pPart;
	if ( pPlacement )
		pPart = pScene->CreateGeometry( CreateObjectInfo( pModel ), pMat, pPlacement, GetGroupInfo(_r) );
	else
		pPart = pScene->CreateGeometry( CreateObjectInfo( pModel ), pMat, GetGroupInfo(_r) );
	pRes->AddPart( pPart );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateMesh( NDb::CModel *pModel, const SFBTransform &placement, 
	const SRoomInfo &_r )
{
	CPtr<NDb::CModel> pHoldModel( pModel );
	if ( !IsValid( pModel->pGeometry ) )
		return 0;
	CRenderNode *pRes = NewRenderNode();
	for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
		AddModelPart( pRes, i, pModel->pGeometry, pModel->pMaterials[i], placement, _r );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateMesh( CMemObject *pModel, const CVec4 &color, const SFBTransform &placement, 
	const SRoomInfo &_r )
{
	CRenderNode *pRes = NewRenderNode();
	IMaterial *pMat = color.w < 1 ? transparentMaterials.CreateMaterial( color ) 
		: colorMaterials.CreateMaterial( CVec3( color.r, color.g, color.b) );
	CObjectBase *pPart = pScene->CreateGeometry(
		CreateObjectInfo( pModel ),
		pMat,
		placement, GetGroupInfo(_r) );
	pRes->AddPart( pPart );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateSkin( NDb::CModel *pModel, 
	CFuncBase<NAnimation::SSkeletonPose> *pAnimation, const SRoomInfo &_r )
{
	CPtr<NDb::CModel> pHoldModel( pModel );
	if ( pAnimation && !IsValid( pModel->pSkeleton ) )
	{
		ASSERT( 0 ); // animation for non skinned model
		pAnimation = 0;
	}
	if ( !pAnimation )
	{
		ASSERT( 0 ); // deep shit, we could create non skinned version
		return 0;
	}
	NGScene::CBind *pBind = new NGScene::CBind;
	pBind->pBinds = shareBinds.Get( pModel->pGeometry->GetRecordID() );
	pBind->pAnimation = pAnimation;
	pBind->pSkeleton = NAnimation::shareSkeletons.Get( pModel->pSkeleton->GetRecordID() );
	CRenderNode *pRes = NewRenderNode();
	for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
		AddSkinModelPart( pRes, i, pModel->pGeometry, pModel->pMaterials[i], pBind, _r );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateOccluder( NDb::CAIGeometry *pGeom, const SFBTransform &placement )
{
	if ( !pGeom )
		return 0;
	CRenderNode *pRes = NewRenderNode();
	IMaterial *pMat = pMaterials->CreateOccluderMaterial();
	CObjectBase *pPart = pScene->CreateGeometry(
		shareAIObjInfoConverters.Get( pGeom->GetRecordID() ),
		pMat, 
		placement, SGroupInfo() );
	pRes->AddPart( pPart );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateOccluder( NDb::CAIGeometry *pGeom, NDb::CSkeleton *pSkeleton, CFuncBase<NAnimation::SSkeletonPose> *pAnimation )
{
	NGScene::CBind *pBind = new NGScene::CBind;
	pBind->pBinds = NAI::shareAIBinds.Get( pGeom->GetRecordID() );
	pBind->pAnimation = pAnimation;
	pBind->pSkeleton = NAnimation::shareSkeletons.Get( pSkeleton->GetRecordID() );
	CRenderNode *pRes = NewRenderNode();
	CObjectBase *pPart = pScene->CreateGeometry(
		shareAISkinConverters.Get( pGeom->GetRecordID() ),
		pMaterials->CreateOccluderMaterial(),
		pBind, SGroupInfo() );
	pRes->AddPart( pPart );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CSelectionNode* CGameView::CreateSelection( const vector<CObjectBase*> &target, const CVec4 &vColor )
{
	return new CSelectionNode( pScene, target, vColor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGameView::CreateParticles( bool bLit, CPtrFuncBase<CParticleEffect> *pEffect, 
	CFuncBase<SFBTransform> *pPlacement, 
	const SBound &bound, const SRoomInfo &_r )
{
	if ( bLit )
	{
		//IMaterial *pMat = CreateLitParticleMaterial( pTex, pBumpTex );
		return pScene->CreateLitParticles( pEffect, 0, pPlacement, bound, GetGroupInfo(_r, _r.bTreeCrown) );
	}
	else
		return pScene->CreateParticles( pEffect, pPlacement, bound, GetGroupInfo(_r) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
static void InitParticleTextures( T *pAnimator, NDb::CParticleInstance *pInstance )
{
	pAnimator->textureIDs.resize( NDb::N_PARTICLE_TEXTURES );//pInstance->pTextures)
	int nLast = -1;
	for ( int i = 0; i < pAnimator->textureIDs.size(); ++i )
	{
		if ( !IsValid( pInstance->pTextures[i] ) )
			continue;
		nLast = i;
		pAnimator->textureIDs[i] = shareTextures.Get( pInstance->pTextures[i]->GetRecordID() );
	}
	pAnimator->textureIDs.resize( nLast + 1 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGameView::CreateParticles( NDb::CEffect *pEffect, STime stBeginTime, 
	CFuncBase<STime> *pTime, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_r )
{
	if ( pEffect->instances.empty() && pEffect->lights.empty() )
		return 0;
	CRenderNode *pRes = NewRenderNode();
	for ( int i = 0; i < pEffect->instances.size(); ++i )
	{
		NDb::CParticleInstance *pInstance = pEffect->instances[i];
		NDb::CParticle *pParticle = pInstance->pParticle;
		if ( !pParticle )
			continue;

		CParticleAnimator *pAnimator = new NGScene::CParticleAnimator( pInstance, stBeginTime );
		pAnimator->pInfo = shareParticles.Get( pParticle->GetRecordID() );
		pAnimator->pTime = pTime;
		SFBTransform trans;
		CVec3 scale = CVec3( pInstance->fScale, pInstance->fScale, pInstance->fScale );
		MakeMatrix( &trans.forward, pInstance->position, pInstance->rotation, scale );
		trans.backward.HomogeneousInverse( trans.forward );
		CCFBTransform *pTrans = new CCFBTransform( trans );
		CMSRNode *pMSR = new CMSRNode;
		pMSR->pAncestor = pPlacement;
		pMSR->pPos = pTrans;
		pAnimator->pPlacement = pMSR;
		InitParticleTextures( pAnimator, pInstance );
		bool bLit = (pInstance->light == NDb::CParticleInstance::L_LIT);
		pRes->AddPart( CreateParticles( bLit, pAnimator, pMSR, pParticle->bound, _r ) );
	}
	for ( int i = 0; i < pEffect->lights.size(); ++i )
	{
		NDb::CLightInstance *pInstance = pEffect->lights[i];
		NDb::CAnimLight *pLight = pInstance->pLight;
		if ( !pLight )
			continue;
		CLightAnimator *pAnimator = new NGScene::CLightAnimator( pInstance, stBeginTime, pInstance->fScale );
		pAnimator->pInfo = shareAnimLights.Get( pLight->GetRecordID() );
		pAnimator->pTime = pTime;
		SFBTransform trans;
		CVec3 scale = CVec3( pInstance->fScale, pInstance->fScale, pInstance->fScale );
		MakeMatrix( &trans.forward, pInstance->position, pInstance->rotation, scale );
		trans.backward.HomogeneousInverse( trans.forward );
		CCFBTransform *pTrans = new CCFBTransform( trans );
		CMSRNode *pMSR = new CMSRNode;
		pMSR->pAncestor = pPlacement;
		pMSR->pPos = pTrans;
		pAnimator->pPlacement = pMSR;

		pRes->AddPart( CastToObjectBase( pScene->AddPointLight( pAnimator ) ) );
	}
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CGameView::CreateExplosion( CFuncBase<STime> *pTime, NDb::CEffect *pEffect,
	CFuncBase<CExplosionInfo> *pExplosion, const CVec3 &pos, const SRoomInfo &_r )
{
	if ( pEffect->instances.empty() )
		return 0;
	NDb::CParticleInstance *pInstance = pEffect->instances[0];
	NDb::CParticle *pParticle = pInstance->pParticle;
	if ( !pParticle )
		return 0;

	CExplosionAnimator *pAnimator = new NGScene::CExplosionAnimator( pInstance );
	pAnimator->pInfo = shareParticles.Get( pParticle->GetRecordID() );
	pAnimator->pTime = pTime;
	pAnimator->pExplosion = pExplosion;
	CCFBTransform *pPos = new CCFBTransform( MakeTransform( pos ) );
	bool bLit = (pInstance->light == NDb::CParticleInstance::L_LIT);
	InitParticleTextures( pAnimator, pInstance );
	SBound bound;
	bound.SphereInit( pos, 10.f ); // CRAP
	return CreateParticles( bLit, pAnimator, pPos, bound, _r );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CBuilding* CGameView::CreateBuildingPart( int nPartID, const SMapBuilding &info, NBuilding::CBuildingInfoHold *pBI )
{
	CBuilding *pB = new CBuilding;
	//
	pB->Setup( nPartID, info, pBI );

	//vector<SRoomAmbient> rooms;
	//pB->GetRoomLights( &rooms );
	//for( int i = 0; i < rooms.size(); ++i )
	//{
	//	CLightCalcer *pLCalcer = new CLightCalcer( info.pVariant->GetRecordID(), rooms[i].nTargetGroupID, pAmbientColor, rooms[i].pAmbientColor, info.pGrid );
	//	pScene->SetRoomAmbient( rooms[i].nTargetGroupID, pLCalcer );
	//}
	pB->Update( pScene, pMaterials.GetPtr() );
	buildings.push_back( pB );
	return pB;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CPolyline* CGameView::CreatePolyline( const vector<CVec3> &points, const CVec3 &cr )
{
	return pScene->CreatePolyline( new CMemGeometry( points ), cr );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGameView::CreateLSHead( NDb::CComplexHead *pCHead, CFuncBase<NLSHead::SHeadFrame> *pAnimation,
	CFuncBase<STime> *pTime, CFuncBase<SFBTransform> *pPlacement, bool bInterface, bool bHasCap, const SRoomInfo &_r )
{
	SRand rnd;
	CRenderNode *pRes = NewRenderNode();
	if ( !pCHead )
		return pRes;
	if ( IsValid( pCHead->pHead ) )
	{
		NGScene::CMSRConvert *pConvert = new NGScene::CMSRConvert;
		pConvert->pMove = new NGScene::CCVec3( CVec3( 0, -0.035f, 0 ) );
		pConvert->pScale = new NGScene::CCVec3( CVec3( 0.014f, 0.014f, 0.014f ) );
		pConvert->pRotate = new NGScene::CCVec3( CVec3( 0, -FP_PI2, 0 ) );
		NGScene::CMSRNode *pMSR = new NGScene::CMSRNode;
		pMSR->pAncestor = pPlacement;
		pMSR->pPos = pConvert;

		NDb::CHead *pHead = pCHead->pHead;
		CPtr<NLSHead::CHead> pMesh = new NLSHead::CHead( pMSR, pAnimation, NLSHead::shareHeads.Get( pHead->GetRecordID() ) );
		CPtr<NLSHead::CHeadBound> pBound = new NLSHead::CHeadBound( pPlacement );
		if ( IsValid( pHead->pMaterial ) )
		{
			CObjectBase *pPart = pScene->CreateDynamicGeometry( pMesh,
				pMaterials->CreateMaterial( pHead->pMaterial->GetMaterial( &rnd ) ), pBound, GetGroupInfo(_r) );
			pRes->AddPart( pPart );
		}
	}

	vector< CPtr<NDb::CModel> > models;

	if ( bInterface || !bHasCap )
	{
		if ( IsValid( pCHead->pHair ) )
			models.push_back( pCHead->pHair->CreateModel(&rnd) );
	}
	else
	{
		if ( IsValid( pCHead->pHairInCap ) )
			models.push_back( pCHead->pHairInCap->CreateModel(&rnd) );
		else if ( IsValid( pCHead->pHair ) )
			models.push_back( pCHead->pHair->CreateModel(&rnd) );
	}

	if ( bInterface )
	{
		for ( int i = 0; i < NDb::N_HEAD_MESHES; ++i )
			if ( IsValid( pCHead->pIFMeshes[i] ) )
				models.push_back( pCHead->pIFMeshes[i]->CreateModel(&rnd) );
	}
	else
	{
		for ( int i = 0; i < NDb::N_HEAD_MESHES; ++i )
			if ( IsValid( pCHead->pMeshes[i] ) )
				models.push_back( pCHead->pMeshes[i]->CreateModel(&rnd) );
	}

	for ( int j = 0; j < models.size(); ++j )
	{
		NDb::CModel *pModel = models[j];
		for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
			AddModelPart( pRes, i, pModel->pGeometry, pModel->pMaterials[i], pPlacement, _r );
	}

	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::Draw( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC )
{
	//for ( list<CPtr<CGrassTracker> >::iterator i = grassTrackers.begin(); i != grassTrackers.end(); ++i )
	//	(*i)->Update();
	nodes.clear();
	fog.fTime = GetTickCount() / 1024.0f;
	SGroupSelect mask( GetFloorMask( nCutFloor ), GetParticlesRequireFlag( bShowParticles ) );
	EFogMode usedFog = fogMode;
	if ( usedFog == FOG_DYNAMIC && NGfx::GetHardwareLevel() < NGfx::HL_GFORCE3 )
		usedFog = FOG_PERVERTEX;
	pScene->Draw( pTS, pClipTS, pRC, mask, usedFog, fog, hsrMode );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::MakeTargetRect( CTRect<float> *pRes, const SDrawInfo &drawInfo )
{
	CTRect<float> &rFullScreen = *pRes;
	CVec2 vSize = pScene->GetScreenRect();
	rFullScreen.x1 = vSize.x * drawInfo.vOrigin.x; 
	rFullScreen.x2 = rFullScreen.x1 + vSize.x * drawInfo.vSize.x;
	rFullScreen.y1 = vSize.y * drawInfo.vOrigin.y; 
	rFullScreen.y2 = rFullScreen.y1 + vSize.y * drawInfo.vSize.y;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void MakeClipTS( CTransformStack *pRes, const CTransformStack &ts, const CVec2 &vOrigin, const CVec2 &vSize )
{
	const SFBTransform &projection = ts.GetProjection();
	SHMatrix m = projection.forward;
	m.x = m.x * (1.0f / vSize.x) + m.w * ( ( 1 - 2 * vOrigin.x - vSize.x ) / vSize.x );
	m.y = m.y * (1.0f / vSize.y) + m.w * ( (-1 + 2 * vOrigin.y + vSize.y ) / vSize.y );
	pRes->Make( m );
	pRes->Push43( projection.backward * ts.Get().forward );
}
void CGameView::Draw( const SDrawInfo &drawInfo )
{
	if ( bHasTerrain )
	{
		if ( memcmp( &drawInfo.pTS->Get().forward, &mPrevView, sizeof(mPrevView) ) != 0 )
		{
			mPrevView = drawInfo.pTS->Get().forward;
			nSameViewCount = 0;
		}
		else
			++nSameViewCount;
		SetTerrainStressMode( nSameViewCount < 5 );
	}
	bool bUseRegister = pScene->GetRenderMode() >= RM_BEST_GF2;
	NGfx::CRenderContext rc;
	if ( bUseRegister )
		rc.SetVirtualRT();
	if ( drawInfo.bOverlay )
	{
		if ( bUseRegister )
			NGScene::Clear( &rc, CVec3(0,0,0) );
	}
	else
	{
		if ( drawInfo.bUseDefaultClearColor )
      NGScene::Clear( &rc, vDefaultClearColor );
		else
			NGScene::Clear( &rc, drawInfo.vClearColor );
	}
	CTransformStack tsClip;
	MakeClipTS( &tsClip, *drawInfo.pTS, drawInfo.vOrigin, drawInfo.vSize );
	if ( bUseRegister )
		Draw( &tsClip, &tsClip, &rc );
	else
		Draw( drawInfo.pTS, &tsClip, &rc );
	if ( bUseRegister )
	{
		//if ( drawInfo.bOverlay )
		CTRect<float> rFullScreen;
		MakeTargetRect( &rFullScreen, drawInfo );
		CopyRegisterOnScreen( rFullScreen, drawInfo.bOverlay );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec2 CGameView::GetScreenRect()
{
	return pScene->GetScreenRect();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::SetAmbient( const CVec3 &ambientColor )
{
	pScene->SetAmbient( ambientColor );
	pAmbientColor->Set( ambientColor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ILight* CGameView::AddDirectionalLight( const CVec3 &ptColor, const CVec3 &ptLight, const CVec3 &ptOrigin, const CVec2 &ptSize, float fMaxHeight, bool bLightmapOnly ) 
{
	CCVec3 *pCColor = new CCVec3( ptColor );
	return pScene->AddDirectionalLight( pCColor, pCColor, CVec3(0,0,0), ptLight, ptOrigin, ptSize, fMaxHeight, bLightmapOnly );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ILight* CGameView::AddPointLight( const CVec3 &ptColor, const CVec3 &ptOrigin, float fR, bool bLightmapOnly )
{
	return pScene->AddPointLight( ptColor, ptOrigin, fR, bLightmapOnly );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ILight* CGameView::AddSpotLight( const CVec3 &ptColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, 
	float fRadius, NDb::CTexture *pMask, bool bLightmapOnly )
{
	if ( pMask )
		return pScene->AddSpotLight( new CCVec3(ptColor), ptOrigin, ptDir, fFOV, fRadius, shareTextures.Get( pMask->GetRecordID() ), bLightmapOnly );
	return pScene->AddSpotLight( new CCVec3(ptColor), ptOrigin, ptDir, fFOV, fRadius, 0, bLightmapOnly );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CheckCircularGF2LightLink( NDb::CAmbientLight *pLight )
{
	if ( pLight && pLight->pGF2Light )
		pLight->pGF2Light->pGF2Light = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::SetAmbient( NDb::CAmbientLight *pLight, ELightMode lm )
{
	if ( !IsValid( pLight ) )
	{
		pAmbientDirectional = 0;
		pScene->SetAmbient( CVec3(0.5f, 0.5f, 0.5f) );
		pAmbientColor->Set( CVec3(0.5f, 0.5f, 0.5f) );
		pMaterials->SetSky( 0 );
		return;
	}
	vDefaultClearColor =  pLight->vBackColor;
	CheckCircularGF2LightLink( pLight );
	if ( NGfx::GetHardwareLevel() < NGfx::HL_GFORCE3 && pLight->pGF2Light )
	{
		SetAmbient( pLight->pGF2Light, lm );
		return;
	}
	CVec3 vAmbientColor( pLight->vAmbientColor ), vLightColor( pLight->vLightColor );
	vAmbientColor.Minimize( CVec3( 1,1,1 ) );
	vLightColor.Minimize( CVec3( 1,1,1 ) );
	// directional light
	CVec3 vLightDir;
	float fHor = sin( ToRadian( pLight->fPitch ) );
	vLightDir.z = -cos( ToRadian( pLight->fPitch ) );
	vLightDir.x = fHor * cos( ToRadian( pLight->fYaw ) );
	vLightDir.y = fHor * sin( ToRadian( pLight->fYaw ) );
	if ( lm == LT_ZONE )
		pAmbientDirectional = pScene->AddDirectionalLight( 
			new CCVec3( vLightColor ), new CCVec3( pLight->vGlossColor ), 
			pLight->vShadowColor,
			vLightDir, CVec3(0,0,0), CVec2( 150, 150 ), 20, false );
	else
		pAmbientDirectional = pScene->AddDirectionalLight( 
			new CCVec3( vLightColor ), new CCVec3( pLight->vGlossColor ), 
			pLight->vShadowColor,
			vLightDir, CVec3(0,0,1), CVec2(1,1), 3, false );
	// fog
	fog.fDist = pLight->fFogDistance;
	fog.vFogColor = pLight->vFogColor;
	fog.fHeight = pLight->fVapourHeight;
	fog.fDensity = pLight->fVapourDensity;
	fog.vWaterColor = pLight->vVapourColor;
	fog.fCameraHeight = 10;
	fog.fVapourNoiseParam = pLight->fVapourNoiseParam;
	fog.fVapourSpeed = pLight->fVapourSpeed;
	fog.fVapourSwitchTime = pLight->fVapourSwitchTime;
	fog.fTime = 0;
	fog.fDistStart = pLight->fFogStartDistance;
	vCurrentFogColor = fog.vFogColor;
	// ambient light
	pScene->SetAmbient( vAmbientColor );
	pAmbientColor->Set( vAmbientColor );
	if ( IsValid( pLight->pSky ) )
		pMaterials->SetSky( shareCubeTextures.Get( pLight->pSky->GetRecordID() ) );
	else
		pMaterials->SetSky( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::SetRenderMode( ESceneRenderMode mode )
{
	pScene->SetRenderMode( mode );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ESceneRenderMode CGameView::GetRenderMode() const
{
	return pScene->GetRenderMode();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EFogMode CGameView::GetFogMode() const
{
	return fogMode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::SetFogMode( EFogMode mode )
{
	fogMode = mode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGameView::TraceScene( const CRay &r, float *pfT, CVec3 *pNormal, CVec3 *pColor )
{
	return pScene->TraceScene( r, pfT, pNormal, pColor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CGameView::GetCutFloor()
{
	return nCutFloor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGameView::SetCutFloor( int _nFloor )
{
	nCutFloor = Clamp( _nFloor, -3, 5 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
IGameView* CreateNewView()
{
	return new CGameView;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IGameView* CreateNewFastInterfaceView()
{
	CGameView *pRes = new CGameView;
	pRes->GetGScene()->SetLightmaps( false );
	pRes->SetFastMode();
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands/Vars
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSetHSR( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	defaultHSRMode = HSR_NONE;
	if ( sValue.GetFloat() != 0 )
		defaultHSRMode = HSR_FAST;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSetFog( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	defaultFogMode = FOG_NONE;
	if ( sValue.GetFloat() == 2 )
		defaultFogMode = FOG_DYNAMIC;
	else if ( sValue.GetFloat() == 1 )
		defaultFogMode = FOG_PERVERTEX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSetShadows( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	defaultRenderMode = RM_FASTEST;//SHADOW_NOSHADOWS;
	if ( sValue.GetFloat() == 2 )
		defaultRenderMode = RM_BEST_GF3;//SHADOW_FULL;
	else if ( sValue.GetFloat() == 1 )
		defaultRenderMode = RM_BEST_GF2;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(GView)
	REGISTER_VAR( "gfx_hsr", VarSetHSR, 0, true )
	REGISTER_VAR( "gfx_fog", VarSetFog, 0, true )
	REGISTER_VAR( "gfx_shadows", VarSetShadows, 0, true )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x01741140, CGameView )
REGISTER_SAVELOAD_CLASS( 0x01741141, CRenderNode )
REGISTER_SAVELOAD_CLASS( 0x01741142, CSelectionNode )
