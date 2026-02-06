#ifndef __A5_UI_COMMON_CONTROLS_H__
#define __A5_UI_COMMON_CONTROLS_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
	class CCTPoint;
	class CCWString;
	class CBWScreenshotTexture;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class IMLLayout;
class CImageDraw;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Common controls styles
const int
	SLDRSTYLE_HORZ				= 0x00000100,
	SLDRSTYLE_VERT				= 0x00000200;
// Scroll
const int
	SCRLSTYLE_HORZ				= 0x00000100,
	SCRLSTYLE_VERT				= 0x00000200;
// ProgressBar
const int
	PBARSTYLE_HORZ				= 0x00000100,
	PBARSTYLE_VERT				= 0x00000200,
	PBARSTYLE_SCALE				= 0x00000400,
	PBARSTYLE_CENTERED		= 0x00000800;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Common controls events
const int
	EVENT_LISTVIEW_ITEMSELECTED	=	0x00000100 | EVENT_FLAG_NOTIFY;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Common controls flags
const int
// ListView flags
	ELV_ITEMSSELECTED_TRUE		= 0x000000001,
	ELV_ITEMSSELECTED_FALSE		= 0x000000000;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CEdit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CEdit: public CWindow
{
	OBJECT_NOCOPY_METHODS(CEdit);
private:
	ZDATA_(CWindow)
	int nSize;
	int nCursor;
	bool bActiveState;
	bool bCursorVisible;
	SRect sTexRect;
	STime sFlashTime;
	wstring wsText;
	////
	SCursorInfo sCursorInfo;
	CObj<NGScene::CCTPoint> pSize;
	CObj<NGScene::CCWString> pTextString;
	CDGPtr< CFuncBase<NGScene::SText> > pText;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&nSize); f.Add(3,&nCursor); f.Add(4,&bActiveState); f.Add(5,&bCursorVisible); f.Add(6,&sTexRect); f.Add(7,&sFlashTime); f.Add(8,&wsText); f.Add(9,&sCursorInfo); f.Add(10,&pSize); f.Add(11,&pTextString); f.Add(12,&pText); return 0; }
	
public:
	CEdit() {}
	CEdit( const SWindowInfo &sInfo );
	
	void GetText( wstring *pwsString ) const;
	void SetText( const wstring &wsString );
	void SetEditSize( int nSize );
	void SetCursorPosition( int nPos );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CButton
////////////////////////////////////////////////////////////////////////////////////////////////////
class CButton: public CWindow
{
	OBJECT_NOCOPY_METHODS(CButton);
private:
	ZDATA_(CWindow)
	int nState;
	bool bPushed;
	bool bMouseEnter;
	CPtr<CImage> pGlow;
	CPtr<CImage> pMountUp;
	CPtr<CImage> pMountDown;
	CPtr<CImage> pMountDisabled;
	NGfx::SPixel8888 sColor;
	CObj<CObjectBase> pMouseCaptture;
	hash_map<int,CObj<CWindow> > statesMap;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&nState); f.Add(3,&bPushed); f.Add(4,&bMouseEnter); f.Add(5,&pGlow); f.Add(6,&pMountUp); f.Add(7,&pMountDown); f.Add(8,&pMountDisabled); f.Add(9,&sColor); f.Add(10,&pMouseCaptture); f.Add(11,&statesMap); return 0; }

protected:
	virtual void OnAction();

public:
	CButton() {}
	CButton( const SWindowInfo &sInfo );

	const NGfx::SPixel8888& GetColor() const;
	void SetColor( const NGfx::SPixel8888 &sColor );

	CWindow* AddState( int nID );
	void RemoveState( int nID );
	CWindow* GetState( int nID ) const;

	CWindow* AddTextState( int nID, const wstring &wsText );
	CWindow* AddImageState( int nID, NDb::CUITexture *pTexture, const NGfx::SPixel8888 &sColor = NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF ) );

	int GetActiveState();
	void SetActiveState( int nID );

	bool IsPushed() const;
	bool IsMouseCover() const;

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPushButton
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPushButton: public CButton
{
	OBJECT_NOCOPY_METHODS(CPushButton);
private:
	ZDATA_(CButton)
	wstring wsText;
	CObj<CTextDraw> pText;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CButton*)this); f.Add(2,&wsText); f.Add(3,&pText); return 0; }
	
public:
	CPushButton() {}
	CPushButton( const SWindowInfo &sInfo );

	const wstring& GetText() const;
	void SetText( const wstring &wsText );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCheckButton
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCheckButton: public CPushButton
{
	OBJECT_NOCOPY_METHODS(CCheckButton);
private:
	ZDATA_(CPushButton)
	bool bChecked;
	CPtr<CImage> pCheck;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CPushButton*)this); f.Add(2,&bChecked); f.Add(3,&pCheck); return 0; }
	
protected:
	virtual void OnAction();

public:
	CCheckButton() {}
	CCheckButton( const SWindowInfo &sInfo );

	bool IsChecked() const;
	void SetChecked( bool bState );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CToolTip
////////////////////////////////////////////////////////////////////////////////////////////////////
class CToolTip: public CWindow
{
	OBJECT_NOCOPY_METHODS(CToolTip);
private:
	ZDATA_(CWindow)
	CObj<CMLText> pText;
	CObj<CImageDraw> pBackground;
	hash_map<wstring,wstring> valuesMap;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pText); f.Add(3,&pBackground); f.Add(4,&valuesMap); return 0; }

protected:
	void UpdateFormat( const string &szID, CText *pText );

