#include "StdAfx.h"
#include "RPGItemSet.h"
#include "RPGUnit.h"
#include "..\Misc\2DArray.h"
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_SIZE_X = 7;
const int N_SIZE_Y = 12;
const int N_SPACE = N_SIZE_X * N_SIZE_Y;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Для хранения предметов
class CInventory: public IInventory
{
	OBJECT_BASIC_METHODS(CInventory);
	ZDATA
	int nSpace;
	int nWeight;
	int nActiveSlot;
	CArray2D<bool> backpackMap;
	vector<SBackPackItem> items;
	vector< CObj<IInventoryItem> > slots;
	CPtr<IInventoryItem> uniform;
	CPtr<CUnit> pOwner;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nSpace); f.Add(3,&nWeight); f.Add(4,&nActiveSlot); f.Add(5,&backpackMap); f.Add(6,&items); f.Add(7,&slots); f.Add(8,&uniform); f.Add(9,&pOwner); return 0; }

public:
	CInventory( CUnit *_pOwner = 0 );
	
	virtual int GetWeight() const { return nWeight; }
	virtual int GetEmptySpace() const { return N_SPACE - nSpace; }
	virtual void Take( IInventoryItem *pItem );
	virtual bool CanPlace( const CTPoint<int> &sPos, const IInventoryItem *pItem, bool bCheckSpace = true ) const;
	virtual bool FindPlace( const IInventoryItem *pItem, CTPoint<int> *pPos ) const;
	virtual bool Place( const CTPoint<int> &sPos, IInventoryItem *pItem );
	virtual const vector<SBackPackItem>& GetItems() const { return items; };

	virtual bool CanEquip( NDb::ESlot where, const IInventoryItem *pWhat ) const;
	virtual bool Equip( NDb::ESlot where, IInventoryItem *pWhat );
	virtual bool CanWear( const IInventoryItem *pWhat ) const;
	virtual bool Wear( IInventoryItem *pWhat );
	virtual IInventoryItem *UnWear();
	virtual IInventoryItem *TakeOff( NDb::ESlot where );
	virtual IInventoryItem *Get( NDb::ESlot where ) const;
	virtual IInventoryItem *GetUniform() const { return uniform; }

	virtual bool Activate( NDb::ESlot where );
	virtual IInventoryItem *GetActive() const;
	virtual int GetActiveSlot() const;
	virtual int GetPlaceBySubType( NDb::EItemSubType subType ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CInventory::CInventory( CUnit *_pOwner ): pOwner(_pOwner), backpackMap( N_SIZE_X, N_SIZE_Y ), nSpace( 0 ), nWeight( 0 )
{
	slots.resize(NDb::N_SLOTS);
	backpackMap.FillZero();
	nActiveSlot = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CInventory::Take( IInventoryItem *pItem )
{
	for ( vector<SBackPackItem>::iterator iTemp = items.begin(); iTemp != items.end(); iTemp++ )
	{
		if ( iTemp->pItem == pItem )
		{
			nSpace -= pItem->GetSize().x * pItem->GetSize().y;
			nWeight -= pItem->GetWeight();
			for( int nTempY = 0; nTempY < pItem->GetSize().y; nTempY++ )
			{
				for( int nTempX = 0; nTempX < pItem->GetSize().x; nTempX++ )
				{
					backpackMap[iTemp->sPos.y + nTempY][iTemp->sPos.x + nTempX] = false;
				}
			}

			items.erase( iTemp );

			return;
		}
	}

	ASSERT( 0 && "Item doesn't exist " );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventory::CanPlace( const CTPoint<int> &sPos, const IInventoryItem *pItem, bool bCheckSpace ) const
{
	if ( ( sPos.x < 0 ) || ( sPos.y < 0 ) || ( sPos.x + pItem->GetSize().x > backpackMap.GetXSize() ) || ( sPos.y + pItem->GetSize().y > backpackMap.GetYSize() ) )
		return false;

	if ( !bCheckSpace )
		return true;

	for( int nTempY = 0; nTempY < pItem->GetSize().y; nTempY++ )
	{
		for( int nTempX = 0; nTempX < pItem->GetSize().x; nTempX++ )
		{
			if ( backpackMap[sPos.y + nTempY][sPos.x + nTempX] )
				return false;
		}
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventory::FindPlace( const IInventoryItem *pItem, CTPoint<int> *pPos ) const
{
	for( int nTempY = 0; nTempY < backpackMap.GetYSize(); nTempY++ )
	{
		for( int nTempX = 0; nTempX < backpackMap.GetXSize(); nTempX++ )
		{
			if ( CanPlace( CTPoint<int>( nTempX, nTempY ), pItem ) )
			{
				*pPos = CTPoint<int>( nTempX, nTempY );
				return true;
			}
		}
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventory::Place( const CTPoint<int> &_sPos, IInventoryItem *pItem )
{
	CTPoint<int> sPos( _sPos );

	if ( ( sPos.x == -1 ) && ( sPos.y == -1 ) && !FindPlace( pItem, &sPos ) )
		return false;

	for( int nTempY = 0; nTempY < pItem->GetSize().y; nTempY++ )
	{
		for( int nTempX = 0; nTempX < pItem->GetSize().x; nTempX++ )
		{
			backpackMap[sPos.y + nTempY][sPos.x + nTempX] = true;
		}
	}

	nSpace += pItem->GetSize().x * pItem->GetSize().y;
	nWeight += pItem->GetWeight();
	items.push_back( SBackPackItem( sPos, pItem ) );
	
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventory::CanEquip( NDb::ESlot where, const IInventoryItem *pWhat ) const
{
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventory::Equip( NDb::ESlot where, IInventoryItem *pWhat )
{
	if ( !CanEquip( where, pWhat ) )
		return false;
	slots[where] = pWhat;
	dynamic_cast<CInventoryItem*>(pWhat)->OnEquip( pOwner );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventory::CanWear( const IInventoryItem *pWhat ) const
{
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventory::Wear( IInventoryItem *pWhat )
{
	if ( !CanWear( pWhat ) )
		return false;
	uniform = pWhat;
	dynamic_cast<CInventoryItem*>(pWhat)->OnEquip( pOwner );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IInventoryItem *CInventory::UnWear()
{
	CPtr<IInventoryItem> pWhat = uniform;
	dynamic_cast<CInventoryItem*>(pWhat.GetPtr())->OnUnEquip( pOwner );
	uniform = 0;
	return pWhat.Extract();	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IInventoryItem *CInventory::TakeOff( NDb::ESlot where )
{
	CObj<IInventoryItem> pWhat = slots[where];
	dynamic_cast<CInventoryItem*>(pWhat.GetPtr())->OnUnEquip( pOwner );
	slots[where] = 0;
	return pWhat.Extract();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CInventory::Activate( NDb::ESlot where )
{
	nActiveSlot = where;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IInventoryItem *CInventory::GetActive() const
{
	ASSERT( nActiveSlot >= 0 && nActiveSlot < NDb::N_SLOTS );
	if ( nActiveSlot < 0 || nActiveSlot >= NDb::N_SLOTS ) 
		return 0; 
	return slots[nActiveSlot];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IInventoryItem* CInventory::Get( NDb::ESlot where ) const 
{ 
	ASSERT( where >= 0 && where < NDb::N_SLOTS );
	if ( where < 0 || where >= NDb::N_SLOTS ) 
		return 0; 
	return slots[where]; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CInventory::GetActiveSlot() const
{
	return nActiveSlot;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CInventory::GetPlaceBySubType( NDb::EItemSubType subType ) const
{
	if ( !uniform )
		return -1;
	CDynamicCast<NRPG::IUniformItem> pUniform( uniform );
	NDb::CRPGUniform *pDBUniform = 0;
	if ( pUniform )
		pDBUniform = pUniform->GetDBUniform();
	for ( int i = 0; i < NDb::N_ITEM_PLACES; ++i )
	{
		if ( pDBUniform->subTypes[i] == subType )
			return i;
	}
	return -1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
IInventory *CreateInventory( CUnit *pOwner )
{
	return new CInventory(pOwner);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
REGISTER_SAVELOAD_CLASS_NM( 0xE0591410, CInventory, NRPG );
using namespace NRPG;
BASIC_REGISTER_CLASS(IInventoryItem)
