#include "StdAfx.h"

#include "wObject.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataAnimation.h"
#include "..\DBFormat\DataFormat.h"
#include "RPGObject.h"
#include "RPGItem.h"
#include "Transform.h"
#include "GAnimation.h"
#include "GAnimBase.h"
#include "wMain.h"
#include "aiGrid.h"
#include "wUnitMove.h"
#include "wOSBase.h"
#include "wUnitServer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CWindowDoor
////////////////////////////////////////////////////////////////////////////////////////////////////
CWindowDoor::CWindowDoor( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
	NDb::CObject *pO, NRPG::IObject *pRPG, CFuncBase<STime> *_pTime, bool bOpen )
	: CAnimObjectServerBase( pWorld, pos, bLightMap, pO, pRPG, _pTime )
{
	bIsOpen = bOpen;
	if ( bOpen )
	{
		STime t = pTime->GetValue();
		CPtr<NAnimation::CAnimation> pAnim = pAnimator->CreateAnimation(
			pSkeleton->GetAnimation( NDb::CAnimation::DEACTIVATE, 0 ), t );
		if ( pAnim )
		{
			pAnim->SetStand( t, position.ptPos, position.fAngle );
			pAnimator->AddAnimator( t, pAnim );
			pAnimator->AddMemorizer( t );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWindowDoor::IsBroken() const
{
	return bBroken;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CWindowDoor::ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor )
{
	int nRes = CAnimObjectServerBase::ProcessAttack( nUserID, pAttack, pArmor );
	// ищем направление перемещения из текущего положения в другое
	//if ( pAttack->bBlastWave && !IsBroken() )
	if ( !IsBroken() )
	{
		CDynamicCast<NAI::CPathNetwork> pNetwork( pWorld->GetPathNetwork() );
		NAI::CPathNetwork::SFlipper *pFlipper = 0;
		pFlipper = pNetwork->GetFlipper( this );
		if ( pFlipper != 0 && !pFlipper->locksOpen.empty() && !pFlipper->locksClosed.empty() )
		{
			//CRAP{
			bool bBigGates = pFlipper->locksOpen.size() > 100 || pFlipper->locksClosed.size() > 100;
			//CRAP}
			if ( !bBigGates )
			{
				CVec3 ptOpened, ptClosed;
				ptOpened = pNetwork->GetCP( pFlipper->locksOpen.begin()->first );
				ptClosed = pNetwork->GetCP( pFlipper->locksClosed.begin()->first );
				CVec3 ptDir;
				if ( pFlipper->bOpen )
					ptDir = ptClosed - ptOpened;
				else	
					ptDir = ptOpened - ptClosed;
				// если направление хорошое, то шевелим дверью
				if ( ptDir * pAttack->rTtrajectory.ptDir > 0 )
				{
					bool bTmpOpened = pFlipper->bOpen;
					OpenClose( !bTmpOpened, true );
					pNetwork->FlipperOpenClose( this, !bTmpOpened );
				}
			}
		}
	}
	//
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWindowDoor::OpenClose( bool bOpen, bool bAbruptly )
{
	const STime tAbruptly = 100;
	//
	if ( IsBroken() )
		return;
	if ( bOpen == bIsOpen )
		return;
	bIsOpen = bOpen;
	tEnd = pTime->GetValue();
	CPtr<NAnimation::CAnimation> pAnim;
	if ( bOpen )
	{
		if ( !bAbruptly )
			pWorld->MakeSound( position.ptPos, pDbObject->pDoor->pOpenSound );
		pAnim = pAnimator->CreateAnimation(
			pSkeleton->GetAnimation( NDb::CAnimation::ACTIVATE, 0 ), tEnd );
		//pCurrentWorld->Add3DSound( NDb::GetSound( 13 ), new NGScene::CCFBTransform( position ), 0 );
	}
	else
	{
		if ( !bAbruptly )
			pWorld->MakeSound( position.ptPos, pDbObject->pDoor->pCloseSound );
		pAnim = pAnimator->CreateAnimation(
			pSkeleton->GetAnimation( NDb::CAnimation::DEACTIVATE, 0 ), tEnd );
		//pCurrentWorld->Add3DSound( NDb::GetSound( 14 ), new NGScene::CCFBTransform( position ), 0 );
	}
	if ( pAnim )
	{
		pAnim->SetStand( tEnd, position.ptPos, position.fAngle );
		pAnimator->AddAnimator( tEnd, pAnim );
		if ( bAbruptly )
			pAnim->SetInterval( tEnd, tEnd + tAbruptly );
		tEnd += pAnim->GetTime();
		pAction = pCurrentWorld->GetActiveCounter();
		pAnimator->AddMemorizer( tEnd );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWindowDoor::Segment()
{
	STime tCur = pTime->GetValue();
	if ( tCur > tEnd && IsValid( pAction ) )
	{
		pAction = 0;
		pWorld->UpdateVisible();
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWindowDoor::Visit( IAIVisitor *p )
{
	nDestroyStage = pRPG->GetDestroyStage();
	SFBTransform rv;
	CreateTransform( &rv );

	NDb::CContainerModel *pCont = pDbObject->pModels[nDestroyStage];
	if ( pCont )
	{
		NDb::CModel *pModel = pCont->pModel;
		if ( pModel && pModel->pGeometry && pModel->pGeometry->pAIGeometry )
		{
			int nMask = GetMask( pModel->pGeometry->pAIGeometry ) | TS_PICK;
			if ( !bBroken && pModel->pSkeleton == pSkeleton )
			{
				NAnimation::CAnimation *pAnim1, *pAnim2;
				// this is a crap animator made to get only one frame #0 ( i.e. when door is closed ) 
				CPtr<NAnimation::CSkeletonAnimator> pAn1 = new NAnimation::CSkeletonAnimator( pSkeleton );
				STime current = pTime->GetValue();
				pAn1->pTime = new CCTime( current );
				pAnim1 = pAn1->CreateAnimation(
					pSkeleton->GetAnimation( NDb::CAnimation::ACTIVATE, 0 ), current );
				if ( pAnim1 )
				{
					pAnim1->SetStand( current, pState->state.pos, pState->state.fAngle );
					pAn1->AddAnimator( current, pAnim1 );
				}
				// this is a crap animator made to get only one frame #0 ( i.e. when door is open )
				CPtr<NAnimation::CSkeletonAnimator> pAn2 = new NAnimation::CSkeletonAnimator( pSkeleton );
				pAn2->pTime = new CCTime( current );
				pAnim2 = pAn2->CreateAnimation(
					pSkeleton->GetAnimation( NDb::CAnimation::DEACTIVATE, 0 ), current );
				if ( pAnim2 )
				{
					pAnim2->SetStand( current, pState->state.pos, pState->state.fAngle );
					pAn2->AddAnimator( current, pAnim2 );
				}
				p->AddFlippingHull( pModel->pGeometry->pAIGeometry, pSkeleton, pAn1, pAn2, pModel->pRPGArmor, 
					position.nFloor, nMask, bIsOpen );
			}
			else
			{
				bBroken = true;
				p->AddHull( pModel->pGeometry->pAIGeometry, rv, pModel->pRPGArmor, position.nFloor, nMask );
			}
		}
		AddEffects( p, pCont, rv );
	}
	
	if ( IsValid( pDbObject->pChild ) )
		AddObject( p, pDbObject->pChild, rv );

}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCannon
////////////////////////////////////////////////////////////////////////////////////////////////////
CCannon::CCannon( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
	NDb::CObject *pO, NRPG::IObject *pRPG, CFuncBase<STime> *_pTime )
	: CAnimObjectServerBase( pWorld, pos, bLightMap, pO, pRPG, _pTime )
{
	ASSERT( IsValid( pO->pGun ) );
	pItem = NRPG::CreateWeaponItem( pO->pGun->pWeapon );
	ASSERT( pItem );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 CCannon::GetPosition()
{
	return position.ptPos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CCannon::GetDirection()
{
	return position.fAngle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CCannon::IsBroken() const
{
	return bBroken;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPassageObject
////////////////////////////////////////////////////////////////////////////////////////////////////
CPassageObject::CPassageObject( CWorld *pWorld, const SObjectPlace &pos, 
	bool bLightMap, NDb::CObject *pO, NRPG::IObject *pRPG, CFuncBase<STime> *_pTime, 
	int _nPassageZoneID, int _nPassageObjectID, int _nAPRadius ):
		CAnimObjectServerBase( pWorld, pos, bLightMap, pO, pRPG, _pTime ), 
		nPassageZoneID( _nPassageZoneID ), nPassageObjectID( _nPassageObjectID ), nAPRadius( _nAPRadius )
{
	ASSERT( IsValid( pWorld ) );
	ASSERT( IsValid( pO ) );
	ASSERT( IsValid( pO->pPassage ) );
	ASSERT( IsValid( pRPG ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPassageObject::CanPass( CUnitServer *pUS )
{
	vector<NAI::SPathPlace> approaches;
	GetApproaches( &approaches, GetWorld()->GetPathNetwork() );
	CPtr<NAI::CPath> pPath = GetWorld()->FindPath( pUS, 
		pUS->GetPosition().pos.p, approaches, pUS, false, NAI::PF_DEFAULT, false, false, true );
	if ( IsValid( pPath ) )
	{
		CPtr<CCommandExecute> pExec = CreateSimpleMoveExecutor( pUS, pPath, NAI::PF_DEFAULT );
		if ( IsValid( pExec ) )
			return pExec->GetActionAP() <= nAPRadius;
	}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPassageObject::IsBroken() const
{
	return bBroken;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPassageObject::UsePassageObject( CUnitServer *pUS )
{
	if ( !IsBroken() )
	{
		bool bRes = pWorld->UsePassageObject( pUS, nPassageZoneID );
		if ( !bRes )
			OutputDebugString( "[PASSAGE] Can't use passage\n" );
		else
			OutputDebugString( "[PASSAGE] Passage activated\n" );
		return bRes;
	}
	else
	{
		OutputDebugString( "[PASSAGE] Passage broken\n" );
		return false;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0x11291180, CObjectServer )
REGISTER_SAVELOAD_CLASS( 0x12512140, CWindowDoor )
REGISTER_SAVELOAD_CLASS( 0x12512141, CCannon )
REGISTER_SAVELOAD_CLASS( 0x11122120, CAnimObjectServer )
REGISTER_SAVELOAD_CLASS( 0x51892141, CPassageObject )