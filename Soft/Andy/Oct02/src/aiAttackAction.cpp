#include "StdAfx.h"

#include "RPGItem.h"
#include "RPGItemSet.h"
#include "RPGItemInfo.h"

#include "aiLog.h"
#include "aiJob.h"
#include "aiUnit.h"
#include "aiState.h"
#include "aiWeapon.h"
#include "aiAction.h"
#include "aiIterator.h"
#include "aiInventory.h"
#include "aiCriterion.h"

#include "wMain.h"
#include "wUnitServer.h"
#include "wUnitCommands.h"

#include "..\DBFormat\DataRPG.h"

#include "aiAttackAction.h"

namespace NAI
{
const int N_MIN_DAMAGE_TO_SHOOT = 2;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIShootAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIShootAction: public CAIAction
{
	OBJECT_BASIC_METHODS(CAIShootAction)
	ZDATA
	ZPARENT( CAIAction );
	int nSpentAP;
	bool bNeedReload;
	int nShotHP, nShotAP, nAmmo;
	bool bBestWeaponChose;
	int nHitCover;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIAction *)this); f.Add(3,&nSpentAP); f.Add(4,&bNeedReload); f.Add(5,&nShotHP); f.Add(6,&nShotAP); f.Add(7,&nAmmo); f.Add(8,&bBestWeaponChose); f.Add(9,&nHitCover); return 0; }
	//
	void ChooseBestWeapon();
	void MakeShot();
