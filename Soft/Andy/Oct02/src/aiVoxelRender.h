#ifndef __AIVOXELRENDER_H_
#define __AIVOXELRENDER_H_
//
#include "Transform.h"
#include "Render.h"
#include "..\Misc\2DArray.h"
//
namespace NDb
{
	class CRPGArmor;
}
//
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_VOXEL_TERRAIN = 1;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SConvexHull;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVoxelRenderer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVoxelRenderer: public CRasterizer<CVoxelRenderer>
{
public:
	enum EAxis
	{
		AXIS_X,
		AXIS_Y,
		AXIS_Z
	};
	//
	struct SExplObject
	{
		ZDATA
		bool bDestroyed;
		bool bTerrain;
		int nUserID;
		CPtr<NWorld::IVisObj> pUserData;
		CDBPtr<NDb::CRPGArmor> pArmor;
		// damage
		int nVolume;
		CRay rDir;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bDestroyed); f.Add(3,&bTerrain); f.Add(4,&nUserID); f.Add(5,&pUserData); f.Add(6,&pArmor); f.Add(7,&nVolume); f.Add(8,&rDir); return 0; }
		//
		SExplObject() {}
		SExplObject( NWorld::IVisObj *_pUserData, int _nUserID, NDb::CRPGArmor *_pArmor, bool _bTerrain ):
			nUserID( _nUserID ), pUserData( _pUserData ), pArmor( _pArmor ), 
			bTerrain( _bTerrain ), bDestroyed( false ), nVolume( 0 ) {}
	};
	typedef unsigned short ushort;
	struct SExplVoxel
	{
		ZDATA
		ushort nObject;
		ushort nIndex;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nObject); f.Add(3,&nIndex); return 0; }
		//
		SExplVoxel() {}
		SExplVoxel( unsigned short _nObject, unsigned short _nIndex ):
			nObject( _nObject ), nIndex( _nIndex ) {}
	};
	//
private:
	CVec3 vCenter;
	float fCubeSize;
	int nResolution;
	SHMatrix transform; // world space ==> camera space
	CTransformStack ts;
	CTRect<int> region;
	int nCurrentObjectID;
	EAxis axis;
	//
	bool DoRenderBackface() const { return true; }
	void RealTraceEntity( const SConvexHull &e );
	void AddObject( SExplObject object );
	//
	void ClipVertical( int *pnSY, int *pnFY2, int *pnFY )
	{
		(*pnSY) = Max( *pnSY, region.y1 );
		(*pnFY2) = Min( *pnFY2, region.y2 );
		(*pnFY) = Min( *pnFY, region.y2 );
	}
	//
	void ClipHorizontal( int *pnSX, int *pnFX )
	{
		(*pnSX) = Max( *pnSX, region.x1 );
		(*pnFX) = Min( *pnFX, region.x2 );
	}
	//
	void RasterSpan( int nY, int nLeft, int nRight, float fZ, float fDZ, int nBackface )
	{
		float fWZ = fZ;
		float fShift = nBackface - 0.5f;
		for ( int nX = nLeft; nX < nRight; ++nX, fWZ += fDZ )
			if ( fWZ >= 0 && fWZ <= 1 )
			{
				int nWX, nWY, nWZ;
				switch ( axis )
				{
					case AXIS_X:
						nWX = Float2Int( fShift + fWZ * ( nResolution - 1 ) );
						nWZ = nY - region.y1;
						nWY = nResolution - nX + region.x1;
						break;
					case AXIS_Y:
						nWY = Float2Int( fShift + fWZ * ( nResolution - 1 ) );
						nWZ = nY - region.y1;
						nWX = nX - region.x1;
						break;
					case AXIS_Z:
						nWZ = Float2Int( fShift + fWZ * ( nResolution - 1 ) );
						nWX = nResolution - nY + region.y1;
						nWY = nResolution - nX + region.x1;
						break;
				}
				//
				voxels[nWX][nWY][nWZ].nObject = nCurrentObjectID;
			}
	}
public:
	//
	CArray3D<SExplVoxel> voxels;
	vector<SExplObject> *objects; // объект с индексом 0 - зарезервирован, 1 - terrain
	int *nObjectsEnd;
	//
	int operator&( CStructureSaver &f ) { ASSERT(0); return 0; }
	//
	bool TestSphere( const CVec3 &ptCenter, float fR );
	void GetPoints( vector<CVec3> *pEnters ) const {}
	void Init( const CVec3 &_vCenter, float _fCubeSize, int _nResolution, vector<SExplObject> *_objects, int *_nObjectsEnd );
	void InitParallel( EAxis _axis );
	//
	void TraceEntity( const vector<SConvexHull> &e, bool bTerrain )	{	ASSERT(0); }
	void TraceEntity( const SConvexHull &e, bool bTerrain );
	//
	friend class CRasterizer<CVoxelRenderer>;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __AIVOXELRENDER_H_