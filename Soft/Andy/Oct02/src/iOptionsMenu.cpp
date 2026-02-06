#include "StdAfx.h"
#include "Gfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "..\Misc\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iOptionsMenu.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_SLIDER_STEPS = 100;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CComplexTextSlider
////////////////////////////////////////////////////////////////////////////////////////////////////
class CComplexTextSlider: public CSlider
{
	OBJECT_BASIC_METHODS(CComplexTextSlider)
private:
	ZDATA_(CSlider)
	CPtr<CText> pText;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CSlider*)this); f.Add(2,&pText); return 0; }

public:
	CComplexTextSlider() {}
	CComplexTextSlider( const SWindowInfo &sInfo );

	void UpdateText();

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CComplexTextSlider::CComplexTextSlider( const SWindowInfo &sInfo ):
	CSlider( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CComplexTextSlider::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pText = new CText( sEvent.pLoader->GetControl( "slider" ) );
			break;
		}
	}

	return CSlider::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CComplexTextSlider::UpdateText()
{
	WCHAR wsBuffer[256];
	swprintf( wsBuffer, L"<font face=Courier size=18pt><center>%d", GetValue() );
	pText->SetText( GetDBString( 4404 ) + wsBuffer );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CComplexSlider
////////////////////////////////////////////////////////////////////////////////////////////////////
class CComplexSlider: public CWindow
{
	OBJECT_BASIC_METHODS(CComplexSlider)
private:
	ZDATA_(CWindow)
	CObj<CHoverButton> pPlus;
	CObj<CHoverButton> pMinus;
	CPtr<CProgressBar> pProgress;
	CObj<CComplexTextSlider> pSlider;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pPlus); f.Add(3,&pMinus); f.Add(4,&pProgress); f.Add(5,&pSlider); return 0; }

public:
	CComplexSlider() {}
	CComplexSlider( const SWindowInfo &sInfo );

	float GetValue();
	void SetValue( float fVal );

	void UpdateControls();

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CComplexSlider::CComplexSlider( const SWindowInfo &sInfo ):
	CWindow( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CComplexSlider::GetValue()
{
	return float( pSlider->GetValue() ) / N_SLIDER_STEPS;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CComplexSlider::SetValue( float fVal )
{
	pSlider->SetValue( N_SLIDER_STEPS * fVal );
	UpdateControls();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CComplexSlider::UpdateControls()
{
	pSlider->UpdateText();
	pProgress->SetValue( ( float( pSlider->GetValue() ) / N_SLIDER_STEPS ) * 0.9f + 0.1f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CComplexSlider::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "plus" )
				pSlider->SetValue( pSlider->GetValue() + N_SLIDER_STEPS / 10 );
			else if ( sEvent.szID == "minus" )
				pSlider->SetValue( pSlider->GetValue() - N_SLIDER_STEPS / 10 );

			UpdateControls();
			return true;
		}
	case EVENT_TEMPLATELOAD:
		{
			pPlus = new CHoverButton( sEvent.pLoader->GetControl( "plus" ) );
			pMinus = new CHoverButton( sEvent.pLoader->GetControl( "minus" ) );
			pSlider = new CComplexTextSlider( sEvent.pLoader->GetControl( "slider" ) );

			pPlus->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4418 ) );
			pPlus->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4418 ) );
			pMinus->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4419 ) );
			pMinus->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4419 ) );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pProgress = GetUIWindow<CProgressBar>( this, "progress" );

			UpdateControls();
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CComplexComboBox
////////////////////////////////////////////////////////////////////////////////////////////////////
class CComplexComboBox: public CComboBox
{
	OBJECT_NOCOPY_METHODS(CComplexComboBox)
private:
	ZDATA_(CComboBox)
	CObj<CHoverButton> pDropDown;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CComboBox*)this); f.Add(2,&pDropDown); return 0; }

public:
	CComplexComboBox() {}
	CComplexComboBox( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CComplexComboBox::CComplexComboBox( const SWindowInfo &sInfo ):
	CComboBox( sInfo )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CComplexComboBox::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pDropDown = new CHoverButton( sEvent.pLoader->GetControl( "drop_list" ) );

			pDropDown->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4429 ) );
			pDropDown->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4429 ) );
			break;
		}
	}

	return CComboBox::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// COptionsUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class COptionsUI: public CWindow
{
	OBJECT_NOCOPY_METHODS(COptionsUI)
private:
	ZDATA_(CWindow)
	NGame::EOptionsScreen eScreen;
	CObj<CHoverButton> pClose;
	CObj<CHoverButton> pVideoOptions;
	CObj<CHoverButton> pAudioOptions;
	CObj<CHoverButton> pGamePlayOptions;
	CObj<CHoverButton> pControlsOptions;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&eScreen); f.Add(3,&pClose); f.Add(4,&pVideoOptions); f.Add(5,&pAudioOptions); f.Add(6,&pGamePlayOptions); f.Add(7,&pControlsOptions); return 0; }

