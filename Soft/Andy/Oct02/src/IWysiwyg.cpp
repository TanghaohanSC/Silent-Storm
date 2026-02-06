#include "StdAfx.h"
#include "GView.h"
#include "GSceneUtils.h"
#include "iMain.h"
#include "aiMap.h"
#include "Camera.h"
#include "Transform.h"
#include "iWysiwyg.h"
#include "Interface.h"
#include "wEditor.h"
#include "RWGame.h"
#include "iAIViewer.h"
#include "iMapEditor.h"
#include "WysiwygSelection.h"
#include "WysiwygMaterial.h"
#include "Grid.h"
#include "WysiwygSpotSel.h"
#include "..\Misc\BasicShare.h"
#include "MakeBuilding.h"
#include "BuildingGrid.h"
#include "BSchemaViewer.h"
#include "MEUserSettings.h"
#include "MESerialize.h"
#include "MELayers.h"
#include "..\Misc\HPTimer.h"
#include "aiWaypoint.h"
#include "WysiwygClipboard.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataFormat.h"
#include "..\Input\Bind.h"
#include "G2DView.h"
#include "WysiwygTerrain.h"
#include "meDragDrop.h"
#include "weInterface.h"
#include "..\MapEdit\WaypointDB.h"
#include "..\MapEdit\TemplDBCmd.h"
#include "WysiwygUndo.h"

