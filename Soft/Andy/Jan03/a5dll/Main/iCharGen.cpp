#include "StdAfx.h"
#include "Gfx.h"
#include "iMain.h"
#include "G2DView.h"
#include "..\Misc\StrProc.h"
#include "..\MiscDll\Commands.h"
#include "..\Input\Bind.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
#include "RPGUnit.h"
#include "RPGMerc.h"
#include "RPGGlobal.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iCharGen.h"
#include "iFaceGen.h"
#include "iGlobalMap.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
const int N_STAT_MAX_VALUE = 10;
struct SClassToID
{
	const char *pszID;
	NDb::CSide::EPersClass eClass;
};
static SClassToID sClassTable[] = 
{
	"medic", NDb::CSide::MEDIC,
	"scout", NDb::CSide::SCOUT,
	"sniper", NDb::CSide::SNIPER,
	"soldier", NDb::CSide::SOLDIER,
	"engineer", NDb::CSide::ENGINEER,
	"grenadier", NDb::CSide::GRENADIER
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatPoints
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStatPoints: public CObjectBase
{
	OBJECT_BASIC_METHODS(CStatPoints);
private:
	ZDATA
	int nValue;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nValue); return 0; }

public:
	CStatPoints(): nValue( 0 ) {}

	bool Inc() { nValue++; return true; }
	bool Dec() { if ( nValue <= 0 ) return false; nValue--; return true; }

	int GetValue() const { return nValue; }
	void SetValue( int _nValue ) { nValue = _nValue; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatChange
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStatChange: public CWindow
{
	OBJECT_BASIC_METHODS(CStatChange);
private:
	ZDATA_(CWindow)
	int nValue;
	CPtr<CStatPoints> pPoints;
	////
	CPtr<CText> pText;
	CPtr<CComplexButton> pUp;
	CPtr<CComplexButton> pDown;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&nValue); f.Add(3,&pPoints); f.Add(4,&pText); f.Add(5,&pUp); f.Add(6,&pDown); return 0; }

