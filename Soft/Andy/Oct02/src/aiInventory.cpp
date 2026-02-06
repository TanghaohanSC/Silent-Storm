#include "stdafx.h"

#include "RPGUnitMission.h"
#include "RPGItemSet.h"
#include "RPGItem.h"
#include "RPGAttackMech.h"

#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataConst.h"

#include "aiUnit.h"
#include "aiWeapon.h"
#include "aiPosition.h"
#include "aiInventory.h"

#include "wUnitServer.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIInventory
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIInventory: public IAIInventory
{
	OBJECT_BASIC_METHODS(CAIInventory);
	ZDATA
	CPtr<IAIWeapon> pCurrentWeapon;
	vector< CObj<IAIWeapon> > Weapon; // содержит pCurrentWeapon
	CPtr<IAIUnit> pOwner;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCurrentWeapon); f.Add(3,&Weapon); f.Add(4,&pOwner); return 0; }
	//
	void GetInventoryItems( list< CPtr<NRPG::IInventoryItem> > *lItems );
	void FetchFireArmsItems();
	void FetchClipItems();
	void FetchGrenadeItems();
	void OutputDebugInfo();
	bool HasThisWeapon( IAIWeapon *pAIWeapon );
	IAIWeapon *GetAIWeapon( NRPG::IInventoryItem *pWeaponItem );
