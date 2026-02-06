#include "StdAfx.h"
#include "wInterface.h"
#include "MEDragDrop.h"
#include "iWysiwyg.h"
#include "BuildingInfo.h"
#include "gView.h"
#include "MEUserSettings.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataObject.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataTerrain.h"
#include "..\DBFormat\DataGeometry.h"
#include "Transform.h"
#include "iMain.h"
#include "WysiwygSelection.h"
#include "..\MapEdit\FinDBCmd.h"
#include "..\MapEdit\dbDefs.h"
#include "WysiwygClipboard.h"
#include "Grid.h"
#include "MemObject.h"
#include "..\MapEdit\RectsDBCmd.h"
#include "..\MapEdit\UnitDB.h"
#include "..\MapEdit\WaypointDB.h"
#include "DiscretePos.h"
#include "WysiwygUndo.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
const int CLUE_SLOT_ID = 188;
const int EXPLOSION_ID = 189;
extern bool GetDropInfo( COleDataObject* pDataObject, int *pnTree, int *pnItem );
namespace NWysiwyg
{
	int AddTerrSpotDB( int nVarID, const NBuilding::SProjectedSpot &spot );
}
namespace NGfx
{
	HWND GetHWND();
}
namespace NGScene
{
	extern CResourceTracker objChecker;
}
static SRand rnd;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDragDrop: public IDragDrop
{
	OBJECT_NOCOPY_METHODS(CDragDrop)
	ZDATA
	CPtr<NGScene::IGameView> pScene;
	CPtr<ICamera> pCamera;
	CPtr<NWysiwyg::ISelection> pSelection;
	int nWorldID;
	ZEND

	class COleDropTargetInternal: public COleDropTarget
	{
		CPtr<CDragDrop> pParent;
	public:
		COleDropTargetInternal( CDragDrop *_pParent = 0 ): pParent(_pParent) { if ( pParent ) Register( CWnd::FromHandle( NGfx::GetHWND() ) ); }
		~COleDropTargetInternal() { Revoke(); }

		virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,	DWORD dwKeyState, CPoint point);
		virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
		virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
		virtual void OnDragLeave(CWnd* pWnd);
	} dropTarget;
	friend class COleDropTargetInternal;

	bool bUpdated;
	int  nType;
	int nObjectID;
	CVec2 ptPoint;
	CObj<NDb::CObject> pObject;
	CObj<NDb::CModel> pModel, pModel2;
	CObj<NDb::CConstructionPart> pCPart;
	CObj<CMemObject> pSphere;
	list<CObj<NGScene::CRenderNode> > objects;

	CVec3 GetPos( const CVec2 &ptPoint );
	DROPEFFECT OnDragMove( COleDataObject* pDataObject, CPoint point );
	void InsertObject( int nPlacableID );
	void InsertSubTemplate( NDb::CTemplate *pTemplate );
	void InsertPers( NDb::CRPGPers *pPers );
	void InsertSpot( NDb::CSpot *pSpot );
	void InsertConstructionPart( int nCPartID );
	void InsertWaypoint( int nWaypointNameID );

