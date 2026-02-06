#include "StdAfx.h"
#include "wUnitExec.h"
#include "wUnitMove.h"
#include "wUnitAttack.h"
#include "wUnitServer.h"
#include "wMain.h"
#include "wMisc.h"
#include "RPGItem.h"
#include "RPGUnitMission.h"
#include "wObject.h"
#include "wAckBase.h"
#include "..\misc\RandomGen.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataAI.h"
#include "..\DBFormat\DataRPG.h"
#include "wUnitAttackExec.h"
namespace NWorld
{
/*
// TEST{ for testing purposes only - do not remove
const int N_TEST_MODELS = 8;
//static int nTestModelIDs[N_TEST_MODELS] = { 433, 467, 466 };
//static int nTestModelIDs[N_TEST_MODELS] = { 94, 95, 63, 468, 469, 470 };
//static int nTestModelIDs[N_TEST_MODELS] = { 64, 67, 65 };
static int nTestModelIDs[N_TEST_MODELS] = { 467, 433, 963, 958, 981, 982, 64, 67 };
//static int nTestModelIDs[N_TEST_MODELS] = { 433 };
//static int nTestModelIDs[N_TEST_MODELS] = { 963, 958 };
static int nTestModel = 0;
// TEST}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecReload
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecReload: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecReload);
private:
	ZDATA_(CCommandExecute)
	CPtr<NRPG::IInventoryItem> pItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&pItem); return 0; }

public:
	CExecReload( CUnitServer *_pUS = 0, NRPG::IInventoryItem *_pItem = 0 ): 
		CCommandExecute(_pUS), pItem(_pItem)
	{
		if ( !IsValid( pItem ) )
			pItem = pUS->GetUnitRPG()->GetInventory()->GetActive();
	}
	int GetStartAP() const { return pUS->GetActionAP( NRPG::AC_RELOAD ); }
	int GetActionAP() const { return pUS->GetActionAP( NRPG::AC_RELOAD ); }
	virtual void Run()
	{
		/*
		// TEST{ for testing purposes only - do not remove
		for ( int i = 0; i < 16; ++i )
		{
			pUS->GetWorld()->AddDebris( NDb::GetModel( nTestModelIDs[nTestModel] ), pUS->GetWorld()->GetAIMap(),
				pUS->GetPosition().GetCP() + CVec3(0,0,1), QNULL, CVec3(1,0,20), pUS->GetWorld()->GetTime() );
			nTestModel = (nTestModel + 1) % N_TEST_MODELS;
		}
		// TEST}
		return;
		*/
		pUS->DoAction( NRPG::AC_RELOAD );
		pUS->animator.Reload( pUS->GetPosition() );
		StartAction( pUS->GetWorld(), SKIPPABLE );
	}
	virtual bool TimeLabelReached()
	{
		//return false;
		pUS->GetUnitRPG()->Reload();
		if ( CDynamicCast<NRPG::IWeaponItem> pW( pItem ) )
		{
			NDb::CSound *pSound = pW->GetDBWeapon()->pSoundReload;
			NDb::CAISound *pAISound = NDb::GetAISound( 26 );
			pUS->GetWorld()->MakeAISound( pAISound, pUS, 0, pSound );
		}
		pUS->Update();
		return false;
	}
	EUnitCommandResult CanDoIt()
	{
		if ( CDynamicCast<NRPG::IWeaponItem> pW( pItem ) )
		{
			if ( !pW->CanReload( pUS->GetUnitRPG()->GetInventory() ) )
				return UCR_NO_EQUIPMENT;

			return UCR_OK;
		}

		return UCR_OK;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecCriticalLostWeapon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecCriticalLostWeapon: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecCriticalLostWeapon);
	ZDATA_(CCommandExecute)
	bool bOnlyTwoHanded;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&bOnlyTwoHanded); return 0; }
