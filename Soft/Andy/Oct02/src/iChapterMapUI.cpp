#include "StdAfx.h"
#include "GView.h"
#include "G2DView.h"
#include "Transform.h"
#include "GSceneUtils.h"
#include "MemObject.h"
#include "RPGGlobal.h"
#include "Sound.h"
#include "PolyUtils.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iMission.h"
#include "iChapterMap.h"
#include "ChapterInfo.h"
#include "iChapterMapUI.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Misc\RandomGen.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "scFlowChartItems.h"
#include "scScenarioTracker.h"
#include "UIWrap.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
const int
	N_MARKER_MORPHTIME					= 500,
	N_DESCRIPTION_MORPHTIME			= 250,
	N_RANDOMZONE_TTL						= 10000,	// TimeToLife
	N_RANDOMZONE_MIN_DIST				= 8,
	N_RANDOMZONE_MAX_DIST				= 256,
	N_RANDOMZONE_ROLLTRY_COUNT	= 100;
const float
	F_PATH_CHECKLEN = 50;
const CVec4
	V4_SECTOR_NORMALCOLOR = CVec4( 0.3f, 1, 0.3f, 0.5f ),
	V4_SECTOR_SELECTEDCOLOR = CVec4( 1, 0.3f, 0.3f, 0.5f );
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTeamMarker
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTeamMarker: public CButton
{
	OBJECT_BASIC_METHODS(CTeamMarker);
public:
	enum EMode
	{
		MODE_MOVE,
		MODE_NORMAL,
		MODE_ZONE
	};

private:
	ZDATA_(CButton)
	EMode eMode, eTargetMode;
	STime sMorphTime, sFlashTime;
	////
	CObj<CImageDraw> pMoving;
	CObj<CImageDraw> pZone;
	CObj<CImageDraw> pZoneFlash;
	CObj<CImageDraw> pNormal;
	CObj<CImageDraw> pNormalFlash;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CButton*)this); f.Add(2,&eMode); f.Add(3,&eTargetMode); f.Add(4,&sMorphTime); f.Add(5,&sFlashTime); f.Add(6,&pMoving); f.Add(7,&pZone); f.Add(8,&pZoneFlash); f.Add(9,&pNormal); f.Add(10,&pNormalFlash); return 0; }