public:
	CStatChange() {}
	CStatChange( const SWindowInfo &sInfo, CStatPoints *pPoints );

	int GetValue() const;
	void SetValue( int nValue );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CStatChange::CStatChange( const SWindowInfo &sInfo, CStatPoints *_pPoints ):
	CWindow( sInfo ), nValue( 0 ), pPoints( _pPoints )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CStatChange::GetValue() const
{
	return nValue;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CStatChange::SetValue( int _nValue )
{
	nValue = _nValue;

	WCHAR wsText[256];
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", nValue );
	pText->SetText( wsText );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStatChange::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "up" )
			{
				if ( GetValue() > 2 )
				{
					if ( pPoints->Inc() )
						SetValue( GetValue() - 1 );
				}
				return true;
			}
			else if ( sEvent.szID == "down" )
			{
				if ( GetValue() < N_STAT_MAX_VALUE )
				{
					if ( pPoints->Dec() )
						SetValue( GetValue() + 1 );
				}
				return true;
			}

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pUp = new CComplexButton( sEvent.pLoader->GetControl( "up" ), 0, 0, 0, 0 );
			pDown = new CComplexButton( sEvent.pLoader->GetControl( "down" ), 0, 0, 0, 0 );
			pUp->Set( NDb::GetUITexture( 398 ), 0, CComplexButton::NORMAL, "up" );
			pDown->Set( NDb::GetUITexture( 399 ), 0, CComplexButton::NORMAL, "down" );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pText = GetUIWindow<CText>( this, "text" );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCharGenUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCharGenUI: public CWindow
{
	OBJECT_BASIC_METHODS(CCharGenUI);
private:
	ZDATA_(CWindow)
	CObj<NRPG::CUnit> pMerc;
	CDBPtr<NDb::CSide> pSide;
	CDBPtr<NDb::CRPGPers> pPers;
	CDBPtr<NDb::CComplexHead> pHead;
	CDBPtr<NDb::CNationality> pNationality;
	////
	bool bChanged;
	////
	CObj<NRPG::CUnit> pTempMerc;
	CObj<CHoverButton> pPlay;
	CObj<CHoverButton> pBack;
	CObj<CInteractiveUnitView> pUnitView;
	////
	CObj<CComplexButton> pMale;
	CObj<CComplexButton> pFemale;
	////
	CObj<CComplexButton> pNation1;
	CObj<CComplexButton> pNation2;
	CObj<CComplexButton> pNation3;
	////
	vector<CObj<CComplexButton> > classIconsSet;
	////
	CPtr<CText> pPointsText;
	CObj<CStatPoints> pPoints;
	CObj<CStatChange> pStrength;
	CObj<CStatChange> pDexteriry;
	CObj<CStatChange> pIntelligence;
	////
	CPtr<CEdit> pName;
	CPtr<CEdit> pSurname;
	CPtr<CEdit> pNickname;
	////
	CPtr<CText> pEvasion;
	CPtr<CText> pActionPoints;
	CPtr<CText> pVitalityPoints;
	////
	CPtr<CText> pHide;
	CPtr<CText> pSpot;
	CPtr<CText> pBurst;
	CPtr<CText> pMelee;
	CPtr<CText> pSnipe;
	CPtr<CText> pMedicine;
	CPtr<CText> pShooting;
	CPtr<CText> pThrowing;
	CPtr<CText> pInterrupt;
	CPtr<CText> pEngineering;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMerc); f.Add(3,&pSide); f.Add(4,&pPers); f.Add(5,&pHead); f.Add(6,&pNationality); f.Add(7,&bChanged); f.Add(8,&pTempMerc); f.Add(9,&pPlay); f.Add(10,&pBack); f.Add(11,&pUnitView); f.Add(12,&pMale); f.Add(13,&pFemale); f.Add(14,&pNation1); f.Add(15,&pNation2); f.Add(16,&pNation3); f.Add(17,&classIconsSet); f.Add(18,&pPointsText); f.Add(19,&pPoints); f.Add(20,&pStrength); f.Add(21,&pDexteriry); f.Add(22,&pIntelligence); f.Add(23,&pName); f.Add(24,&pSurname); f.Add(25,&pNickname); f.Add(26,&pEvasion); f.Add(27,&pActionPoints); f.Add(28,&pVitalityPoints); f.Add(29,&pHide); f.Add(30,&pSpot); f.Add(31,&pBurst); f.Add(32,&pMelee); f.Add(33,&pSnipe); f.Add(34,&pMedicine); f.Add(35,&pShooting); f.Add(36,&pThrowing); f.Add(37,&pInterrupt); f.Add(38,&pEngineering); return 0; }

protected:
	void Generate();
	void GenerateEmpty();
	void UpdateNationalIcons();
	void UpdateUnit();

public:
	CCharGenUI() {}
	CCharGenUI( const SWindowInfo &sInfo, NDb::CSide *pSide );

	NDb::CRPGPers * GetPers() const;
	NDb::CComplexHead* GetHead() const;
	NDb::CNationality* GetNationality() const;

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CCharGenUI::CCharGenUI( const SWindowInfo &sInfo, NDb::CSide *_pSide ): 
	CWindow( sInfo ), pSide( _pSide ), bChanged( false )
{
	pPoints = new CStatPoints;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CRPGPers* CCharGenUI::GetPers() const
{
	return pPers;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CComplexHead* CCharGenUI::GetHead() const
{
	return pHead;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CNationality* CCharGenUI::GetNationality() const
{
	return pNationality;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CCharGenUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_NOTIFY:
		{
			if ( sEvent.szID == "default" )
			{
				bChanged = true;
				return true;
			}
			else if ( sEvent.szID == "reset" )
			{
				pPoints->SetValue( pPoints->GetValue() + pStrength->GetValue() - 2 );
				pStrength->SetValue( 2 );

				pPoints->SetValue( pPoints->GetValue() + pDexteriry->GetValue() - 2 );
				pDexteriry->SetValue( 2 );

				pPoints->SetValue( pPoints->GetValue() + pIntelligence->GetValue() - 2 );
				pIntelligence->SetValue( 2 );
				return true;
			}

			int nProfession = -1;
			bool bMale = false, bFemale = false;
			bool bNational0 = false, bNational1 = false, bNational2 = false;
			if ( sEvent.szID == "male" )
				bMale = true;
			else if ( sEvent.szID == "female" )
				bFemale = true;
			if ( bMale || bFemale )
			{
				bChanged = true;
				pMale->SetChecked( bMale );
				pFemale->SetChecked( bFemale );
				return true;
			}

			if ( sEvent.szID == "national_0" )
				bNational0 = true;
			else if ( sEvent.szID == "national_1" )
				bNational1 = true;
			else if ( sEvent.szID == "national_2" )
				bNational2 = true;
			if ( bNational0 || bNational1 || bNational2 )
			{
				bChanged = true;
				pNation1->SetChecked( bNational0 );
				pNation2->SetChecked( bNational1 );
				pNation3->SetChecked( bNational2 );
				return true;
			}

			for ( int nTemp = 0; nTemp < classIconsSet.size(); nTemp++ )
			{
				if ( classIconsSet[nTemp]->GetWindowID() == sEvent.szID )
					nProfession = nTemp;
			}
			if ( nProfession != -1 )
			{
				bChanged = true;

				for ( int nTemp = 0; nTemp < classIconsSet.size(); nTemp++ )
					classIconsSet[nTemp]->SetChecked( false );

				classIconsSet[nProfession]->SetChecked( true );
				return true;
			}

			break;
		}
	case EVENT_TEMPLATELOAD:
		{
			pPlay = new CHoverButton( sEvent.pLoader->GetControl( "play" ) );
			pPlay->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10894 ) + GetDBString( 10896 ) );
			pPlay->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10892 ) + GetDBString( 10896 ) );
			pPlay->AddTextState( CHoverButton::STATE_DISABLED, GetDBString( 10893 ) + GetDBString( 10896 ) );

			pBack = new CHoverButton( sEvent.pLoader->GetControl( "cancel" ) );
			pBack->AddTextState( CHoverButton::STATE_HOVER, GetDBString( 10894 ) + GetDBString( 10895 ) );
			pBack->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 10892 ) + GetDBString( 10895 ) );
			pBack->AddTextState( CHoverButton::STATE_DISABLED, GetDBString( 10893 ) + GetDBString( 10895 ) );

			pUnitView = new CInteractiveUnitView( sEvent.pLoader->GetControl( "unitshow" ) );

			pMale = new CComplexButton( sEvent.pLoader->GetControl( "male" ), 0, 0, NDb::GetUITexture( 566 ), NDb::GetUITexture( 408 ) );
			pFemale = new CComplexButton( sEvent.pLoader->GetControl( "female" ), 0, 0, NDb::GetUITexture( 566 ), NDb::GetUITexture( 408 ) );
			pMale->Set( NDb::GetUITexture( 580 ), NDb::GetUITexture( 582 ), CComplexButton::UNCHECKED, "male" );
			pFemale->Set( NDb::GetUITexture( 581 ), NDb::GetUITexture( 583 ), CComplexButton::UNCHECKED, "female" );

			pNation1 = new CComplexButton( sEvent.pLoader->GetControl( "national_0" ), 0, 0, NDb::GetUITexture( 566 ), NDb::GetUITexture( 408 ) );
			pNation2 = new CComplexButton( sEvent.pLoader->GetControl( "national_1" ), 0, 0, NDb::GetUITexture( 566 ), NDb::GetUITexture( 408 ) );
			pNation3 = new CComplexButton( sEvent.pLoader->GetControl( "national_2" ), 0, 0, NDb::GetUITexture( 566 ), NDb::GetUITexture( 408 ) );
			if ( IsValid( pSide->pNationality1 ) )
			{
				pNation1->Set( pSide->pNationality1->pIconNormal, pSide->pNationality1->pIconDisabled, CComplexButton::UNCHECKED, "national_0" );
				pNation1->GetToolTip()->SetText( GetDBString( pSide->pNationality1->pToolTip ) );
			}
			if ( IsValid( pSide->pNationality2 ) )
			{
				pNation2->Set( pSide->pNationality2->pIconNormal, pSide->pNationality2->pIconDisabled, CComplexButton::UNCHECKED, "national_1" );
				pNation2->GetToolTip()->SetText( GetDBString( pSide->pNationality2->pToolTip ) );
			}
			if ( IsValid( pSide->pNationality3 ) )
			{
				pNation3->Set( pSide->pNationality3->pIconNormal, pSide->pNationality3->pIconDisabled, CComplexButton::UNCHECKED, "national_2" );
				pNation3->GetToolTip()->SetText( GetDBString( pSide->pNationality3->pToolTip ) );
			}

			const vector<CPtr<NDb::CRPGPers> > &classes = pSide->malePersesSet;

			classIconsSet.resize( ARRAY_SIZE( sClassTable ) );
			for ( int nTemp = 0; nTemp < ARRAY_SIZE( sClassTable ); nTemp++ )
			{
				CPtr<NDb::CRPGPers> pTempPers = classes[sClassTable[nTemp].eClass];
				CPtr<CComplexButton> pButton = new CComplexButton( sEvent.pLoader->GetControl( sClassTable[nTemp].pszID ), 0, 0, NDb::GetUITexture( 566 ), NDb::GetUITexture( 408 ) );

				if ( IsValid( pTempPers ) && IsValid( pTempPers->pClass ) )
					pButton->Set( pTempPers->pClass->pIcon, pTempPers->pClass->pIconDisabled, CComplexButton::UNCHECKED );

				classIconsSet[nTemp] = pButton;
			}

			pStrength = new CStatChange( sEvent.pLoader->GetControl( "strength" ), pPoints );
			pDexteriry = new CStatChange( sEvent.pLoader->GetControl( "dexterity" ), pPoints );
			pIntelligence = new CStatChange( sEvent.pLoader->GetControl( "intelligence" ), pPoints );
			break;
		}
	case EVENT_TEMPLATELOADCOMPLETE:
		{
			pName = GetUIWindow<CEdit>( this, "name" );
			pSurname = GetUIWindow<CEdit>( this, "surname" );
			pNickname = GetUIWindow<CEdit>( this, "nickname" );
			pName->SetTextFormat( GetDBString( 10899 ) );
			pSurname->SetTextFormat( GetDBString( 10899 ) );
			pNickname->SetTextFormat( GetDBString( 10899 ) );

			pPointsText = GetUIWindow<CText>( this, "points" );

			pEvasion = GetUIWindow<CText>( this, "evasion" );
			pActionPoints = GetUIWindow<CText>( this, "ap" );
			pVitalityPoints = GetUIWindow<CText>( this, "vp" );

			pHide = GetUIWindow<CText>( this, "hide" );
			pSpot = GetUIWindow<CText>( this, "spot" );
			pBurst = GetUIWindow<CText>( this, "burst" );
			pMelee = GetUIWindow<CText>( this, "melee" );
			pSnipe = GetUIWindow<CText>( this, "snipe" );
			pMedicine = GetUIWindow<CText>( this, "medicine" );
			pShooting = GetUIWindow<CText>( this, "shooting" );
			pThrowing = GetUIWindow<CText>( this, "throwing" );
			pInterrupt = GetUIWindow<CText>( this, "interrupt" );
			pEngineering = GetUIWindow<CText>( this, "engineering" );
			break;
		}
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharGenUI::Draw( const STime &sTime, NGScene::I2DGameView *pView )
{
	bool bEnable = true;
	bool bSomethingVisible = false, bFlagVisible = false;

	pMale->SetStyle( STYLE_ENABLED, bEnable );
	pFemale->SetStyle( STYLE_ENABLED, bEnable );

	bEnable = bEnable && ( pMale->IsChecked() || pFemale->IsChecked() );
	bSomethingVisible = bEnable;

	pNation1->SetStyle( STYLE_ENABLED, bEnable );
	pNation2->SetStyle( STYLE_ENABLED, bEnable );
	pNation3->SetStyle( STYLE_ENABLED, bEnable );

	bEnable = bEnable && ( pNation1->IsChecked() || pNation2->IsChecked() || pNation3->IsChecked() );
	bFlagVisible = bEnable;

	for ( int nTemp = 0; nTemp < classIconsSet.size(); nTemp++ )
		classIconsSet[nTemp]->SetStyle( STYLE_ENABLED, bEnable );

	bool bChecked = false;
	for ( int nTemp = 0; nTemp < classIconsSet.size(); nTemp++ )
		bChecked = bChecked || classIconsSet[nTemp]->IsChecked();

	bEnable = bEnable && bChecked;

	pPointsText->SetStyle( STYLE_VISIBLE, bEnable );
	pStrength->SetStyle( STYLE_VISIBLE, bEnable );
	pDexteriry->SetStyle( STYLE_VISIBLE, bEnable );
	pIntelligence->SetStyle( STYLE_VISIBLE, bEnable );

	if ( bEnable && bChanged )
	{
		bChanged = false;
		pPoints->SetValue( 0 );
		UpdateUnit();

		if ( IsValid( pMerc ) )
		{
			NRPG::CUnit* pUnit = pMerc;

			pUnitView->SetUnit( pUnit );
			pUnitView->SetStyle( STYLE_VISIBLE, true );

			pStrength->SetValue( (int)pUnit->Skills( NDb::ST_STR ) );
			pDexteriry->SetValue( (int)pUnit->Skills( NDb::ST_DEX ) );
			pIntelligence->SetValue( (int)pUnit->Skills( NDb::ST_INT ) );
		}
	}
	else if ( bSomethingVisible && bChanged )
	{
		bChanged = false;

		if ( pMale->IsChecked() )
			pTempMerc = NRPG::CreateMerc( NDb::GetPers( 54 ) );
		else if ( pFemale->IsChecked() )
			pTempMerc = NRPG::CreateMerc( NDb::GetPers( 3 ) );

		if ( IsValid( pTempMerc ) )
		{
			pUnitView->SetUnit( pTempMerc );
			pUnitView->SetStyle( STYLE_VISIBLE, true );
		}
	}

	if ( IsValid( pMerc ) && bEnable )
	{
		NRPG::CUnit* pUnit = pMerc;

		WCHAR wsText[256];
		swprintf( wsText, L"<font face=Courier size=16pt><color=black><center>%d", pPoints->GetValue() );
		pPointsText->SetText( wsText );

		pUnit->Skills( NDb::ST_STR ).SetNewBaseValue( pStrength->GetValue() );
		pUnit->Skills( NDb::ST_DEX ).SetNewBaseValue( pDexteriry->GetValue() );
		pUnit->Skills( NDb::ST_INT ).SetNewBaseValue( pIntelligence->GetValue() );
		pUnit->UpdateSkills();

		Generate();
	}
	else
		GenerateEmpty();

	if ( bEnable && !pNickname->GetText().empty() && ( pPoints->GetValue() == 0 ) )
	{
		pMerc->SetName( pNickname->GetText() );

		pPlay->SetStyle( STYLE_ENABLED, true );
	}
	else
		pPlay->SetStyle( STYLE_ENABLED, false );

	CWindow::Draw( sTime, pView );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharGenUI::Generate()
{
	if ( !IsValid( pMerc ) )
		return;

	NRPG::CUnit* pUnit = pMerc;

	WCHAR wsText[256];

	pStrength->SetValue( (int)pUnit->Skills( NDb::ST_STR ) );
	pDexteriry->SetValue( (int)pUnit->Skills( NDb::ST_DEX ) );
	pIntelligence->SetValue( (int)pUnit->Skills( NDb::ST_INT ) );

	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_IC ) );
	pEvasion->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_AP ) );
	pActionPoints->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_VP ) );
	pVitalityPoints->SetText( wsText );

	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_STEALTH ) );
	pHide->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_SPOT ) );
	pSpot->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_BURST ) );
	pBurst->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_MELEE ) );
	pMelee->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_SNIPE ) );
	pSnipe->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_MEDICINE ) );
	pMedicine->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_SHOOTING ) );
	pShooting->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_THROWING ) );
	pThrowing->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_INTERRUPT ) );
	pInterrupt->SetText( wsText );
	swprintf( wsText, L"<font face=Courier size=16pt><center>%d", (int)pUnit->Skills( NDb::ST_ENGINEERING ) );
	pEngineering->SetText( wsText );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharGenUI::GenerateEmpty()
{
	pEvasion->SetText( L"" );
	pActionPoints->SetText( L"" );
	pVitalityPoints->SetText( L"" );

	pHide->SetText( L"" );
	pSpot->SetText( L"" );
	pBurst->SetText( L"" );
	pMelee->SetText( L"" );
	pSnipe->SetText( L"" );
	pMedicine->SetText( L"" );
	pShooting->SetText( L"" );
	pThrowing->SetText( L"" );
	pInterrupt->SetText( L"" );
	pEngineering->SetText( L"" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharGenUI::UpdateNationalIcons()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharGenUI::UpdateUnit()
{
	if ( pNation1->IsChecked() )
		pNationality = pSide->pNationality1;
	else if ( pNation2->IsChecked() )
		pNationality = pSide->pNationality2;
	else if ( pNation3->IsChecked() )
		pNationality = pSide->pNationality3;

	vector<CPtr<NDb::CRPGPers> > classes;
	if ( pMale->IsChecked() )
	{
		pHead = pNationality->pMaleHead;
		classes = pSide->malePersesSet;
	}
	else if ( pFemale->IsChecked() )
	{
		pHead = pNationality->pFemaleHead;
		classes = pSide->femalePersesSet;
	}

	for ( int nTemp = 0; nTemp < classIconsSet.size(); nTemp++ )
	{
		if ( classIconsSet[nTemp]->IsChecked() )
		{
			ASSERT( nTemp < ARRAY_SIZE(sClassTable) );
			pPers = classes[sClassTable[nTemp].eClass];
			break;
		}
	}
	pMerc = NRPG::CreateMerc( pPers, pHead );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCharGenMenuInterface
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCharGenMenuInterface: public NMainLoop::IInterfaceBase
{
	OBJECT_BASIC_METHODS(CCharGenMenuInterface);
private:
	NInput::CBind bindClose, bindPlay;

	ZDATA
	CDBPtr<NDb::CSide> pSide;
	////
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	////
	CObj<NUI::CCharGenUI> pMenuUI;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pSide); f.Add(3,&pCursor); f.Add(4,&pInterface); f.Add(5,&pMenuUI); return 0; }

public:
	CCharGenMenuInterface();

	void Initialize( NDb::CSide *pSide );

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &eEvent );
	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CCharGenMenuInterface::CCharGenMenuInterface():
	bindClose( "cancel" ), bindPlay( "play" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharGenMenuInterface::Initialize( NDb::CSide *_pSide )
{
	pSide = _pSide;

	pCursor = NUI::ICursor::Create();
	pInterface = new NUI::CInterface( pCursor );

	pMenuUI = new NUI::CCharGenUI( NUI::SWindowInfo( pInterface, NUI::SPoint( 0, 0 ), NUI::SPoint( 1024, 768 ), "chargenUI", NUI::STYLE_ENABLED ), pSide );
	NUI::LoadTemplate( pMenuUI, NDb::GetUIContainer( 345 ) );
	pMenuUI->ShowWindow( NUI::SWTYPE_SHOW );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharGenMenuInterface::Step()
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
void CCharGenMenuInterface::OnGetFocus()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CCharGenMenuInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	NInput::SetSection( "menu" );

	pCursor->ProcessEvent( sEvent );

	if ( pInterface->ProcessEvent( sEvent ) )
		return true;

	if ( bindClose.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NMainLoop::CICExitModal() );
		return true;
	}
	else if ( bindPlay.ProcessEvent( sEvent ) )
	{
		NMainLoop::Command( new NGame::CICFaceGen( pSide, pMenuUI->GetPers(), pMenuUI->GetHead() ) ); 
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CCharGenMenuInterface::RenderFrame()
{
	NGScene::ClearScreen( CVec3(0.5f, 0.5f, 0.5f ) );
	pInterface->Draw( GetTime() );
	NGScene::Flip();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICCharGen::CICCharGen( NDb::CSide *_pSide ):
	pSide( _pSide )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICCharGen::Exec()
{
	CCharGenMenuInterface *pRes = new CCharGenMenuInterface();
	pRes->Initialize( pSide );
	PushInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB1025160, CStatChange );
REGISTER_SAVELOAD_CLASS( 0xB1025161, CCharGenUI );
REGISTER_SAVELOAD_CLASS( 0xB1025162, CStatPoints );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB1023180, CCharGenMenuInterface );