public:
	CExecCriticalLostWeapon() {}
	CExecCriticalLostWeapon( CUnitServer *_pUS, bool _bOnlyTwoHanded ): CCommandExecute(_pUS), bOnlyTwoHanded(_bOnlyTwoHanded) {}
	virtual void Run()
	{
		Finished();
		if ( pUS->animator.GetCannon() )
		{
			ASSERT( 0 ); // this should never happen, state cannon should be dropped on this critical
		}
		else
		{
			NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
			NRPG::IInventory *pInventory = pRPG->GetInventory();
			bool bTwoHanded = false;
			
			if ( CDynamicCast<NRPG::IWeaponItem> pW( pInventory->GetActive() ) )
				bTwoHanded = pW->GetDBWeapon()->pWeaponType->bTwoHanded;
			if ( bOnlyTwoHanded && !bTwoHanded )
				return;
			CUnitServer::SResItem item;
			CVec3 velocity = VNULL3;
			if ( pUS->TearOffItem( &item, (NDb::ESlot)pInventory->GetActiveSlot() ) )
			{
				if ( !bTwoHanded )
				{
					CVec3 v( 0, 0, 1 );
					item.q.Rotate( &v, v );
					velocity = 2 * v;
				}
				pUS->LaunchItem( item, velocity );
				pUS->Update();
			}
		}
	}
};
CCommandExecute* CreateLostWeapon( CUnitServer *pUS, bool bOnlyTwoHanded )
{
	return new CExecCriticalLostWeapon( pUS, bOnlyTwoHanded );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecAccidentalShot
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecAccidentalShot: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecAccidentalShot);
public:
	CExecAccidentalShot( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}
	virtual void Run()
	{
		Finished();
		NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
		if ( pUS->animator.GetCannon() )
		{
			ASSERT( 0 ); // this should never happen, state cannon should be dropped on this critical
		}
		else
		{
			CRay ray;
			if ( !pUS->GetBarrelDir( &ray ) )
				return;
			//
			vector<NRPG::CAttackPortion> attack;
			pRPG->CreateAttack( &attack, true, 0 );
			if ( attack.empty() )
				return;
			//
			vector<NRPG::IAttackable *> ignores;
			CPtr<CCannon> pCannon = pUS->animator.GetCannon();
			if ( pCannon )
				ignores.push_back( pCannon );
			else
				ignores.push_back( pUS );
			//
			CPtr<NRPG::IWeaponItem> pWeaponItem = pUS->GetUnitRPG()->GetWeaponItem();
			if ( !IsValid( pWeaponItem ) )
				return;
			CDBPtr<NDb::CRPGWeapon> pWeapon = pWeaponItem->GetDBWeapon();
			//
			CPtr<NDb::CModel> pTrailEffect = 0;
			if ( pWeapon->pTrailEffect )
			{
				SRand sRand;
				pTrailEffect = pWeapon->pTrailEffect->CreateModel( &sRand );
			}
			//
			ray.ptOrigin += ray.ptDir * pUS->GetMinClearDistance();
			for ( vector<NRPG::CAttackPortion>::const_iterator i = attack.begin(); i != attack.end(); ++i )
				pUS->GetWorld()->PerformRangedAttack( *i, ray, ignores, 
					pUS->GetWorld()->GetTime()->GetValue(), pTrailEffect, pWeapon->fTrailSpeed );
			pUS->CreateFlash();
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CreateAccidentalShot( CUnitServer *pUS )
{
	CRay ray;
	if ( !pUS->GetBarrelDir( &ray ) )
		return 0;
	//
	if ( pUS->GetUnitRPG()->GetWeaponType() == NDb::WT_RLAUNCHER )
		return new CExecLaunchRocket( pUS, VNULL3 );
	else
		return new CExecAccidentalShot( pUS );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecStartCombat
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecStartCombat: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecStartCombat);
public:
	CExecStartCombat( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}
	virtual void Run()
	{
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecExplode
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecExplode: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecExplode);
public:
	CExecExplode( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}
	virtual void Run()
	{
		StartAction( pUS->GetWorld(), NORMAL );
		pUS->GetWorld()->Explode( pUS->GetPosition().GetCP(), 100 );
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecShootMode
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecShootMode: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecShootMode);
private:
	ZDATA_(CCommandExecute)
	CObj<CCmdShootMode> pCmd;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&pCmd); return 0; }
public:
	CExecShootMode() {}
	CExecShootMode( CUnitServer *_pUS, CCmdShootMode *_pCmd ): CCommandExecute(_pUS), pCmd( _pCmd ) {}
	virtual void Run()
	{
		CPtr<NRPG::IInventoryItem> pItem = pUS->GetUnitRPG()->GetInventory()->GetActive();
		ASSERT( IsValid( pItem ) );
		if ( IsValid( pItem ) )
		{
			if ( CDynamicCast<NRPG::IWeaponItem> pWeapon( pItem ) )
				pWeapon->SetShootMode( pCmd->eMode );
		}

		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecTeleport
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecTeleport: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecTeleport);
	ZDATA_(CCommandExecute)
	CObj<CCmdTeleport> pCmd;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&pCmd); return 0; }
