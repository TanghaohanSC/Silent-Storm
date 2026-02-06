#include "StdAfx.h"
//#include "RPGGame.h"
#include "RPGItemInfo.h"
#include "RPGUnitInfo.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
//#include "GAnimation.h"
#include "InventoryUnit.h"
#include "..\Misc\RandomGen.h"
//#include "wInterface.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int nItemPlaces2UnitItemTypes[ NDb::N_ITEM_PLACES ] = 
{
	UIT_BELT_L1,
	UIT_BELT_R1,
	UIT_BELT_M1,
	UIT_BELT_MEDIUM_L1,
	UIT_BELT_MEDIUM_R1,
	UIT_BELT_MEDIUM_L2,
	UIT_BELT_MEDIUM_R2,
	UIT_WAIST_BELT_L1,
	UIT_WAIST_BELT_R1,
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const char *pszItemEffectors[N_UNIT_ITEM_TYPES] = 
{
	0, 0, 
	"Slot5",
	"Slot3",
	"Slot4",
	"Slot9",
	"Slot6",
	"Slot8",
	"Slot7",
	"Slot2",
	"Slot1",
	"Cap",
	"BackPack",
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* GetBoneName( EUnitItemType type, NRPG::IInventoryItem *pItem )
{
	const char *pszBoneName = 0;
	switch ( type )
	{
		case UIT_WEAPON_HEAVY:
			if ( CDynamicCast<NRPG::IWeaponItemInfo> pWeapon(pItem) )
			{
				NDb::CAnimWeaponType *pWType = pWeapon->GetDBWeapon()->pAnimWeaponType;
				ASSERT( pWType );
				if ( !pWType )
					return "";
				switch ( pWType->type )
				{
					case NDb::WT_RIFLE:
						pszBoneName = "Rifle";
						break;
					case NDb::WT_SUB_MACHINE_GUN:
						pszBoneName = "SubMachineGun";
						break;
					case NDb::WT_MACHINE_GUN:
						pszBoneName = "MachineGun";
						break;
					case NDb::WT_RLAUNCHER:
						pszBoneName = "RocketLauncher";
						break;
					default:
						ASSERT( 0 );
						return "";
				}
			}
			break;
		case UIT_HAND:
			pszBoneName = "Item";
			break;
		case UIT_BELT_L1:
		case UIT_BELT_R1:
		case UIT_BELT_M1:
		case UIT_BELT_MEDIUM_L1:
		case UIT_BELT_MEDIUM_R1:
		case UIT_BELT_MEDIUM_L2:
		case UIT_BELT_MEDIUM_R2:
		case UIT_WAIST_BELT_L1:
		case UIT_WAIST_BELT_R1:
		case UIT_CAP:
		case UIT_BACKPACK:
			pszBoneName = pszItemEffectors[type];
			break;
	}
	return pszBoneName;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AttachItem( vector<IRenderVisitor::SBoundMesh> *pRes, SRand *pRnd, 
	EUnitItemType type, NDb::CTRndModel *pModel, NRPG::IInventoryItem *pItem = 0 )
{
	if ( !pModel )
		return;
	const char *pszBoneName = GetBoneName( type, pItem );
	pRes->push_back( IRenderVisitor::SBoundMesh( pModel->CreateModel( pRnd ), pszBoneName ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* GetBoneName( NDb::ESlot slot, NRPG::IUnitMissionInfo *pRPG, bool bUndrawWeapon )
{
	CPtr<NRPG::IInventoryInfo> pInventory = pRPG->GetInventoryInfo();
	CDynamicCast<NRPG::IUniformItem> pUniform( pInventory->GetUniform() );
	NDb::CRPGUniform *pDBUniform = 0;
	if ( pUniform )
		pDBUniform = pUniform->GetDBUniform();
	NRPG::IInventoryItem *pItem = pInventory->Get( slot );
	NRPG::IInventoryItem *pActiveItem = pInventory->GetActive();
	if ( !pItem || !pDBUniform )
		return "";
	if ( pItem->GetDBItem()->subType == NDb::SUBTYPE_HEAVY )
		return GetBoneName( UIT_WEAPON_HEAVY, pItem );
	if ( pItem == pActiveItem && !bUndrawWeapon )
		return GetBoneName( UIT_HAND, pItem );
	for ( int i = 0; i < NDb::N_ITEM_PLACES; ++i )
	{
		if ( pDBUniform->subTypes[i] == pItem->GetDBItem()->subType )
		{
			EUnitItemType uit = (EUnitItemType)nItemPlaces2UnitItemTypes[i];
			return GetBoneName( uit, pItem );
		}
	}
	return "";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetItemsBindPlaces( vector<IRenderVisitor::SBoundMesh> *pRes, NRPG::IUnitMissionInfo *pRPG, 
	bool bUndrawWeapon, bool bNoHeavyWeapon )
{
	pRes->clear();
	SRand rnd;
	CPtr<NRPG::IInventoryInfo> pInventory = pRPG->GetInventoryInfo();
	CDynamicCast<NRPG::IUniformItem> pUniform( pInventory->GetUniform() );
	NDb::CRPGUniform *pDBUniform = 0;
	if ( pUniform )
		pDBUniform = pUniform->GetDBUniform();
	// cap & backpack
	if ( pDBUniform )
	{
		if ( pDBUniform->pCapModel )
			AttachItem( pRes, &rnd, UIT_CAP, pDBUniform->pCapModel );
		if ( pDBUniform->pBackpackModel )
			AttachItem( pRes, &rnd, UIT_BACKPACK, pDBUniform->pBackpackModel );
	}
	bool bHeavy = false;
	hash_map<int,NDb::CRPGItem*> slotItems;
	NRPG::IInventoryItem *pActiveIItem = pInventory->GetActive();
	NDb::CRPGItem *pActiveItem = 0;
	if ( pActiveIItem )
		pActiveItem = pActiveIItem->GetDBItem();
	if ( pActiveItem )
	{
		if ( pActiveItem->subType == NDb::SUBTYPE_HEAVY )
		{
			if ( !bNoHeavyWeapon )
				AttachItem( pRes, &rnd, UIT_WEAPON_HEAVY, pActiveItem->pModel, pActiveIItem );
			bHeavy = true;
		}
		else if ( !bUndrawWeapon )
			AttachItem( pRes, &rnd, UIT_HAND, pActiveItem->pModel, pActiveIItem );
	}
	for ( int i = 0; i < NDb::N_SLOTS; ++i )
	{
		NRPG::IInventoryItem *pIItem = pInventory->Get( (NDb::ESlot)i );
		if ( !pIItem || pInventory->GetActiveSlot() == i )
			continue;
		NDb::CRPGItem *pItem = pIItem->GetDBItem();
		if ( pItem->subType == NDb::SUBTYPE_HEAVY && !bHeavy )
		{
			if ( !bNoHeavyWeapon )
				AttachItem( pRes, &rnd, UIT_WEAPON_HEAVY, pItem->pModel, pIItem );
			bHeavy = true;
		}
		else
			slotItems[ pItem->subType ] = pItem;
	}
	if ( pActiveItem && pActiveItem->subType != NDb::SUBTYPE_HEAVY )
		slotItems[ pActiveItem->subType ] = pActiveItem;
	for ( int i = 0; i < NDb::N_ITEM_PLACES; ++i )
	{
		EUnitItemType uit = (EUnitItemType)nItemPlaces2UnitItemTypes[i];
		NDb::EItemSubType subType = pDBUniform->subTypes[i];
		ASSERT( subType != NDb::SUBTYPE_HEAVY );
		if ( subType == NDb::SUBTYPE_NONE && pDBUniform->fixedModels[i] )
		{
			AttachItem( pRes, &rnd, uit, pDBUniform->fixedModels[i] );
			continue;
		}
		if ( pActiveItem && pActiveItem->subType == subType )
		{
			if ( bUndrawWeapon )
				AttachItem( pRes, &rnd, uit, pActiveItem->GetItemModel( false, pDBUniform ) );
			else if ( subType == NDb::SUBTYPE_PISTOL || subType == NDb::SUBTYPE_KNIFE )
				AttachItem( pRes, &rnd, uit, pActiveItem->GetItemModel( true, pDBUniform ) );
			else
			{
				NDb::CRPGItem *pResultItem = 0;
				int nMaxPriority = -1;
				const vector<NRPG::SBackPackItem> &bpItems = pInventory->GetItems();
				for ( vector<NRPG::SBackPackItem>::const_iterator it = bpItems.begin(); it != bpItems.end(); ++it )
				{
					NDb::CRPGItem *pI = it->pItem->GetDBItem();
					if ( pI == pActiveItem )
					{
						pResultItem = pActiveItem;
						break;
					}
					else if ( pI->subType == subType && pI->nSubTypePriority > nMaxPriority )
					{
						pResultItem = pI;
						nMaxPriority = pI->nSubTypePriority;
					}
				}
				if ( pResultItem )
					AttachItem( pRes, &rnd, uit, pResultItem->GetItemModel( false, pDBUniform ) );
			}
		}
		else
		{
			if ( slotItems.find(subType) != slotItems.end() )
				AttachItem( pRes, &rnd, uit, slotItems[subType]->GetItemModel( false, pDBUniform ) );
			else
			{
				NDb::CRPGItem *pResultItem = 0;
				int nMaxPriority = -1;
				const vector<NRPG::SBackPackItem> &bpItems = pInventory->GetItems();
				for ( vector<NRPG::SBackPackItem>::const_iterator it = bpItems.begin(); it != bpItems.end(); ++it )
				{
					NDb::CRPGItem *pI = it->pItem->GetDBItem();
					if ( pI->subType == subType && pI->nSubTypePriority > nMaxPriority )
					{
						pResultItem = pI;
						nMaxPriority = pI->nSubTypePriority;
					}
				}
				if ( pResultItem )
					AttachItem( pRes, &rnd, uit, pResultItem->GetItemModel( false, pDBUniform ) );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
