#include "StdAfx.h"
#include "RPGVision.h"
#include "aiRender.h"
#include "DG.h"
#include "wTSFlags.h"
#include "aiVision.h"
#include "..\DBFormat\DataRPG.h"
#include "aiMap.h"

const int N_HALFSIZE = 32;

namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TraceSide( NAI::IAIMap *pMap, CArray2D<float> *pRes, const CVec3 &ptFrom, int nSide )
{
	NAI::CFastRenderer render;
	NAI::TraceSide( pMap, &render, ptFrom, nSide, N_SIGHTDISTANCE, N_HALFSIZE, NWorld::TS_VISION );
	pRes->SetSizes( N_HALFSIZE * 2, N_HALFSIZE * 2 );
	ASSERT( pRes->GetXSize() == render.resGrid.GetXSize() && pRes->GetYSize() == render.resGrid.GetYSize() );
	for ( int y = 0; y < render.resGrid.GetYSize(); ++y )
	{
		for ( int x = 0; x < render.resGrid.GetXSize(); ++x )
		{
			float fTransp = 1;	// ѕо умолчанию лучик не чего не закрывает
			float fDepth = N_SIGHTDISTANCE;
			for ( NAI::CFastRenderer::SResult *p = render.resGrid[y][x]; p; p = p->pNext, fDepth = N_SIGHTDISTANCE * fTransp )
			{
				if ( p->fEnter > fDepth )//N_SIGHTDISTANCE )//&& fDistance <= intr[i].fExit )
					break;//continue;

				NDb::CRPGArmor *pArmor = p->GetInfo().pArmor;
				if ( pArmor && pArmor->fTransparency > 0 )
				{
					float fExit = Min( p->fExit, fDepth );//N_SIGHTDISTANCE );
					float fLength = fExit - p->fEnter;
					ASSERT( fLength >= 0 );
					float fMod = pow( 2, - fLength / pArmor->fTransparency );
					if ( N_SIGHTDISTANCE * fTransp * fMod < p->fExit )
					{
						// search for solution
						while ( N_SIGHTDISTANCE * fTransp * fMod < p->fEnter + fLength )
						{
							fLength /= 2;
							fMod = sqrt( fMod );
						}
						fTransp = ( p->fEnter + fLength ) / N_SIGHTDISTANCE;
					}
					else
					{
						// full piercing is possible
						fTransp *= fMod;
					}
				}
				else
				{
					fDepth = p->fEnter;
					break;
				}
			}
			(*pRes)[y][x] = fDepth;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVisionCube: public NAI::IAIMapTracker
{
	OBJECT_BASIC_METHODS(CVisionCube);
public:
	ZDATA
	CVec3 ptFrom;
	CPtr<NAI::IAIMap> pAIMap;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&ptFrom); f.Add(3,&pAIMap); return 0; }
	CArray2D<float> sides[6];
	bool bCalced[6];

	CVisionCube() { Zero( bCalced ); }
	CVisionCube( NAI::IAIMap *_pMap, const CVec3 &_ptFrom ): pAIMap(_pMap), ptFrom(_ptFrom)
	{ 
		SBound b;
		b.SphereInit( ptFrom, N_SIGHTDISTANCE );
		pAIMap->AddTracker( this, b, NWorld::TS_VISION, true );
		Zero( bCalced );
	}
	virtual void OnChange() { Zero( bCalced ); }
	float GetVisionDistance( const CVec3 &ptTarget );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
float CVisionCube::GetVisionDistance( const CVec3 &ptTarget )
{
	NAI::SCalcIndex index;
	NAI::GetIndex( &index, ptFrom, ptTarget, N_HALFSIZE );
	if ( !bCalced[ index.nSide ] )
	{
		//for ( int k = 0; k < 50; ++k )
		TraceSide( pAIMap, &sides[ index.nSide ], ptFrom, index.nSide );
		bCalced[ index.nSide ] = true;
	}
	return sides[index.nSide][index.nY][index.nX];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVisionTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
CVisionCube* CVisionTracker::GetCube( const CVec3 &ptFrom )
{
	CVisionHash::iterator i = visionCache.find( ptFrom );
	if ( i != visionCache.end() )
		return i->second;
	if ( visionCache.size() > 500 )
		visionCache.clear();
	CObj<CVisionCube> &pRes = visionCache[ptFrom];
	pRes = new CVisionCube( pAIMap, ptFrom );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsVisible( CVisionCube *pCube, const CVec3 &ptTarget, float fTestDist )
{
	return pCube->GetVisionDistance( ptTarget ) >= fTestDist;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVisionTracker::IsCubeVisible( const CVec3 &ptFrom, const CVec3 &ptTarget, const CVec3 &ptForward )
{
	CVec3 ptDif = ptTarget - ptFrom;
	if ( ptDif * ptForward < -1e-5f )
		return false;
	float fDistance = fabs( ptDif );
	if ( fDistance >= N_SIGHTDISTANCE )
		return false;
	CObj<CVisionCube> pCube = GetCube( ptFrom );
	int nCount = 0;
	nCount += IsVisible( pCube, ptTarget, fDistance );
	nCount += IsVisible( pCube, ptTarget + CVec3(-0.2f, -0.2f, -0.2f), fDistance );
	nCount += IsVisible( pCube, ptTarget + CVec3(0.2f, -0.2f, -0.2f), fDistance );
	nCount += IsVisible( pCube, ptTarget + CVec3(-0.2f, 0.2f, -0.2f), fDistance );
	nCount += IsVisible( pCube, ptTarget + CVec3(0.2f, 0.2f, -0.2f), fDistance );
	nCount += IsVisible( pCube, ptTarget + CVec3(-0.2f, -0.2f, 0.2f), fDistance );
	nCount += IsVisible( pCube, ptTarget + CVec3(0.2f, -0.2f, 0.2f), fDistance );
	nCount += IsVisible( pCube, ptTarget + CVec3(-0.2f, 0.2f, 0.2f), fDistance );
	nCount += IsVisible( pCube, ptTarget + CVec3(0.2f, 0.2f, 0.2f), fDistance );
	return nCount >= 4;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NRPG;
REGISTER_SAVELOAD_CLASS( 0x026b1180, CVisionCube )
