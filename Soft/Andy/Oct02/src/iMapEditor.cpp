#include "StdAfx.h"
#include "iMain.h"
#include "GView.h"
#include "G2DView.h"
#include "GSceneUtils.h"
#include "..\Input\Input.h"
#include "..\Input\Bind.h"
#include "Camera.h"
#include "Transform.h"
#include "iMapEditor.h"
#include "MapBuild.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataAnimation.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataObject.h"
#include "..\DBFormat\DataSound.h"
#include "..\Misc\StrProc.h"
#include "GAnimation.h"
#include "BuildingGrid.h"
#include "TerrainInfo.h"
#include "GBuilding.h"
#include "Grid.h"
#include "Interface.h"
#include "MemObject.h"
#include "Sound.h"
#include "aiMap.h"
#include "wInterface.h"
#include "RWGame.h"
#include "MEUserSettings.h"
#include "MEParams.h"
#include "..\MapEdit\UserSettingsSetup.h"
#include "GSceneUtils.h"
#include "LSHead.h"
#include "LSController.h"
#include "RPGUnitMission.h"
#include "RPGGame.h"
#include "InventoryUnit.h"
#include "RPGItemInfo.h"
#include "RPGItem.h"

extern float gfMayaCameraSpeed;
extern ERPGItemCamera geRPGInventoryCamera;
extern vector<pair<string, int> > gvAnimItems; // <effector name, model id>
extern int gnHeadID;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMapViewer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMapViewer: public CIMapEditor
{
	OBJECT_BASIC_METHODS(CMapViewer);
	//
	CICMapEditor::SMapParams mapParams;
	CVec3 crClear;
	SMapInfo info;
	//
	CPtr<ICamera> pCamera;
	CPtr<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	CPtr<NGScene::IGameView> pScene;
	CPtr<NGScene::I2DGameView> p2DScene;
	CPtr<NSound::ISoundScene> pSoundScene;
	CObj<NWorld::IWorld> pWorld;
	CObj<NRender::IRenderGame> pRender;
	CObj< CCTerrainInfo > pTerrainInfo;
	CObj< NRPG::IUnitMission> pUnitMission;
	list< CObj<NGScene::CRenderNode> > objects;
	list< CObj<NGScene::ILight> > lights;
	list< CObj<CObjectBase> > particles;
	list< CObj<NGScene::CBuilding> > buildings;
	list< CObj<NBuilding::CBuildingGrid> > buildingGrids;
  list< CObj<NAnimation::CSkeletonAnimator> > animators;
	NInput::CBind cExec, cExit, cSave, cLoad, cBuildParts, cCutFloorUp, cCutFloorDown, cShadows, cRoll, cFog, cUpdate;
	NInput::CBind cCameraInfo, cHSR, cNextPhase, cCameraReset, cChapterCamera;

  CObj<NDb::CModel> pTempModel;
  CObj<NDb::CModel> pTempModel2;
  CObj<NGScene::CRects> pRects;
	CObj<NLSHead::CHeadsController> pHeadsController;
	CObj<NLSHead::CHeadAnimator> pHAnimator;

	CTimeCounter timer;
	HWND hWnd;
	CObj<NGScene::CCTRect> pFrameRect;
	RECT rLastWindowRect;
	CRectLayout sLayout;
	CPtr<NDb::CTexture> pFrameTex;
	int nPhase; // фаза разрушений
	float fFov;
	CObj<NGScene::CLightGroup> pLG;
	
  void InsertObjects( const SMapInfo &info );
	//void MakeBuildings( vector<SMapBuilding> &info );
	void ComputeRPGItemRect();
	void ComputeRPGPersRect();
	void SetRect( float fw, float fh );
	void SetContainer( int nID );
	void SetObjectPhase( int nID, int nVarID, int nPhase );
	void AddItems( NAnimation::CSkeletonAnimator *pAnimator, const vector<NWorld::IRenderVisitor::SBoundMesh> &items, NDb::CComplexHead *pHead );
	void AddAIGeometry( int nAIGeomID, const CVec3 &ptShift = VNULL3 );
	
public:
	CMapViewer();
	//
  void  BuildMap( CICMapEditor::EViewType nView, int nObjectID, int nExtra );
	void  SetCommand( const CICMapEditor::SMapParams &cmd ) { mapParams = cmd; mapParams.bCameraReset = false; }
  void  SetupLight( SLightPrefs prefs, NGScene::ESceneRenderMode mode, NGScene::EFogMode fogMode );
  void  SetCameraPos( const CVec3 &ptCameraPos );
  void  SetCameraPos( const ICamera::SCameraPos &pos );
	void  SetCutFloor( int nFloor ) { ASSERT( pScene ); pScene->SetCutFloor( nFloor ); }
	int   GetCutFloor() const { ASSERT( pScene ); return pScene->GetCutFloor(); }
  void  GetCameraPos( ICamera::SCameraPos *pPos ) const;
	HWND  GetHWND() const { return hWnd; }
	void  SetHWND( HWND h ) { hWnd = h; }
	NGScene::ESceneRenderMode GetRenderMode() const { ASSERT( pScene ); return pScene->GetRenderMode(); }
	NGScene::EFogMode GetFogMode() const { ASSERT( pScene ); return pScene->GetFogMode(); }
	virtual bool ProcessEvent( const NInput::SEvent &eEvent );
	virtual void Step();
  virtual void OnGetFocus();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMapViewer::ProcessEvent( const NInput::SEvent &eEvent )
{
	NInput::SetSection( "mapeditor" );
	bool bRet = pInterface->ProcessEvent( eEvent );
	if ( bRet )
		return true;
		
	pCamera->ProcessEvent( eEvent );
	
	if ( cExit.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( 0 );
		return true;
	}
	else if ( cExec.ProcessEvent( eEvent ) )
	{
		/*
		for ( list< CObj<NBuilding::CBuildingGrid> >::iterator it = buildingGrids.begin(); it != buildingGrids.end(); ++it )
			(*it)->Explode( CVec3( 0.0f, 0.0f, 1.5f ), 1000 );
		buildings.clear();
		for ( int i = 0; i < info.buildings.size(); ++i )
		{
			buildings.push_back( pScene->CreateBuilding( info.buildings[i] ) );
		}
		*/
		return true;
	}
	else if ( cSave.ProcessEvent( eEvent ) )
		return true;
	else if ( cLoad.ProcessEvent( eEvent ) )
		return true;
	else if ( cCutFloorUp.ProcessEvent( eEvent ) )
	{
		pScene->SetCutFloor( pScene->GetCutFloor() + 1 );
		return true;
	}
	else if ( cCutFloorDown.ProcessEvent( eEvent ) )
	{
		pScene->SetCutFloor( pScene->GetCutFloor() - 1 );
		return true;
	}
	else if ( cShadows.ProcessEvent( eEvent ) )
	{
		pScene->SetNextShadowsMode();
		return true;
	}	
	else if ( cFog.ProcessEvent( eEvent ) )
	{
		pScene->SetNextFogMode();
	}
	else if ( cBuildParts.ProcessEvent( eEvent ) )
	{
		for ( list< CObj<NBuilding::CBuildingGrid> >::iterator it = buildingGrids.begin(); it != buildingGrids.end(); ++it )
			(*it)->Explode( CVec3( 2.5, 4.0, 1.5 ), 0, 1 );
		for ( list< CObj<NGScene::CBuilding> >::iterator it = buildings.begin(); it != buildings.end(); ++it )
			(*it)->ToggleParts();
		
		return true;
	}
	else if ( cRoll.ProcessEvent( eEvent ) || cUpdate.ProcessEvent( eEvent ) )
	{
		NMainLoop::Command( new CICMapEditor( mapParams, GetHWND() ) );
		return true;
	}
	else if ( cChapterCamera.ProcessEvent( eEvent ) )
	{
		ICamera::SCameraPos sPos;

		if ( mapParams.nView != CICMapEditor::VIEW_PLACEMENT )
			return true;
		NDb::CTemplVariant *pVar = NDb::GetTemplVariant( mapParams.nObjectID );
		if ( !IsValid( pVar ) || !IsValid( pVar->pTemplate ) )
			return true;
		CTPoint<int> sSize( pVar->pTemplate->nWidth, pVar->pTemplate->nHeight );
		sPos.fYaw = 0;
		sPos.fPitch = ToRadian( -90.0f );
		sPos.fRod = 0.5f * sSize.x * FP_GRID_STEP / tan( ToRadian( 0.5f * pCamera->GetFOV() ) );
		sPos.ptAnchor = CVec3( 0.5f * sSize.x * FP_GRID_STEP, 0.5f * sSize.y * FP_GRID_STEP, 0 );
		pCamera->SetPlacement( sPos );
		return true;
	}
	else if ( cHSR.ProcessEvent( eEvent ) )
		pScene->SetNextHSRMode();
	else if ( cCameraInfo.ProcessEvent( eEvent ) )
	{
		ICamera::SCameraPos pos;
		pCamera->GetPlacement( &pos );
		GetUserSettingsSetup().SetCameraInfo( pos );
	}
	else if ( cNextPhase.ProcessEvent( eEvent ) )
	{
		if ( mapParams.nView == CICMapEditor::VIEW_OBJECT )
		{
			nPhase = (nPhase + 1) % NDb::N_DESTROY_STAGES;
			if ( 0 == nPhase )
				timer.ResetTiming();
			SetObjectPhase( mapParams.nObjectID, mapParams.nExtra, nPhase );
		}
	}
	else if ( cCameraReset.ProcessEvent( eEvent ) )
	{
		pCamera->SetPlacement( mapParams.camera );
		return true;
	}


	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::OnGetFocus()
{
	timer.ResetTiming();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::Step()
{
	STime currentTime = GetTime();
	timer.Advance( CanRender(), currentTime );
	if ( CanRender() )
	{
		//
		for ( list< CObj<NGScene::CBuilding> >::iterator it = buildings.begin(); it != buildings.end(); ++it )
			(*it)->CheckHP();
    pCamera->Update( GetTime() );
		pInterface->Step( GetTime() );
		CTransformStack ts;
		ts.MakeProjective( pScene->GetScreenRect(), fFov, 0.1f, 300 );
		ts.SetCamera( pCamera->GetPos() );
		if ( IsValid( pRender ) )
			pRender->FastUpdate( GetTime() );
		NGScene::IGameView::SDrawInfo drawInfo;
		drawInfo.pTS = &ts;
		drawInfo.vClearColor = crClear;
		pScene->Draw( drawInfo );
		//pScene->DrawFullScreen( drawInfo );//&ts, crClear );
		pInterface->Draw( GetTime() );
		p2DScene->StartNewFrame();
		switch ( mapParams.nView )
		{
			case CICMapEditor::VIEW_RPGITEM:
			case CICMapEditor::VIEW_PERS:
			{
				CRect r;
				GetWindowRect( hWnd, &r );
				CTRect<int> drect( 0, 0, r.Width(), r.Height() );
				if ( r != rLastWindowRect )
				{
					if ( mapParams.nView == CICMapEditor::VIEW_RPGITEM )
						ComputeRPGItemRect();
					else
						ComputeRPGPersRect();
					rLastWindowRect = r;
				}
				p2DScene->CreateDynamicRects( NDb::GetTexture( 1307 ), sLayout, CTPoint<int>( 0, 0), drect );
				break;
			}
			case CICMapEditor::VIEW_MODEL:
			{
				SLightPrefs pref = mapParams.lightPrefs;
				//CVec3 ptDir = pCamera->GetForwardDir();
				ICamera::SCameraPos pos;
				pCamera->GetPlacement( &pos );
				SFBTransform m;
				MakeMatrix( &m, pos.fPitch, pos.fYaw, VNULL3 );
				//SHMatrix pos = pCamera->GetPos();
				m.forward.RotateHVector( &pref.ptLight, pref.ptLight );
				NGScene::ESceneRenderMode mode = pScene->GetRenderMode();
				NGScene::EFogMode fogMode = pScene->GetFogMode();
				SetupLight( pref, mode, fogMode );
				break;
			}
		}
		p2DScene->Flush();
		NGScene::Flip();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void RotatePt( CVec3 *pVec, int nAngle )
{
  const float fAng = -ToRadian( (float) nAngle );
  float fc = cos( fAng );
  float fs = sin( fAng );
  float x = fc * pVec->x + fs * pVec->y;
  pVec->y = -fs * pVec->x + fc * pVec->y;
  pVec->x = x;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::InsertObjects( const SMapInfo &info )
{
  for ( list<SMapElement>::const_iterator i = info.items.begin(); i != info.items.end(); ++i )
  {
		if ( IsValid( i->pObject ) && IsValid( i->pObject->pModels[0] ) )
		{
			NDb::CContainerModel *pCM = i->pObject->pModels[0];
			NDb::CModel *pM = pCM->pModel;
			if ( IsValid( pM ) )
			{
				NGScene::CRenderNode *pR = pScene->CreateMesh( pM, MakeTransform( i->pos.ptPos, i->pos.fRotation ) * MakeTransform( VNULL3, i->pos.ptScale ) );
				objects.push_back( pR );
			}
			NDb::CEffect *pEff = pCM->pEffect;
			if ( pEff )
			{
				NGScene::CCFBTransform *pMSR = new NGScene::CCFBTransform;
				CVec3 ptPos = i->pos.ptPos;
				SFBTransform tr = MakeTransform( ptPos, i->pos.fRotation );
				tr.forward.RotateHVector( &ptPos, pCM->ptEffectPos );
				pMSR->Set( MakeTransform( ptPos, i->pos.fRotation ) );
				CObjectBase *pP = pScene->CreateParticles( pEff, 0, timer.GetTime(), pMSR );
				particles.push_back( pP );
			}
			if ( fabs2( pCM->ptPLightCr ) > FP_EPSILON )
			{
				CVec3 ptPos = i->pos.ptPos;
				SFBTransform tr = MakeTransform( ptPos, i->pos.fRotation );
				tr.forward.RotateHVector( &ptPos, pCM->ptPLightPos );
				NGScene::ILight* pL = pScene->AddPointLight( pCM->ptPLightCr, ptPos, pCM->fPLightRadius );
				lights.push_back( pL );
			}
			if ( fabs2( pCM->ptSLightCr ) > FP_EPSILON )
			{
				CVec3 ptPos = i->pos.ptPos;
				SFBTransform tr = MakeTransform( ptPos, i->pos.fRotation );
				tr.forward.RotateHVector( &ptPos, pCM->ptSLightPos );
				CVec3 ptDir = pCM->ptSLightDir;
				RotatePt( &ptDir, i->pos.fRotation );
				NGScene::ILight *pL = pScene->AddSpotLight( pCM->ptSLightCr, ptPos, ptDir, pCM->fSLightFOV, pCM->fSLightRadius, pCM->pSLightMask );
				lights.push_back( pL );
			}
		}
  }
  for ( list<SMapUnit>::const_iterator i = info.units.begin(); i != info.units.end(); ++i )
  {
		SRand rand;
		CPtr<NDb::CModel> pModel = i->pPers->pModel->CreateModel( &rand );
		if ( !pModel )
			continue;
		NGScene::CRenderNode *pR;
		NDb::CAnimation *pA = 0;
		if ( IsValid( pModel->pSkeleton ) )
		{
			pA = pModel->pSkeleton->GetAnimation( NDb::CAnimation::ATTACK );
			if ( !pA )
				pA = pModel->pSkeleton->GetAnimation( NDb::CAnimation::POSE );
		}
		if ( pA )
    {
			//NGScene::CCFBTransform *pMSR = new NGScene::CCFBTransform;
			//pMSR->Set( MakeTransform( i->pos.ptPos, i->pos.nRotation ) );
			NAnimation::CSkeletonAnimator *pAnimator = 0;
      pAnimator = new NAnimation::CSkeletonAnimator( pModel->pSkeleton );
      pAnimator->pTime = timer.GetTime();
			NAnimation::CAnimation *pAnim = pAnimator->CreateAnimation( pA, 0, true );
			pAnim->SetStand( 0, i->pos.ptPos, ToRadian( i->pos.fRotation ) );
      pAnimator->AddAnimator( 0, pAnim );
      animators.push_back( pAnimator );
			pR = pScene->CreateSkin( pModel, pAnimator );
    }
		else
			pR = pScene->CreateMesh( pModel, MakeTransform( i->pos.ptPos, i->pos.fRotation ) );
		//
    objects.push_back( pR );
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void CMapViewer::MakeBuildings( vector<SMapBuilding> &info )
{
	for ( int i = 0; i < info.size(); ++i )
	{
		NGScene::CBuilding *pR = pScene->CreateBuilding( info[i] );
		buildings.push_back( pR );
		buildingGrids.push_back( (NBuilding::CBuildingGrid*)info[i].pGrid );
	}
}
*/
void CMapViewer::SetContainer( int nID )
{
	NDb::CContainerModel *pContainer = NDb::GetContainer( nID );
  if ( !IsValid( pContainer ) )
    return;
  if ( IsValid( pContainer->pModel ) )
	{
		CPtr<NDb::CModel> pObj = pContainer->pModel;
		NGScene::CRenderNode *pR = pScene->CreateMesh( pObj, MakeTransform( CVec3(0,0,0), 0 ) );
		objects.push_back( pR );
	}
	timer = CTimeCounter();
  if ( IsValid( pContainer->pEffect ) )
	{
		NGScene::CCFBTransform *pMSR = new NGScene::CCFBTransform;
		pMSR->Set( MakeTransform( FP_GRID_STEP * pContainer->ptEffectPos, 0 ) );
		CObjectBase *pP = pScene->CreateParticles( pContainer->pEffect, 0, timer.GetTime(), pMSR );
		particles.push_back( pP );
	}
	if ( IsValid( pContainer->pDestroySound ) )
	{
		static SRand rnd;

		NDb::CSoundVariant *pSVar = pContainer->pDestroySound->GetSound( &rnd );
		if ( IsValid( pSVar ) )
			pSoundScene->Add2DSound( pSVar->pSound );
	}
	if ( fabs( pContainer->ptPLightCr ) > FP_EPSILON )
	{
		NGScene::ILight* pL = pScene->AddPointLight( pContainer->ptPLightCr, pContainer->ptPLightPos, pContainer->fPLightRadius );
		lights.push_back( pL );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::SetObjectPhase( int nID, int nVarID, int nPhase )
{
	if ( nPhase < 0 || nPhase >= NDb::N_DESTROY_STAGES )
		return;
	objects.clear();
	particles.clear();
	lights.clear();

	CDBTable<NDb::CTRndObject> *pTbl = NDatabase::GetTable<NDb::CTRndObject>();
	if ( !pTbl )
		return;
	NDb::CTRndObject *pO = pTbl->GetRecord( nID );
	if ( pO == 0 )
		return;
	NDb::CRndObject *pRO = pO->GetVariant( nVarID );
	if ( !pRO || !IsValid( pRO->pModels[nPhase] ) )
		return;
	SetContainer( pRO->pModels[nPhase]->GetRecordID() );

	if ( GetUserSettings().GetParam( ME_OBJECTS_SHOWGROUND ) )
	{
		NDb::CModel *pM = NDb::GetModel( 1411 ); // CUBE
		NGScene::CRenderNode *pR = pScene->CreateMesh( pM, MakeTransform( CVec3(-0.5f,-0.5f,-1), CVec3( 10, 10, 1 ) ) );
		objects.push_back( pR );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::BuildMap( CICMapEditor::EViewType nView, int nObjectID, int nExtra )
{
  if ( -1 == nObjectID )
    return;
	static SRand rand;


	pWorld = 0;
	pRender = 0;
	
	fFov = 35.0f;
	crClear = mapParams.lightPrefs.vFogColor;//CVec3(0.65f, 0.65f, 0.7f );
  switch ( nView )
  {
  case  CICMapEditor::VIEW_TEXTURE:
    {
      NDb::CTexture *pTexture = NDb::GetTexture( nObjectID );
      if ( !IsValid( pTexture ) )
        return;
      pRects = p2DScene->CreateFullRect( pTexture );
//      pScene->SetAmbient( CVec3( 0.3f, 0.3f, 0.3f ) );
//      pScene->SetFogColor( CVec3(0.8f, 0.8f, 0.8f) );
      return;
    }
  case CICMapEditor::VIEW_PLACEMENT:
    {
			/*
			CObj<NAI::IPathNetwork> pPathNetwork( NAI::CreateNodesNetwork( 0 ) );
			info = SMapInfo();
			vector<string> params;

			string *pszStr = reinterpret_cast<string*>( nExtra );
			if ( pszStr )
				NStr::SplitString( *pszStr, params, ',' );
      if ( ::BuildMap( nObjectID, params, pPathNetwork, &info ) ) // CRAP explicit path
			{
				pTerrainInfo = new CCTerrainInfo( info.terrain );			
				pScene->CreateTerrain( pTerrainInfo, timer.GetTime() );
			}
			*/
			pWorld = NWorld::CreateWorld( 0 );
			pWorld->CreateRandom( nObjectID, false, list<CPtr<NScenario::CScenarioClue> >(), 0, 0, SRandomSeed( GetTickCount() ) );
			pRender = NRender::CreateRenderGame( pWorld, pScene );
			//InsertObjects( info );
			//MakeBuildings( info.buildings );
			crClear = mapParams.lightPrefs.vFogColor;
			//      pScene->SetAmbient( light );      
			//      pScene->SetFogColor( CVec3(0.65f, 0.65f, 0.7f) );
      return;
    }
  case CICMapEditor::VIEW_MODEL:
    {
			NDb::CModel *pModel = NDb::GetModelVariant( nObjectID, &rand );
      if ( !IsValid( pModel ) )
        return;
			pTempModel = pModel;
			NGScene::CRenderNode *pR = pScene->CreateMesh( pModel, MakeTransform( CVec3(0,0,0) ) );
			objects.push_back( pR );
      return;
    }
	case CICMapEditor::VIEW_CONTAINER:
		SetContainer( nObjectID );
    return;
	case CICMapEditor::VIEW_PARTICLES:
    {
			NDb::CEffect *pEffect = NDb::GetEffect( nObjectID );
      if ( IsValid( pEffect ) )
			{
				NGScene::CCFBTransform *pMSR = new NGScene::CCFBTransform;
				pMSR->Set( MakeTransform( CVec3(0,0,0), 0 ) );
				CObjectBase *pP = pScene->CreateParticles( pEffect, 0, timer.GetTime(), pMSR );
				particles.push_back( pP );
			}
      return;
    }
  case CICMapEditor::VIEW_RNDMODEL:
    {
			NDb::CModel *pModel = NDb::GetModel( nObjectID );
      if ( !IsValid( pModel ) )
        return;
			NGScene::CRenderNode *pR = pScene->CreateMesh( pModel, MakeTransform( CVec3(0,0,0), 0 ) );
			objects.push_back( pR );
      return;
    }
  case CICMapEditor::VIEW_GEOMETRY:
    {
      pTempModel = new NDb::CModel();
      pTempModel->pGeometry  = NDb::GetGeometry( nObjectID );
			NDb::CMaterial *pDefMaterial = NDb::GetMaterial( NDb::N_DEF_MATERIAL_ID );
			if ( !IsValid( pTempModel->pGeometry ) || !IsValid( pDefMaterial ) )
				return;
			for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
			{
				pTempModel->pMaterials[i] = pDefMaterial;
			}
      //
			NGScene::CRenderNode *pR = pScene->CreateMesh( pTempModel, MakeTransform( CVec3(0,0,0), 0 ) );
			objects.push_back( pR );
//      pScene->SetAmbient( CVec3( 0.3f, 0.3f, 0.3f ) );
//      pScene->SetFogColor( CVec3(0.1f, 0.1f, 0.1f) );
			if ( IsValid( pTempModel->pGeometry->pAIGeometry ) )
			{
				AddAIGeometry( pTempModel->pGeometry->pAIGeometry->GetRecordID() );
			}
      return;
    }
  case CICMapEditor::VIEW_MATERIAL:
    {
      pTempModel  = new NDb::CModel();
      pTempModel2 = new NDb::CModel();
      pTempModel->pGeometry   = NDb::GetGeometry( NDb::N_CUBE_GEOMETRY_ID );
      pTempModel2->pGeometry  = NDb::GetGeometry( NDb::N_SPHERE_GEOMETRY_ID );
      if (  !IsValid( pTempModel->pGeometry ) || !IsValid( pTempModel2->pGeometry ) )
        return;
      NDb::CMaterial *pM = NDb::GetMaterial( nObjectID );
      if ( !IsValid( pM ) )
        return;
			for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
			{
	      pTempModel->pMaterials[i]  = pM;
				pTempModel2->pMaterials[i]  = pM;
			}
      //
      SMapElement me;
      
			NGScene::CRenderNode *pR;
			pR = pScene->CreateMesh( pTempModel, MakeTransform( CVec3(0,0,0), 0 ) );
			objects.push_back( pR );
			pR = pScene->CreateMesh( pTempModel2, MakeTransform( CVec3( 1.5f, 0, 0), 0 ) );
			objects.push_back( pR );
      SetCameraPos( CVec3( 1, 0, 2.5f ) );
			//      pScene->SetAmbient( CVec3( 0.3f, 0.3f, 0.3f ) );
			//      pScene->SetFogColor( CVec3(0.6f, 0.6f, 0.6f) );
      return;
    }
  case CICMapEditor::VIEW_ANIMATION:
    {
			NDb::CModel *pModel = NDb::GetModel( nExtra );
			NDb::CAnimation *pDbAnimation = NDb::GetAnimation( nObjectID );
      //
			if ( IsValid( pModel ) && IsValid( pDbAnimation ) )
			{
				vector<SSphere> addSpheres;
				NAnimation::CSkeletonAnimator *pAnimator = 0;
				pAnimator = new NAnimation::CSkeletonAnimator( pModel->pSkeleton );
				pAnimator->pTime = timer.GetTime();
	      pAnimator->AddAnimator( 0, pAnimator->CreateAnimation( pDbAnimation, 0, true ) );
				animators.push_back( pAnimator );
				NGScene::CRenderNode *pR = pScene->CreateSkin( pModel, pAnimator );
				objects.push_back( pR );
				float fAngleMult = 0;
				switch ( pDbAnimation->nType )
				{
					case NDb::CAnimation::ATTACK_CEILING:
					case NDb::CAnimation::ATTACK_UP:
						fAngleMult = 1;
						break;
					case NDb::CAnimation::ATTACK_FLOOR:
					case NDb::CAnimation::ATTACK_DOWN:
						fAngleMult = -1;
						break;
				}
				switch ( pDbAnimation->nType )
				{
					case NDb::CAnimation::ATTACK:
					case NDb::CAnimation::ATTACK_UP:
					case NDb::CAnimation::ATTACK_DOWN:
					case NDb::CAnimation::ATTACK_FLOOR:
					case NDb::CAnimation::ATTACK_CEILING:
						{
							NDb::CAnimWeaponType *pWType = NDb::GetAnimWeaponType( pDbAnimation->nFlags );
							if ( !pWType || pWType->type == NDb::WT_DEFAULT )
								break;
							CVec3 shootOrigin;
							if ( pDbAnimation->nFlags & NDb::CAnimation::POSE_STAND )
								shootOrigin = pWType->stand;
							else if ( pDbAnimation->nFlags & NDb::CAnimation::POSE_CROUCH )
								shootOrigin = pWType->crouch;
							else if ( pDbAnimation->nFlags & NDb::CAnimation::POSE_CRAWL )
								shootOrigin = pWType->crawl;
							float fShootDistance = pWType->fMinDistance;
							float fAngle = fAngleMult * ToRadian( (float)pDbAnimation->nAngle );
							CVec3 shootEnd = shootOrigin + CVec3( cos(fAngle) * fShootDistance, 0, sin(fAngle) * fShootDistance );
							addSpheres.push_back( SSphere( shootOrigin, 0.1f ) );
							addSpheres.push_back( SSphere( shootEnd, 0.1f ) );
						}
				}
				//
				vector<NWorld::IRenderVisitor::SBoundMesh> items;
				for ( int k = 0; k < gvAnimItems.size(); ++k )
				{
					if ( gvAnimItems[k].second < 0 )
						continue;
					NWorld::IRenderVisitor::SBoundMesh m;
					m.pszBindBone = gvAnimItems[k].first.c_str();
					m.pModel = NDb::GetModel( gvAnimItems[k].second );
					items.push_back( m );
				}
				AddItems( pAnimator, items, gnHeadID > 0 ? NDb::GetComplexHead( gnHeadID ) : 0 );
				//
				for ( int i=0; i<addSpheres.size(); ++i )
				{
					CPtr<CMemObject> pModel = new CMemObject;
					pModel->CreateSphere( addSpheres[i].ptCenter, addSpheres[i].fRadius, 2 );
					CVec4 color( 1, 1.0f, 1.0f, 1.0f );
					objects.push_back( pScene->CreateMesh( pModel, color, 0 ) );
				}
			}
			//      pScene->SetAmbient( CVec3( 0.3f, 0.3f, 0.3f ) );
			//      pScene->SetFogColor( CVec3(0.1f, 0.1f, 0.1f) );
      return;
    }
	case CICMapEditor::VIEW_AIMODEL:
		AddAIGeometry( nObjectID );
		break;
	case CICMapEditor::VIEW_SOUND:
		{
			NDb::CSound *pSnd = NDb::GetSound( nObjectID );
			if ( !pSnd )
				break;
			pSoundScene->Add2DSound( pSnd );
			break;
		}
	case CICMapEditor::VIEW_RPGITEM:
		{
			SRand rand;
			fFov = 60.f;
			NDb::CRPGItem *pItem = NDb::GetRPGItem( nObjectID );
			if ( !IsValid( pItem ) || !IsValid( pItem->pModel ) )
				return;
			CPtr<NDb::CModel> pModel = pItem->pModel->CreateModel( &rand );
			if ( !IsValid( pModel ) )
				return;
			if ( mapParams.camera.fRod != -1 )
				pCamera->SetPlacement( mapParams.camera );
			else
			{
				ICamera::SCameraPos pos;
				pCamera->GetPlacement( &pos );
				pos.fRod = 3;
				pCamera->SetPlacement( pos );
			}
			NGScene::CRenderNode *pR = pScene->CreateMesh( pModel, MakeTransform( CVec3(0,0,0), 0 ) );
      NDb::CTexture *pTexture = NDb::GetTexture( 1307 );
      if ( !IsValid( pTexture ) )
        return;
			ComputeRPGItemRect();
			return;
		}
	case CICMapEditor::VIEW_INTERFACE:
		Sleep(0);
		NDatabase::Refresh<NDb::CUIContainer>();
		NDatabase::Refresh<NDb::CUIControl>();
		NDatabase::Refresh<NDb::CUITexture>();
		NUI::LoadTemplate( pInterface, NDb::GetUIContainer( nObjectID ) );
		break;
	case CICMapEditor::VIEW_HEAD:
		{
			NDb::CComplexHead *pH = NDb::GetComplexHead( nObjectID );
			if ( pH && IsValid(pH->pHead) )
			{
				CPtr<NLSHead::CHeadAnimator> pAnimator = new NLSHead::CHeadAnimator( timer.GetTime(), pH->pHead );
				NGScene::CCFBTransform *pMSR = new NGScene::CCFBTransform;
				SFBTransform trans;
				CQuat q;
				q.FromEulerAngles( 0, ToRadian( -90.0f ), ToRadian( -90.0f ) );
				MakeMatrix( &trans.forward, CVec3(0,0,0), q );
				InvertMatrix( &trans.backward, trans.forward );
				//pMSR->Set( trans );
				pMSR->Set( MakeTransform( VNULL3 ) );
				pLG = pScene->CreateLightGroup();
				NGScene::CRenderNode *pR = pScene->CreateLSHead( pH, pAnimator, timer.GetTime(), pMSR, false, false, NGScene::SRoomInfo( pLG ) );
				objects.push_back( pR );
			}
		}
		break;
	case CICMapEditor::VIEW_OBJECT:
		nPhase = 0;
		SetObjectPhase( nObjectID, nExtra, nPhase );
		break;
	case CICMapEditor::VIEW_PERS:
		{
			fFov = 60.f;
			NDb::CRPGPers *pPers = NDb::GetPers( nObjectID );
			if ( !pPers || !IsValid( pPers->pModel ) )
				break;
			if ( !IsValid( pPers->pBaseValue ) )
			{
				MessageBox( 0, "Pers base values not found", "Warning", MB_OK );
				break;
			}
			NDb::CModel *pModel = pPers->pModel->CreateModel( &rand );
			if ( !IsValid( pModel ) )
				break;
			pTempModel = pModel;
			if ( !IsValid( pModel->pSkeleton ) )
			{
				MessageBox( 0, "Model skeleton not found", "Warning", MB_OK );
				break;
			}
			//
			if ( mapParams.camera.fRod != -1 )
				pCamera->SetPlacement( mapParams.camera );
			else
			{
				ICamera::SCameraPos pos;
				pCamera->GetPlacement( &pos );
				pos.fRod = 3;
				pCamera->SetPlacement( pos );
			}
			//
			NAnimation::CSkeletonAnimator *pAnimator = 0;
			pAnimator = new NAnimation::CSkeletonAnimator( pModel->pSkeleton );
			animators.push_back( pAnimator );
			pAnimator->pTime = timer.GetTime();
			//
			pUnitMission = NRPG::CreateUnit( pPers );
			// Animation
			int nAnimFlags = NDb::CAnimation::POSE_STAND;
			NRPG::IWeaponItemInfo *pActiveWItem = dynamic_cast<NRPG::IWeaponItemInfo*>( pUnitMission->GetInventory()->GetActive() );
			if ( pActiveWItem )
			{
				NDb::CRPGWeapon *pWeapon = pActiveWItem->GetDBWeapon();
				if ( IsValid( pWeapon ) && IsValid( pWeapon->pAnimWeaponType ) )
					nAnimFlags |= NDb::WeaponTypeToAnimFlags( pWeapon->pAnimWeaponType->type, true );
				else
					nAnimFlags |= NDb::CAnimation::WEAPON_NONE;
			}
			else
				nAnimFlags |= NDb::CAnimation::WEAPON_NONE;
			NDb::CAnimation *pA = pModel->pSkeleton->GetAnimation( NDb::CAnimation::IDLE, nAnimFlags );
			if ( !pA )
				pA = pModel->pSkeleton->GetAnimation( NDb::CAnimation::POSE, nAnimFlags );
			if ( pA )
				pAnimator->AddAnimator( 0, pAnimator->CreateAnimation( pA, 0, true ) );
			objects.push_back( pScene->CreateSkin( pModel, pAnimator ) );
			//
			vector<NWorld::IRenderVisitor::SBoundMesh> items;
			NWorld::GetItemsBindPlaces( &items, pUnitMission, false );
			AddItems( pAnimator, items, pPers->pHead );
			NDb::CTexture *pTexture = NDb::GetTexture( 1307 );
			if ( !IsValid( pTexture ) )
				break;
			ComputeRPGPersRect();
		}
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::AddItems( NAnimation::CSkeletonAnimator *pAnimator, const vector<NWorld::IRenderVisitor::SBoundMesh> &items, NDb::CComplexHead *pHead )
{
	bool bCap = false;
	for ( int k = 0; k < items.size(); ++k )
	{
		const NWorld::IRenderVisitor::SBoundMesh &m = items[k];
		int nIndex = pAnimator->GetBoneIndex( m.pszBindBone );
		if ( nIndex < 0 )
			continue;
		if ( !IsValid( m.pModel ) )
			continue;
		NAnimation::CAddBoneFilter *pFilter = new NAnimation::CAddBoneFilter( nIndex );
		pFilter->pAnimation = pAnimator;
		if ( gvAnimItems[k].first == "Cap" )
			bCap = true;
			
		objects.push_back( pScene->CreateMesh( m.pModel, pFilter ) );
	}
	//
	if ( IsValid( pHead ) )
	{
		NAnimation::CAddBoneFilter *pFilter = new NAnimation::CAddBoneFilter(12); // CRAP - head bone
		pFilter->pAnimation = pAnimator;
		if ( pHead->pHead )
		{
			pHAnimator = new NLSHead::CHeadAnimator(pHeadsController->GetTime(), pHead->pHead );
			pLG = pScene->CreateLightGroup();
			objects.push_back( pScene->CreateLSHead( pHead, pHAnimator, pHeadsController->GetTime(), pFilter, false, bCap, NGScene::SRoomInfo( pLG ) ) );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::AddAIGeometry( int nAIGeomID, const CVec3 &ptShift )
{
	list<NAI::SObjectInfo> objs;
	vector<SMassSphere> spheres;
	bool bClosed;
	NAI::GetGeometry( &objs, &spheres, nAIGeomID, &bClosed );
	SFBTransform id;
	id = MakeTransform( ptShift, 0 );
	for ( list<NAI::SObjectInfo>::iterator i = objs.begin(); i != objs.end(); ++i )
	{
		NAI::SObjectInfo &info = *i;
		if ( info.tris.size() > 0 )
		{
			CPtr<CMemObject> pModel = new CMemObject;
			pModel->Create( info.points, info.tris );
			CVec4 color = CVec4( 0.3f, 0.3f, 1, 0.5f );
			NGScene::CRenderNode *pR = pScene->CreateMesh( pModel, color, id );
			objects.push_back( pR );
		}
		if ( !bClosed )
			AfxMessageBox( " Non closed AI geometry! ", MB_ICONEXCLAMATION | MB_OK );
		//
		for ( int i=0; i<spheres.size(); ++i )
		{
			CPtr<CMemObject> pModel = new CMemObject;
			pModel->CreateSphere( spheres[i].ptCenter, spheres[i].fRadius, 2 );
			CVec4 color( 1, 0.7f, 0.7f, 0.7f );
			objects.push_back( pScene->CreateMesh( pModel, color, 0 ) );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::SetCameraPos( const CVec3 &ptCameraPos )
{
  ICamera::SCameraPos pl;
  pl.ptAnchor = ptCameraPos;
	pl.ptAnchor.z = 0;
  pl.fRod   = ptCameraPos.z;
  pl.fPitch = ToRadian( -89.0f );
  pl.fYaw   = ToRadian( -0.0f );
  pCamera->SetPlacement( pl );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::SetCameraPos( const ICamera::SCameraPos &pos )
{
  pCamera->SetPlacement( pos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::GetCameraPos( ICamera::SCameraPos *pPos ) const
{
  pCamera->GetPlacement( pPos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CMapViewer::CMapViewer() : pTempModel( 0 ),
	cExec( "vk_enter" ), 
	cExit( "exit" ), 
	cSave( "save" ), 
	cLoad( "load" ),
	cBuildParts( "explode" ),
	cCutFloorUp( "next_floor" ),
	cCutFloorDown( "prev_floor" ),
	cShadows("toggle_shadows"),
	cRoll("roll" ),
	cUpdate("update"),
	cFog("toggle_fog"),
	cHSR("toggle_hsr"),
	cCameraInfo("camera_info"),
	cNextPhase( "next_phase" ),
	cCameraReset( "camera_reset" ),
	cChapterCamera( "chapter_camera" )
{
  pScene = NGScene::CreateNewView();
	pScene->SetFastMode();
  pScene->SetAmbient( CVec3( 0.5f, 0.5f, 0.5f ) );
//	pScene->SetNextHSRMode();
//  pScene->SetFogColor( CVec3(0.2f, 0.2f, 0.2f) );
  p2DScene = NGScene::CreateNew2DView();
	pCamera = CreateCamera( CAMERA_MAYA, gfMayaCameraSpeed );
	pCursor = NUI::ICursor::Create( false );
	pInterface = new NUI::CInterface( pCursor );
	pSoundScene = NSound::CreateSoundScene( 0 );
	pFrameRect = new NGScene::CCTRect;
	GetWindowRect( hWnd, &rLastWindowRect );
	pFrameTex = NDb::GetTexture( 1307 );
	pHeadsController = new NLSHead::CHeadsController;
	nPhase = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::SetupLight( SLightPrefs prefs, NGScene::ESceneRenderMode mode, NGScene::EFogMode fogMode )
{
	CVec3 vLightColor = prefs.vLightColor - prefs.vAmbientColor;
	vLightColor.Maximize( VNULL3 );
	const float fToDegrees = 180.0 / PI;
	CVec3 ptDir = prefs.ptLight;
	Normalize( &ptDir );
	float fPitch = fToDegrees * acos( -ptDir.z );
	float fSinP = sin( ToRadian( fPitch ) );
	//float fYaw = fabs( ptDir.x ) < 1e-3 ? ( fabs( ptDir.y ) < 1e-3 ? 0 : 90 ) : fToDegrees * atan( ptDir.y / ptDir.x );
	float fYaw = fabs( fSinP ) < 1e-3 ? 0 : Sign( ptDir.y ) * fToDegrees * acos( ptDir.x / fSinP );

	CPtr<NDb::CAmbientLight> pLight = new NDb::CAmbientLight;
	pLight->vAmbientColor = prefs.vAmbientColor;
	pLight->vLightColor   = vLightColor;
	pLight->vGlossColor   = prefs.vGlossColor;
	pLight->vFogColor     = prefs.vFogColor;
	pLight->vVapourColor  = prefs.vVapourColor;
	pLight->vShadowColor  = prefs.vShadowColor;
	pLight->fPitch        = fPitch;
	pLight->fYaw          = fYaw;
	pLight->fFogDistance  = prefs.fFogDistance;
	pLight->fVapourHeight = prefs.fVapourHeight;
	pLight->fVapourDensity = prefs.fVapourDensity;
	pLight->fVapourNoiseParam = prefs.fVapourNoiseParam;
	pLight->fVapourSpeed = prefs.fVapourSpeed;
	pLight->fVapourSwitchTime = prefs.fVapourSwitchTime;
	pLight->fFogStartDistance = prefs.fFogStart;
	pLight->pSky = NDb::GetCubeTexture( prefs.nSkyID );
	pLight->vBackColor = prefs.vBackColor;

	pScene->SetAmbient( pLight );
	pScene->SetRenderMode( mode );
	pScene->SetFogMode( fogMode );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::SetRect( float fw, float fh )
{
	if ( !IsValid( pFrameTex ) )
		return;

	CRect rw;
	GetWindowRect( hWnd, &rw );
	CTRect<int> r;
	int nSide = (36.0f / 1024) * rw.Width();
	int w = fw * nSide;
	int h = fh * nSide;
	r.left = (rw.Width() - w) / 2;
	r.right = r.left + w;
	r.top = (rw.Height() - h) / 2;
	r.bottom = r.top + h;
	pFrameRect->Set( r );

	sLayout = CRectLayout();
	sLayout.AddRect( r.left, r.top, CRectLayout::STextureCoord( CTRect<float>( 0, 0, pFrameTex->nWidth, pFrameTex->nHeight ) ) );
	sLayout.scale.x = float(w) / pFrameTex->nWidth;
	sLayout.scale.y = float(h) / pFrameTex->nHeight;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::ComputeRPGItemRect()
{
	NDb::CRPGItem *pItem = NDb::GetRPGItem( mapParams.nObjectID );
	if ( !IsValid( pItem ) )
		return;

	float w, h;
	switch ( geRPGInventoryCamera )
	{
		case CAM_SLOT:
			w = 5.74f;
			h = 2.63f;
			break;
		case CAM_AMMO:
			w = 1.3333f;
			h = 1.3333f;
			break;
		default:
			w = pItem->sSize.x;
			h = pItem->sSize.y;
			break;
	}
	SetRect( w, h );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMapViewer::ComputeRPGPersRect()
{
	SetRect( 2.8056f, 2.8611f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMapEditor
////////////////////////////////////////////////////////////////////////////////////////////////////
CICMapEditor::CICMapEditor( const SMapParams &params, HWND hwnd, bool _bBuildMap ) 
	: mapParams( params ), bBuildMap(_bBuildMap), hWnd(hwnd)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICMapEditor::Exec()
{
  CIMapEditor *pOldI = dynamic_cast<CIMapEditor*>( GetInterface() );
  ICamera::SCameraPos pos;
	NGScene::ESceneRenderMode mode = NGScene::RM_BEST_GF3;
	NGScene::EFogMode fogMode = NGScene::FOG_NONE;
  CMapViewer *pView = new CMapViewer();
	pView->SetHWND( hWnd );
  if ( pOldI )
  {
    pOldI->GetCameraPos( &pos );
		CMapViewer *pMV = dynamic_cast<CMapViewer*>( GetInterface() );
		if ( pMV )
		{
			mode = pMV->GetRenderMode();
			fogMode = pMV->GetFogMode();
			pView->SetCutFloor( pMV->GetCutFloor() );
		}
  }
	mapParams.bCameraReset = GetUserSettings().IsCameraReset();
  ResetStack();
	pView->SetCommand( mapParams );
  if ( pOldI /*&& !mapParams.bCameraReset*/ )
    pView->SetCameraPos( pos );
  else
    pView->SetCameraPos( mapParams.ptCameraPos );
	if ( bBuildMap )
		pView->BuildMap( mapParams.nView, mapParams.nObjectID, mapParams.nExtra );
	else
		pView->BuildMap( mapParams.nView, -1, mapParams.nExtra );
  pView->SetupLight( mapParams.lightPrefs, mode, fogMode );
  SetInterface( pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
