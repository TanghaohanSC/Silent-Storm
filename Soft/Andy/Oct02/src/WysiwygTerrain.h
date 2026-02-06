#ifndef __IWYSIWYGTERRAIN_H_
#define __IWYSIWYGTERRAIN_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
	class IEditorWorld;
}
namespace NGScene
{
	class IGameView;
	class CRenderNode;
	class CPolyline;
}
class CMETerrainInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWysiwyg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const float TERR_ERASE_RADIUS = 1;
const CVec3 PT_INVALID( -1, -1, -1 );
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWysiwygTerrain: public CObjectBase
{
	OBJECT_BASIC_METHODS(CWysiwygTerrain);
	int nTerrainID;
	NWorld::IEditorWorld *pWorld;
	CPtr<NGScene::IGameView> pScene;
	bool bLBDown;
	vector<CObj<NGScene::CRenderNode> > renderParts;
	list< CObj<NGScene::CPolyline> > lines;
	bool bModified;

	CMETerrainInfo* GetTerrain();
	void PaintGrass( int nLayerID, const CVec2 &ptTilePos );
	void EraseGrass( int nLayerID, const CVec2 &ptTilePos );
	void PaintTexture( const CVec2 &ptTilePos, float fRadius );
	void InvalidateTexture( const CTRect<float> &r );
	void InvalidateTexture( const CVec2 &pt );
	void InvalidateGeometry( const CTRect<float> &r );
	void InvalidateGrass( const CTRect<float> &r );
	void Serialize();
	void DrawBrush( const SFBTransform &pos, float fRadius, const CVec3 &color, int nSegs );
	void SetTile( CArray2D<unsigned char> *pTiles, int nx, int ny, unsigned char val );

public:
	CWysiwygTerrain() {}
	CWysiwygTerrain( NWorld::IEditorWorld *pWorld, NGScene::IGameView *pScene, int nTerrainID );

	void OnLButtonDown( const CVec2 &ptPos, CObjectBase *pObj, int nUserID, const CVec3 &ptCrossNormal );
	void OnMouseMove( const CVec2 &ptPos );
	void OnLButtonUp( const CVec2 &ptPos );
	void Erase( const CVec2 &ptPos );
	bool Update( const CVec3 &ptCross, CObjectBase *pObj, int nUserID );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsTerrainLayer( ELayer el );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __IWYSIWYGTERRAIN_H_
