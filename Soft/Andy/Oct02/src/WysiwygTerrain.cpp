#include "StdAfx.h"
#include "IWysiwyg.h"
#include "WysiwygTerrain.h"
#include "weInterface.h"
#include "MEUserSettings.h"
#include "MESerialize.h"
#include "METerrain.h"
#include "MELayers.h"
#include "..\Misc\BasicShare.h"
#include "Grid.h"
#include "MemObject.h"
#include "gView.h"
#include "transform.h"
#include "BuildingInfo.h"

extern CBasicShare<int, CMETerrainLoader> shareTerrains;
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsPressed( int nKey )
{
	return 0x8000 & GetAsyncKeyState( nKey );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWysiwyg
{
bool IsTerrainLayer( ELayer el ) { return el == LID_HEIGHTS || el == LID_TILES || el == LID_GRASS || el == LID_ALPHA; }

static float brSizes[] = { 0.3f, 0.7f, 3.0f };
static vector<CObj<CMemObject> > brushes;
////////////////////////////////////////////////////////////////////////////////////////////////////
inline int FixBrushIndex( int nSizeID )
{
	return Min( (int)ARRAY_SIZE(brSizes) - 1, Max( 0, nSizeID ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CWysiwygTerrain::CWysiwygTerrain( NWorld::IEditorWorld *_pWorld, NGScene::IGameView *_pScene, int nTerrID ) 
: pWorld(_pWorld), pScene(_pScene), nTerrainID(nTerrID)
{
	if ( brushes.empty() )
	{
		for ( int i = 0; i < ARRAY_SIZE( brSizes ); ++i )
		{
			CMemObject* pO = *brushes.insert( brushes.end(), new CMemObject );
			pO->CreateCylinder( CVec3( 0, 0, 0.1f ), CVec3( 0, 0, 0.15f ), FP_GRID_STEP * brSizes[i], 10, true  );
		}
	}
	//
	bModified = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CMETerrainInfo* CWysiwygTerrain::GetTerrain()
{
	CDGPtr< CPtrFuncBase<CMETerrainInfo> > pLoader = shareTerrains.Get( nTerrainID );
	pLoader.Refresh();
	return pLoader->GetValue();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ELayer GetActiveLayer()
{
	ELayer eType;
	int nLayer;
	NBuilding::GetLayerID( GetUserSettings().GetActiveLayerID(), &eType, &nLayer );
	return eType;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwygTerrain::OnLButtonDown( const CVec2 &ptPos, CObjectBase *pObj, int nUserID, const CVec3 &ptCrossNormal )
{
	bLBDown = true;
	OnMouseMove( ptPos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwygTerrain::OnMouseMove( const CVec2 &ptPos )
{
	if ( !bLBDown )
		return;
	if ( !IsPressed( VK_LBUTTON ) )
		OnLButtonUp( ptPos );

	const IUserSettings &settings = GetUserSettings();

	ELayer eType;
	int nLayer;
	NBuilding::GetLayerID( settings.GetActiveLayerID(), &eType, &nLayer );
	EEditMode eMode = settings.GetMode();
	if ( eMode != EM_SELECT )
		return;

	CVec2 ptTilePos = FP_INV_GRID_STEP * ptPos;
	CMETerrainInfo *pTerr = GetTerrain();
	if ( !pTerr || ptPos == CVec2( PT_INVALID.x, PT_INVALID.y ) 
		|| ptTilePos.x < 0 || ptTilePos.x > pTerr->info.nWidth || ptTilePos.y < 0 || ptTilePos.y > pTerr->info.nHeight )
		return;
	const int nSize = FixBrushIndex( settings.GetBrushSize() );

	switch ( eType )
	{
	case  LID_HEIGHTS:
		{
			/*
			pTerr->info.heightMap[int(FP_INV_GRID_STEP * ptPos.y)][int(FP_INV_GRID_STEP * ptPos.x)] = settings.GetActiveFloor() * NBuilding::WALL_HEIGHT;
			CDGPtr<CFuncBase<STerrainInfo> > p = pWorld->GetEditableTerrain();
			p.Refresh();
			const STerrainInfo *pInfo = &p->GetValue();
			const_cast<STerrainInfo*>( pInfo )->heightMap = pTerr->info.heightMap;
			pWorld->UpdateTerrain( CTRect<float>( ptPos.x, ptPos.y, ptPos.x, ptPos.y ) );
			*/
			break;
		}
	case LID_GRASS:
		PaintGrass( nLayer, ptTilePos );
		break;
	case LID_TILES:
		PaintTexture( ptTilePos, brSizes[nSize] );
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwygTerrain::OnLButtonUp( const CVec2 &ptPos )
{
	bLBDown = false;
	if ( bModified )
		Serialize();
	bModified = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwygTerrain::Erase( const CVec2 &ptPos )
{
	const IUserSettings &settings = GetUserSettings();
	ELayer eType;
	int nLayer;
	NBuilding::GetLayerID( settings.GetActiveLayerID(), &eType, &nLayer );
	//
	CVec2 ptTilePos = FP_INV_GRID_STEP * ptPos;
	CMETerrainInfo *pTerr = GetTerrain();
	if ( !pTerr || ptPos == CVec2( PT_INVALID.x, PT_INVALID.y ) )
		return;
	//
	switch( eType )
	{
		case LID_GRASS:
			EraseGrass( nLayer, ptTilePos );
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWysiwygTerrain::Update( const CVec3 &ptCross, CObjectBase *pObj, int nUserID )
{
	renderParts.clear();
	lines.clear();

	if ( ptCross == PT_INVALID )
		return false;

	const IUserSettings &user = GetUserSettings();
	const ELayer eLayer = GetActiveLayer();
	const EEditMode eMode = user.GetMode();
	const int nSize = FixBrushIndex( user.GetBrushSize() );

	switch ( eLayer )
	{
		case LID_GRASS:
			if ( eMode == EM_ERASE )
				renderParts.push_back( pScene->CreateMesh( brushes[nSize], CVec4( 0.9f, 0.4f, 0.35f, 0.8f ), MakeTransform( ptCross ) ) );
			break;
		case LID_TILES:
			if ( eMode == EM_SELECT )
			{
				CVec3 pt( ptCross );
				pt.x = FP_GRID_STEP * Float2Int( pt.x * FP_INV_GRID_STEP );
				pt.y = FP_GRID_STEP * Float2Int( pt.y * FP_INV_GRID_STEP );
				pt.z = pt.z + 0.2f;
				DrawBrush( MakeTransform( pt ), brSizes[nSize], CVec3(0,0,1), 6 );
			}
			break;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CWysiwygTerrain::InvalidateTexture( const CTRect<float> &r )
{
	pWorld->InvalidateTerrainTextureRect( r );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CWysiwygTerrain::InvalidateTexture( const CVec2 &pt )
{
	pWorld->InvalidateTerrainTextureRect( CTRect<float>( Max( 0.0f, pt.x - 1 ), Max( 0.0f, pt.y - 1 ), pt.x + 1, pt.y + 1 ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CWysiwygTerrain::InvalidateGeometry( const CTRect<float> &r )
{
	pWorld->InvalidateTerrainGeometryRect( r );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CWysiwygTerrain::InvalidateGrass( const CTRect<float> &r )
{
	pWorld->InvalidateTerrainGrassRect( r );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CWysiwygTerrain::Serialize()
{
	CMETerrainInfo *pTerr = GetTerrain();
	if ( pTerr )
		SerializeTerrain( pTerr, nTerrainID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwygTerrain::PaintGrass( int nLayerID, const CVec2 &ptTilePos )
{
	if ( !(0x8000 & GetAsyncKeyState( VK_CONTROL )) )
		return;
	//
	CMETerrainInfo *pTerr = GetTerrain();

	for ( int i = 0; i < pTerr->info.grass.size(); ++i )
	{
		SGrassLayer &grass = pTerr->info.grass[i];
		if ( nLayerID == grass.nID )
		{
			grass.blades.push_back( ptTilePos );
			Serialize();
			pWorld->UpdateGrass( CTRect<float>() );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwygTerrain::EraseGrass( int nLayerID, const CVec2 &ptTilePos )
{
	CMETerrainInfo *pTerr = GetTerrain();

	for ( int i = 0; i < pTerr->info.grass.size(); ++i )
	{
		SGrassLayer &grass = pTerr->info.grass[i];
		if ( nLayerID == grass.nID )
		{
			for ( vector<CVec2>::iterator it = grass.blades.begin(); it != grass.blades.end(); )
			{
				if ( fabs( ptTilePos - *it ) < brSizes[FixBrushIndex( GetUserSettings().GetBrushSize() )] )
					it = grass.blades.erase( it );
				else
					++it;
			}
			Serialize();
			pWorld->UpdateGrass( CTRect<float>() );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CWysiwygTerrain::SetTile( CArray2D<unsigned char> *pTiles, int nx, int ny, unsigned char val )
{
	if ( nx < 0 || ny < 0 || nx >= pTiles->GetXSize() || ny >= pTiles->GetYSize() )
		return;
	if ( (*pTiles)[ny][nx] == val )
		return;
	(*pTiles)[ny][nx] = val;
	InvalidateTexture( CVec2( nx, ny ) * FP_GRID_STEP );
	bModified = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwygTerrain::PaintTexture( const CVec2 &ptTilePos, float fRadius )
{
	int nTex = GetUserSettings().GetSelectedBrushID( NBuilding::MakeFragmentID( LID_TILES, 0 ) );
	if ( nTex < 0 )
		return;
	CMETerrainInfo *pTerr = GetTerrain();

	int nx = Float2Int( ptTilePos.x );
	int ny = Float2Int( ptTilePos.y );
	int nRadius = Float2Int( FP_INV_GRID_STEP * fRadius );
	CArray2D<unsigned char> &tiles = pTerr->info.typeMap;
	if ( nRadius < 1 )
	{
		SetTile( &tiles, nx, ny, nTex );
		return;
	}
	//
	for ( int x = nx - nRadius; x <= nx + nRadius; ++x )
		for ( int y = ny - nRadius; y <= ny + nRadius; ++y )
			if ( sqr( nx - x ) + sqr( ny - y ) <= nRadius * nRadius )
				SetTile( &tiles, x, y, nTex );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWysiwygTerrain::DrawBrush( const SFBTransform &pos, float fRadius, const CVec3 &color, int nSegs )
{
	vector<CVec3> points;

	const float fStep = FP_PI2 / nSegs;
	for ( float f = 0; f < FP_PI2; f += fStep )
		points.push_back( CVec3( fRadius * cos( f ), fRadius * sin( f ), 0 ) );
	for ( int i = points.size() - 1; i >= 0; --i )
	{
		const CVec3 &pt = points[i];
		points.push_back( CVec3( -pt.x, pt.y, pt.z ) );
	}
	for ( int i = points.size() - 1; i >= 0; --i )
	{
		const CVec3 &pt = points[i];
		points.push_back( CVec3( pt.x, -pt.y, pt.z ) );
	}
	for ( int i = 0; i < points.size(); ++i )
		pos.forward.RotateHVector( &points[i], points[i] );
	lines.push_back( pScene->CreatePolyline( points, color ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
