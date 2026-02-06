#include "StdAfx.h"
#include "wInterface.h"
#include "GView.h"
#include "GSceneUtils.h"
#include "Transform.h"
#include "RPGUnit.h"
#include "RWGame.h"
#include "GAnimFormat.h"
#include "GAnimation.h"
#include "GAnimPath.h"
#include "..\Misc\BasicShare.h"
#include "TerrainInfo.h"
#include "GTerrain.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataAnimation.h"
#include "GMatShare.h"
#include "wInterface.h"
#include "InventoryUnit.h"
///
#include "GMaterial.h"

#include "GGrass.h"
#include "GParticles.h"
#include "GParticleInfo.h"

#include "RPGUnitInfo.h"
#include "RPGItemInfo.h"

#include "LSHead.h"
#include "LSController.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "MemObject.h"
vector<SSphere> sphereParticles;	// test sphere visualization
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRender
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_FOV = 60;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSelection: public CObjectBase
{
	OBJECT_BASIC_METHODS( CSelection );
public:
	ZDATA
	CVec4 vColor;
	CObj<NGScene::CSelectionNode> pSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&vColor); f.Add(3,&pSelection); return 0; }

	CSelection() {}
	CSelection( const CVec4 &_vColor, NGScene::CSelectionNode *_pSelection ): vColor( _vColor ), pSelection( _pSelection ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSetRender: public COrdinarySyncDst<NWorld::IVisObj,CSetRender>, public NWorld::IRenderVisitor
{
	typedef COrdinarySyncDst<NWorld::IVisObj,CSetRender> TParent;
	typedef hash_map<CPtr<NWorld::IVisObj>, CPtr<CSelection>, SPtrHash> CSelectionHash;
	ZDATA_(TParent)
	CPtr<NGScene::IGameView> pScene;
	CPtr<NGScene::CGrass> pGrass;
	CPtr<CFuncBase<STime> > pTime, pAimTime;
	vector<CPtr<NWorld::IVisObj> > objects;
	CSelectionHash selections;
	CPtr<NLSHead::CHeadsController> pHeadsController;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TParent*)this); f.Add(2,&pScene); f.Add(3,&pGrass); f.Add(4,&pTime); f.Add(5,&pAimTime); f.Add(6,&objects); f.Add(7,&selections); f.Add(8,&pHeadsController); return 0; }
private:
	virtual void PostVisit( int nID, NWorld::IVisObj *pObject );
	CSelection* CreateSelection( int nID, const CVec4 &vColor, CSelection *pSource = 0 );