public:
	//
	CAIShootAction() {}
	CAIShootAction( IAIState *_pAIState, IAIJob *_pParentJob );
	// IAIAction
	virtual int GetMinAP();
	virtual void Think();
	virtual bool IsNeedRemainAP() { return true; }
	virtual bool IsUsable() { return IsValid( pAIState->GetCurrentAIUnit() ) && 
		!pAIState->GetCurrentAIUnit()->HasInactivePose(); } 
	// IAIJob
	virtual void DoJob();
	virtual bool IsJobFinished();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIShootAction::CAIShootAction( IAIState *_pAIState, IAIJob *_pParentJob ):
	CAIAction( _pAIState, _pParentJob )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIShootAction::GetMinAP()
{
	ASSERT( IsValid(pAIState->GetCurrentAIUnit()) );
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIShootAction::ChooseBestWeapon()
{
	int nDamage = 0;
	CPtr<IAIUnit> pAIUnit = pAIState->GetCurrentAIUnit();
	CPtr<IAIUnit> pAIEnemy = pAIState->GetCurrentAIEnemy();
	if ( !IsValid( pAIEnemy ) || !IsValid( pAIUnit ) )
		return;
	int nMaxToHit;
	NDb::EShootMode eShootMode = NDb::SM_Snap;
	CPtr<IAIWeapon> pWeapon = pAIUnit->GetAIInventory()->GetBestWeapon( pAIEnemy, 
		GetMaxAP(), &nHitCover, &nDamage, &eShootMode, &nMaxToHit );
	//
	GetAILog()->Add( new CAILogChangeMaxToHit( pAIUnit, nMaxToHit ), true );
	// DEBUG{
	char szStr[128];
	sprintf( szStr, "[SHOOT ACTION] Shoot MaxToHit = %d\n", pAIUnit->GetMaxToHit() );
	OutputDebugString( szStr );
	// DEBUG}
	if ( nDamage < 1 )
	{
		pWeapon = pAIUnit->GetAIInventory()->GetCurrentWeapon();
		if ( IsValid( pWeapon ) )
		{
			if ( pWeapon->GetCurrentClip()->GetAmmoCount() <= 0 && IsValid( pWeapon->GetNextClip() ) )
				GetAILog()->Add( new CAILogReloadWeapon( pAIUnit ), true );
		}
		nSpentAP = 0xFFFF;
	}
	else
	{
		if ( IsValid( pWeapon ) )
		{
			if ( pWeapon != pAIUnit->GetAIInventory()->GetCurrentWeapon() )
				GetAILog()->Add( new CAILogChangeWeapon( pAIUnit, pWeapon ), true );
			//
			GetAILog()->Add( new CAILogChangeShootMode( pAIUnit, eShootMode ) );
		}
		// если был нулевой Damage, а стал не нулевой, то это хорошо
		GetAILog()->Add( new CAILogExpediency( pAIUnit, 100 ), true );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIShootAction::Think()
{
	OutputDebugString( "[SHOOT ACTION] Think\n" );
	IAIJobManager *pAIJobManager = pAIState->GetWorld()->GetAIJobManager();
	ASSERT( IsValid( pAIJobManager ) );
	//
	nSpentAP = 0;
	pAIState->GetCurrentAIUnit()->SetMaxToHit( 0 );
	pAIJobManager->Remove( this );
	pAILog->Clear();
	bBestWeaponChose = false;
	pAIJobManager->Add( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIShootAction::DoJob()
{
	CPtr<IAIUnit> pAIUnit = pAIState->GetCurrentAIUnit();
	CPtr<IAIUnit> pAIEnemy = pAIState->GetCurrentAIEnemy();
	CPtr<IAIWeapon> pWeapon = pAIUnit->GetAIInventory()->GetCurrentWeapon();
	//
	if ( !bBestWeaponChose )
	{
		ChooseBestWeapon();
		bBestWeaponChose = true;
	}
	else if ( !IsJobFinished() )
	{
		pWeapon->GetShotParameters( nMaxAP, nHitCover, &nShotAP, &nAmmo, pAIEnemy, &nShotHP, &bNeedReload );
		nSpentAP += nShotAP;
		if ( !IsJobFinished() )
		{
			if ( bNeedReload )
				GetAILog()->Add( new CAILogReloadWeapon( pAIUnit ), true );
			//
			GetAILog()->Add( pWeapon->GetAttackCommand( pAIUnit, pAIEnemy ) );
			GetAILog()->Add( new CAILogSpendAP( pAIUnit, nShotAP ), true ); // тратим AP
			GetAILog()->Add( new CAILogSpendAmmo( pWeapon->GetCurrentClip(), nAmmo ), true ); // тратим патроны
			if ( pAIEnemy->GetHP() > 0 )
			{
				GetAILog()->Add( new CAILogHurt( pAIUnit, nShotHP ), true ); // сохраняем сколько нанесли повреждений
				GetAILog()->Add( new CAILogSpendHP( pAIEnemy, nShotHP ), true ); // наносим повреждения
			}	
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIShootAction::IsJobFinished()
{
	if ( !bBestWeaponChose )
		return false;
	//
	if ( nSpentAP > nMaxAP )
		return true;
	//
	CPtr<IAIUnit> pAIUnit = pAIState->GetCurrentAIUnit();
	CPtr<IAIUnit> pAIEnemy = pAIState->GetCurrentAIEnemy();
	if ( !IsValid( pAIEnemy ) || !IsValid( pAIUnit ) )
		return true;
	//
	CPtr<IAIWeapon> pWeapon = pAIState->GetCurrentAIUnit()->GetAIInventory()->GetCurrentWeapon();
	if ( !IsValid( pWeapon ) || pWeapon->GetCurrentClip()->GetAmmoCount() <= 0 && !IsValid( pWeapon->GetNextClip() ) )
		return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAILootAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAILootAction: public CAIAction
{
	OBJECT_BASIC_METHODS(CAILootAction)
	ZDATA
	ZPARENT( CAIAction );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIAction *)this); return 0; }
public:
	//
	CAILootAction() {}
	CAILootAction( IAIState *_pAIState, IAIJob *_pParentJob );
	// IAIAction
	virtual void Think();	
	// IAIJob
	virtual void DoJob() {}
	virtual bool IsJobFinished() { return true; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAILootAction::CAILootAction( IAIState *_pAIState, IAIJob *_pParentJob ):
	CAIAction( _pAIState, _pParentJob )
{
	nMinAP = 1; // CRAP // на Loot нужно 1 AP
	nAPStep = 0xFFFF;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAILootAction::Think()
{
	CPtr<IAIUnit> pAIUnit = pAIState->GetCurrentAIUnit();
	if ( !IsValid( pAIUnit ) )
		return;
	//
	vector<NWorld::SItem> Items;
	CPtr<NWorld::CWorld> pWorld = pAIUnit->GetUnitServer()->GetWorld();
	pWorld->FindCloseGroundItems( pAIUnit->GetUnitServer(), &Items );
	// добавляем оружие
	if ( pAIUnit->GetAIInventory()->GetWeaponCount() < 3 ) // CRAP
	{
		for ( vector<NWorld::SItem>::iterator i = Items.begin(); i != Items.end(); ++i )
			if ( CDynamicCast<NRPG::CWeaponItem> pWeaponItem( (*i).pItem ) )
			{
				CDynamicCast<NRPG::IInventoryItem> pInventoryItem( pWeaponItem.GetPtr() );
				GetAILog()->Add( new CAILogPickUpItem( pAIUnit, pInventoryItem ), true );
				CPtr<IAIWeapon> pAIWeapon = CreateFireArmsAIWeapon( pAIUnit, pWeaponItem );
				GetAILog()->Add( new CAILogAddWeapon( pAIUnit, pAIWeapon ), true );
			}
	}
	// добавляем обоймы
	for ( vector<NWorld::SItem>::iterator i = Items.begin(); i != Items.end(); ++i )
		if ( CDynamicCast<NRPG::CClipItem> pClipItem( (*i).pItem ) )
		{
			CPtr<IAIWeaponClip> pAIClip = CreateAIFireArmsWeaponClip( pClipItem );
			CPtr<IAIWeapon> pAIWeapon = pAIUnit->GetAIInventory()->GetSuitableWeapon( pAIClip );
			if ( IsValid( pAIWeapon ) )
			{
				CDynamicCast<NRPG::IInventoryItem> pInventoryItem( pClipItem.GetPtr() );
				GetAILog()->Add( new CAILogPickUpItem( pAIUnit, pInventoryItem ), true );
				GetAILog()->Add( new CAILogAddWeaponClip( pAIUnit, pAIWeapon, pAIClip ), true );
			}
		}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIAction *CreateAIShootAction( IAIState *pAIState, IAIJob *pParentJob )
{
	return new CAIShootAction( pAIState, pParentJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIAction *CreateAILootAction( IAIState *pAIState, IAIJob *pParentJob )
{
	return new CAILootAction( pAIState, pParentJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NAI;
//
REGISTER_SAVELOAD_CLASS( 0x50952143, CAIShootAction );
REGISTER_SAVELOAD_CLASS( 0x51362141, CAILootAction );