public:
	COptionsUI() {}
	COptionsUI( const SWindowInfo &sInfo, NGame::EOptionsScreen eScreen = NGame::OS_EMPTY );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
COptionsUI::COptionsUI( const SWindowInfo &sInfo, NGame::EOptionsScreen _eScreen ):
	CWindow( sInfo ), eScreen( _eScreen )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool COptionsUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pClose = new CHoverButton( sEvent.pLoader->GetControl( "cancel" ) );
			pVideoOptions = new CHoverButton( sEvent.pLoader->GetControl( "video" ) );
			pAudioOptions = new CHoverButton( sEvent.pLoader->GetControl( "audio" ) );
			pGamePlayOptions = new CHoverButton( sEvent.pLoader->GetControl( "gameplay" ) );
			pControlsOptions = new CHoverButton( sEvent.pLoader->GetControl( "controls" ) );

			pClose->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4406 ) );
			pClose->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4406 ) );
			pVideoOptions->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4382 ) );
			pVideoOptions->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4382 ) );
			pAudioOptions->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4383 ) );
			pAudioOptions->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4383 ) );
			pGamePlayOptions->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4384 ) );
			pGamePlayOptions->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4384 ) );
			pControlsOptions->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4385 ) );
			pControlsOptions->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4385 ) );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			switch( eScreen )
			{
			case NGame::OS_VIDEO:
				GetUIWindow<CImage>( this, "videomark" )->SetStyle( STYLE_VISIBLE, true );
				break;
			case NGame::OS_AUDIO:
				GetUIWindow<CImage>( this, "audiomark" )->SetStyle( STYLE_VISIBLE, true );
				break;
			case NGame::OS_GAMEPLAY:
				GetUIWindow<CImage>( this, "gameplaymark" )->SetStyle( STYLE_VISIBLE, true );
				break;
			case NGame::OS_CONTROLS:
				GetUIWindow<CImage>( this, "controlsmark" )->SetStyle( STYLE_VISIBLE, true );
				break;
			}
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CEmptyOptionsUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CEmptyOptionsUI: public CWindow
{
	OBJECT_NOCOPY_METHODS(CEmptyOptionsUI)
private:
	ZDATA_(CWindow)
	SCursorInfo sCursor;
	CObj<COptionsUI> pBase;
	NGame::EOptionsScreen eScreen;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&sCursor); f.Add(3,&pBase); f.Add(4,&eScreen); return 0; }