public:
	CTeamMarker() {}
	CTeamMarker( const SWindowInfo &sInfo );

	void SetMode( EMode eMode );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CTeamMarker::CTeamMarker( const SWindowInfo &sInfo ):
	CButton( sInfo ), eMode( MODE_NORMAL ), eTargetMode( MODE_NORMAL ), sMorphTime( 0 ), sFlashTime( 0 )
{
	CPtr<NDb::CUITexture> pNormalTexture = NDb::GetUITexture( 524 );
	if ( IsValid( pNormalTexture ) )
		SetSize( SPoint( pNormalTexture->nWidth, pNormalTexture->nHeight ) );

	pMoving = new CImageDraw( SRect( 0, 0, GetSize().x, GetSize().y ), NDb::GetUITexture( 525 ) );
	pZone = new CImageDraw( SRect( 0, 0, GetSize().x, GetSize().y ), NDb::GetUITexture( 530 ) );
	pZoneFlash = new CImageDraw( SRect( 0, 0, GetSize().x, GetSize().y ), NDb::GetUITexture( 531 ) );
	pNormal = new CImageDraw( SRect( 0, 0, GetSize().x, GetSize().y ), NDb::GetUITexture( 524 ) );
	pNormalFlash = new CImageDraw( SRect( 0, 0, GetSize().x, GetSize().y ), NDb::GetUITexture( 526 ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTeamMarker::SetMode( EMode _eMode )
{
	eTargetMode = _eMode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTeamMarker::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	float fNormal = 0.0f, fMoving = 0.0f, fFlash = 0.0f;

	if ( ( eMode != eTargetMode ) && ( sMorphTime + N_MARKER_MORPHTIME > sTime ) )
	{
		sFlashTime = sTime;

		switch( eTargetMode )
		{
		case MODE_MOVE:
			fMoving = float( sTime - sMorphTime ) / N_MARKER_MORPHTIME;
			fNormal = 1.0f;
			break;
		case MODE_ZONE:
		case MODE_NORMAL:
			fNormal = float( sTime - sMorphTime ) / N_MARKER_MORPHTIME;
			fMoving = 1.0f;
			break;
		}
	}
	else
	{
		eMode = eTargetMode;
		sMorphTime = sTime;

		switch( eMode )
		{
		case MODE_MOVE:
			fMoving = 1.0f;
			break;
		case MODE_ZONE:
		case MODE_NORMAL:
			fNormal = 1.0f;
			fFlash = float( ( sTime - sFlashTime ) % ( N_MARKER_MORPHTIME * 2 ) ) / N_MARKER_MORPHTIME;
			if ( fFlash > 1 )
				fFlash = 2 - fFlash;
			if ( IsMouseCover() )
			{
				fFlash = 1.0f;
				sFlashTime = sTime - N_MARKER_MORPHTIME;
			}
			break;
		}
	}

	pMoving->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fMoving ) );
	pZone->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fNormal ) );
	pZoneFlash->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fFlash ) );
	pNormal->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fNormal ) );
	pNormalFlash->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fFlash ) );

	switch( eTargetMode )
	{
	case MODE_MOVE:
		if ( eMode == MODE_ZONE )
			pZone->Draw( this, sTime, pView );
		else if ( eMode == MODE_NORMAL )
			pNormal->Draw( this, sTime, pView );

		pMoving->Draw( this, sTime, pView );

		if ( eMode == MODE_ZONE )
			pZoneFlash->Draw( this, sTime, pView );
		else if ( eMode == MODE_NORMAL )
			pNormalFlash->Draw( this, sTime, pView );
		break;
	case MODE_ZONE:
		pMoving->Draw( this, sTime, pView );
		pZone->Draw( this, sTime, pView );
		pZoneFlash->Draw( this, sTime, pView );
		break;
	case MODE_NORMAL:
		pMoving->Draw( this, sTime, pView );
		pNormal->Draw( this, sTime, pView );
		pNormalFlash->Draw( this, sTime, pView );
		break;
	}

	CButton::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDescriptionText
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDescriptionText: public CWindow
{
	OBJECT_BASIC_METHODS(CDescriptionText);
public:
	enum EMode
	{
		MODE_HIDDEN,
		MODE_VISIBLE
	};

private:
	ZDATA_(CWindow)
	EMode eMode, eTargetMode;
	STime sMorphTime;
	float fCoeff;
	////
	CPtr<CText> pText;
	CPtr<CImage> pBackground;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&eMode); f.Add(3,&eTargetMode); f.Add(4,&sMorphTime); f.Add(5,&fCoeff); f.Add(6,&pText); f.Add(7,&pBackground); return 0; }

public:
	CDescriptionText() {}
	CDescriptionText( const SWindowInfo &sInfo );

	void Set( EMode eMode, const wstring &wsText = L"" );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CDescriptionText::CDescriptionText( const SWindowInfo &sInfo ):
	CWindow( sInfo ), eMode( MODE_HIDDEN ), eTargetMode( MODE_HIDDEN ), sMorphTime( 0 ), fCoeff( 0 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDescriptionText::Set( EMode _eMode, const wstring &wsText )
{
	if ( ( eTargetMode != _eMode ) && ( eTargetMode != eMode ) )
		eMode = eTargetMode;

	eTargetMode = _eMode;
	pText->SetText( wsText );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDescriptionText::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pText = GetUIWindow<CText>( this, "text" );
			pBackground = GetUIWindow<CImage>( this, "background" );
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDescriptionText::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( eTargetMode != eMode )
	{
		float fDelta = float( sTime - sMorphTime ) / N_DESCRIPTION_MORPHTIME;
		if ( eTargetMode == MODE_HIDDEN )
		{
			fCoeff -= fDelta;
			if ( fCoeff < 0 )
			{
				fCoeff = 0;
				eMode = eTargetMode;
			}
		}
		else
		{
			fCoeff += fDelta;
			if ( fCoeff > 1 )
			{
				fCoeff = 1;
				eMode = eTargetMode;
			}
		}

		pText->SetStyle( STYLE_VISIBLE, false );
		pBackground->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF * fCoeff ) );
		SetStyle( STYLE_VISIBLE, true );
	}
	else
	{
		eMode = eTargetMode;

		pText->SetStyle( STYLE_VISIBLE, true );
		pBackground->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, 0xFF ) );

		if ( eMode == MODE_HIDDEN )
			SetStyle( STYLE_VISIBLE, false );
		else
			SetStyle( STYLE_VISIBLE, true );
	}

	sMorphTime = sTime;
	CWindow::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// ISector
