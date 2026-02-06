#include "StdAfx.h"
#include "Gfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "RPGGlobal.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "..\DBFormat\DataScenario.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iCluesMenu.h"
#include "UIWrap.h"
#include "scScenarioTracker.h"
#include "scFlowChartItems.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScrollWindow
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CScrollWindow: public CWindow
{
	OBJECT_BASIC_METHODS(CScrollWindow);
private:
	ZDATA_(CWindow)
	CVec2 vValue;
	CObj<TYPE> pScrollWindow;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&vValue); f.Add(3,&pScrollWindow); return 0; }

public:
	CScrollWindow() {}
	CScrollWindow( const SWindowInfo &sInfo ): CWindow( sInfo )
	{
		pScrollWindow = new TYPE( SWindowInfo( this, SPoint( 0, 0 ), GetSize(), GetWindowID(), STYLE_ENABLED | STYLE_VISIBLE ) );
	}

	const CVec2& GetValue() const { return vValue; }
	void SetValue( const CVec2 &_vValue )
	{
		vValue = _vValue;

		const SPoint &sViewSize = GetSize();
		const SPoint &sClientSize = pScrollWindow->GetSize();

		SPoint sDelta( Max( 0, sClientSize.x - sViewSize.x ), Max( 0, sClientSize.y - sViewSize.y ) );
		sDelta.x = -sDelta.x * vValue.x;
		sDelta.y = -sDelta.y * vValue.y;

		pScrollWindow->SetPosition( sDelta );
	}

	TYPE* GetClientWindow() const {	return pScrollWindow; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCluesUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCluesItem: public CButton
{
	OBJECT_NOCOPY_METHODS(CCluesItem)
private:
	ZDATA_(CButton)
	CPtr<NScenario::CScenarioClue> pClue;
	////
	CObj<CMLText> pText;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CButton*)this); f.Add(2,&pClue); f.Add(3,&pText); return 0; }

protected:
	void OnAction();

