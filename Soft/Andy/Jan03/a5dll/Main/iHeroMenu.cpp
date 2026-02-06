#include "StdAfx.h"
#include "Transform.h"
#include "GView.h"
#include "G2DView.h"
#include "GSceneUtils.h"
#include "wInterface.h"
#include "Sound.h"
#include "RWGame.h"
#include "RWSound.h"
#include "RPGGame.h"
#include "RPGGlobal.h"
#include "RPGMerc.h"
#include "Interface.h"
#include "iMain.h"
#include "iCharGen.h"
#include "iFaceGen.h"
#include "iHeroMenu.h"
#include "iRenderWorld.h"
#include "iCommonUI.h"
#include "iGlobalMap.h"
#include "..\Misc\StrProc.h"
#include "..\MiscDll\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataSound.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataCamera.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_HEROMENU_CAMERA = 44;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScriptButton
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScriptButton: public CButton
{
	OBJECT_BASIC_METHODS(CScriptButton);
private:
	ZDATA_(CButton)
	CPtr<NGame::CRenderBaseInterface> pInterface;
	////
	bool bHoverNotify;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CButton*)this); f.Add(2,&pInterface); f.Add(3,&bHoverNotify); return 0; }

protected:
	virtual void OnAction();

public:
	CScriptButton() {}
	CScriptButton( const SWindowInfo &sInfo, NGame::CRenderBaseInterface *pInterface );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CScriptButton::CScriptButton( const SWindowInfo &sInfo, NGame::CRenderBaseInterface *_pInterface ):
	CButton( sInfo ), pInterface( _pInterface ), bHoverNotify( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScriptButton::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	if ( IsMouseCover() != bHoverNotify )
	{
		NWorld::CCommand *pCmd = 
			new NWorld::CCmdCallScriptFunction( "OnScriptNotify", "si", GetWindowID().c_str(), IsMouseCover() ? 1 : 2 );
		pInterface->Command( pCmd );
	}

	bHoverNotify = IsMouseCover();

	CButton::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScriptButton::OnAction()
{
	NWorld::CCommand *pCmd = new NWorld::CCmdCallScriptFunction( "OnScriptNotify", "si", GetWindowID().c_str(), 0 );
	pInterface->Command( pCmd );
	CButton::OnAction();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CHeroMenuUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeroMenuUI: public CWindow
{
	OBJECT_BASIC_METHODS(CHeroMenuUI);
private:
	ZDATA_(CWindow)
	CDBPtr<NDb::CSide> pSide;
	CPtr<NGame::CRenderBaseInterface> pInterface;
	CDBPtr<NDb::CRPGPers> pSelectedPers;
	////
	CObj<CHoverButton> pBack;
	CObj<CHoverButton> pPlay;
	CObj<CHoverButton> pCustomChar;
	CObj<CScriptButton> pNat1Male;
	CObj<CScriptButton> pNat1Female;
	CObj<CScriptButton> pNat2Male;
	CObj<CScriptButton> pNat2Female;
	CObj<CScriptButton> pNat3Male;
	CObj<CScriptButton> pNat3Female;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pSide); f.Add(3,&pInterface); f.Add(4,&pSelectedPers); f.Add(5,&pBack); f.Add(6,&pPlay); f.Add(7,&pCustomChar); f.Add(8,&pNat1Male); f.Add(9,&pNat1Female); f.Add(10,&pNat2Male); f.Add(11,&pNat2Female); f.Add(12,&pNat3Male); f.Add(13,&pNat3Female); return 0; }
public:
	CHeroMenuUI() {}
	CHeroMenuUI( const SWindowInfo &sInfo, NGame::CRenderBaseInterface *pInterface, NDb::CSide *pSide );

	NDb::CRPGPers* GetPers() const;

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CHeroMenuUI::CHeroMenuUI( const SWindowInfo &sInfo, NGame::CRenderBaseInterface *_pInterface, NDb::CSide *_pSide ): 
	CWindow( sInfo ), pInterface( _pInterface ), pSide( _pSide )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CRPGPers* CHeroMenuUI::GetPers() const
{
	return pSelectedPers;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CHeroMenuUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "n1male" )
				pSelectedPers = pSide->defaultPersesSet[0];
			else if ( sEvent.szID == "n1female" )
				pSelectedPers = pSide->defaultPersesSet[1];
			else if ( sEvent.szID == "n2male" )
				pSelectedPers = pSide->defaultPersesSet[2];
			else if ( sEvent.szID == "n2female" )
				pSelectedPers = pSide->defaultPersesSet[3];
			else if ( sEvent.szID == "n3male" )
				pSelectedPers = pSide->defaultPersesSet[4];
			else if ( sEvent.szID == "n3female" )
				pSelectedPers = pSide->defaultPersesSet[5];

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pBack = new CHoverButton( sEvent.pLoader->GetControl( "cancel" ) );
			pBack->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11133 ) + GetDBString( 11135 ) );
			pBack->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11134 ) + GetDBString( 11135 ) );

			pPlay = new CHoverButton( sEvent.pLoader->GetControl( "play" ) );
			pPlay->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11133 ) + GetDBString( 11137 ) );
			pPlay->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11134 ) + GetDBString( 11137 ) );
			pPlay->AddTextState( CHoverButton::STATE_DISABLED, GetDBString( 11134 ) + GetDBString( 11137 ) );

			pCustomChar = new CHoverButton( sEvent.pLoader->GetControl( "customchar" ) );
			pCustomChar->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 11133 ) + GetDBString( 11136 ) );
			pCustomChar->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 11134 ) + GetDBString( 11136 ) );

			pNat1Male = new CScriptButton( sEvent.pLoader->GetControl( "n1male" ), pInterface );
			pNat1Female = new CScriptButton( sEvent.pLoader->GetControl( "n1female" ), pInterface );
			pNat2Male = new CScriptButton( sEvent.pLoader->GetControl( "n2male" ), pInterface );
			pNat2Female = new CScriptButton( sEvent.pLoader->GetControl( "n2female" ), pInterface );
			pNat3Male = new CScriptButton( sEvent.pLoader->GetControl( "n3male" ), pInterface );
			pNat3Female = new CScriptButton( sEvent.pLoader->GetControl( "n3female" ), pInterface );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CHeroMenuUI::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	pPlay->SetStyle( STYLE_ENABLED, IsValid( pSelectedPers ) );
	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CHeroMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeroMenuInterface: public CRenderBaseInterface
{
	OBJECT_BASIC_METHODS(CHeroMenuInterface);
private:
	NInput::CBind bindClose, bindPlay, bindCustomChar;

	ZDATA_(CRenderBaseInterface)
	CDBPtr<NDb::CSide> pSide;
	////
	CObj<NUI::CHeroMenuUI> pHeroMenuUI;
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CRenderBaseInterface*)this); f.Add(2,&pSide); f.Add(3,&pHeroMenuUI); f.Add(4,&pGlobalPlayer); return 0; }