public:
	//
	CAIInventory() {}
	CAIInventory( IAIUnit *_pOwner );
	// IAIInventory
	virtual void AddWeapon( IAIWeapon *pWeapon );
	virtual void RemoveWeapon( IAIWeapon *pWeapon, bool bRemoveClips );
	virtual void AddClip( IAIWeaponClip *pClip );
	virtual IAIWeapon *GetBestWeapon( IAIUnit *pTarget, int nAP, int *nBestWeaponHitCover,
		int *nQuality, NDb::EShootMode *eShootMode, int *nMaxToHit, bool bLog = false );
	virtual void SetCurrentWeapon( IAIWeapon *pWeapon );
	virtual IAIWeapon *GetCurrentWeapon() { return pCurrentWeapon; }
	virtual int GetWeaponCount() { return Weapon.size(); }
	virtual IAIWeapon* GetSuitableWeapon( IAIWeaponClip *pClip );
	virtual NRPG::CAttackPortion GetAttackPortion();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIInventory::CAIInventory( IAIUnit *_pOwner ):
	pCurrentWeapon(0), pOwner(_pOwner)
{
	FetchFireArmsItems();
	FetchClipItems();
	SetCurrentWeapon( GetAIWeapon( pOwner->GetUnitServer()->GetUnitRPG()->GetInventory()->GetActive() ) );
	// FetchGrenadeItems();
	// OutputDebugInfo();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeapon *CAIInventory::GetAIWeapon( NRPG::IInventoryItem *pWeaponItem )
{
	for ( vector< CObj<IAIWeapon> >::iterator i = Weapon.begin(); i != Weapon.end(); ++i )
	{
		CDynamicCast<NRPG::IInventoryItem> pTmpWeaponItem( (*i)->GetInventoryItem() );
		if ( pTmpWeaponItem == pWeaponItem )
			return *i;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::SetCurrentWeapon( IAIWeapon *pWeapon )
{
	pCurrentWeapon = pWeapon;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::OutputDebugInfo()
{
	int n = 1;
	char szBuf[128];
	OutputDebugString("[AI INVENTORY] Person weapon :\n");
	NRPG::SUnitInfo sInfo;
	pOwner->GetUnitServer()->GetUnitRPG()->GetInfo( NAI::WALK, &sInfo );

	for ( vector< CObj<IAIWeapon> >::iterator i = Weapon.begin(); i != Weapon.end(); ++i  )
	{
		sprintf( szBuf, "[AI INVENTORY]           Weapon %d : Ammo in weapon %d, clips %d, Ammo per Shot %d, Ammo per AP %d \n", 
			n, (*i)->GetCurrentClip()->GetAmmoCount(), 
			(*i)->GetClipCount(), (*i)->GetAmmoCountPerShot( sInfo.nAP, sInfo.nMaxAP ), 
			(*i)->GetAmmoCountPerAP( sInfo.nMaxAP ) );
		OutputDebugString( szBuf );
		++n;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::GetInventoryItems( list< CPtr<NRPG::IInventoryItem> > *lItems )
{
	lItems->clear();
	// вещи из рюкзачка
	NRPG::IUnitMission *pRPG = pOwner->GetUnitServer()->GetUnitRPG();
	const vector<NRPG::SBackPackItem> &vBackPackItems = pRPG->GetInventory()->GetItems();
	for ( vector<NRPG::SBackPackItem>::const_iterator i = vBackPackItems.begin(); 
		i != vBackPackItems.end(); ++i )
	{
		lItems->push_back( (*i).pItem.GetPtr() );
	}
	// вещи из слотов
	for ( int nSlot = 0; nSlot < NDb::N_SLOTS; ++nSlot )
		lItems->push_back( pRPG->GetInventory()->Get( (NDb::ESlot)nSlot ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::FetchFireArmsItems()
{
	list< CPtr<NRPG::IInventoryItem> > lItems;
	GetInventoryItems( &lItems );

	for ( list< CPtr<NRPG::IInventoryItem> >::iterator i = lItems.begin(); i != lItems.end(); ++i  )
	{
		if ( CDynamicCast<NRPG::CWeaponItem> pWeaponItem(*i) )
			AddWeapon( CreateFireArmsAIWeapon( pOwner, pWeaponItem ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::FetchClipItems()
{
	list< CPtr<NRPG::IInventoryItem> > lItems;
	GetInventoryItems( &lItems );

	for ( list< CPtr<NRPG::IInventoryItem> >::iterator i = lItems.begin(); i != lItems.end(); ++i  )
	{
		if ( CDynamicCast<NRPG::CClipItem> pClipItem(*i) )
			AddClip( CreateAIFireArmsWeaponClip( pClipItem ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::FetchGrenadeItems()
{
	list< CPtr<NRPG::IInventoryItem> > lItems;
	GetInventoryItems( &lItems );
	// добавляем оружие
	for ( list< CPtr<NRPG::IInventoryItem> >::iterator i = lItems.begin(); i != lItems.end(); ++i  )
		if ( CDynamicCast<NRPG::CGrenadeItem> pGrenadeItem(*i) )
		{
			CPtr<IAIWeapon> pGrenadeWeapon = CreateGrenadeAIWeapon( pOwner, pGrenadeItem );
			if ( !HasThisWeapon( pGrenadeWeapon ) )
				AddWeapon( pGrenadeWeapon );
		}
	// добавляем патроны
	for ( list< CPtr<NRPG::IInventoryItem> >::iterator i = lItems.begin(); i != lItems.end(); ++i  )
		if ( CDynamicCast<NRPG::CGrenadeItem> pGrenadeItem(*i) )
		{
			CPtr<IAIWeaponClip> pGrenadeClip = CreateAIGrenadeWeaponClip( pGrenadeItem );
			CPtr<IAIWeapon> pGrenadeWeapon = GetSuitableWeapon( pGrenadeClip );
			if ( IsValid( pGrenadeItem ) )
			{
				if ( !IsValid( pGrenadeWeapon->GetCurrentClip() ) )
					pGrenadeWeapon->SetCurrentClip( pGrenadeClip );
				else
					pGrenadeWeapon->AddClip( pGrenadeClip );
			}
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIInventory::HasThisWeapon( IAIWeapon *pAIWeapon )
{
	for ( vector< CObj<IAIWeapon> >::iterator i = Weapon.begin(); i != Weapon.end(); ++i  )
		if ( (*i)->IsSameWeapon( pAIWeapon ) )
			return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::AddWeapon( IAIWeapon *pWeapon )
{
	Weapon.push_back( pWeapon );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::RemoveWeapon( IAIWeapon *pWeapon, bool bRemoveClips )
{
	vector< CObj<IAIWeapon> >::iterator i = find( Weapon.begin(), Weapon.end(), pWeapon );
	if ( i != Weapon.end() )
		Weapon.erase( i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeapon* CAIInventory::GetSuitableWeapon( IAIWeaponClip *pClip )
{
	for ( vector< CObj<IAIWeapon> >::iterator i = Weapon.begin(); i != Weapon.end(); ++i  )
		if ( (*i)->IsSuitableClip( pClip ) )
			return *i;

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIInventory::AddClip( IAIWeaponClip *pClip )
{
	CPtr<IAIWeapon> pWeapon = 0;
	pWeapon = GetSuitableWeapon( pClip );
	if ( IsValid(pWeapon) )
	{
		pWeapon->AddClip( pClip );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CAttackPortion CAIInventory::GetAttackPortion()
{
	NRPG::CAttackPortion AttackPortion;
	AttackPortion.nK = 0;
	for ( vector< CObj<IAIWeapon> >::iterator i = Weapon.begin(); i != Weapon.end(); ++i )
	{
		CDynamicCast<NRPG::CWeaponItem> pWeaponItem( (*i)->GetInventoryItem() );
		if ( IsValid( pWeaponItem ) )
		{
			vector<NRPG::CAttackPortion> TmpAttackPortions;
			pWeaponItem->CreateNewAttackPortion( &TmpAttackPortions, false );
			if ( TmpAttackPortions.front().nK > AttackPortion.nK )
				AttackPortion = TmpAttackPortions.front();
		}
	}
	return AttackPortion;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeapon *CAIInventory::GetBestWeapon( IAIUnit *pTarget, int nAP, int *nBestWeaponHitCover, int *nQuality,
	NDb::EShootMode *eShootMode, int *nMaxToHit, bool bLog )
{
	int nShootMode = 0;
	IAIWeapon *pBestWeapon = 0;
	*nMaxToHit = 0;
	*nBestWeaponHitCover = 0;
	*nQuality = -0xFFFF;
	for ( vector< CObj<IAIWeapon> >::iterator i = Weapon.begin(); i != Weapon.end(); ++i  )
	{
		CDynamicCast<NRPG::CWeaponItem> pWeaponItem( (*i)->GetInventoryItem() );
		int nHitCover = pOwner->GetCoverForFixedUnit( pTarget->GetUnitServer(), pWeaponItem, NAI::HL_ANY );
		//
		for ( int nShootMode = (int)NDb::SM_Snap; nShootMode != (int)NDb::SM_Snipe; ++nShootMode )
		{
			if ( !pWeaponItem->IsShootModeSupported( (NDb::EShootMode)nShootMode ) )
				continue;
			//
			int nTmpMaxToHit;
			int nTmpQuality = (*i)->GetDamage( pTarget, nHitCover, NAI::WALK, nAP, (NDb::EShootMode)nShootMode, &nTmpMaxToHit );
			if ( bLog )
			{
				char szStr[128];
				sprintf( szStr, "      [AI WEAPON] HitCover = %d, ShootMode = %d, Damage = %d\n", nHitCover, nShootMode, nTmpQuality );
				OutputDebugString( szStr );
			}
			*nMaxToHit = max( *nMaxToHit, nTmpMaxToHit );
			if ( nTmpQuality > *nQuality )
			{
				pBestWeapon = *i;
				*nQuality = nTmpQuality;
				*eShootMode = (NDb::EShootMode)nShootMode;
				*nBestWeaponHitCover = nHitCover;
			}
		}
	}
	return pBestWeapon;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIInventory *CreateAIInventory( IAIUnit *_pOwner )
{
	ASSERT( !_pOwner->GetUnitServer()->IsDead() );
	//
	return new CAIInventory( _pOwner );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}

using namespace NAI;

REGISTER_SAVELOAD_CLASS( 0x52642102, CAIInventory );