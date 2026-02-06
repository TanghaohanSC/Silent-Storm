#include "StdAfx.h"
#include "DG.h"
#include "Gfx.h"
#include "GfxBuffers.h"
#include "2DSceneSW.h"
#include "SWTexture.h"
#include "GSceneUtils.h"
#include "ScreenShot.h"
#include "..\Misc\2DArray.h"
#include "..\DBFormat\DataFormat.h"
#include "..\Misc\BasicShare.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScreenshotTexture
///////////////////////////////////////////////////////////////////////////////////////////////////
void CBWScreenshotTexture::Generate()
{
	CVec2 vScreenRect = NGfx::GetScreenRect();

	sSize = CTPoint<int>( GetNextPow2( Float2Int( vScreenRect.x ) ), GetNextPow2( Float2Int( vScreenRect.y ) ) );

	int nMinSize = Min( sSize.x, sSize.y );
	for( ; nMinSize > 0; nMinSize >>= 1 )
	{
		pValue = NGfx::MakeTexture( sSize.x, sSize.y, 1, NGfx::SPixel8888::ID, NGfx::REGULAR, NGfx::CLAMP );
		if ( IsValid( pValue ) )
			break;

		sSize.x >>= 1;
		sSize.y >>= 1;
	}

	if ( !IsValid( pValue ) )
		return;

	NGfx::MakeScreenShot( &sScreenshot, false );

	if ( ( sScreenshot.GetXSize() > sSize.x ) || ( sScreenshot.GetYSize() > sSize.y ) )
	{
		CDGPtr<CPtrFuncBase<CSWTextureData> > pTexture = new CBilinearTexture( sScreenshot, 512, 512 );
		pTexture.Refresh();

		CPtr<CSWTextureData> pData = pTexture->GetValue();

		const CArray2D<NGfx::SPixel8888> &sSource = pData->mips[0];
		NGfx::CTextureLock<NGfx::SPixel8888> sLock( pValue, 0, NGfx::INPLACE );

		for ( int nTempY = 0; nTempY < sSource.GetYSize(); nTempY++ )
		{
			for ( int nTempX = 0; nTempX < sSource.GetXSize(); nTempX++ )
			{
				NGfx::SPixel8888 &sDst = sLock[nTempY][nTempX];
				const NGfx::SPixel8888 &sSrc = sSource[nTempY][nTempX];

				sDst.a = 0xFF;
				sDst.r = sDst.g = sDst.b = Float2Int( ( sSrc.r * 0.3086f ) + ( sSrc.g * 0.6094f ) + ( sSrc.b * 0.0820f ) );
			}
		}

		sSize.x = sLock.GetXSize();
		sSize.y = sLock.GetYSize();
	}
	else
	{
		NGfx::CTextureLock<NGfx::SPixel8888> sLock( pValue, 0, NGfx::INPLACE );

		for ( int nTempY = 0; nTempY < sScreenshot.GetYSize(); nTempY++ )
		{
			for ( int nTempX = 0; nTempX < sScreenshot.GetXSize(); nTempX++ )
			{
				NGfx::SPixel8888 &sDst = sLock[nTempY][nTempX];
				const NGfx::SPixel8888 &sSrc = sScreenshot[nTempY][nTempX];

				sDst.a = 0xFF;
				sDst.r = sDst.g = sDst.b = Float2Int( ( ( sSrc.r * 0.3086f ) + ( sSrc.g * 0.6094f ) + ( sSrc.b * 0.0820f ) ) / 2 );
			}
		}

		sSize.x = sScreenshot.GetXSize();
		sSize.y = sScreenshot.GetYSize();
	}

	Updated();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const CTPoint<int>& CBWScreenshotTexture::GetSize() const
{
	return sSize;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CBWScreenshotTexture::Recalc()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0xB2030101, CBWScreenshotTexture );
