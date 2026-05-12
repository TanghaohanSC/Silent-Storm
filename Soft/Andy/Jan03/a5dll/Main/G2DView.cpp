#include "StdAfx.h"
#include "GSceneUtils.h"
#include "GMemFormat.h"
#include "GFont.h"
#include "GLocale.h"
#include "GText.h"
#include "GTexture.h"
#include "GMemFormat.h"
#include "GMemBuilder.h"
#include "..\Misc\BasicShare.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
#include "2DScene.h"
#include "G2DView.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// share objects
CBasicShare<int, CFileFont> shareFonts(107);
externA5 CBasicShare<STextureKey, CFileTexture, STextureKeyHash> shareTextures;
////////////////////////////////////////////////////////////////////////////////////////////////////
class C2DGameView: public I2DGameView
{
	OBJECT_BASIC_METHODS(C2DGameView);
private:
	ZDATA
	CDGPtr<CCVec2> pScreenRect;
	CObj<I2DScene> pScene;
	CObj<CTextLocaleInfo> pLocale;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pScreenRect); f.Add(3,&pScene); f.Add(4,&pLocale); return 0; }

public:
	C2DGameView();
	
	CRects* CreateRects( NDb::CTexture *pTexture, CFuncBase<CRectLayout> *pLayout, CFuncBase< CTRect<int> > *pSize = 0 );
	CRects* CreateFullRect( NDb::CTexture *pTexture, CFuncBase< CTRect<int> > *pSize = 0 );
	CRects* CreateClearRects( CFuncBase<CRectLayout> *pLayout, CFuncBase< CTRect<int> > *pSize = 0 );

	void CreateDynamicRects( CFuncBase<SText> *pText, const CTPoint<int> &sPosition, const CTRect<int> &sWindow );
	void CreateDynamicRects( NDb::CTexture *pTexture, const CRectLayout &sLayout, const CTPoint<int> &sPosition, const CTRect<int> &sWindow );
	void CreateDynamicRects( CPtrFuncBase<NGfx::CTexture> *pTexture, const CRectLayout &sLayout, const CTPoint<int> &sPosition, const CTRect<int> &sWindow );
	CFuncBase<SText>* CreateText( CFuncBase<wstring> *pText, CFuncBase< CTPoint<int> > *pSize, bool bProcessTAGs = true );
	void CreateDynamicClearRects( const CRectLayout &sLayout, const CTPoint<int> &sPosition, const CTRect<int> &sClipWindow, float fZ = 1.0f );

	virtual const CVec2& GetViewportSize() { return pScreenRect->GetValue(); }
	CTextLocaleInfo* GetLocaleInfo() const { return pLocale; }
	void StartNewFrame();
	void Flush();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// C2DGameView