public:
	CEmptyOptionsUI() {}
	CEmptyOptionsUI( const SWindowInfo &sInfo, NGame::EOptionsScreen _eScreen = NGame::OS_EMPTY );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CEmptyOptionsUI::CEmptyOptionsUI( const SWindowInfo &sInfo, NGame::EOptionsScreen _eScreen ):
	CWindow( sInfo ), eScreen( _eScreen )
{
	sCursor = SCursorInfo( NDb::GetUITexture( 492 ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CEmptyOptionsUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_MOUSEMOVE:
		{
			GetInterface()->SetCursorInfo( sCursor );
			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pBase = new COptionsUI( sEvent.pLoader->GetControl( "base" ), eScreen );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVideoOptionsUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVideoOptionsUI: public CEmptyOptionsUI
{
	OBJECT_NOCOPY_METHODS(CVideoOptionsUI)
private:
	ZDATA_(CEmptyOptionsUI)
	CObj<CHoverButton> pApply;
	CObj<CHoverButton> pDefault;
	CObj<CComplexSlider> pGammaSlider;
	CObj<CComplexComboBox> pFog;
	CObj<CComplexComboBox> pLightmaps;
	CObj<CComplexComboBox> pResolution;
	CObj<CComplexComboBox> pSunShadows;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CEmptyOptionsUI*)this); f.Add(2,&pApply); f.Add(3,&pDefault); f.Add(4,&pGammaSlider); f.Add(5,&pFog); f.Add(6,&pLightmaps); f.Add(7,&pResolution); f.Add(8,&pSunShadows); return 0; }

public:
	CVideoOptionsUI() {}
	CVideoOptionsUI( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CVideoOptionsUI::CVideoOptionsUI( const SWindowInfo &sInfo ):
	CEmptyOptionsUI( sInfo, NGame::OS_VIDEO )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVideoOptionsUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "apply" )
			{
				NGlobal::SetVar( "gfx_fog", pFog->GetSelectedItem() );
				NGlobal::SetVar( "gfx_resolution", pResolution->GetSelectedItem() );
				NGlobal::ProcessCommand( L"gfx_update" );
			}
			else if ( sEvent.szID == "default" )
			{
				pFog->SetSelectedItem( 0 );
				pResolution->SetSelectedItem( 1024 );
			}

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pApply = new CHoverButton( sEvent.pLoader->GetControl( "apply" ) );
			pDefault = new CHoverButton( sEvent.pLoader->GetControl( "default" ) );
			pGammaSlider = new CComplexSlider( sEvent.pLoader->GetControl( "gamma_slider" ) );

			pApply->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4376 ) );
			pApply->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4376 ) );
			pDefault->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4377 ) );
			pDefault->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4377 ) );

			pFog = new CComplexComboBox( sEvent.pLoader->GetControl( "fog" ) );
			pFog->SetStateInfo( CComplexComboBox::STATE_NORMAL, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pFog->SetStateInfo( CComplexComboBox::STATE_HILIGHTED, CComplexComboBox::SInfo( NUI::GetDBString( 4401 ) ) );
			pFog->SetStateInfo( CComplexComboBox::STATE_SELECTED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pFog->SetStateInfo( CComplexComboBox::STATE_DISABLED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pFog->AddItem( 0, CComplexComboBox::SInfo( NUI::GetDBString( 4407 ) ), 171 );
			pFog->AddItem( 1, CComplexComboBox::SInfo( NUI::GetDBString( 4408 ) ), 172 );
			pFog->AddItem( 2, CComplexComboBox::SInfo( NUI::GetDBString( 4409 ) ), 173 );

			pLightmaps = new CComplexComboBox( sEvent.pLoader->GetControl( "lightmaps" ) );
			pLightmaps->SetStateInfo( CComplexComboBox::STATE_NORMAL, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pLightmaps->SetStateInfo( CComplexComboBox::STATE_HILIGHTED, CComplexComboBox::SInfo( NUI::GetDBString( 4401 ) ) );
			pLightmaps->SetStateInfo( CComplexComboBox::STATE_SELECTED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pLightmaps->SetStateInfo( CComplexComboBox::STATE_DISABLED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pLightmaps->AddItem( 0, CComplexComboBox::SInfo( NUI::GetDBString( 4413 ) ), 171 );
			pLightmaps->AddItem( 1, CComplexComboBox::SInfo( NUI::GetDBString( 4414 ) ), 172 );
			pLightmaps->AddItem( 2, CComplexComboBox::SInfo( NUI::GetDBString( 4415 ) ), 173 );

			pResolution = new CComplexComboBox( sEvent.pLoader->GetControl( "resolution" ) );
			pResolution->SetStateInfo( CComplexComboBox::STATE_NORMAL, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pResolution->SetStateInfo( CComplexComboBox::STATE_HILIGHTED, CComplexComboBox::SInfo( NUI::GetDBString( 4401 ) ) );
			pResolution->SetStateInfo( CComplexComboBox::STATE_SELECTED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pResolution->SetStateInfo( CComplexComboBox::STATE_DISABLED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			list<NGfx::SVideoMode> modesList;
			NGfx::GetModesList( &modesList );
			for( list<NGfx::SVideoMode>::iterator iTemp = modesList.begin(); iTemp != modesList.end(); )
			{
				float fAspect = float( iTemp->nXSize ) / float( iTemp->nYSize );
				if ( fabs( fAspect - 4.0f / 3.0f ) < FP_EPSILON )
					iTemp++;
				else
					iTemp = modesList.erase( iTemp );
			}
			for( list<NGfx::SVideoMode>::iterator iTemp = modesList.begin(); iTemp != modesList.end(); iTemp++ )
			{
				int nTemplate = 172;
				list<NGfx::SVideoMode>::iterator iNext = iTemp; iNext++;
				if ( iTemp == modesList.begin() )
					nTemplate = 171;
				if ( iNext == modesList.end() )
					nTemplate = 173;

				WCHAR wsBuffer[1024];
				swprintf( wsBuffer, L"<right>%dx%d", iTemp->nXSize, iTemp->nYSize );
				pResolution->AddItem( iTemp->nXSize, NUI::CComplexComboBox::SInfo( wsBuffer ), nTemplate );
			}


			pSunShadows = new CComplexComboBox( sEvent.pLoader->GetControl( "sunshadows" ) );
			pSunShadows->SetStateInfo( CComplexComboBox::STATE_NORMAL, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pSunShadows->SetStateInfo( CComplexComboBox::STATE_HILIGHTED, CComplexComboBox::SInfo( NUI::GetDBString( 4401 ) ) );
			pSunShadows->SetStateInfo( CComplexComboBox::STATE_SELECTED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pSunShadows->SetStateInfo( CComplexComboBox::STATE_DISABLED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pSunShadows->AddItem( 0, CComplexComboBox::SInfo( NUI::GetDBString( 4410 ) ), 171 );
			pSunShadows->AddItem( 1, CComplexComboBox::SInfo( NUI::GetDBString( 4411 ) ), 172 );
			pSunShadows->AddItem( 2, CComplexComboBox::SInfo( NUI::GetDBString( 4412 ) ), 173 );

			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pFog->SetSelectedItem( int( NGlobal::GetVar( "gfx_fog", 0 ).GetFloat() ) );
			pResolution->SetSelectedItem( int( NGlobal::GetVar( "gfx_resolution", 1024 ).GetFloat() ) );
			break;
		}
	}

	return CEmptyOptionsUI::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAudioOptionsUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAudioOptionsUI: public CEmptyOptionsUI
{
	OBJECT_NOCOPY_METHODS(CAudioOptionsUI)
private:
	ZDATA_(CEmptyOptionsUI)
	CObj<CHoverButton> pApply;
	CObj<CHoverButton> pDefault;
	CObj<CComplexSlider> pSoundVolume;
	CObj<CComplexSlider> pMusicVolume;
	CObj<CComplexComboBox> pOutputType;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CEmptyOptionsUI*)this); f.Add(2,&pApply); f.Add(3,&pDefault); f.Add(4,&pSoundVolume); f.Add(5,&pMusicVolume); f.Add(6,&pOutputType); return 0; }

public:
	CAudioOptionsUI() {}
	CAudioOptionsUI( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAudioOptionsUI::CAudioOptionsUI( const SWindowInfo &sInfo ):
	CEmptyOptionsUI( sInfo, NGame::OS_AUDIO )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAudioOptionsUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "apply" )
			{
				NGlobal::SetVar( "sound_sfxvolume", pSoundVolume->GetValue() );
				NGlobal::SetVar( "sound_musicvolume", pMusicVolume->GetValue() );
			}
			else if ( sEvent.szID == "default" )
			{
				pSoundVolume->SetValue( 1.0f );
				pMusicVolume->SetValue( 0.5f );
			}

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pApply = new CHoverButton( sEvent.pLoader->GetControl( "apply" ) );
			pDefault = new CHoverButton( sEvent.pLoader->GetControl( "default" ) );
			pSoundVolume = new CComplexSlider( sEvent.pLoader->GetControl( "sound_volume" ) );
			pMusicVolume = new CComplexSlider( sEvent.pLoader->GetControl( "music_volume" ) );

			pApply->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4376 ) );
			pApply->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4376 ) );
			pDefault->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4377 ) );
			pDefault->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4377 ) );

			pOutputType = new CComplexComboBox( sEvent.pLoader->GetControl( "outputtype" ) );
			pOutputType->SetStateInfo( CComplexComboBox::STATE_NORMAL, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pOutputType->SetStateInfo( CComplexComboBox::STATE_HILIGHTED, CComplexComboBox::SInfo( NUI::GetDBString( 4401 ) ) );
			pOutputType->SetStateInfo( CComplexComboBox::STATE_SELECTED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pOutputType->SetStateInfo( CComplexComboBox::STATE_DISABLED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pOutputType->AddItem( 0, CComplexComboBox::SInfo( NUI::GetDBString( 4420 ) ), 171 );
			pOutputType->AddItem( 1, CComplexComboBox::SInfo( NUI::GetDBString( 4421 ) ), 172 );
			pOutputType->AddItem( 3, CComplexComboBox::SInfo( NUI::GetDBString( 4422 ) ), 172 );
			pOutputType->AddItem( 4, CComplexComboBox::SInfo( NUI::GetDBString( 4423 ) ), 172 );
			pOutputType->AddItem( 5, CComplexComboBox::SInfo( NUI::GetDBString( 4424 ) ), 172 );
			pOutputType->AddItem( 6, CComplexComboBox::SInfo( NUI::GetDBString( 4425 ) ), 173 );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pSoundVolume->SetValue( NGlobal::GetVar( "sound_sfxvolume", 0 ).GetFloat() );
			pMusicVolume->SetValue( NGlobal::GetVar( "sound_musicvolume", 0 ).GetFloat() );
			break;
		}
	}

	return CEmptyOptionsUI::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGamePlayOptionsUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGamePlayOptionsUI: public CEmptyOptionsUI
{
	OBJECT_NOCOPY_METHODS(CGamePlayOptionsUI)
private:
	ZDATA_(CEmptyOptionsUI)
	CObj<CHoverButton> pApply;
	CObj<CHoverButton> pDefault;
	CObj<CComplexComboBox> pDifficulty;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CEmptyOptionsUI*)this); f.Add(2,&pApply); f.Add(3,&pDefault); f.Add(4,&pDifficulty); return 0; }

