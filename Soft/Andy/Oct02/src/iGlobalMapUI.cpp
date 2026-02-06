#include "StdAfx.h"
#include "GView.h"
#include "G2DView.h"
#include "Transform.h"
#include "GSceneUtils.h"
#include "MemObject.h"
#include "PolyUtils.h"
#include "RPGGlobal.h"
#include "ChapterInfo.h"
#include "Sound.h"
#include "Interface.h"
#include "iMission.h"
#include "iGlobalMap.h"
#include "iChapterMap.h"
#include "iGlobalMapUI.h"
#include "..\Misc\BasicShare.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "scFlowChartItems.h"
#include "scScenarioTracker.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
extern CBasicShare<int, CChapterInfoLoader> shareChapterInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_ZONEFLASH_FLASHTIME = 500,
	N_ZONEFLASH_MORPHTIME = N_ZONEFLASH_FLASHTIME / 1.5f;
////////////////////////////////////////////////////////////////////////////////////////////////////
// ISector
////////////////////////////////////////////////////////////////////////////////////////////////////
class IGlobalSector: public CObjectBase
{
public:
	virtual bool HitTest( int nX, int nY ) = 0;

	virtual bool IsSelected() const = 0;
	virtual void SetSelected( bool bState ) = 0;

	virtual void SetDescription( CWindow *pPanel ) = 0;

	virtual const SGlobalSector& GetSector() const = 0;

	virtual void Update( const STime &sTime )= 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalSector
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGlobalSector: public IGlobalSector
{
	OBJECT_BASIC_METHODS(CGlobalSector)
private:
	ZDATA
	CPtr<CWindow> pMapView;
	SGlobalSector sSector;
	CPtr<NGScene::IGameView> pView;
	CPtr<NGame::IGlobalMap> pGlobal;
	////
	bool bSelected;
	bool bRecommended;
	////
	float fCoeff;
	STime sMorphTime;
	STime sFlashTime;
	////
	CObj<CImage> pMarker;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pMapView); f.Add(3,&sSector); f.Add(4,&pView); f.Add(5,&pGlobal); f.Add(6,&bSelected); f.Add(7,&bRecommended); f.Add(8,&fCoeff); f.Add(9,&sMorphTime); f.Add(10,&sFlashTime); f.Add(11,&pMarker); return 0; }

protected:
	void UpdateSector();

