#ifndef __RPGVision_H_
#define __RPGVision_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
	class IAIMap;
	class IPathNetwork;
}
namespace NWorld
{
	class CUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
const int N_SIGHTDISTANCE = 20;

class CVisionCube;
struct SVisibilitySpot;
class CVisionTracker
{
	typedef hash_map<CVec3, CObj<CVisionCube>, SVec3Hash> CVisionHash;
	ZDATA
	CVisionHash visionCache;
	CPtr<NAI::IAIMap> pAIMap;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&visionCache); f.Add(3,&pAIMap); return 0; }
private:
	CVisionCube* GetCube( const CVec3 &ptFrom );
public:
	CVisionTracker() {}
	CVisionTracker( NAI::IAIMap *_pAIMap ): pAIMap(_pAIMap) {}
	bool IsCubeVisible( const CVec3 &ptFrom, const CVec3 &ptTarget, const CVec3 &ptForward );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif