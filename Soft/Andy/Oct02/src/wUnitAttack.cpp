#include "StdAfx.h"
#include "wUnitAttack.h"
#include "wUnitMove.h"
#include "wUnitServer.h"
#include "Grid.h"
#include "wMain.h"
#include "RPGItem.h"
//#include "RPGItemSet.h" // CRAP
#include "RPGUnitMission.h"
#include "RPGGame.h"
#include "aiMap.h"
#include "aiCollider.h"
#include "..\misc\RandomGen.h"
#include "wObject.h"
#include "wUnitStates.h"
#include "wAckBase.h"
#include "RPGToHit.h"
#include "..\Misc\LogStream.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataAI.h"
#include "RPGCritical.h"
#include "aiPath.h"
#include "wUnitAttackExec.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EAllowPoseMask
{
	PM_LAY     = 1,
	PM_CROUCH  = 2,
	PM_STAND   = 4,
	PM_ALL     = 7
};
////////////////////////////////////////////////////////////////////////////////////////////////////
EActionType GetActionType( CUnitServer *pUS )
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	if ( !pRPG )
	{
		ASSERT(0);
		return AT_NONE;
	}
	if ( pRPG->GetCannonItem() )
		return AT_CANNON;
	NRPG::IInventory *pInventory = pRPG->GetInventory();
	NRPG::IInventoryItem *pItem = pInventory->GetActive();
	//

	if ( pItem == 0 )
		return AT_MELEE;
	else if ( CDynamicCast<NRPG::IWeaponItem>pWeapon( pItem ) )
	{
		if ( pWeapon->GetDBWeapon()->bBazookaLogic )
			return AT_BAZOOKA;
		else
		{
			if ( pWeapon->GetShootMode() == NDb::SM_Snipe && pUS->IsSniping() )
				return AT_SNIPE;
			else
				return AT_SHOOT;
		}
	}
	else if ( CDynamicCast<NRPG::IFirstAidItem>( pItem ) )
		return AT_FIRSTAID;
	else if ( CDynamicCast<NRPG::IGrenadeItem>( pItem ) )
		return AT_GRENADE;
	else if ( CDynamicCast<NRPG::IMeleeWeaponItem> pMelee(pItem) )
	{
		if ( pMelee->GetDBMeleeWeapon()->bThrowing )
			return AT_THROW;

		return AT_MELEE;
	}

	return AT_NONE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void GetDirectedPoints( NAI::IPathNetwork *pNet, const NAI::SPathPlace &from, const CVec3 &ptTarget, 
	vector<NAI::SPathPlace> *pRes, int nPoseMask = PM_ALL )
{
	NAI::EDirection dir = NRPG::GetShootDirection( pNet, from, ptTarget ) ;
	if ( nPoseMask & PM_LAY )
		pRes->push_back( NAI::SPathPlace( from.GetX(), from.GetY(), from.GetLayer(), dir, NAI::CM_LAY, false ) );
	if ( nPoseMask & PM_CROUCH )
		pRes->push_back( NAI::SPathPlace( from.GetX(), from.GetY(), from.GetLayer(), dir, NAI::CM_CROUCH, false ) );
	if ( nPoseMask & PM_STAND )
		pRes->push_back( NAI::SPathPlace( from.GetX(), from.GetY(), from.GetLayer(), dir, NAI::CM_STAND, false ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void GetMeleeAttackPlaces( CUnitServer *pUS, const CVec3 &ptTarget, vector<NAI::SPathPlace> *pRes )
{
	NAI::IPathNetwork *pNet = pUS->GetWorld()->GetPathNetwork();
	SSphere s;
	s.ptCenter = pUS->GetPosition().GetCP();
	s.fRadius = 5; // CRAP need more grounded number then this
	vector<NAI::SPathPlace> res;
	pNet->GetNearPlaces( s, &res );
	NAI::SUnitPosition from( pUS->GetPosition() );
	for ( int k = 0; k < res.size(); ++k )
	{
		if ( !pNet->IsNativePassable( res[k] ) )
			continue;
		from.pos.p = res[k];
		if ( IsWithinHumanReach( from.GetCP(), ptTarget, F_MELEE_DISTANCE ) )
			GetDirectedPoints( pNet, from.pos.p, ptTarget, pRes );
	}	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void GetSnipeAttackPlaces( CUnitServer *pUS, CUnitServer *pTarget, vector<NAI::SPathPlace> *pRes )
{
	if ( !IsValid( pTarget ) || !IsValid( pUS ) )
		return;
	//
	CWorld *pWorld = pUS->GetWorld();
	NAI::SPathPlace p = pUS->GetPosition().pos.p;
	p.SetDirection( pWorld->GetPathNetwork()->GetClosestDir( p,	pTarget->GetPosition().pos.p ) );
	pRes->push_back( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void GetHumanReachPlaces( CUnitServer *pUS, const CVec3 &ptTarget, vector<NAI::SPathPlace> *pRes )
{
	NAI::IPathNetwork *pNet = pUS->GetWorld()->GetPathNetwork();
	SSphere s;
	s.ptCenter = ptTarget; 
	s.fRadius = 5; // CRAP need more grounded number then this
	vector<NAI::SPathPlace> res;
	pNet->GetNearPlaces( s, &res );
	NAI::SUnitPosition from( pUS->GetPosition() );
	for ( int k = 0; k < res.size(); ++k )
	{
		if ( !pNet->IsNativePassable( res[k] ) )
			continue;
		from.pos.p = res[k];
		if ( IsWithinHumanReach( from.GetCP(), ptTarget, F_HEAL_DISTANCE ) )
			GetDirectedPoints( pNet, from.pos.p, ptTarget, pRes );
	}	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, CCmdShootTile *pCmd, vector<NAI::SPathPlace> *pRes )
{
	CWorld *pWorld = pUS->GetWorld();
	NAI::IPathNetwork *pNet = pWorld->GetPathNetwork();
	switch ( GetActionType( pUS ) )
	{
		case AT_MELEE:
			GetMeleeAttackPlaces( pUS, pCmd->ptTarget, pRes );
			break;
		case AT_SHOOT:
		case AT_GRENADE:
		case AT_THROW:
		case AT_BAZOOKA:
			GetDirectedPoints( pNet, pUS->GetPosition().pos.p, pCmd->ptTarget, pRes );
			break;
		default:
			ASSERT( 0 );
			break;
	}

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, CCmdShootObject *pCmd, vector<NAI::SPathPlace> *pRes )
{
	CDynamicCast<CUnitServer> pTarget( pCmd->pTarget );
	if ( !IsValid( pTarget ) )
	{
		pRes->push_back( pUS->GetPosition().pos.p );
		return UCR_NO_TARGET;
	}

	CWorld *pWorld = pUS->GetWorld();
	NAI::IPathNetwork *pNet = pWorld->GetPathNetwork();
	CVec3 ptTo;
	pWorld->GetAIMap()->GetUnitHLPos( &ptTo, pTarget, pCmd->eHL == NAI::HL_ANY ? NAI::HL_BODY : pCmd->eHL );
	switch ( GetActionType( pUS ) )
	{
		case AT_MELEE:
			GetMeleeAttackPlaces( pUS, ptTo, pRes );
			break;
		case AT_SNIPE:
		case AT_SHOOT:
		case AT_BAZOOKA:
			GetDirectedPoints( pNet, pUS->GetPosition().pos.p, ptTo, pRes );
			break;
		default:
			ASSERT( 0 );
	}

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, IGetApproaches *pObject, vector<NAI::SPathPlace> *pRes )
{
	ASSERT( pObject );
	pObject->GetApproaches( pRes, pUS->GetWorld()->GetPathNetwork() );
	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, CCmdHeal *pCmd, vector<NAI::SPathPlace> *pRes )
{
	CDynamicCast<CUnitServer> pTarget( pCmd->pTarget );
	if ( !IsValid( pTarget ) )
	{
		pRes->push_back( pUS->GetPosition().pos.p );
		return UCR_NO_TARGET;
	}

	if ( pTarget == pUS )
		pRes->push_back( pUS->GetPosition().pos.p );
	else
		GetHumanReachPlaces( pUS, pTarget->GetPosition().GetEyePosition(), pRes );

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, CCmdCannon *pCmd, vector<NAI::SPathPlace> *pRes )
{
	if ( !IsValid( pCmd->pObject ) )
		return UCR_NO_TARGET;

	CDynamicCast<IGetApproaches> pAppr( pCmd->pObject );
	return GetActionValidPlaces( pUS, pAppr, pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, CCmdOpenClose *pCmd, vector<NAI::SPathPlace> *pRes )
{
	if ( !IsValid( pCmd->pObject ) )
		return UCR_NO_TARGET;

	CDynamicCast<IGetApproaches> pAppr( pCmd->pObject );
	return GetActionValidPlaces( pUS, pAppr, pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, CCmdUsePassage *pCmd, vector<NAI::SPathPlace> *pRes )
{
	if ( !IsValid( pCmd->pPassageObject ) )
		return UCR_NO_TARGET;

	CDynamicCast<IGetApproaches> pAppr( pCmd->pPassageObject );
	return GetActionValidPlaces( pUS, pAppr, pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, CCmdTakeCorpse *pCmd, vector<NAI::SPathPlace> *pRes )
{
	if ( !IsValid( pCmd->pCorpse ) )
		return UCR_NO_TARGET;

	NAI::IPathNetwork *pNet = pUS->GetWorld()->GetPathNetwork();
	CVec3 ptTarget(0,0,0);
	pUS->GetWorld()->GetAIMap()->GetUnitHLPos( &ptTarget, pCmd->pCorpse, -1 );
	SSphere s;
	s.ptCenter = ptTarget - CVec3(0,0,0.5f);
	s.fRadius = 1; // CRAP need more grounded number then this
	vector<NAI::SPathPlace> res;
	pNet->GetNearPlaces( s, &res );
	for ( int k = 0; k < res.size(); ++k )
		GetDirectedPoints( pNet, res[k], ptTarget, pRes, PM_STAND );

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult GetActionValidPlaces( CUnitServer *pUS, CCmdMoveInventoryItem *pCmd, vector<NAI::SPathPlace> *pRes )
{
	if ( pCmd->GetSource().eType == SItem::GROUND )
	{
		if ( !IsValid( pCmd->GetSource().pWorldItem ) )
			return UCR_NO_TARGET;

		CVec3 ptTo = pCmd->GetSource().pWorldItem->GetPos();
		GetHumanReachPlaces( pUS, ptTo, pRes );
		return UCR_OK;
	}

	GetHumanReachPlaces( pUS, pCmd->GetTarget().pUnit->GetPosition().GetCP(), pRes );
	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Execute creators
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TExec>
CCommandExecute* CreateSimpleAction( CUnitServer *pUS, TExec *pExec, EUnitCommandResult *pError )
{
	CPtr<TExec> p( pExec );
	*pError = pExec->CanDoIt( pUS->GetPosition() );
	if ( *pError != UCR_OK )
		return 0;
	return p.Extract();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CreateMoveExecutor( CUnitServer *pUS, NAI::CPath *pPath, NAI::EFindPathParams eParams, ENeedActiveItem eActive, EUnitCommandResult *pError )
{
	if ( !pUS->GetUnitRPG()->CanMove() )
	{
		*pError = UCR_GENERAL_FAILURE;
		return 0;
	}

	CExecQueue *pCommand = new CExecQueue( pUS );
	pCommand->AddPath( pPath, eParams, eActive );
	return pCommand;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static CCommandExecute* CreateActionExecMove( CUnitServer *pUS, const vector<NAI::SPathPlace> &dst, ENeedActiveItem eActive, EUnitCommandResult *pError )
{
	CWorld *pWorld = pUS->GetWorld();
	CObj<NAI::CPath> pPath = pWorld->FindPath( pUS, pUS->GetPosition().pos.p, dst, 0, false, NAI::PF_USE_POSEDIR );
	if ( IsValid( pPath ) )
		return CreateMoveExecutor( pUS, pPath, NAI::PF_USE_POSEDIR, eActive, pError );

	*pError = UCR_PATH_NOT_FOUND;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TCommand, class TExecAction>
static CCommandExecute* CreateActionQueue( CUnitServer *pUS, TCommand *pCmd, TExecAction *pAction, ENeedActiveItem eActive, EUnitCommandResult *pError )
{
	CObj<CCommandExecute> pHold( pAction );

	vector<NAI::SPathPlace> dst;
	*pError = GetActionValidPlaces( pUS, pCmd, &dst );
	if ( dst.empty() )
		return 0;

	NAI::SUnitPosition from = pUS->GetPosition();
	EUnitCommandResult eResult = UCR_GENERAL_FAILURE;
	vector<NAI::SPathPlace> spots;
	for ( int k = 0; k < dst.size(); ++k )
	{
		from.pos.p = dst[k];
		eResult = pAction->CanDoIt( from );
		if ( eResult == UCR_OK )
			spots.push_back( dst[k] );
	}
	if ( spots.empty() )
	{
		*pError = eResult;
		return 0;
	}

	CCommandExecute *pMove = CreateActionExecMove( pUS, spots, eActive, pError );
	if ( !pMove )
		return 0;

	if ( CDynamicCast<CExecQueue> pQueue( pMove ) )
	{
		pQueue->AddExecutor( pAction );
		return pMove;
	}

	CExecQueue *pRes = new CExecQueue( pUS );
	pRes->AddExecutor( pMove );
	pRes->AddExecutor( pAction );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CreateActionExecutor( CUnitServer *pUS, CCmd *pCmd, EUnitCommandResult *pError )
{
	*pError = UCR_OK;

	if ( CDynamicCast<CCmdShootTile> pAttackTile( pCmd ) )
	{
		EActionType eType = GetActionType( pUS );
		switch ( eType )
		{
			case AT_NONE:
			case AT_FIRSTAID:
				*pError = UCR_UNAVAILABLE;
				return 0;
			case AT_MELEE:
				{
					NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
					ENeedActiveItem eActive = ( pRPG->GetWeaponType() == NDb::WT_DEFAULT ? ITEM_INACTIVE : ITEM_ACTIVE );
					return CreateActionQueue( pUS, pAttackTile.GetPtr(), new CExecMeleeTile( pUS, pAttackTile->ptTarget ), eActive, pError );
				}
			case AT_GRENADE:
				return CreateActionQueue( pUS, pAttackTile.GetPtr(), new CExecThrowGrenade( pUS, pAttackTile->ptTarget ), ITEM_ACTIVE, pError );
			case AT_SHOOT:
				return CreateActionQueue( pUS, pAttackTile.GetPtr(), new CExecShootTile( pUS, pAttackTile->ptTarget ), ITEM_ACTIVE, pError );
			case AT_THROW:
				return CreateActionQueue( pUS, pAttackTile.GetPtr(), new CExecThrowKnife( pUS, pAttackTile->ptTarget + CVec3(0,0,0.5f) ), ITEM_ACTIVE, pError );
			case AT_BAZOOKA:
				return CreateActionQueue( pUS, pAttackTile.GetPtr(), new CExecLaunchRocket( pUS, pAttackTile->ptTarget + CVec3(0,0,0.5f) ), ITEM_ACTIVE, pError );
			case AT_CANNON:
				return CreateSimpleAction( pUS, new CExecShootTile( pUS, pAttackTile->ptTarget ), pError );
			default:
				*pError = UCR_GENERAL_FAILURE;
				ASSERT( 0 );
				return 0;
		}
	}
	else if ( CDynamicCast<CCmdShootObject> pAttackObject( pCmd ) )
	{
		CDynamicCast<NWorld::CUnitServer> pTarget( pAttackObject->pTarget );
		if ( CDynamicCast<NRPG::IWeaponItem> pWeapon( pUS->GetUnitRPG()->GetInventory()->GetActive() ) )
		{
			if ( IsValid( pWeapon ) && pWeapon->GetShootMode() == NDb::SM_Snipe && !pUS->IsSniping() )
				return CreateActionQueue( pUS, pAttackObject.GetPtr(), new CExecSnipeAim( pUS, pTarget ), ITEM_ACTIVE, pError );
		}

		if ( IsValid( pTarget ) || !IsValid( pAttackObject->pTarget ) ) //// CRAP!: Для CanDo
		{
			EActionType eType = GetActionType( pUS );
			switch ( eType )
			{
				case AT_NONE:
				case AT_FIRSTAID:
					*pError = UCR_UNAVAILABLE;
					return 0;
				case AT_MELEE:
					{
						NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
						ENeedActiveItem eActive = ( pRPG->GetWeaponType() == NDb::WT_DEFAULT ? ITEM_INACTIVE : ITEM_ACTIVE );
						return CreateActionQueue( pUS, pAttackObject.GetPtr(), new CExecMeleeUnit( pUS, pTarget, pAttackObject->eHL, pAttackObject->nExtraAttackAP ), eActive, pError );
					}
				case AT_SHOOT:
				case AT_SNIPE:
					return CreateActionQueue( pUS, pAttackObject.GetPtr(), new CExecShootUnit( pUS, pTarget, pAttackObject->eHL, pAttackObject->nExtraAttackAP ), ITEM_ACTIVE, pError );
				case AT_CANNON:
					return CreateSimpleAction( pUS, new CExecShootUnit( pUS, pTarget, pAttackObject->eHL, pAttackObject->nExtraAttackAP ), pError );
			}
		}

		/// CRAP #1: Некотырые виды оружия атакуют тайл
		/// CRAP #2: Атака объекта = атака тайла под объектом
		if ( !IsValid( pAttackObject->pTarget ) )
		{
			CObj<CCmdShootTile> pShoot( new CCmdShootTile( pUS->GetPosition().GetCP() ) );
			CCommandExecute *pExecutor = CreateActionExecutor( pUS, pShoot, pError );
			if ( *pError == UCR_OK )
				*pError = UCR_NO_TARGET;

			return pExecutor;
		}

		CVec3 ptTarget(0,0,0);
		pUS->GetWorld()->GetAIMap()->GetUnitHLPos( &ptTarget, pAttackObject->pTarget, -1 );
		CObj<CCmdShootTile> pShoot( new CCmdShootTile( ptTarget ) );
		return CreateActionExecutor( pUS, pShoot, pError );
	}
	else if ( CDynamicCast<CCmdHeal> pHeal( pCmd ) )
	{
		CDynamicCast<NWorld::CUnitServer> pTarget( pHeal->pTarget );
		return CreateActionQueue( pUS, pHeal.GetPtr(), new CExecHeal( pUS, pTarget ), ITEM_ACTIVE, pError );
	}
	else if ( CDynamicCast<CCmdCannon> pCannonAtk( pCmd ) )
		return CreateActionQueue( pUS, pCannonAtk.GetPtr(), new CExecCannon( pUS, pCannonAtk->pObject, true ), ITEM_INACTIVE, pError );
	else if ( CDynamicCast<CCmdExitCannon> pCannonExit( pCmd ) )
		return CreateSimpleAction( pUS, new CExecCannon( pUS, pCannonExit->pCannon, false ), pError );
	else if ( CDynamicCast<CCmdOpenClose> pOpenClose( pCmd ) )
	{
		CCommandExecute* pExec = CreateActionQueue( pUS, pOpenClose.GetPtr(), 
			new CExecOpenClose( pUS, pOpenClose ), ITEM_INACTIVE, pError );
		if ( CDynamicCast<CExecQueue> pQueue( pExec ) )
			pQueue->CheckOpenCloseOnce();
		else if ( pExec != 0 )
		{
			ASSERT(0);
		}
		return pExec;
	}
	else if ( CDynamicCast<CCmdUsePassage> pUsePassage( pCmd ) )
		return CreateActionQueue( pUS, pUsePassage.GetPtr(), new CExecUsePassage( pUS, pUsePassage ), ITEM_INACTIVE, pError );
	else if ( CDynamicCast<CCmdMoveInventoryItem> pMoveItem( pCmd ) )
	{
		if ( pMoveItem->GetSource().eType == SItem::GROUND )
			return CreateActionQueue( pUS, pMoveItem.GetPtr(), new CExecMoveInventoryItem( pUS, pMoveItem ), ITEM_INACTIVE, pError );
		else if ( pMoveItem->GetSource().pUnit != pMoveItem->GetTarget().pUnit )
			return CreateActionQueue( pUS, pMoveItem.GetPtr(), new CExecMoveInventoryItem( pUS, pMoveItem ), ITEM_INACTIVE, pError );

		return CreateSimpleAction( pUS, new CExecMoveInventoryItem( pUS, pMoveItem ), pError );
	}
	else if ( CDynamicCast<CCmdTakeCorpseOnDeploy> pCmdCorpse( pCmd ) )
		return new CExecTakeCorpseOnDeploy( pCmdCorpse->pCarrier, pCmdCorpse->pCorpse, pCmdCorpse->bDead );
	else if ( CDynamicCast<CCmdTakeCorpse> pCmdCorpse( pCmd ) )
	{
		CDynamicCast<CUnitServer> pDeadUnit( pCmdCorpse->pCorpse );
		return CreateActionQueue( pUS, pCmdCorpse.GetPtr(), new CExecCorpse( pUS, pDeadUnit, true ), ITEM_INACTIVE, pError );
	}
	else if ( CDynamicCast<CCmdDropCorpse> pCmdCorpse( pCmd ) )
		return CreateSimpleAction( pUS, new CExecCorpse( pUS, pCmdCorpse->pCorpse, false ), pError );
	else if ( CDynamicCast<CCmdCollectSnipeAP> pCollectSnipeAP(pCmd) )
		return CreateSimpleAction( pUS, new CExecCollectSnipeAP( pUS, pCollectSnipeAP->eAP ), pError );

	*pError = UCR_INVALID_COMMAND;
	ASSERT( 0 );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NWorld;