public:
	CGlobalSector() {}
	CGlobalSector( NGame::IGlobalMap *pGlobal, CWindow *pMapView, const SGlobalSector &sSector, NGScene::IGameView *pView, bool bRecommended );

	bool HitTest( int nX, int nY );

	bool IsSelected() const;
	void SetSelected( bool bState );

	void SetDescription( CWindow *pPanel );

	const SGlobalSector& GetSector() const;

	void Update( const STime &sTime );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CGlobalSector::CGlobalSector( NGame::IGlobalMap *_pGlobal, CWindow *_pMapView, const SGlobalSector &_sSector, NGScene::IGameView *_pView, bool _bRecommended ):
	pGlobal( _pGlobal ), pMapView( _pMapView ), sSector( _sSector ), pView( _pView ), bRecommended( _bRecommended ), bSelected( false ), fCoeff( 0 ), sFlashTime( 0 ), sMorphTime( 0 )
{
	SPoint sMarkerPos( 0, 0 ), sMarkerSize( 0, 0 );

	pMapView->ScreenToClient( SPoint( sSector.vImagePos.x, sSector.vImagePos.y ), &sMarkerPos );
	CPtr<NDb::CUITexture> pMarkerTexture = NDb::GetUITexture( sSector.nImageID );
	if ( IsValid( pMarkerTexture ) )
		sMarkerSize = SPoint( pMarkerTexture->nWidth, pMarkerTexture->nHeight );

	pMarker = new CImage( SWindowInfo( pMapView, sMarkerPos, sMarkerSize, "", STYLE_ENABLED | STYLE_VISIBLE | STYLE_NOACTIVATE ) );
	pMarker->SetImage( pMarkerTexture );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGlobalSector::HitTest( int nX, int nY )
{
	return IsPointInPolygon( sSector.pointsSet, CVec2( nX, nY ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGlobalSector::IsSelected() const
{
	return bSelected;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalSector::SetSelected( bool bState )
{
	if ( bSelected == bState )
		return;

	bSelected = bState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalSector::SetDescription( CWindow *pPanel )
{
	CPtr<CText> pText = GetUIWindow<CText>( pPanel, "text" );

	if ( sSector.nDescriptionID == -1 )
	{
		pText->SetText( L"<color=red>ERROR: DESCRIPTION NOT SET" );
		return;
	}

	pText->SetText( GetDBString( sSector.nDescriptionID ) );
	pPanel->SetStyle( STYLE_VISIBLE, true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SGlobalSector& CGlobalSector::GetSector() const
{
	return sSector;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalSector::Update( const STime &sTime )
{
	float fTargetCoeff = 0;
	if ( !bSelected && bRecommended )
	{
		fTargetCoeff = float( ( sTime - sFlashTime ) % ( N_ZONEFLASH_FLASHTIME * 2 ) ) / N_ZONEFLASH_FLASHTIME;
		if ( fTargetCoeff > 1 )
			fTargetCoeff = 2 - fTargetCoeff;

		fTargetCoeff = fTargetCoeff * 0.5f + 0.3f;
	}
	else
	{
		sFlashTime = sTime - N_ZONEFLASH_FLASHTIME;
		fTargetCoeff = bSelected ? 1.0f : 0.3f;
	}

	float fDelta = float( sTime - sMorphTime ) / N_ZONEFLASH_MORPHTIME;
	if ( fCoeff < fTargetCoeff )
	{
		fCoeff += fDelta;
		if ( fCoeff > fTargetCoeff )
			fCoeff = fTargetCoeff;
	}
	else if ( fCoeff > fTargetCoeff )
	{
		fCoeff -= fDelta;
		if ( fCoeff < fTargetCoeff )
			fCoeff = fTargetCoeff;
	}

	sMorphTime = sTime;

	pMarker->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fCoeff ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalMapUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class C3DWindow: public CWindow
{
	OBJECT_NOCOPY_METHODS(C3DWindow);
private:
	ZDATA_(CWindow)
	CObj<NGScene::IGameView> pScene;
	CObj<NGScene::ILight> pAmbientDirectional;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pScene); f.Add(3,&pAmbientDirectional); return 0; }

public:
	C3DWindow() {}
	C3DWindow( const SWindowInfo &sInfo );

	NGScene::IGameView* GetView() const;

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
C3DWindow::C3DWindow( const SWindowInfo &sInfo ):
	CWindow( sInfo )
{
	pScene = NGScene::CreateNewFastInterfaceView();//CreateNewView();
	pScene->SetAmbient( 0 );
	pAmbientDirectional = pScene->AddDirectionalLight( CVec3(0.5f,0.4f,0.45f), CVec3( 0.6f,1.4f,-1), CVec3(5,5,0), CVec2( 150, 150 ), 20 );
	pScene->SetAmbient( CVec3( 0.5f, 0.5f, 0.5f ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NGScene::IGameView* C3DWindow::GetView() const
{
	return pScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void C3DWindow::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	CRectLayout sLayout;
	sLayout.AddRect( 0, 0, CTRect<float>( 0, 0, 1024, 768 ) );
	pView->CreateDynamicClearRects( sLayout, SPoint( 0, 0 ), SRect( 0, 0, 1024, 768 ) );
	pView->Flush();

	CTransformStack ts;
	ts.MakeDirect( CVec2( 1024, 768 ) );

	NGScene::IGameView::SDrawInfo drawInfo;
	drawInfo.pTS = &ts;
	drawInfo.vOrigin = CVec2( 0, 0 );
	drawInfo.vSize = CVec2( 1, 1 );
	drawInfo.bOverlay = true;
	pScene->Draw( drawInfo );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalMapUI
////////////////////////////////////////////////////////////////////////////////////////////////////
CGlobalMapUI::CGlobalMapUI( const SWindowInfo &sInfo, NGame::IGlobalMap *_pGlobal ):
	CWindow( sInfo ), pGlobal( _pGlobal )
{
	sCursor = SCursorInfo( NDb::GetUITexture( 492 ) );

	CDGPtr<CPtrFuncBase<CGlobalInfo> > pGlobalInfo = pGlobal->GetGlobalInfo();
	pGlobalInfo.Refresh();
	pInfo = pGlobalInfo->GetValue();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGlobalMapUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_MOUSEMOVE:
		{
			GetInterface()->SetCursorInfo( sCursor );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pMapView = GetUIWindow<CWindow>( this, "view" );
			p3DView = new C3DWindow( SWindowInfo( pMapView, SPoint( 0, 0 ), pMapView->GetSize(), "3dview", STYLE_ENABLED | STYLE_VISIBLE | STYLE_BOTTOMMOST ) );

			pBackground = GetUIWindow<CImage>( this, "background" );
			pBackground->SetImage( NDb::GetUITexture( pInfo->nMapID ) );

			sectorsSet.reserve( pInfo->sectorsSet.size() );
			for ( int nTemp = 0; nTemp < pInfo->sectorsSet.size(); nTemp++ )
			{
				SGlobalSector &sGlobalSector = pInfo->sectorsSet[nTemp];
				if ( sGlobalSector.pointsSet.empty() )
					continue;

				bool bVisible = false, bRecommended = false;
				GetGlobalSectorInfo( sGlobalSector, &bVisible, &bRecommended );
				if ( bVisible )
					sectorsSet.push_back( new CGlobalSector( pGlobal, pMapView, sGlobalSector, p3DView->GetView(), bRecommended ) );
			}
			break;
		}
	}

	if ( CWindow::ProcessMessage( sEvent ) )
		return true;

	switch( sEvent.nEvent )
	{
	case EVENT_LBUTTONUP:
		{
			if ( pMapView->HitTest( sEvent.nX, sEvent.nY ) )
			{
				for ( int nTemp = 0; nTemp < sectorsSet.size(); nTemp++ )
				{
					IGlobalSector *pSector = sectorsSet[nTemp];
					const SGlobalSector &sSector = pSector->GetSector();
					if ( pSector->HitTest( sEvent.nX, sEvent.nY ) )
					{
						NMainLoop::Command( new NGame::CICBeginChapter( sSector.nTemplate, pGlobal->GetGlobalGame() ) );
						break;
					}
				}
			}

			return true;
		}
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONDBLCLK:
	case EVENT_RBUTTONUP:
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONDBLCLK:
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalMapUI::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	const SPoint &sCursorPos = GetInterface()->GetCursorPos();
	for ( int nTemp = 0; nTemp < sectorsSet.size(); nTemp++ )
	{
		IGlobalSector *pSector = sectorsSet[nTemp];
		pSector->SetSelected( false );
		if ( pSector->HitTest( sCursorPos.x, sCursorPos.y ) )
			pSector->SetSelected( true );

		pSector->Update( sTime );
	}

	CWindow::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalMapUI::GetGlobalSectorInfo( const SGlobalSector &sSector, bool *pbVisible, bool *pRecommended )
{
	*pbVisible = true;
	*pRecommended = false;

	if ( sSector.nTemplate == -1 )
		return;

	CPtr<NRPG::CGlobalGame> pGame = pGlobal->GetGlobalGame();

	*pbVisible = false;
	list<CPtr<NScenario::CScenarioZone> > zonesList;
	pGame->pScenarioTracker->GetAvailableZones( &zonesList );

	CDGPtr<CPtrFuncBase<CChapterInfo> > pChapterInfo = shareChapterInfo.Get( sSector.nTemplate );
	pChapterInfo.Refresh();
	CObj<CChapterInfo> pInfo = pChapterInfo->GetValue();

	for( int nTemp = 0; nTemp < pInfo->sectorsSet.size(); nTemp++ )
	{
		const SChapterSector &sChapterSector = pInfo->sectorsSet[nTemp];
		if ( sChapterSector.eType == RANDOM )
			continue;

		CPtr<NScenario::CScenarioZone> pZone = pGame->pScenarioTracker->GetZoneByDBZone( NDb::GetDBScenarioZone( sChapterSector.nTemplate ) );
		if ( find( zonesList.begin(), zonesList.end(), pZone ) != zonesList.end() )
		{
			*pbVisible = true;
			if ( pGame->pScenarioTracker->GetRecommendedZone() == pZone )
				*pRecommended = true;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0912170, CGlobalMapUI );
REGISTER_SAVELOAD_CLASS( 0xB0912171, CGlobalSector );
REGISTER_SAVELOAD_CLASS( 0xB0912172, C3DWindow );
