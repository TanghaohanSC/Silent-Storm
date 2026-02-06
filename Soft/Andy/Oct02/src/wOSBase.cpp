#include "StdAfx.h"

#include "wOSBase.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataAnimation.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataSound.h"
#include "..\DBFormat\DataRPG.h"
#include "RPGObject.h"
#include "Transform.h"
#include "aiStability.h"
#include "wMain.h"
#include "wMisc.h"
#include "GAnimation.h"
#include "Grid.h"
#include "GSceneUtils.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\LogStream.h"

namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CObjectServerBase
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectServerBase::CObjectServerBase( CWorld *_pWorld, const SObjectPlace &_pos, bool _bLightMap,
	NDb::CObject *_pDbO, NRPG::IObject *_pRPG )
	: pWorld(_pWorld), position(_pos), bLightMap(_bLightMap)
{
	pRPG = _pRPG;
	pDbObject = _pDbO;
	nDestroyStage = 0;
	tStageChange = 0;
	tLastSound = pWorld->GetTime()->GetValue();
	bindGlobal.Link( pWorld->GetActive(), this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CDebrisMaterial* CObjectServerBase::GetDebrisMaterial() const
{
	return pDbObject->pDebrisMaterial;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObjectServerBase::IsTargetable() const
{
	return pDbObject->bTargetable;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObjectServerBase::NeedSegment() const
{
	NDb::CContainerModel *pCont = pDbObject->pModels[nDestroyStage];
	if ( IsValid( pCont ) && pCont->eSoundType == NDb::ST_RANDOM && pCont->pSound )
		return true;
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::CreateTransform( SFBTransform *pRes )
{
	SFBTransform &rv = *pRes;
	MakeMatrix( &rv, CVec3(1,1,1), position.ptPos, position.fAngle );
	rv = rv * MakeTransform( VNULL3, position.ptScale );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObjectServerBase::CheckStability()
{
	NDb::CContainerModel *pCont = pDbObject->pModels[ pRPG->GetDestroyStage() ];
	if ( !pCont || !IsValid( pCont->pModel ) )
		return true;
	if ( pRPG->IsDead() )
		return true;
	SFBTransform rv;
	CreateTransform( &rv );
	if ( !NAI::CheckObjectStability( pCurrentWorld->GetAIMap(), pCont->pModel, rv.forward, this ) )
	{
		Kill( VNULL3 );
		return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::Kill( const CVec3 &ptDir )
{
	//pCurrentWorld->AddActionLocator( new CActionLocator( CActionLocator::TYPE_DIE, this, position.ptPos ), 1000 );
	/*
	pCurrentWorld->GenerateDebris( GetDebrisMaterial(), position.ptPos, ptDir, 5 );
	pCurrentWorld->ActivateDebris( SSphere( position.ptPos, 5 ), pCurrentWorld->GetAIMap(), pCurrentWorld->GetTime() ); // CRAP
	pCurrentWorld->KillObject( this );
	*/
	int nPrevStage = pRPG->GetDestroyStage();
	pRPG->Kill();
	if ( nPrevStage != pRPG->GetDestroyStage() )
	{
		tStageChange = pCurrentWorld->GetAimTime()->GetValue();
		bindGlobal.Update();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CObjectServerBase::ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor )
{
	if ( !IsValid( this ) )
		return false;
	CDynamicCast<NRPG::IAttackable> pAtk( pRPG );
	ASSERT( pAtk );
	int nPrevStage = pRPG->GetDestroyStage();
	int nRes = pAtk->ProcessAttack( nUserID, pAttack, pArmor );
	if ( nPrevStage != pRPG->GetDestroyStage() )
	{
		for ( NDb::CObject *pO = pDbObject; pO; pO = pO->pChild )
		{
			NDb::CContainerModel *pCont = pDbObject->pModels[pRPG->GetDestroyStage()];
			if ( pCont && IsValid( pCont->pDestroySound ) )
			{
				static SRand rnd;
				NDb::CSoundVariant *pSVar = pCont->pDestroySound->GetSound( &rnd );
				if ( IsValid( pSVar ) && IsValid( pSVar->pSound ) )
				{
					pWorld->MakeSound( position.ptPos, pSVar->pSound );
				}
			}
		}
		/*
		if ( pRPG->IsDead() )
			Kill( ptDir );
		else
		{
			pCurrentWorld->GenerateDebris( GetDebrisMaterial(), position.ptPos, VNULL3, 1 );
			//pCurrentWorld->AttachMiscObject( new C3DSound( ptPlace, NDb::GetSound(8) ) );
			tStageChange = pCurrentWorld->GetAimTime()->GetValue();
			bindGlobal.Update();
		}
		*/
		tStageChange = pCurrentWorld->GetAimTime()->GetValue();
		bindGlobal.Update();
	}
	return nRes;
/*	else if ( CDynamicCast<CWObject> pObj(pTarget) )
	{
		CObjectServer *pObjectServer = GetObject( pObj );
		if ( pObjectServer->IsValid() )
		{
			if ( pObjectServer->GetAttackable()->IsDead() )
			{
				pCurrentWorld->AddActionLocator( new CActionLocator( CActionLocator::TYPE_DIE, pObj, pObj->aiPos.pos.GetCP() ), 1000 );
				KillObject( pObjectServer, ptDir );
			}
			//else if ( pObjectServer->IsTargetable() )
			//{
				// TEMPORARILY //
				//KillObject( pObjectServer, NRPG::GetDir(ray) );
				//Explode( pObj->position.forward.GetTranslation(), 1000 );
			//}
			else
				pCurrentWorld->Add3DSound( NDb::GetSound(8),
					new NGScene::CCFBTransform( MakeTransform( ptPlace, 0 ) ), 0 );
		}
	}*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::AddEffects( IRenderVisitor *p, NDb::CContainerModel *pCont, const SFBTransform &rv )
{
	if ( !pCont )
		return;
	if ( pCont->ptPLightCr != VNULL3 )
	{
		SFBTransform m;
		CVec3 ptOrigin;
		rv.forward.RotateHVector( &ptOrigin, pCont->ptPLightPos );
		p->AddPointLight( pCont->ptPLightCr, ptOrigin, pCont->fPLightRadius, bLightMap );//pCont->xz );
	}
	if ( pCont->ptSLightCr != VNULL3 )
	{
		CVec3 ptOrigin;
		rv.forward.RotateHVector( &ptOrigin, pCont->ptSLightPos );
		p->AddSpotLight( pCont->ptSLightCr, ptOrigin, pCont->ptSLightDir, pCont->fSLightFOV, 
			pCont->fSLightRadius, pCont->pSLightMask, false );//pCont->bLightmapOnly );
	}
	if ( pCont->pEffect )
	{
		SFBTransform tr;
		tr = rv * MakeTransform( pCont->ptEffectPos, CVec3(1,1,1 ) );
		p->AddParticleEffect( tStageChange, pCont->pEffect, new NGScene::CCFBTransform(tr) );
		for ( int i = 0; i < pCont->pEffect->instances.size(); ++i )
		{
			NDb::CParticle *pParticle = pCont->pEffect->instances[i]->pParticle;
			p->AddOccluder( pParticle->pAIGeometry, rv );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::AddObject( IRenderVisitor *p, NDb::CObject *pO, const SFBTransform &rv )
{
	NDb::CContainerModel *pCont = pO->pModels[nDestroyStage];
	if ( pCont )
	{
		NDb::CModel *pModel = pCont->pModel;
		if ( pModel )
		{
			p->AddMesh( pModel, rv, 0, position.nFloor );
			if ( pModel->pGeometry && ( pModel->pRPGArmor == 0 || pModel->pRPGArmor->fTransparency < 0.01f ) )
			{
				NDb::CGeometry *pGeom = pModel->pGeometry;
				if ( pGeom->pAIGeometry )
					p->AddOccluder( pGeom->pAIGeometry, rv );
				if ( pGeom->pAIGeometry2 )
					p->AddOccluder( pGeom->pAIGeometry2, rv );
			}
		}
		AddEffects( p, pCont, rv );
	}
	if ( IsValid( pO->pChild ) )
		AddObject( p, pO->pChild, rv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::Visit( IRenderVisitor *p )
{
	nDestroyStage = pRPG->GetDestroyStage();
	SFBTransform rv;
	CreateTransform( &rv );
	AddObject( p, pDbObject, rv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetMask( NDb::CAIGeometry *pAIGeometry )
{
	if ( pAIGeometry && NDb::IsIgnored( pAIGeometry->traficability ) )
		return TS_OBJECTS|TS_FRAGMENTED|TS_VISION|TS_COVER;
	int nMask = TS_PASS_BLOCKER|TS_OBJECTS|TS_FRAGMENTED|TS_VISION|TS_COVER|TS_WEAPON_BLOCKER;
	if ( pAIGeometry && NDb::CanGoOver( pAIGeometry->traficability ) )
		nMask |= TS_GO_OVER;
	return nMask;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::AddEffects( IAIVisitor *p, NDb::CContainerModel *pCont, const SFBTransform &rv )
{
	if ( !pCont )
		return;
	NDb::CEffect *pEffect = 0;
	if ( IsValid( pCont->pEffect ) )
		pEffect = pCont->pEffect;
	if ( IsValid( pEffect ) )
	{
		for ( int i = 0; i < pEffect->instances.size(); ++i )
		{
			NDb::CParticle *pParticle = pEffect->instances[i]->pParticle;
			int nMask = GetMask( pParticle->pAIGeometry );
			p->AddHull( pParticle->pAIGeometry, rv, pParticle->pRPGArmor, position.nFloor, nMask );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::AddObject( IAIVisitor *p, NDb::CObject *pO, const SFBTransform &rv )
{
	NDb::CContainerModel *pCont = pO->pModels[nDestroyStage];
	if ( pCont )
	{
		NDb::CModel *pModel = pCont->pModel;
		if ( pModel && pModel->pGeometry )
		{
			NDb::CGeometry *pGeom = pModel->pGeometry;
			if ( pGeom->pAIGeometry )
			{
				int nMask = GetMask( pGeom->pAIGeometry ) | TS_PICK;
				p->AddHull( pGeom->pAIGeometry, rv, pModel->pRPGArmor, position.nFloor, nMask );
			}
			if ( pGeom->pAIGeometry2 )
			{
				int nMask = GetMask( pGeom->pAIGeometry2 ) | TS_PICK;
				p->AddHull( pGeom->pAIGeometry2, rv, pModel->pRPGArmor, position.nFloor, nMask );
			}
		}
		AddEffects( p, pCont, rv );
	}
	if ( IsValid( pO->pChild ) )
		AddObject( p, pO->pChild, rv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::Visit( IAIVisitor *p )
{
	nDestroyStage = pRPG->GetDestroyStage();
	SFBTransform rv;
	CreateTransform( &rv );
	AddObject( p, pDbObject, rv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::AddObject( ISoundVisitor *p, NDb::CObject *pO, const SFBTransform &rv )
{
	NDb::CContainerModel *pCont = pO->pModels[nDestroyStage];
	if ( pCont && pCont->eSoundType == NDb::ST_PERMANENT && pCont->pSound )
	{
		CVec3 ptOrigin;
		static SRand rnd;
		rv.forward.RotateHVector( &ptOrigin, pCont->ptSoundPos );
		NDb::CSoundVariant *pSVar = pCont->pSound->GetSound( &rnd );
		if ( IsValid( pSVar ) )
			p->Add3DSound( 0, pSVar->pSound, new NGScene::CCVec3( ptOrigin ) );
	}
	if ( IsValid( pO->pChild ) )
		AddObject( p, pO->pChild, rv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectServerBase::Visit( ISoundVisitor *p )
{
	SFBTransform rv;
	CreateTransform( &rv );
	AddObject( p, pDbObject, rv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObjectServerBase::Segment()
{
	NDb::CContainerModel *pCont = pDbObject->pModels[nDestroyStage];
	if ( pCont && pCont->eSoundType == NDb::ST_RANDOM && pCont->pSound )
	{
		static SRand rnd;
		STime t = pWorld->GetTime()->GetValue();
		float fInterval = float( t - tLastSound ) / 1000.0f;
		float fAvrgInterval = Max( 1.0f, pCont->fSoundAvgInterval );
		if ( fInterval > rnd.GetFloat( 0.5f * fAvrgInterval, 20.0f * fAvrgInterval ) )
		{
			char buf[256];
			sprintf( buf, "Random sound interval=%f, current=%f\n", pCont->fSoundAvgInterval, fInterval );
			//OutputDebugString( buf );
			NDb::CSoundVariant *pSVar = pCont->pSound->GetSound( &rnd );
			if ( IsValid( pSVar ) && IsValid( pWorld ) )
			{
				CVec3 ptOrigin;
				SFBTransform rv;
				CreateTransform( &rv );
				rv.forward.RotateHVector( &ptOrigin, pCont->ptSoundPos );
				pWorld->MakeSound( ptOrigin, pSVar->pSound );
			}
			tLastSound = t;
		}
	}
	return !NeedSegment();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAnimObjectServerBase
////////////////////////////////////////////////////////////////////////////////////////////////////
CAnimObjectServerBase::CAnimObjectServerBase( CWorld *pWorld, const SObjectPlace &pos, bool bLightMap,
	NDb::CObject *pO, NRPG::IObject *pRPG, CFuncBase<STime> *_pTime ) :
	CObjectServerBase( pWorld, pos, bLightMap, pO, pRPG ), pTime(_pTime)
{
	NDb::CContainerModel *pCont = pDbObject->pModels[nDestroyStage];
	ASSERT(pCont);
	ASSERT( IsValid( pCont->pModel ) );
	ASSERT( IsValid( pCont->pModel->pSkeleton ) );
	pSkeleton = pCont->pModel->pSkeleton;
	pAnimator = new NAnimation::CSkeletonAnimator( pSkeleton );
	pAnimator->pTime = pTime;
	pState = new NAnimation::CSkeletonState;
	pState->pAnimator = pAnimator;
	pState->pTime = _pTime;
	pState->state.nAnimFlags = 0;
	pState->state.pos = pos.ptPos;
	pState->state.fAngle = pos.fAngle;
	pState->state.cIdleBannedFlags = 0;

	STime t = pTime->GetValue();
	CPtr<NAnimation::CAnimation> pAnim = pAnimator->CreateAnimation(
		pSkeleton->GetAnimation( NDb::CAnimation::POSE, 0 ), t );
	if ( pAnim )
	{
		pAnim->SetStand( t, pos.ptPos, pos.fAngle );
		pAnimator->AddAnimator( t, pAnim );
		pAnimator->AddMemorizer( t );
	}
	IdleOn();
	bBroken = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAnimObjectServerBase::IdleOn()
{
	pState->state.cIdleBannedFlags &= ~NAnimation::E_INTERNAL_IDLE_OFF;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAnimObjectServerBase::IdleOff()
{
	pState->state.cIdleBannedFlags |= NAnimation::E_INTERNAL_IDLE_OFF;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAnimObjectServerBase::GetApproaches( vector<NAI::SPathPlace> *pRes, NAI::IPathNetwork *pNet ) const
{
	pRes->clear();
	char pszName[16];
	vector<NAI::SPathPlace> tmp;
	for ( int i = 1; i < 10; ++i )
	{
		sprintf( pszName, "Unit%d", i );
		int nIndex = pAnimator->GetBoneIndex( pszName );
		if ( nIndex < 0 )
			break;
		NAnimation::SBonePose bone;
		pAnimator->GetDefaultBonePos( nIndex, &bone );
		CQuat rot( position.fAngle, CVec3(0,0,1) );
		bone.pos = rot.Rotate(bone.pos) + position.ptPos;
		bone.rot = rot * bone.rot;
		CVec3 axis = bone.rot.GetXAxis();
		float fAngle = atan2( axis.y, axis.x );
		vector<NAI::SPathPlace> places;
		pNet->GetNearPlaces( SSphere( bone.pos, FP_GRID_STEP ), &places );
		float fMin;
		int nMin = -1;
		NAI::SPathPlace res;
		for ( int k = 0; k < places.size(); ++k )
		{
			NAI::SPathPlace p;
			p = places[k];
			p.SetPose( NAI::CM_STAND );
			p.SetDirection( pNet->GetClosestDir( p.GetLayer(), fAngle ) );
			if ( !pNet->IsNativePassable( p ) )
				continue;
			NAI::SPosition pos;
			pos.SetNetwork( pNet );
			pos.p = p;
			float fDist = fabs2( pos.GetCP() - bone.pos );
			if ( nMin < 0 || fDist < fMin )
			{
				fMin = fDist;
				nMin = k;
				res = p;
			}
		}
		if ( nMin >= 0 )
			tmp.push_back( res );
	}
	for ( int i = 0; i < tmp.size(); ++i )
	{
		pRes->push_back( tmp[i] );
		NAI::SPathPlace p = tmp[i];
		p.SetPose( NAI::CM_CROUCH );
		if ( pNet->IsNativePassable( p ) )
			pRes->push_back( p );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAnimObjectServerBase::Visit( IRenderVisitor *p )
{
	nDestroyStage = pRPG->GetDestroyStage();
	SFBTransform rv;
	CreateTransform( &rv );

	NDb::CContainerModel *pCont = pDbObject->pModels[nDestroyStage];
	if ( pCont )
	{
		NDb::CModel *pModel = pCont->pModel;
		if ( pModel )
		{
			if ( !bBroken && pModel->pSkeleton == pSkeleton )
			{
				vector<IRenderVisitor::SBoundMesh> boundMeshes;
				p->AddMesh( pModel, pAnimator, pState, boundMeshes, 0, position.nFloor );
				PlayDestructAnimation();
				if ( pModel->pGeometry && pModel->pRPGArmor->fTransparency < 0.01f )
				{
					NDb::CGeometry *pGeom = pModel->pGeometry;
					if ( pGeom->pAIGeometry )
						p->AddOccluder( pGeom->pAIGeometry, pModel->pSkeleton, pAnimator );
					if ( pGeom->pAIGeometry2 )
						p->AddOccluder( pGeom->pAIGeometry2, pModel->pSkeleton, pAnimator );
				}
			}
			else
			{
				bBroken = true;
				p->AddMesh( pModel, rv, 0, position.nFloor );
			}
		}
		AddEffects( p, pCont, rv );
	}
	
	if ( IsValid( pDbObject->pChild ) )
		AddObject( p, pDbObject->pChild, rv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAnimObjectServerBase::Visit( IAIVisitor *p )
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
			/*if ( !bBroken && pModel->pSkeleton == pSkeleton )
				p->AddAnimatedHull( pModel->pGeometry->pAIGeometry, pSkeleton, pAnimator, pModel->pRPGArmor, position.nFloor, nMask );
			else
			{
				bBroken = true;
				p->AddHull( pModel->pGeometry->pAIGeometry, rv, pModel->pRPGArmor, position.nFloor, nMask );
			}*/
			if ( pModel->pSkeleton != pSkeleton )
				bBroken = true;
			p->AddHull( pModel->pGeometry->pAIGeometry, rv, pModel->pRPGArmor, position.nFloor, nMask );
		}
		AddEffects( p, pCont, rv );
	}
	
	if ( IsValid( pDbObject->pChild ) )
		AddObject( p, pDbObject->pChild, rv );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAnimObjectServerBase::Segment()
{
	STime tCur = pTime->GetValue();
	if ( tCur > tEnd && IsValid( pAction ) )
		pAction = 0;
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAnimObjectServerBase::PlayDestructAnimation()
{
	NDb::CAnimation::EType type;
	switch ( nDestroyStage )
	{
		case 1: type = NDb::CAnimation::DESTRUCT_1; break;
		case 2: type = NDb::CAnimation::DESTRUCT_2; break;
		case 3: type = NDb::CAnimation::DESTRUCT_3; break;
		case 4: type = NDb::CAnimation::DESTRUCT_4; break;
		default: return;
	}
	IdleOff();
	STime t = pTime->GetValue();
	CPtr<NAnimation::CAnimation> pAnim = pAnimator->CreateAnimation(
		pSkeleton->GetAnimation( type, 0 ), t );
	if ( pAnim )
	{
		pAnim->SetStand( t, position.ptPos, position.fAngle );
		pAnimator->AddAnimator( t, pAnim );
		pAnimator->AddMemorizer( t + pAnim->GetTime() );
		tEnd = t + pAnim->GetTime();
		pAction = pWorld->GetActiveCounter();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