public:
	CSetRender() {}
	CSetRender( CSyncSrc<NWorld::IVisObj> *pSrc, NGScene::IGameView *_pScene )
		: TParent(pSrc), pScene(_pScene) {}
	virtual void SetNewSource( CSyncSrc<NWorld::IVisObj> *_pSrc );
	void SetTimer( CFuncBase<STime> *_pTime, CFuncBase<STime> *_pAimTime ) { pTime = _pTime; pAimTime = _pAimTime; }
	void SetHeadsController( NLSHead::CHeadsController *_pHeadsController ) { pHeadsController = _pHeadsController; }
	void SetGrass( NGScene::CGrass *_pGrass ) { pGrass = _pGrass; }
	virtual NGScene::CLightGroup* MakeGroup();
	virtual void AddParticleEffect( STime tBegin, NDb::CEffect *pEffect, CFuncBase<SFBTransform> *pPosition );
	virtual void AddPointLight( const CVec3 &ptColor, const CVec3 &ptOrigin, float fRadius, bool bLightmapOnly );
	virtual void AddSpotLight( const CVec3 &ptColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, float fRadius, NDb::CTexture *pMask, bool bLightmapOnly );
	virtual void AddMesh( NDb::CModel *pModel, const SFBTransform &position, NGScene::CLightGroup *pGroup, int nFloor );
	virtual void AddMesh( CMemObject *pModel, const CVec4 &color, const SFBTransform &position );
	virtual void AddItemMesh( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation );
	virtual void AddMesh( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation, CFuncBase<NAnimation::SSkeletonState> *pState, const vector<SBoundMesh> &boundMeshes, NGScene::CLightGroup *pGroup, int nFloor, NWorld::CUnit *pUnit = 0 );
	virtual void AddBuildingPart( int nPartID, const SMapBuilding &info, NBuilding::CBuildingInfoHold *pBI );
	virtual void AddTerrainParts( const SRandomSeed &sSeed, const CTRect<int> &sRegion, const list<CObj<CPtrFuncBase<CTerrainPart> > > &partsList, CFuncBase<STerrainInfo> *pInfo, CVersioningBase *pUpdateRegion );
	virtual void AddTerrainWallPart( CPtrFuncBase<CTerrainPart> *pPart, NDb::CTexture *pTexture );
	virtual void AddGrass( CFuncBase<STerrainInfo> *pInfo );
	virtual void AddGrassEvent( const CVec3 &ptPlace );
	virtual void AddExplosion( NDb::CEffect *pEffect, CFuncBase<NGScene::CExplosionInfo> *pExplosion, const CVec3 &pos );
	virtual void AddPolyline( const vector<CVec3> &points, const CVec3 &cr );
	virtual void AddHead( NWorld::CUnit *pUnit, CFuncBase<SFBTransform> *pPosition, const NGScene::SRoomInfo &room );
	virtual void AddHead( NDb::CComplexHead *pHead, CFuncBase<SFBTransform> *pPosition, const NGScene::SRoomInfo &room );
	virtual void AddOccluder( NDb::CAIGeometry *pAIGeom, const SFBTransform &pos );
	virtual void AddOccluder( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, CFuncBase<NAnimation::SSkeletonPose> *pAnimation );
	//
	CObjectBase* Select( NWorld::IVisObj *pSelect, const CVec4 &vColor = CVec4( 0, 1, 1, 1 ) );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSetRender
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::SetNewSource( CSyncSrc<NWorld::IVisObj> *_pSrc )
{
	TParent::SetNewSource( _pSrc );
	objects.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CSelection* CSetRender::CreateSelection( int nID, const CVec4 &vColor, CSelection *pSource )
{
	const vector<CObj<CObjectBase> > &ob = GetObjects( nID );
	vector<CObjectBase*> t;
	for ( int k = 0; k < ob.size(); ++k )
		t.push_back( ob[k] );

	if ( pSource )
	{
		pSource->vColor = vColor;
		pSource->pSelection = pScene->CreateSelection( t, vColor );
		return pSource;
	}
	return new CSelection( vColor, pScene->CreateSelection( t, vColor ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::PostVisit( int nID, NWorld::IVisObj *pObject )
{
	if ( nID >= objects.size() )
		objects.resize( nID + 1 );
	objects[nID] = pObject;
	CSelectionHash::iterator i = selections.find( pObject );
	if ( i != selections.end() && IsValid( i->second ) )
		CreateSelection( nID, i->second->vColor, i->second );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CSetRender::Select( NWorld::IVisObj *pSelect, const CVec4 &_vColor )
{
	CSelectionHash::iterator iTemp = selections.find( pSelect );
	if ( iTemp != selections.end() )
	{
		if ( IsValid( iTemp->second ) && iTemp->second->vColor == _vColor )
			return iTemp->second;
	}
	for ( int k = 0; k < objects.size(); ++k )
	{
		if ( objects[k] == pSelect )
		{
			CSelection *pRes = CreateSelection( k, _vColor );
			selections[pSelect] = pRes;
			return pRes;
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NGScene::CLightGroup* CSetRender::MakeGroup()
{
	NGScene::CLightGroup *pRes = pScene->CreateLightGroup();
	Register( pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddParticleEffect( STime tBegin, NDb::CEffect *pEffect, CFuncBase<SFBTransform> *pPosition )
{
	if ( !IsValid( pEffect ) )
		return;
	//CFuncBase<SFBTransform> *pPos = pPosition;
	/*if ( pSrc->pUnit->IsValid() )
	{
		const SViewUnit *pViewUnit = GetViewUnit( pSrc->pUnit );
		if ( !pViewUnit )
			return 0;
		pViewUnit->pLocators->pLocators.Refresh();
		int nIndex = pViewUnit->pLocators->pLocators->GetValue()->GetBoneIndex( pSrc->szLocator.c_str() );
		if ( nIndex < 0 )
			return 0;
		CPtr<NAnimation::CAddBoneFilter> pFilter = new NAnimation::CAddBoneFilter( nIndex );
		pFilter->pAnimation = pViewUnit->pLocators;
		pPos = pFilter;
	}*/
	NGScene::SRoomInfo r;
	r.bTreeCrown = true; // CRAP
	Register( pScene->CreateParticles( pEffect, tBegin, pTime, pPosition, r ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddPointLight( const CVec3 &ptColor, const CVec3 &ptOrigin, float fRadius, bool bLightmapOnly )
{
	Register( pScene->AddPointLight( ptColor, ptOrigin, fRadius, bLightmapOnly ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddSpotLight( const CVec3 &ptColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, float fRadius, NDb::CTexture *pMask, bool bLightmapOnly )
{
	Register( pScene->AddSpotLight( ptColor, ptOrigin, ptDir, fFOV, fRadius, pMask, bLightmapOnly ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddMesh( NDb::CModel *pModel, const SFBTransform &position, NGScene::CLightGroup *pGroup, int nFloor )
{
	NGScene::SRoomInfo room( pGroup, nFloor );
	Register( pScene->CreateMesh( pModel, position, room ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddPolyline( const vector<CVec3> &points, const CVec3 &cr )
{
	Register( pScene->CreatePolyline( points, cr ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddMesh( CMemObject *pModel, const CVec4 &color, const SFBTransform &position )
{
	Register( pScene->CreateMesh( pModel, color, position ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddItemMesh( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation )
{
	NAnimation::CSkeletonAnimator *pAnimator = new NAnimation::CSkeletonAnimator;
	pAnimator->pTime = pTime;
	pAnimator->bServer = false;
	pAnimator->bItem = true;
	pAnimator->AddAimer( 0, pAnimation, pAimTime );
	NAnimation::CAddBoneFilter *pFilter = new NAnimation::CAddBoneFilter;
	pFilter->pAnimation = pAnimator;
	Register( pScene->CreateMesh( pModel, pFilter ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddMesh( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation, 
	CFuncBase<NAnimation::SSkeletonState> *pState, const vector<SBoundMesh> &boundMeshes, NGScene::CLightGroup *pGroup, 
	int nFloor, NWorld::CUnit *pUnit )
{
	NGScene::SRoomInfo room( pGroup, nFloor );
	CPtr<NAnimation::CSkeletonAnimator> pAnimator = new NAnimation::CSkeletonAnimator( pModel->pSkeleton );
	pAnimator->pTime = pTime;
	pAnimator->AddSmartAimer( 0, pAnimation, pState, pAimTime, pAnimator, pModel->pSkeleton );
	pAnimator->bServer = false;
	Register( pScene->CreateSkin( pModel, pAnimator, room ) );

	for ( int k = 0; k < boundMeshes.size(); ++k )
	{
		const SBoundMesh &m = boundMeshes[k];
		int nIndex = pAnimator->GetBoneIndex( m.pszBindBone );
		if ( nIndex < 0 )
			continue;;
		NAnimation::CAddBoneFilter *pFilter = new NAnimation::CAddBoneFilter( nIndex );
		pFilter->pAnimation = pAnimator;
			
		/*NAnimation::CAddBoneLocators *pLocators = new NAnimation::CAddBoneLocators( nIndex, pModel->pGeometry );
		pLocators->pAnimation = pO->pAnimator;
		pO->pLocators = pLocators;*/
			
		Register( pScene->CreateMesh( m.pModel, pFilter, room ) );
	}

	if ( pUnit )
	{
		NAnimation::CAddBoneFilter *pFilter = new NAnimation::CAddBoneFilter(12); // CRAP - head bone
		pFilter->pAnimation = pAnimator;
		AddHead( pUnit, pFilter, room );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddTerrainParts( const SRandomSeed &sSeed, const CTRect<int> &sRegion, const list<CObj<CPtrFuncBase<CTerrainPart> > > &partsList, CFuncBase<STerrainInfo> *pInfo, CVersioningBase *pUpdateRegion )
{
	Register( pScene->CreateTerrainRegion( pInfo, pUpdateRegion, sSeed, sRegion, partsList, pGrass->CreateTracker( pInfo ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddTerrainWallPart( CPtrFuncBase<CTerrainPart> *pPart, NDb::CTexture *pTexture )
{
	Register( pScene->CreateTerrainWall( pPart, pTexture ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddGrass( CFuncBase<STerrainInfo> *pInfo )
{
	CPtr<NGScene::CGrassTracker> pGrassTracker = pGrass->CreateTracker( pInfo );

	SBound bound;
	SFBTransform transform;
	for ( int nLayer = 0; nLayer < pGrassTracker->GetNumLayers(); ++nLayer )
	{
		int nTexID = pGrassTracker->GetTextureLayerID( nLayer );
		if ( nTexID < 0 )
			continue;
		for ( int nY = 0; nY < pGrassTracker->GetNumSectorsY(); ++nY )
		{
			for ( int nX = 0; nX < pGrassTracker->GetNumSectorsX(); ++nX )
			{
				CPtrFuncBase<NGScene::CGrassPosition> *pGrassPos = pGrassTracker->GetGrassPosCalcer( nLayer, nX, nY );
				if ( pGrassPos )
				{
					NGScene::CGrassAnimator *pAnimator = new NGScene::CGrassAnimator(
						pGrassTracker->GetGrass( nLayer ), pGrassTracker, pGrassPos,
						pTime, nLayer );
					pGrassTracker->GetSectorBound( nLayer, nX, nY, &bound );
					pGrassTracker->GetBoundTransform( nX, nY, &transform );
					NGScene::CCFBTransform *pPlace = new NGScene::CCFBTransform( transform );
					Register( pScene->CreateGrassSector( pAnimator, NDb::GetTexture( nTexID ), pPlace, bound ) );
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddBuildingPart( int nPartID, const SMapBuilding &info, NBuilding::CBuildingInfoHold *pBI )
{
	Register( pScene->CreateBuildingPart( nPartID, info, pBI ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddGrassEvent( const CVec3 &ptPlace )
{
	pGrass->Wave( ptPlace );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddExplosion( NDb::CEffect *pEffect, CFuncBase<NGScene::CExplosionInfo> *pExplosion, const CVec3 &pos )
{
	Register( pScene->CreateExplosion( pTime, pEffect, pExplosion, pos ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddHead( NWorld::CUnit *pUnit, CFuncBase<SFBTransform> *pPosition, const NGScene::SRoomInfo &room )
{
	if ( !IsValid(pHeadsController) )
		return;

	NLSHead::CHeadAnimator *pAnimator = pHeadsController->GetAnimator(pUnit);
	if ( !pAnimator )
		return;

	bool bHasCap = pUnit->IsCapPresent();
	Register( pScene->CreateLSHead( pUnit->GetDBHead(), pAnimator, pHeadsController->GetTime(), pPosition, false, bHasCap, room ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddHead( NDb::CComplexHead *pHead, CFuncBase<SFBTransform> *pPosition, const NGScene::SRoomInfo &room )
{
	if ( !IsValid(pHeadsController) )
		return;

	NLSHead::CHeadAnimator *pAnimator = new NLSHead::CHeadAnimator( pTime, pHead->pHead );
	if ( !pAnimator )
		return;

	bool bHasCap = false; //pUnit->IsCapPresent();
	Register( pScene->CreateLSHead( pHead, pAnimator, pTime, pPosition, false, bHasCap, room ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddOccluder( NDb::CAIGeometry *pAIGeom, const SFBTransform &pos ) 
{
	Register( pScene->CreateOccluder( pAIGeom, pos ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSetRender::AddOccluder( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, CFuncBase<NAnimation::SSkeletonPose> *pAnimation )
{
	Register( pScene->CreateOccluder( pAIGeom, pSkeleton, pAnimation ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFakeRPGUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFakeRPGUnit: public NWorld::IVisObj
{
	OBJECT_NOCOPY_METHODS(CFakeRPGUnit);
	ZDATA
	CPtr<NGScene::IGameView> pView;
	CPtr<NDb::CModel> pModel;
	CPtr<NRPG::CUnit> pUnit;
	CObj<CFuncBase<STime> > pTime;
	CObj<NAnimation::CAnimation> pAnimation;
	CObj<NAnimation::CSkeletonAnimator> pAnimator;
	CSyncSrcBind<NWorld::IVisObj> bindGlobal;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pView); f.Add(3,&pModel); f.Add(4,&pUnit); f.Add(5,&pTime); f.Add(6,&pAnimation); f.Add(7,&pAnimator); f.Add(8,&bindGlobal); return 0; }
public:
	CFakeRPGUnit() {}
	CFakeRPGUnit( NGScene::IGameView *pView, CSyncSrc<NWorld::IVisObj> *pSrc, NRPG::CUnit *_pUnit, CFuncBase<STime>* _pTime );

	virtual void Visit( NWorld::IRenderVisitor *p );
	void Update( float fAngle );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CFakeRPGUnit::CFakeRPGUnit( NGScene::IGameView *_pView, CSyncSrc<NWorld::IVisObj> *pSrc, NRPG::CUnit *_pUnit, CFuncBase<STime>* _pTime ):
	pView( _pView ), pModel( _pUnit->pModel ), pUnit( _pUnit ), pTime( _pTime )
{
	pAnimator = new NAnimation::CSkeletonAnimator( pModel->pSkeleton );
	pAnimator->pTime = _pTime;
	pAnimator->bServer = false;
	pAnimation = pAnimator->CreateAnimation( pModel->pSkeleton->GetAnimation( NDb::CAnimation::POSE, NDb::CAnimation::POSE_STAND | NDb::CAnimation::WEAPON_NONE ), 0, true );
	pAnimator->AddAnimator( 0, pAnimation );

	bindGlobal.Link( pSrc, this );
	Update( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFakeRPGUnit::Update( float fAngle )
{
//	pAnimation->SetStand( 0, CVec3( 0, 0, 0 ), fAngle );
	bindGlobal.Update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFakeRPGUnit::Visit( NWorld::IRenderVisitor *p )
{
	vector<NWorld::IRenderVisitor::SBoundMesh> boundMeshes;
	p->AddMesh( pModel, pAnimator, 0, boundMeshes, 0, 0 );

	NAnimation::CAddBoneFilter *pFilter = new NAnimation::CAddBoneFilter(12); // CRAP - head bone
	pFilter->pAnimation = pAnimator;
	p->AddHead( pUnit->GetPers()->pHead, pFilter, NGScene::SRoomInfo() );
/*
	NAnimation::CAddBoneFilter *pFilter = new NAnimation::CAddBoneFilter(12); // CRAP - head bone
	pFilter->pAnimation = pAnimator;

	NLSHead::CHeadAnimator *pHeadAnimator = new NLSHead::CHeadAnimator( pTime, pUnit->GetPers()->pHead->pHead );
	pView->CreateLSHead( pUnit->GetPers()->pHead, pHeadAnimator, pTime, pFilter, false, false );
*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFakeWorldUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFakeWorldUnit: public NWorld::IVisObj
{
	OBJECT_NOCOPY_METHODS(CFakeWorldUnit);
	ZDATA
	CPtr<NDb::CModel> pModel;
	CObj<NAnimation::CSkeletonAnimator> pAnimator;
	CSyncSrcBind<NWorld::IVisObj> bindGlobal;
	CPtr<NRPG::IUnitMissionInfo> pRPG;
	CPtr<NWorld::CUnit> pUnit;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pModel); f.Add(3,&pAnimator); f.Add(4,&bindGlobal); f.Add(5,&pRPG); f.Add(6,&pUnit); return 0; }
public:
	CFakeWorldUnit() {}
	CFakeWorldUnit( CSyncSrc<NWorld::IVisObj> *pSrc, NWorld::CUnit *_pUnit, CFuncBase<STime>* _pTime );

	virtual void Visit( NWorld::IRenderVisitor *p );
	void Update( float fAngle );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CFakeWorldUnit::CFakeWorldUnit( CSyncSrc<NWorld::IVisObj> *pSrc, 
	NWorld::CUnit *_pUnit, CFuncBase<STime>* _pTime )
: pModel( _pUnit->GetModel() ), pRPG( _pUnit->GetRPG() ), pUnit( _pUnit )
{
	pAnimator = new NAnimation::CSkeletonAnimator( pModel->pSkeleton );
	pAnimator->pTime = _pTime;
	pAnimator->bServer = false;
	bindGlobal.Link( pSrc, this );
	Update( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFakeWorldUnit::Update( float fAngle )
{
	// CRAP - animation should be set once
	int nAnimFlags = NDb::CAnimation::POSE_STAND;
	nAnimFlags |= NDb::WeaponTypeToAnimFlags( pRPG->GetWeaponType(), pRPG->GetInventoryInfo()->GetActive() );
	NAnimation::CAnimation *pAnimation;
	pAnimation = pAnimator->CreateAnimation(
		pModel->pSkeleton->GetAnimation( NDb::CAnimation::POSE, nAnimFlags ), 0, true );
	pAnimation->SetStand( 0, CVec3( 0, 0, 0 ), fAngle );
	pAnimator->AddAnimator( 0, pAnimation );
	bindGlobal.Update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFakeWorldUnit::Visit( NWorld::IRenderVisitor *p )
{
	vector<NWorld::IRenderVisitor::SBoundMesh> boundMeshes;
	NWorld::GetItemsBindPlaces( &boundMeshes, pRPG, false );
	p->AddMesh( pModel, pAnimator, 0, boundMeshes, 0, 0, pUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SCreateSyncSrc
{
	ZDATA
	CObj<CSyncSrc<NWorld::IVisObj> > pShow;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pShow); return 0; }
	SCreateSyncSrc(): pShow( new NWorld::CWorldSyncSrc ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CShowRPGUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CShowRPGUnit: public IShowUnit, public SCreateSyncSrc
{
	OBJECT_NOCOPY_METHODS(CShowRPGUnit);
	ZDATA_(SCreateSyncSrc)
	CSetRender r;
	CObj<CFakeRPGUnit> pUnit;	
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(SCreateSyncSrc*)this); f.Add(2,&r); f.Add(3,&pUnit); return 0; }
public:
	CShowRPGUnit() {}
	CShowRPGUnit( NGScene::IGameView *pView, NRPG::CUnit *_pUnit, CFuncBase<STime>* _pTime, NLSHead::CHeadsController *pHdController );
	void Update( float fAngle );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CShowRPGUnit::CShowRPGUnit( NGScene::IGameView *pView, NRPG::CUnit *_pUnit, CFuncBase<STime>* _pTime, NLSHead::CHeadsController *pHdController ):
	r( pShow, pView )
{
	r.SetTimer( _pTime, _pTime );
	r.SetHeadsController( pHdController );
	pUnit = new CFakeRPGUnit( pView, pShow, _pUnit, _pTime );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CShowRPGUnit::Update( float fAngle )
{
	pUnit->Update( fAngle );
	r.Sync();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IShowUnit* CreateShowUnit( NGScene::IGameView *pView, NRPG::CUnit *pUnit, CFuncBase<STime>* pTime, IRenderGame *pRenderGame )
{
	CPtr<NLSHead::CHeadsController> pController;
	if ( IsValid( pRenderGame ) )
		pController = pRenderGame->GetHeadController();
	else
		pController = new NLSHead::CHeadsController;

	return new CShowRPGUnit( pView, pUnit, pTime, pController );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CShowWorldUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CShowWorldUnit: public IShowUnit, public SCreateSyncSrc
{
	OBJECT_NOCOPY_METHODS(CShowWorldUnit);
	ZDATA_(SCreateSyncSrc)
	CSetRender r;
	CObj<CFakeWorldUnit> pUnit;	
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(SCreateSyncSrc*)this); f.Add(2,&r); f.Add(3,&pUnit); return 0; }
public:
	CShowWorldUnit() {}
	CShowWorldUnit( NGScene::IGameView *pView, NWorld::CUnit *_pUnit, CFuncBase<STime>* _pTime, NLSHead::CHeadsController *pHdController );
	void Update( float fAngle );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CShowWorldUnit::CShowWorldUnit( NGScene::IGameView *pView, NWorld::CUnit *_pUnit, CFuncBase<STime>* _pTime, NLSHead::CHeadsController *pHdController )
: r( pShow, pView )
{
	r.SetTimer( _pTime, _pTime );
	r.SetHeadsController( pHdController );
	pUnit = new CFakeWorldUnit( pShow, _pUnit, _pTime );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CShowWorldUnit::Update( float fAngle )
{
	pUnit->Update( fAngle );
	r.Sync();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IShowUnit* CreateShowUnit( NGScene::IGameView *pView, NWorld::CUnit *pUnit, CFuncBase<STime>* pTime, IRenderGame *pRenderGame )
{
	CPtr<NLSHead::CHeadsController> pController;
	if ( IsValid( pRenderGame ) )
		pController = pRenderGame->GetHeadController();
	else
		pController = new NLSHead::CHeadsController;

	return new CShowWorldUnit( pView, pUnit, pTime, pController );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CShowUnitHead
////////////////////////////////////////////////////////////////////////////////////////////////////
class CShowUnitHead: public IShowUnitHead
{
	OBJECT_NOCOPY_METHODS(CShowUnitHead);
	ZDATA
	CPtr<NWorld::CUnit> pUnit;
	CObj<NGScene::CRenderNode> pRenderNode;
	CPtr<NLSHead::CHeadsController> pController;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pUnit); f.Add(3,&pRenderNode); f.Add(4,&pController); return 0; }
public:
	CShowUnitHead() {}
	CShowUnitHead( NGScene::IGameView *pView, NWorld::CUnit *pUnit, NLSHead::CHeadsController *pController, CFuncBase<SFBTransform> *pTransform );

	void SetSequence( NDb::CSequence *pSequence );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CShowUnitHead::CShowUnitHead( NGScene::IGameView *pView, NWorld::CUnit *_pUnit, NLSHead::CHeadsController *_pController, CFuncBase<SFBTransform> *pTransform ):
	pUnit( _pUnit ), pController( _pController )
{
	NLSHead::CHeadAnimator *pAnimator = pController->GetAnimator( pUnit );
	pRenderNode = pView->CreateLSHead( pUnit->GetDBHead(), pAnimator, pController->GetTime(), pTransform, true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CShowUnitHead::SetSequence( NDb::CSequence *pSequence )
{
	pController->PlaySequence( pUnit, pSequence );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IShowUnitHead* CreateShowUnitHead( NGScene::IGameView *pView, NWorld::CUnit *pUnit, NLSHead::CHeadsController *pController, CFuncBase<SFBTransform> *pTransform )
{
	return new CShowUnitHead( pView, pUnit, pController, pTransform );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CRenderGame
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SVisibleHolder
{
	ZDATA
	CObj<CSetSyncSrc<NWorld::IVisObj> > pVisible;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pVisible); return 0; }
	
	SVisibleHolder(): pVisible( new CSetSyncSrc<NWorld::IVisObj> ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRenderGame: public IRenderGame, public SVisibleHolder
{
	OBJECT_BASIC_METHODS(CRenderGame);
	ZDATA_(SVisibleHolder)
		// test sphere visualization
	list< CObj<NGScene::CRenderNode> > testSpheres;
	
	CPtr<NWorld::IWorld> pWorld;
	CPtr<NGScene::IGameView> pScene;
	CTimeCounter timer;
	CSetRender r, rUnits;
	bool bPrevShowUnits;
	CPtr<NWorld::IPlayer> pPrevViewFrom;
	CObj<NGScene::CGrass> pGrass;
	CObj<NLSHead::CHeadsController> pHeadsController;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(SVisibleHolder*)this); f.Add(2,&testSpheres); f.Add(3,&pWorld); f.Add(4,&pScene); f.Add(5,&timer); f.Add(6,&r); f.Add(7,&rUnits); f.Add(8,&bPrevShowUnits); f.Add(9,&pPrevViewFrom); f.Add(10,&pGrass); f.Add(11,&pHeadsController); return 0; }
	//
	void UpdateVisible( NWorld::IPlayer *pViewFrom, bool bShowUnits );
public:
	CRenderGame() {}
	CRenderGame( NWorld::IWorld *_pWorld, NGScene::IGameView *_pScene );
	
	CObjectBase* Select( NWorld::IVisObj *pSelect, const CVec4 &vColor );
	
	CCTime* GetTime() { return timer.GetTime(); }
	NLSHead::CHeadsController* GetHeadController() const { return pHeadsController; }
	
	void UpdateViewWorld( bool bAdvanceTime, STime currentTime, NWorld::IPlayer *pViewFrom, bool bShowUnits );
	void FastUpdate( STime currentTime );
	void ResetTiming();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderGame::CRenderGame( NWorld::IWorld *_pWorld, NGScene::IGameView *_pScene )
: 
r( _pWorld->GetActive(), _pScene ), 
rUnits( _pWorld->GetUnits(), _pScene ), 
pWorld(_pWorld), pScene(_pScene), bPrevShowUnits( true )
{
	r.SetTimer( timer.GetTime(), pWorld->GetAimTime() );
	rUnits.SetTimer( timer.GetTime(), pWorld->GetAimTime() );

	pGrass = new NGScene::CGrass( pWorld->GetAIMap() );
	r.SetGrass( pGrass );
	rUnits.SetGrass( pGrass );

	pHeadsController = new NLSHead::CHeadsController;
	r.SetHeadsController( pHeadsController );
	rUnits.SetHeadsController( pHeadsController );
/*
	if ( pScene != 0 )
	{
		pTerrain = pScene->CreateTerrain( pWorld->GetTerrain()->pInfo, timer.GetTime() );
		r.SetTerrain( pTerrain );
		rUnits.SetTerrain( pTerrain );
		/ *for ( int x = 0; x < 50; ++x )
		{
			for ( int y = 0; y < 50; ++y )
				pScene->AddPointLight( CVec3(0.5f,0.5f,0.5f), CVec3(2 + 11 * x,2 + 11 * y, 6), 8 );
		}* /
		//pScene->AddSpotLight( CVec3(1,1,1), CVec3( 2, 0, 6 ), CVec3( 0, 2, -1 ), 70, 10, NDb::GetTexture(13), -1 );//92) );
	}
*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CRenderGame::Select( NWorld::IVisObj *pSelect, const CVec4 &vColor )
{
	CObjectBase* pSelection;

	pSelection =  r.Select( pSelect, vColor );
	if ( pSelection )
		return pSelection;

	pSelection = rUnits.Select( pSelect, vColor );
	if ( pSelection )
		return pSelection;

	return 0;
}
/*
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderGame::Unselect( NWorld::IVisObj *pSelect )
{
	r.Unselect( pSelect );
	rUnits.Unselect( pSelect );
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderGame::ResetTiming()
{
	timer.ResetTiming();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderGame::UpdateVisible( NWorld::IPlayer *pViewFrom, bool bShowUnits )
{
	if ( !bShowUnits )
	{
		if ( bPrevShowUnits != bShowUnits )
			rUnits.SetNewSource( new CSetSyncSrc<NWorld::IVisObj>() );

		bPrevShowUnits = bShowUnits;
		return;
	}

	bPrevShowUnits = bShowUnits;

	NWorld::IPlayer::CUnitSet units;
	if ( pViewFrom )
	{
		pViewFrom->GetUnits(&units);
		if ( units.empty() )
			pViewFrom = 0;
	}
	if ( pPrevViewFrom != pViewFrom )
	{
		if ( pViewFrom )
			rUnits.SetNewSource( 
				new CBoolSyncSrc<NWorld::IVisObj, CIntersectionFunc>( pWorld->GetUnits(), pVisible )  
				);
		else
			rUnits.SetNewSource( pWorld->GetUnits() );
		pPrevViewFrom = pViewFrom;
	}
	if ( pViewFrom )
	{
		list<CPtr<NWorld::CUnit> > res;
		pViewFrom->GetVisible( &res );
		vector<NWorld::IVisObj*> vis;
		for ( list<CPtr<NWorld::CUnit> >::iterator i = res.begin(); i != res.end(); ++i )
		{
			vis.push_back( *i );
			(*i)->AddVisitableChildren( &vis );
		}
		pViewFrom->GetSounds( &vis );
		pVisible->Set( vis );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderGame::FastUpdate( STime currentTime )
{
	UpdateVisible( 0, false );
	timer.Advance( true, currentTime );
	// render them all
	r.Sync();
	rUnits.Sync();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderGame::UpdateViewWorld( bool bAdvanceTime, STime currentTime, NWorld::IPlayer *pViewFrom, bool bShowUnits )
{
	UpdateVisible( pViewFrom, bShowUnits );

	timer.Advance( bAdvanceTime, currentTime );

	pHeadsController->Advance( currentTime );

	STime t = timer.GetTime()->GetValue();
	pWorld->UpdateWorld( t, pViewFrom );
	pGrass->Update( t );

	// test sphere visualization
	testSpheres.clear();
	for ( int i=0; i<sphereParticles.size(); ++i )
	{
		CPtr<CMemObject> pModel = new CMemObject;
		pModel->CreateSphere( sphereParticles[i].ptCenter, sphereParticles[i].fRadius, 1 );
		CVec4 color( 1, 0.3f, 0.3f, 1.0f );
		testSpheres.push_back( pScene->CreateMesh( pModel, color, 0 ) );
	}
	//sphereParticles.clear();
	// render them all
	r.Sync();
	rUnits.Sync();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IRenderGame* CreateRenderGame( NWorld::IWorld *_pWorld, NGScene::IGameView *_pScene )
{
	return new CRenderGame( _pWorld, _pScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NRender;
BASIC_REGISTER_CLASS( IShowUnit );
BASIC_REGISTER_CLASS( IRenderGame );
REGISTER_SAVELOAD_CLASS( 0x01941130, CRenderGame );
REGISTER_SAVELOAD_CLASS( 0x01941131, CSelection );
REGISTER_SAVELOAD_CLASS( 0x01941132, CShowWorldUnit );
REGISTER_SAVELOAD_CLASS( 0x01941133, CFakeWorldUnit );
REGISTER_SAVELOAD_CLASS( 0x01941134, CShowUnitHead );
REGISTER_SAVELOAD_CLASS( 0x01941135, CShowRPGUnit );
REGISTER_SAVELOAD_CLASS( 0x01941136, CFakeRPGUnit );