public:
	CExecTeleport() {}
	CExecTeleport( CUnitServer *_pUS, CCmdTeleport *_p ): CCommandExecute(_pUS), pCmd(_p) {}
	virtual void Run()
	{
		CObj<CExecTeleport> pHold(this);
		bool bHasCheat = pUS->GetPlayer()->IsCheatEnabled( CHEAT_TELEPORT );
		ASSERT( bHasCheat );
		const NAI::SPosition &pos = pCmd->pos.pos;
		if ( bHasCheat && pos.GetNetwork()->IsPassable( pos.p ) )
		{
			pUS->animator.PlaceUnit( pCmd->pos );
			pUS->SetPosition( pCmd->pos );
		}
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecLeaveInventory
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecLeaveInventory: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecLeaveInventory);
public:
	CExecLeaveInventory( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}
	virtual void Run()
	{
		/*
		CUnitAnimator &animator = pUS->animator;
		NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
		NRPG::IInventory *pInventory = pRPG->GetInventory();
		const NAI::SUnitPosition &position = pUS->GetPosition();
		if ( animator.IsActiveItem() && pInventory->GetActiveSlot() < 0 )
			animator.SetActiveItem( false );
		bool bActive = animator.IsActiveItem();
		animator.SetWeaponAnimation( bActive ? pRPG->GetWeaponType() : NDb::WT_DEFAULT );
		pUS->SetUndrawItem( !bActive );
		animator.PlaceUnit( position );
		*/
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecSetActiveItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecSetActiveItem: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecSetActiveItem);
	ZDATA_(CCommandExecute)
	NDb::ESlot slot;
	int nStage;
	bool bTwoHeavy;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&slot); f.Add(3,&nStage); f.Add(4,&bTwoHeavy); return 0; }
public:
	CExecSetActiveItem() {}
	CExecSetActiveItem( CUnitServer *_pUS, NDb::ESlot _slot ): CCommandExecute(_pUS), slot(_slot) {}
	virtual void Run()
	{
		NRPG::IInventory *pInventory = pUS->GetUnitRPG()->GetInventory();
		NDb::ESlot oldSlot = (NDb::ESlot)pInventory->GetActiveSlot();

		NDb::EItemSubType subType = NDb::SUBTYPE_NONE;
		if ( pInventory->Get(oldSlot) )
			subType = pInventory->Get(oldSlot)->GetDBItem()->subType;
		NDb::EItemSubType subTypeNext = NDb::SUBTYPE_NONE;
		if ( pInventory->Get(slot) )
			subTypeNext = pInventory->Get(slot)->GetDBItem()->subType;

		pInventory->Activate( slot );

		bool bActive = pUS->animator.IsActiveItem();
		if ( slot == oldSlot && bActive )
		{
			Finished();
			return;
		}
		if ( !pUS->animator.CanActivateItem() ) // on ladder, for example
		{
			NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
			pUS->animator.SetWeaponAnimation( bActive ? pRPG->GetWeaponType() : NDb::WT_DEFAULT );
			pUS->SetUndrawItem( !bActive );
			Finished();
			return;
		}
		StartAction( pUS->GetWorld(), NOBLOCK );
		bTwoHeavy = false;
		if ( subTypeNext == NDb::SUBTYPE_HEAVY && subType == NDb::SUBTYPE_HEAVY )
			bTwoHeavy = true;
		if ( subTypeNext == NDb::SUBTYPE_MINE_DETECTOR || subType == NDb::SUBTYPE_MINE_DETECTOR )
			bTwoHeavy = true;
		nStage = 1;
		if ( bActive && subType != NDb::SUBTYPE_NONE )
		{
			if ( subType == NDb::SUBTYPE_HEAVY || subType == NDb::SUBTYPE_MINE_DETECTOR )
				pUS->animator.DeactivateItem( pUS->GetPosition(), true, bTwoHeavy, NDb::BELT_M1 );
			else
			{
				int nPlace = pInventory->GetPlaceBySubType( subType );
				pUS->animator.DeactivateItem( pUS->GetPosition(), false, nPlace == -1, (NDb::EItemPlace)nPlace );
			}
		}
		else
			AnimationFinished();
	}
	virtual bool TimeLabelReached()
	{
		if ( !nStage )
			return false;
		pUS->SetUndrawItem( nStage == 1, bTwoHeavy && nStage == 1 );
		return nStage == 1;
	}
	virtual void AnimationFinished()
	{
		if ( !nStage )
			return;
		NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
		NRPG::IInventory *pInventory = pRPG->GetInventory();
		if ( nStage == 2 || !pInventory->Get(slot) )
		{
			Finished();
			return;
		}
		nStage = 2;
		NDb::EItemSubType subType = pInventory->Get(slot)->GetDBItem()->subType;
		if ( subType == NDb::SUBTYPE_HEAVY || subType == NDb::SUBTYPE_MINE_DETECTOR )
			pUS->animator.ActivateItem( pUS->GetPosition(), true, bTwoHeavy, NDb::BELT_M1, pRPG->GetWeaponType() );
		else
		{
			int nPlace = pInventory->GetPlaceBySubType( subType );
			pUS->animator.ActivateItem( pUS->GetPosition(), false, nPlace == -1, (NDb::EItemPlace)nPlace, pRPG->GetWeaponType() );
		}
	}
	virtual void Cancel()
	{
		NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
		NRPG::IInventory *pInventory = pUS->GetUnitRPG()->GetInventory();
		bool bActive = pInventory->GetActive() != 0;
		pUS->SetUndrawItem( !bActive );
		pUS->animator.SetWeaponAnimation( bActive ? pRPG->GetWeaponType() : NDb::WT_DEFAULT );
		pUS->animator.SetActiveItem( bActive );
		pUS->animator.AlignTime( 50 );
		pUS->animator.PlaceUnit( pUS->GetPosition() );
		nStage = 0;
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecUnloadInventoryItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecUnloadInventoryItem: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecUnloadInventoryItem);
	ZDATA_(CCommandExecute)
	CPtr<NRPG::IInventoryItem> pItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&pItem); return 0; }
