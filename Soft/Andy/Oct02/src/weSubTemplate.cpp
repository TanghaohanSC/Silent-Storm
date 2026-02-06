#include "StdAfx.h"
#include "weInterface.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataFormat.h"
#include "BuildingInfo.h"
#include "Transform.h"
#include "iWysiwyg.h"
#include "MemObject.h"
#include "Grid.h"
#include "wBuilding.h"
#include "BuildingGrid.h"
#include "MEUserSettings.h"
#include "MEParams.h"
#include "MELayers.h"
#include "MapBuild.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void RotatePt( T *pVec, float fAngle )
{
	const float fAng = -ToRadian( fAngle );
	const float fc = cos( fAng ), fs = sin( fAng );

	float x = fc * pVec->x + fs * pVec->y;
	pVec->y = -fs * pVec->x + fc * pVec->y;
	pVec->x = x;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSubTemplate: public IEditorSubTemplate
{
	OBJECT_NOCOPY_METHODS(CSubTemplate);

	ZDATA
	CPtr<IEditorWorld> pWorld;
	CDBPtr<NDb::CRectangle> pRect;
	CDBPtr<NDb::CTemplVariant> pVariant;
	CTRect<float> rLastOccupied;
	SMapInfo mapInfo;
	int nUserID;
	CObj<CMemObject> pBox;
	CVec3 ptBox;
	struct SBuilding
	{
		CObj<CBuilding> pBuilding;
		SMapPosition pos;

		SBuilding( CBuilding *p, const SMapPosition &_pos ): pBuilding(p), pos(_pos) {}
	};
	vector<SBuilding> buildings;
	CSyncSrcBind<IVisObj> bindGlobal;
	CObj<NAI::IPathNetwork> pPathNetwork;
	int nTemplateID;
	int nCutFloor;
	bool bTerrAlignment;
	CVec3 ptCreatePos;
	float fCreateRotation;
	bool bShowInfo;
	ZEND

	bool IsValidRect() const;
	SFBTransform MakePosition() const;
	void AddItems( IRenderVisitor *p, const SFBTransform &pos );
	void AddBuildings();
	void AddBox( IRenderVisitor *p, const SFBTransform &pos );
	void CreateMap();
	CVec2 GetRectCenter() const;
	CVec3 GetCreatePos() const;
	void SetBuildingPos( SBuilding *p, const CVec3 &ptDelta, float fRotationDelta );

public:

	CSubTemplate() {}
	CSubTemplate( IEditorWorld *pWorld, NDb::CRectangle *pRect, int nCutFloor );

	virtual void Visit( IRenderVisitor *p );
	virtual void Visit( IAIVisitor *p );

	virtual int GetUserID() const { return nUserID; }
	virtual NDb::CRectangle* GetDBRect() const { IsValidRect(); return pRect; }

	virtual void Update( int nCutFloor, vector<CTRect<float> > *pUpdatedRects );
	virtual bool GetOccupiedRect( CTRect<float> *pRes ) const;
	virtual void ShowInfo( bool bShow );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CSubTemplate::CSubTemplate( IEditorWorld *_pWorld, NDb::CRectangle *_pRect, int _nCutFloor )
 : pRect( _pRect ), pWorld(_pWorld), nCutFloor(_nCutFloor), rLastOccupied( CTRect<float>(0,0,0,0) )
{
	bindGlobal.Link( pWorld->GetActive(), this );
	bShowInfo = true;

	pPathNetwork = NAI::CreateNodesNetwork( pWorld->GetAIMap() );
	//
	nUserID = NWysiwyg::MakeUserID( BT_SUBTEMPLATE, pRect->GetRecordID() );
	CreateMap();
	GetOccupiedRect( &rLastOccupied );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 CSubTemplate::GetCreatePos() const
{
	CVec2 ptRCenter = GetRectCenter();
	SFBTransform posz = pWorld->GetTerrainTransform( ptRCenter.x, ptRCenter.y );
	CVec3 pt(VNULL3);
	posz.forward.RotateHVector( &pt, pt );

	return CVec3( pRect->ptCenter, pt.z );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::CreateMap()
{
	static SRand rand;
	if ( IsValid( pRect->pTemplate ) )
	{
		pVariant = pRect->pTemplate->GetRnd( &rand );
		nTemplateID = pRect->pTemplate->GetRecordID();
	}
	//
	pBox = new ::CMemObject;
	float fDH = rand.GetFloat( 0, 0.1f * NBuilding::WALL_HEIGHT );
	ptBox = CVec3( FP_GRID_STEP * pRect->fWidth, FP_GRID_STEP * pRect->fHeight, NBuilding::WALL_HEIGHT + fDH );
	pBox->CreateCube( CVec3(0,0,0.05f), ptBox, true );
	//
	mapInfo = SMapInfo();
	bTerrAlignment = pWorld->GetTerrAlign();
	if ( IsValid( pVariant ) && IsValid( pRect->pVariant ) && IsValid( pRect->pVariant->pTemplate ) )
	{
		ptCreatePos = GetCreatePos();
		fCreateRotation = pRect->fRotation;
		mapInfo.terrain = pWorld->GetTerrainInfo();
		int nDepth = GetUserSettings().GetSubTemplateDepth();
		SMapPosition pos;
		pos.fRotation = pRect->fRotation;
		pos.nFloor = pRect->nFloor;
		pos.ptPos = ptCreatePos;
		pos.ptPos.z = pRect->nFloor * NBuilding::WALL_HEIGHT + pRect->fDZ;
		CVec3 ptParentBuilding( pRect->pVariant->pTemplate->nWidth, pRect->pVariant->pTemplate->nHeight, 0 );
		ptParentBuilding *= 0.5f * FP_GRID_STEP;
		BuildMapEditMap( pVariant->GetRecordID(), pPathNetwork, &mapInfo, nDepth, pos, ptParentBuilding, bTerrAlignment );
	}
	//
	buildings.clear();
	if ( IsLayerVisible( NBuilding::MakeFragmentID( LID_SUBTEMPLATES, 0 ) ) && pRect->nFloor <= nCutFloor )
		AddBuildings();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::Update( int _nCutFloor, vector<CTRect<float> > *pUpdatedRects )
{ 
	SFBTransform pos = MakePosition();
	SFBTransform transform = MakeTransform( GetCreatePos() - ptCreatePos, pRect->fRotation - fCreateRotation );
	bindGlobal.Update();
	nCutFloor = _nCutFloor;
	//
	CVec3 ptShift = GetCreatePos() - ptCreatePos;
	float fRotation = pRect->fRotation - fCreateRotation;
	//
	if ( IsValidRect() && IsLayerVisible( NBuilding::MakeFragmentID( LID_SUBTEMPLATES, 0 ) ) && pRect->nFloor <= nCutFloor )
	{
		bool bTerrA = pWorld->GetTerrAlign();
		if ( IsValid( pRect->pTemplate ) && (pRect->pTemplate->GetRecordID() != nTemplateID || bTerrAlignment != bTerrA ) )
		{
			CreateMap();
		}
		else
		{
			if ( buildings.empty() )
				AddBuildings();
			else
				for ( vector<SBuilding>::iterator i = buildings.begin(); i != buildings.end(); ++i )
				{
					SetBuildingPos( i, ptShift, fRotation );
					i->pBuilding->UpdateAllParts();
				}
		}
	}
	else
		buildings.clear();
	//
	if ( pUpdatedRects )
	{
		CTRect<float> r;
		if ( GetOccupiedRect( &r ) )
		{
			float fD = fabs( rLastOccupied.TopLeft() - r.TopLeft() ) + fabs( rLastOccupied.BottomRight() - r.BottomRight() );
			if ( fD > 0.05f )
			{
				CTRect<float> upd( Min( rLastOccupied.minx, r.minx ), Min( rLastOccupied.miny, r.miny ), Max( rLastOccupied.maxx, r.maxx ), Max( rLastOccupied.maxy, r.maxy ) );
				pUpdatedRects->push_back( upd );
				rLastOccupied = r;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSubTemplate::IsValidRect() const
{
	if ( !IsValid( pRect ) || !IsValid( pRect->pVariant ) )
		return false;
	// есть ли этот подтемплейт в текущей расстановке?
	const vector< CPtr<NDb::CRectangle> > &rects = pRect->pVariant->rects;
	for ( int i = 0; i < rects.size(); ++i )
		if ( IsValid( rects[i] ) && pRect->GetRecordID() == rects[i]->GetRecordID() )
			return true;
	//
	const_cast<CSubTemplate*>( this )->pRect = 0;
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SFBTransform CSubTemplate::MakePosition() const
{
	CVec3 pt( pRect->ptCenter.x, pRect->ptCenter.y, 0 );
	pt.z = pRect->nFloor * NBuilding::WALL_HEIGHT + pRect->fDZ;
	SFBTransform tr = MakeTransform( pt, pRect->fRotation );
	CVec3 ptC( pRect->fWidth, pRect->fHeight, 0 );
	tr.forward.RotateHVector( &ptC, 0.5f * FP_GRID_STEP * ptC );
	return pWorld->GetTerrainTransform( ptC.x, ptC.y ) * tr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::Visit( IRenderVisitor *p )
{
	if ( !IsValidRect() || !IsLayerVisible( NBuilding::MakeFragmentID( LID_SUBTEMPLATES, 0 ) ) || pRect->nFloor > nCutFloor )
		return;
	SFBTransform pos = MakePosition();
	AddItems( p, pos );
	if ( bShowInfo )
		AddBox( p, pos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::Visit( IAIVisitor *p )
{
	if ( !IsValidRect() || !IsLayerVisible( NBuilding::MakeFragmentID( LID_SUBTEMPLATES, 0 ) ) || pRect->nFloor > nCutFloor )
		return;
	SFBTransform pos = MakePosition();
	if ( bShowInfo )
		p->AddHull( pBox, pos, 0, 0, TS_PICK, nUserID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::AddItems( IRenderVisitor *p, const SFBTransform &posParent )
{
	CVec3 ptShift = GetCreatePos() - ptCreatePos;
	float fRotation = pRect->fRotation - fCreateRotation;
	for ( list<SMapElement>::const_iterator i = mapInfo.items.begin(); i != mapInfo.items.end(); ++i )
	{
		if ( IsValid( i->pObject->pModels[0] ) && IsValid( i->pObject->pModels[0]->pModel ) )
		{
			SFBTransform pos;
			CVec3 ptPos = i->pos.ptPos;
			CVec3 vLocalOld = ptPos - ptCreatePos;
			CVec3 vLocalNew = vLocalOld;
			RotatePt( &vLocalNew, fRotation );
			ptPos += ptShift + vLocalNew - vLocalOld;
			MakeMatrix( &pos, i->pos.ptScale, ptPos, ToRadian( i->pos.fRotation + fRotation ) );
			p->AddMesh( i->pObject->pModels[0]->pModel, pos, 0, i->pos.nFloor );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::AddBuildings()
{
	CVec3 ptShift = GetCreatePos() - ptCreatePos;
	float fRotation = pRect->fRotation - fCreateRotation;
	//
	for ( int i = 0; i < mapInfo.buildings.size(); ++i )
	{
		SMapBuilding &b = mapInfo.buildings[i];
		if ( b.pGrid->NeedComputeStability() )
		{
			b.pGrid->ToggleStability();
			b.pGrid->Reset();
		}
		// ставим здание в нужное место
		b.pos = MakeTransform( VNULL3 );
		CBuilding *pB = new CBuilding( pWorld->GetActive(), b, pWorld, true );
		SBuilding sb( pB, b.mpos );
		buildings.push_back( sb );
		SetBuildingPos( &sb, ptShift, fRotation );
		pB->UpdateAllParts();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::AddBox( IRenderVisitor *p, const SFBTransform &pos )
{
	CVec3 color( 0.4f, 0.6f, 0.7f );
	CVec4 cr( 0.58f, 0.77f, 0.95f, GetUserSettings().GetParam( ME_SUBTEMPLATE_ALPHA ) );
	const CVec3 ptOrig( VNULL3 );
	const CVec3 ptSize( ptBox );
	vector<CVec3> points( 10, ptOrig );

	p->AddMesh( pBox, cr, pos );
	//
	points[1].x += ptSize.x;
	points[2]   += CVec3( ptSize.x, ptSize.y, 0 );
	points[3].y += ptSize.y;
	points[5].z += ptSize.z;
	points[6]   += CVec3( ptSize.x, 0, ptSize.z );
	points[7]   += ptSize;
	points[8]   += CVec3( 0, ptSize.y, ptSize.z );
	points[9].z += ptSize.z;
	for ( int i = 0; i < points.size(); ++i )
		pos.forward.RotateHVector( &points[i], points[i] );
	p->AddPolyline( points, color );

	points.clear();
	points.resize( 2, ptOrig );
	points[0].x += ptSize.x;
	points[1]   += CVec3( ptSize.x, 0, ptSize.z );
	for ( int i = 0; i < points.size(); ++i )
		pos.forward.RotateHVector( &points[i], points[i] );
	p->AddPolyline( points, color );

	points[0] = ptOrig + CVec3( ptSize.x, ptSize.y, 0 );
	points[1] = ptOrig + CVec3( ptSize.x, ptSize.y, ptSize.z );
	for ( int i = 0; i < points.size(); ++i )
		pos.forward.RotateHVector( &points[i], points[i] );
	p->AddPolyline( points, color );

	points[0] = ptOrig + CVec3( 0, ptSize.y, 0 );
	points[1] = ptOrig + CVec3( 0, ptSize.y, ptSize.z );
	for ( int i = 0; i < points.size(); ++i )
		pos.forward.RotateHVector( &points[i], points[i] );
	p->AddPolyline( points, color );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSubTemplate::GetOccupiedRect( CTRect<float> *pRes ) const
{
	if ( !IsValidRect() )
	{
		*pRes = rLastOccupied;
		return false;
	}
	CVec2 va( pRect->ptCenter );
	CVec2 vb( FP_GRID_STEP * pRect->fWidth, FP_GRID_STEP * pRect->fHeight );
	CVec2 vc( FP_GRID_STEP * pRect->fWidth, 0 );
	CVec2 vd( 0, FP_GRID_STEP * pRect->fHeight );

	RotatePt( &vb, pRect->fRotation );
	RotatePt( &vc, pRect->fRotation );
	RotatePt( &vd, pRect->fRotation );
	vb += va;
	vc += va;
	vd += va;
	pRes->minx = Min( va.x, Min( vb.x, Min( vc.x, vd.x ) ) ) - FP_GRID_STEP;
	pRes->miny = Min( va.y, Min( vb.y, Min( vc.y, vd.y ) ) ) - FP_GRID_STEP;
	pRes->maxx = Max( va.x, Max( vb.x, Max( vc.x, vd.x ) ) ) + FP_GRID_STEP;
	pRes->maxy = Max( va.y, Max( vb.y, Max( vc.y, vd.y ) ) ) + FP_GRID_STEP;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec2 CSubTemplate::GetRectCenter() const
{
	CVec2 v( pRect->fWidth, pRect->fHeight );
	v *= 0.5f * FP_GRID_STEP;
	RotatePt( &v, pRect->fRotation );
	return pRect->ptCenter + v;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::ShowInfo( bool bShow )
{
	bShowInfo = bShow;
	bindGlobal.Update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSubTemplate::SetBuildingPos( SBuilding *p, const CVec3 &ptDelta, float fRotationDelta )
{
	SFBTransform pos;
	CVec3 ptPos = p->pos.ptPos;
	CVec3 vLocalOld = ptPos - ptCreatePos;
	CVec3 vLocalNew = vLocalOld;
	RotatePt( &vLocalNew, fRotationDelta );
	ptPos += ptDelta + vLocalNew - vLocalOld;
	MakeMatrix( &pos, CVec3(1,1,1), ptPos, ToRadian( p->pos.fRotation + fRotationDelta ) );

	p->pBuilding->SetPosition( pos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IEditorSubTemplate* IEditorSubTemplate::Create( IEditorWorld *pWorld, NDb::CRectangle *pRect, int nCutFloor )
{
	return new CSubTemplate( pWorld, pRect, nCutFloor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
