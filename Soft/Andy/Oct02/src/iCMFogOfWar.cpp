#include "StdAfx.h"
#include "DG.h"
#include "GfxBuffers.h"
#include "2DSceneSW.h"
#include "iCMFogOfWar.h"
#include "GSceneUtils.h"
#include "SWTexture.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// Terrain texture
////////////////////////////////////////////////////////////////////////////////////////////////////
CFogOfWarTexture::CFogOfWarTexture( const SChapterInfo &sInfo ):
	bUpdated( false )
{
	sFogMap.SetSizes( sInfo.sPassabilityMap.GetXSize(), sInfo.sPassabilityMap.GetXSize() );
	sFogMap.FillEvery( 0xFF );

	pScene = Make2DSWScene();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFogOfWarTexture::Recalc()
{
	if ( !IsValid( pValue ) )
		pValue = NGfx::MakeTexture( sFogMap.GetXSize(), sFogMap.GetYSize(), 0, NGfx::SPixel8888::ID, NGfx::REGULAR, NGfx::CLAMP );
	if ( !IsValid( pValue ) )
		return;

	NGfx::CTextureLock<NGfx::SPixel8888> sLock( pValue, 0, NGfx::INPLACE );
	ASSERT( ( sLock.GetXSize() == sFogMap.GetXSize() ) && ( sLock.GetYSize() == sFogMap.GetYSize() ) );
	for ( int nTempY = 0; nTempY < sLock.GetYSize(); nTempY++ )
		for ( int nTempX = 0; nTempX < sLock.GetXSize(); nTempX++ )
			sLock[nTempY][nTempX] = NGfx::SPixel8888( 0x0, 0x0, 0x0, sFogMap[nTempY][nTempX] * 0.75f );

	bUpdated = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFogOfWarTexture::NeedUpdate()
{
	return bUpdated;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFogOfWarTexture::AddSpot( NDb::CTexture *pTexture, const CVec2 &vPos, float fRadius )
{
	CSWRectLayout sLayout;

	ASSERT( ( pTexture->nWidth != 0 ) && ( pTexture->nHeight != 0 ) );
	if ( ( pTexture->nWidth == 0 ) || ( pTexture->nHeight == 0 ) )
		return;

	sLayout.scale.x = fRadius * 2 / pTexture->nWidth;
	sLayout.scale.y = fRadius * 2 / pTexture->nHeight;
	sLayout.AddRect( vPos.x - fRadius, vPos.y - fRadius, CTRect<short>( 0, 0, pTexture->nWidth, pTexture->nHeight ) );

	CObj<ISWRects> pRects = pScene->CreateRects( GetSWTex( pTexture ), new CCSWRectLayout( sLayout ) );
	pScene->DrawFog( &sFogMap, CTPoint<int>( sFogMap.GetXSize(), sFogMap.GetYSize() ) );

	Recalc();
	bUpdated = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0xB8051161, CFogOfWarTexture );
