#ifndef __IGLOBAL_SAVELOADUI_H_
#define __IGLOBAL_SAVELOADUI_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGfx
{
	class CTexture;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Interface.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
enum ESaveLoad
{
	SAVE,
	LOAD
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CBackground;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFileList
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFileListItem;
class CFileList: public CDecorator<CWindow>
{
	OBJECT_BASIC_METHODS(CFileList);
private:
	struct SItem
	{
		ZDATA
		string szFileName;
		wstring wsTitle;
		CObj<IWindow> pLine;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&szFileName); f.Add(3,&wsTitle); f.Add(4,&pLine); return 0; }
	};
	ZDATA_(TBaseClass)
	ESaveLoad eMode;
	vector<SItem> itemsSet;
	CObj<IWindow> pEditLine;
	CObj<CBackground> pBackground;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&eMode); f.Add(3,&itemsSet); f.Add(4,&pEditLine); f.Add(5,&pBackground); return 0; }

protected:
	void MakeList();
	
public:
	CFileList() {}
	CFileList( IWindow *pContainer, ESaveLoad eMode );

	void SetMode( ESaveLoad eMode );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const SScene &sScene );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTabControl
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTabControlItem;
class CTabControl: public CDecorator<CWindow>
{
	OBJECT_BASIC_METHODS(CTabControl);
private:
	struct SItem
	{
		ZDATA
		string szID;
		wstring wsTitle;
		CObj<CTabControlItem> pTabItem;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&szID); f.Add(3,&wsTitle); f.Add(4,&pTabItem); return 0; }
	};
	ZDATA_(TBaseClass)
	bool bUpdated;
	vector<SItem> itemsSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&bUpdated); f.Add(3,&itemsSet); return 0; }

public:
	CTabControl() {}
	CTabControl( IWindow *pContainer );

	void AddTab( const wstring &wsText, const string &szID );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const SScene &sScene );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSaveLoadUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSaveLoadUI: public CDecorator<CWindow>
{
	OBJECT_BASIC_METHODS(CSaveLoadUI);
private:
	ZDATA_(TBaseClass)
	ESaveLoad eMode;
	CPtr<CFileList> pList;
	CPtr<CTabControl> pTabControl;
	CPtr<CBackground> pBackground;
	CDGPtr<CPtrFuncBase<NGfx::CTexture> > pBackgroundTexture;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&eMode); f.Add(3,&pList); f.Add(4,&pTabControl); f.Add(5,&pBackground); f.Add(6,&pBackgroundTexture); return 0; }

protected:
	void DrawBackground( const SScene &sScene );

public:
	CSaveLoadUI() {}
	CSaveLoadUI( IWindow *pContainer, ESaveLoad eMode, CPtrFuncBase<NGfx::CTexture> *_pBackgroundTexture );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const SScene &sScene );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iMain.h"
#include "..\Input\Bind.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSaveLoad: public NMainLoop::CInterfaceBase
{
	OBJECT_BASIC_METHODS(CSaveLoad);
private:
	NInput::CBind bindExit, bindSaveLoad;

	ZDATA_(NMainLoop::CInterfaceBase)
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::IInterface> pInterface;
	CObj<NUI::IWindow> pSaveLoad;
	CObj<NUI::CSaveLoadUI> pSaveLoadCtrl;
	CDGPtr<CPtrFuncBase<NGfx::CTexture> > pBackgroundTexture;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(NMainLoop::CInterfaceBase*)this); f.Add(2,&pCursor); f.Add(3,&pInterface); f.Add(4,&pSaveLoad); f.Add(5,&pSaveLoadCtrl); f.Add(6,&pBackgroundTexture); return 0; }

public:
	CSaveLoad();

	void Initialize( ESaveLoad eMode );
	void Terminate();
	void Step();
	bool ProcessEvent( const NInput::SEvent &eEvent );

	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