////////////////////////////////////////////////////////////////////////////////////////////////////
class ISector: public CObjectBase
{
public:
	virtual bool HitTest( int nX, int nY ) = 0;

	virtual bool IsSelected() const = 0;
	virtual void SetSelected( bool bState ) = 0;

	virtual void SetDescription( CDescriptionText *pPanel ) = 0;

	virtual const SChapterSector& GetSector() const = 0;

	virtual void Update( const STime &sTime, const CVec2 &vTeamPose )= 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CZoneSector
////////////////////////////////////////////////////////////////////////////////////////////////////
class CZoneSector: public ISector
{
	OBJECT_BASIC_METHODS(CZoneSector)
private:
	ZDATA
	CPtr<CWindow> pView;
	SChapterSector sSector;
	CPtr<NGame::IChapterMap> pChapter;
	////
	bool bUpdated;
	bool bSelected;
	bool bVisible;
	bool bRecommended;
	////
	SPoint sMarkerPos;
	CObj<CImage> pMarker;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pView); f.Add(3,&sSector); f.Add(4,&pChapter); f.Add(5,&bUpdated); f.Add(6,&bSelected); f.Add(7,&bVisible); f.Add(8,&bRecommended); f.Add(9,&sMarkerPos); f.Add(10,&pMarker); return 0; }

protected:
	void UpdateInfo();
	void UpdateSector();