public:
	CCluesItem() {}
	CCluesItem( const SWindowInfo &sInfo, NScenario::CScenarioClue *_pClue, const wstring &wsText );

	NScenario::CScenarioClue* Get() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CCluesItem::CCluesItem( const SWindowInfo &sInfo, NScenario::CScenarioClue *_pClue, const wstring &wsText ):
	CButton( sInfo ), pClue( _pClue )
{
	pText = new CMLText( SWindowInfo( this, SPoint( 0, 0 ), GetSize(), "iml-text", STYLE_ENABLED | STYLE_VISIBLE ) );
	pText->SetText( wsText );

	SPoint sSize;
	pText->GetRealSize( &sSize );

	sSize.x = GetSize().x;
	SetSize( sSize );
	pText->SetSize( sSize );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NScenario::CScenarioClue* CCluesItem::Get() const
{
	return pClue;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCluesItem::OnAction()
{
	SendMessage( GetParent(), SEvent( EVENT_LISTVIEW_ITEMSELECTED, this ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCluesUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCluesUI: public CWindow
{
	OBJECT_NOCOPY_METHODS(CCluesUI)
private:
	enum EFilter
	{
		ORDER,
		ZONE_CLUES,
		ZONE_POINT
	};
	ZDATA_(CWindow)
	CPtr<NRPG::CGlobalGame> pGame;
	////
	CObj<CMLText> pDescription;
	CObj<CScroll> pDescriptionScroll;
	CObj<CScrollWindow<CMLText> > pDescriptionView;
	////
	CObj<CScroll> pListScroll;
	CObj<CListView> pList;
	CObj<CScrollWindow<CListView> > pListView;
	////
	CPtr<CImage> pSelection;
	CObj<CFlashButton> pCloseButton;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pGame); f.Add(3,&pDescription); f.Add(4,&pDescriptionScroll); f.Add(5,&pDescriptionView); f.Add(6,&pListScroll); f.Add(7,&pList); f.Add(8,&pListView); f.Add(9,&pSelection); f.Add(10,&pCloseButton); return 0; }

protected:
	void GenerateList( EFilter eFilter );

public:
	CCluesUI() {}
	CCluesUI( const SWindowInfo &sInfo,  NRPG::CGlobalGame *_pGame );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CCluesUI::CCluesUI( const SWindowInfo &sInfo,  NRPG::CGlobalGame *_pGame ):
	CWindow( sInfo ), pGame( _pGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CCluesUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "list_scroll" )
			{
				pListView->SetValue( CVec2( 0, float( pListScroll->GetValue() ) / 100 ) );
				return true;
			}
			else if ( sEvent.szID == "descr_scroll" )
			{
				pDescriptionView->SetValue( CVec2( 0, float( pDescriptionScroll->GetValue() ) / 100 ) );
				return true;
			}
			else if ( sEvent.szID == "view" )
			{
				if ( IsValid( pGame->pScenarioTracker ) )
				{
					int nID = pList->GetSelectedItem();
					CPtr<CCluesItem> pItem = dynamic_cast<CCluesItem*>( pList->GetItem( pList->GetSelectedItem() ) );
					if ( IsValid( pItem ) )
					{
						CPtr<NDb::CString> pClueDescription = pGame->pScenarioTracker->GetClueDescriptionFromObjective( pItem->Get() );
						if ( IsValid( pClueDescription ) )
							pDescription->SetText( pClueDescription->szStr );
						else
							pDescription->SetText( L"<color=red>[ERROR]Description not set" );
					}
				}

				return true;
			}

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pCloseButton = new CFlashButton( sEvent.pLoader->GetControl( "cancel" ) );

			pListView = new CScrollWindow<CListView>( sEvent.pLoader->GetControl( "view" ) );
			pList = pListView->GetClientWindow();

			pDescriptionView = new CScrollWindow<CMLText>( sEvent.pLoader->GetControl( "description" ) );
			pDescription = pDescriptionView->GetClientWindow();

			GenerateList( ORDER );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pSelection = GetUIWindow<CImage>( this, "selection" );
			pSelection->SetStyle( STYLE_VISIBLE, false );

			pListScroll = GetUIWindow<CScroll>( this, "list_scroll" );
			pListScroll->SetRange( 0, 100 );

			pDescriptionScroll = GetUIWindow<CScroll>( this, "descr_scroll" );
			pDescriptionScroll->SetRange( 0, 100 );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCluesUI::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	int nID = pList->GetSelectedItem();
	CPtr<CCluesItem> pItem = dynamic_cast<CCluesItem*>( pList->GetItem( pList->GetSelectedItem() ) );
	if ( IsValid( pItem ) )
	{
		const SPoint &sSize = pItem->GetSize();
		SRect sWindow;
		SPoint sPosition;
		if ( pItem->ClientToScreen( &sPosition, &sWindow ) )
		{
			const SPoint &sSelectionSize = pSelection->GetSize();
			const SPoint &sSelectionPosition = pSelection->GetPosition();
			pSelection->SetStyle( STYLE_VISIBLE, true );
			pSelection->SetPosition( SPoint( sSelectionPosition.x, sPosition.y + sSize.y - sSelectionSize.y ) );
		}
		else
			pSelection->SetStyle( STYLE_VISIBLE, false );
	}

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SOrder
{
	bool operator()( NScenario::CScenarioClue *p1, NScenario::CScenarioClue *p2 ) const 
	{ 
		return true; 
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SZone
{
private:
	CPtr<NScenario::CScenarioTracker> pTracker;

public:
	SZone( NScenario::CScenarioTracker *_pTracker ): pTracker( _pTracker ) {}

	bool operator()( NScenario::CScenarioClue *p1, NScenario::CScenarioClue *p2 ) const 
	{
		return true; 
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SZonePoint
{
private:
	CPtr<NScenario::CScenarioTracker> pTracker;

public:
	SZonePoint( NScenario::CScenarioTracker *_pTracker ): pTracker( _pTracker ) {}

	bool operator()( NScenario::CScenarioClue *p1, NScenario::CScenarioClue *p2 ) const 
	{ 
		return true; 
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCluesUI::GenerateList( EFilter eFilter )
{
	pList->RemoveAllItems();

	list<CPtr<NScenario::CScenarioClue> > cluesList;
	pGame->pScenarioTracker->GetAvailableClues( &cluesList );

	switch( eFilter )
	{
	case ORDER:
		cluesList.sort( SOrder() );
		break;
	case ZONE_CLUES:
		cluesList.sort( SZone( pGame->pScenarioTracker ) );
		break;
	case ZONE_POINT:
		cluesList.sort( SZonePoint( pGame->pScenarioTracker ) );
		break;
	}

	int nCount = 0;
	for( list<CPtr<NScenario::CScenarioClue> >::iterator iTemp = cluesList.begin(); iTemp != cluesList.end(); iTemp++ )
	{
		WCHAR wsBuffer[1024];
		swprintf( wsBuffer, L"<font face=Courier size=18pt><color=FF2E4051>%d. %s", nCount + 1, GetDBString( (*iTemp)->GetDBClue()->pDescription ).c_str() );

		wstring wsTemp( wsBuffer );
		pList->AddItem( nCount, new CCluesItem( SWindowInfo( pList, SPoint( 0, 0 ), SPoint( pList->GetSize().x, 0 ), "", STYLE_ENABLED | STYLE_VISIBLE ), (*iTemp), wsTemp ) );
		nCount++;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// COptionsInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCluesInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CCluesInterface);
private:
	NInput::CBind bindExit;

	ZDATA
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	CPtr<NRPG::CGlobalGame> pGame;
	////
	CObj<NUI::CWindow> pCluesUI;
	CObj<NUI::CScreenShot> pScreenShot;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCursor); f.Add(3,&pInterface); f.Add(4,&pGame); f.Add(5,&pCluesUI); f.Add(6,&pScreenShot); return 0; }

public:
	CCluesInterface();

	void Initialize( NRPG::CGlobalGame *pGame );

	void Step();
	void OnGetFocus() {}
	bool ProcessEvent( const NInput::SEvent &eEvent );
	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CCluesInterface::CCluesInterface():
	bindExit( "cancel" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCluesInterface::Initialize( NRPG::CGlobalGame *_pGame )
{
	pGame = _pGame;

	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pCluesUI = new NUI::CCluesUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "clues", NUI::STYLE_ENABLED ), pGame );
	NUI::LoadTemplate( pCluesUI, NDb::GetUIContainer( 181 ) );

	pScreenShot = new NUI::CScreenShot( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "clues", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_BOTTOMMOST ) );
	pScreenShot->Generate();

	pCluesUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCluesInterface::Step()
{
	MarkNewDGFrame();
	if ( CanRender() )
	{
		pInterface->UpdateCursor();
		pInterface->Step( GetTime() );
		RenderFrame();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CCluesInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCluesInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );

	pInterface->Draw( GetTime() );

	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICClues::CICClues( NRPG::CGlobalGame *_pGame ):
	pGame( _pGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICClues::Exec()
{
	CCluesInterface *pRes = new CCluesInterface();
	pRes->Initialize( pGame );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0820130, CCluesUI );
REGISTER_SAVELOAD_CLASS( 0xB0820131, CCluesItem )
REGISTER_SAVELOAD_TEMPL_CLASS( 0xB0820132, CScrollWindow<CMLText>, CScrollWindow );
REGISTER_SAVELOAD_TEMPL_CLASS( 0xB0820133, CScrollWindow<CListView>, CScrollWindow );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB082013A, CCluesInterface );
