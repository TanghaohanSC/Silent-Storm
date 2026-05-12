#include "StdAfx.h"
#include "GPixelFormat.h"
#include "Transform.h"
#include "GSceneUtils.h"
#include "RectLayout.h"
#include "GView.h"
#include "G2DView.h"
#include "GText.h"
#include "Interface.h"
#include "UIWrap.h"
#include "DiscretePos.h"
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataLight.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTextDraw
////////////////////////////////////////////////////////////////////////////////////////////////////
CTextDraw::CTextDraw( const SPoint &_sPosition, const SPoint &_sSize, const wstring &_wsText ):
	sPosition( _sPosition ), sSize( _sSize ), sRealSize( _sSize ), wsText(_wsText)
{
	pSize = new NGScene::CCTPoint;
	pTextString = new NGScene::CCWString;

	pSize->Set( sSize );
	pTextString->Set( wsText );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SPoint& CTextDraw::GetSize( NGScene::I2DGameView *pView )
{
	if ( IsValid( pView ) && ( ( sSize.x == -1 ) || ( sSize.y == -1 ) ) )
	{
		UpdateSize( pView );

		if ( !pText )
			pText = pView->CreateText( pTextString, pSize );

		pText.Refresh();
		const CVec2 &vScreenRect = pView->GetViewportSize();
		const NGScene::SText &sText = pText->GetValue();

		sRealSize = sSize;
		if ( sSize.x == -1 )
			sRealSize.x = sText.sSize.x * 1024 / vScreenRect.x;
		if ( sSize.y == -1 )
			sRealSize.y = sText.sSize.y * 768 / vScreenRect.y;

		return sRealSize;
	}

	return sSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTextDraw::SetSize( const SPoint &_sSize )
{
	sSize = _sSize;
	sRealSize = _sSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SPoint& CTextDraw::GetPosition() const
{
	return sPosition;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTextDraw::SetPosition( const SPoint &_sPosition )
{
	sPosition = _sPosition;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const wstring& CTextDraw::GetText() const
{
	return wsText;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTextDraw::SetText( const wstring &_wsText )
{
	wsText = _wsText;
	pTextString->Set( wsText );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// silent-storm-port Phase 1.5 r3 — debug-text + debug-rect relays into the
// bgfx overlay / ss_ui shader.  Defined in port/src/renderer/bgfx_init.cpp.
// ABI is extern-C / POD-only so STLport-compiled callers can use them
// without a C++ header dep.
extern "C" void ss_dbg_text_push(int virtX, int virtY, unsigned attr, const char* text);
extern "C" void ss_dbg_rect_push(int x1, int y1, int x2, int y2, unsigned abgr);
extern "C" void ss_dbg_glyph_push(int virtX, int virtY, unsigned abgr,
                                  int scale_x, int scale_y, const char* text);

void CTextDraw::Draw( CWindow *pWindow, const STime &sTime, NGScene::I2DGameView *pView )
{
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"CTextDraw::Draw #%d enter pText=%p\n",n,pText.GetPtr()); fclose(_f);} ++n; } }

	// Phase 1.5 r4 iter 4: parse markup tags inline.  For each visible char
	// in wsText, track the current ABGR color (default white) and split into
	// independent "runs" at each <br> (which becomes a newline on screen) or
	// each <color=...> change.  Each run gets its own ss_dbg_glyph_push at
	// the correct position so we render multi-line, multi-color text.
	//
	// Supported tags:
	//   <br>                — newline; advance pen Y by one line
	//   <color=red>         — change pen color (named: red/white/green/yellow/cyan)
	//   <color=#RRGGBB>     — change pen color (hex; converted to abgr)
	//   <center>            — centering hint (whole-string center; honored at flush)
	//   <font ...>          — ignored (we use one fixed bitmap font)
	{
		// Per-run buffer.
		char run_buf[96];
		int  run_n = 0;
		uint32_t run_color = 0xffffffffu;   // ABGR (LE) white
		uint32_t pen_color = 0xffffffffu;
		// Alignment intent: 0=left (default), 1=center, 2=right.
		int align = 0;
		// Line counter — used to lay out multiple <br>-separated runs vertically.
		int line = 0;

		bool in_tag = false;
		char tag_name[32];
		int  tag_n = 0;

		// Inline helpers ----------------------------------------------------
		struct Helper {
			static uint32_t named_color(const char* n) {
				if (!n[0]) return 0xffffffffu;
				if (!_stricmp(n, "red"))     return 0xff2020e0u; // ABGR: rich red
				if (!_stricmp(n, "white"))   return 0xffffffffu;
				if (!_stricmp(n, "green"))   return 0xff20c020u;
				if (!_stricmp(n, "yellow"))  return 0xff20e0e0u;
				if (!_stricmp(n, "cyan"))    return 0xffe0e020u;
				if (!_stricmp(n, "blue"))    return 0xffe02020u;
				if (!_stricmp(n, "gray"))    return 0xff808080u;
				if (!_stricmp(n, "black"))   return 0xff000000u;
				return 0xffffffffu;
			}
			static int hex_nyb(char c) {
				if (c >= '0' && c <= '9') return c - '0';
				if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
				if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
				return -1;
			}
			static uint32_t parse_color_attr(const char* s) {
				// Handles `color=red` or `color=#rrggbb`.  Returns ABGR.
				const char* eq = s;
				while (*eq && *eq != '=') ++eq;
				if (!*eq) return 0xffffffffu;
				++eq;
				if (*eq == '#') {
					++eq;
					int v[6] = {0};
					int i = 0;
					while (i < 6 && eq[i]) { int h = hex_nyb(eq[i]); if (h<0) break; v[i]=h; ++i; }
					if (i < 6) return 0xffffffffu;
					uint32_t r = (uint32_t)((v[0]<<4) | v[1]);
					uint32_t g = (uint32_t)((v[2]<<4) | v[3]);
					uint32_t b = (uint32_t)((v[4]<<4) | v[5]);
					return 0xff000000u | (b<<16) | (g<<8) | r;
				}
				return named_color(eq);
			}
		};

		auto flush_run = [&](int line_idx, uint32_t col){
			run_buf[run_n] = '\0';
			if (run_n <= 0) return;
			int lead = 0;
			while (run_buf[lead] == ' ') ++lead;
			if (!run_buf[lead]) { return; }
			// dbg_text path: ASCII overlay still useful, push first run only.
			if (line_idx == 0)
				ss_dbg_text_push(sPosition.x, sPosition.y, 0x0f, run_buf + lead);

			int n_chars = 0;
			for (int k = lead; run_buf[k]; ++k) ++n_chars;
			const int kCellW = 8;
			const int kCellH = 16;
			const int scale  = 2;
			int draw_w = n_chars * kCellW * scale;
			const int line_h = kCellH * scale + 4;   // 36 px between lines
			int draw_x = sPosition.x;
			int draw_y = sPosition.y + line_idx * line_h;
			// Apply horizontal alignment (<center>/<right>) within container.
			int avail_x = sSize.x;
			if (avail_x < 200 && sPosition.x == 0) avail_x = 1024;
			if (align == 1 && avail_x >= 200)
				draw_x = sPosition.x + (avail_x - draw_w) / 2;
			else if (align == 2 && avail_x >= 200)
				draw_x = sPosition.x + avail_x - draw_w - 16;  // 16px right margin
			// Vertical center only when the *whole* container is huge AND the
			// caller pinned sPosition.y to 0 — i.e. the "centered banner" case.
			// Otherwise honour Nival's sPosition.y so successive CText widgets
			// don't stack on top of each other (e.g. fullscreen INTERMISSION
			// box + "Work in progress" at y=32 are independent).
			if (align == 1 && sPosition.y == 0 && sSize.y >= 600) {
				int center_y = sPosition.y + (sSize.y - kCellH * scale) / 2;
				draw_y = center_y - line_h + line_idx * line_h;
			}
			ss_dbg_glyph_push(draw_x, draw_y, col, scale, scale, run_buf + lead);
		};

		for (int i = 0; i < (int)wsText.size(); ++i) {
			wchar_t wc = wsText[i];
			if (wc == L'<') { in_tag = true; tag_n = 0; continue; }
			if (wc == L'>') {
				in_tag = false;
				if (tag_n >= (int)sizeof(tag_name)) tag_n = (int)sizeof(tag_name) - 1;
				tag_name[tag_n] = '\0';
				// Tag handlers
				if (tag_n >= 2 && (tag_name[0]=='b'||tag_name[0]=='B') && (tag_name[1]=='r'||tag_name[1]=='R')) {
					// <br> — flush current run on its own line, start new line in same color
					flush_run(line, pen_color);
					run_n = 0;
					++line;
				} else if (tag_n >= 5 && (tag_name[0]=='c'||tag_name[0]=='C')
				           && !_strnicmp(tag_name, "color=", 6)) {
					// Color change in mid-run.  Flush what we have (with the OLD color),
					// then update pen color for subsequent chars.  Same line.
					if (run_n > 0) {
						flush_run(line, pen_color);
						run_n = 0;
						++line;   // visual offset to make color change visible
					}
					pen_color = Helper::parse_color_attr(tag_name);
					run_color = pen_color;
				} else if (tag_n >= 6 && !_strnicmp(tag_name, "center", 6)) {
					align = 1;
				} else if (tag_n >= 5 && !_strnicmp(tag_name, "right", 5)) {
					align = 2;
				} else if (tag_n >= 4 && !_strnicmp(tag_name, "left", 4)) {
					align = 0;
				}
				// Other tags (font, /color, etc.) ignored.
				continue;
			}
			if (in_tag) {
				if (tag_n < (int)sizeof(tag_name) - 1 && wc < 127)
					tag_name[tag_n++] = (char)wc;
				continue;
			}
			// Visible character — append to run if buffer has space.
			if (run_n >= (int)sizeof(run_buf) - 1) continue;
			if (wc >= 32 && wc < 127)       run_buf[run_n++] = (char)wc;
			else if (wc == 9 || wc == 10 || wc == 13) run_buf[run_n++] = ' ';
			else                            run_buf[run_n++] = '?';
		}
		// Flush any trailing run.
		if (run_n > 0) flush_run(line, pen_color);
	}
	if ( !pText )
	{
		{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
		  if(_f){fprintf(_f,"CTextDraw::Draw #%d pView->CreateText\n",n); fclose(_f);} ++n; } }
		pText = pView->CreateText( pTextString, pSize );
		{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
		  if(_f){fprintf(_f,"CTextDraw::Draw #%d CreateText returned %p\n",n,pText.GetPtr()); fclose(_f);} ++n; } }
	}

	SPoint sTextSize = GetSize( pView );
	SRect sScrWindow( sPosition.x, sPosition.y, sPosition.x + sTextSize.x, sPosition.y + sTextSize.y );
	SPoint sScrPosition( sPosition );
	if ( pWindow )
	{
		if ( !pWindow->ClientToScreen( &sScrPosition, &sScrWindow, false ) )
			return;
		pWindow->VirtualToScreen( &sScrPosition, &sScrWindow );
	}
	else
	{
		CVec2 vScreenRect = pView->GetViewportSize();

		float fXCoef = vScreenRect.x / 1024.0f;
		float fYCoef = vScreenRect.y / 768.0f;
		sScrPosition.x *= fXCoef;
		sScrPosition.y *= fYCoef;
		sScrWindow.x1 *= fXCoef;
		sScrWindow.y1 *= fYCoef;
		sScrWindow.x2 *= fXCoef;
		sScrWindow.y2 *= fYCoef;
	}

	UpdateSize( pView );

	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"CTextDraw::Draw #%d CreateDynamicRects\n",n); fclose(_f);} ++n; } }
	pView->CreateDynamicRects( pText, sScrPosition, sScrWindow );
	{ static int n=0; if(n<3){ FILE* _f=NULL; fopen_s(&_f,"silent_storm_im.log","a");
	  if(_f){fprintf(_f,"CTextDraw::Draw #%d CreateDynamicRects ok\n",n); fclose(_f);} ++n; } }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTextDraw::UpdateSize( NGScene::I2DGameView *pView )
{
	CVec2 vScreenRect = pView->GetViewportSize();
	SPoint sSetSize( sSize );
	if ( sSize.x != -1 )
		sSetSize.x = sSize.x * vScreenRect.x / 1024;
	if ( sSize.y != -1 )
		sSetSize.y = sSize.y * vScreenRect.y / 768;

	pSize.Refresh();
	if ( pSize->GetValue() != sSetSize )
		pSize->Set( sSetSize );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CImageDraw
////////////////////////////////////////////////////////////////////////////////////////////////////
CImageDraw::CImageDraw( const SRect &_sWindow, NDb::CUITexture* _pTexture, const SRect &_sTexRect, const NGfx::SPixel8888 &_sColor ):
	sWindow( _sWindow ), pUITexture(_pTexture), sTextureRect( _sTexRect ), sColor( _sColor ), vScale( 1, 1 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SRect& CImageDraw::GetWindow()
{
	return sWindow;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CImageDraw::SetWindow( const SRect &_sWindow )
{
	sWindow = _sWindow;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CImageDraw::SetScale( const CVec2 &_vScale )
{
	vScale = _vScale;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CImageDraw::SetColor( const NGfx::SPixel8888 &_sColor )
{
	sColor = _sColor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CImageDraw::SetImage( NDb::CUITexture* _pTexture, const SRect &sTexRect )
{
	pUITexture = _pTexture;
	sTextureRect = sTexRect;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CImageDraw::Draw( CWindow *pWindow, const STime &sTime, NGScene::I2DGameView *pView )
{
	CVec2 vScreenRect = pView->GetViewportSize();

	// Phase 1.5 r3 iter 1: emit an [IMG WxH] label into the bgfx dbg overlay
	// to indicate that an image draw was attempted at this position.
	{
		char buf[80];
		int w = sWindow.x2 - sWindow.x1;
		int h = sWindow.y2 - sWindow.y1;
		_snprintf_s(buf, sizeof(buf), _TRUNCATE,
		            "[IMG %dx%d %s]", w, h,
		            IsValid(pUITexture) ? "tex" : "no-tex");
		ss_dbg_text_push(sWindow.x1, sWindow.y1, 0x3f, buf);
		// r3 iter 5: outline rect so the image's footprint is visible too.
		ss_dbg_rect_push(sWindow.x1, sWindow.y1, sWindow.x2, sWindow.y2,
		                 IsValid(pUITexture) ? 0x6020c060u : 0x60606060u);
	}

	SRect sScrWindow( sWindow );
	SPoint sScrPosition( sWindow.x1, sWindow.y1 );
	if ( pWindow )
	{
		if ( !pWindow->ClientToScreen( &sScrPosition, &sScrWindow, false ) )
			return;
		pWindow->VirtualToScreen( &sScrPosition, &sScrWindow );
	}
	else
	{
		float fXCoef = vScreenRect.x / 1024.0f;
		float fYCoef = vScreenRect.y / 768.0f;
		sScrPosition.x *= fXCoef;
		sScrPosition.y *= fYCoef;
		sScrWindow.x1 *= fXCoef;
		sScrWindow.y1 *= fYCoef;
		sScrWindow.x2 *= fXCoef;
		sScrWindow.y2 *= fYCoef;
	}

	if ( IsValid( pUITexture ) )
	{
		NDb::EUIMode eMode;
		CTPoint<float> scale( 1.0f, 1.0f );
		switch( Float2Int( vScreenRect.x ) )
		{
			case 1600:
				if ( IsValid( pUITexture->pTextures[NDb::UIM_1600x1200] ) )
				{
					eMode = NDb::UIM_1600x1200;
					break;
				}
				scale.x *= 1600.0f / 1280.0f;
				scale.y *= 1200.0f / 1024.0f;
			case 1280:
				if ( IsValid( pUITexture->pTextures[NDb::UIM_1280x1024] ) )
				{
					eMode = NDb::UIM_1280x1024;
					break;
				}
				scale.x *= 1280.0f / 1024.0f;
				scale.y *= 1024.0f / 768.0f;
			case 1024:
				if ( IsValid( pUITexture->pTextures[NDb::UIM_1024x768] ) )
				{
					eMode = NDb::UIM_1024x768;
					break;
				}
				scale.x *= 1024.0f / 800.0f;
				scale.y *= 768.0f / 600.0f;
			case 800:
				if ( IsValid( pUITexture->pTextures[NDb::UIM_800x600] ) )
				{
					eMode = NDb::UIM_800x600;
					break;
				}
			default:
				if ( !IsValid( pUITexture->pTextures[NDb::UIM_1024x768] ) )
					return;

				eMode = NDb::UIM_1024x768;
				scale.x = vScreenRect.x / 1024.0f;
				scale.y = vScreenRect.y / 768.0f;
				break;
		}

		NDb::CTexture *pTexture = pUITexture->pTextures[eMode];
		ASSERT( ( pTexture->nWidth != 0 ) && ( pTexture->nHeight != 0 ) );
		if ( ( pTexture->nWidth == 0 ) || ( pTexture->nHeight == 0 ) )
			return;

		CTRect<float> sTexRect( sTextureRect.x1, sTextureRect.y1, sTextureRect.x2, sTextureRect.y2 );
		if ( ( sTexRect.Width() == 0 ) && ( sTexRect.Height() == 0 ) )
		{
			sTexRect.x1 = 0;
			sTexRect.x2 = pTexture->nWidth;
			sTexRect.y1 = pTexture->nHeight;
			sTexRect.y2 = 0;
		}

		CRectLayout sLayout;
		sLayout.scale.x = scale.x * vScale.x;
		sLayout.scale.y = scale.y * vScale.y;
		for ( int nTempY = 0; nTempY < sWindow.Height(); nTempY += pUITexture->nHeight )
			for ( int nTempX = 0; nTempX < sWindow.Width(); nTempX += pUITexture->nWidth )
				sLayout.AddRect( nTempX * sLayout.scale.x, nTempY * sLayout.scale.y, sTexRect, sColor );

		pView->CreateDynamicRects( pTexture, sLayout, sScrPosition, sScrWindow );
	}
	else
	{
		CRectLayout sLayout;
		sLayout.scale.x = pView->GetViewportSize().x / 1024.0f;
		sLayout.scale.y = pView->GetViewportSize().y / 768.0f;
		sLayout.AddRect( 0, 0, CTRect<float>( 0, 0, sWindow.Width(), sWindow.Height() ), sColor );
		pView->CreateDynamicRects( (NDb::CTexture*)0, sLayout, sScrPosition, sScrWindow );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CModelWrap
////////////////////////////////////////////////////////////////////////////////////////////////////
CModelDraw::CModelDraw( const SRect &_sWindow, NDb::CModel* _pModel, CFBTransform *_pBaseTransform ):
	sWindow( _sWindow ), pModel( _pModel ), pBaseTransform( _pBaseTransform ), sColor( 0xFF, 0xFF, 0xFF, 0xFF )
{
	CTransformStack ts;
	ts.Init();
	pTransform = new NGScene::CCFBTransform;
	pTransform->Set( ts.Get() );

	p3DView = NGScene::CreateNewFastInterfaceView();

	SRand rnd;
	NDb::CTAmbientLight *p = NDb::GetTAmbientLight(7);
	p3DView->SetAmbient( p->GetLight( &rnd ), NGScene::IGameView::LT_INVENTORY );
/*
	p3DView = NGScene::CreateNewFastInterfaceView();//CreateNewView();
	p3DView->SetAmbient( 0 );
	pAmbientDirectional = p3DView->AddDirectionalLight( CVec3(0.5f,0.4f,0.45f), CVec3( 0.6f,1.4f,-1), CVec3(5,5,0), CVec2( 150, 150 ), 20 );
	p3DView->SetAmbient( CVec3( 0.5f, 0.5f, 0.5f ) );
*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SRect& CModelDraw::GetWindow() const
{
	return sWindow;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelDraw::SetWindow( const SRect &_sWindow )
{
	sWindow = _sWindow;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const NGfx::SPixel8888& CModelDraw::GetColor() const
{
	return sColor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelDraw::SetColor( const NGfx::SPixel8888 &_sColor )
{
	sColor = _sColor;
	CVec3 vAmbient( 0.5f * float( sColor.r ) / 0xFF, 0.5f * float( sColor.g ) / 0xFF, 0.5f * float( sColor.b ) / 0xFF );
	p3DView->SetAmbient( vAmbient, vAmbient );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CModel* CModelDraw::GetModel() const
{
	return pModel;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelDraw::SetModel( NDb::CModel* _pModel )
{
	pModel = _pModel;
	pRender = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CFBTransform* CModelDraw::GetTransform() const
{
	return pBaseTransform;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelDraw::SetTransform( CFBTransform *_pBaseTransform )
{
	pBaseTransform = _pBaseTransform;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelDraw::Draw( CWindow *pWindow, const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( !pModel )
		return;

	SRect sScrWindow( sWindow );
	SPoint sScrPosition( sWindow.x1, sWindow.y1 );
	if ( pWindow )
	{
		if ( !pWindow->ClientToScreen( &sScrPosition, &sScrWindow, false ) )
			return;
	}

	SPoint sSize( (float)sWindow.Width() * p3DView->GetScreenRect().x / 1024.0f, (float)sWindow.Height() * p3DView->GetScreenRect().y / 768.0f );
	SRect s2DScrWindow( sScrWindow );
	SPoint s2DScrPosition( sScrPosition );
	if ( pWindow )
		pWindow->VirtualToScreen( &s2DScrPosition, &s2DScrWindow );
	else
	{
		CVec2 vScreenRect = pView->GetViewportSize();

		float fXCoef = vScreenRect.x / 1024.0f;
		float fYCoef = vScreenRect.y / 768.0f;
		s2DScrPosition.x *= fXCoef;
		s2DScrPosition.y *= fYCoef;
		s2DScrWindow.x1 *= fXCoef;
		s2DScrWindow.y1 *= fYCoef;
		s2DScrWindow.x2 *= fXCoef;
		s2DScrWindow.y2 *= fYCoef;
	}

	CRectLayout sLayout;
	sLayout.AddRect( 0, 0, CTRect<float>( 0, 0, s2DScrWindow.Width(), s2DScrWindow.Height() ) );
	pView->CreateDynamicClearRects( sLayout, s2DScrPosition, s2DScrWindow, 1.0f );
	pView->Flush();

	if ( !IsValid( pRender ) )
		pRender = p3DView->CreateMesh( pModel, pTransform );

	CVec2 vPos( sScrPosition.x + sWindow.Width() / 2, sScrPosition.y + sWindow.Height() / 2 );
	CTransformStack ts;
	SHMatrix sMatrix;
	if ( pBaseTransform )
		sMatrix = pBaseTransform->pos.forward;
//	MakeMatrix( &sMatrix, ToRadian( 0 ), ToRadian( 90.0f ), CVec3( 10, (float)( 512 - vPos.x ) * 5 / 1024, (float)( vPos.y - 384 ) * 3.75f / 768 ) );
	ts.Init();
	ts.MakeProjective( CVec2( 1024, 768 ), 60, 0.1f, 300 );
	ts.SetCamera( sMatrix );

	SHMatrix sShift;
	Identity( &sShift);
	sShift._14 = (float)( vPos.x - 512 ) / 512;
	sShift._24 = (float)( 384 - vPos.y ) / 384;

	SHMatrix sRes;
	Multiply( &sRes, sShift, ts.Get().forward );
	ts.Init( sRes );

	NGScene::IGameView::SDrawInfo drawInfo;
	drawInfo.pTS = &ts;
	drawInfo.vOrigin = CVec2( sScrPosition.x / 1024.0f, sScrPosition.y / 768.0f );
	drawInfo.vSize = CVec2( sWindow.Width() / 1024.0f, sWindow.Height() / 768.0f );
	drawInfo.bOverlay = true;
	p3DView->Draw( drawInfo );

	pView->CreateDynamicClearRects( sLayout, s2DScrPosition, s2DScrWindow, 0.0f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0814160, CTextDraw )
REGISTER_SAVELOAD_CLASS( 0xB0814161, CImageDraw )
REGISTER_SAVELOAD_CLASS( 0xB0814163, CModelDraw )