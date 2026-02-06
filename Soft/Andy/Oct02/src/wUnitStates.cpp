#include "StdAfx.h"
#include "wUnitMove.h"
#include "wUnitServer.h"
#include "wUnitExec.h"
#include "RPGUnitMission.h"
#include "wMain.h"
#include "RPGItemSet.h" // CRAP
#include "wUnitAttack.h"
#include "wObject.h"
#include "..\DBFormat\DataRPG.h"
#include "..\misc\RandomGen.h"
#include "..\Misc\LogStream.h"
#include "wAckBase.h"
#include "RPGCritical.h"
#include "time.h"
#include "RPGGame.h"
#include "aiPosition.h"
#include "RPGGlobal.h"

#include "wUnitStates.h"

namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitState
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitState::DisableCriticals()
{
	ASSERT( pUS );
	pUS->GetUnitRPG()->EnableCriticals();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateNormal
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitStateNormal::IsInactive() const
{
	return pUS->GetPosition().pos.p.GetPose() == NAI::CM_INACTIVE;		
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateNormal::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult )
{
	*pResult = UCR_UNAVAILABLE;

	if ( IsInactive() )
	{
		if ( CDynamicCast<CCmdReload> p(pCmd ) )
			return 0;
		if ( CDynamicCast<CCmdSetActiveItem> p(pCmd ) )
			return 0;
		if ( CDynamicCast<CCmdMoveInventoryItem> p(pCmd ) )
			return 0;
	}
	else
	{
		if ( CDynamicCast<CCmdDropCorpse> p(pCmd ) )
			return 0;
		if ( CDynamicCast<CCmdExitCannon> p(pCmd ) )
			return 0;
	}

	if ( CDynamicCast<CCmdSnipeAttack> p(pCmd ) )
		return 0;

	return NWorld::CreateExecutor( pUS, pCmd, pResult );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateNormal::ProcessCritical( NDb::ECritical eCA )
{
	CCommandExecute *p = 0;
	switch ( eCA )
	{
		case NDb::C_MOTIONLESS:
			{
				CPtr<NAI::CPath> pPath;
				EUnitCommandResult eResult;
				vector<NAI::SPathPlace> dst;
				NAI::SPathPlace src( pUS->GetPosition().pos.p ), dstp;
				dstp = src;
				dstp.SetPose( NAI::CROUCH );
				dst.push_back( dstp );
				pPath = pUS->GetWorld()->FindPath( pUS, src, dst, 0, false, NAI::PF_USE_POSEDIR );
				if ( IsValid( pPath ) )
					p = CreateMoveExecutor( pUS, pPath, NAI::PF_USE_POSEDIR, ITEM_NO_MATTER, &eResult );
			}
			break;
		case NDb::C_LOST_WEAPON:
			p = CreateLostWeapon( pUS, false );
			break;
		case NDb::C_IDLE_HAND:
			p = CreateLostWeapon( pUS, true );
			break;
		case NDb::C_ACCIDENTAL_SHOT:
			p = CreateAccidentalShot( pUS );
			break;
		case NDb::C_DAMAGE_WEAPON:
			break;
		case NDb::C_BLIND:
			break;
		case NDb::C_STUN:
			pUS->SetState( new CUnitStateStun( pUS ) );
			break;
		default:
			ASSERT(0);
			break;
	}

	if ( p )
		pUS->RunCriticalExecutor( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateNormal::DisableCriticals()
{
	pUS->GetUnitRPG()->EnableCriticals();

	if ( IsInactive() )
	{
		pUS->GetUnitRPG()->DisableCritical( NDb::C_STUN, NRPG::CS_REROLL );
		pUS->GetUnitRPG()->DisableCritical( NDb::C_MOTIONLESS, NRPG::CS_REROLL );
		pUS->GetUnitRPG()->DisableCritical( NDb::C_ACCIDENTAL_SHOT, NRPG::CS_REROLL );
		pUS->GetUnitRPG()->DisableCritical( NDb::C_DAMAGE_WEAPON, NRPG::CS_REROLL );
		pUS->GetUnitRPG()->DisableCritical( NDb::C_LOST_WEAPON, NRPG::CS_REROLL );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateSniping
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitStateSniping::CUnitStateSniping( CUnitServer *_pUS, CUnitServer *_pTarget, int _nBaseAP ): 
 CUnitState(_pUS), pTarget(_pTarget), nBaseAP(_nBaseAP), nSnipeAP(0)
{
	TargetPosition = pTarget->GetPosition();
	InitialTargetPosition = pTarget->GetPosition();
	pUS->GetUnitRPG()->SaveAP( NRPG::SSnipeAP( nSnipeAP, pTarget->GetUnitRPG() ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitStateSniping::CheckTarget()
{
	bool bRes = true;
	//
	bRes = bRes && !pTarget->IsDead() && !pTarget->IsUnconscious();
	// проверяем видимость цели
	bRes = bRes && pUS->GetWorld()->GetGame()->CheckVisibility( pUS, pTarget );
	// проверяем угол на который переместилась цель
	CVec3 ptInitialDir = ( InitialTargetPosition.GetCenter() - pUS->GetPosition().GetCenter() );
	CVec3 ptCurrentDir = ( pTarget->GetPosition().GetCenter() - pUS->GetPosition().GetCenter() );
	Normalize( &ptInitialDir );
	Normalize( &ptCurrentDir );
	float fAngle = acos( ptInitialDir * ptCurrentDir );
	bRes = bRes && (fAngle <= PI/8);
	//
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateSniping::CollectSnipeAP( int _nAP )
{ 
	NRPG::SUnitInfo info;
	pUS->GetUnitRPG()->GetInfo( NAI::WALK, &info );
	if ( info.nAP > 0 )
	{
		pUS->GetUnitRPG()->SpendAP( _nAP );
		nSnipeAP += _nAP;
		pUS->GetUnitRPG()->SaveAP( NRPG::SSnipeAP( nSnipeAP, pTarget->GetUnitRPG() ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateSniping::Segment()
{
	if ( TargetPosition.GetCenter() != pTarget->GetPosition().GetCenter() )
	{
		if ( !CheckTarget() )
			CancelSnipe();
		else
		{
//			pUS->Do( new CCmdSetCommand( pUS, new CCmdSnipeAim( pTarget ) ) );
//			pUS->Do( new CCmdSetCommand( pUS, new CCmdContinue() ) );
			TargetPosition = pTarget->GetPosition();
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateSniping::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult ) 
{
	*pResult = UCR_OK;

	if ( CDynamicCast<CCmdCollectSnipeAP> pCollect(pCmd) )
		return NWorld::CreateExecutor( pUS, pCmd, pResult );
	else if ( CDynamicCast<CCmdSnipeAttack> pSnipeAttack(pCmd) )
		return NWorld::CreateExecutor( pUS, new CCmdShootObject( pTarget, 0 ), pResult );
	else if ( CDynamicCast<CCmdShootObject> pShootObject(pCmd) )
	{
		if ( IsValid( pShootObject->pTarget ) && ( dynamic_cast<CUnitServer*>( pShootObject->pTarget.GetPtr() ) == pTarget ) )
			return NWorld::CreateExecutor( pUS, pShootObject, pResult );
	}

	*pResult = UCR_UNAVAILABLE;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateSniping::ProcessCritical( NDb::ECritical eCA ) 
{
	pUS->PostponeCritical( eCA );
		CancelSnipe();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateSniping::CancelSnipe()
{
	pUS->GetUnitRPG()->SaveAP( NRPG::SSnipeAP( 0, 0 ) );
	// сбрасываем состояние
	pUS->SetState( new CUnitStateNormal( pUS ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateUsingCannon
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateUsingCannon::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult ) 
{ 
	*pResult = UCR_OK;

	if ( CDynamicCast<CCmdShootTile> p(pCmd ) )
		return NWorld::CreateExecutor( pUS, pCmd, pResult );
	if ( CDynamicCast<CCmdShootObject> p(pCmd ) )
		return NWorld::CreateExecutor( pUS, pCmd, pResult );
	if ( CDynamicCast<CCmdReload> p(pCmd ) )
		return NWorld::CreateExecutor( pUS, pCmd, pResult );
	if ( CDynamicCast<CCmdExitCannon> p(pCmd ) )
	{
		p->pCannon = pCannon;
		return NWorld::CreateExecutor( pUS, p, pResult );
	}

	*pResult = UCR_UNAVAILABLE;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateUsingCannon::ProcessCritical( NDb::ECritical eCA ) 
{
	EUnitCommandResult eResult;
	switch ( eCA )
	{
		case NDb::C_DEATH:
		case NDb::C_BLIND:
		case NDb::C_DEAF:
		case NDb::C_AP_REDUCTION:
		case NDb::C_ENCUMBRANCE:
		case NDb::C_DAMAGE_WEAPON:
		case NDb::C_WEAPONSKILL_REDUCTION:
			break;
		case NDb::C_ACCIDENTAL_SHOT:
			pUS->RunCriticalExecutor( CreateAccidentalShot( pUS ) );
			break;
		default:
			pUS->PostponeCritical( eCA );
			pUS->RunCriticalExecutor( NWorld::CreateExecutor( pUS, new CCmdExitCannon( pCannon ), &eResult ) );
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateUsingCannon::OnActionFinish()
{
	EUnitCommandResult eResult;
	// check if cannon is destroyed or unusable
	if ( !IsValid( pCannon ) || pCannon->IsBroken() )
		pUS->RunCriticalExecutor( NWorld::CreateExecutor( pUS, new CCmdExitCannon( pCannon ), &eResult ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateUsingCannon::OnDeath()
{
	pUS->GetUnitRPG()->SetCannonItem(0);
	pCannon->SetCurrentUnit(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateCorpseCarrier
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitStateCorpseCarrier::CUnitStateCorpseCarrier( CUnitServer *_pUS, CUnitServer *_pDead ): 
	CUnitState(_pUS), pDeadUnit(_pDead)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateCorpseCarrier::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult ) 
{ 
	*pResult = UCR_OK;

	if ( CDynamicCast<CCmdPath> p(pCmd ) )
		return NWorld::CreateExecutor( pUS, pCmd, pResult );
	if ( CDynamicCast<CCmdWishPose> p(pCmd ) )
	{
		if ( p->pose == NAI::WALK)
			return NWorld::CreateExecutor( pUS, pCmd, pResult );
		else
			return 0;
	}
	if ( CDynamicCast<CCmdDropCorpse> p(pCmd ) )
	{
		p->pCorpse = pDeadUnit;
		return NWorld::CreateExecutor( pUS, p, pResult );
	}

	*pResult = UCR_UNAVAILABLE;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateCorpseCarrier::OnStateStarted()
{
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer = pUS->GetPlayer()->GetGlobalPlayer();
	if ( IsValid( pGlobalPlayer ) )
	{
		CPtr<NRPG::CUnit> pCarrier = pUS->GetUnitRPG()->GetRPGUnit();
		CPtr<NRPG::CUnit> pCorpse = pDeadUnit->GetUnitRPG()->GetRPGUnit();
		//
		if ( !pDeadUnit->IsDead() )
		{
			pGlobalPlayer->MarkUnitAsAlive( pCorpse );
			if ( pUS->GetPlayer() == pDeadUnit->GetPlayer() )
				pGlobalPlayer->RescueUnit( pCarrier, pCorpse );
			else
				pGlobalPlayer->CaptureUnit( pCarrier,	pCorpse );
		}
		else
			pGlobalPlayer->TakeUnitCorpse( pCarrier, pCorpse );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateCorpseCarrier::OnStateFinished()
{
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer = pUS->GetPlayer()->GetGlobalPlayer();
	if ( IsValid( pGlobalPlayer ) )
	{
		pGlobalPlayer->FreeUnit( pUS->GetUnitRPG()->GetRPGUnit() );
		pGlobalPlayer->MarkUnitAsDead( pDeadUnit->GetUnitRPG()->GetRPGUnit() );
	}
	pDeadUnit->animator.BeDropped();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateCorpseCarrier::ProcessCritical( NDb::ECritical eCA ) 
{
	switch ( eCA )
	{
		case NDb::C_DEATH:
		case NDb::C_BLIND:
		case NDb::C_DEAF:
		case NDb::C_AP_REDUCTION:
		case NDb::C_WEAPONSKILL_REDUCTION:
			break;
		default:
			pUS->PostponeCritical( eCA );
			pUS->SetState( new CUnitStateNormal( pUS ) );
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateHealer
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::DoHealing( int nSpentAP )
{
	ASSERT( IsValid( pTarget ) );
	ASSERT( nRequiredAP > 0 );
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	if ( IsValid( pTarget ) && nRequiredAP > 0 )
	{
//		csRPG << CC_WHITE << "FirstAid " << pRPG->GetName() << " \n{\n";
		NRPG::SFirstAid fa;
		pRPG->CreateFirstAid( &fa, nSpentAP );
		nRequiredAP -= nSpentAP;
		int ndTmpVP = (int)fdVPFraction;
		fa.fdVP += ndTmpVP;
		pTarget->GetUnitRPG()->Heal( fa );
		fdVPFraction += fa.fdVP - (int)fa.fdVP - ndTmpVP;
//		csRPG << "}\n";
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::SayAck()
{
	NRPG::SUnitInfo Info;
	pTarget->GetUnitRPG()->GetInfo( NAI::WALK, &Info );
	if ( Info.nAP < Info.nMaxAP )
		pUS->GetWorld()->GetGlobalAck()->OnCannotFinishHeal( pUS, pTarget );
	else
		pUS->GetWorld()->GetGlobalAck()->OnHealFinished( pUS, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::Cancel()
{
	FinishHealing();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::FinishHealing()
{
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	NRPG::IInventory *pInventory = pRPG->GetInventory();
	CDynamicCast<NRPG::CFirstAidItem> pFA( pRPG->GetInventory()->GetActive() );
	pFA->SpendPotion();
	if ( pFA->IsEmpty() )
	{
		// лечение окончено и аптечки закончились, удаляем итем
		CObj<NRPG::IInventoryItem> pErase = pInventory->TakeOff( (NDb::ESlot)pInventory->GetActiveSlot() );
		pUS->Update();
	}
	pUS->animator.FinishHealing( pUS->GetPosition() );
	pTarget->GetUnitRPG()->RemoveCritical( NDb::C_PATIENT );
	SayAck();

	pUS->SetState( new CUnitStateNormal( pUS ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitStateHealer::StartHeal( CUnitServer *_pTarget )
{
	pTarget = _pTarget;
	NRPG::IUnitMission *pRPG = pUS->GetUnitRPG();
	pTarget->GetUnitRPG()->ApplyCritical( NRPG::SCritical( NDb::CL_ANY, NDb::C_PATIENT ) );
	//
	fdVPFraction = 0;
	NRPG::SUnitInfo Info;
	NRPG::SFirstAid TmpFirstAid;
	pUS->GetUnitRPG()->CreateFirstAid( &TmpFirstAid, 0 );
	pTarget->GetUnitRPG()->GetInfo( NAI::WALK, &Info );
	int nTargetVPToHeal = Info.nMaxHP - Info.nHP;
	int nTargetHealedVP = Info.nHealedHP;
	int nVPToHeal = max( 0, TmpFirstAid.nMaxVP - nTargetHealedVP );
	nVPToHeal = min( nVPToHeal, nTargetVPToHeal );
	if ( nVPToHeal > 0 )
		nRequiredAP = 60.f / pUS->GetUnitRPG()->GetRPGUnit()->Skills( NDb::ST_MEDICINE ) * nVPToHeal;
	else
	{
		FinishHealing();
		return true;
	}
	//
	if ( !pUS->GetWorld()->IsRealTime() )
	{
		int nAP = Min( pUS->GetAP(), nRequiredAP );
		pUS->SpendAP( nAP );
		DoHealing( nAP );
		bNewSegment = false;
		if ( nRequiredAP <= 0 )
			FinishHealing();
	}
		else
	nPrevTime = pUS->GetWorld()->GetTime()->GetValue();
	//
	return nRequiredAP == 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::OnFinishOwnTurn()
{
	if ( bNewSegment )
	{
		int nAP = Min( pUS->GetAP(), nRequiredAP );
		pUS->SpendAP( nAP );
		DoHealing( nAP );
	}
	bNewSegment = true;
	if ( nRequiredAP <= 0 )
		FinishHealing();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::OnStartRealTime()
{
	nPrevTime = pUS->GetWorld()->GetTime()->GetValue();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::OnDeath() 
{
	FinishHealing();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::Segment()
{
	CPtr<NWorld::CWorld> pWorld = pUS->GetWorld();
	if ( pWorld->IsRealTime() )
	{
		int nTmpTime = pUS->GetWorld()->GetTime()->GetValue();
		if ( nTmpTime - nPrevTime >= 100 )
		{
			DoHealing( 1 );
			nPrevTime = nTmpTime;
			if ( nRequiredAP <= 0 )
				FinishHealing();
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateHealer::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult ) 
{
	// all commands are ignored, need cmdStopHealing
	*pResult = UCR_UNAVAILABLE;
	return 0; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::ProcessCritical( NDb::ECritical eCA ) 
{
	switch ( eCA )
	{
		case NDb::C_DEATH:
		case NDb::C_DEAF:
		case NDb::C_AP_REDUCTION:
		case NDb::C_MOTIONLESS:
		case NDb::C_ENCUMBRANCE:
		case NDb::C_WEAPONSKILL_REDUCTION:
			break;
		case NDb::C_ACCIDENTAL_SHOT:
			FinishHealing();
			// наносим небольшие повреждения ( 4 d4 )
			for ( int i = 0; i < 4; ++i)
				pTarget->GetUnitRPG()->MakeDirectDamage( random.Get( 1, 4 ) );
			break;
		default:
			pUS->PostponeCritical( eCA );
			FinishHealing();
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateHealer::OnActionFinish()
{
	FinishHealing();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateEngineering
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateEngineering::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult ) 
{
	*pResult = UCR_UNAVAILABLE;
	return 0; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateEngineering::ProcessCritical( NDb::ECritical eCA ) 
{
	switch ( eCA )
	{
		case NDb::C_DEATH:
		case NDb::C_DEAF:
		case NDb::C_AP_REDUCTION:
		case NDb::C_MOTIONLESS:
		case NDb::C_ENCUMBRANCE:
		case NDb::C_WEAPONSKILL_REDUCTION:
			break;
		case NDb::C_ACCIDENTAL_SHOT:
			// взрываем ловушку
			// переходим в нормальное состояние
			pUS->SetState( new CUnitStateNormal( pUS ) );			
			break;
		default:
			pUS->PostponeCritical( eCA );
			pUS->SetState( new CUnitStateNormal( pUS ) );			
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateStun
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateStun::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult ) 
{ 
	*pResult = UCR_UNAVAILABLE;
	return 0; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateStun::ProcessCritical( NDb::ECritical eCA ) 
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateStun::OnActionFinish()
{
	if ( !pUS->GetUnitRPG()->HasCritical( NDb::C_STUN ) )
		pUS->SetState( new CUnitStateNormal( pUS ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateDeath
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateDeath::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult ) 
{ 
	*pResult = UCR_UNAVAILABLE;
	return 0; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateDeath::ProcessCritical( NDb::ECritical eCA ) 
{
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateDeath::DisableCriticals()
{
	pUS->GetUnitRPG()->DisableCriticals();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateDeath::OnStateStarted()
{
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer =	pUS->GetPlayer()->GetGlobalPlayer();
	if ( IsValid( pGlobalPlayer ) )
		pGlobalPlayer->MarkUnitAsDead( pUS->GetUnitRPG()->GetRPGUnit() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitStateUnconscious
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitStateUnconscious::CUnitStateUnconscious( CUnitServer *_pUS ):
	CUnitState(_pUS)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommandExecute* CUnitStateUnconscious::CreateExecutor( CCmd *pCmd, EUnitCommandResult *pResult )
{
	*pResult = UCR_UNAVAILABLE;
	return 0; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateUnconscious::ProcessCritical( NDb::ECritical eCA )
{
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateUnconscious::DisableCriticals()
{
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitStateUnconscious::OnStateStarted()
{
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer =	pUS->GetPlayer()->GetGlobalPlayer();
	if ( IsValid( pGlobalPlayer ) )
		pGlobalPlayer->MarkUnitAsDead( pUS->GetUnitRPG()->GetRPGUnit() );
	//
	pUS->GetWorld()->UnitWasKilled( pUS ); //CRAP
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NWorld;
BASIC_REGISTER_CLASS( CUnitState )
REGISTER_SAVELOAD_CLASS( 0x01822160, CUnitStateNormal )
REGISTER_SAVELOAD_CLASS( 0x01822161, CUnitStateSniping )
REGISTER_SAVELOAD_CLASS( 0x01822162, CUnitStateUsingCannon )
REGISTER_SAVELOAD_CLASS( 0x01822163, CUnitStateCorpseCarrier )
REGISTER_SAVELOAD_CLASS( 0x01822164, CUnitStateHealer )
REGISTER_SAVELOAD_CLASS( 0x01822165, CUnitStateEngineering )
REGISTER_SAVELOAD_CLASS( 0x01822166, CUnitStateDeath )
REGISTER_SAVELOAD_CLASS( 0x01822167, CUnitStateStun )
REGISTER_SAVELOAD_CLASS( 0x52322110, CUnitStateUnconscious )