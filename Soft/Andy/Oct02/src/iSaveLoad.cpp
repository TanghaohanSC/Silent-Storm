#include "StdAfx.h"
#include "iSaveLoadIC.h"
/*
#include "GSceneUtils.h"
#include "RectLayout.h"
#include "GView.h"
#include "G2DView.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataFormat.h"
#include "iSaveLoad.h"
#include "ScreenShot.h"
#include <io.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
/*
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFileListItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFileListItem: public CDecorator<IWindow>
{
	OBJECT_BASIC_METHODS(CFileListItem);
private:
	ZDATA_(TBaseClass)
	wstring wsText;
	CPtr<IText> pText;
	CPtr<IImage> pSelection;
	CObj<CObjectBase> pMouseCapture;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&wsText); f.Add(3,&pText); f.Add(4,&pSelection); f.Add(5,&pMouseCapture); return 0; }

public:
	CFileListItem() {}
	CFileListItem( IWindow *pContainer, const wstring &_wsText );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CFileListItem::CFileListItem( IWindow *pContainer, const wstring &_wsText ):
	TBaseClass( pContainer ), wsText( _wsText )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFileListItem::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_LBUTTONDOWN:
		{
			return true;
		}
		case EVENT_LBUTTONUP:
		{
			SendMessage( GetParent(), SEvent( EVENT_NOTIFY, GetWindowID() ) );
			return true;
		}
		case EVENT_MOUSEMOVE:
		{
			if ( HitTest( sEvent.nX, sEvent.nY) )
			{
				if ( !IsValid( pMouseCapture ) )
					pMouseCapture = GetInterface()->CreateMouseCapture( this );

				pSelection->SetStyle( STYLE_VISIBLE, true );
			}

			break;
		}
		case EVENT_MOUSEEXIT:
		case EVENT_MOUSECAPTURELOSE:
		{
			pMouseCapture = 0;
			pSelection->SetStyle( STYLE_VISIBLE, false );
			break;
		}
		case EVENT_TEMPLATELOAD:
		{
			pText = GetUIWindow<IText>( this, "text", sEvent.pLoader );
			pSelection = GetUIWindow<IImage>( this, "selection", sEvent.pLoader );

			pText->SetText( wsText );
			pSelection->SetColor( NGfx::SPixel8888( 0x7C, 0x7C, 0xFF, 0x7C ) );
			pSelection->SetStyle( STYLE_VISIBLE, false );
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFileListItemEdit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFileListItemEdit: public CDecorator<IWindow>
{
	OBJECT_BASIC_METHODS(CFileListItemEdit);
private:
	ZDATA_(TBaseClass)
	wstring wsText;
	CPtr<IEdit> pEdit;
	CPtr<IImage> pSelection;
	CObj<CObjectBase> pMouseCapture;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&wsText); f.Add(3,&pEdit); f.Add(4,&pSelection); f.Add(5,&pMouseCapture); return 0; }

public:
	CFileListItemEdit() {}
	CFileListItemEdit( IWindow *pContainer, const wstring &_wsText );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CFileListItemEdit::CFileListItemEdit( IWindow *pContainer, const wstring &_wsText ):
	TBaseClass( pContainer ), wsText( _wsText )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFileListItemEdit::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_NOTIFY:
		{
			wstring wsEditText;
			pEdit->GetText( &wsEditText );
			SendMessage( GetParent(), SEvent( EVENT_NOTIFY, NStr::ToAscii( wsEditText ) ) );
			return true;
		}
		case EVENT_MOUSEMOVE:
		{
			if ( HitTest( sEvent.nX, sEvent.nY) )
			{
				if ( !IsValid( pMouseCapture ) )
					pMouseCapture = GetInterface()->CreateMouseCapture( this );

				pSelection->SetStyle( STYLE_VISIBLE, true );
			}

			break;
		}
		case EVENT_MOUSEEXIT:
		case EVENT_MOUSECAPTURELOSE:
		{
			pMouseCapture = 0;
			pSelection->SetStyle( STYLE_VISIBLE, false );
			break;
		}
		case EVENT_TEMPLATELOAD:
		{
			pEdit = GetUIWindow<IEdit>( this, "edit", sEvent.pLoader );
			pSelection = GetUIWindow<IImage>( this, "selection", sEvent.pLoader );

			pEdit->SetText( wsText );
			pSelection->SetColor( NGfx::SPixel8888( 0x7C, 0x7C, 0xFF, 0x7C ) );
			pSelection->SetStyle( STYLE_VISIBLE, false );
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFileList
////////////////////////////////////////////////////////////////////////////////////////////////////
CFileList::CFileList( IWindow *pContainer, ESaveLoad _eMode ):
	TBaseClass( pContainer ), eMode( _eMode )
{
	_finddata_t sFindData;
	itemsSet.reserve( 12 );
	int nHandle = _findfirst( "*.sav", &sFindData );

	int nRet = nHandle;
	while ( nRet != -1 )
	{
		SItem &sItem = *itemsSet.insert( itemsSet.end() );

		sItem.szFileName = sFindData.name;
		sItem.wsTitle = NStr::ToUnicode( sItem.szFileName );

		nRet = _findnext( nHandle, &sFindData );
	}

	_findclose( nHandle );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileList::SetMode( ESaveLoad _eMode )
{
	eMode = _eMode;
	MakeList();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileList::MakeList()
{
	int nY = 0;
	if ( eMode == SAVE )
	{
		CPtr<IWindow> pWindow = IWindow::Create( this, SRect( 0, nY, 0, nY ), "edit", STYLE_ENABLED | STYLE_VISIBLE );
		pEditLine = new CFileListItemEdit( pWindow, L"newsave.sav" );
		LoadTemplate( pEditLine, NDb::GetUIContainer( 97 ) );

		nY += pEditLine->GetSize().y;
	}
	else
		pEditLine = 0;

	for ( int nTemp = 0; nTemp < itemsSet.size(); nTemp++ )
	{
		CPtr<IWindow> pWindow = IWindow::Create( this, SRect( 0, nY, 0, nY ), itemsSet[nTemp].szFileName, STYLE_ENABLED | STYLE_VISIBLE );

		if ( eMode == LOAD )
		{
			itemsSet[nTemp].pLine = new CFileListItem( pWindow, itemsSet[nTemp].wsTitle );
			LoadTemplate( itemsSet[nTemp].pLine, NDb::GetUIContainer( 90 ) );
		}
		else
		{
			itemsSet[nTemp].pLine = new CFileListItem( pWindow, itemsSet[nTemp].wsTitle );
			LoadTemplate( itemsSet[nTemp].pLine, NDb::GetUIContainer( 90 ) );
		}
		itemsSet[nTemp].pLine->ShowWindow( SWTYPE_SHOW );

		nY += itemsSet[nTemp].pLine->GetSize().y;
	}

	if ( eMode == SAVE )
		pEditLine->ShowWindow( SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFileList::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_NOTIFY:
		{
			if ( eMode == SAVE )
			{
				NMainLoop::Command( new NMainLoop::CICExitModal() );
				NMainLoop::Command( new NMainLoop::CICSave( sEvent.szID.c_str() ) );
			}
			else
				NMainLoop::Command( new NMainLoop::CICLoad( sEvent.szID.c_str() ) );

			break;
		}
		case EVENT_TEMPLATELOAD:
		{
			pBackground = new CBackground( GetUIWindow<IWindow>( this, "background", sEvent.pLoader ), F_BASE_TRANSPARENCY );
			pBackground->SetStyle( STYLE_TRANSPARENT, true );

			MakeList();

			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileList::Draw( const SScene &sScene )
{
	TBaseClass::Draw( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTabControlItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTabControlItem: public CDecorator<IWindow>
{
	OBJECT_BASIC_METHODS(CTabControlItem);
private:
	ZDATA_(TBaseClass)
	wstring wsText;
	CPtr<IText> pText;
	CPtr<CBackground> pActiveBackground;
	CPtr<CBackground> pNotActiveBackground;
	CObj<CObjectBase> pMouseCapture;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&wsText); f.Add(3,&pText); f.Add(4,&pActiveBackground); f.Add(5,&pNotActiveBackground); f.Add(6,&pMouseCapture); return 0; }

public:
	CTabControlItem() {}
	CTabControlItem( IWindow *pContainer, const wstring &_wsText );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const SScene &sScene );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CTabControlItem::CTabControlItem( IWindow *pContainer, const wstring &_wsText ):
	TBaseClass( pContainer ), wsText( _wsText )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTabControlItem::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_LBUTTONDOWN:
		{
			break;
		}
		case EVENT_LBUTTONUP:
		{
			SendMessage( GetParent(), SEvent( EVENT_NOTIFY, GetWindowID() ) );
			break;
		}
		case EVENT_MOUSEMOVE:
		{
			if ( HitTest( sEvent.nX, sEvent.nY) )
			{
				if ( !IsValid( pMouseCapture ) )
					pMouseCapture = GetInterface()->CreateMouseCapture( this );

//				pSelection->SetStyle( STYLE_VISIBLE, true );
			}

			break;
		}
		case EVENT_MOUSEEXIT:
		case EVENT_MOUSECAPTURELOSE:
		{
			pMouseCapture = 0;
//			pSelection->SetStyle( STYLE_VISIBLE, false );
			break;
		}
		case EVENT_TEMPLATELOAD:
		{
			pText = GetUIWindow<IText>( this, "text", sEvent.pLoader );
			pActiveBackground = new CBackground( GetUIWindow<IWindow>( this, "active_background", sEvent.pLoader ), F_BASE_TRANSPARENCY );
			pNotActiveBackground = new CBackground( GetUIWindow<IWindow>( this, "nonactive_background", sEvent.pLoader ), F_BASE_TRANSPARENCY / 2 );

			pText->SetText( wsText );
			pActiveBackground->SetStyle( STYLE_VISIBLE, false );
			pActiveBackground->SetStyle( STYLE_TRANSPARENT, true );
			pNotActiveBackground->SetStyle( STYLE_VISIBLE, false );
			pNotActiveBackground->SetStyle( STYLE_TRANSPARENT, true );
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTabControlItem::Draw( const SScene &sScene )
{
	if ( IsActive() )
	{
		pActiveBackground->SetStyle( STYLE_VISIBLE, true );
		pNotActiveBackground->SetStyle( STYLE_VISIBLE, false );
	}
	else
	{
		pActiveBackground->SetStyle( STYLE_VISIBLE, false );
		pNotActiveBackground->SetStyle( STYLE_VISIBLE, true );
	}

	TBaseClass::Draw( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTabControl
////////////////////////////////////////////////////////////////////////////////////////////////////
CTabControl::CTabControl( IWindow *pContainer ):
	TBaseClass( pContainer )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTabControl::AddTab( const wstring &wsText, const string &szID )
{
	SItem &sItem = *itemsSet.insert( itemsSet.end() );

	sItem.szID = szID;
	sItem.wsTitle = wsText;

	bUpdated = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTabControl::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOAD:
		{
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTabControl::Draw( const SScene &sScene )
{
	if ( bUpdated )
	{
		int nX = 0;
		for ( int nTemp = 0; nTemp < itemsSet.size(); nTemp++ )
		{
			CPtr<IWindow> pWindow = IWindow::Create( this, SRect( nX, 0, nX, 0 ), itemsSet[nTemp].szID, STYLE_ENABLED | STYLE_VISIBLE );
			itemsSet[nTemp].pTabItem = new CTabControlItem( pWindow, itemsSet[nTemp].wsTitle );
			LoadTemplate( itemsSet[nTemp].pTabItem, NDb::GetUIContainer( 95 ) );

			nX += itemsSet[nTemp].pTabItem->GetSize().x;
		}

		bUpdated = false;
	}
	
	TBaseClass::Draw( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSaveLoadUI
////////////////////////////////////////////////////////////////////////////////////////////////////
CSaveLoadUI::CSaveLoadUI( IWindow *pContainer, ESaveLoad _eMode, CPtrFuncBase<NGfx::CTexture> *_pBackgroundTexture ):
	TBaseClass( pContainer ), eMode( _eMode ), pBackgroundTexture( _pBackgroundTexture )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSaveLoadUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "wnd_save" )
			{
				eMode = SAVE;
				pList->SetMode( eMode );
			}
			else if ( sEvent.szID == "wnd_load" )
			{
				eMode = LOAD;
				pList->SetMode( eMode );
			}
			break;
		}
		case EVENT_TEMPLATELOAD:
		{
			pList = new CFileList( GetUIWindow<IWindow>( this, "list", sEvent.pLoader ), eMode );
			pTabControl = new CTabControl( GetUIWindow<IWindow>( this, "tabcontrol", sEvent.pLoader ) );

			if ( eMode == SAVE )
			{
				pTabControl->AddTab( L"<font face=Courier size=18pt><center>Save", "wnd_save" );
				pTabControl->AddTab( L"<font face=Courier size=18pt><center>Load", "wnd_load" );
			}
			else
			{
				pTabControl->AddTab( L"<font face=Courier size=18pt><center>Load", "wnd_load" );
				pTabControl->AddTab( L"<font face=Courier size=18pt><center>Save", "wnd_save" );
			}

			pBackground = new CBackground( GetUIWindow<IWindow>( this, "background", sEvent.pLoader ), F_BASE_TRANSPARENCY );
			pBackground->SetStyle( STYLE_TRANSPARENT, true );

			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSaveLoadUI::Draw( const SScene &sScene )
{
	DrawBackground( sScene );
	TBaseClass::Draw( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSaveLoadUI::DrawBackground( const SScene &sScene )
{
	SRect sScrWindow;
	SPoint sScrPosition;
	if ( !ClientToScreen( &sScrPosition, &sScrWindow ) )
		return;
	VirtualToScreen( sScene, &sScrPosition, &sScrWindow );

	CRectLayout sLayout;
	sLayout.scale = CTPoint<float>( sScene.pView->GetViewportSize().x / 512.0f, sScene.pView->GetViewportSize().y / 512.0f );
	sLayout.AddRect( 0, 0, CTRect<float>( 0, 0, 512, 512 ) );
	sScene.pView->CreateDynamicRects( pBackgroundTexture, sLayout, sScrPosition, sScrWindow );
}
* /
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMissionInventory
////////////////////////////////////////////////////////////////////////////////////////////////////
CSaveLoad::CSaveLoad():	bindExit( "cancel" ), bindSaveLoad( "saveload" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSaveLoad::Initialize( ESaveLoad eMode )
{
	NUI::ESaveLoad eUIMode = NUI::LOAD;
	if ( eMode == TYPE_LOAD )
		eUIMode = NUI::LOAD;
	if ( eMode == TYPE_SAVE )
		eUIMode = NUI::SAVE;

	pCursor = NUI::ICursor::Create();
	pInterface = NUI::IInterface::Create( pCursor );

	pBackgroundTexture = new NGScene::CScreenshotTexture;
	pBackgroundTexture.Refresh();
/ *
	pSaveLoad = NUI::IWindow::Create( pInterface, NUI::SRect( 0, 0, 1024, 768 ), "saveload" );
	pSaveLoadCtrl = new NUI::CSaveLoadUI( pSaveLoad, eUIMode, pBackgroundTexture );
	NUI::LoadTemplate( pSaveLoadCtrl, NDb::GetUIContainer( 87 ) );
	pSaveLoadCtrl->ShowWindow( NUI::SWTYPE_SHOW );
* /
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSaveLoad::Terminate()
{
	NMainLoop::Command( new NMainLoop::CICExitModal() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSaveLoad::Step()
{
	if ( CanRender() )
	{
		pInterface->UpdateCursor();
		pInterface->Step( GetTime() );
		RenderFrame();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSaveLoad::ProcessEvent( const NInput::SEvent &sEvent )
{
	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) || bindSaveLoad.ProcessEvent( sEvent ) )
	{
		Terminate();
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSaveLoad::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );

	pInterface->Draw( GetTime() );

	NGScene::Flip();
	MarkNewDGFrame();
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICSaveLoad::CICSaveLoad( ESaveLoad _eMode ):
	eMode( _eMode )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICSaveLoad::Exec()
{
//	CSaveLoad *pRes = new CSaveLoad();
//	pRes->Initialize( eMode );
//	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
//using namespace NUI;
/*
REGISTER_SAVELOAD_CLASS( 0xB0412100, CSaveLoadUI );
REGISTER_SAVELOAD_CLASS( 0xB0412101, CFileList );
REGISTER_SAVELOAD_CLASS( 0xB0412102, CFileListItem );
REGISTER_SAVELOAD_CLASS( 0xB0412103, CFileListItemEdit );
REGISTER_SAVELOAD_CLASS( 0xB0412104, CTabControl );
REGISTER_SAVELOAD_CLASS( 0xB0412105, CTabControlItem );
*/
//using namespace NGame;
//REGISTER_SAVELOAD_CLASS( 0xB041210F, CSaveLoad );