public:
	CZoneSector() {}
	CZoneSector( NGame::IChapterMap *_pChapter, CWindow *_pView, const SChapterSector &_sSector );

	bool HitTest( int nX, int nY );

	bool IsSelected() const;
	void SetSelected( bool bState );

	void SetDescription( CDescriptionText *pPanel );

	const SChapterSector& GetSector() const;

	void Update( const STime &sTime, const CVec2 &vTeamPos );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CZoneSector::CZoneSector( NGame::IChapterMap *_pChapter, CWindow *_pView, const SChapterSector &_sSector ):
	pChapter( _pChapter ), pView( _pView ), sSector( _sSector ), bSelected( false ), bUpdated( false ), sMarkerPos( 0, 0 )
{
	CVec2 vPos( 0, 0 );
	if ( !sSector.pointsSet.empty() )
		vPos = sSector.pointsSet.front();
	else
		ASSERT( 0 );

	pView->ScreenToClient( SPoint( vPos.x, vPos.y ), &sMarkerPos );

	UpdateSector();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CZoneSector::HitTest( int nX, int nY )
{
	return pMarker->HitTest( nX, nY );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CZoneSector::IsSelected() const
{
	return bSelected;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CZoneSector::SetSelected( bool bState )
{
	if ( bSelected == bState )
		return;

	bUpdated = true;
	bSelected = bState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CZoneSector::SetDescription( CDescriptionText *pPanel )
{
	if ( sSector.nDescriptionID == -1 )
	{
		pPanel->Set( CDescriptionText::MODE_VISIBLE, L"<color=red>ERROR: DESCRIPTION NOT SET" );
		return;
	}

	pPanel->Set( CDescriptionText::MODE_VISIBLE, GetDBString( sSector.nDescriptionID ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SChapterSector& CZoneSector::GetSector() const
{
	return sSector;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CZoneSector::Update( const STime &sTime, const CVec2 &vTeamPos )
{
	CPtr<NRPG::CGlobalGame> pGame = pChapter->GetGlobalGame();

	bool bVisible = pMarker->GetStyle( STYLE_VISIBLE );
	CPtr<NScenario::CScenarioZone> pZone = pGame->pScenarioTracker->GetZoneByDBZone( NDb::GetDBScenarioZone( sSector.nTemplate ) );
	if ( IsValid( pZone ) )
	{
		pMarker->SetStyle( STYLE_VISIBLE, true );
		if ( HitTest( vTeamPos.x, vTeamPos.y ) )
		{
			if ( NGlobal::GetVar( "chaptermap_debug", 0 ).GetFloat() == 0 )
				pGame->pScenarioTracker->OpenZone( pZone );
			else
				pGame->pScenarioTracker->CheatOpenZone( pZone );
		}
	}
	pMarker->SetStyle( STYLE_VISIBLE, bVisible );

	UpdateInfo();

	if ( bUpdated )
	{
		bUpdated = false;
		UpdateSector();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CZoneSector::UpdateInfo()
{
	bool bNewVisible = true;
	bool bNewRecommended = false;

	CPtr<NRPG::CGlobalGame> pGame = pChapter->GetGlobalGame();

	CPtr<NScenario::CScenarioZone> pZone = pGame->pScenarioTracker->GetZoneByDBZone( NDb::GetDBScenarioZone( sSector.nTemplate ) );
	if ( IsValid( pZone ) )
	{
		bNewVisible = false;

		list<CPtr<NScenario::CScenarioZone> > zonesList;
		pGame->pScenarioTracker->GetAvailableZones( &zonesList );

		if ( find( zonesList.begin(), zonesList.end(), pZone ) != zonesList.end() )
		{
			bNewVisible = true;
			if ( pGame->pScenarioTracker->GetRecommendedZone() == pZone )
				bNewRecommended = true;
		}
	}

	if ( ( bNewVisible != bVisible ) || ( bNewRecommended != bRecommended ) )
	{
		bUpdated = true;
		bVisible = bNewVisible;
		bRecommended = bNewRecommended;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CZoneSector::UpdateSector()
{
	CPtr<NDb::CUITexture> pMarkerTexture = NDb::GetUITexture( 520 );
	if ( bRecommended )
		pMarkerTexture = NDb::GetUITexture( 521 );

	SPoint sMarkerSize( pMarkerTexture->nWidth, pMarkerTexture->nHeight );
	pMarker = new CImage( SWindowInfo( pView, sMarkerPos, sMarkerSize, "", STYLE_ENABLED | STYLE_NOACTIVATE | STYLE_BOTTOMMOST ) );
	pMarker->SetImage( pMarkerTexture );

	if ( bVisible )
		pMarker->SetStyle( STYLE_VISIBLE, true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CRandomSector
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRandomSector: public ISector
{
	OBJECT_BASIC_METHODS(CRandomSector)
private:
	ZDATA
	CPtr<CWindow> pView;
	SChapterSector sSector;
	CPtr<NGame::IChapterMap> pChapter;
	////
	bool bSelected;
	////
	bool bVisible;
	STime sUpdateTime;
	SPoint sMarkerPos;
	CObj<CImage> pMarker;
	CTRect<float> sZone;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pView); f.Add(3,&sSector); f.Add(4,&pChapter); f.Add(5,&bSelected); f.Add(6,&bVisible); f.Add(7,&sUpdateTime); f.Add(8,&sMarkerPos); f.Add(9,&pMarker); f.Add(10,&sZone); return 0; }

public:
	CRandomSector() {}
	CRandomSector( NGame::IChapterMap *pChapter, CWindow *pView, const SChapterSector &sSector );

	bool HitTest( int nX, int nY );

	bool IsSelected() const;
	void SetSelected( bool bState );

	void SetDescription( CDescriptionText *pPanel );

	const SChapterSector& GetSector() const;

	void Update( const STime &sTime, const CVec2 &vTeamPos );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CRandomSector::CRandomSector( NGame::IChapterMap *_pChapter, CWindow *_pView, const SChapterSector &_sSector ):
	pChapter( _pChapter ), pView( _pView ), sSector( _sSector ), bSelected( false ), bVisible( false ), sMarkerPos( 0, 0 ), sUpdateTime( 0 ), sZone( 0, 0, 0, 0 )
{
	for ( int nPoint = 0; nPoint < sSector.pointsSet.size(); nPoint++ )
	{
		const CVec2 &vSectorPoint = sSector.pointsSet[nPoint];
		sZone.x1 = min( sZone.x1, vSectorPoint.x );
		sZone.x2 = max( sZone.x2, vSectorPoint.x );
		sZone.y1 = min( sZone.y1, vSectorPoint.y );
		sZone.y2 = max( sZone.y2, vSectorPoint.y );
	}

	CPtr<NDb::CUITexture> pMarkerTexture = NDb::GetUITexture( 519 );
	SPoint sMarkerSize( pMarkerTexture->nWidth, pMarkerTexture->nHeight );
	pMarker = new CImage( SWindowInfo( pView, SPoint( 0, 0 ), sMarkerSize, "", STYLE_ENABLED | STYLE_NOACTIVATE | STYLE_BOTTOMMOST ) );
	pMarker->SetImage( pMarkerTexture );

	CPtr<CToolTip> pToolTip = new CToolTip( SWindowInfo( pMarker->GetInterface(), SPoint( 0, 0 ), SPoint( 0, 0 ), "tooltip", STYLE_ENABLED ) );
	pToolTip->SetText( GetDBString( 5343 ) );
	pMarker->SetToolTip( pToolTip );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CRandomSector::HitTest( int nX, int nY )
{
	if ( !bVisible )
		return false;

	return pMarker->HitTest( nX, nY );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CRandomSector::IsSelected() const
{
	return bSelected;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRandomSector::SetSelected( bool bState )
{
	bSelected = bState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRandomSector::SetDescription( CDescriptionText *pPanel )
{
	pPanel->SetStyle( STYLE_VISIBLE, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SChapterSector& CRandomSector::GetSector() const
{
	return sSector;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRandomSector::Update( const STime &sTime, const CVec2 &vTeamPos )
{
	static SRand sRand;

	if ( HitTest( vTeamPos.x, vTeamPos.y ) )
		NMainLoop::Command( new NGame::CICBeginMission( NGame::GetVariantForTemplate( sSector.nTemplate, 
			pChapter->GetGlobalGame() ), pChapter->GetGlobalGame() ) );

	if ( sUpdateTime > sTime )
		return;

	sUpdateTime = sTime + N_RANDOMZONE_TTL * sRand.GetFloat( 0.5f, 1.0f );
	if ( !bVisible )
	{
		if ( sRand.Get( 100 ) < sSector.nProbability )
			return;

		for ( int nTemp = 0; nTemp < N_RANDOMZONE_ROLLTRY_COUNT; nTemp++ )
		{
			CVec2 vPoint = CVec2( sRand.GetFloat( sZone.x1, sZone.x2 ), sRand.GetFloat( sZone.y1, sZone.y2 ) );

			if ( !IsPointInPolygon( sSector.pointsSet, vPoint ) )
				continue;

			float fDist = fabs( vTeamPos - vPoint );
			if ( ( fDist < N_RANDOMZONE_MIN_DIST ) || ( fDist > N_RANDOMZONE_MAX_DIST ) )
				continue;

			bVisible = true;

			const SPoint &sSize = pMarker->GetSize();
			pView->ScreenToClient( SPoint( vPoint.x - sSize.x / 2, vPoint.y - sSize.y / 2 ), &sMarkerPos );

			pMarker->SetStyle( STYLE_VISIBLE, true );
			pMarker->SetPosition( sMarkerPos );
		}
	}
	else
	{
		SRand sRand;
		if ( sRand.Get( 100 ) < sSector.nProbability / 4 )
			return;

		bVisible = false;

		pMarker->SetStyle( STYLE_VISIBLE, false );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CChapterMapUI
////////////////////////////////////////////////////////////////////////////////////////////////////
CChapterMapUI::CChapterMapUI( const SWindowInfo &sInfo, NGame::IChapterMap *_pChapter ):
	CWindow( sInfo ), pChapter( _pChapter )
{
	sCursor = SCursorInfo( NDb::GetUITexture( 492 ) );

	CDGPtr<CPtrFuncBase<CChapterInfo> > pChapterInfo = pChapter->GetChapterInfo();
	pChapterInfo.Refresh();
	pInfo = pChapterInfo->GetValue();

	vCurrentPos = pChapter->GetGlobalGame()->vChapterPos;
	vTargetPos = vCurrentPos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CChapterMapUI::SetTarget( const CVec2 &_vTargetPos )
{
	vTargetPos = _vTargetPos;
	float fXDelta = vTargetPos.x - vCurrentPos.x;
	float fYDelta = vTargetPos.y - vCurrentPos.y;
	if ( abs( fYDelta ) > abs( fXDelta ) )
	{
		fXK = fXDelta / abs( fYDelta );
		fYK = fYDelta / abs( fYDelta );
	}
	else
	{
		fXK = fXDelta / abs( fXDelta );
		fYK = fYDelta / abs( fXDelta );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CChapterMapUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "action" )
			{
				for ( int nTemp = 0; nTemp < sectorsSet.size(); nTemp++ )
				{
					if ( !sectorsSet[nTemp]->HitTest( vCurrentPos.x, vCurrentPos.y ) )
						continue;

					const SChapterSector &sSector = sectorsSet[nTemp]->GetSector();

					if( !sSector.szID.empty() )
						pChapter->SetGlobalVar( sSector.szID, 2.0f );

					CPtr<NScenario::CScenarioTracker> pScenario = pChapter->GetGlobalGame()->pScenarioTracker;
					if ( IsValid( pScenario ) )
					{
						CPtr<NScenario::CScenarioZone> pZone = pScenario->GetZoneByDBZone( NDb::GetDBScenarioZone( sSector.nTemplate ) );
						if ( IsValid( pZone ) )
							NMainLoop::Command( new NGame::CICBeginMission( pZone->GetDefaultTemplateID(), pChapter->GetGlobalGame() ) );
					}
				}
				return true;
			}

			break;
		}
	case EVENT_MOUSEMOVE:
		{
			GetInterface()->SetCursorInfo( sCursor );
			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pExitChapter = new CFlashButton( sEvent.pLoader->GetControl( "exitchapter" ) );

			pTextLeft = new CDescriptionText( sEvent.pLoader->GetControl( "text_left" ) );
			pTextRight = new CDescriptionText( sEvent.pLoader->GetControl( "text_right" ) );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pMapView = GetUIWindow<CWindow>( this, "view" );

			pBackground = GetUIWindow<CImage>( this, "background" );
			pBackground->SetImage( NDb::GetUITexture( pInfo->nMapID ) );

			pTeamMarker = new CTeamMarker( SWindowInfo( pMapView, SPoint( vCurrentPos.x, vCurrentPos.y ), SPoint( 0, 0 ), "action", STYLE_ENABLED | STYLE_VISIBLE ) );

			sectorsSet.reserve( pInfo->sectorsSet.size() );
			for ( int nTemp = 0; nTemp < pInfo->sectorsSet.size(); nTemp++ )
			{
				SChapterSector &sChapterSector = pInfo->sectorsSet[nTemp];
				if ( sChapterSector.pointsSet.empty() )
					continue;

				switch( sChapterSector.eType )
				{
				case ZONE:
					{
						sectorsSet.push_back( new CZoneSector( pChapter, pMapView, sChapterSector ) );
						break;
					}
				case RANDOM:
					{
						sectorsSet.push_back( new CRandomSector( pChapter, pMapView, sChapterSector ) );
						break;
					}
				default:
					ASSERT( 0 );
					break;
				}
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
			SetTarget( CVec2( sEvent.nX, sEvent.nY ) );

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
void CChapterMapUI::Update( const STime &sTime, NGScene::I2DGameView *pView )
{
	bool bMoving = false;
	STime sTargetTime = sTime;
	if ( fabs2( vTargetPos - vCurrentPos ) > 1 )
	{
		bMoving = true;

		while ( ( fabs( vTargetPos - vCurrentPos ) > 1 ) && ( sLastUpdateTime < sTargetTime ) )
		{
			vCurrentPos.x += fXK;
			vCurrentPos.y += fYK;
			fPassedPathLen += ( fXK + fYK );

			SPoint sTile( vCurrentPos.x, vCurrentPos.y );
			float fSpeed = 20;

			sLastUpdateTime = sLastUpdateTime + 1000.0f / fSpeed;

			if ( fPassedPathLen > F_PATH_CHECKLEN )
			{
				fPassedPathLen -= F_PATH_CHECKLEN;

				for ( int nTemp = 0; nTemp < sectorsSet.size(); nTemp++ )
					sectorsSet[nTemp]->Update( sTime, vCurrentPos );
			}
		}
	}

	bool bHideLeft = true, bHideRight = true;
	const SPoint &sCursorPos = GetInterface()->GetCursorPos();
	for ( int nTemp = 0; nTemp < sectorsSet.size(); nTemp++ )
	{
		ISector *pSector = sectorsSet[nTemp];
		pSector->SetSelected( false );
		if ( pSector->HitTest( sCursorPos.x, sCursorPos.y ) )
		{
			if ( sCursorPos.x > 512 )
			{
				bHideLeft = false;
				pSector->SetDescription( pTextLeft );
			}
			else
			{
				bHideRight = false;
				pSector->SetDescription( pTextRight );
			}

			pSector->SetSelected( true );
		}

		pSector->Update( sTime, vCurrentPos );
	}
	if ( bHideLeft )
		pTextLeft->Set( CDescriptionText::MODE_HIDDEN );
	if ( bHideRight )
		pTextRight->Set( CDescriptionText::MODE_HIDDEN );

	pChapter->GetGlobalGame()->vChapterPos = vCurrentPos;
	sLastUpdateTime = sTargetTime;

	SPoint sPoint;
	pMapView->ScreenToClient( SPoint( vCurrentPos.x, vCurrentPos.y ), &sPoint );

	bool bHitSector = false;
	for ( int nTemp = 0; nTemp < sectorsSet.size(); nTemp++ )
	{
		if ( !sectorsSet[nTemp]->HitTest( vCurrentPos.x, vCurrentPos.y ) )
			continue;

		bHitSector = true;
		break;
	}

	const SPoint &sSize = pTeamMarker->GetSize();
	pTeamMarker->SetPosition( SPoint( sPoint.x - sSize.x / 2, sPoint.y - sSize.y / 2 ) );
	pTeamMarker->SetMode( bMoving ? CTeamMarker::MODE_MOVE : bHitSector ? CTeamMarker::MODE_ZONE : CTeamMarker::MODE_NORMAL );

	CWindow::Update( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB2606150, CTeamMarker );
REGISTER_SAVELOAD_CLASS( 0xB2606152, CChapterMapUI );
REGISTER_SAVELOAD_CLASS( 0xB2606153, CZoneSector );
REGISTER_SAVELOAD_CLASS( 0xB2606154, CRandomSector );
REGISTER_SAVELOAD_CLASS( 0xB2606155, CDescriptionText );
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(Gfx)
	REGISTER_VAR( "chaptermap_debug", NULL, 0, false )
FINISH_REGISTER
