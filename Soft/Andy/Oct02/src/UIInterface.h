#ifndef __A5_UI_INTERFACE_H__
#define __A5_UI_INTERFACE_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
	class CString;
}
#include "..\Input\Bind.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWindow;
class CConsole;
class CTextDraw;
class CMouseCaptureHandler;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
wstring GetDBString( int nID );
wstring GetDBString( NDb::CString *pString );
void LoadTemplate( CWindow *pWindow, NDb::CUIContainer *pTemplate );
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLoader
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLoader: public CObjectBase
{
	OBJECT_BASIC_METHODS(CLoader)
private:
	struct SWindow
	{
		SWindowInfo sInfo;
		CPtr<NDb::CUIControl> pControl;
	};
	typedef pair<CPtr<CWindow>,SWindow> TTemplateWindow;

	CPtr<CWindow> pParent;
	vector<TTemplateWindow> windowsSet;

public:
	CLoader() {}

	void Load( CWindow* _pParent, NDb::CUIContainer *pTemplate );

	const SWindowInfo& GetControl( const string &szID );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CInterface: public CWindow
{
	OBJECT_BASIC_METHODS(CInterface);
private:
	NInput::CBind cmdConsole, cmdLButtonDown, cmdLButtonUp, cmdRButtonDown, cmdRButtonUp, cmdFPSShow;

	ZDATA_(CWindow)
	STime sLastLButtonDownTime, sLastRButtonDownTime, sDoubleClickTime;
	SPoint sCursorPoint;
	SCursorInfo sDefaultCursor;
	CTimeCounter sTimer;
	CDGPtr<CCTime> pTimer;
	CPtr<ICursor> pCursor;
	CPtr<CConsole> pConsole;
	CObj<NGScene::I2DGameView> pView;
	CMObj<CMouseCaptureHandler> pMouseCapture;
	////
	CObj<CWindow> pToolTip;
	CPtr<CWindow> pToolTipOwner;
	////
	bool bShowFPSStats;
	CObj<CTextDraw> pFPSText;
	///CRAP
	CObj<CTextDraw> pNonPublicDemo;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&sLastLButtonDownTime); f.Add(3,&sLastRButtonDownTime); f.Add(4,&sDoubleClickTime); f.Add(5,&sCursorPoint); f.Add(6,&sDefaultCursor); f.Add(7,&sTimer); f.Add(8,&pTimer); f.Add(9,&pCursor); f.Add(10,&pConsole); f.Add(11,&pView); f.Add(12,&pMouseCapture); f.Add(13,&pToolTip); f.Add(14,&pToolTipOwner); f.Add(15,&bShowFPSStats); f.Add(16,&pFPSText); f.Add(17,&pNonPublicDemo); return 0; }

protected:
	void UpdateFPSText();

public:
	CInterface();
	CInterface( ICursor* _pCursor );

	const SPoint& GetCursorPos() const;

	const SCursorInfo& GetCursorInfo() const;
	const SCursorInfo& GetDefaultCursorInfo() const;
	void SetCursorInfo( const SCursorInfo &sInfo );

	void SetToolTipOwner( CWindow *pOwner );
	CObjectBase* CreateMouseCapture( CWindow *pWindow );

	NGScene::I2DGameView* GetView();

	bool IsActive() const { return true; };
	bool IsMouseCover() const { return true; };

	bool ProcessEvent( const NInput::SEvent &eEvent );
	bool ProcessMessage( const SEvent &sEvent );
	void UpdateCursor();
	void Step( const STime &sTime );
	void Draw( const STime &sTime );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