public:
	CGamePlayOptionsUI() {}
	CGamePlayOptionsUI( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CGamePlayOptionsUI::CGamePlayOptionsUI( const SWindowInfo &sInfo ):
	CEmptyOptionsUI( sInfo, NGame::OS_GAMEPLAY )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGamePlayOptionsUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "apply" )
			{
			}
			else if ( sEvent.szID == "default" )
			{
			}

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pApply = new CHoverButton( sEvent.pLoader->GetControl( "apply" ) );
			pDefault = new CHoverButton( sEvent.pLoader->GetControl( "default" ) );

			pApply->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4376 ) );
			pApply->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4376 ) );
			pDefault->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4377 ) );
			pDefault->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4377 ) );

			pDifficulty = new CComplexComboBox( sEvent.pLoader->GetControl( "gamedifficulty" ) );
			pDifficulty->SetStateInfo( CComplexComboBox::STATE_NORMAL, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pDifficulty->SetStateInfo( CComplexComboBox::STATE_HILIGHTED, CComplexComboBox::SInfo( NUI::GetDBString( 4401 ) ) );
			pDifficulty->SetStateInfo( CComplexComboBox::STATE_SELECTED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pDifficulty->SetStateInfo( CComplexComboBox::STATE_DISABLED, CComplexComboBox::SInfo( NUI::GetDBString( 4402 ) ) );
			pDifficulty->AddItem( 0, CComplexComboBox::SInfo( NUI::GetDBString( 4426 ) ), 171 );
			pDifficulty->AddItem( 1, CComplexComboBox::SInfo( NUI::GetDBString( 4427 ) ), 172 );
			pDifficulty->AddItem( 2, CComplexComboBox::SInfo( NUI::GetDBString( 4428 ) ), 173 );

			break;
		}
	}

	return CEmptyOptionsUI::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CControlsOptionsUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CControlsOptionsUI: public CEmptyOptionsUI
{
	OBJECT_NOCOPY_METHODS(CControlsOptionsUI)
private:
	ZDATA_(CEmptyOptionsUI)
	CObj<CHoverButton> pApply;
	CObj<CHoverButton> pDefault;
	CObj<CComplexSlider> pMouseSensivity;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CEmptyOptionsUI*)this); f.Add(2,&pApply); f.Add(3,&pDefault); f.Add(4,&pMouseSensivity); return 0; }