public:
	CHeroMenuInterface();

	void Initialize( NDb::CSide *pSide );

	void Step();
	bool ProcessEvent( const NInput::SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CHeroMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
CHeroMenuInterface::CHeroMenuInterface():
	bindClose( "cancel" ), bindPlay( "play" ), bindCustomChar( "customchar" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CHeroMenuInterface::Initialize( NDb::CSide *_pSide )
{
	pSide = _pSide;

	CRenderBaseInterface::Initialize( pSide->nHeroSelectTemplate );

	CPtr<NDb::CDBCamera> pDBCamera = NDb::GetDBCamera( N_HEROMENU_CAMERA );
	ICamera::SCameraPos sCameraPos( pDBCamera->vAnchor, pDBCamera->fDistance, pDBCamera->fPitch, pDBCamera->fYaw, pDBCamera->fRoll, pDBCamera->fFOV );
	GetCamera()->SetPlacement( sCameraPos );

	pGlobalPlayer = NRPG::CreateGlobalPlayer();

	pHeroMenuUI = new NUI::CHeroMenuUI( NUI::SWindowInfo( GetInterface(), NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "heromenuUI" ), this, pSide );
	NUI::LoadTemplate( pHeroMenuUI, NDb::GetUIContainer( 354 ) );
	pHeroMenuUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CHeroMenuInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	NInput::SetSection( "menu" );

	if ( CRenderBaseInterface::ProcessEvent( sEvent ) )
		return true;

	if ( bindClose.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() ); 
		return true;
	}
	else if ( bindPlay.ProcessEvent( sEvent ) )
	{
		if ( !IsValid( pHeroMenuUI->GetPers() ) )
			return true;

		NMainLoop::Command( new NGame::CICFaceGen( pSide, pHeroMenuUI->GetPers(), 0 ) ); 
		return true;
	}
	else if ( bindCustomChar.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NGame::CICCharGen( pSide ) ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CHeroMenuInterface::Step()
{
	CRenderBaseInterface::Step();

	if ( CanRender() )
	{
		NUI::SRect sScrWindow;
		NUI::SPoint sScrPosition;
		NUI::CWindow *pClientWindow = NUI::GetUIWindow<NUI::CWindow>( pHeroMenuUI, "clientview" );
		const NUI::SPoint &sScrSize = pClientWindow->GetSize();
		pClientWindow->ClientToScreen( &sScrPosition, &sScrWindow );

		NUI::SRect sScrClientRect( sScrPosition.x, sScrPosition.y, sScrPosition.x + sScrSize.x, sScrPosition.y + sScrSize.y );
		GetCamera()->SetScreenRect( CTRect<float>( float( sScrClientRect.x1 ) / 1024.0f, float( sScrClientRect.y1 ) / 768.0f, float( sScrClientRect.x2 ) / 1024.0f, float( sScrClientRect.y2 ) / 768.0f ) );

		RenderFrame( GetTime(), GetCamera() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMainMenu
////////////////////////////////////////////////////////////////////////////////////////////////////
CICHeroMenu::CICHeroMenu( NDb::CSide *_pSide ):
	pSide( _pSide )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICHeroMenu::Exec()
{
	CHeroMenuInterface *pRes = new CHeroMenuInterface;
	pRes->Initialize( pSide );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB1112190, CHeroMenuUI );
REGISTER_SAVELOAD_CLASS( 0xB1112191, CScriptButton );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB111219A, CHeroMenuInterface );