extern bool bWYSIWYGActive;
extern float gfMayaCameraSpeed;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
	extern CBasicShare<int, NBuilding::CBuildInfoLoader> shareBuildings;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NBuilding
{
	class CBuildingGrid;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWysiwyg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
int AddTerrSpotDB( int nVarID, const NBuilding::SProjectedSpot &spot );
extern bool UpdateTerrSpotDB( const NDb::CTerrainSpot *pSpot );
void UpdateWallSpot( int nSpotID, NBuilding::CBuildInfo *pInfo, NBuilding::CBuildingGrid *pGrid );
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 200.0f;
////////////////////////////////////////////////////////////////////////////////////////////////////
int MakeUserID( EBrushType nBrush, int nFragmentID )
{
	return (int)nBrush << 24 | nFragmentID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetFragmentID( int nUserID, EBrushType *pnBrush, int *pnFragmentID )
{
	ASSERT( pnBrush && pnFragmentID );
	*pnBrush = (EBrushType)(nUserID >> 24);
	*pnFragmentID = nUserID & 0xFFFFFF;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void DrawLine( NGScene::IGameView *pScene, const CVec3 &src, const CVec3 &dst, const CVec3 &color,
	list<CObj<NGScene::CPolyline> > *pLines )
{
	vector<CVec3> points;
	points.push_back( src );
	points.push_back( dst );
	pLines->push_back( pScene->CreatePolyline( points, color ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void DrawGrid( NGScene::IGameView *pScene, CVec2 ptSize, float fStep, const CVec3 &color, list<CObj<NGScene::CPolyline> > *pLines )
{
	char buf[512];
	sprintf( buf, "WYSIWYG GRID: %d x %d, step = %f\n", (int)ptSize.x, (int)ptSize.y, fStep );
	OutputDebugString( buf );
	for ( float y = 0; y <= ptSize.y; y += fStep )
		for ( float x = 0; x <= ptSize.x; x += fStep )
		{
			DrawLine( pScene, CVec3( x, 0, -0.02f ), CVec3( x, ptSize.y, -0.02f ), color, pLines );
			DrawLine( pScene, CVec3( 0, y, -0.02f ), CVec3( ptSize.x, y, -0.021f ), color, pLines );
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CWysiwyg
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWysiwyg: public CIMapEditor
{
	OBJECT_BASIC_METHODS(CWysiwyg);
	//
	HWND hWnd;
	CObj<NWorld::CEditorWorld> pWorld;
	CObj<NGScene::IGameView> pScene;
	CObj<NGScene::I2DGameView> p2DScene;
	CObj<NRender::IRenderGame> pRender;
	CPtr<NAI::IAIMap> pAIMap;
	int nBuilding;
	CPtr<ICamera> pCamera;
	CPtr<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	//CPtr<CObjectBase> pSelection;
	NInput::CBind cExit, cPrevLayer, cAIView, cNewSpot, cStability, cViewSchema, cCutFloorUp, cCutFloorDown;
	NInput::CBind cShadows, cLBDown, cLBUp, cSave, cDelete, cReconstruct, cLoad, cNewTerrSpot, cCenter, cDeselect;
	NInput::CBind cUpdate, cHSR, cNewWaypoint, cCopy, cPaste, cNewObject, cUpdateObjects, cSelectAll, cUpdateTerrain;
	NInput::CBind cTopmostSpot, cSelectSpotFragments, cAssingSpotFragments, cSerializeBuilding, cNewLadder;
	NInput::CBind cRotate, cUpdateSubTemplates, cUpdateSubTemplate, cCameraReset, cUpdateUnit, cSelectionTemplate;
	NInput::CBind cTemplatePaste, cUpdateLadder, cUndo, cRedo, cUpdateObject, cUpdateWallSpot, cShowInfo;
	list<CObj<NGScene::CRenderNode> > objects, tracePoints;
	list<CObj<NGScene::CRenderNode> > knots;
	list<CObj<NGScene::CPolyline> > traceLines;
	list<CObj<NGScene::CPolyline> > gridlines;
	int nCurrentLayer;
	CObj<ISelection> pSelection;
	CObj<CWysiwygMaterial> pMaterial;
	CObj<CWysiwygTerrain> pTerrain;
	CPtr<NBuilding::CBuildingGrid> pBGrid;
	CObj<CObjectBase> pViewSchema;
	CVec2 ptMapSize;
	CPtr<NDb::CTemplVariant> pVariant;
	CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pBuildingInfo;
	NHPTimer::STime dblClickTime;
	CObj<IDragDrop> pDragAndDrop;
	bool bShowInfo;
	//
	int nUserIDRequest;
	CVec3 ptCrossRequest;
	CVec3 ptNormalRequest;
	CObjectBase *pObjRequest;
	// 
	bool GetPointUnderCursor( CVec2 *pRes );
	CObjectBase* GetObjectUnderCursor( int *pnUserID, CVec3 *pptCross, CVec3 *pptCrossFaceNormal, int nFlags = NWorld::TS_ALL | NWorld::TS_PICK );
	void SortTracedObjects( vector<NAI::SInterval> *pIntervals );
	bool Select( CObjectBase *pObject, int nUserID, const CVec3 &ptCrossFaceNormal ); // true, если тыкнулись в уже поселекченный объект
	void NewTexSpot( const CVec3 &ptCross, const CVec3 &ptNormal );
	void NewTerrSpot();
	void NewWaypoint();
	void NewObject();
	void NewLadder();
	bool HitTest( const CVec2 &ptHit );
	void Paste( bool bNewTemplatePaste );

public:
	CWysiwyg( HWND hWnd = 0, int nVariantID = -1 );
	//
	virtual void GetCameraPos( ICamera::SCameraPos *pPos ) const { pCamera->GetPlacement( pPos ); }
	virtual void SetCamera( const ICamera::SCameraPos &cameraPos );
	virtual int  GetSelectionMask() const;
	virtual void SaveHitTestObject();
	virtual bool ProcessEvent( const NInput::SEvent &eEvent );
	virtual void Step();
	virtual void OnGetFocus();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CWysiwyg::CWysiwyg( HWND _hWnd, int nVariantID )
	: hWnd( _hWnd ), cExit("exitAIView"), cPrevLayer("aiPrevLayer"), cShadows("toggle_shadows"), 
	cLBDown("leftbutton_down"), cLBUp("leftbutton_up"), cAIView("showai"), cSave("save"), cDelete( "delete" ),
	cNewSpot("new_texprojection"), cStability( "checkStability" ), cReconstruct( "reconstruct" ),
	cViewSchema( "viewSchema" ), cLoad( "load" ), cNewTerrSpot("new_terrspot"), 
	cCutFloorUp( "next_floor" ), cCutFloorDown( "prev_floor" ), cCenter( "center" ), cDeselect( "deselect" ),
	cUpdate( "update" ), cHSR( "toggle_hsr" ), cNewWaypoint( "new_waypoint" ), cCopy( "copy" ), cPaste( "paste" ),
	cNewObject( "new_object" ), cUpdateObjects( "update_objects" ), cSelectAll( "select_all" ), 
	cUpdateTerrain( "update_terrain" ), cTopmostSpot( "topmost_spot" ), cSelectSpotFragments( "select_spot_fragments" ),
	cAssingSpotFragments( "assign_spot_fragments" ), cSerializeBuilding( "serialize_building" ), cNewLadder( "new_ladder" ),
	cRotate( "rotate" ), cUpdateSubTemplates( "update_subtemplates" ), cUpdateSubTemplate( "update_subtemplate" ),
	cCameraReset( "camera_reset" ),	cUpdateUnit( "update_unit" ), cSelectionTemplate( "selection_template" ),
	cTemplatePaste( "template_paste" ), cUpdateLadder( "update_ladder" ), cUndo( "undo" ), cRedo( "redo" ),
	cUpdateObject( "update_object" ), cUpdateWallSpot( "update_wallspot" ), cShowInfo( "show_info" )
{
	nBuilding = nVariantID;
	NWorld::CEditorWorld *pW = new NWorld::CEditorWorld;
	pWorld = pW;
	pWorld->CreateRandom( nVariantID, false, list<CPtr<NScenario::CScenarioClue> >(), 0, 0, SRandomSeed( GetTickCount() ) );
	pBGrid = pW->GetMainBuilding();
	pAIMap = pWorld->GetAIMap();
	pScene = NGScene::CreateNewView();
	pScene->SetFastMode();
	pRender = NRender::CreateRenderGame( pWorld, pScene );
	p2DScene = NGScene::CreateNew2DView();
	pScene->AddDirectionalLight( CVec3(0.8f,0.8f,0.8f), CVec3( 0.1f,0.1f,-1), CVec3(-1,-1,0), CVec2( 150, 150 ), 20 );
	pScene->SetAmbient( CVec3( 0.4f,0.4f,0.4f ) );
	pCamera = CreateCamera( CAMERA_MAYA, gfMayaCameraSpeed );
	pSelection = CreateSelection( nVariantID, pWorld, pW->GetMainBuilding(), pScene, pCamera );
	pMaterial = new CWysiwygMaterial( pWorld, pScene, pCamera );
	pTerrain = new CWysiwygTerrain( pW, pScene, nBuilding );
	pDragAndDrop = CreateDragAndDrop( nBuilding, pScene, pCamera, pSelection );
	pScene->SetNextHSRMode();
	///
	pCursor = NUI::ICursor::Create( false );
	pInterface = new NUI::CInterface( pCursor );
	nCurrentLayer = -1;

	pObjRequest = 0;
	bShowInfo = true;

	ptMapSize = VNULL2;
  CDBTable<NDb::CTemplVariant> *pVarTable = NDatabase::GetTable<NDb::CTemplVariant>();
  if ( !pVarTable )
		return;
	pVariant = pVarTable->GetRecord( nVariantID );
	if ( IsValid( pVariant ) && IsValid( pVariant->pTemplate ) )
		ptMapSize = CVec2( pVariant->pTemplate->nWidth * FP_GRID_STEP, pVariant->pTemplate->nHeight * FP_GRID_STEP );
	if ( GetUserSettings().IsGridVisible() )
		DrawGrid( pScene, ptMapSize, FP_GRID_STEP, CVec3(1,1,1), &gridlines );
	pBuildingInfo = NGScene::shareBuildings.Get( nBuilding );
	NHPTimer::GetTime( &dblClickTime );
	NMapEditor::SetNewWorld( pW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::SetCamera( const ICamera::SCameraPos &cameraPos )
{
	pCamera->SetPlacement( cameraPos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::OnGetFocus()
{
	pSelection->Clear();
	Step();
}
inline void EraseSpot( vector<NBuilding::SBuildFragment> *pFrags, const vector<int> &frags, int nSpotID )
{
	for ( int i = 0; i < frags.size(); ++i )
	{
		int nInd = frags[i];
		if ( nInd < 0 || nInd >= pFrags->size() )
		{
			ASSERT(0);
			continue;
		}
		NBuilding::SBuildFragment &fr = (*pFrags)[nInd];
		if ( fr.spots.empty() )
			return;
		vector<int>::iterator it = find( fr.spots.begin(), fr.spots.end(), nSpotID );
		if ( it != fr.spots.end() )
			fr.spots.erase( it );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWysiwyg::HitTest( const CVec2 &ptHit )
{
	if ( ptHit.y <= 0 || ptHit.x <= 0 ) // похоже кликнули за пределами окна
		return false;
	CVec2 r = pScene->GetScreenRect();
	if ( ptHit.y >= r.y || ptHit.x >= r.x )
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWysiwyg::ProcessEvent( const NInput::SEvent &eEvent )
{
	NInput::SetSection( "mapeditor" );

	if ( bWYSIWYGActive )
	{ // Update cursor
		CPoint sPoint;
		GetCursorPos( &sPoint );
		ScreenToClient( hWnd, &sPoint );

		CVec2 scrSize = pScene->GetScreenRect();
		CVec2 vPos;
		vPos.x = sPoint.x;
		vPos.y = sPoint.y;
		vPos.x = Max( vPos.x, 0.0f );
		vPos.x = Min( vPos.x, scrSize.x ); 
		vPos.y = Max( vPos.y, 0.0f );
		vPos.y = Min( vPos.y, scrSize.y );
		pCursor->SetPos( vPos );
	}
	else
	{
		pSelection->OnLButtonUp( pCursor->GetPos() );
//		pSelection->Clear();
	}
	pCamera->ProcessEvent( eEvent );

	bool bRet = pInterface->ProcessEvent( eEvent );
	if ( bRet )
		return true;
	else if ( cShadows.ProcessEvent( eEvent ) )
		pScene->SetNextShadowsMode();
	else if ( cHSR.ProcessEvent( eEvent ) )
		pScene->SetNextHSRMode();
	else if ( cExit.ProcessEvent( eEvent ) && bWYSIWYGActive )
	{
		pSelection->Clear();
//		NMainLoop::Command( 0 );
//		return true;
	}
	else if ( cSave.ProcessEvent( eEvent ) )
	{
		return true;
	}
	else if ( cLBDown.ProcessEvent( eEvent ) && bWYSIWYGActive )
	{
		bool bSelEmpty = pSelection->IsEmpty();
		double dTime = NHPTimer::GetTimePassed( &dblClickTime );
		bool bDblClick = !bSelEmpty && dTime < 0.4;
		if ( ::GetFocus() != hWnd || !bWYSIWYGActive )
			return true;
		CVec2 ptCursor = pCursor->GetPos();
		if ( !HitTest( ptCursor ) ) // похоже кликнули за пределами окна
			return true;
		int nUserID, nTerrUserID;
		CVec3 ptCross;
		CVec3 ptNormal, ptTerrNormal;
		CVec3 ptTerrCross( PT_INVALID );
		CObjectBase *pTerrObj = GetObjectUnderCursor( &nTerrUserID, &ptTerrCross, &ptTerrNormal, NWorld::TS_TERRAINS );
		CObjectBase *pObj = GetObjectUnderCursor( &nUserID, &ptCross, &ptNormal );
		ELayer eType;
		int nLayer;
		NBuilding::GetLayerID( GetUserSettings().GetActiveLayerID(), &eType, &nLayer );
		//
		const IUserSettings &settings = GetUserSettings();
		switch ( settings.GetMode() )
		{
			case EM_SELECT:
				if (  IsTerrainLayer( eType ) )
				{
					pTerrain->OnLButtonDown( CVec2( ptTerrCross.x, ptTerrCross.y ), pTerrObj, nTerrUserID, ptTerrNormal );
				}
				else if ( !(0x8000 & GetAsyncKeyState( VK_CONTROL )) )
				{
					bDblClick = Select( pObj, nUserID, ptNormal ) ? bDblClick : false;
				}
				else
				{
					if ( bSelEmpty && settings.GetSelectedBrushID( settings.GetActiveLayerID() ) > 0 )
					{
						//pSelection->AddObject( pCursor->GetPos() );
					}
					else
						bDblClick = Select( pObj, nUserID, ptNormal ) ? bDblClick : false;
				}
				if ( bDblClick )
					pSelection->OnLBDblClick( pCursor->GetPos() );
				break;
			case EM_MATERIAL:
				pSelection->Clear();
				pMaterial->OnLButtonDown( pCursor->GetPos(), pObj, nUserID, ptNormal );
				break;
			case EM_ERASE:
				if ( nUserID == BT_TERRAIN )
				{
					pTerrain->Erase( CVec2( ptTerrCross.x, ptTerrCross.y ) );
				}
				break;
			case EM_RECTANGULAR_SELECTION:
				if ( !(0x8000 & GetAsyncKeyState( VK_CONTROL )) )
					pSelection->Clear();
				break;
			default:
				pSelection->Clear();
				break;
		}
		pSelection->OnLButtonDown( pCursor->GetPos() );
	}
	else if ( cLBUp.ProcessEvent( eEvent ) && bWYSIWYGActive )
	{
		bool bSelEmpty = pSelection->IsEmpty();
		const IUserSettings &settings = GetUserSettings();
		switch ( settings.GetMode() )
		{
			case EM_SELECT:
				if ( 0x8000 & GetAsyncKeyState( VK_CONTROL ) )
				{
					//if ( bSelEmpty && settings.GetSelectedBrushID( settings.GetActiveLayerID() ) > 0 )
					//	pSelection->AddObject( pCursor->GetPos() );
				}
				break;
		}
		return true;
	}
	else if ( cDelete.ProcessEvent( eEvent ) && bWYSIWYGActive )
	{
		pSelection->DeleteSelected();
		return true;
	}
	else	if ( cAIView.ProcessEvent( eEvent ) && bWYSIWYGActive)
	{
		ICamera::SCameraPos cameraPos;
		pCamera->GetPlacement( &cameraPos );
		NMainLoop::Command( new CICAIView( pWorld->GetAIMap(), pWorld->GetPathNetwork(), cameraPos ) );
		return true;
	}
	else if ( cNewSpot.ProcessEvent( eEvent ) )
	{
		if ( pObjRequest )
			NewTexSpot( ptCrossRequest, ptNormalRequest );
	}
	else if ( cStability.ProcessEvent( eEvent ) )
	{
		NBuilding::UpdateBuildingStability( nBuilding, pBGrid );
	}
	else if ( cReconstruct.ProcessEvent( eEvent ) )
	{
		if ( IsValid( pBGrid ) )
			pBGrid->Reset();
	}
	else if ( cViewSchema.ProcessEvent( eEvent ) )
	{
		CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pLoader = NGScene::shareBuildings.Get( nBuilding );
		pLoader.Refresh();
		NBuilding::CBuildInfo *pBInfo = pLoader->GetValue();
		if ( !pBInfo )
			return true;
		if ( IsValid( pViewSchema ) )
		{
			pViewSchema = 0;
			pWorld->ShowMainBuilding( true );
		}
		else
		{
			pViewSchema = ViewBuildingSchema( pScene, pBInfo, pBGrid, MakeTransform( VNULL3 ) );
			pWorld->ShowMainBuilding( false );
		}
	}
	else if ( cLoad.ProcessEvent( eEvent ) )
		return true;
	else if ( cNewTerrSpot.ProcessEvent( eEvent ) )
	{
		NewTerrSpot();
	}
	else if ( cCutFloorUp.ProcessEvent( eEvent ) && bWYSIWYGActive )
	{
		pScene->SetCutFloor( pScene->GetCutFloor() + 1 );
		pBGrid->SetCutFloor( pScene->GetCutFloor() );
		CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pLoader = NGScene::shareBuildings.Get( nBuilding );
		pLoader->Updated();
		pWorld->UpdateCutFloor();
		return true;
	}
	else if ( cCutFloorDown.ProcessEvent( eEvent ) && bWYSIWYGActive )
	{
		pScene->SetCutFloor( pScene->GetCutFloor() - 1 );
		pBGrid->SetCutFloor( pScene->GetCutFloor() );
		CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pLoader = NGScene::shareBuildings.Get( nBuilding );
		pLoader->Updated();
		pWorld->UpdateCutFloor();
		return true;
	}
	else if ( cCenter.ProcessEvent( eEvent ) )
	{
		SBound box;
		if ( pSelection->GetSelectionBound( &box ) )
		{
			static const float fTan = tan( FP_PI8 );
			ICamera::SCameraPos pos;
			pCamera->GetPlacement( &pos );
			pos.ptAnchor = box.s.ptCenter;
			pos.fRod = 3 + box.s.fRadius / fTan;
			SFBTransform tr = pWorld->GetTerrainTransform( pos.ptAnchor.x, pos.ptAnchor.y );
			CVec3 ptShift( VNULL3 );
			tr.forward.RotateHVector( &ptShift, ptShift );
			pos.ptAnchor.z += ptShift.z;
			pCamera->SetPlacement( pos );
		}
	}
	else if ( cDeselect.ProcessEvent( eEvent ) )
	{
		pSelection->Clear();
	}
	else if ( cUpdate.ProcessEvent( eEvent ) )
	{
		pBuildingInfo->Updated();
		pWorld->UpdateAll();
	}
	else if ( cNewWaypoint.ProcessEvent( eEvent ) )
	{
		NewWaypoint();
		return true;
	}
	else if ( cCopy.ProcessEvent( eEvent ) )
	{
		pSelection->OnCopy();
		SetClipboardData();
		return true;
	}
	else if ( cPaste.ProcessEvent( eEvent ) )
	{
		Paste( false );
		return true;
	}
	else if ( cTemplatePaste.ProcessEvent( eEvent ) )
	{
		Paste( true );
		SetActiveTemplateVariant( nBuilding );
		return true;
	}
	else if ( cNewObject.ProcessEvent( eEvent ) )
	{
		NewObject();
		return true;
	}
	else if ( cUpdateObjects.ProcessEvent( eEvent ) )
	{
		pWorld->UpdateObjects();
		return true;
	}
	else if ( cUpdateObject.ProcessEvent( eEvent ) )
	{
		pWorld->UpdateObject( GetUserSettings().GetSelectedBrushID( LID_OBJECTS ) );
		return true;
	}
	else if ( cSelectAll.ProcessEvent( eEvent ) )
	{
		pSelection->SelectAll();
		return true;
	}
	else if ( cUpdateTerrain.ProcessEvent( eEvent ) )
	{
		pWorld->UpdateTerrain();
		return true;
	}
	else if ( cTopmostSpot.ProcessEvent( eEvent ) )
	{
		pSelection->ProcessEvent( "topmost_spot" );
		return true;
	}
	else if ( cSelectSpotFragments.ProcessEvent( eEvent ) )
	{
		int nSpotID = pSelection->GetSelectedSpotID();
		if ( nSpotID < 0 )
			return true;
		pBuildingInfo.Refresh();
		NBuilding::CBuildInfo *pInfo = pBuildingInfo->GetValue();
		SForceSelection sel;
		vector<int> walls;
		pInfo->GetSpotFragments( nSpotID, &sel.fragsUserIDs, &walls );
		sel.nWorldID = nBuilding;
		for ( int i = 0; i < walls.size(); ++i )
			sel.fragsUserIDs.push_back( walls[i] | 0x40000000 );
		sel.wallSpotsIDs.push_back( nSpotID );
		pSelection->Select( sel );
		return true;
	}
	else if ( cAssingSpotFragments.ProcessEvent( eEvent ) )
	{
		int nSpotID = pSelection->GetSelectedSpotID();
		if ( nSpotID < 0 )
			return true;
		pBuildingInfo.Refresh();
		NBuilding::CBuildInfo *pInfo = pBuildingInfo->GetValue();
		vector<int> walls, solids;
		pInfo->GetSpotFragments( nSpotID, &solids, &walls );
		EraseSpot( &pInfo->solidFragments, solids, nSpotID );
		EraseSpot( &pInfo->wallFragments, walls, nSpotID );
		pSelection->AssignBuildingSpot( nSpotID );
		return true;
	}
	else if ( cSerializeBuilding.ProcessEvent( eEvent ) )
	{
		pBuildingInfo.Refresh();
		SerializeBuilding( pBuildingInfo->GetValue(), nBuilding );
		pBuildingInfo->Updated();
		pWorld->UpdateAllBuildingParts();
		return true;
	}
	else if ( cNewLadder.ProcessEvent( eEvent ) )
	{
		NewLadder();
		return true;
	}
	else if ( cRotate.ProcessEvent( eEvent ) )
	{
		pSelection->Rotate( 90 );
		return true;
	}
	else if ( cUpdateSubTemplates.ProcessEvent( eEvent ) )
	{
		pWorld->UpdateSubTemplates();
		return true;
	}
	else if ( cUpdateSubTemplate.ProcessEvent( eEvent ) )
	{
		pWorld->UpdateSubTemplate( MakeUserID( BT_SUBTEMPLATE, GetUserSettings().GetSelectedBrushID( LID_SUBTEMPLATES ) ) );
		return true;
	}
	else if ( cCameraReset.ProcessEvent( eEvent ) )
	{
		ICamera::SCameraPos pos;

		float l = 0.5f * Max( ptMapSize.x, ptMapSize.y );
		float z = 4.0f + l / tan( ToRadian( 17.5f ) );
		pos.fRod = Max( z, 5.0f );
		pos.ptAnchor.x = 0.5f * ptMapSize.x;
		pos.ptAnchor.y = 0.5f * ptMapSize.y;
		pos.ptAnchor.z = 0;
		pos.fPitch = ToRadian( -89.0f );
		pos.fYaw   = ToRadian( -0.0f );
		pCamera->SetPlacement( pos );
		return true;
	}
	else if ( cUpdateUnit.ProcessEvent( eEvent ) )
	{
		pWorld->UpdateUnit( GetUserSettings().GetSelectedBrushID( LID_UNITS ) );
		return true;
	}
	else if ( cSelectionTemplate.ProcessEvent( eEvent ) )
	{
		if ( pSelection->IsEmpty() )
			return true;
		SBound b;
		pSelection->GetSelectionBound( &b );
		int nX = 1 + ceil( FP_INV_GRID_STEP * 2 * b.ptHalfBox.x );
		int nY = 1 + ceil( FP_INV_GRID_STEP * 2 * b.ptHalfBox.y );
		int nID = AddNewTemplate( nX, nY );
		if ( nID > 0 )
		{
			pSelection->OnCopy();
			SetClipboardData();
			Sleep(10);
			NDatabase::Refresh<NDb::CTemplate>();
			NDatabase::Refresh<NDb::CTemplVariant>();
			ICamera::SCameraPos pos;
			pCamera->GetPlacement( &pos );
			NMainLoop::Command( new CICWysiwyg( hWnd, nID, pos, true ) );
			NInput::PostEvent( "camera_reset" );
		}
		return true;
	}
	else if ( cUpdateLadder.ProcessEvent( eEvent ) )
	{
		pWorld->UpdateLadders();
		return true;
	}
	else if ( cUndo.ProcessEvent( eEvent ) )
	{
		NMapEditor::DoUndo();
		return true;
	}
	else if ( cRedo.ProcessEvent( eEvent ) )
	{
		NMapEditor::DoRedo();
		return true;
	}
	else if ( cUpdateWallSpot.ProcessEvent( eEvent ) )
	{
		int nWallSpotID = GetUserSettings().GetSelectedBrushID( LID_WALLS );
		pBuildingInfo.Refresh();
		UpdateWallSpot( nWallSpotID, pBuildingInfo->GetValue(), pBGrid );
		UpdateBuildInfo( pWorld->GetMainBuildingInterface() );
		pWorld->UpdateWallSpot( nWallSpotID );
		pWorld->GetAIMap()->Sync();
		pSelection->ProcessEvent( "update_spot" );
		return true;
	}
	else if ( cShowInfo.ProcessEvent( eEvent ) )
	{
		bShowInfo = !bShowInfo;
		pWorld->ShowInfo( bShowInfo );
		if ( !bShowInfo )
			gridlines.clear();
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::Step()
{
	if ( CanRender() )
	{
		pCamera->Update( GetTime() );
	
		CTransformStack ts;
		ts.MakeProjective( pScene->GetScreenRect(), N_FOV, NEAR_PLANE, FAR_PLANE );
		//ts.MakeProjective( pScene->GetScreenRect(), N_FOV );
		ts.SetCamera( pCamera->GetPos() );
		pRender->UpdateViewWorld( true, GetTime(), 0 );
		pDragAndDrop->Update();
		//
		if ( bWYSIWYGActive )
		{
			pInterface->Step( GetTime() );
			const IUserSettings &settings = GetUserSettings();
			int nUserID, nTerrUserID;
			CVec3 ptCross;
			CVec3 ptNormal;
			CVec3 ptTerrCross( PT_INVALID );
			CObjectBase *pTerrObj = GetObjectUnderCursor( &nTerrUserID, &ptTerrCross, &ptNormal, NWorld::TS_TERRAINS );
			CObjectBase *pObj = GetObjectUnderCursor( &nUserID, &ptCross, &ptNormal );

			pTerrain->Update( ptTerrCross, pTerrObj, nTerrUserID );
			pTerrain->OnMouseMove( CVec2( ptTerrCross.x, ptTerrCross.y ) );
			if ( !pSelection->Update( pCursor->GetPos(), pObj, nUserID ) )
			{
				switch ( settings.GetMode() )
				{
					case EM_SELECT:
		//			pSelection->Test( pCursor->GetPos() );
						break;
					case EM_MATERIAL:
						pMaterial->Update( pObj, nUserID, ptNormal );
						break;
				}
			}
			else
			{
				int i =0;
			}
		}
		CVec3	crClear = CVec3(0.55f,0.55f,0.55f);

		NGScene::IGameView::SDrawInfo drawInfo;
		drawInfo.pTS = &ts;
		drawInfo.vClearColor = crClear;
		pScene->Draw( drawInfo );
		//pScene->DrawFullScreen( &ts, crClear );
		pSelection->Draw();
		pInterface->Draw( GetTime() );
		if ( GetUserSettings().IsGridVisible() && bShowInfo )
		{
			if ( gridlines.empty() )
				DrawGrid( pScene, ptMapSize, FP_GRID_STEP, CVec3(1,1,1), &gridlines );
		}
		else
			gridlines.clear();
		NGScene::Flip();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void DrawBox( NGScene::IGameView *pScene, SDiscretePos dpos, const CVec3 &ptOrig, const CVec3 ptSizes, const CVec3 &color,
	list<CObj<NGScene::CPolyline> > *pL )
{
	vector<CVec3> points( 10, ptOrig );

	points[1].x += ptSizes.x;
	points[2]   += CVec3( ptSizes.x, ptSizes.y, 0 );
	points[3].y += ptSizes.y;
	points[5].z += ptSizes.z;
	points[6]   += CVec3( ptSizes.x, 0, ptSizes.z );
	points[7]   += ptSizes;
	points[8]   += CVec3( 0, ptSizes.y, ptSizes.z );
	points[9].z += ptSizes.z;
	dpos.MoveAndRotate( &points );
	pL->push_back( pScene->CreatePolyline( points, color ) );

	points.clear();
	points.resize( 2, ptOrig );
	points[0].x += ptSizes.x;
	points[1]   += CVec3( ptSizes.x, 0, ptSizes.z );
	dpos.MoveAndRotate( &points );
	pL->push_back( pScene->CreatePolyline( points, color ) );

	points[0] = ptOrig + CVec3( ptSizes.x, ptSizes.y, 0 );
	points[1] = ptOrig + CVec3( ptSizes.x, ptSizes.y, ptSizes.z );
	dpos.MoveAndRotate( &points );
	pL->push_back( pScene->CreatePolyline( points, color ) );

	points[0] = ptOrig + CVec3( 0, ptSizes.y, 0 );
	points[1] = ptOrig + CVec3( 0, ptSizes.y, ptSizes.z );
	dpos.MoveAndRotate( &points );
	pL->push_back( pScene->CreatePolyline( points, color ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWysiwyg::Select( CObjectBase *pObj, int nUserID, const CVec3 &ptCrossFaceNormal )
{
	if ( !pObj )
	{
		pSelection->Clear();
		return false;
	}
	// пытаемся выделить уже выделенный объект?
	bool bCheck = pSelection->CheckSelection( pObj, nUserID );
	if ( IsPressed( VK_CONTROL ) && !IsRotation() )
	{
		// нажат CTRL, добавляем его. Если объект уже выделен - снимается выделение
		pSelection->AddSelection( pObj, nUserID );
	}
	else if ( !bCheck )
		pSelection->SetSelection( pObj, nUserID );

	return bCheck;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWysiwyg::GetPointUnderCursor( CVec2 *pRes )
{
	CRay r;
	MakeProjectiveRay( &r.ptDir, &r.ptOrigin, pCamera->GetPos(), pScene->GetScreenRect(), pCursor->GetPos(), N_FOV );
	if ( fabs( r.ptDir.z ) < FP_EPSILON )
		return false;
	float t = -(r.ptOrigin.z - (GetUserSettings().GetActiveFloor() * NBuilding::WALL_HEIGHT)) / r.ptDir.z;
	//float t = -r.ptOrigin.z / r.ptDir.z;
	*pRes = CVec2( r.ptOrigin.x + t * r.ptDir.x, r.ptOrigin.y + t * r.ptDir.y );

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectBase* CWysiwyg::GetObjectUnderCursor( int *pnUserID, CVec3 *pptCross, CVec3 *pptCrossFaceNormal, int nFlags )
{
	ASSERT( pnUserID && pptCross && pptCrossFaceNormal );
	CRay r, rcenter;
	vector<NAI::SInterval> intervals;
	CVec2 ptCursor = pCursor->GetPos();
	if ( ptCursor.x <= 0 || ptCursor.y <= 0 )
		return 0;
	MakeProjectiveRay( &r.ptDir, &r.ptOrigin, pCamera->GetPos(), pScene->GetScreenRect(), ptCursor, N_FOV );
	MakeProjectiveRay( &rcenter.ptDir, &rcenter.ptOrigin, pCamera->GetPos(), CVec2(1000, 1000), CVec2(500, 500), N_FOV );
	pAIMap->Trace( r, &intervals, nFlags );
	if ( intervals.empty() )
		return 0;
	SortTracedObjects( &intervals );
	for ( int i = 0; i < intervals.size(); ++i )
	{
		float d = intervals[i].enter.fT * (r.ptDir * rcenter.ptDir);
		if ( d > NEAR_PLANE )
		{
			*pnUserID = intervals[i].nUserID;
			*pptCross = r.ptOrigin + intervals[i].enter.fT * r.ptDir;
			*pptCrossFaceNormal = intervals[i].enter.ptNormal;
			if ( intervals[i].pSrc->pUserData == 0 )
				*pnUserID = BT_TERRAIN;
			return (CObjectBase*)( intervals[i].pSrc->pUserData );
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::NewTexSpot( const CVec3 &ptWorldCross, const CVec3 &ptNormal )
{
	CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pLoader = NGScene::shareBuildings.Get( nBuilding );
	pLoader.Refresh();
	NBuilding::CBuildInfo *pBInfo = pLoader->GetValue();
	if ( !pBInfo )
		return;
	//
	SFBTransform posTerr = pWorld->GetTerrainTransform( ptWorldCross.x, ptWorldCross.y );
	InvertMatrix( &posTerr.backward, posTerr.forward );
	CVec3 ptCross;
	posTerr.backward.RotateHVector( &ptCross, ptWorldCross );
	//
	const CVec2 ptSize( 1, 1 );
	SHMatrix m;
	MakeMatrix( &m, VNULL3, ptNormal );
	CVec3 ptd( -ptSize.y / 2, 0, -ptSize.x / 2 );
	m.RotateHVector( &ptd, ptd );
	
	NBuilding::SProjectedSpot spot;
	spot.ptOrigin = ptCross - ptd;
	spot.ptNormal = ptNormal;
	Normalize( &spot.ptNormal );
	spot.ptSize = ptSize;
	spot.nRotation = 0;
	spot.nMaterialID = GetUserSettings().GetSpotMaterialID();
	int nSpotID = pBInfo->CreateNextSpotID();
	spot.nID = nSpotID;
	pBInfo->spots.push_back( spot );
	pSelection->AssignBuildingSpot( nSpotID );
	pLoader->Updated();
	pWorld->UpdateAll();
	SerializeBuilding( pBInfo, nBuilding );
	pSelection->WallSpotUpdated( nSpotID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::NewTerrSpot()
{
	if ( !IsValid( pVariant ) )
		return;
	const IUserSettings &settings = GetUserSettings();
	int nMaterial = settings.GetSpotMaterialID();
	if ( nMaterial <= 0 )
		return;
	CPtr<NDb::CTMaterial> pTMat = NDb::GetTMaterial( nMaterial );
	SRand rand;
	NDb::CMaterial *pMat = pTMat->GetMaterial( &rand );
	if ( !IsValid( pMat ) || !IsValid( pMat->pTexture ) )
		return;
	CRay r;
	MakeProjectiveRay( &r.ptDir, &r.ptOrigin, pCamera->GetPos(), pScene->GetScreenRect(), pCursor->GetPos(), N_FOV );
	if ( fabs( r.ptDir.z ) < FP_EPSILON )
		return;
	float t = -r.ptOrigin.z / r.ptDir.z;
	NBuilding::SProjectedSpot spot;
	spot.nMaterialID = nMaterial;
	spot.ptOrigin = NearestTile( CVec3( r.ptOrigin.x + t * r.ptDir.x, r.ptOrigin.y + t * r.ptDir.y, 0 ) );
	spot.ptNormal = CVec3( 0, 0, 1 );
	spot.ptSize = CVec2( pMat->pTexture->nWidth, pMat->pTexture->nHeight );
	spot.ptSize *= FP_GRID_STEP * 4.0f / 256.0f;
	spot.nRotation = 0;
	int nID = AddTerrSpotDB( pVariant->GetRecordID(), spot );
	if ( nID < 0 )
		return;
	Sleep( 30 );
	NDatabase::Refresh<NDb::CRndTerrainSpot>();
	NMapEditor::PushUndoCmd( CreateTerrSpotUndo( CWysiwygUndo::UA_INSERT, NDb::GetRndTerrainSpot( nID ), 0 ) );
	//CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pUpdate = NGScene::shareBuildings.Get( nBuilding );
	//pUpdate->Updated();
	vector<NBuilding::SProjectedSpot> spots( 1, spot );
	pWorld->UpdateTerrainSpots( spots );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::NewWaypoint()
{
	int nID = GetUserSettings().GetSelectedBrushID( LID_WAYPOINTS );
	CVec2 ptPos;
	GetPointUnderCursor( &ptPos );
	SetWaypointPos( nID, ptPos, GetUserSettings().GetActiveFloor() );
	Sleep(100);
	NDatabase::Refresh<NDb::CWaypoint>();
	NDatabase::Refresh<NDb::CWaypointName>();
	pBuildingInfo->Updated();
	pWorld->UpdateAll();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::NewObject()
{
	int nID = GetUserSettings().GetSelectedBrushID( LID_OBJECTS );
	if ( nID <= 0 )
		return;
	Sleep( 50 );
	NDatabase::Refresh<NDb::CFinalElement>();
	SForceSelection sel;
	sel.nWorldID = nBuilding;
	sel.objectIDs.push_back( nID );
	pSelection->OnPaste( sel, pCursor->GetPos() );
	pWorld->UpdateObjects();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::SaveHitTestObject()
{
	pObjRequest = GetObjectUnderCursor( &nUserIDRequest, &ptCrossRequest, &ptNormalRequest );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CWysiwyg::GetSelectionMask() const
{
	return pSelection->GetSelectionMask();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::NewLadder()
{
	CVec2 ptTemp;
	if ( !GetPointUnderCursor( &ptTemp ) )
		return;
	CVec3 ptWorldCross( ptTemp, 0 );

	CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pLoader = NGScene::shareBuildings.Get( nBuilding );
	pLoader.Refresh();
	NBuilding::CBuildInfo *pBInfo = pLoader->GetValue();
	if ( !pBInfo )
		return;
	//
	SFBTransform posTerr = pWorld->GetTerrainTransform( ptWorldCross.x, ptWorldCross.y );
	InvertMatrix( &posTerr.backward, posTerr.forward );
	CVec3 ptCross;
	posTerr.backward.RotateHVector( &ptCross, ptWorldCross );
	//
	NBuilding::SLadder &lad = *pBInfo->ladders.insert( pBInfo->ladders.end(), NBuilding::SLadder() );
	int nLadderID = pBInfo->CreateNextLadderID();
	lad.nID = nLadderID;
	lad.nHeight = 4;
	lad.pos.ptMove.x = ptCross.x;
	lad.pos.ptMove.y = ptCross.y;
	lad.pos.ptMove.z = GetUserSettings().GetActiveFloor();
	lad.pos.nRotation = GetUserSettings().GetActiveRotationID();
	pLoader->Updated();
	SerializeBuilding( pBInfo, nBuilding );
	pWorld->UpdateLadders();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetPolygon( vector<CVec2> *pPoly, NDb::CRectangle *pRect )
{
	SFBTransform tr = MakeTransform( CVec3( pRect->ptCenter.x, pRect->ptCenter.y, 0 ), pRect->fRotation );
	CVec3 ptSize( pRect->fWidth, pRect->fHeight, 0 );
	vector<CVec3> points;

	points.push_back( VNULL3 );
	points.push_back( CVec3( ptSize.x, 0, 0 ) );
	points.push_back( ptSize );
	points.push_back( CVec3( 0, ptSize.y, 0 ) );
	pPoly->clear();
	for ( int i = 0; i < points.size(); ++i )
	{
		CVec3 pt;
		tr.forward.RotateHVector( &pt, points[i] );
		pPoly->push_back( CVec2( pt.x, pt.y ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool CompareSubtemplates( int nUserIDa, int nUserIDb )
{
	if ( nUserIDa == nUserIDb )
		return false;
	EBrushType type;
	int nIDa, nIDb;

	GetFragmentID( nUserIDa, &type, &nIDa );
	GetFragmentID( nUserIDb, &type, &nIDb );

	NDb::CRectangle *pA = NDb::GetRectangle( nIDa );
	NDb::CRectangle *pB = NDb::GetRectangle( nIDb );
	if ( !IsValid( pA ) || !IsValid( pB ) )
		return false;
	vector<CVec2> polyA, polyB;
	GetPolygon( &polyA, pA );
	GetPolygon( &polyB, pB );
	int nInA = 0, nInB = 0;
	for ( int i = 0; i < 4; ++i )
	{
		if ( IsPointInPolygon( polyA, polyB[i] ) )
			++nInA;
		if ( IsPointInPolygon( polyB, polyA[i] ) )
			++nInB;
	}
	if ( nInA < 2 && nInB < 2 )
		return false;
	return nInA < nInB;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline EBrushType GetObjectType( const NAI::SInterval &o )
{
	EBrushType type;
	int tmp;

	GetFragmentID( o.nUserID, &type, &tmp );

	if ( o.pSrc->pUserData == 0 ) type = BT_TERRAIN;
	else if ( dynamic_cast<NWorld::IEditorObject*>( o.pSrc->pUserData.GetPtr() ) ) type = BT_OBJECT;
	else if ( dynamic_cast<NWorld::IEditorUnit*>( o.pSrc->pUserData.GetPtr() ) ) type = BT_UNIT;
	else if ( dynamic_cast<NWorld::IBuilding*>( o.pSrc->pUserData.GetPtr() ) ) type = BT_GEOMETRY;

	return type;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SortTraced( const NAI::SInterval &a, const NAI::SInterval &b )
{
	EBrushType typeA = GetObjectType( a );
	EBrushType typeB = GetObjectType( b );
	ELayer eActiveType;
	int nLayer;
	NBuilding::GetLayerID( GetUserSettings().GetActiveLayerID(), &eActiveType, &nLayer );

	if ( typeA == BT_SUBTEMPLATE && typeB == BT_SUBTEMPLATE )
		return CompareSubtemplates( a.nUserID, b.nUserID );
	else if ( (typeA | typeB) & BT_SUBTEMPLATE )
	{
		if ( eActiveType == LID_SUBTEMPLATES )
			return typeA == BT_SUBTEMPLATE;
		else
			return typeA != BT_SUBTEMPLATE && typeA != BT_TERRAIN;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::SortTracedObjects( vector<NAI::SInterval> *pIntervals )
{
	sort( pIntervals->begin(), pIntervals->end(), SortTraced );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwyg::Paste( bool bNewTemplatePaste )
{
	SForceSelection sel;

	NMapEditor::BeginUndoList();
	sel.nWorldID = nBuilding;
	PasteClipboard( &sel, nBuilding );
	pBuildingInfo.Refresh();
	NBuilding::CBuildInfo *pInfo = pBuildingInfo->GetValue();

	pBuildingInfo->Updated();
	CVec2 pt = bNewTemplatePaste ? CVec2(-1,-1) : pCursor->GetPos();
	pSelection->OnPaste( sel, pt );
	SerializeBuilding( pInfo, nBuilding );
	pWorld->UpdateAll();
	NMapEditor::EndUndoList();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICWysiwyg
////////////////////////////////////////////////////////////////////////////////////////////////////
CICWysiwyg::CICWysiwyg( HWND _hWnd, int _nPlacementID, const ICamera::SCameraPos &_cameraPos, bool _bPasteClipboard )
	: hWnd( _hWnd ), nPlacementID(_nPlacementID), cameraPos(_cameraPos), bPasteClipboard(_bPasteClipboard)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICWysiwyg::Exec()
{
  CIMapEditor *pOldI = dynamic_cast<CIMapEditor*>( GetInterface() );
  if ( pOldI )
  {
    pOldI->GetCameraPos( &cameraPos );
  }
	ResetStack();
	NWysiwyg::CWysiwyg *pView = new NWysiwyg::CWysiwyg( hWnd, nPlacementID );
	pView->SetCamera( cameraPos );
	SetInterface( pView );
	if ( bPasteClipboard )
		NInput::PostEvent( "template_paste" );
}