////////////////////////////////////////////////////////////////////////////////////////////////////
static void ss_2dv_trace(const char* s) {
	FILE* fp = NULL; fopen_s(&fp, "silent_storm_step_trace.log", "a");
	if (fp) { fprintf(fp, "[2DV] %s\n", s); fclose(fp); }
}
C2DGameView::C2DGameView()
{
	ss_2dv_trace("C2DGV.0 entry");
	pScene = Make2DScene();
	ss_2dv_trace("C2DGV.1 Make2DScene ok");
	pScreenRect = new CCVec2( NGfx::GetScreenRect() );
	ss_2dv_trace("C2DGV.2 ScreenRect ok");
	pLocale = new CTextLocaleInfo;
	ss_2dv_trace("C2DGV.3 new CTextLocaleInfo ok");
	pLocale->Setup( NGfx::GetScreenRect() );
	ss_2dv_trace("C2DGV.4 pLocale->Setup ok");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRects* C2DGameView::CreateRects( NDb::CTexture *pTexture, CFuncBase<CRectLayout> *pLayout, CFuncBase< CTRect<int> > *pSize )
{
	return pScene->CreateRects( shareTextures.Get( pTexture->GetRecordID() ), pLayout, pSize );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRects* C2DGameView::CreateFullRect( NDb::CTexture *pTexture, CFuncBase< CTRect<int> > *pSize )
{
	CRectLayout sLayout;
	sLayout.AddRect( 0, 0, CTRect<float>( 0, pTexture->nHeight, pTexture->nWidth, 0 ) );
	CPtr<CCRectLayout> pLayout = new CCRectLayout;
	pLayout->Set( sLayout );

	return CreateRects( pTexture, pLayout, pSize );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CRects* C2DGameView::CreateClearRects( CFuncBase<CRectLayout> *pLayout, CFuncBase< CTRect<int> > *pSize )
{
	return pScene->CreateClearRects( pLayout, pSize );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void C2DGameView::CreateDynamicRects( CFuncBase<SText> *pText, const CTPoint<int> &sPosition, const CTRect<int> &sWindow )
{
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"C2DGV::CreateDynRects #%d pText=%p\n",n,pText); fclose(_f);} ++n; } }
	CDGPtr< CFuncBase<SText> > pFormater( pText );

	pFormater.Refresh();
	const SText &sText = pFormater->GetValue();
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"C2DGV::CreateDynRects #%d rectLayouts.size=%d\n",n,(int)sText.rectLayouts.size()); fclose(_f);} ++n; } }
	for ( int nTemp = 0; nTemp < sText.rectLayouts.size(); nTemp++ )
	{
		const SText::SFontLayout &sLayout = sText.rectLayouts[nTemp];
		{ static int n=0; if(n<6){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
		  if(_f){fprintf(_f,"C2DGV::CreateDynRects #%d iter=%d pFontInfo=%p\n",n,nTemp,(void*)sLayout.pFontInfo.GetPtr()); fclose(_f);} ++n; } }
		// silent-storm-port Phase 1.5 r2: skip glyph layouts whose font info
		// hasn't been resolved yet (was a crash before — null deref).
		if ( !sLayout.pFontInfo )
		{
			static int nWarn = 0;
			if ( nWarn < 4 )
			{
				++nWarn;
				FILE* _f = NULL; fopen_s(&_f,"silent_storm_im.log","a");
				if (_f) { fprintf(_f,"C2DGV::CreateDynRects skip iter=%d (null pFontInfo)\n", nTemp); fclose(_f); }
			}
			continue;
		}
		pScene->CreateDynamicRects( sLayout.pFontInfo->GetTexture(), sLayout.sLayout, sPosition, sWindow );
	}

	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void C2DGameView::CreateDynamicRects( NDb::CTexture *pTexture, const CRectLayout &sLayout, const CTPoint<int> &sPosition, const CTRect<int> &sWindow )
{
	if ( !pTexture )
		pScene->CreateDynamicRects( 0, sLayout, sPosition, sWindow );
	else
		pScene->CreateDynamicRects( shareTextures.Get( pTexture->GetRecordID() ), sLayout, sPosition, sWindow );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void C2DGameView::CreateDynamicRects( CPtrFuncBase<NGfx::CTexture> *pTexture, const CRectLayout &sLayout, const CTPoint<int> &sPosition, const CTRect<int> &sWindow )
{
	pScene->CreateDynamicRects( pTexture, sLayout, sPosition, sWindow );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CFuncBase<SText>* C2DGameView::CreateText( CFuncBase<wstring> *pText, CFuncBase< CTPoint<int> > *pSize, bool bProcessTAGs )
{
	return CreateTextFormater( pLocale, pScreenRect, pText, pSize, bProcessTAGs );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void C2DGameView::CreateDynamicClearRects( const CRectLayout &sLayout, const CTPoint<int> &sPosition, const CTRect<int> &sClipWindow, float fZ )
{
	pScene->CreateDynamicClearRects( sLayout, sPosition, sClipWindow, fZ );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void C2DGameView::StartNewFrame()
{
	pScreenRect.Refresh();
	if ( pScreenRect->GetValue() != NGfx::GetScreenRect() )
		pScreenRect->Set( NGfx::GetScreenRect() );
	pLocale->Setup( NGfx::GetScreenRect() );

	pScene->StartNewFrame( 0, NGfx::GetScreenRect() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void C2DGameView::Flush()
{
	pScene->Flush();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Create 2D View
////////////////////////////////////////////////////////////////////////////////////////////////////
I2DGameView* CreateNew2DView()
{
	return new C2DGameView;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0xF1741142, C2DGameView );