public:
	CControlsOptionsUI() {}
	CControlsOptionsUI( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CControlsOptionsUI::CControlsOptionsUI( const SWindowInfo &sInfo ):
	CEmptyOptionsUI( sInfo, NGame::OS_CONTROLS )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CControlsOptionsUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "apply" )
			{
			}
			else if ( sEvent.szID == "default" )
			{
			}

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pApply = new CHoverButton( sEvent.pLoader->GetControl( "apply" ) );
			pDefault = new CHoverButton( sEvent.pLoader->GetControl( "default" ) );
			pMouseSensivity = new CComplexSlider( sEvent.pLoader->GetControl( "mouse_sensivity" ) );

			pApply->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4376 ) );
			pApply->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4376 ) );
			pDefault->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 4404 ) + GetDBString( 4377 ) );
			pDefault->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 4405 ) + GetDBString( 4377 ) );
			break;
		}
	}

	return CEmptyOptionsUI::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// COptionsInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class COptionsInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(COptionsInterface);
private:
	NInput::CBind bindExit;
	NInput::CBind bindVideoOptions, bindAudioOptions, bindGamePlayOptions, bindControlsOptions;

	ZDATA
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	CObj<NUI::CScreenShot> pScreenShot;
	////
	CObj<NUI::CWindow> pOptionsUI;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCursor); f.Add(3,&pInterface); f.Add(4,&pScreenShot); f.Add(5,&pOptionsUI); return 0; }

