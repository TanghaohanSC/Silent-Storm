#ifndef __aiVoxel_H_
#define __aiVoxel_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\Misc\2DArray.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIViewer;
namespace NWorld
{
struct IVisObj;
}
namespace NAI
{
class CFastRenderer;
struct SDoorColliderAnalyzer;
const float F_VOXEL_HEIGHT = 0.1f;
enum EAdditionalCode
{
	ADC_CLOSED = 1,
	ADC_OPEN = 2,
	ADC_ADD_TO_SPARE_NUMBER = 4
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVoxelCube : public CObjectBase
{
	OBJECT_BASIC_METHODS(CVoxelCube);
	CArray3D<unsigned char> intersectCodes;
	typedef hash_map<CPtr<NWorld::IVisObj>, char, SPtrHash> CObjectNumbers;
	CObjectNumbers objectNumbers;
	vector< CPtr<NWorld::IVisObj> > objects;
	int nSpareNumber;
	CVec2 ptOrig, ptXDir;
	char GetObjectIntersectCode( NWorld::IVisObj *pObject, int nTSFlags );
	void CheckVoxel( int nCode, SDoorColliderAnalyzer *pAn );
	void CheckCircle( int nX, int nY, int z, SDoorColliderAnalyzer *pAn );
public:
	void Prepare( CFastRenderer *pRender );
	// coords are relative, not absolute; radius is always 3 voxels
	void SphereTest( int nX, int nY, float fH, SDoorColliderAnalyzer *pAn );
	void MovingSphereTest( int nX, int nY, float fH, int nDX, int nDY, float fDZ, SDoorColliderAnalyzer *pAn );
	void MovingSphereTest( const CVec3 &ptCenter, const CVec3 &vel, SDoorColliderAnalyzer *pAn );
	CVoxelCube() {}
	CVoxelCube( const CVec2 &_ptOrig, const CVec2 &_ptXDir ) 
		: nSpareNumber(ADC_ADD_TO_SPARE_NUMBER), ptOrig(_ptOrig), ptXDir(_ptXDir)  { objects.resize(64); }
		friend class ::CAIViewer;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif