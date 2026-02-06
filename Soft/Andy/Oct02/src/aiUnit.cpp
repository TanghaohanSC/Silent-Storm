#include "StdAfx.h"

#include "aiLog.h"
#include "aiMap.h"
#include "aiState.h"
#include "aiPlayer.h"
#include "aiWeapon.h"
#include "aiPosition.h"
#include "aiCriterion.h"
#include "aiInventory.h"
#include "aiScaleConverter.h"
#include "aiControl.h"

#include "RPGGame.h"
#include "RPGItem.h"
#include "RPGToHit.h"
#include "RPGItemSet.h"
#include "RPGUnitInfo.h"
#include "RPGItemInfo.h"
#include "RPGUnitMission.h"
#include "RPGAttackMech.h"
#include "RPGUnit.h"

#include "..\DBFormat\DataRPG.h"

#include "wMain.h"
#include "wUnitServer.h"
#include "wUnitAttack.h"

#include "aiUnit.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIUnit: public IAIUnit
{
	OBJECT_BASIC_METHODS( CAIUnit );
	ZDATA
	SPosition ptPosition;
	int nAP, nMaxAP, nHP, nMaxHP;
	CPtr<NWorld::CUnitServer> pUnitServer;
	CPtr<NWorld::CCannon> pCannon;
	SPosition LastSeenEnemyPosition;
	CObj<IAIInventory> pInventory;
	CPtr<IAIUnit> pCurrentEnemy;
	SPosition ptPrevPosition;
	int nTurnStartHP;
	CPtr<IAIUnit> pLastSeenEnemy;
	int nHurtHP; // повреждения нанесенные нами за данный ход
	vector< CObj<IAIControl> > Controls;
	bool bUnderAIControl;
	int nAdditionalExpediency;
	int nMaxToHit;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&ptPosition); f.Add(3,&nAP); f.Add(4,&nMaxAP); f.Add(5,&nHP); f.Add(6,&nMaxHP); f.Add(7,&pUnitServer); f.Add(8,&pCannon); f.Add(9,&LastSeenEnemyPosition); f.Add(10,&pInventory); f.Add(11,&pCurrentEnemy); f.Add(12,&ptPrevPosition); f.Add(13,&nTurnStartHP); f.Add(14,&pLastSeenEnemy); f.Add(15,&nHurtHP); f.Add(16,&Controls); f.Add(17,&bUnderAIControl); f.Add(18,&nAdditionalExpediency); f.Add(19,&nMaxToHit); return 0; }
	//
	void GetUnitSkillValues();