public:
	CExecUnloadInventoryItem() {}
	CExecUnloadInventoryItem( CUnitServer *_pUS, NRPG::IInventoryItem* _pItem ): CCommandExecute(_pUS), pItem(_pItem) {}
	virtual void Run()
	{
		ASSERT( pItem );
		pUS->GetUnitRPG()->Unload( pItem );
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecSetWishPose
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecSetWishPose: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecSetWishPose);
	ZDATA_(CCommandExecute)
	NAI::EPose pose;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); f.Add(2,&pose); return 0; }
public:
	CExecSetWishPose() {}
	CExecSetWishPose( CUnitServer *_pUS, NAI::EPose _pose ): CCommandExecute(_pUS), pose(_pose) {}
	int GetActionAP() const
	{
		switch( pose )
		{
		case NAI::RUN:
			return pUS->GetActionAP( NRPG::AC_POSE_RUN );
		case NAI::WALK:
			return pUS->GetActionAP( NRPG::AC_POSE_WALK );
		case NAI::CROUCH:
			return pUS->GetActionAP( NRPG::AC_POSE_CROUCH );
		case NAI::CRAWL:
			return pUS->GetActionAP( NRPG::AC_POSE_CRAWL );
		}

		ASSERT( 0 );
		return 0;
	}
	virtual void Run()
	{
		pUS->SetWishPose( pose );
		pUS->SetRunning( pose == NAI::RUN );
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecNeedReload
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecNeedReload: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecNeedReload);
	ZDATA_(CCommandExecute)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); return 0; }
public:
	CExecNeedReload( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}
	virtual void Run()
	{
		pUS->GetWorld()->GetGlobalAck()->OnLastPieceOfAmmo( pUS );
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecWeaponJammed
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecWeaponJammed: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecWeaponJammed);
	ZDATA_(CCommandExecute)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); return 0; }
public:
	CExecWeaponJammed( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}
	virtual void Run()
	{
		pUS->GetWorld()->GetGlobalAck()->OnWeaponJammed( pUS );
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecOrderConfirmation
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecOrderConfirmation: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecOrderConfirmation);
	ZDATA_(CCommandExecute)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); return 0; }
public:
	CExecOrderConfirmation( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}
	virtual void Run()
	{
		pUS->GetWorld()->GetGlobalAck()->OnOrderConfirmation( pUS );
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecImpossibleToPerformAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CExecImpossibleToPerformAction: public CCommandExecute
{
	OBJECT_BASIC_METHODS(CExecImpossibleToPerformAction);
	ZDATA_(CCommandExecute)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCommandExecute*)this); return 0; }
