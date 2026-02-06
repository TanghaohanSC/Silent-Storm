#ifndef __GTERRAININFO_H_
#define __GTERRAININFO_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
#include "..\Misc\2Darray.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
	class CMaterial;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct STerrainHole
{
	int nHeight;
	bool bVisible;
	vector<CVec2> vPolygon;

	int operator&( CStructureSaver &f );
};
//////////////////////////////////////////////////////////////////////////////////////	
const float FP_GRASS_COLOR_SCALE = 0.5f;
struct SGrassLayer
{
	ZDATA
	int nGrassID;
	CArray2D<BYTE> grass;
	float fMaxDensity;
	CArray2D<DWORD> grassColor;
	vector<CVec2> blades;
	int nID;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nGrassID); f.Add(3,&grass); f.Add(4,&fMaxDensity); f.Add(5,&grassColor); f.Add(6,&blades); f.Add(7,&nID); return 0; }
};
//////////////////////////////////////////////////////////////////////////////////////	
struct STerrainSpot
{
	ZDATA
	CDBPtr<NDb::CMaterial> pMaterial;
	CVec2 ptPos, ptSize;
	float fAngle; // in radians;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pMaterial); f.Add(3,&ptPos); f.Add(4,&ptSize); f.Add(5,&fAngle); return 0; }
	STerrainSpot() {}
	STerrainSpot( NDb::CMaterial *_pM, const CVec2 &_ptPos, const CVec2 &_ptSize, float _fAngle ) 
		: pMaterial(_pM), ptPos(_ptPos), ptSize(_ptSize), fAngle(_fAngle) {}
};
//////////////////////////////////////////////////////////////////////////////////////	
struct STerrainInfo
{
	int nWidth;
	int nHeight;
	CArray2D<unsigned char> typeMap;
	CArray2D<unsigned short> heightMap;
	vector<STerrainHole> holes;
	vector<SGrassLayer>  grass;
	CArray2D<DWORD> color;
	vector<STerrainSpot> spots;
	int nMaxGrassLayerID;
	STerrainInfo() : nWidth( 0 ), nHeight( 0 ), nMaxGrassLayerID( 1 ) { typeMap[0][0] = 0; heightMap[0][0] = 0; color[0][0] = 0xffffffff; }
	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTerrainPart: public CObjectBase
{
	OBJECT_BASIC_METHODS( CTerrainPart );
public:
	ZDATA
	bool bInverseNormals;
	vector<CVec3> verts;
	vector<STriangle> faces;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bInverseNormals); f.Add(3,&verts); f.Add(4,&faces); return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTerrainInfoHolder: public CFuncBase<STerrainInfo>
{
	OBJECT_BASIC_METHODS(CTerrainInfoHolder);
	CArray2D<CObj<CVersioningBase> > geometryGrid;
	CArray2D<CObj<CVersioningBase> > textureGrid;
	CArray2D<CObj<CVersioningBase> > grassGrid;
	
	void UpdateRegion( CArray2D<CObj<CVersioningBase> > *pGrid, const CTRect<int> &sRegion );
	CVersioningBase* GetRegion( const CArray2D<CObj<CVersioningBase> > &grid, const CTRect<int> &sRegion );

public:

	CTerrainInfoHolder() {}
	CTerrainInfoHolder( const STerrainInfo &info );

	int operator&( CStructureSaver &f );

	void UpdateRegionGeometry( const CTRect<int> &sRegion );
	void UpdateRegionTexture( const CTRect<int> &sRegion );
	void UpdateRegionGrass( const CTRect<int> &sRegion );
	CVersioningBase* GetRegionGeometry( const CTRect<int> &sRegion );
	CVersioningBase* GetRegionTexture( const CTRect<int> &sRegion );
	CVersioningBase* GetRegionGrass( const CTRect<int> &sRegion );
	STerrainInfo& GetWritableInfo();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_DG_CONSTANT_NODE( CCTerrainInfo, STerrainInfo );
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