public:
	CToolTip() {}
	CToolTip( const SWindowInfo &sInfo );

	void SetVal( const wstring &szID, int nVal );
	void SetVal( const wstring &szID, float fVal );
	void SetVal( const wstring &szID, const wstring &wsVal );

	void SetText( const wstring &szText );

	void SetPosition( const SPoint &_sPosition );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );

	static void VALUEHandler( IMLLayout *pLayout, const vector<wstring> &paramsSet, void *pContext );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSlider
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSlider: public CWindow
{
	OBJECT_NOCOPY_METHODS(CSlider);
private:
	ZDATA_(CWindow)
	int nValue;
	int nMinValue;
	int nMaxValue;
	bool bSlide;
	CPtr<CWindow> pSlider;
	CObj<CObjectBase> pMouseCapture;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&nValue); f.Add(3,&nMinValue); f.Add(4,&nMaxValue); f.Add(5,&bSlide); f.Add(6,&pSlider); f.Add(7,&pMouseCapture); return 0; }

protected:
	void Slide( int nX, int nY );
	void PageSlide( int nX, int nY );
	virtual void OnAction();

public:
	CSlider() {}
	CSlider( const SWindowInfo &sInfo );

	int GetValue();
	void SetValue( int nVal );
	void SetRange( int nMinValue, int nMaxValue );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScroll
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScroll: public CWindow
{
	OBJECT_NOCOPY_METHODS(CScroll);
private:
	ZDATA_(CWindow)
	CPtr<CSlider> pSlider;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pSlider); return 0; }

protected:
	virtual void OnAction();

public:
	CScroll() {}
	CScroll( const SWindowInfo &sInfo );

	int GetValue();
	void SetValue( int nVal );
	void SetRange( int nMinValue, int nMaxValue );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CListView
////////////////////////////////////////////////////////////////////////////////////////////////////
class CListView: public CWindow
{
	OBJECT_NOCOPY_METHODS(CListView);
private:
	struct SItem
	{
		ZDATA
		int nID;
		CObj<CWindow> pWindow;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nID); f.Add(3,&pWindow); return 0; }

		SItem(): nID( -1 ) {}
		SItem( int _nID, CWindow *_pWindow ): nID( _nID ), pWindow( _pWindow ) {}
	};
	ZDATA_(CWindow)
	SItem sSelected;
	list<SItem> itemsList;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&sSelected); f.Add(3,&itemsList); return 0; }

protected:
	bool FindItemByID( int nID, SItem *pItem ) const;
	bool FindItemByWindow( CWindow *pWindow, SItem *pItem ) const;
	virtual void OnAction();

public:
	CListView() {}
	CListView( const SWindowInfo &sInfo );
	
	void AddItem( int nID, CWindow *pItem );
	void RemoveItem( int nID );
	void RemoveAllItems();
	CWindow* GetItem( int nID ) const;

	int GetSelectedItem() const;
	void SetSelectedItem( int nID );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CComboBox
////////////////////////////////////////////////////////////////////////////////////////////////////
class CComboBox: public CWindow
{
	OBJECT_NOCOPY_METHODS(CComboBox)
public:
	enum EState
	{
		STATE_NORMAL		= 0,
		STATE_HILIGHTED,
		STATE_SELECTED,
		STATE_DISABLED,
		STATE_LAST
	};
	struct SInfo
	{
		ZDATA
		wstring wsText;
		NGfx::SPixel8888 sColor;
		CDBPtr<NDb::CUITexture> pImage;

		SInfo(): sColor( 0, 0, 0, 0 ) {}
		SInfo( const wstring &_wsText, NDb::CUITexture *_pImage = 0, const NGfx::SPixel8888 &_sColor = NGfx::SPixel8888( 0, 0, 0, 0 ) ): wsText( _wsText ), pImage( _pImage ), sColor( _sColor ) {}
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&wsText); f.Add(3,&sColor); f.Add(4,&pImage); return 0; }
	};

private:
	ZDATA_(CWindow)
	CObj<CWindow> pSelected;
	CPtr<CWindow> pSelectedView;
	CObj<CListView> pList;
	CObj<CObjectBase> pMouseCapture;
	vector<SInfo> statesSet;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pSelected); f.Add(3,&pSelectedView); f.Add(4,&pList); f.Add(5,&pMouseCapture); f.Add(6,&statesSet); return 0; }

public:
	CComboBox() {}
	CComboBox( const SWindowInfo &sInfo );

	void AddItem( int nID, const SInfo &sItem, int nTemplate = -1 );
	void RemoveAllItems();
	bool GetItem( int nID, SInfo *pInfo );
	void SetStateInfo( EState eState, const SInfo &sInfo );

	int GetSelectedItem() const;
	void SetSelectedItem( int nID );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CProgressBar
////////////////////////////////////////////////////////////////////////////////////////////////////
class CProgressBar: public CWindow
{
	OBJECT_NOCOPY_METHODS(CProgressBar);
private:
	ZDATA_(CWindow)
	float fValue;
	int nImageWidth;
	CObj<CImageDraw> pImage;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&fValue); f.Add(3,&nImageWidth); f.Add(4,&pImage); return 0; }

public:
	CProgressBar() {}
	CProgressBar( const SWindowInfo &sInfo );
	
	float GetValue();
	void SetValue( float fValue );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScreenShot
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScreenShot: public CWindow
{
	OBJECT_NOCOPY_METHODS(CScreenShot);
private:
	ZDATA_(CWindow)
	CObj<NGScene::CBWScreenshotTexture> pTexture;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pTexture); return 0; }

public:
	CScreenShot() {}
	CScreenShot( const SWindowInfo &sInfo );
	
	void Generate();

	NGScene::CBWScreenshotTexture* GetTexture() const;
	void SetTexture( NGScene::CBWScreenshotTexture* pTexture );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