public:
	CExecImpossibleToPerformAction( CUnitServer *_pUS = 0 ): CCommandExecute(_pUS) {}
	virtual void Run()
	{
		pUS->GetWorld()->GetGlobalAck()->OnImpossibleToPerformAction( pUS );
		Finished();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TExec>
CCommandExecute* CreateSimpleExec( TExec *pExec, EUnitCommandResult *pError )
{
	CPtr<TExec> p( pExec );

	*pError = pExec->CanDoIt();
	if ( *pError != UCR_OK )
		return 0;

	return p.Extract();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CreateExecutor( CUnitServer *pUS, CCmd *pCmd, EUnitCommandResult *pError )
{
	CPtr<CCmd> pHold( pCmd );
	CWorld *pWorld = pUS->GetWorld();
	CUnitAnimator &animator = pUS->animator;
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	const NAI::SUnitPosition &position = pUS->GetPosition();

	*pError = UCR_OK;

	if ( CDynamicCast<CCmdPath> pCmdPath( pCmd ) )
	{
		vector<NAI::SPathPlace> dst;
		dst.push_back( pCmdPath->ptDst.p );
		CPtr<NAI::CPath> pPath = pWorld->FindPath( pUS, position.pos.p, dst, 0, true, pCmdPath->eParams, pCmdPath->bStrafe, true );
		if ( IsValid( pPath ) )
			return CreateMoveExecutor( pUS, pPath, pCmdPath->eParams, ITEM_NO_MATTER, pError );
		return 0;
	}
	else if ( CDynamicCast<CCmdStartCombat> pCmdStartCombat( pCmd ) )
		return new CExecStartCombat( pUS );
	else if ( CDynamicCast<CCmdExplode> pExplode( pCmd ) )
		return new CExecExplode( pUS );
	else if ( CDynamicCast<CCmdShootMode> pShootMode( pCmd ) )
		return new CExecShootMode( pUS, pShootMode );
	else if ( CDynamicCast<CCmdInventoryLeave> pLeaveInventory( pCmd ) )
		return new CExecLeaveInventory( pUS );
	else if ( CDynamicCast<CCmdSetActiveItem> pSetActiveItem( pCmd ) )
		return new CExecSetActiveItem( pUS, (NDb::ESlot)pSetActiveItem->nSlot );
	else if ( CDynamicCast<CCmdWishPose> p( pCmd ) )
		return new CExecSetWishPose( pUS, p->pose );
	else if ( CDynamicCast<CCmdReload> pReload( pCmd ) )
		return CreateSimpleExec( new CExecReload( pUS, pReload->pItem ), pError );
	else if ( CDynamicCast<CCmdUnloadInventoryItem> pUnloadItem( pCmd ) )
		return new CExecUnloadInventoryItem( pUS, pUnloadItem->GetItem() );
	else if ( CDynamicCast<CCmdTeleport> pTeleport( pCmd ) )
		return new CExecTeleport( pUS, pTeleport );
	else if ( CDynamicCast<CCmdNeedReload> pNeedReload( pCmd ) )
		return new CExecNeedReload( pUS );
	else if ( CDynamicCast<CCmdWeaponJammed> pWeaponJammed( pCmd ) )
		return new CExecWeaponJammed( pUS );
	else if ( CDynamicCast<CCmdOrderConfirmation> pOrderConfirmation( pCmd ) )
		return new CExecOrderConfirmation( pUS );
	else if ( CDynamicCast<CCmdImpossibleToPerformAction> pImpossibleToPerformAction( pCmd ) )
		return new CExecImpossibleToPerformAction( pUS );
	else if ( CCommandExecute *pExec = CreateActionExecutor( pUS, pCmd, pError ) )
		return pExec;

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0x00122170, CExecReload )
REGISTER_SAVELOAD_CLASS( 0x00722170, CExecCriticalLostWeapon )
REGISTER_SAVELOAD_CLASS( 0x00722171, CExecAccidentalShot )
REGISTER_SAVELOAD_CLASS( 0x01122130, CExecExplode )
REGISTER_SAVELOAD_CLASS( 0x01122131, CExecLeaveInventory )
REGISTER_SAVELOAD_CLASS( 0x01122133, CExecSetActiveItem )
REGISTER_SAVELOAD_CLASS( 0x01122134, CExecSetWishPose )
REGISTER_SAVELOAD_CLASS( 0x01122135, CExecTeleport )
REGISTER_SAVELOAD_CLASS( 0x52062175, CExecNeedReload )
REGISTER_SAVELOAD_CLASS( 0x52062176, CExecWeaponJammed )
REGISTER_SAVELOAD_CLASS( 0x52062177, CExecOrderConfirmation )
REGISTER_SAVELOAD_CLASS( 0x52062178, CExecImpossibleToPerformAction )
REGISTER_SAVELOAD_CLASS( 0xB2062179, CExecStartCombat )