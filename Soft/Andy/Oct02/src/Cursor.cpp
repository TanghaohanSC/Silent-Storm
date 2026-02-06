#include "StdAfx.h"
#include "Gfx.h"
#include "GSceneUtils.h"
#include "G2DView.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "Interface.h"
#include "UIWrap.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGfx
{
	HWND GetHWND();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
static CVec2 vCursorPos = CVec2( 0, 0 );
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCursor
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCursor: public ICursor
{
	OBJECT_BASIC_METHODS(CCursor);
protected:
	NInput::CBind bindX, bindY;
	ZDATA
	bool bShow;
	float fThreshold1, fThreshold2, fAcceleration;
	STime sLastUpdateTime;
	SCursorInfo sInfo;
	CTimeCounter sTimer;
	CDGPtr<CCTime> pTimer;
	CObj<CImageDraw> pImage;
	CObj<CTextDraw> pText;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bShow); f.Add(3,&fThreshold1); f.Add(4,&fThreshold2); f.Add(5,&fAcceleration); f.Add(6,&sLastUpdateTime); f.Add(7,&sInfo); f.Add(8,&sTimer); f.Add(9,&pTimer); f.Add(10,&pImage); f.Add(11,&pText); return 0; }

protected:
	float AccelerateAxis( float fDelta, const STime &sDelta );

public:
	CCursor( bool bShow = true );

	const CVec2& GetPos() const;
	void SetPos( const CVec2 &vCursorPos );
	
	const SCursorInfo& GetCursor() const;
	void SetCursor( const SCursorInfo &sInfo );

	void SetCursorText( const wstring &wsText );

	void Update();

	void ProcessEvent( const NInput::SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
ICursor* ICursor::Create( bool bShowCursor, CVec2 vBegPos )
{
	if ( ( vBegPos.x > 0 ) && ( vBegPos.y > 0 ) )
		vCursorPos = vBegPos;

	return new CCursor( bShowCursor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCursor
////////////////////////////////////////////////////////////////////////////////////////////////////
CCursor::CCursor( bool _bShow ): 
	bindX( "cursor_x" ), bindY( "cursor_y" ), bShow(_bShow)
{
	pText = new CTextDraw();
	pImage = new CImageDraw();
	pTimer = sTimer.GetTime();

	DWORD pdwParams[3];
	SystemParametersInfo( SPI_GETMOUSE, 0, pdwParams, 0 );

	fThreshold1 = pdwParams[0];
	fThreshold2 = pdwParams[1];
	fAcceleration = pdwParams[2];

	if ( fAcceleration == 0 ) /// CRAP: WinME WTF ?
		fAcceleration = 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const CVec2& CCursor::GetPos() const
{
	return vCursorPos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCursor::SetPos( const CVec2 &_vCursorPos )
{
	vCursorPos = _vCursorPos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SCursorInfo& CCursor::GetCursor() const
{
	return sInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCursor::SetCursor( const SCursorInfo &_sInfo )
{
	sInfo = _sInfo;
	pText->SetText( sInfo.wsText );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CCursor::AccelerateAxis( float fDelta, const STime &sDelta )
{
	float fSecondDelta = fabs( fDelta ) * 1000 / sDelta;

	float fInc = fDelta;
	if ( fSecondDelta > fThreshold1 )
		fInc *= fAcceleration;
	if ( fSecondDelta > fThreshold2 )
		fInc *= 2;

	return fInc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCursor::Update()
{
	sTimer.Advance( true, GetTickCount() );
	pTimer.Refresh();
	STime sDelta = pTimer->GetValue() - sLastUpdateTime;
	sLastUpdateTime = pTimer->GetValue();
	if ( sDelta == 0 )
		return;

	const CVec2 &vSize = NGfx::GetScreenRect();
	vCursorPos.x += AccelerateAxis( bindX.GetDelta() * 250.0f, sDelta );
	vCursorPos.y += AccelerateAxis( bindY.GetDelta() * 250.0f, sDelta );

	vCursorPos.x = Max( vCursorPos.x, 0.0f );
	vCursorPos.x = Min( vCursorPos.x, vSize.x - 1 ); 
	vCursorPos.y = Max( vCursorPos.y, 0.0f );
	vCursorPos.y = Min( vCursorPos.y, vSize.y - 1 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCursor::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( sInfo.pTexture && bShow )
	{
		CVec2 vVirtCursorPos( vCursorPos.x * 1024.0f / pView->GetViewportSize().x, vCursorPos.y * 768.0f / pView->GetViewportSize().y );
		SPoint sPos( vVirtCursorPos.x - sInfo.vCenter.x * sInfo.pTexture->nWidth, vVirtCursorPos.y - sInfo.vCenter.y * sInfo.pTexture->nHeight );
		pImage->SetWindow( SRect( sPos.x, sPos.y, sPos.x + sInfo.pTexture->nWidth, sPos.y + sInfo.pTexture->nHeight ) );
		pImage->SetImage( sInfo.pTexture );
		pImage->Draw( 0, sTime, pView );

		pText->SetPosition( SPoint( sPos.x + sInfo.pTexture->nWidth, sPos.y ) );
		pText->Draw( 0, sTime, pView );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCursor::ProcessEvent( const NInput::SEvent &sEvent )
{
	bindX.ProcessEvent( sEvent );
	bindY.ProcessEvent( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class CEditorCursor: public CCursor
{
	OBJECT_BASIC_METHODS(CEditorCursor);
public:
	void ProcessEvent( const NInput::SEvent &eEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CEditorCursor::ProcessEvent( const NInput::SEvent &eEvent )
{
	POINT sPoint;
	GetCursorPos( &sPoint );
	ScreenToClient( NGfx::GetHWND(), &sPoint );

	CVec2 scrSize = NGfx::GetScreenRect();
	vCursorPos.x = sPoint.x;
	vCursorPos.y = sPoint.y;
	vCursorPos.x = Max( vCursorPos.x, 0.0f );
	vCursorPos.x = Min( vCursorPos.x, scrSize.x ); 
	vCursorPos.y = Max( vCursorPos.y, 0.0f );
	vCursorPos.y = Min( vCursorPos.y, scrSize.y );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ICursor* ICursor::CreateEditorCursor()
{
	return new CEditorCursor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0x00821183, CCursor );
REGISTER_SAVELOAD_CLASS( 0xA2812160, CEditorCursor );
////////////////////////////////////////////////////////////////////////////////////////////////////