public:
	CDragDrop(){}
	CDragDrop( int nWorldID, NGScene::IGameView *pScene, ICamera *pCamera, NWysiwyg::ISelection *pSelection );

	virtual void Update();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CDragDrop::CDragDrop( int _nWorldID, NGScene::IGameView *_pScene, ICamera *_pCamera, NWysiwyg::ISelection *_pSelection )
	: pScene(_pScene), pCamera(_pCamera), nWorldID(_nWorldID), pSelection(_pSelection), dropTarget(this)
{
	bUpdated = false;
	pSphere = new ::CMemObject;
	pSphere->CreateSphere( VNULL3, 0.5f * FP_GRID_STEP, 4 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 CDragDrop::GetPos( const CVec2 &ptPoint )
{
	float fFloor = GetUserSettings().GetActiveFloor();
	CVec2 ptTile = pSelection->GetTileUnderPos( pSelection->GetProjectiveRay( ptPoint ), fFloor );
	return CVec3( ptTile, fFloor * NBuilding::WALL_HEIGHT );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static inline CVec2 GetSpotSizeTiles( NDb::CSpot *pSpot )
{
	CVec2 ptSize( 2, 2 );
	CPtr<NDb::CMaterial> pM;
	if ( IsValid( pSpot->pMaterial ) )
	{
		pM = pSpot->pMaterial->GetMaterial( &rnd );
		if ( IsValid( pM ) && IsValid( pM->pTexture ) )
		{
			ptSize = CVec2( pM->pTexture->nWidth, pM->pTexture->nHeight );
			ptSize *= 4.0f / 256.0f;
		}
	}
	return ptSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CModel* CreateModel( NDb::CGeometry *pGeometry, const CPtr<NDb::CMaterial> pMaterials[NDb::N_MODEL_MATERIALS] )
{
	static NDb::CMaterial *pDefM = NDb::GetMaterial( NDb::N_DEF_MATERIAL_ID );
	NDb::CModel *pModel = new NDb::CModel();
	const int nID = pGeometry->GetRecordID();

	pModel->pGeometry = pGeometry;
	for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
	{
		NGScene::SPartKey k( nID, i );
		if ( !NGScene::objChecker.DoesExist( k ) )
			continue;
		pModel->pMaterials[i] = IsValid( pMaterials[i] ) ? pMaterials[i] : pDefM;
	}

	return pModel;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDragDrop::Update()
{
	if ( !bUpdated )
		return;
	bUpdated = false;
	objects.clear();
	//
	CVec3 ptPos = GetPos(ptPoint );
	SFBTransform pos = pSelection->GetTerrainTransform( ptPos.x, ptPos.y ) * MakeTransform( ptPos );
	switch ( nType )
	{
		case IDC_OBJECTS_TREE:
		{
			NDb::CTRndObject *pTObj = NDb::GetTRndObject( nObjectID );
			if ( IsValid( pTObj ) )
				pObject = pTObj->CreateObject( &rnd );
			if ( IsValid( pObject ) )
			{
				if ( IsValid( pObject->pModels[0] ) && IsValid( pObject->pModels[0]->pModel ) )
					objects.push_back( pScene->CreateMesh( pObject->pModels[0]->pModel, pos ) );
				else
					objects.push_back( pScene->CreateMesh( pSphere, CVec4( 0.7f, 0.7f, 0.9f, 1.0f ), pos ) );
			}
			break;
		}
		case IDC_RPG_ITEMS_TREE:
		{
			NDb::CRPGItem *pRPG = NDb::GetRPGItem( nObjectID );
			if ( IsValid( pRPG ) )
			{
				if ( IsValid( pRPG->pModel ) )
					pModel = pRPG->pModel->CreateModel( &rnd );
				if ( IsValid( pModel ) )
					objects.push_back( pScene->CreateMesh( pModel, pos ) );
				else
				{
					CVec4 cr = pRPG->GetRecordID() == CLUE_SLOT_ID ? CVec4( 1.0f, 0.5f, 0.6f, 1.0f ) : CVec4( 0.7f, 0.7f, 0.9f, 1.0f );
					if ( pRPG->GetRecordID() == EXPLOSION_ID )
						cr = CVec4( 0.77f, 0.55f, 0.1f, 1.0f );
					objects.push_back( pScene->CreateMesh( pSphere, cr, pos ) );
				}
			}
			break;
		}
		case IDC_TEMPLATE_TREE:
		{
			NDb::CTemplate *pT = NDb::GetTemplate( nObjectID );
			if ( !IsValid( pT ) )
				break;
			CObj<CMemObject> pBox = new ::CMemObject;
			CVec3 ptBox = CVec3( FP_GRID_STEP * pT->nWidth, FP_GRID_STEP * pT->nHeight, 1.1f * NBuilding::WALL_HEIGHT );
			pBox->CreateCube( VNULL3, ptBox, true );
			objects.push_back( pScene->CreateMesh( pBox, CVec4( 0.68f, 0.87f, 1.0f, 0.2f ), pos ) );
			break;
		}
		case IDC_RPG_PERS_TREE:
		{
			NDb::CRPGPers *pPers = NDb::GetPers( nObjectID );
			if ( IsValid( pPers ) )
				pModel = pPers->pModel->CreateModel( &rnd );
			if ( IsValid( pModel ) )
				objects.push_back( pScene->CreateMesh( pModel, pos * MakeTransform( VNULL3, 90 ) ) );
			break;
		}
		case IDC_SPOTS_TREE:
		{
			NDb::CSpot *pSpot = NDb::GetSpot( nObjectID );
			if ( !IsValid( pSpot ) )
				break;
			CVec2 ptSize = GetSpotSizeTiles( pSpot );
			pModel = new NDb::CModel();
			pModel->pGeometry = NDb::GetGeometry( NDb::N_CUBE_GEOMETRY_ID );
			if (  !IsValid( pModel->pGeometry ) )
				return;
			CPtr<NDb::CMaterial> pM = IsValid( pSpot->pMaterial ) ? pSpot->pMaterial->GetMaterial( &rnd ) : 0;
			if ( IsValid( pM ) )
				for ( int i = 0; i < NDb::N_MODEL_MATERIALS; ++i )
					pModel->pMaterials[i]  = pM;
			objects.push_back( pScene->CreateMesh( pModel, pos * MakeTransform( VNULL3, CVec3( ptSize * FP_GRID_STEP, 1 ) ) ) );
			break;
		}
		case IDC_CONSTRUCTIONPARTS_TREE:
		{
			pos = pos * MakeTransform( VNULL3, RotationIDToAngle( GetUserSettings().GetActiveRotationID() ) );
			NDb::CTConstructionPart *pTCPart = NDb::GetTConstructionPart( nObjectID );
			if ( IsValid( pTCPart ) )
				pCPart = pTCPart->CreateConstructionPart( &rnd );
			if ( !IsValid( pCPart ) )
				break;
			if ( IsValid( pCPart->pGeometry ) )
			{
				pModel = CreateModel( pCPart->pGeometry, pCPart->pDefMaterials );
				objects.push_back( pScene->CreateMesh( pModel, pos ) );
			}
			else
			{
				CObj<CMemObject> pBox = new ::CMemObject;
				float fY = pCPart->nSizeY == 0 ? pCPart->fThickness : 2 * pCPart->nSizeY * FP_GRID_STEP;
				CVec3 ptBox = CVec3( 2 * FP_GRID_STEP * pCPart->nSizeX, fY, pCPart->nSizeZ * NBuilding::WALL_HEIGHT );
				pBox->CreateCube( VNULL3, ptBox, true );
				objects.push_back( pScene->CreateMesh( pBox, CVec4( 0.68f, 0.87f, 1.0f, 0.2f ), pos ) );
			}
			if ( IsValid( pCPart->p2ndGeometry ) )
			{
				pModel2 = CreateModel( pCPart->pGeometry, pCPart->pDefMaterials + NDb::N_MODEL_MATERIALS );
				objects.push_back( pScene->CreateMesh( pModel2, pos ) );
			}
			if ( IsValid( pCPart->pObject ) && IsValid( pCPart->pObject->pModels[0] ) && IsValid( pCPart->pObject->pModels[0]->pModel ) )
					objects.push_back( pScene->CreateMesh( pCPart->pObject->pModels[0]->pModel, pos ) );
			break;
		}
		case  IDC_WAYPOINTNAMES_TREE:
		{
			CObj<CMemObject> pBox = new ::CMemObject;
			pBox->CreateFlag( VNULL3, 1.5f, 0.04f, 0.6f );
			objects.push_back( pScene->CreateMesh( pBox, CVec4( 0.9f, 0.2f, 0.2f, 1.0f ), pos ) );
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
DROPEFFECT CDragDrop::OnDragMove( COleDataObject* pDataObject, CPoint point )
{
	int nTree;
	int nID;

	nType = -1;
	bUpdated = true;
	if ( !pDataObject || ! GetDropInfo( pDataObject, &nTree, &nID ) 
		|| (nTree != IDC_TEMPLATE_TREE && nTree != IDC_OBJECTS_TREE && nTree != IDC_RPG_ITEMS_TREE && nTree != IDC_RPG_PERS_TREE 
		&&  nTree != IDC_SPOTS_TREE && nTree != IDC_CONSTRUCTIONPARTS_TREE && nTree != IDC_WAYPOINTNAMES_TREE ) )
	{
		NMainLoop::StepApp( true, true );
		return DROPEFFECT_NONE;
	}
	nType = nTree;
	nObjectID = nID;
	ptPoint = CVec2( point.x, point.y );
	NMainLoop::StepApp( true, true );
	return DROPEFFECT_MOVE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
DROPEFFECT CDragDrop::COleDropTargetInternal::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,	DWORD dwKeyState, CPoint point)
{
	return pParent->OnDragMove( pDataObject, point );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
DROPEFFECT CDragDrop::COleDropTargetInternal::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return pParent->OnDragMove( pDataObject, point );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDragDrop::COleDropTargetInternal::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	int type;
	int nID;

	if ( !GetDropInfo( pDataObject, &type, &nID ) )
		return false;
	switch ( type )
	{
		case IDC_OBJECTS_TREE:
		{
			NDb::CTRndObject *pO = NDb::GetTRndObject( pParent->nObjectID );
			if ( IsValid( pO ) && IsValid( pO->pPlacable ) )
				pParent->InsertObject( pO->pPlacable->GetRecordID() );
			break;
		}
		case IDC_RPG_ITEMS_TREE:
		{
			NDb::CRPGItem *pI = NDb::GetRPGItem( pParent->nObjectID );
			if ( IsValid( pI ) && IsValid( pI->pPlaceObj ) )
				pParent->InsertObject( pI->pPlaceObj->GetRecordID() );
			break;
		}
		case IDC_TEMPLATE_TREE:
		{
			NDb::CTemplate *pTemplate = NDb::GetTemplate( pParent->nObjectID );
			if ( IsValid( pTemplate ) )
				pParent->InsertSubTemplate( pTemplate );
			break;
		}
		case IDC_RPG_PERS_TREE:
		{
			NDb::CRPGPers *pPers = NDb::GetPers( pParent->nObjectID );
			if ( IsValid( pPers ) )
				pParent->InsertPers( pPers );
			break;
		}
		case IDC_SPOTS_TREE:
		{
			NDb::CSpot *pSpot = NDb::GetSpot( pParent->nObjectID );
			if ( IsValid( pSpot ) )
				pParent->InsertSpot( pSpot );
			break;
		}
		case IDC_CONSTRUCTIONPARTS_TREE:
		{
			pParent->InsertConstructionPart( pParent->nObjectID );
			break;
		}
		case IDC_WAYPOINTNAMES_TREE:
		{
			pParent->InsertWaypoint( pParent->nObjectID );
			break;
		}
	}
	pParent->OnDragMove( 0, point );
	NMainLoop::StepApp( true, true );
	::SetFocus( NGfx::GetHWND() );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDragDrop::COleDropTargetInternal::OnDragLeave(CWnd* pWnd)
{
	pParent->OnDragMove( 0, CPoint() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDragDrop::InsertObject( int nPlacableID )
{
	static CFinPosDB db;

	int nID = db.Insert( nWorldID, nPlacableID );
	if ( nID > 0 )
	{
		CVec3 ptPos = FP_INV_GRID_STEP * GetPos( ptPoint );
		db.SetPos( nID, CVec2( ptPos.x, ptPos.y ), 0, GetUserSettings().GetActiveFloor(), 0 );
		db.Close();
		Refresh<NDb::CFinalElement>( nID );
		NDb::CFinalElement *p = NDb::GetFinalElement( nID );
		if ( IsValid( p ) )
			NMapEditor::PushUndoCmd( CreateObjSelectionUndo( CWysiwygUndo::UA_INSERT, p, 0 ) );
		else
		{
			ASSERT(0);
		}
		SForceSelection sel;
		sel.nWorldID = nWorldID;
		sel.objectIDs.push_back( nID );
		pSelection->OnPaste( sel, ptPoint, false );
		pSelection->ObjectUpdated( nID );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDragDrop::InsertSubTemplate( NDb::CTemplate *pTemplate )
{
	static CRectPosDB db;

	if ( !IsValid( pTemplate ) )
		return;
	int nID = db.Insert( nWorldID, pTemplate->GetRecordID() );
	if ( nID <= 0 )
		return;
	CVec3 ptPos = GetPos( ptPoint );
	db.SetPos( nID, CVec2( ptPos.x, ptPos.y ), 0, GetUserSettings().GetActiveFloor(), 0 );
	//
	Refresh<NDb::CRectangle>( nID );
	NMapEditor::PushUndoCmd( CreateSubTemplateUndo( CWysiwygUndo::UA_INSERT, NDb::GetRectangle( nID ), 0 ) );
	SForceSelection sel;
	sel.nWorldID = nWorldID;
	sel.subtemplateIDs.push_back( nID );
	pSelection->OnPaste( sel, ptPoint, false );
	pSelection->SubTemplateUpdated( NWysiwyg::MakeUserID( BT_SUBTEMPLATE, nID ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDragDrop::InsertPers( NDb::CRPGPers *pPers )
{
	static CUnitPosDB db;

	int nID = db.Insert( nWorldID, pPers->GetRecordID() );
	if ( nID <= 0 )
		return;
	CVec3 ptPos = FP_INV_GRID_STEP * GetPos( ptPoint );
	db.SetPos( nID, ptPos.x, ptPos.y, GetUserSettings().GetActiveFloor(), 0 );
	//
	Refresh<NDb::CUnit>( nID );
	NDb::CUnit *p = NDb::GetUnit( nID );
	if ( IsValid( p ) )
		NMapEditor::PushUndoCmd( CreateUnitUndo( CWysiwygUndo::UA_INSERT, p, 0 ) );
	else
	{
		ASSERT(0);
	}
	SForceSelection sel;
	sel.nWorldID = nWorldID;
	sel.unitIDs.push_back( nID );
	pSelection->OnPaste( sel, ptPoint, false );
	pSelection->UnitUpdated( nID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDragDrop::InsertSpot( NDb::CSpot *pSpot )
{
	NBuilding::SProjectedSpot s;
	s.ptOrigin = GetPos( ptPoint );
	s.ptNormal = CVec3( 0, 0, 1 );
	s.ptSize = FP_GRID_STEP * GetSpotSizeTiles( pSpot );
	s.nRotation = 0;
	s.nMaterialID = pSpot->GetRecordID();
	int nID = NWysiwyg::AddTerrSpotDB( nWorldID, s );
	if ( nID < 0 )
		return;
	//
	Refresh<NDb::CRndTerrainSpot>( nID );
	NMapEditor::PushUndoCmd( CreateTerrSpotUndo( CWysiwygUndo::UA_INSERT, NDb::GetRndTerrainSpot( nID ), 0 ) );
	SForceSelection sel;
	sel.nWorldID = nWorldID;
	sel.terrSpotsIDs.push_back( nID );
	pSelection->OnPaste( sel, ptPoint, false );
	pSelection->TerrainSpotUpdated( vector<NBuilding::SProjectedSpot>( 1, s ) );	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDragDrop::InsertConstructionPart( int nCPartID )
{
	vector<int> ids;
	pSelection->AddConstructionPart( &ids, nCPartID, GetPos( ptPoint ) );
	SForceSelection sel;
	sel.nWorldID = nWorldID;
	sel.fragsUserIDs.insert( sel.fragsUserIDs.end(), ids.begin(), ids.end() );
	pSelection->OnPaste( sel, ptPoint, false );
	pSelection->BuildingUpdated();	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDragDrop::InsertWaypoint( int nWaypointNameID )
{
	int nID = AddWaypoint2DB( nWaypointNameID, nWorldID );
	if ( nID < 0 )
		return;
	CVec3 pt = GetPos( ptPoint );
	SetWaypointPos( nID, CVec2( pt.x, pt.y ), GetUserSettings().GetActiveFloor() );
	Refresh<NDb::CWaypoint>( nID );
	SForceSelection sel;
	sel.nWorldID = nWorldID;
	sel.waypointIDs.push_back( nID );
	pSelection->OnPaste( sel, ptPoint, false );
	pSelection->WaypointUpdated( nID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IDragDrop* CreateDragAndDrop( int nWorldID, NGScene::IGameView *pScene, ICamera *pCamera, NWysiwyg::ISelection *pSelection )
{
	return new CDragDrop( nWorldID, pScene, pCamera, pSelection );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
