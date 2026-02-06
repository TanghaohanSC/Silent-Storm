#include "StdAfx.h"
#include "wGrenade.h"
#include "wMain.h"
#include "GAnimation.h"
#include "GAnimParticles.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataRPG.h"
#include "aiMap.h"
#include "RPGAttackMech.h"

namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGrenadeServer
////////////////////////////////////////////////////////////////////////////////////////////////////
CGrenadeServer::CGrenadeServer( CWorld *_pWorld, const CVec3 &vFrom, 
	const CVec3 &vSpeed, STime tThrow, float fTDelay, NDb::CModel *_pModel, 
	NDb::CRPGGrenade *_pRPGGrenade, CUnitServer *_pUnitServer )
: pWorld(_pWorld), pModel(_pModel), pRPGGrenade(_pRPGGrenade), pUnitServer(_pUnitServer)//tExplode(_tExplode)//, pItem(_pItem)
{
/*	float tFly = fTGrenade;
	
	STime tThrow = animator.GetTimeLabel1();
	SUnitItem sItem;
	GetItem( UIT_HAND, &sItem );
	ASSERT( sItem.pItem );
	tExplode = tThrow + (STime)Float2Int(tFly * 1000) + 100;
	//CGrenadeServer *pGrenade = new CGrenadeServer( , sItem.pItem );
	//pWorld->AddGrenade( pGrenade );
	DetachItem( UIT_HAND );
	bindGlobal.Update();*/

	// launch actual item
	vector<SMassSphere> spheres;
	CVec3 massCenter;
	NAI::GetSpheres( pModel, &spheres, &massCenter );
	pSphere = new NAnimation::CASphereSet;
	pSphere->pTime = pWorld->GetTime();
	pSphere->pMap = pWorld->GetAIMap();
	pSphere->InitSpheres( spheres );
	pSphere->InitBound( pModel->pGeometry->boundCenter, pModel->pGeometry->boundSize );	
	pSphere->Init( tThrow, vFrom, QNULL, vSpeed, true );
	pAnimator = new NAnimation::CSkeletonAnimator;
	pAnimator->pTime = pWorld->GetTime();
	pAnimator->AddAnimator( pWorld->GetTime()->GetValue(), pSphere );
	//sItem.pAnimItem->AddTransit( tThrow, tThrow + 200, pSphere );
	bindGlobal.Link( pWorld->GetActive(), this );
	tExplode = tThrow + (STime)int( (float(pRPGGrenade->nMaxDelay) - fTDelay) * 1000 );
	tErase = tExplode + (STime)( 10 * 1000 ); // больше 10-ти секунд гранаты не летают
	pAction = pWorld->GetActiveCounter();
	bTimeDelayGrenade = ( pRPGGrenade->nMaxDelay != 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const CVec3 CGrenadeServer::GetPosition()
{
/*	CDGPtr<	CFuncBase<NAnimation::SSkeletonPose> > pPosition( pItem->pPosition );
	pPosition.Refresh();
	NAnimation::SSkeletonPose pose = pPosition->GetValue();
	return pose[0].pos;*/
	pAnimator.Refresh();
	NAnimation::SSkeletonPose pos = pAnimator->GetValue();
	return pos[0].pos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGrenadeServer::Visit( IRenderVisitor *p )
{
	p->AddItemMesh( pModel, pAnimator );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGrenadeServer::Segment()
{
	if ( pWorld->GetTime()->GetValue() > tErase )
		return true;
	//
	bool bCollisionExplode = !bTimeDelayGrenade && pSphere->DidCollide();
	bool bIsTimeToExplode = bTimeDelayGrenade && IsTimeToExplode( pWorld->GetTime()->GetValue() );
	if ( bIsTimeToExplode || bCollisionExplode )
	{
		pWorld->AddGrenadeExplosion( GetPosition(), pRPGGrenade, pUnitServer );
		return true; // = erase
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CClickOfDeath
////////////////////////////////////////////////////////////////////////////////////////////////////
CClickOfDeath::CClickOfDeath( CActionCounter *_pAction, CObjectBase *_pTarget, int _nUserID, const CRay &_ray )
: pAction(_pAction), pTarget(_pTarget), nUserID(_nUserID), ray(_ray)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClickOfDeath::Segment()
{
	NRPG::CAttackPortion atk;
	atk.MakeClickOfDeath( ray );
	if ( CDynamicCast<NRPG::IAttackable> pT( pTarget ) )
		pT->ProcessAttack( nUserID, &atk, NDb::GetArmor( NDb::CRPGArmor::WOOD ) ); // CRAP, WOOD should be DEFAULT
	return true; // = erase
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0x118B1160, CGrenadeServer )
REGISTER_SAVELOAD_CLASS( 0x118B1161, CClickOfDeath )
