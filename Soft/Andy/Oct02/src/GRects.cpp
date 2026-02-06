#include "StdAfx.h"
#include "GRects.h"
#include "GfxUtils.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderRectLayout( NGfx::C2DQuadsRenderer *pRes, NGfx::CTexture *pTex, const CRectLayout &l, float fZ, ELayoutRenderMode lrm )
{
	for ( int i = 0; i < l.rects.size(); ++i )
	{
		const CRectLayout::SRect &r = l.rects[i];
		NGfx::SPixel8888 color = r.sColor;
		if ( lrm == LRM_CLEAR_RECT )
			color.a = 0;
		CTRect<float> rTarget( r.fX, r.fY, r.fX + abs( r.sTex.GetWidth() ) * l.scale.x, r.fY + abs( r.sTex.GetHeight() ) * l.scale.y );
		CTRect<float> rSrc( r.sTex.rcTexRect ); 
		pRes->AddRect( rTarget, pTex, rSrc, color, fZ );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NGScene;
