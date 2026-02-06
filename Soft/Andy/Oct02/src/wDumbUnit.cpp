#include "StdAfx.h"
#include "wDumbUnit.h"
#include "wMain.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataGeometry.h"
#include "RPGGame.h"
#include "RPGItem.h"
#include "RPGUnitMission.h"
#include "..\misc\RandomGen.h"
#include "GAnimation.h"
#include "InventoryUnit.h"
#include "wMisc.h"
#include "wObject.h"
#include "wAckBase.h"
#include "aiCollider.h"
#include "aiInterval.h"
#include "..\Misc\LogStream.h"
#include "..\DBFormat\DataTerrain.h"
#include "..\DBFormat\DataAI.h"
#include "..\DBFormat\DataTerrain.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataSound.h"
#include "aiSignal.h"
#include "aiMap.h"

namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDumbUnitServer
////////////////////////////////////////////////////////////////////////////////////////////////////
CDumbUnitServer::CDumbUnitServer( CWorld *_pWorld, NRPG::IUnitMission *_pRPG, NDb::CModel *_pModel, 
	const NAI::SUnitPosition &pos ): 
	animator( _pWorld->GetTime(), pos, _pModel->pSkeleton, _pWorld->GetAIMap() ), 
	pWorld(_pWorld), bLocksTwoPlaces(false), bUnconscious( false ), bAlive( true )
{
	CPtr<NWorld::CDumbUnitServer> pHold(this);
	pRPG = _pRPG;
	pModel = _pModel;
	SetPositionCore( pos );
	wishPose = pos.GetPose();
	animator.SetWeaponAnimation( pRPG->GetWeaponType() );
	NRPG::IInventory *pInventory = pRPG->GetInventory();
	if ( IsValid( pInventory->GetActive() ) )
		animator.SetActiveItem( true );
	animator.PlaceUnit( pos );
	pHold.Extract();
	bindGlobal.Link( pWorld->GetUnits(), this );
	bUndrawWeapon = false;
	bNoHeavyWeapon = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::Visit( IRenderVisitor *p )
{
	vector<IRenderVisitor::SBoundMesh> boundMeshes;
	if ( bAlive && !bUnconscious )
		GetItemsBindPlaces( &boundMeshes, pRPG, bUndrawWeapon, bNoHeavyWeapon );
	NGScene::CLightGroup *pGroup = p->MakeGroup();
	p->AddMesh( pModel, animator.GetSkeletonAnimator(), animator.GetSkeletonState(), 
		boundMeshes, pGroup, position.pos.GetFloor(), dynamic_cast<CUnit*>(this) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::Visit( IAIVisitor *p )
{
	p->AddAnimatedHull( 
		pModel->pGeometry->pAIGeometry, pModel->pSkeleton, animator.GetSkeletonAnimator(), 
		pRPG->GetRPGArmor(), 
		pWorld->GetPathNetwork()->GetFloor( GetUnitPosition().pos.p.GetLayer() ), 
		TS_UNITS|TS_FRAGMENTED|TS_PICK|TS_COVER|TS_WEAPON_BLOCKER );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::GetBonePos( CVec3 *pRes, CQuat *pQuat, const char *pszBoneName )
{
	int nIndex = animator.GetSkeletonAnimator()->GetBoneIndex( pszBoneName );
	if ( nIndex < 0 )
		return;
	NAnimation::CAddBoneFilter *pFilter = new NAnimation::CAddBoneFilter( nIndex );
	pFilter->pAnimation = animator.GetSkeletonAnimator();
	CDGPtr<CFuncBase<SFBTransform> > p( pFilter );
	p.Refresh();
	const SFBTransform &t = p->GetValue();
	*pRes = t.forward.GetTranslation();
	pQuat->FromEulerMatrix( t.forward );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDumbUnitServer::TearOffItem( SResItem *pRes, NDb::ESlot slot, bool bPlaceNextSameItem )
{
	NRPG::IInventory *pInventory = pRPG->GetInventory();
	pRes->pItem = pInventory->Get( slot );
	const char *pszBoneName = GetBoneName( slot, pRPG, bUndrawWeapon );
	bool bRes = false;
	if ( IsValid( pRes->pItem ) )
	{
		if ( pszBoneName && pszBoneName[0] != 0 && IsValid( pRes->pItem->GetDBItem()->pModel ) )
		{
			SRand rnd;
			pRes->pModel = pRes->pItem->GetDBItem()->pModel->CreateModel( &rnd );
			GetBonePos( &pRes->ptCenter, &pRes->q, pszBoneName );
			bRes = true;
		}

		pInventory->TakeOff( slot );

		if ( bPlaceNextSameItem && IsValid( pRes->pItem ) )
		{
			const vector<NRPG::SBackPackItem> &sItems = pInventory->GetItems();
			for ( int nTemp = 0; nTemp < sItems.size(); nTemp++ )
			{
				if ( sItems[nTemp].pItem->GetDBItem() == pRes->pItem->GetDBItem() )
				{
					CObj<NRPG::IInventoryItem> pItem( sItems[nTemp].pItem );
					pInventory->Take( pItem );
					pInventory->Equip( slot, pItem );
					break;
				}
			}
		}
	}
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::SetPositionCore( const NAI::SUnitPosition &dst )
{
	animator.SetPose( dst.GetPose() );
	position = dst;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::LockNextPlace( const NAI::SUnitPosition &dst )
{
	if ( !IsDeadUnit() )
	{
		bLocksTwoPlaces = true;
		nextLock = dst.pos.p;
		pWorld->GetPathNetwork()->LockMovingObject( this, position.pos.p, dst.pos.p );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::SetPosition( const NAI::SUnitPosition &dst )
{
	bool bNeedUpdate = dst.pos.GetFloor() != position.pos.GetFloor();
	bool bRealMove = false;
	if ( position.GetPose() == dst.GetPose() && position.GetDir() == dst.GetDir() ) // собственно перемещение
			bRealMove = true;

	SetPositionCore( dst );
	if ( !IsDeadUnit() && !bUnconscious )
	{
		bLocksTwoPlaces = false;
		pWorld->GetPathNetwork()->Lock( this, position.pos.p );
	}
	pWorld->UpdateVisible();

	if ( bRealMove )
		MakeStepSound( false );
	if ( bNeedUpdate )
		bindGlobal.Update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::AddMiscObjects( vector<IVisObj*> *pRes )
{
	for ( list<CObj<IDynamicObject> >::const_iterator i = miscObjects.begin(); i != miscObjects.end(); ++i )
		pRes->push_back( *i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::AttachMiscObject( CTimedObject *p )
{
	p->Attach( pWorld->GetUnits(), pWorld );
	miscObjects.push_back( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::LaunchItem( const SResItem &item, const CVec3 &vel )
{
	pWorld->AddDebris( item.pModel, pWorld->GetAIMap(), item.ptCenter, item.q, vel, pWorld->GetTime(), item.pItem );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::PlaySound( NDb::CTSound *pSound )
{
	NDb::CSound *pS = NDb::GetSound( pSound );
	if ( pS )
		AttachMiscObject( new C3DSound( position.GetEyePosition(), pS ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::DropItems( bool bHands, bool bBackPack )
{
	SResItem item;
	CPtr<NRPG::IInventoryInfo> pInventoryInfo = pRPG->GetInventoryInfo();
	// поразбросим шмотье
	if ( bHands )
	{
		CDynamicCast<NRPG::IUniformItem> pUniform( pInventoryInfo->GetUniform() );
		NDb::CRPGUniform *pDBUniform = 0;
		SRand rnd;
		if ( pUniform )
			pDBUniform = pUniform->GetDBUniform();
		if ( pDBUniform )
		{
			if ( pDBUniform->pCapModel )
			{
				// и в воздух чепчики бросали
				GetBonePos( &item.ptCenter, &item.q, GetBoneName( UIT_CAP ) );
				item.pModel = pDBUniform->pCapModel->CreateModel(&rnd);
				LaunchItem( item );
			}
		}
		// item in hand goes away
		for ( int nSlot = 0; nSlot != NDb::N_SLOTS; ++nSlot )
			if ( TearOffItem( &item, (NDb::ESlot)nSlot ) )
				LaunchItem( item );
	}
	// выкидываем все из рюкзака
	if ( bBackPack )
	{
		const vector<NRPG::SBackPackItem> &sItems = pInventoryInfo->GetItems();
		for ( int n = 0; n < sItems.size(); ++n )
		{
			item.pItem = sItems[n].pItem;
			const char *pszBoneName = GetBoneName( UIT_BACKPACK, item.pItem );
			if ( IsValid( item.pItem ) && pszBoneName && 
				pszBoneName[0] != 0 && IsValid( item.pItem->GetDBItem()->pModel ) )
			{
				SRand rnd;
				item.pModel = item.pItem->GetDBItem()->pModel->CreateModel( &rnd );
				GetBonePos( &item.ptCenter, &item.q, pszBoneName );
				pRPG->GetInventory()->Take( item.pItem );
				LaunchItem( item );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::FallAsIfDead( const CVec3 &ptDir, bool bDropItemsFromBackPack )
{
	DropItems( true, bDropItemsFromBackPack );
	animator.Die( position, ptDir );
	pWorld->GetPathNetwork()->Unlock( this );
	bindGlobal.Update();
	pWorld->UpdateVisible();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::KillUnit( const CVec3 &ptDir )
{
	if ( !bAlive )
		return;
	bAlive = false;
	if ( !bUnconscious )
		FallAsIfDead( ptDir, true );
	else
		DropItems( false, true );
	OnUnitWasKilled();
	PlaySound( pRPG->GetRPGPers()->pSoundDeath );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::MakeUnconscious( const CVec3 &ptDir )
{
	if ( bUnconscious || !bAlive )
		return;
	bUnconscious = true;
	//
	OnUnitMadeUnconscious();
	FallAsIfDead( ptDir, false );
	PlaySound( pRPG->GetRPGPers()->pSoundDeath );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CDumbUnitServer::ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor )
{
	CDynamicCast<NRPG::IAttackable> pAtk( pRPG );
	ASSERT( pAtk );
	int nRes = pAtk->ProcessAttack( nUserID, pAttack, pArmor );

	if ( !IsDeadUnit() )
	{
		CVec3 ptHit;
		pWorld->GetAIMap()->GetUnitHLPos( &ptHit, this, NAI::HL_HEAD );
		pWorld->AddHitLocator( new CHitLocator( nRes, ptHit ) );
		PlaySound( pRPG->GetRPGPers()->pSoundHit );
	}

	if ( pRPG->IsDead() && !IsDeadUnit() )
		KillUnit( pAttack->rTtrajectory.ptDir );
	else if ( pRPG->IsUnconscious() )
		MakeUnconscious( pAttack->rTtrajectory.ptDir );
	else
	{		
		animator.Wound();

		NRPG::SUnitInfo sInfo;
		GetUnitRPG()->GetInfo( NAI::WALK, &sInfo );
		OnSuffersDamage( float(sInfo.nHP) / sInfo.nMaxHP );
		GetWorld()->GetAISignalManager()->Add( NAI::CreateAIHitSignal( pAttack->pAttacker, pAttack->pTarget ) );
	}

	if ( IsDeadUnit() )
		return nRes;

	NDb::ECritical eCA = pRPG->GetLastCritical();
	ProcessCritical( eCA );
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::Segment()
{
	CallSegment( &miscObjects );
	STime tCur = GetWorld()->GetTime()->GetValue();
	ProcessSteps( tCur );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// performs move from game logic`s point of view
// returns false if movement should be aborted after this transition
void CDumbUnitServer::DoGameMove( const NAI::SUnitPosition &dst )
{
	//SpendAP( pWorld->GetMoveActionType( position, dst, animator.IsCarryingCorpse() ) );
///	char buf[200];
//	NWorld::CDumbUnitServer *pAddr = this;
//	sprintf( buf, "DoGameMove : unitid %d, x=%d, y=%d, l=%d, p=%d\n", pAddr, dst.pos.p.GetX(), dst.pos.p.GetY(), dst.pos.p.GetLayer(), dst.pos.p.GetPose());
//	OutputDebugString( buf);

	SetPosition( dst );
	AttachMiscObject( new CDGrassEvent( dst.GetCP() ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDumbUnitServer::CheckPassable( const NAI::SUnitPosition &dst )
{
	pWorld->GetPathNetwork()->Unlock( this );
	bool bPassable = pWorld->GetPathNetwork()->IsPassable( dst.pos.p );//dst.pos.IsLocked();
	if ( !IsDeadUnit() )
	{
		if ( bLocksTwoPlaces )
			pWorld->GetPathNetwork()->LockMovingObject( this, position.pos.p, nextLock );
		else
			pWorld->GetPathNetwork()->Lock( this, position.pos.p );
	}
	return bPassable;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDumbUnitServer::CanDoGameMove( const NAI::SUnitPosition &dst )
{
	if ( !CheckPassable( dst ) || !pRPG->CanMove() )
		return false;
	return true;
		//CanSpendAP( pWorld->GetMoveActionType( position, dst, animator.IsCarryingCorpse() ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::PlaceOnPassablePlace()
{
	if ( IsDeadUnit() )
		return;
	NAI::IPathNetwork *pNet = pWorld->GetPathNetwork();
	pNet->Unlock( this );
	if ( pNet->IsPassable( position.pos.p ) )
	{
		bLocksTwoPlaces = false;
		pNet->Lock( this, position.pos.p );
		return;
	}
	CVec3 cp = position.GetCP();
	float fRadius = 1;
	for( int i = 0; i < 4; ++i, fRadius *= 2 )
	{
		vector<NAI::SPathPlace> places;
		pNet->GetNearPlaces( SSphere( cp, fRadius ), &places );
		float fBest = 1e38f;
		NAI::SPathPlace best;
		for ( int k = 0; k < places.size(); ++k )
		{
			places[k].SetPose( NAI::CM_STAND );
			if ( pNet->IsPassable( places[k] ) )
			{
				NAI::SPosition pos( position.pos );
				pos.p = places[k];
				float fDist = fabs2( cp - pos.GetCP() );
				if ( fDist < fBest )
				{
					fBest = fDist;
					best = places[k];
				}
			}
		}
		if ( fBest != 1e38f )
		{
			NAI::SPathPlace p = best;
			p.SetPose( position.pos.p.GetPose() );
			p.SetDirection( position.pos.p.GetDirection() );
			NAI::SUnitPosition newPos( position );
			newPos.pos.p = p;
			animator.PlaceUnit( newPos );
			SetPosition( newPos );
			return;			
		}
	}
	ASSERT( 0 ); // no suitable place was found
	OutputDebugString( "impossible to place unit correctly\n" );
	bLocksTwoPlaces = false;
	pNet->Lock( this, position.pos.p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::CreateFlash()
{
	NAnimation::SBonePose barrel;
	bool bTest = false;
	NRPG::IWeaponItem *pRPGWeapon;
	if ( animator.GetCannon() )
	{
		NRPG::IWeaponItem *pCannonItem = animator.GetCannon()->GetItem();
		if ( !pCannonItem )
			return;
		pRPGWeapon = pCannonItem;
	}
	else if ( CDynamicCast<NRPG::IWeaponItem> pW( pRPG->GetInventory()->GetActive() ) )
		pRPGWeapon = pW;
	else
		return;
	NDb::CRPGWeapon *pWeapon = pRPGWeapon->GetDBWeapon();
	ASSERT( pWeapon );
	bTest = animator.GetBarrelPos( animator.GetCannon() ? 0 : pWeapon->GetModel()->pGeometry, &barrel );
	ASSERT( bTest );
	if ( pWeapon->pShotEffect )
	{
		CQuat rndX( random.GetFloat( - FP_PI / 8, FP_PI / 8 ), CVec3(1,0,0) );
		SRand rnd;
		AttachMiscObject( new CDParticles( barrel.pos, barrel.rot * rndX, pWeapon->pShotEffect->GetEffect( &rnd ) ) );
	}
	AttachMiscObject( new CDFlash( barrel.pos, CVec3(0.3f,0.3f,0.3f), 2.5f, 3 ) );
	NDb::CSound *pSound = 0;
	NDb::CAISound *pAISound = 0;
	bool bDoPlay = true;
	switch ( pRPGWeapon->GetShootMode() )
	{
		case NDb::SM_Snap:
		case NDb::SM_Aimed:
		case NDb::SM_Careful:
		case NDb::SM_Snipe:
			pSound = pWeapon->pSound; // AttachMiscObject // barrel.pos
			pAISound = pRPGWeapon->GetDBWeapon()->pWeaponType->pAISound;
			break;
		case NDb::SM_ShortBurst:
		case NDb::SM_LongBurst:
			if ( pRPG->GetNBullets() == 1 )
			{
				pSound = pWeapon->pSoundBurst; // AttachMiscObject( // barrel.pos
				pAISound = pRPGWeapon->GetDBWeapon()->pWeaponType->pBurstAISound;
			}
			else
				bDoPlay = false;
			break;
	}
	//
	if ( bDoPlay )
		pWorld->MakeAISound( pAISound, this, 0, pSound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CDumbUnitServer::GetActionAP( NRPG::EAction action ) const
{
	return pRPG->GetActionAP( GetUnitPosition().GetPose(), action ); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CDumbUnitServer::GetAP() const
{
	return pRPG->GetAP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDumbUnitServer::CanSpendAP( int nAP ) const
{
	if ( pWorld->IsRealTime() )
		return true;
	return pRPG->CanSpendAP( nAP );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::SpendAP( int nAP )
{
	if ( pWorld->IsRealTime() )
		return;
	pRPG->SpendAP( nAP );
	//NRPG::IUnitMission *pRPG = GetRPGUnitMission();
/*	int nRet = pRPG->GetActionAP( GetUnitPosition().GetPose(), action, nMaxRequiredAP );
	if ( !pWorld->IsRealTime() )
		pRPG->SpendAP( GetUnitPosition().GetPose(), action, nMaxRequiredAP );
	else
		nRet = nMaxRequiredAP;
	return nRet;*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::DoAction( NRPG::EAction action )
{
	SpendAP( GetActionAP( action ) );
	pRPG->RegisterAction( action );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::GetTileOrArmor( NDb::CTerrainTile **pTile, NDb::CRPGArmor **pArmor )
{
	*pTile = 0;
	*pArmor = 0;

	CVec3 point = position.GetCP();
	point.z += 0.2f;
	vector<SSphere> spheres;
	vector<CVec3> vels;
	vector<NAI::CCollider::SCollisionPoint> ress;
	spheres.push_back( SSphere( point, 0.1f ) );
	vels.push_back( CVec3(0,0,-1.0f) );
	NAI::CollideInfo( pWorld->GetAIMap(), spheres, vels, &ress, TS_GO_OVER );
	NAI::CCollider::SCollisionPoint &res = ress[0];
	if ( res.fDist != NAI::FP_NO_COLLISION )
	{
		if ( res.pSrc->pUserData )
		{
			// armor
			*pArmor = res.pSrc->pArmor;
		}
		else
		{
			// terrain
			CDGPtr<CFuncBase<STerrainInfo> > pInfo = pWorld->GetTerrainInfo();
			pInfo.Refresh();
			const STerrainInfo &info = pInfo->GetValue();
			point /= FP_GRID_STEP;
			int nX = Float2Int( point.x );
			int nY = Float2Int( point.y );
			if ( !( nX < 0 || nY < 0 || nX >= info.typeMap.GetXSize() || nY >= info.typeMap.GetYSize() ) )
				*pTile = NDb::GetTerrainTile( info.typeMap[nY][nX] );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CAISound *CDumbUnitServer::GetStepAISound()
{
	int nID = 0;
	if ( IsStrafing() )
	{
		nID = 7;
	}
	else
	{
		switch ( position.GetPose() )
		{
		case NAI::CRAWL:
			nID = 3; break;
		case NAI::CROUCH:
			nID = 4; break;
		case NAI::WALK:
			nID = 5; break;
		case NAI::RUN:
			nID = 6; break;
		}
	}
	return NDb::GetAISound( nID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CSound* CDumbUnitServer::GetStepSound( NDb::CRPGArmor *pArmor, NDb::CTerrainTile *pTile )
{
	if ( position.GetPose() == NAI::CRAWL )
		return 0;

	if ( !pTile && !pArmor )
		return 0;

	SRand rand;
	if ( pArmor )
	{
		if ( pArmor->pSoundStep )
			return pArmor->pSoundStep->GetSound( &rand )->pSound;
		return 0;
	}

	if ( !pTile || !pTile->pSoundStep )
		return 0;
	return pTile->pSoundStep->GetSound( &rand )->pSound;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CDumbUnitServer::GetAISoundType( NDb::CRPGArmor *pArmor, NDb::CTerrainTile *pTile ) const
{
	if ( !pTile && !pArmor )
		return 0;
	if ( pArmor )
		return pArmor->nAISoundType;
	return pTile->nAISoundType;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::MakeStepSound( bool bSound )
{
	NDb::CAISound *pAISound = GetStepAISound();
	int nSoundType = 0;
	NDb::CSound *pSound = 0;

	NDb::CRPGArmor *pArmor;
	NDb::CTerrainTile *pTile;
	GetTileOrArmor( &pTile, &pArmor );

	if ( bSound )
		pSound = GetStepSound( pArmor, pTile );

	pWorld->MakeAISound( pAISound, this, GetAISoundType( pArmor, pTile ), pSound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::ProcessSteps( const STime tCurrent )
{
	STime tStep;
	while ( animator.GetStepTime( &tStep ) && tCurrent >= tStep )
	{
		MakeStepSound( true );
		animator.DropStep();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::GetRealUnitPosition( CVec3 *pRes )
{
	if ( animator.GetHipPos( pRes ) )
		return;

	*pRes = position.GetCP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDumbUnitServer::InitAsCorpse( bool bDead ) 
{ 
	GetUnitRPG()->InitAsCorpse( bDead );
	bUnconscious = !bDead; 
	bAlive = !bDead; 
	bindGlobal.Update(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
