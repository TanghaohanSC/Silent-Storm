#include "StdAfx.h"
#include "GView.h"
#include "G2DView.h"
#include "wInterface.h"
#include "RWGame.h"
#include "RPGItemInfo.h"
#include "RPGUnitInfo.h"
#include "..\Input\Bind.h"
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "iMain.h"
#include "iMissionGame.h"
#include "Interface.h"
#include "IDecorators.h"
#include "iCommonUI.h"
#include "iGround.h"
#include "iGroundIC.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_GROUND_WIDTH		= 36,
	N_GROUND_HEIGHT		= 21;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGroundSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
CGroundSlot::CGroundSlot( IWindow *pContainer, NGame::IMissionGame *_pGame, NWorld::CUnit *_pUnit ):
	CSlot( pContainer, _pGame, N_GROUND_WIDTH, N_GROUND_HEIGHT ), pGame( _pGame ), pUnit( _pUnit ), groundMap( N_GROUND_WIDTH, N_GROUND_HEIGHT )
{
	groundMap.FillEvery( false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGroundSlot::CanPlaceOnGround( const CTPoint<int> &sPos, NRPG::IInventoryItem *pItem ) const
{
	if ( ( sPos.x + pItem->GetSize().x >= groundMap.GetXSize() ) || ( sPos.y + pItem->GetSize().y >= groundMap.GetYSize() ) )
		return false;

	for( int nTempY = 0; nTempY < pItem->GetSize().y; nTempY++ )
	{
		for( int nTempX = 0; nTempX < pItem->GetSize().x; nTempX++ )
		{
			if ( groundMap[sPos.y + nTempY][sPos.x + nTempX] )
				return false;
		}
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGroundSlot::FindNewPlaceOnGround( NRPG::IInventoryItem *pItem, CTPoint<int> *pPos )
{
	bool bPosSet = false;
	for( int nTempY = 0; nTempY < groundMap.GetYSize(); nTempY++ )
	{
		for( int nTempX = 0; nTempX < groundMap.GetXSize(); nTempX++ )
		{
			if ( CanPlaceOnGround( CTPoint<int>( nTempX, nTempY ), pItem ) )
			{
				pPos->x = nTempX;
				pPos->y = nTempY;
				bPosSet = true;
				break;
			}
		}

		if ( bPosSet )
			break;
	}

	return bPosSet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGroundSlot::Place( int nX, int nY, NRPG::IInventoryItem *pItem )
{
	SPoint sPos;
	GetInSlotPos( nX, nY, &sPos );

	NWorld::SItem sTarget;
	sTarget.eType = NWorld::SItem::GROUND;
	sTarget.pUnit = pUnit;
	sTarget.sPosition = sPos;

	pGame->Command( pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::CCmdMoveInventoryItem::UNIT_OR_GROUND, sTarget ) );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGroundSlot::Activate( int nX, int nY, NRPG::IInventoryItem *pItem )
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGroundSlot::GetItemsList( vector<SItem> *pItemsSet )
{
	vector<NWorld::SItem> groundItemsSet;
	pGame->GetWorld()->FindCloseGroundItems( pUnit, &groundItemsSet );

	vector<SItemIcon> newItemsSet( groundItemsSet.size() );
	for ( int nTemp = 0; nTemp < groundItemsSet.size(); nTemp++ )
	{
		const NWorld::SItem &sWorldItem = groundItemsSet[nTemp];

		bool bExist = false;
		for ( int nItem = 0; nItem < itemsSet.size(); nItem++ )
		{
			if ( itemsSet[nItem].pItem != sWorldItem.pItem )
				continue;

			bExist = true;
			newItemsSet[nTemp] = itemsSet[nItem];
			break;
		}
		if ( bExist )
			continue;

		CTPoint<int> sPoint;
		if ( FindNewPlaceOnGround( sWorldItem.pItem, &sPoint ) )
		{
			SItemIcon &sItem = newItemsSet[nTemp];
			sItem.sPos = sPoint;
			sItem.nSlot = sWorldItem.nSlot;
			sItem.pUnit = sWorldItem.pUnit;
			sItem.pItem = sWorldItem.pItem;

			for( int nTempY = 0; nTempY < sItem.pItem->GetSize().y; nTempY++ )
				for( int nTempX = 0; nTempX < sItem.pItem->GetSize().x; nTempX++ )
					groundMap[sPoint.y + nTempY][sPoint.x + nTempX] = true;
		}
	}

	itemsSet = newItemsSet;

	pItemsSet->resize( itemsSet.size() );
	for ( int nItem = 0; nItem < itemsSet.size(); nItem++ )
	{
		(*pItemsSet)[nItem].bActive = false;
		(*pItemsSet)[nItem].sPos = itemsSet[nItem].sPos;
		(*pItemsSet)[nItem].nSlot = itemsSet[nItem].nSlot;
		(*pItemsSet)[nItem].pItem = itemsSet[nItem].pItem;
		(*pItemsSet)[nItem].pUnit = itemsSet[nItem].pUnit;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGroundSlot::CreateDragAndDrop( const SItem &sItem )
{
	NWorld::SItem sInvItem;
	sInvItem.eType = NWorld::SItem::GROUND;
	sInvItem.nSlot = sItem.nSlot;
	sInvItem.pItem = sItem.pItem;
	sInvItem.pUnit = sItem.pUnit;
	sInvItem.sPosition = sItem.sPos;
	pGame->Command( pUnit, new NWorld::CCmdMoveInventoryItem( NWorld::CCmdMoveInventoryItem::PLAYERHAND, sInvItem ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGroundUI
////////////////////////////////////////////////////////////////////////////////////////////////////
CGroundUI::CGroundUI( IWindow *pContainer, NGame::IMissionGame *_pGame, NWorld::CUnit *_pUnit ):
	NGame::CDesktop( pContainer ), pGame( _pGame ), pUnit( _pUnit )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGroundUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOAD:
		{
//			pUnitPanel = new CUnitPanel( GetUIWindow<IWindow>( this, "unitpanel", sEvent.pLoader ), pGame );
			pControlPanel = new CControlPanel( GetUIWindow<IWindow>( this, "controlpanel", sEvent.pLoader ), pGame, NGame::GROUNDAREA );

			pGroundSlot = new CGroundSlot( GetUIWindow<IWindow>( this, "ground", sEvent.pLoader ), pGame, pUnit );
			break;
		}
	}

	return NGame::CDesktop::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGroundUI::Draw( const SScene &sScene )
{
	NGame::CDesktop::Draw( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMissionInventory
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGroundInterface: public NMainLoop::CInterfaceBase
{
	OBJECT_NOCOPY_METHODS(CGroundInterface);
private:
	NInput::CBind bindExit, bindGround;
	ZDATA_(NMainLoop::CInterfaceBase)
	CObj<IMissionGame> pGame;
	////
	CPtr<NWorld::CUnit> pUnit;
	CObj<NUI::IWindow> pGround;
	CObj<NUI::CGroundUI> pGroundCtrl;
	vector< CPtr<IUnitTracker> > selectedUnitsSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(NMainLoop::CInterfaceBase*)this); f.Add(2,&pGame); f.Add(3,&pUnit); f.Add(4,&pGround); f.Add(5,&pGroundCtrl); f.Add(6,&selectedUnitsSet); return 0; }

public:
	CGroundInterface();
	~CGroundInterface();

	void Initialize( IMissionGame *pGame );
	void Terminate();
	void Step();
	bool ProcessEvent( const NInput::SEvent &eEvent );

	void RenderFrame();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CGroundInterface::CGroundInterface():	
	bindExit( "cancel" ), bindGround( "groundarea" )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CGroundInterface::~CGroundInterface()
{
	Terminate();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGroundInterface::Initialize( IMissionGame *_pGame )
{
	pGame = _pGame;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGroundInterface::Terminate()
{
	if ( IsValid( pGame ) && IsValid( pGroundCtrl ) )
		pGame->PopDesktop();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGroundInterface::Step()
{
	pGame->Step();

	vector< CPtr<IUnitTracker> > newSelectedUnitsSet;
	pGame->GetSelectedUnits( &newSelectedUnitsSet );
	if ( newSelectedUnitsSet != selectedUnitsSet )
	{
		selectedUnitsSet = newSelectedUnitsSet;
		if ( selectedUnitsSet.size() != 1 )
		{
			NMainLoop::Command( new NMainLoop::CICExitModal() );
			return;
		}

		if ( IsValid( pGroundCtrl ) )
			pGame->PopDesktop();

		pGround = NUI::IWindow::Create( pGame->GetInterface(), NUI::SRect( 0, 0, 1024, 768 ), "groundUI" );
		pGroundCtrl = new NUI::CGroundUI( pGround, pGame, (*selectedUnitsSet.begin())->GetUnit() );
		NUI::LoadTemplate( pGroundCtrl, NDb::GetUIContainer( 82 ) );
		pGroundCtrl->ShowWindow( NUI::SWTYPE_SHOW );

		pGame->PushDesktop( pGroundCtrl );
	}
	
	if ( CanRender() )
		pGame->RenderFrame( N_RENDERMODE_2D, GetTime() );
	else
		pGame->GetRenderGame()->ResetTiming();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGroundInterface::ProcessEvent( const NInput::SEvent &sEvent )
{
	if ( pGame->ProcessEvent( sEvent ) )
		return true;

	if ( bindExit.ProcessEvent( sEvent ) || bindGround.ProcessEvent( sEvent ) )
	{
		pGame->SetDefaultInterface();
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CICMission
////////////////////////////////////////////////////////////////////////////////////////////////////
CICGroundArea::CICGroundArea( IMissionGame *_pGame ):
	pGame( _pGame )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CICGroundArea::Exec()
{
	ResetStack();
	CGroundInterface *pRes = new CGroundInterface();
	pRes->Initialize( pGame );
	SetInterface( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB2040500, CGroundSlot );
REGISTER_SAVELOAD_CLASS( 0xB2040501, CGroundUI );
using namespace NGame;
REGISTER_SAVELOAD_CLASS( 0xB2040502, CGroundInterface );