public:
	CAIUnit() {}
	CAIUnit( NWorld::CUnitServer *_pUnitServer, bool _bUnderAIControl ); 
	// IAIUnit
	virtual NWorld::CUnitServer *GetUnitServer() { return pUnitServer; }
	virtual SPosition GetPosition() { return ptPosition; } 
	virtual SUnitPosition GetUnitPosition();
	virtual void SetPosition( SPosition _ptPrevPosition );
	virtual void SetPosition( SPathPlace _ptPrevPosition );
	virtual SPosition GetPrevPosition();
	virtual void SavePrevPosition();
	virtual void GetHP( int *_nHP, int *_nMaxHP )
	{ *_nHP = nHP; *_nMaxHP = nMaxHP; }
	virtual int GetHP() { return nHP; }
	virtual void SetHP( int _nHP, int _nMaxHP );
	virtual void GetAP( int *_nAP, int *_nMaxAP)
	{ *_nAP = nAP; *_nMaxAP = nMaxAP; }
	virtual int GetAP() { return nAP; }
	virtual void SetAP( int _nAP, int _nMaxAP );
	virtual void SpendHP( int _nHP );
	virtual void SpendAP( int _nAP );
	virtual void Synchronize();
	virtual bool IsDead() { return nHP <= 0; }
	virtual int GetRemainAP() { return Max( 0, nMaxAP - nAP ); }
	virtual int GetToHit( IAIUnit *pTarget );
	virtual void SetPose( int pose );
	virtual bool IsPerformingAction() { return pUnitServer->IsPerformingAction(); }
	virtual bool IsUsingCannon() { return ( pCannon != 0 ); }
	virtual void SetCannon( NWorld::CCannon *_pCannon ) { pCannon = _pCannon; }
	virtual NWorld::CCannon *GetCannon() { return pCannon; }
	virtual IAIUnit *GetCurrentEnemy() { return pCurrentEnemy; }
	virtual void SetCurrentEnemy( IAIUnit *_pCurrentEnemy );
	virtual bool IsMovedThisTurn() { return GetUnitServer()->GetUnitRPG()->GetMoveInLastTurn(); }
	virtual void OnTurnStarted() {}
	virtual void GetLastSeenEnemy( SPosition *Position, IAIUnit **ppAIUnit );
	virtual void SetLastSeenEnemy( IAIUnit *pAIUnit );
	virtual IAIInventory *GetAIInventory() { return pInventory; }
	virtual int GetTurnStartHP() { return nTurnStartHP; }
	virtual int GetHurtHP() { return nHurtHP; }
	virtual void SetHurtHP( int _nHurtHP ) { nHurtHP = _nHurtHP; }
	virtual int GetCoverForFixedUnit( NWorld::CUnitServer *pTarget, 
		NRPG::CWeaponItem *pWeaponItem, NAI::EHitLocation HitLocation );
	virtual bool HasInactivePose();
	virtual void AssignControl( IAIControl *pAIControl );
	virtual void OnControlFinished();
	virtual bool IsUnderAIControl() { return bUnderAIControl; }
	virtual int GetAdditionalExpediency() { return nAdditionalExpediency; }
	virtual void SetAdditionalExpediency( int nExpediency ) { nAdditionalExpediency = nExpediency; }
	virtual int GetMaxAP() { return nMaxAP; }
	virtual int GetMaxHP()  { return nMaxHP; }
	virtual void SetMaxToHit( int _nMaxToHit ) { nMaxToHit = _nMaxToHit; }
	virtual int GetMaxToHit() { return nMaxToHit; }
	virtual bool HasVisibleEnemies();
	virtual void DebugOutput();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIUnit::CAIUnit( NWorld::CUnitServer *_pUnitServer, bool _bUnderAIControl ) : 
	pUnitServer(_pUnitServer), pCannon(0), pLastSeenEnemy( 0 ), bUnderAIControl( _bUnderAIControl )
{ 
	GetUnitSkillValues();
	ptPosition = pUnitServer->GetPosition().pos; 
	pInventory = CreateAIInventory( this );
	pUnitServer->GetRPG()->PrintLog( false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::GetUnitSkillValues()
{
	CPtr<NRPG::CUnit> pRPGUnit = GetUnitServer()->GetUnitRPG()->GetRPGUnit();
	SetAP( pRPGUnit->Skills(NDb::ST_AP), pRPGUnit->Skills(NDb::ST_AP).GetCurrentMaxValue() );
	SetHP( pRPGUnit->Skills(NDb::ST_VP), pRPGUnit->Skills(NDb::ST_VP).GetCurrentMaxValue() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::DebugOutput()
{
	OutputDebugString( "[AI UNIT] [\n" );
	for ( vector< CObj<IAIControl> >::reverse_iterator i = Controls.rbegin(); i != Controls.rend(); ++i )
		(*i)->DebugOutput();
	OutputDebugString( "[AI UNIT] ]\n" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIUnit::HasVisibleEnemies()
{
	const list< CPtr<NWorld::CUnitServer> > &units = GetUnitServer()->GetTBSVisible();
	for ( list< CPtr<NWorld::CUnitServer> >::const_iterator i = units.begin(); i != units.end(); ++i )
		if ( !(*i)->IsDead() && 
			!(*i)->IsUnconscious() && (*i)->GetPlayer() != GetUnitServer()->GetPlayer() )
			return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SetCurrentEnemy( IAIUnit *_pCurrentEnemy ) 
{
	pCurrentEnemy = _pCurrentEnemy; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::AssignControl( IAIControl *pAIControl )
{
	CPtr<IAIControl> pHolder = pAIControl;
	//
	ASSERT( IsValid( pAIControl ) );
	//
	if ( !IsUnderAIControl() )
		return;
	//
	if ( !Controls.empty() )
	{
		CPtr<IAIControl> pAICurrentControl = Controls.back();
		if ( pAICurrentControl->GetType() == AI_CONTROL_UNINTERRUPTABLE )
			return;
		else 
		{
			pAICurrentControl->DeActivate();
			if ( pAICurrentControl->GetType() == AI_CONTROL_ERASABLE )
				Controls.pop_back();
		}
	}
	//
	GetUnitServer()->Do( new NWorld::CCmdCancel( GetUnitServer() ) );
	Controls.push_back( pAIControl );
	pAIControl->Activate();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::OnControlFinished()
{
	if ( !IsUnderAIControl() )
		return;
	//
	if ( !Controls.empty() )
	{
		Controls.back()->DeActivate();
		Controls.pop_back();
	}
	if ( !Controls.empty() )
		Controls.back()->Activate();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIUnit::GetCoverForFixedUnit( NWorld::CUnitServer *pTarget, 
	NRPG::CWeaponItem *pWeaponItem, NAI::EHitLocation HitLocation )
{
	int nHitCover = 0;
	NRPG::CAttackPortion AttackPortion;
	if ( IsValid( pWeaponItem ) )
	{
		vector<NRPG::CAttackPortion> AttackPortions;
		pWeaponItem->CreateNewAttackPortion( &AttackPortions, false );
		//
		ASSERT( AttackPortions.size() > 0 );
		if ( AttackPortions.size() == 0 )
			return nHitCover;
		//
		AttackPortion = AttackPortions.front();
	} 
	else
		AttackPortion = GetAIInventory()->GetAttackPortion();
	//
	nHitCover = GetUnitServer()->GetWorld()->GetGame()->GetCoverForAIUnit( GetUnitPosition().GetEyePosition(),
		GetUnitServer(), pTarget, AttackPortion, HitLocation );
	//
	return nHitCover;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::GetLastSeenEnemy( SPosition *Position, IAIUnit **ppAIUnit )
{
	*Position = LastSeenEnemyPosition;
	*ppAIUnit = pLastSeenEnemy;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SetLastSeenEnemy( IAIUnit *pAIUnit )
{
	LastSeenEnemyPosition = pAIUnit->GetUnitServer()->GetPosition().pos;
	pLastSeenEnemy = pAIUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIUnit::HasInactivePose()
{
	return ptPosition.p.GetPose() == NAI::CM_INACTIVE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SUnitPosition CAIUnit::GetUnitPosition()
{
	NAI::SUnitPosition res;
	res.pos = ptPosition;
	res.bRun = false; // CRAP
	return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SetPosition( SPosition _ptPosition ) 
{ 
	ptPosition = _ptPosition;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SetPosition( SPathPlace _pPosition )
{
	SPosition s;
	s.p = _pPosition;
	s.SetNetwork( GetUnitServer()->GetWorld()->GetPathNetwork() );
	SetPosition( s );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SPosition CAIUnit::GetPrevPosition()
{
	return ptPrevPosition;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SavePrevPosition()
{
	ptPrevPosition = ptPosition;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::Synchronize()
{
	GetUnitSkillValues();
	ptPosition = pUnitServer->GetPosition().pos; 
	pCannon = GetUnitServer()->animator.GetCannon();
	nTurnStartHP = GetHP();
	SetHurtHP( 0 );
	SetMaxToHit( 0 );
	nAdditionalExpediency = 0;
	pInventory = CreateAIInventory( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SetHP( int _nHP, int _nMaxHP )
{ 
	nHP = _nHP; nMaxHP = _nMaxHP; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SetAP( int _nAP, int _nMaxAP )
{ 
	nAP = _nAP; nMaxAP = _nMaxAP; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SpendHP( int _nHP )
{
	SetHP( Max( 0, nHP - _nHP ), nMaxHP );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SpendAP( int _nAP )
{
	SetAP( Max( 0, nAP - _nAP ), nMaxAP );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIUnit::GetToHit( IAIUnit *pTarget )
{
	int nToHit = 100;
	CDynamicCast<NRPG::CWeaponItem> pWeaponItem( pTarget->GetAIInventory()->GetCurrentWeapon()->GetInventoryItem() );
	if ( IsValid( pWeaponItem ) )
		nToHit = GetCoverForFixedUnit( pTarget->GetUnitServer(), pWeaponItem, NAI::HL_ANY );
	CPtr<NRPG::CAIUnitToHitCalcer> pCalcer = new NRPG::CAIUnitToHitCalcer( this, 
		pTarget, nToHit, HL_ANY, 0, GetAIInventory()->GetCurrentWeapon()->GetInventoryItem(), 0 );
	return pCalcer->GetToHit();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnit::SetPose( int pose )
{
	ptPosition.p.SetPose( pose );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIUnit *CreateAIUnit( NWorld::CUnitServer *pUnitServer, bool bUnderAIControl ) 
{
	ASSERT( IsValid( pUnitServer ) );
	//
	return new CAIUnit( pUnitServer, bUnderAIControl );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}

using namespace NAI;

REGISTER_SAVELOAD_CLASS( 0x52822100, CAIUnit );
REGISTER_SAVELOAD_CLASS( 0x52822090, CAICriterionData );
REGISTER_SAVELOAD_CLASS( 0x52822138, CAIAllyDamageCriterionData );
REGISTER_SAVELOAD_CLASS( 0x52822139, CAIEnemyDamageCriterionData );
REGISTER_SAVELOAD_CLASS( 0x52822140, SAICriterionData );