public:
	COptionsInterface();

	void Initialize( EOptionsScreen eScreen, NGScene::CBWScreenshotTexture *pScreenShotTexture );

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &eEvent );
	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
COptionsInterface::COptionsInterface():
	bindExit( "cancel" ), bindVideoOptions( "video" ), bindAudioOptions( "audio" ), bindGamePlayOptions( "gameplay" ), bindControlsOptions( "controls" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void COptionsInterface::Initialize( EOptionsScreen eScreen, NGScene::CBWScreenshotTexture *pScreenShotTexture )
{
	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pScreenShot = new NUI::CScreenShot( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "clues", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE | NUI::STYLE_BOTTOMMOST ) );
	if ( !IsValid( pScreenShotTexture ) )
		pScreenShot->Generate();
	else
		pScreenShot->SetTexture( pScreenShotTexture );

	switch( eScreen )
	{
	case OS_VIDEO:
		pOptionsUI = new NUI::CVideoOptionsUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "options", NUI::STYLE_ENABLED ) );
		NUI::LoadTemplate( pOptionsUI, NDb::GetUIContainer( 161 ) );
		break;
	case OS_AUDIO:
		pOptionsUI = new NUI::CAudioOptionsUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "options", NUI::STYLE_ENABLED ) );
		NUI::LoadTemplate( pOptionsUI, NDb::GetUIContainer( 162 ) );
		break;
	case OS_GAMEPLAY:
		pOptionsUI = new NUI::CGamePlayOptionsUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "options", NUI::STYLE_ENABLED ) );
		NUI::LoadTemplate( pOptionsUI, NDb::GetUIContainer( 163 ) );
		break;
	case OS_CONTROLS:
		pOptionsUI = new NUI::CControlsOptionsUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "options", NUI::STYLE_ENABLED ) );
		NUI::LoadTemplate( pOptionsUI, NDb::GetUIContainer( 164 ) );
		break;
	default:
		pOptionsUI = new NUI::CEmptyOptionsUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "options", NUI::STYLE_ENABLED ) );
		NUI::LoadTemplate( pOptionsUI, NDb::GetUIContainer( 170 ) );
		break;
	}

	pOptionsUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void COptionsInterface::Step()
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
void COptionsInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool COptionsInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}
	else if ( bindVideoOptions.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		NMainLoop::Command( new NGame::CICOptions( NGame::OS_VIDEO, pScreenShot->GetTexture() ) );
	}
	else if ( bindAudioOptions.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		NMainLoop::Command( new NGame::CICOptions( NGame::OS_AUDIO, pScreenShot->GetTexture() ) );
	}
	else if ( bindGamePlayOptions.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		NMainLoop::Command( new NGame::CICOptions( NGame::OS_GAMEPLAY, pScreenShot->GetTexture() ) );
	}
	else if ( bindControlsOptions.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		NMainLoop::Command( new NGame::CICOptions( NGame::OS_CONTROLS, pScreenShot->GetTexture() ) );
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void COptionsInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );

	pInterface->Draw( GetTime() );

	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICOptions::CICOptions( EOptionsScreen _eScreen, NGScene::CBWScreenshotTexture *_pScreenShotTexture ):
	eScreen( _eScreen ), pScreenShotTexture( _pScreenShotTexture )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICOptions::Exec()
{
	COptionsInterface *pRes = new COptionsInterface();
	pRes->Initialize( eScreen, pScreenShotTexture );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB0815150, COptionsUI );
REGISTER_SAVELOAD_CLASS( 0xB0815151, CVideoOptionsUI );
REGISTER_SAVELOAD_CLASS( 0xB0815152, CAudioOptionsUI );
REGISTER_SAVELOAD_CLASS( 0xB0815153, CGamePlayOptionsUI );
REGISTER_SAVELOAD_CLASS( 0xB0815154, CControlsOptionsUI );
REGISTER_SAVELOAD_CLASS( 0xB0815155, CComplexSlider );
REGISTER_SAVELOAD_CLASS( 0xB0815156, CComplexTextSlider );
REGISTER_SAVELOAD_CLASS( 0xB0815157, CComplexComboBox );
REGISTER_SAVELOAD_CLASS( 0xB0815158, CEmptyOptionsUI );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB081515A, COptionsInterface );
