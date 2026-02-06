#include "StdAfx.h"
#include "wBullet.h"
#include "wMain.h"
#include "GAnimation.h"
#include "GAnimParticles.h"
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataRPG.h"
#include "aiMap.h"
#include "wUnitServer.h"
#include "RPGUnitMission.h"
#include "wAckBase.h"
#include "aiSignal.h"

namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGrenadeServer
////////////////////////////////////////////////////////////////////////////////////////////////////
CBulletServer::CBulletServer( CWorld *_pWorld, const vector<NRPG::STrailPoint> &_trail, STime _sCast, NDb::CModel *_pTrailModel, float _fTrailSpeed )
	:pWorld(_pWorld), trailpointsSet( _trail ), sCast( _sCast ), pModel( _pTrailModel ), fTrailSpeed( _fTrailSpeed ), nTrailCount( 0 )
{
	sPassTime.resize( trailpointsSet.size() );
	vector<NAnimation::CATrailPath::STrailPoint> animTrailPoints( trailpointsSet.size() );

	for ( int nTemp = 0; nTemp < trailpointsSet.size(); nTemp++ )
	{
		if ( pModel && ( nTemp > 0 ) )
			sPassTime[nTemp] = sCast + fabs( trailpointsSet[nTemp].vPosition - trailpointsSet[nTemp - 1].vPosition ) * 1000.0f / fTrailSpeed;
		else
			sPassTime[nTemp] = sCast;

		CVec3 vDir( trailpointsSet[nTemp].vDir );
		Normalize( &vDir );

		animTrailPoints[nTemp].vDir = vDir * fTrailSpeed;
		animTrailPoints[nTemp].vPosition = trailpointsSet[nTemp].vPosition;
		animTrailPoints[nTemp].sPassTime = sPassTime[nTemp];
	}

	if ( pModel )
	{
		NAnimation::CATrailPath *pRay = new NAnimation::CATrailPath;
		pRay->Init( sCast, animTrailPoints );

		pAnimator = new NAnimation::CSkeletonAnimator;
		pAnimator->pTime = pWorld->GetTime();
		pAnimator->AddAnimator( pWorld->GetTime()->GetValue(), pRay );
	}

	bindGlobal.Link( pWorld->GetActive(), this );
	pAction = pWorld->GetActiveCounter();
	pShooter = pWorld->GetUnitServer( trailpointsSet[0].sAttack.pAttacker );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CBulletServer::Visit( IRenderVisitor *pVisitor )
{
	if ( pModel )
		pVisitor->AddItemMesh( pModel, pAnimator );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBulletServer::Segment()
{
	STime sSegmentTime = pWorld->GetTime()->GetValue();
	for ( int nTemp = nTrailCount + 1; nTemp < trailpointsSet.size(); nTemp++ )
	{
		if ( sPassTime[nTemp] > sSegmentTime )
			break;

		nTrailCount = nTemp;
		NRPG::STrailPoint &sCurrent = trailpointsSet[nTemp];
		if ( sCurrent.pArmor )
		{
			if ( sCurrent.pArmor->pShotEffect )
			{
				CVec3 dir = -trailpointsSet[nTemp].vDir;
				CQuat rndX( random.GetFloat(0,10000), CVec3(1,0,0) );
				if ( fabs( dir.x ) < 0.999f )
					rndX = CQuat( acos( dir.x ), CVec3( 0, -dir.z, dir.y ), true ) * rndX;
				else if ( dir.x < 0 )
					rndX = CQuat( FP_PI, CVec3(0,1,0) ) * rndX;
				SRand rnd;
				pWorld->CreateParticle( sCurrent.vPosition, rndX, sCurrent.pArmor->pShotEffect->GetEffect( &rnd ) );
			}
			if ( sCurrent.pArmor->pSoundShot )
			{
				NDb::CSound *pS = NDb::GetSound( sCurrent.pArmor->pSoundShot );
				pWorld->MakeSound( sCurrent.vPosition, pS );
				pWorld->GetAISignalManager()->Add( NAI::CreateAISoundSignal( sCurrent.vPosition, pShooter, 3 ) );
			}
		}
		if ( CDynamicCast<NRPG::IAttackable> pAttackCatcher( trailpointsSet[nTemp].pCatcher ) )
		{
			pAttackCatcher->ProcessAttack( sCurrent.nUserID, &sCurrent.sAttack, sCurrent.pArmor );
		
			if ( CDynamicCast<NWorld::CUnitServer> pUS( trailpointsSet[nTemp].pCatcher ) )
			{
				pUS->GetUnitRPG()->BulletHit();
				CPtr<NWorld::CUnitServer> pTarget = pWorld->GetUnitServer( trailpointsSet[nTemp].sAttack.pTarget );
				if ( IsValid( pShooter ) )
				{
					if ( pTarget.GetPtr() == pUS.GetPtr() )
						pWorld->GetGlobalAck()->OnDoDamage( pShooter, pUS );
					else
						pWorld->GetGlobalAck()->OnDoAccidentalDamage( pShooter, pUS );
				}
			}
		}
	}

	if ( nTrailCount >= trailpointsSet.size() - 1 )
	{
		if ( IsValid( pShooter ) )
		{
			CRay ray;
			int nLast = trailpointsSet.size() - 1;
			ray.ptOrigin = trailpointsSet[0].vPosition;
			ray.ptDir = trailpointsSet[nLast].vPosition - ray.ptOrigin;
			Normalize( &( ray.ptDir ) );
			pWorld->GetAISignalManager()->Add( NAI::CreateAIShootSignal( pShooter, ray ) );
		}
		return true; // = erase
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0xB18B1160, CBulletServer )
