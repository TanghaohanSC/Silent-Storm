#include "StdAfx.h"
#include "wUnitAttack.h"
#include "wUnitMove.h"
#include "wUnitServer.h"
#include "Grid.h"
#include "wMain.h"
#include "wUnitCommands.h"
#include "RPGItem.h"
#include "RPGItemSet.h" // CRAP
#include "RPGUnitMission.h"
#include "RPGGame.h"
#include "aiMap.h"
#include "aiCollider.h"
#include "..\misc\RandomGen.h"
#include "..\Misc\LogStream.h"
#include "wObject.h"
#include "wUnitStates.h"
#include "wAckBase.h"
#include "RPGToHit.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataAI.h"
#include "RPGCritical.h"
#include "aiPath.h"
#include "wUnitAttack.h"
#include "wUnitAttackExec.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsWithinHumanReach( const CVec3 &ptFrom, const CVec3 &ptTarget, float fPlaneDist )
{
	if ( sqr( ptFrom.x - ptTarget.x ) + sqr( ptFrom.y - ptTarget.y ) > sqr( fPlaneDist ) )
		return false;
	if ( ptTarget.z < ptFrom.z - 0.5 || ptTarget.z > ptFrom.z + 2 )
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult HasWorkingWeapon( CUnitServer *pUS )
{
	NRPG::IInventoryItem *pItem = pUS->GetUnitRPG()->GetWeaponItem();
	CDynamicCast<NRPG::IWeaponItem> pW(pItem);
	if ( !IsValid( pW ) )
		return UCR_UNAVAILABLE;
	if ( !pW->HasAmmo() )
		return UCR_NEED_RELOAD;

	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	bool bRPGCanUse = pW->GetDBWeapon()->pWeaponType->bTwoHanded ? pRPG->CanUseTwoHanded() : true;
	if ( !bRPGCanUse )
		return UCR_GENERAL_FAILURE;

	if ( !pW->IsWorking() )
		return UCR_WEAPON_JAMMED;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool HasGrenade( CUnitServer *pUS )
{
	NRPG::IInventoryItem *pItem = pUS->GetUnitRPG()->GetInventory()->GetActive();
	if ( dynamic_cast<NRPG::IGrenadeItem*>( pItem ) )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool HasThrowingKnife( CUnitServer *pUS )
{
	NRPG::IInventoryItem *pItem = pUS->GetUnitRPG()->GetInventory()->GetActive();
	if ( CDynamicCast<NRPG::IMeleeWeaponItem> pMelee( pItem ) )
	{
		if ( pMelee->GetDBMeleeWeapon()->bThrowing )
			return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool HasBazookaAndRockets( CUnitServer *pUS )
{
	NRPG::IInventoryItem *pItem = pUS->GetUnitRPG()->GetInventory()->GetActive();
	if ( CDynamicCast<NRPG::IWeaponItem> pWeapon( pItem ) )
	{
		if ( pWeapon->GetDBWeapon()->bBazookaLogic && pWeapon->HasAmmo() )
			return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult HasActiveUsableFirstAid( CUnitServer *pUS )
{
	CDynamicCast<NRPG::CFirstAidItem> pFA( pUS->GetUnitRPG()->GetInventory()->GetActive() );
	if ( !IsValid( pFA ) )
		return UCR_UNAVAILABLE;
	if ( pFA->IsEmpty() )
		return UCR_NO_EQUIPMENT;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CanAttackWithCannon( CCannon *pCannon, const CVec3 &ptTarget )
{
	ASSERT(pCannon);
	if ( !pCannon )
		return UCR_UNAVAILABLE;

	CVec3 pos = pCannon->GetPosition();
	float fAngle = pCannon->GetDirection();
	CVec3 dir = ptTarget - pos;
	float fHAngle = SignumNormalizeAngleInRadian( atan2( dir.y, dir.x ) - fAngle );
	if ( fabs(fHAngle) > FP_PI8 )
		return UCR_TARGET_OUT_OF_RANGE;

	float fVAngleSin = dir.z / fabs(dir);
	if ( fabs(fVAngleSin) > sin(FP_PI8) )
		return UCR_TARGET_OUT_OF_RANGE;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CanDoFirstAid( CUnitServer *pUS, const NAI::SUnitPosition &from, CUnitServer *pTarget )
{
	EUnitCommandResult eResult = HasActiveUsableFirstAid( pUS );
	if ( eResult != UCR_OK )
		return eResult;

	if ( IsValid( pTarget ) )
	{
		if ( pTarget->IsDead() || pTarget->IsUnconscious() )
			return UCR_INVALID_COMMAND;

		CVec3 ptTarget = pTarget->GetPosition().GetCP();
		if ( !IsWithinHumanReach( from.GetCP(), ptTarget, F_HEAL_DISTANCE ) )
			return UCR_TARGET_OUT_OF_RANGE;

		if ( !pUS->GetUnitRPG()->CanHeal( pTarget->GetRPG() ) )
			return UCR_GENERAL_FAILURE;
	}

	if ( !IsValid( pTarget ) )
		return UCR_NO_TARGET;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool CanPickUpCorpse( CUnitServer *pUS, const NAI::SUnitPosition &from, CUnitServer *pTarget )
{
	CVec3 ptTarget = pTarget->GetPosition().GetCP();
	return fabs2( from.GetCP() - ptTarget ) < 4; // CRAP
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool CanDropCorpse( CUnitServer *pUS )
{
	const NAI::SUnitPosition &from = pUS->GetPosition();
	CVec3 center = from.GetCP();
	center.z += 0.9f;
	vector<SSphere> spheres;
	vector<CVec3> vels;
	vector<NAI::CCollider::SCollisionPoint> ress;
	float fDir = from.GetDirection();
	vels.push_back( CVec3( cos(fDir) * FP_GRID_STEP, sin(fDir) * FP_GRID_STEP, 0 ) );
	spheres.push_back( SSphere( center, 0.45f ) );
	NAI::CollideInfo( pUS->GetWorld()->GetAIMap(), spheres, vels, &ress, NWorld::TS_PASS_BLOCKER );
	return (ress[0].fDist == NAI::FP_NO_COLLISION);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool CheckParabolaIntersect( NAI::CCollider *pCollider, const float fTFly, const CVec3 &from, const CVec3 &to )
{
	CVec3 gravity(0,0,-F_GRAVITY);
	
	vector<bool> results;
	vector<SSphere> spheres;
	vector<CVec3> vels;	
	
	CVec3 startVel = (to - from) / fTFly - 0.5f * fTFly * gravity;
	CVec3 endVel = (to - from) / fTFly + 0.5f * fTFly * gravity;
	float fTStart = 0;//1.f / fabs(startVel);
	float fTEnd = fTFly - 1.0f / fabs(endVel);
	if ( fTEnd < 0.01f )
		return false;
	for ( float fT = fTStart; fT < fTEnd; fT += 0.1f )
	{
		SSphere s;
		s.ptCenter = from + startVel * fT + gravity * fT * fT * 0.5f;
		s.fRadius = F_GRENADE_SPHERE_RADIUS;
		spheres.push_back( s );
		//sphereParticles.push_back( spheres.back() );
	}
	if ( spheres.empty() )
		return false;
	for ( int i = 0; i < spheres.size() - 1; ++i )
		vels.push_back( spheres[i+1].ptCenter - spheres[i].ptCenter );
	vels.push_back( from + startVel * fTEnd + gravity * fTEnd * fTEnd * 0.5f - spheres.back().ptCenter );
	//sphereParticles.push_back( SSphere(from + startVel * fTEnd + gravity * fTEnd * fTEnd * 0.5f, F_GRENADE_SPHERE_RADIUS ) );
	
	pCollider->CollideCheck( spheres, vels, &results );
	for ( int i = 0; i < results.size(); ++i )
	{
		if ( results[i] )
			return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool FindGrenadeParams( CWorld *pWorld, const NAI::SUnitPosition &pos, const CVec3 &to, float fMaxVel, SGrenadeParams *pRes ) 
{
	CVec3 from = pos.GetCenter();
	
	vector<CVec3> boundPoints;
	boundPoints.push_back( from );
	boundPoints.push_back( to );
	
//	float fMaxVel = 14.0f; // CRAP, maximum velocity, should take it from RPG skills
	float a = sqr(F_GRAVITY) * 0.25f;
	float b = F_GRAVITY * (to.z - from.z) - sqr(fMaxVel);
	float c = fabs2(to - from);
	float d = sqr(b) - 4 * a * c;
	if ( d < 0 )
	{
		// cannot throw there - too far
		return false;
	}
	float fTFlyMin = 0.5f * (- b - sqrt(d)) / a;
	float fTFlyMax = 0.5f * (- b + sqrt(d)) / a;
	ASSERT( fTFlyMin > 0 );
	ASSERT( fTFlyMax > 0 );
	fTFlyMin = sqrt(fTFlyMin);
	fTFlyMax = sqrt(fTFlyMax);
	
	// upper point of all trajectories
	float fTUp = (to.z - from.z) / F_GRAVITY / fTFlyMax + fTFlyMax * 0.5f;
	if ( fTUp > 0 && fTUp < fTFlyMax )
		boundPoints.push_back( CVec3( to.x, to.y, from.z + F_GRAVITY * fTUp * fTUp * 0.5f ) );
	
	float fAngle = pos.GetDirection();
	float fX = F_GRENADE_CHECK_SIDE * cos(fAngle), fY = F_GRENADE_CHECK_SIDE * sin(fAngle);
	CVec3 rightSide(fY,-fX,0);
	CVec3 leftSide(-fY,fX,0);
	boundPoints.push_back( from + leftSide );
	boundPoints.push_back( from + rightSide );
	
	SBound bound;
	CalcBound( &bound, boundPoints, SGetSelf<CVec3>() );
	bound.Extend( Max( F_GRENADE_CHECK_RADIUS, F_GRENADE_SPHERE_RADIUS ) );
	CPtr<NAI::CCollider> pCollider = new NAI::CCollider;
	NAI::CCollider &collider = *pCollider;
	pWorld->GetAIMap()->PrepareCollider( &collider, bound, F_GRENADE_SPHERE_RADIUS, TS_PASS_BLOCKER );//TS_TERRAINS|TS_OBJECTS );
	
	vector<SSphere> spheres;
	vector<CVec3> vels;
	vector<bool> results;
	
	spheres.push_back( SSphere( from, F_GRENADE_CHECK_RADIUS ) );
	vels.push_back( rightSide );
	spheres.push_back( SSphere( from, F_GRENADE_CHECK_RADIUS ) );
	vels.push_back( leftSide );
	collider.CollideCheck( spheres, vels, &results );
	
	vector<CVec3> startPoints;
	startPoints.push_back( from );
	startPoints.push_back( from + rightSide ); // right side
	startPoints.push_back( from + leftSide ); // left side
	
	bool bFound = false;
	int nSide = 0;
	float fTFly;
	for ( fTFly = fTFlyMin; fTFly < fTFlyMax + F_GRENADE_DELAY_STEP; fTFly += F_GRENADE_DELAY_STEP )
	{
		int nSPSize = startPoints.size();
		for ( int k = 0; k < nSPSize; ++k )
		{
			if ( k > 0 && results[k - 1] )
				continue;
			from = startPoints[k];
			bFound = !CheckParabolaIntersect( &collider, Min(fTFly, fTFlyMax), from, to );
			if ( bFound ) 
			{
				nSide = k;
				break;
			}
		}
		if ( bFound )
			break;
	}
	if ( bFound )
	{
		SGrenadeParams &gp = *pRes;
		gp.ptOriginalTarget = to;
		gp.ptStart = from;
		gp.vel = (to - from) / fTFly - 0.5f * fTFly * CVec3(0,0,-F_GRAVITY);
		gp.fT = fTFly; 
		gp.nSide = nSide;
	}
	return bFound;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CanUnitThrowGrenade( CUnitServer *pUS, const NAI::SUnitPosition &from, const CVec3 &ptTarget, NRPG::IGrenadeItem *pGrenade )
{
	SGrenadeParams sParams;

	CPtr<NRPG::CGrenadeToHitCalcer> pToHitCalcer;
	int nDistance = fabs( ptTarget - from.GetCP() ) / FP_GRID_STEP;
	pToHitCalcer = new NRPG::CGrenadeToHitCalcer( pUS->GetUnitRPG(), from.GetPose(), 
		nDistance, from.GetCP(), pUS->GetWorld()->IsFirstTurn(), CVec3(1,1,1), ptTarget, pGrenade->GetDBGrenade() );
/*	float fGrenadeMaxDistance = pToHitCalcer->GetGrenadeMaxDistance();

	if ( nDistance > fGrenadeMaxDistance )
		return UCR_TARGET_OUT_OF_RANGE;*/
	if ( !FindGrenadeParams( pUS->GetWorld(), from, ptTarget, pToHitCalcer->GetMaxGrenadeVelocity(), &sParams ) )
		return UCR_TARGET_OUT_OF_RANGE;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CanUnitThrowKnife( CUnitServer *pUS, const NAI::SUnitPosition &from, const CVec3 &ptTarget, NRPG::IMeleeWeaponItem *pMelee )
{
	CPtr<NRPG::CThrowKnifeTileToHitCalcer> pToHitCalcer;
	int nDistance = fabs( ptTarget - from.GetCP() ) / FP_GRID_STEP;
	pToHitCalcer = new NRPG::CThrowKnifeTileToHitCalcer( pUS->GetUnitRPG(), from.GetPose(), 
		nDistance, from.GetCP(), pUS->GetWorld()->IsFirstTurn(), CVec3(1,1,1), NAI::THL_MIDDLE, ptTarget );

 	float fKnifeMaxDistance = pToHitCalcer->GetKnifeMaxDistance();

	if ( nDistance > fKnifeMaxDistance )
		return UCR_TARGET_OUT_OF_RANGE;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CanUnitLaunchRocket( CUnitServer *pUS, 
	const NAI::SUnitPosition &from, const CVec3 &ptTarget, int nExtraAP, NRPG::IWeaponItem *pBazooka )
{
	float fDistance = fabs( ptTarget - from.GetCP() );
	int nDistance =  fDistance / FP_GRID_STEP;
	CPtr<NRPG::CRLauncherToHitCalcer> pToHitCalcer = 
		new NRPG::CRLauncherToHitCalcer( pUS->GetUnitRPG(), from.GetPose(), nDistance, 
			from.GetCP(), nExtraAP, pUS->GetWorld()->IsFirstTurn(), CVec3(1,1,1), NAI::THL_MIDDLE, ptTarget );
	//
	if ( nDistance >= pToHitCalcer->GetMaxDistance() )
		return UCR_TARGET_OUT_OF_RANGE;
	//
	vector<NRPG::CAttackPortion> attack;
	pUS->GetUnitRPG()->CreateAttack( &attack, false );
	if ( attack.empty() )
		return UCR_GENERAL_FAILURE;
	//
	CObj<NRPG::CCoverInfo> pCover = pUS->GetWorld()->CalcCoversForTile( pUS->GetAttackOrigin( from ), 
		attack[0], pUS, ptTarget, pUS->GetMinClearDistance() );
	if ( !NRPG::CanShoot( pCover ) )
		return UCR_GENERAL_FAILURE;
	//
	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsPointInRect( int nX, int nY, const CTRect<int> &sRect )
{
	if ( ( nX >= sRect.x1 ) && ( nX <= sRect.x2 ) && ( nY >= sRect.y1 ) && ( nY <= sRect.y2 ) )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsRectsIntersect( const CTRect<int> &s1, const CTRect<int> &s2 )
{
	if ( IsPointInRect( s1.x1, s1.y1, s2 ) || IsPointInRect( s1.x2, s1.y1, s2 ) || IsPointInRect( s1.x2, s1.y2, s2 ) || IsPointInRect( s1.x1, s1.y2, s2 ) )
		return true;
	if ( IsPointInRect( s2.x1, s2.y1, s1 ) || IsPointInRect( s2.x2, s2.y1, s1 ) || IsPointInRect( s2.x2, s2.y2, s1 ) || IsPointInRect( s2.x1, s2.y2, s1 ) )
		return true;

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static EUnitCommandResult CanMoveInventoryItem( CUnitServer *pUS, CCmdMoveInventoryItem *pMoveItem )
{
	CObj<NRPG::IInventoryItem> pItem;

	const SItem &sSource = pMoveItem->GetSource();
	const SItem &sTarget = pMoveItem->GetTarget();

	switch( sSource.eType )
	{
	case SItem::HAND:
		{
			IPlayer::SItemInfo sItem;
			if ( !pUS->GetTBSPlayer()->GetInHandItem( &sItem ) )
				return UCR_INVALID_COMMAND;

			pItem = sItem.pItem;
			break;
		}
	case SItem::SLOT:
		{
			ASSERT( IsValid( sSource.pUnit ) );
			CDynamicCast<CUnitServer> pUSSource( sSource.pUnit );
			ASSERT( IsValid( pUSSource ) );
			ASSERT( !pUSSource->IsDead() );
			NRPG::IInventory *pInventory = pUSSource->GetUnitRPG()->GetInventory();

			pItem = pInventory->Get( (NDb::ESlot)sSource.nSlot );
			if ( !IsValid( pItem ) )
				return UCR_INVALID_COMMAND;

			break;
		}
	case SItem::BACKPACK:
		{
			ASSERT( IsValid( sSource.pUnit ) );
			CDynamicCast<CUnitServer> pUSSource( sSource.pUnit );
			ASSERT( IsValid( pUSSource ) );
			ASSERT( !pUSSource->IsDead() );
			pItem = sSource.pItem;
			break;
		}
	case SItem::GROUND:
		{
			if ( IsValid( sSource.pUnit ) )
			{
				CDynamicCast<CUnitServer> pUSSource( sSource.pUnit );
				ASSERT( IsValid( pUSSource ) );
				ASSERT( pUSSource->IsDead() );
				NRPG::IInventory *pDeadInventory = pUSSource->GetUnitRPG()->GetInventory();
				if ( sSource.nSlot >= 0 )
				{
					pItem = pDeadInventory->Get( (NDb::ESlot)sSource.nSlot );
					if ( !IsValid( pItem ) )
						return UCR_INVALID_COMMAND;
				}
				else
					pItem = sSource.pItem;
			}
			else
				pItem = sSource.pItem;

			break;
		}
	case SItem::UNIT_ANYPLACE:
		ASSERT( 0 );
		return UCR_INVALID_COMMAND;
	}

	ASSERT( IsValid( pItem ) );

	switch( sTarget.eType )
	{
	case SItem::HAND:
		{
			break;
		}
	case SItem::SLOT:
		{
			ASSERT( IsValid( sTarget.pUnit ) );
			CDynamicCast<CUnitServer> pUSTarget( sTarget.pUnit );
			if( !IsValid( pUSTarget ) || pUSTarget->IsDead() )
				return UCR_GENERAL_FAILURE;

			NRPG::IInventory *pInventory = pUSTarget->GetUnitRPG()->GetInventory();
			if ( !pInventory->CanEquip( (NDb::ESlot)sTarget.nSlot, pItem ) )
				return UCR_GENERAL_FAILURE;

			break;
		}
	case SItem::BACKPACK:
		{
			ASSERT( IsValid( sTarget.pUnit ) );
			CDynamicCast<CUnitServer> pUSTarget( sTarget.pUnit );
			if( !IsValid( pUSTarget ) || pUSTarget->IsDead() )
				return UCR_GENERAL_FAILURE;

			NRPG::IInventory *pInventory = pUSTarget->GetUnitRPG()->GetInventory();
			CTPoint<int> sTargetPosition = sTarget.sPosition;
			if ( ( sTargetPosition.x == -1 ) && ( sTargetPosition.y == -1 ) )
			{
				if ( !pInventory->FindPlace( pItem, &sTargetPosition ) )
					return UCR_INVENTORY_NO_PLACE;
			}
			const CTPoint<int> &sTargetSize = pItem->GetSize();
			CTRect<int> sTargetRect( sTargetPosition.x, sTargetPosition.y, sTargetPosition.x + sTargetSize.x - 1, sTargetPosition.y + sTargetSize.y - 1 );

			list<NRPG::SBackPackItem> intersectedItemslist;
			const vector<NRPG::SBackPackItem> &itemsSet = pInventory->GetItems();
			for ( int nTemp = 0; nTemp < itemsSet.size(); nTemp++ )
			{
				const CTPoint<int> &sSize = itemsSet[nTemp].pItem->GetSize();
				const CTPoint<int> &sPosition = itemsSet[nTemp].sPos;
				CTRect<int> sItemRect( sPosition.x, sPosition.y, sPosition.x + sSize.x - 1, sPosition.y + sSize.y - 1 );

				if ( IsRectsIntersect( sTargetRect, sItemRect ) )
					intersectedItemslist.push_back( itemsSet[nTemp] );
			}

			if ( intersectedItemslist.size() > 1 )
				return UCR_GENERAL_FAILURE;

			if ( !pInventory->CanPlace( sTargetPosition, pItem, false ) )
				return UCR_GENERAL_FAILURE;

			break;
		}
	case SItem::GROUND:
		{
			break;
		}
	case SItem::UNIT_ANYPLACE:
		{
			ASSERT( IsValid( sTarget.pUnit ) );
			CDynamicCast<CUnitServer> pUSTarget( sTarget.pUnit );
			if( !IsValid( pUSTarget ) || pUSTarget->IsDead() )
				return UCR_GENERAL_FAILURE;

			NRPG::IInventory *pInventory = pUSTarget->GetUnitRPG()->GetInventory();
			for ( int nTemp = 0; nTemp < NDb::N_SLOTS; nTemp++ )
			{
				if ( !IsValid( pInventory->Get( (NDb::ESlot)nTemp ) ) && pInventory->CanEquip( (NDb::ESlot)nTemp, pItem ) )
					return UCR_OK;
			}

			CTPoint<int> sTargetPosition;
			if ( pInventory->FindPlace( pItem, &sTargetPosition ) )
				return UCR_OK;

			return UCR_INVENTORY_NO_PLACE;
		}
	}

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void MoveInventoryItem( CUnitServer *pUS, CCmdMoveInventoryItem *pMoveItem )
{
	CObj<NRPG::IInventoryItem> pItem;

	SItem sSource = pMoveItem->GetSource();
	SItem sTarget = pMoveItem->GetTarget();

	switch( sSource.eType )
	{
	case SItem::HAND:
		{
			IPlayer::SItemInfo sItem;
			pUS->GetTBSPlayer()->GetInHandItem( &sItem );

			ASSERT( IsValid( sItem.pUnit ) );
			CDynamicCast<CUnitServer> pUSSource( sItem.pUnit );
			ASSERT( IsValid( pUSSource ) );
			ASSERT( !pUSSource->IsDead() );
			NRPG::IInventory *pInventory = pUSSource->GetUnitRPG()->GetInventory();
			pItem = pInventory->TakeOff( NDb::SLOT_HAND );
			break;
		}
	case SItem::SLOT:
		{
			ASSERT( IsValid( sSource.pUnit ) );
			CDynamicCast<CUnitServer> pUSSource( sSource.pUnit );
			ASSERT( IsValid( pUSSource ) );
			ASSERT( !pUSSource->IsDead() );
			NRPG::IInventory *pInventory = pUSSource->GetUnitRPG()->GetInventory();
			pItem = pInventory->TakeOff( (NDb::ESlot)sSource.nSlot );
			break;
		}
	case SItem::BACKPACK:
		{
			ASSERT( IsValid( sSource.pUnit ) );
			CDynamicCast<CUnitServer> pUSSource( sSource.pUnit );
			ASSERT( IsValid( pUSSource ) );
			ASSERT( !pUSSource->IsDead() );
			NRPG::IInventory *pInventory = pUSSource->GetUnitRPG()->GetInventory();
			pItem = sSource.pItem;
			pInventory->Take( sSource.pItem );
			break;
		}
	case SItem::GROUND:
		{
			if ( IsValid( sSource.pUnit ) )
			{
				CDynamicCast<CUnitServer> pUSSource( sSource.pUnit );
				ASSERT( IsValid( pUSSource ) );
				ASSERT( pUSSource->IsDead() );
				NRPG::IInventory *pDeadInventory = pUSSource->GetUnitRPG()->GetInventory();
				if ( sSource.nSlot >= 0 )
				{
					NDb::ESlot eSlot = (NDb::ESlot)sSource.nSlot;
					ASSERT( pDeadInventory->Get( eSlot ) == sSource.pItem );
					pItem = pDeadInventory->TakeOff( eSlot );
				}
				else
				{
					pItem = sSource.pItem;
					pDeadInventory->Take( sSource.pItem );
				}
				pUSSource->Update();
			}
			else
			{
				pItem = sSource.pItem;
				pUS->GetWorld()->RemoveFrozenItem( sSource.pItem );
			}

			sSource.pUnit = pUS;
			break;
		}
	case SItem::UNIT_ANYPLACE:
		ASSERT( 0 );
		return;
	}

	ASSERT( IsValid( pItem ) );

	switch( sTarget.eType )
	{
	case SItem::HAND:
		{
			ASSERT( IsValid( sTarget.pUnit ) );
			CDynamicCast<CUnitServer> pUSTarget( sTarget.pUnit );
			ASSERT( IsValid( pUSTarget ) );
			ASSERT( !pUSTarget->IsDead() );
			NRPG::IInventory *pInventory = pUSTarget->GetUnitRPG()->GetInventory();

			pInventory->Equip( NDb::SLOT_HAND, pItem );
			break;
		}
	case SItem::SLOT:
		{
			ASSERT( IsValid( sTarget.pUnit ) );
			CDynamicCast<CUnitServer> pUSTarget( sTarget.pUnit );
			ASSERT( IsValid( pUSTarget ) );
			ASSERT( !pUSTarget->IsDead() );
			NRPG::IInventory *pInventory = pUSTarget->GetUnitRPG()->GetInventory();

			CObj<NRPG::IInventoryItem> pOldItem = pInventory->Get( (NDb::ESlot)sTarget.nSlot );
			if ( IsValid( pOldItem ) )
			{
				pInventory->TakeOff( (NDb::ESlot)sTarget.nSlot );
				pInventory->Equip( NDb::SLOT_HAND, pOldItem );
			}

			pInventory->Equip( (NDb::ESlot)sTarget.nSlot, pItem );
			break;
		}
	case SItem::BACKPACK:
		{
			ASSERT( IsValid( sTarget.pUnit ) );
			CDynamicCast<CUnitServer> pUSTarget( sTarget.pUnit );
			ASSERT( IsValid( pUSTarget ) );
			ASSERT( !pUSTarget->IsDead() );
			NRPG::IInventory *pInventory = pUSTarget->GetUnitRPG()->GetInventory();

			CTPoint<int> sTargetPosition = sTarget.sPosition;
			if ( ( sTargetPosition.x == -1 ) && ( sTargetPosition.y == -1 ) )
				pInventory->FindPlace( pItem, &sTargetPosition );

			const CTPoint<int> &sTargetSize = pItem->GetSize();
			CTRect<int> sTargetRect( sTargetPosition.x, sTargetPosition.y, sTargetPosition.x + sTargetSize.x - 1, sTargetPosition.y + sTargetSize.y - 1 );

			list<NRPG::SBackPackItem> intersectedItemslist;
			const vector<NRPG::SBackPackItem> &itemsSet = pInventory->GetItems();
			for ( int nTemp = 0; nTemp < itemsSet.size(); nTemp++ )
			{
				const CTPoint<int> &sSize = itemsSet[nTemp].pItem->GetSize();
				const CTPoint<int> &sPosition = itemsSet[nTemp].sPos;
				CTRect<int> sItemRect( sPosition.x, sPosition.y, sPosition.x + sSize.x - 1, sPosition.y + sSize.y - 1 );

				if ( IsRectsIntersect( sTargetRect, sItemRect ) )
					intersectedItemslist.push_back( itemsSet[nTemp] );
			}

			if ( intersectedItemslist.size() > 1 )
				break;

			if ( !intersectedItemslist.empty() )
			{
				CObj<NRPG::IInventoryItem> pOldItem = intersectedItemslist.front().pItem;
				pInventory->Take( pOldItem );
				pInventory->Equip( NDb::SLOT_HAND, pOldItem );
			}

			pInventory->Place( sTargetPosition, pItem );
			break;
		}
	case SItem::GROUND:
		{
			CVec3 shift;
			shift.x = random.GetFloat( - FP_GRID_STEP * 0.5f, FP_GRID_STEP * 0.5f );
			shift.y = random.GetFloat( - FP_GRID_STEP * 0.5f, FP_GRID_STEP * 0.5f );
			shift.z = 0.1f;
			pUS->GetWorld()->AddFrozenItem( pUS->GetPosition().GetCP() + shift, QNULL, pItem );
			break;
		}
	case SItem::UNIT_ANYPLACE:
		{
			ASSERT( IsValid( sTarget.pUnit ) );
			CDynamicCast<CUnitServer> pUSTarget( sTarget.pUnit );
			ASSERT( IsValid( pUSTarget ) );
			ASSERT( !pUSTarget->IsDead() );
			NRPG::IInventory *pInventory = pUSTarget->GetUnitRPG()->GetInventory();

			int pValidSlots[] = { NDb::SLOT_1, NDb::SLOT_2 };

			bool bComplete = false;
			for ( int nTemp = 0; nTemp < ARRAY_SIZE( pValidSlots ); nTemp++ )
			{
				NDb::ESlot eSlot = (NDb::ESlot)pValidSlots[nTemp];
				if ( !IsValid( pInventory->Get( eSlot ) ) && pInventory->CanEquip( eSlot, pItem ) )
				{
					bComplete = true;
					pInventory->Equip( eSlot, pItem );
					break;
				}
			}

			if ( !bComplete )
			{
				CTPoint<int> sTargetPosition;
				if ( pInventory->FindPlace( pItem, &sTargetPosition ) )
				{
					bComplete = true;
					pInventory->Place( sTargetPosition, pItem );
				}
			}

			ASSERT( bComplete );
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecQueue
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::AddExecutor( CCommandExecute *pExec )
{
	ASSERT( pExec->GetUnitServer() == pUS );
	execList.push_back( pExec );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::CheckOpenCloseOnce()
{
	if ( execList.empty() )
		return;
	CCommandExecute *pExec = execList.back();
	if ( CDynamicCast<CExecOpenClose> pOpen( pExec ) )
	{
		bool bOpen; 
		IObject *pObj; 
		pOpen->GetParams( &bOpen, &pObj );
		list<CObj<CCommandExecute> >::iterator it;
		for ( it = execList.begin(); it != execList.end(); ++it )
		{
			CDynamicCast<CExecOpenClose> pOpenBefore( *it );
			if ( !pOpenBefore )
				continue;
			bool bOpenBefore;
			IObject *pObjectBefore; 
			pOpenBefore->GetParams( &bOpenBefore, &pObjectBefore );
			if ( bOpenBefore == bOpen && pObjectBefore == pObj )
			{
				++it;
				execList.erase( it, execList.end() );
				return;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecQueue::GetStartAP() const 
{ 
	if ( !execList.empty() )
		return execList.front()->GetStartAP();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecQueue::GetActionAP() const 
{ 
	int nTemp = 0;
	NAI::SUnitPosition pos = pUS->GetPosition(), dst = pos;
	for( list<CObj<CCommandExecute> >::const_iterator iTemp = execList.begin(); iTemp != execList.end(); iTemp++ )
	{
		nTemp += (*iTemp)->GetActionAP();
		CObj<NAI::CPath> pPath = (*iTemp)->GetCurrentPath();
		if ( pPath )
			dst.pos.p = pPath->points.back();
		pUS->SetTemporaryPosition( dst );
	}
	pUS->SetTemporaryPosition( pos );
	return nTemp;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::Run()
{
	while ( !execList.empty() )
	{
		execList.front()->Run();
		EFinishType f = execList.front()->GetState();
		if ( f == RUNNING )
			return;
		if ( f == FAILED )
		{
			Failed();
			execList.clear();
			return;
		}
		ASSERT( f == FINISHED );
		execList.pop_front();
		if ( !pUS->CanSpendAP( GetStartAP() ) )
			return;
	}
	Finished();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecQueue::TimeLabelReached() 
{ 
	if ( !execList.empty() )
		return execList.front()->TimeLabelReached();
	return false; 
}
void CExecQueue::AnimationFinished() 
{ 
	if ( !execList.empty() )
	{
		execList.front()->AnimationFinished();
		EFinishType f = execList.front()->GetState();
		if ( f != RUNNING )
		{
			if ( f == FAILED )
			{
				Failed();
				execList.clear();
				return;
			}
			ASSERT( f == FINISHED );
			execList.pop_front();
			if ( pUS->CanSpendAP( GetStartAP() ) )
				Run();
			return;
		}
		else
			return;
	}
	Finished();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::Cancel()
{
	if ( !execList.empty() )
	{
		CCommandExecute *pFront = execList.front();
		pFront->Cancel();
		if ( pFront->GetState() == FINISHED )
			Finished();
		else if ( pFront->GetState() == FAILED )
			Failed();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::FullCancel()
{
	if ( !execList.empty() )
	{
		CPtr<CCommandExecute> pFront = execList.front();
		if ( CDynamicCast<IExecMove> pMove( pFront ) )
			pMove->FullCancel();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecQueue::IsExecuting()
{ 
	if ( !execList.empty() )
		return execList.front()->IsExecuting();
	return true; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecQueue::IsWaitingForPath( NAI::SUnitPosition *p ) 
{ 
	if ( !execList.empty() )
		return execList.front()->IsWaitingForPath( p );
	return false; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NAI::CPath* CExecQueue::GetCurrentPath() const 
{ 
	if ( !execList.empty() )
	{
		NAI::CPath *pRes = 0;
		list<CObj<CCommandExecute> >::const_iterator it;
		for ( it = execList.begin(); it != execList.end(); ++it )
		{
			CObj<NAI::CPath> pCurrent = (*it)->GetCurrentPath();
			if ( !pCurrent )
				continue;
			if ( !pRes )
			{
				pRes = new NAI::CPath;
				pRes->pNet = pUS->GetWorld()->GetPathNetwork();
				pRes->points.push_back( pUS->GetUnitPosition().pos.p );
				pRes->bStrafePath = pCurrent->bStrafePath;
			}
			for ( int i = 0; i < pCurrent->points.size(); ++i )
			{
				if ( !( pCurrent->points[i] == pUS->GetUnitPosition().pos.p ) )
					pRes->points.push_back( pCurrent->points[i] );
			}
		}
		return pRes;
	}
	return 0; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::GetSearchFromPosition( NAI::SPathPlace *pRes )
{
	if ( !execList.empty() )
	{
		CCommandExecute *pFront = execList.front();
		if ( CDynamicCast<IExecMove> pMove( pFront ) )
		{
			pMove->GetSearchFromPosition( pRes );
			return;
		}
	}
	*pRes = pUS->GetPosition().pos.p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::GetDesiredPlace( NAI::SPathPlace *pRes, NAI::EFindPathParams *pParams )
{
	// get last move point
	*pRes = pUS->GetPosition().pos.p;
	*pParams = NAI::PF_DEFAULT;
	list<CObj<CCommandExecute> >::iterator i;
	for ( i = execList.begin(); i != execList.end(); ++i )
	{
		if ( CDynamicCast<IExecMove> pMove( *i ) )
			pMove->GetDesiredPlace( pRes, pParams );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::GetPathPoints( list<SPathPoint> *pRes )
{
	for ( list<CObj<CCommandExecute> >::iterator i = execList.begin(); i != execList.end(); ++i )
	{
		if ( CDynamicCast<IExecMove> pMove( *i ) )
			pMove->GetPathPoints( pRes );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::SetNewPath( NAI::CPath *pPath, NAI::EFindPathParams _eParams, ENeedActiveItem eActive )
{
	CObj<CCommandExecute> pHold = execList.front(); 
	CDynamicCast<IExecMove> pOldCommand( pHold );
	execList.pop_front();
	list<CObj<CCommandExecute> > oldExecList = execList;
	execList.clear();
	AddPath( pPath, _eParams, eActive, pOldCommand );
	list<CObj<CCommandExecute> >::iterator i, firstToAdd = oldExecList.end(); 
	NAI::SPathPlace p;
	NAI::EFindPathParams params;
	for ( i = oldExecList.begin(); i != oldExecList.end(); ++i )
	{
		if ( CDynamicCast<IExecMove> pMove( *i ) )
		{
			pMove->GetDesiredPlace( &p, &params );
			if ( p == pPath->points.back() && _eParams == params )
			{
				firstToAdd = i;
				++firstToAdd;
				break;
			}
		}
	}
	for ( i = firstToAdd; i != oldExecList.end(); ++i )
		execList.push_back( *i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecQueue::AddPath( NAI::CPath *pPath, NAI::EFindPathParams _eParams, ENeedActiveItem eActive, IExecMove *pOldFront )
{
	// разбираем путь
	CPtr<NAI::CPath> pSimplePath = new NAI::CPath;
	pSimplePath->pNet = pPath->pNet;
	pSimplePath->bStrafePath = pPath->bStrafePath;
	int nWaitAction = 0;
	bool bFirstPath = true;
	for ( int i = 0; i < pPath->points.size(); ++i )
	{
		NAI::SPathPlace p( pPath->points[i] );
		pSimplePath->points.push_back( p );
		if ( nWaitAction < pPath->actions.size() )
		{
			NAI::CPath::SPathAction &action = pPath->actions[nWaitAction];
			bool bActionNeeded = false;
			if ( !pPath->bStrafePath )
				bActionNeeded = action.where == p;
			else
				bActionNeeded = action.where.GetX() == p.GetX() && action.where.GetY() == p.GetY() 
				&& action.where.GetLayer() == p.GetLayer();
			if ( !bActionNeeded )
				continue;
			// prepare final rotation
			CDynamicCast<IObject> pObj( action.pObject );
			if ( !pObj )
			{
				ASSERT(0);
				return;
			}
			bool bOpen = ( action.action == NAI::PA_OPEN );
			CPtr<CCmdOpenClose> pOpen = new CCmdOpenClose( pObj, bOpen );
			
			//  Добавляем дополнительную точку поворота, чтобы персонаж открывал дверь с правильной стороны
			vector<NAI::SPathPlace> res;
			CDynamicCast<IGetApproaches> pAppr( pOpen->pObject );
			CWorld *pWorld = pUS->GetWorld();
			pAppr->GetApproaches( &res, pWorld->GetPathNetwork() );
			int nDir = p.GetDirection(), nBestDir, nBestDiff = 7;
			for ( int nRes = 0; nRes < res.size(); ++nRes )
			{
				int nNewDir = pWorld->GetPathNetwork()->GetClosestDir( p, res[ nRes ] );
				int nDiff = abs( nNewDir - nDir ); 
				if ( nDiff > 4 )
					nDiff = 8 - nDiff;
				if ( nDiff < nBestDiff )
				{
					nBestDiff =  nDiff;
					nBestDir = nNewDir;
				}
			}
			if ( nBestDir != nDir )
			{
				NAI::SPathPlace newPoint( p );
				newPoint.SetDirection( nBestDir );
				pSimplePath->points.push_back( newPoint );
				p = newPoint;
			}

			// move
			if ( !bFirstPath || !pOldFront )
			{
				CCommandExecute *pMove = CreateSimpleMoveExecutor( pUS, pSimplePath, _eParams, ITEM_INACTIVE );
				if ( pMove ) 
					execList.push_back( pMove );
				else
				{
					ASSERT(0);
					return;
				}
			}
			else
			{
				pOldFront->SetNewPath( pSimplePath, _eParams, ITEM_INACTIVE );
				CDynamicCast<CCommandExecute> pPush( pOldFront );
				execList.push_back( pPush.GetPtr() );
			}
			bFirstPath = false;
			// open - close door
			CCommandExecute *pOpenClose = new CExecOpenClose( pUS, pOpen );
			execList.push_back( pOpenClose );
			// again
			pSimplePath = new NAI::CPath;
			pSimplePath->pNet = pPath->pNet;
			pSimplePath->bStrafePath = pPath->bStrafePath;
			pSimplePath->points.push_back( p );
			++nWaitAction;
		}
	}
	// last move
	if ( !bFirstPath || !pOldFront )
	{
		CCommandExecute *pMove = CreateSimpleMoveExecutor( pUS, pSimplePath, _eParams, eActive );
		if ( pMove ) 
			execList.push_back( pMove );
		else
			ASSERT(0);
	}
	else
	{
		pOldFront->SetNewPath( pSimplePath, _eParams, eActive );
		CDynamicCast<CCommandExecute> pPush( pOldFront );
		execList.push_back( pPush.GetPtr() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecAttack
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecAttack::CExecAttack( CUnitServer *_pUS ):
	CCommandExecute(_pUS), bAttackCanceled( false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecAttack::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );
	ASSERT( pUS->CanSpendAP( GetStartAP() ) );
	//
	bAttackCanceled = false;
	pUS->GetUnitRPG()->StartAttack();
	Start();
	StartAction( pUS->GetWorld(), NORMAL );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecAttack::TimeLabelReached() // если false, то конец
{
	pUS->GetUnitRPG()->NextBullet();
	bool bRes = OnLabel();						// если вначале поставить NextBullet(), то звук от очереди играться не будет
	if ( !bRes && !bAttackCanceled )			// Нефига исправлять свои баги за счёт чужих! Вертаю всё в зад!
		pUS->GetUnitRPG()->StartAttack(); // reset burts state
	return bRes && !bAttackCanceled;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecAttack::Cancel()
{
	bAttackCanceled = true;
	Finished();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecAttack::CreateAttack( vector<NRPG::CAttackPortion> *pAttack, CUnitServer *pUnitTarget, bool bSpendAmmo ) const
{
	return pUS->GetUnitRPG()->CreateAttack( pAttack, bSpendAmmo, pUS->GetUnitRPG(), pUnitTarget ? pUnitTarget->GetUnitRPG() : 0 );;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecShoot
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecShoot::CExecShoot( CUnitServer *_pUS, int _nExtraAP ): 
	CExecAttack(_pUS), nExtraAP(_nExtraAP)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecShoot::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( GetActionType( pUS ) == AT_CANNON )
	{
		if ( bIgnoreTarget )
			return UCR_OK;

		return CanAttackWithCannon( pUS->animator.GetCannon(), ptAnimTarget );
	}

	return HasWorkingWeapon( pUS );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShoot::PerformShot()
{
	if ( !Attack.empty() )
	{
		Attack.front().rTtrajectory = ray;
		PerformAttack( Attack, ray, fRayToHit );
		pUS->CreateFlash();
		Attack.clear();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecShoot::OnLabel()
{
	bool bBurstComplete = bComplete;
	const NAI::SUnitPosition &position = pUS->GetPosition();

	if ( IsAttackCanceled() )
		bAttackCanceled = true;
	else
	{
		PerformShot();
		if ( !bComplete )
			PrepareShot();
	}

	if ( bBurstComplete || bAttackCanceled )
	{
		pUS->DoAction( NRPG::AC_END_SHOOT );
		CheckShotResult();
	}
	else
		pUS->animator.Attack( position, ray, true );

	return !bBurstComplete || bAttackCanceled;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShoot::PerformAttack( const vector<NRPG::CAttackPortion> &attack, const CRay &ray, float fHit )
{
	vector<NRPG::IAttackable*> ignores;
	CCannon *pCannon = pUS->animator.GetCannon();
	if ( pCannon )
		ignores.push_back( pCannon );
	else
		ignores.push_back( pUS );

	float fTrailSpeed = 0;
	CPtr<NDb::CModel> pTrailEffect = 0;
	NRPG::IWeaponItem *pWeapon = pUS->GetUnitRPG()->GetWeaponItem();
	if ( pWeapon && pWeapon->GetDBWeapon()->pTrailEffect )
	{
		SRand sRand;
		fTrailSpeed = pWeapon->GetDBWeapon()->fTrailSpeed;
		pTrailEffect = pWeapon->GetDBWeapon()->pTrailEffect->CreateModel( &sRand );
	}

	CRay rTempRay( ray ); // CRAP
	rTempRay.ptOrigin += ray.ptDir * pUS->GetMinClearDistance();
	for ( vector<NRPG::CAttackPortion>::const_iterator i = attack.begin(); i != attack.end(); ++i )
		pUS->GetWorld()->PerformRangedAttack( *i, rTempRay, ignores, pUS->GetWorld()->GetTime()->GetValue(), pTrailEffect, fTrailSpeed );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecShoot::CheckBurst( bool bComplete )
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	NRPG::CWeaponItem *pWeapon = pRPG->GetRPGUnit()->GetWeaponItem();
	bool bBurstLastBullet = pRPG->GetNBullets() && pRPG->GetNBullets() >= ( pWeapon->GetDBWeapon()->nRoF / 6 );
	//
	if ( !bComplete || bBurstLastBullet )
	{
		if ( CanDoIt( pUS->GetPosition() ) != UCR_OK )
			return true;
		if ( pUS->GetUnitRPG()->GetNBullets() > 1 )
		{
			if ( !pUS->CanSpendAP( pUS->GetActionAP( NRPG::AC_BURST ) ) )
				return true;
			pUS->DoAction( NRPG::AC_BURST );
		}
	}
	return bComplete;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecShoot::GetActionAP() const 
{ 
	if ( IsAccidental() )
		return 0;
	NRPG::CWeaponItem *pWeapon = pUS->GetUnitRPG()->GetRPGUnit()->GetWeaponItem();
	if ( IsValid( pWeapon ) )
	{
		NDb::EShootMode ShootMode = pWeapon->GetShootMode();
		if ( ShootMode == NDb::SM_Careful || ShootMode == NDb::SM_LongBurst )
		{
			NRPG::SUnitInfo Info;
			pUS->GetInfo( &Info );
			return max( Info.nAP,  GetStartAP() );
		} 
	}

	return GetStartAP(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShoot::Scream()
{
	CPtr<NRPG::CWeaponItem> pWeapon = pUS->GetUnitRPG()->GetRPGUnit()->GetWeaponItem();
	if ( pUS->GetPosition().GetPose() != NAI::CRAWL && IsValid( pWeapon ) && pWeapon->GetShootMode() == NDb::SM_LongBurst )
	{
		if ( IsValid( pWeapon->GetDBWeapon() ) && 
			pWeapon->GetDBWeapon()->pAnimWeaponType->type == NDb::WT_MACHINE_GUN )
				pUS->GetWorld()->MakeSound( pUS->GetPosition().GetCP(), NDb::GetSound( 4060 ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShoot::Start()
{
	bComplete = true;
	bMissed = true;
	nToHit = 0;
	//
	csRPG << CC_WHITE << "Shoot begin :\n";
	CalculateExtraAP();
	PrepareShot();
	SpendAP();
	if ( !IsAccidental() )
	{
		Scream();
		pUS->animator.Attack( pUS->GetPosition(), ray );
	}
	else
		OnLabel();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecShoot::GetStartAP() const
{
	if ( IsAccidental() )
		return 0;
	if ( pUS->animator.IsAiming() )
		return pUS->GetActionAP( NRPG::AC_SHOOT ) + nExtraAP;
	else
		return pUS->GetActionAP( NRPG::AC_PREPARE_AND_SHOOT ) + nExtraAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShoot::CalculateExtraAP()
{
	nExtraAP = 0;
	NRPG::CWeaponItem *pWeapon = pUS->GetUnitRPG()->GetRPGUnit()->GetWeaponItem();
	if ( IsValid( pWeapon ) && pWeapon->GetShootMode() == NDb::SM_Careful )
		nExtraAP = pUS->GetCarefulShotExtraAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShoot::SpendAP()
{
	if ( IsAccidental() )
		return;
	if ( pUS->animator.IsAiming() )
		pUS->DoAction( NRPG::AC_SHOOT );
	else
		pUS->DoAction( NRPG::AC_PREPARE_AND_SHOOT );
	pUS->SpendAP( nExtraAP );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecShootTile
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecShootTile::CExecShootTile( CUnitServer *_pUS, const CVec3 &_ptTarget ): 
	CExecShoot(_pUS, 0), eHL( NAI::THL_LOWER )
{
	ptAnimTarget = _ptTarget;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecShootTile::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	vector<NRPG::CAttackPortion> attack;
	CreateAttack( &attack, 0, false );
	if ( attack.empty() )
		return UCR_GENERAL_FAILURE;

	CWorld *pWorld = pUS->GetWorld();
	CObj<NRPG::CCoverInfo> pCover = pWorld->CalcCoversForTile( pUS->GetAttackOrigin( from ), attack[0], pUS, ptAnimTarget, pUS->GetMinClearDistance() );
	if ( !NRPG::CanShoot( pCover ) )
		return UCR_GENERAL_FAILURE;

	return CExecShoot::CanDoIt( from );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShootTile::PrepareShot() // false, когда последний выстрел
{
	bComplete = CreateAttack( &Attack, 0, true );
	bComplete = CheckBurst( bComplete );
	//
	if ( !Attack.empty() )
	{
		int nTmpToHit;
		bool bTmpMissed;
		CPtr<CWorld> pWorld = pUS->GetWorld();
		CObj<NRPG::CCoverInfo> pCover = pWorld->CalcCoversForTile( pUS->GetAttackOrigin( pUS->GetPosition() ), 
			Attack[0], pUS, ptAnimTarget, pUS->GetMinClearDistance() );
		//fRayToHit = 
		NRPG::CheckTileToHit( pUS, ptAnimTarget, GetExtraAP(), eHL, pCover, pWorld->IsFirstTurn(), &nTmpToHit );
		NRPG::PeekRay( pCover, &ray, 2.f, &bTmpMissed );
		bMissed &= bTmpMissed;
		nToHit = max( nToHit, nTmpToHit );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecShootUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecShootUnit::CExecShootUnit( CUnitServer *_pUS, NWorld::CUnitServer *_pTarget, NAI::EHitLocation _eHL, int _nExtraAttackAP ):
		CExecShoot( _pUS, _nExtraAttackAP ), pTarget(_pTarget), eHL(_eHL)
{ 
	if ( IsValid( pTarget ) )
	{
		if ( eHL == NAI::HL_ANY )
			pUS->GetWorld()->GetAIMap()->GetUnitHLPos( &ptAnimTarget, pTarget, NAI::HL_BODY );
		else
			pUS->GetWorld()->GetAIMap()->GetUnitHLPos( &ptAnimTarget, pTarget, eHL );
	}
	else
	{
		ptAnimTarget = pUS->GetPosition().GetCP();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecShootUnit::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( !IsValid( pTarget ) || bIgnoreTarget )
		return CExecShoot::CanDoIt( from, true );

	vector<NRPG::CAttackPortion> attack;
	CreateAttack( &attack, pTarget, false );
	if ( attack.empty() )
		return UCR_GENERAL_FAILURE;

	CWorld *pWorld = pUS->GetWorld();
	CObj<NRPG::CCoverInfo> pCover = pWorld->CalcCovers( pUS->GetAttackOrigin( from ), attack[0], pUS, pTarget, eHL, pUS->GetMinClearDistance() );
	if ( !NRPG::CanShoot( pCover ) )
		return UCR_GENERAL_FAILURE;

	return CExecShoot::CanDoIt( from );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShootUnit::PrepareShot() // false, когда последний выстрел
{
	bComplete = CreateAttack( &Attack, pTarget );
	bComplete = CheckBurst( bComplete );
	//
	if ( !Attack.empty() )
	{
		int nTmpToHit;
		bool bTmpMissed;
		CPtr<CWorld> pWorld = pUS->GetWorld();
		vector<int> accessibleHLs; // необходимо только для вычисления Melee ToHit
		CObj<NRPG::CCoverInfo> pCover = pWorld->CalcCovers( pUS->GetAttackOrigin(), Attack[0], pUS, pTarget, eHL, pUS->GetMinClearDistance() );
		fRayToHit = NRPG::CheckToHit( pUS, pTarget, GetExtraAP(), eHL, accessibleHLs, pCover, pWorld->IsFirstTurn(), &nTmpToHit );
		NRPG::PeekRay( pCover, &ray, fRayToHit, &bTmpMissed );
		bMissed &= bTmpMissed;
		nToHit = max( nToHit, nTmpToHit );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecShootUnit::CheckShotResult()
{
	csRPG << CC_WHITE << "}\n";
	CWorld *pWorld = pTarget->GetWorld();
	//
	if ( !bMissed )
	{
		if ( nToHit > 30 )
			pWorld->GetGlobalAck()->OnTargetHit( pUS );
		else
			pWorld->GetGlobalAck()->OnHardTargetHit( pUS );
	}
	else 
	{
		if ( nToHit > 60 )
			pWorld->GetGlobalAck()->OnTargetMissed( pUS );
		// добавляем interrupt
		CDynamicCast<CUnitServer> pUnit( pTarget );
		if ( IsValid(pUnit) )
		{
			SInterruptInfo info;
			info.AddEvent( pTarget, pUS, true );
			pWorld->CheckInterrupt( &info );
		}
	}
	//
	pUS->CancelSnipe();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecShootUnit::IsAttackCanceled()
{
	NRPG::IWeaponItem *pWeapon = pUS->GetUnitRPG()->GetWeaponItem();
	if ( IsValid(pWeapon) && pWeapon->GetShootMode() == NDb::SM_LongBurst && pTarget->IsDead() && random.Check(pUS->GetUnitRPG()->GetSkillValue(NDb::ST_BURST)) )
		return true;
	else
		return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecMelee
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecMelee::CExecMelee( CUnitServer *_pUS, int _nExtraAP ): 
	CExecAttack(_pUS), nExtraAP(_nExtraAP)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecMelee::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( bIgnoreTarget )
		return UCR_NO_TARGET;

	if ( !IsWithinHumanReach( from.GetCP(), ptTarget, F_MELEE_DISTANCE ) )
		return UCR_TARGET_OUT_OF_RANGE;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecMelee::GetStartAP() const
{
	return pUS->GetActionAP( NRPG::AC_MELEE ) + nExtraAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecMelee::GetActionAP() const
{
	return GetStartAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecMelee::Start()
{
	const NAI::SUnitPosition &position = pUS->GetPosition();
	pUS->DoAction( NRPG::AC_MELEE );
	pUS->SpendAP( nExtraAP );
	pUS->animator.CloseAttack( position, NAI::GetBlowHeight( position, ptTarget ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecMelee::PerformAttack( const vector<NRPG::CAttackPortion> &attack, const CRay &ray, float fHit )
{
	vector<NRPG::IAttackable*> ignores;
	CCannon *pCannon = pUS->animator.GetCannon();
	if ( pCannon )
		ignores.push_back( pCannon );
	else
		ignores.push_back( pUS );

	for ( vector<NRPG::CAttackPortion>::const_iterator i = attack.begin(); i != attack.end(); ++i )
	{
		pUS->GetWorld()->PerformMeleeAttack( *i, ray, ignores );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecMeleeTile
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecMeleeTile::CExecMeleeTile( CUnitServer *_pUS, const CVec3 &_ptTarget ):
	CExecMelee( _pUS, 0 )
{
	ptTarget = _ptTarget;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecMeleeTile::OnLabel()
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	CWorld *pWorld = pUS->GetWorld();
	const NAI::SUnitPosition &position = pUS->GetPosition();
	bool bComplete = true;

	CRay ray;
	ray.ptOrigin = position.GetEyePosition();
	ray.ptDir = ptTarget - ray.ptOrigin;
	Normalize( &ray.ptDir );
	vector<NRPG::CAttackPortion> attack;
	bComplete = CreateAttack( &attack, 0 );
	ASSERT( bComplete );
	PerformAttack( attack, ray );
	return !bComplete;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecMeleeUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecMeleeUnit::CExecMeleeUnit( CUnitServer *_pUS, CUnitServer *_pTarget, NAI::EHitLocation _eHL, int _nExtraAttackAP ):
	CExecMelee(_pUS, _nExtraAttackAP), pTarget(_pTarget), eHL(_eHL) 
{ 
	bIsHitLocationShot = (eHL != NAI::HL_ANY);

	if ( IsValid( pTarget ) )
		ptTarget = pTarget->GetPosition().GetCP();
	else
		ptTarget = _pUS->GetPosition().GetCP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecMeleeUnit::Start()
{
	CWorld *pWorld = pUS->GetWorld();
	if ( NAI::HL_ANY == eHL )
	{
		// выбираем куда будем бить
		vector<int> hls;
		pWorld->GetAIMap()->GetAccessibleUnitHL( &hls, pUS->GetPosition().GetCenter(), pTarget, F_MELEE_DISTANCE );
		if ( !hls.empty() )
		{
			static SRand rnd;
			eHL = (NAI::EHitLocation)hls[rnd.Get( hls.size() )];
		}
		else
			ASSERT( 0 );
	}
	pWorld->GetAIMap()->GetUnitHLPos( &ptTarget, pTarget, eHL );
	CExecMelee::Start();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecMeleeUnit::OnLabel()
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	CWorld *pWorld = pUS->GetWorld();
	const NAI::SUnitPosition &position = pUS->GetPosition();
	bool bComplete = true;
	
	pUS->GetUnitRPG()->NextBullet();
	csRPG << CC_WHITE << "Melee " << pRPG->GetName() << "\n{\n";
	vector<NRPG::CAttackPortion> attack;
	bComplete = CreateAttack( &attack, pTarget );
	ASSERT( bComplete );
	if ( !attack.empty() )
	{
		vector<int> accessibleHLs; // необходимо только для вычисления Melee ToHit
		//
		if ( bIsHitLocationShot )//-1 == eHL )//pCmd->nSpentAP )
			accessibleHLs.push_back( eHL );
		else
			pWorld->GetAIMap()->GetAccessibleUnitHL( &accessibleHLs, position.GetCenter(), pTarget.GetPtr(), F_MELEE_DISTANCE );

		CVec3 ptAttackPos = NRPG::GetMeleeAttackPos( pUS, ptTarget );
		CObj<NRPG::CCoverInfo> pCover = pWorld->CalcCovers( ptAttackPos, attack[0], pUS, pTarget, eHL, pUS->GetMinClearDistance() );
		//
		int nToHit;
		float fHit = NRPG::CheckToHit( pUS, pTarget, 
			GetExtraAP(), eHL, accessibleHLs, pCover, pWorld->IsFirstTurn(), &nToHit );
		CRay ray;
		bool bIsMiss;
		if ( NRPG::PeekRay( pCover, &ray, fHit, &bIsMiss ) )
			PerformAttack( attack, ray, fHit );
			//PerformAttack( attack, ray, bIsMiss, fHit );
	}
	csRPG << "}\n";
	return !bComplete;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecThrowGrenade
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecThrowGrenade::CExecThrowGrenade( CUnitServer *_pUS, const CVec3 &_ptTarget ): 
	CCommandExecute(_pUS), ptTarget(_ptTarget)
{
	CDynamicCast<NRPG::IGrenadeItem> pGrenade( pUS->GetUnitRPG()->GetInventory()->GetActive() );
	ASSERT( IsValid(pGrenade) );
	int nDistance = fabs( ptTarget - pUS->GetPosition().GetCP() ) / FP_GRID_STEP;
	pToHitCalcer = new NRPG::CGrenadeToHitCalcer( pUS->GetUnitRPG(), pUS->GetPosition().GetPose(), 
		nDistance, pUS->GetPosition().GetCP(), pUS->GetWorld()->IsFirstTurn(), CVec3(1,1,1), ptTarget, pGrenade->GetDBGrenade() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecThrowGrenade::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( !HasGrenade( pUS ) )
		return UCR_NO_EQUIPMENT;

	if ( bIgnoreTarget )
		return UCR_NO_TARGET;

	CDynamicCast<NRPG::IGrenadeItem> pGrenade( pUS->GetUnitRPG()->GetInventory()->GetActive() );
	return CanUnitThrowGrenade( pUS, from, ptTarget, pGrenade );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecThrowGrenade::GetStartAP() const
{
	return pUS->GetActionAP( NRPG::AC_THROW_GRENADE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecThrowGrenade::GetActionAP() const
{
	return GetStartAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecThrowGrenade::CheckToHitAndDelay( NDb::CRPGGrenade *pGrenade )
{
	// проверяем ToHit и находим точку в которую действительно кидаем гранату
	int nToHit = pToHitCalcer->GetToHit();
	pToHitCalcer->Log();

	// находим действительную задержку взрыва гранаты
	int nRandom = random.Get(100);
	if ( nRandom > nToHit )
	{
		// промахиваемся
			// ищем куда действительно полетит граната
		float fD = 0.2f * fabs( grenadeParams.vel );
		grenadeParams.vel.x += random.GetFloat( -fD, +fD );
		grenadeParams.vel.y += random.GetFloat( -fD, +fD );
		grenadeParams.vel.z += random.GetFloat( -fD, +fD );
	}
	csRPG << "<font size=16pt>";
	csRPG << CC_ORANGE << " \tCheck:" << nRandom;

	// Преобразовываем время полёта в задержку
	grenadeParams.fT = Clamp( grenadeParams.fT, 0.f, float(pGrenade->nMaxDelay) );
	grenadeParams.fT = pGrenade->nMaxDelay - grenadeParams.fT;
	csRPG << " True delay: " << grenadeParams.fT;
	nRandom = random.Get(100);
	if ( nRandom >= 99 || nRandom >= pToHitCalcer->GetSkill() ) 
	{
		// меняем время задержки
		grenadeParams.fT *= random.GetFloat( 0.5f, 2.f );
		grenadeParams.fT = Clamp( grenadeParams.fT, 0.f, float(pGrenade->nMaxDelay) * 0.75f );
	}
	csRPG << " Used delay: " << grenadeParams.fT << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecThrowGrenade::ThrowGrenade()
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	NRPG::IInventory *pInventory = pRPG->GetInventory();
	CUnitServer::SResItem item;
	CDynamicCast<NRPG::IGrenadeItem> pGrenade( pInventory->GetActive() );
	if ( !IsValid( pGrenade ) )
		return;

	FindGrenadeParams( pUS->GetWorld(), pUS->GetPosition(), ptTarget, pToHitCalcer->GetMaxGrenadeVelocity(), &grenadeParams );
	CheckToHitAndDelay( pGrenade->GetDBGrenade() );

	pUS->TearOffItem( &item, (NDb::ESlot)pInventory->GetActiveSlot(), true );
	pUS->GetWorld()->ThrowGrenade( grenadeParams.ptStart, grenadeParams.vel, 
		pUS->animator.GetTimeLabel1(), grenadeParams.fT, item.pModel, 
		pGrenade->GetDBGrenade(), pUS ); 
	pUS->Update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecThrowGrenade::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );
	ASSERT( pUS->CanSpendAP( GetStartAP() ) );

	if ( !FindGrenadeParams( pUS->GetWorld(), pUS->GetPosition(), ptTarget, pToHitCalcer->GetMaxGrenadeVelocity(), &grenadeParams ) )
	{
		Failed();
		return;
	}
	CWorld *pWorld = pUS->GetWorld();
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	const NAI::SUnitPosition position = pUS->GetPosition();
	pRPG->StartAttack();
	//
	pUS->DoAction( NRPG::AC_THROW_GRENADE );
	pUS->animator.ThrowGrenade( position, grenadeParams.ptOriginalTarget, grenadeParams.nSide );
	StartAction( pUS->GetWorld(), NORMAL );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecThrowGrenade::TimeLabelReached()
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	pRPG->NextBullet();
	ThrowGrenade();
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecLaunchRocket
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecLaunchRocket::CExecLaunchRocket( CUnitServer *_pUS, const CVec3 &_ptTarget ):
	CExecShoot(_pUS, 0), bAccidental( false )
{
	ptAnimTarget = _ptTarget;
	if ( ptAnimTarget == VNULL3 )
		bAccidental = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecLaunchRocket::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( !HasBazookaAndRockets( pUS ) )
		return UCR_NO_EQUIPMENT;

	if ( IsAccidental() )
		return UCR_OK;

	if ( bIgnoreTarget )
		return UCR_NO_TARGET;

	CDynamicCast<NRPG::IWeaponItem> pWeapon( pUS->GetUnitRPG()->GetInventory()->GetActive() );
	return CanUnitLaunchRocket( pUS, from, ptAnimTarget, nExtraAP, pWeapon );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecLaunchRocket::Start()
{
	if ( IsAccidental() )
		pUS->GetBarrelDir( &ray );
	else
	{
		ray.ptOrigin = pUS->GetAttackOrigin();
		ray.ptDir = ptAnimTarget - ray.ptOrigin;
		Normalize( &ray.ptDir );
	}
	//
	if ( CanDoIt( pUS->GetPosition() ) == UCR_OK )
		CExecShoot::Start();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecLaunchRocket::LaunchRocket( )
{
	vector<NRPG::CAttackPortion> attack;
	pUS->GetUnitRPG()->CreateAttack( &attack, true );
	if ( attack.empty() )
		return;
	//
	NRPG::IWeaponItem *pWeapon = pUS->GetUnitRPG()->GetWeaponItem();
	NRPG::IClipItem *pRocket = pUS->GetUnitRPG()->GetWeaponItem()->GetInnerClip();
	NDb::CRPGItem *pRPGRocket = pRocket->GetDBItem();
	float fSpeed = pWeapon->GetDBWeapon()->fTrailSpeed;
	SRand rnd;
	CPtr<NDb::CModel> pModel = pWeapon->GetDBWeapon()->pTrailEffect->CreateModel( &rnd );
	CPtr<NDb::CEffect> pEffect;
	if ( pWeapon->GetDBWeapon()->pTrailParticle )
		pEffect = pWeapon->GetDBWeapon()->pTrailParticle->GetEffect( &rnd );
	CPtr<NRPG::CRLauncherToHitCalcer> pToHitCalcer;
	int nDistance = fabs( ptAnimTarget - pUS->GetPosition().GetCP() ) / FP_GRID_STEP;
	pToHitCalcer = new NRPG::CRLauncherToHitCalcer( pUS->GetUnitRPG(), 
		pUS->GetPosition().GetPose(), nDistance, pUS->GetPosition().GetCP(), nExtraAP, 
		pUS->GetWorld()->IsFirstTurn(), CVec3(1,1,1), NAI::THL_MIDDLE, ptAnimTarget );
	//
	if ( !IsAccidental() )
	{
		int nToHit = pToHitCalcer->GetToHit();
		CObj<NRPG::CCoverInfo> pCover = pUS->GetWorld()->CalcCoversForTile( pUS->GetAttackOrigin( pUS->GetPosition() ), 
			attack[0], pUS, ptAnimTarget, pUS->GetMinClearDistance() );
		//
		if ( !NRPG::PeekRayForRocket( pCover, &ray, random.Get( 1, 100 ) <= nToHit ) )
		{
			ASSERT( 0 );
			return;
		}
	}
	//
	CVec3 speed = ray.ptDir * fSpeed;
	ray.ptOrigin += ray.ptDir * pUS->GetMinClearDistance();
	STime tThrow = pUS->animator.GetTimeLabel1();
	if ( IsAccidental() )
		tThrow = pUS->GetWorld()->GetTime()->GetValue();
	pUS->GetWorld()->LaunchRocket( ray.ptOrigin, speed, tThrow, 
		pToHitCalcer->GetMaxDistance() * FP_GRID_STEP, pModel, attack.front(), pRocket, pUS, pEffect );
	pUS->Update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecLaunchRocket::OnLabel()
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	pRPG->NextBullet();
	LaunchRocket();
	if ( !IsAccidental() )
		pUS->DoAction( NRPG::AC_END_SHOOT );
	pUS->CreateFlash();
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecOpenClose
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecOpenClose::CExecOpenClose( CUnitServer *_pUS, CCmdOpenClose *_pCmd ): 
	CCommandExecute(_pUS), pCmd(_pCmd)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecOpenClose::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	// ... here we check some conditions about window/door
	CDynamicCast<CWindowDoor> pOS( pCmd->pObject );
	ASSERT( pOS );
	if ( !pOS )
		return UCR_GENERAL_FAILURE;
	return UCR_OK; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecOpenClose::GetStartAP() const
{
	return pUS->GetActionAP( NRPG::AC_OPEN_CLOSE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecOpenClose::GetActionAP() const
{
	return GetStartAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecOpenClose::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );

	pUS->DoAction( NRPG::AC_OPEN_CLOSE );
	pUS->animator.OpenWindowDoor( pUS->GetPosition() );
	StartAction( pUS->GetWorld(), SKIPPABLE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecOpenClose::TimeLabelReached()
{
	CDynamicCast<CWindowDoor> pOS( pCmd->pObject );
	ASSERT( pOS );
	pOS->OpenClose( pCmd->bOpen );

	NDb::CAISound *pAISound;
	if ( pCmd->bOpen )
		pAISound = NDb::GetAISound( 23 );
	else
		pAISound = NDb::GetAISound( 25 );
	pUS->GetWorld()->MakeAISound( pAISound, pUS, 0, 0 );

	pUS->GetWorld()->GetPathNetwork()->FlipperOpenClose( pOS, pCmd->bOpen );
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecUsePassage
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecUsePassage::CExecUsePassage( CUnitServer *_pUS, CCmdUsePassage *_pCmd ):
	CCommandExecute(_pUS), pCmd( _pCmd )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecUsePassage::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecUsePassage::GetStartAP() const
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecUsePassage::Run()
{
	if ( CDynamicCast<NWorld::CPassageObject> pPassage( pCmd->pPassageObject ) )
		pPassage->UsePassageObject( pUS );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecCannon
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecCannon::CExecCannon( CUnitServer *_pUS, IObject *_pCannon, bool _bEnter ): 
	CCommandExecute(_pUS), pCannon(_pCannon), bEnter(_bEnter)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecCannon::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( !bEnter )
		return UCR_OK;
	if ( !IsValid( pCannon ) )
		return UCR_GENERAL_FAILURE;

	if ( bIgnoreTarget )
		return UCR_NO_TARGET;

	CDynamicCast<ICannon> pC( pCannon );
	if ( !pC || pC->IsBroken() || pC->IsOccupied() )
		return UCR_GENERAL_FAILURE;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecCannon::GetStartAP() const
{
	if ( bEnter )
		return pUS->GetActionAP( NRPG::AC_APPROACH_CANNON );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecCannon::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );

	CDynamicCast<CCannon> pOS( pCannon );
	ASSERT( pOS );
	if ( bEnter )
	{
		pUS->DoAction( NRPG::AC_APPROACH_CANNON );
		pUS->GetUnitRPG()->SetCannonItem( pOS->GetItem() );
		CDynamicCast<NRPG::CWeaponItem> pWeaponItem( pOS->GetItem() );
		ASSERT( pWeaponItem );
		pUS->animator.SetWeaponName( pWeaponItem->GetDBWeapon()->szAnimName.c_str() );
		pUS->animator.EnterCannon( pUS->GetPosition(), pOS );
		pOS->SetCurrentUnit( pUS );
		pUS->SetState( new CUnitStateUsingCannon( pUS, pOS ) );
	}
	else
	{
		pUS->GetUnitRPG()->SetCannonItem(0);
		pUS->animator.LeaveCannon( pUS->GetPosition() );
		pOS->SetCurrentUnit(0);
		pUS->SetState( new CUnitStateNormal( pUS ) );
	}
	StartAction( pUS->GetWorld(), SKIPPABLE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecCorpse
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecCorpse::CExecCorpse( CUnitServer *_pUS, CUnitServer *_pCorpse, bool _bTake ): 
	CCommandExecute(_pUS), pDeadUnit(_pCorpse), bTake(_bTake)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecCorpse::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( bIgnoreTarget )
		return UCR_NO_TARGET;

	if ( !IsValid( pDeadUnit ) )
		return UCR_INVALID_COMMAND;
	if ( bTake && pDeadUnit->GetCorpseCarrier() )
		return UCR_GENERAL_FAILURE;
	if ( !bTake && pDeadUnit->GetCorpseCarrier() != pUS )
		return UCR_GENERAL_FAILURE;
	if ( !bTake && !CanDropCorpse( pUS ) )
		return UCR_GENERAL_FAILURE;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecCorpse::GetStartAP() const 
{ 
	if ( bTake )
		return pUS->GetActionAP( NRPG::AC_TAKE_CORPSE ); 
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecCorpse::GetActionAP() const
{
	return GetStartAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecCorpse::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );
	ASSERT( pUS->CanSpendAP( GetStartAP() ) );
	if ( GetState() == FINISHED ) 
		return;
	if ( bTake )
	{
		pUS->DoAction( NRPG::AC_TAKE_CORPSE );
		pUS->animator.TakeCorpse( pUS->GetPosition() );
	}
	else
	{
		pUS->animator.DropCorpse( pUS->GetPosition() );
		NDb::CAISound *pAISound = NDb::GetAISound( 27 );
		pUS->GetWorld()->MakeAISound( pAISound, pUS, 0, 0 );
	}
	StartAction( pUS->GetWorld(), SKIPPABLE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecCorpse::TimeLabelReached()
{
	ASSERT( pDeadUnit );
	if ( bTake )
	{
		pDeadUnit->animator.BeTaken( pUS, &pUS->animator );
		pUS->SetState( new CUnitStateCorpseCarrier( pUS, pDeadUnit ) );
	}
	else
		pUS->SetState( new CUnitStateNormal( pUS ) );
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecCorpse::Cancel()
{
	if ( bTake )
	{
		pUS->animator.DropCorpse( pUS->GetPosition() );
		pUS->SetState( new CUnitStateNormal( pUS ) );
	}
	else
	{
		pUS->animator.TakeCorpse( pUS->GetPosition() );
		pUS->SetState( new CUnitStateCorpseCarrier( pUS, pDeadUnit ) );
	}

	pUS->animator.AlignTime( 50 );
	pUS->animator.PlaceUnit( pUS->GetPosition() );

	StopAction();
	Finished();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecTakeCorpseOnDeploy
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecTakeCorpseOnDeploy::CExecTakeCorpseOnDeploy( CUnitServer *_pUS, CUnitServer *_pCorpse, bool _bDead ):
	CCommandExecute(_pUS), pDeadUnit(_pCorpse), bDead( _bDead ), n( 0 )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecTakeCorpseOnDeploy::CanDoIt( const NAI::SUnitPosition &from, 
	bool bIgnoreTarget ) const
{
	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecTakeCorpseOnDeploy::GetStartAP() const
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecTakeCorpseOnDeploy::GetActionAP() const
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecTakeCorpseOnDeploy::Run()
{
	ASSERT( IsValid( pDeadUnit ) );
	ASSERT( IsValid( pUS ) );
	if ( !IsValid( pDeadUnit ) || !IsValid( pUS ) )
		return;
	//
	if ( !bDead )
		pDeadUnit->SetState( new CUnitStateUnconscious( pDeadUnit ) );
	else
		pDeadUnit->SetState( new CUnitStateDeath( pDeadUnit ) );
	pDeadUnit->GetWorld()->GetPathNetwork()->Unlock( pDeadUnit );
	pDeadUnit->InitAsCorpse( bDead );
	pDeadUnit->animator.InitAsCorpse( pDeadUnit->GetPosition() );
	int nCorpseDir = (int)( pUS->GetPosition().GetDir() ) - 4;
	if ( nCorpseDir < 0 )
		nCorpseDir += 8;
	else if ( nCorpseDir > 7 )
		nCorpseDir -= 8;
	NAI::SUnitPosition pos = pUS->GetPosition();
	pos.pos.p.SetDirection( nCorpseDir );
	pDeadUnit->SetPosition( pos );
	pDeadUnit->GetWorld()->GetPathNetwork()->Unlock( pDeadUnit );
	//
	pUS->animator.InitAsCorpseCarrier( pUS->GetPosition() );
	pUS->SetState( new CUnitStateCorpseCarrier( pUS, pDeadUnit ) );
	pDeadUnit->animator.BeTaken( pUS, &pUS->animator );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecTakeCorpseOnDeploy::TimeLabelReached()
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecHeal
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecHeal::CExecHeal( CUnitServer *_pUS, CUnitServer *_pTarget ): 
	CCommandExecute(_pUS), pTarget(_pTarget)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecHeal::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	return CanDoFirstAid( pUS, from, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecHeal::GetStartAP() const
{
	return pUS->GetActionAP( NRPG::AC_FIRSTAID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecHeal::GetActionAP() const
{
	return GetStartAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecHeal::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );

	CVec3 ptTarget;
	pUS->GetWorld()->GetAIMap()->GetUnitHLPos( &ptTarget, pTarget, -1 );
	pUS->animator.StartHealing( pUS->GetPosition(), NAI::GetBlowHeight( pUS->GetPosition(), ptTarget ) );
	StartAction( pUS->GetWorld(), SKIPPABLE );

	NDb::CAISound *pAISound = NDb::GetAISound( 20 );
	pTarget->GetWorld()->MakeAISound( pAISound, pUS, 0, 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecHeal::AnimationFinished()
{
	StopAction();
	Finished();
	//
	CPtr<CUnitStateHealer> pH = new CUnitStateHealer( pUS );
	if ( !pH->StartHeal( pTarget ) )
		pUS->SetState( pH );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecSnipeAim
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecSnipeAim::CExecSnipeAim( CUnitServer *_pUnitServer, CUnitServer *_pTarget ): 
	CCommandExecute(_pUnitServer), pTarget(_pTarget)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecSnipeAim::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const 
{ 
	if ( !pUS->CanSnipe() )
		return UCR_GENERAL_FAILURE;

	if ( GetActionType( pUS ) == AT_CANNON ) 
	{
		if ( bIgnoreTarget )
			return UCR_NO_TARGET;

		CVec3 ptTarget;
		pUS->GetWorld()->GetAIMap()->GetUnitHLPos( &ptTarget, pTarget, NAI::HL_BODY );
		return CanAttackWithCannon( pUS->animator.GetCannon(), ptTarget );
	}

	return HasWorkingWeapon( pUS );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecSnipeAim::GetStartAP() const 
{ 
	if ( !pUS->animator.IsAiming() )
		return pUS->GetUnitRPG()->GetActionAP( pUS->GetPosition().GetPose(), NRPG::AC_PREPARE );
	else
		return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecSnipeAim::GetActionAP() const
{
	return GetStartAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecSnipeAim::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );

	CRay ray;
	ray.ptOrigin = 	pUS->GetAttackOrigin( pUS->GetPosition() );
	ray.ptDir = pTarget->GetPosition().GetCP() - pUS->GetPosition().GetCP();

	StartAction( pUS->GetWorld(), SKIPPABLE );
	pUS->animator.Snipe( pUS->GetPosition(), ray );

	if ( !pUS->IsSniping()  )
	{
		if ( !pUS->animator.IsAiming() )
			pUS->DoAction( NRPG::AC_PREPARE );
		pUS->SetState( new CUnitStateSniping( pUS, pTarget, 0 ) );	
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecSnipeAim::AnimationFinished()
{
	StopAction();
	Finished();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecCollectSnipeAP
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecCollectSnipeAP::CExecCollectSnipeAP ( CUnitServer *_pUnitServer, ECollectSnipeAP _eAP ): 
	CCommandExecute(_pUnitServer), eAP( _eAP )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecCollectSnipeAP::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( !pUS->IsSniping() )
		return UCR_INVALID_COMMAND;

	if ( GetStartAP() <= 0 )
		return UCR_NOT_ENOUGH_AP;

	return UCR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecCollectSnipeAP::GetStartAP() const
{
	switch( eAP )
	{
		case CSAP_1AP:
			return 1;
		case CSAP_10AP:
			return 10;
		case CSAP_MAX:
			return pUS->GetUnitRPG()->GetSkillValue( NDb::ST_AP ) -	pUS->GetUnitRPG()->GetActionAP( NAI::WALK, NRPG::AC_SHOOT );
		case CSAP_ALL:
			return pUS->GetUnitRPG()->GetSkillValue( NDb::ST_AP );
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecCollectSnipeAP::GetActionAP() const
{
	return GetStartAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecCollectSnipeAP::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );

	pUS->CollectSnipeAP( GetStartAP() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecThrowKnife
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecThrowKnife::CExecThrowKnife( CUnitServer *_pUS, const CVec3 &_ptTarget, CUnitServer *_pTarget ): 
	CExecAttack(_pUS), ptTarget(_ptTarget), pTarget(_pTarget)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecThrowKnife::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget ) const
{
	if ( !HasThrowingKnife( pUS ) )
		return UCR_NO_EQUIPMENT;

	if ( bIgnoreTarget )
		return UCR_NO_TARGET;

	CDynamicCast<NRPG::IMeleeWeaponItem> pMelee( pUS->GetUnitRPG()->GetInventory()->GetActive() );
	return CanUnitThrowKnife( pUS, from, ptTarget, pMelee );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CExecThrowKnife::GetStartAP() const
{
	return pUS->GetActionAP( NRPG::AC_THROW_KNIFE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecThrowKnife::Start()
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	const NAI::SUnitPosition position = pUS->GetPosition();
	pUS->DoAction( NRPG::AC_THROW_KNIFE );
	pUS->animator.ThrowKnife( position, ptTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecThrowKnife::OnLabel()
{
	ThrowKnife();
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecThrowKnife::ThrowKnife()
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	NRPG::IInventory *pInventory = pRPG->GetInventory();
	CUnitServer::SResItem item;
	CDynamicCast<NRPG::IMeleeWeaponItem> pMelee( pInventory->GetActive() );
	if ( !IsValid( pMelee ) )
		return;
	vector<NRPG::CAttackPortion> attack;
	CreateAttack( &attack, 0, false );
	if ( attack.empty() )
		return;
	pUS->TearOffItem( &item, (NDb::ESlot)pInventory->GetActiveSlot() );

	int nToHit;
	int nDistance = fabs( ptTarget - pUS->GetPosition().GetCP() ) / FP_GRID_STEP;
	float fMaxDist;
	if ( pTarget )
	{
		CPtr<NRPG::CThrowKnifeToHitCalcer> pToHitCalcer = 
			new NRPG::CThrowKnifeToHitCalcer( pUS->GetUnitRPG(), pUS->GetPosition().GetPose(), 
			nDistance, pUS->GetPosition().GetCP(), pUS->GetWorld()->IsFirstTurn(), CVec3(1,1,1) );
		nToHit = pToHitCalcer->GetToHit();
		pToHitCalcer->Log();
		fMaxDist = pToHitCalcer->GetKnifeMaxDistance() * FP_GRID_STEP;
	}
	else
	{
		CPtr<NRPG::CThrowKnifeTileToHitCalcer> pToHitCalcer = 
			new NRPG::CThrowKnifeTileToHitCalcer( pUS->GetUnitRPG(), pUS->GetPosition().GetPose(), 
			nDistance, pUS->GetPosition().GetCP(), pUS->GetWorld()->IsFirstTurn(), CVec3(1,1,1), NAI::THL_MIDDLE, ptTarget );
		nToHit = pToHitCalcer->GetToHit();
		pToHitCalcer->Log();
		fMaxDist = pToHitCalcer->GetKnifeMaxDistance() * FP_GRID_STEP;
	}

	float fSpeed = 5.f;
	CVec3 speed = ptTarget - item.ptCenter;
	Normalize(&speed);
	speed *= fSpeed;
	if ( random.Get( 1, 100 ) > nToHit )
	{
		// промахиваемся
		// ищем куда действительно полетит ножик
		float fD = 0.15f * fabs( fSpeed );
		speed.x += random.GetFloat( -fD, +fD );
		speed.y += random.GetFloat( -fD, +fD );
		speed.z += random.GetFloat( -fD, +fD );
	}
	pUS->GetWorld()->ThrowKnife( item.ptCenter, speed, 
		pUS->animator.GetTimeLabel1(), fMaxDist, item.pModel, attack.front(), item.pItem ); 
	pUS->Update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExecMoveItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CExecMoveInventoryItem::CExecMoveInventoryItem( CUnitServer *_pUS, CCmdMoveInventoryItem *_p ):
	CCommandExecute(_pUS), pCmd(_p)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EUnitCommandResult CExecMoveInventoryItem::CanDoIt( const NAI::SUnitPosition &from, bool bIgnoreTarget )
{
	return CanMoveInventoryItem( pUS, pCmd );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecMoveInventoryItem::Run()
{
	ASSERT( CanDoIt( pUS->GetPosition() ) == UCR_OK );

	NRPG::IInventory *pInventory = pUS->GetUnitRPG()->GetInventory();
	NDb::ESlot slot = (NDb::ESlot)pInventory->GetActiveSlot();

	NDb::EItemSubType subType = NDb::SUBTYPE_NONE;
	if ( pInventory->Get(slot) )
		subType = pInventory->Get(slot)->GetDBItem()->subType;
	NDb::EItemSubType subTypeNext = NDb::SUBTYPE_NONE;
	if ( pCmd->GetSource().eType == SItem::HAND )
	{
		IPlayer::SItemInfo sInfo;
		pUS->GetTBSPlayer()->GetInHandItem( &sInfo );
		subTypeNext = sInfo.pItem->GetDBItem()->subType;
	}

	MoveInventoryItem( pUS, pCmd );

	bool bActive = pUS->animator.IsActiveItem();
	if ( (pCmd->GetSource().eType != SItem::SLOT || pCmd->GetSource().nSlot != slot) &&
		(pCmd->GetTarget().eType != SItem::SLOT || pCmd->GetTarget().nSlot != slot) )
	{
		pUS->SetUndrawItem( !bActive );
		Finished();
		return;
	}
	if ( !pUS->animator.CanActivateItem() )
	{
		NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
		pUS->animator.SetWeaponAnimation( bActive ? pRPG->GetWeaponType() : NDb::WT_DEFAULT );
		pUS->SetUndrawItem( !bActive );
		Finished();
		return;
	}
	StartAction( pUS->GetWorld(), NOBLOCK );
	bTwoHeavy = false;
	if ( subTypeNext == NDb::SUBTYPE_HEAVY || subType == NDb::SUBTYPE_HEAVY )
		bTwoHeavy = true;
	if ( subTypeNext == NDb::SUBTYPE_MINE_DETECTOR || subType == NDb::SUBTYPE_MINE_DETECTOR )
		bTwoHeavy = true;
	nStage = 1;
	if ( bActive && subType != NDb::SUBTYPE_NONE )
	{
		if ( subType == NDb::SUBTYPE_HEAVY || subType == NDb::SUBTYPE_MINE_DETECTOR )
			pUS->animator.DeactivateItem( pUS->GetPosition(), true, true, NDb::BELT_M1 );
		else
		{
			int nPlace = pInventory->GetPlaceBySubType( subType );
			pUS->animator.DeactivateItem( pUS->GetPosition(), false, nPlace == -1, (NDb::EItemPlace)nPlace );
		}
	}
	else
		AnimationFinished();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExecMoveInventoryItem::TimeLabelReached()
{
	if ( !nStage )
		return false;
	pUS->SetUndrawItem( nStage == 1, bTwoHeavy && nStage == 1 );
	return nStage == 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecMoveInventoryItem::AnimationFinished()
{
	if ( !nStage )
		return;
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	NRPG::IInventory *pInventory = pRPG->GetInventory();
	NDb::ESlot slot = (NDb::ESlot)pInventory->GetActiveSlot();
	if ( nStage == 2 || !pInventory->Get(slot) )
	{
		Finished();
		return;
	}
	nStage = 2;
	NDb::EItemSubType subType = pInventory->Get(slot)->GetDBItem()->subType;
	if ( subType == NDb::SUBTYPE_HEAVY || subType == NDb::SUBTYPE_MINE_DETECTOR )
		pUS->animator.ActivateItem( pUS->GetPosition(), true, true, NDb::BELT_M1, pRPG->GetWeaponType() );
	else
	{
		int nPlace = pInventory->GetPlaceBySubType( subType );
		pUS->animator.ActivateItem( pUS->GetPosition(), false, nPlace == -1, (NDb::EItemPlace)nPlace, pRPG->GetWeaponType() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExecMoveInventoryItem::Cancel()
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
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0x00122173, CExecShootTile )
REGISTER_SAVELOAD_CLASS( 0x00122174, CExecQueue )
REGISTER_SAVELOAD_CLASS( 0x00122175, CExecCannon )
REGISTER_SAVELOAD_CLASS( 0x00222190, CExecThrowGrenade )
REGISTER_SAVELOAD_CLASS( 0x00422170, CExecShootUnit )
REGISTER_SAVELOAD_CLASS( 0x00422171, CExecMeleeTile )
REGISTER_SAVELOAD_CLASS( 0x00422172, CExecMeleeUnit )
REGISTER_SAVELOAD_CLASS( 0x10422130, CExecOpenClose )
REGISTER_SAVELOAD_CLASS( 0x00422110, CExecCorpse )
REGISTER_SAVELOAD_CLASS( 0x00622140, CExecHeal )
REGISTER_SAVELOAD_CLASS( 0x53042170, CExecSnipeAim )
REGISTER_SAVELOAD_CLASS( 0x10662170, CExecThrowKnife )
REGISTER_SAVELOAD_CLASS( 0x01122132, CExecMoveInventoryItem )
REGISTER_SAVELOAD_CLASS( 0x50372121, CExecCollectSnipeAP )
REGISTER_SAVELOAD_CLASS( 0x50372000, CExecLaunchRocket )
REGISTER_SAVELOAD_CLASS( 0x51892150, CExecUsePassage )
REGISTER_SAVELOAD_CLASS( 0x52492170, CExecTakeCorpseOnDeploy )