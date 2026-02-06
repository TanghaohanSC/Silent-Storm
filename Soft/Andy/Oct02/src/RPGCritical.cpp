#include "StdAfx.h"
#include "..\DBFormat\DataRPG.h"
#include "RPGUnit.h"
#include "RPGCritical.h"
#include "RPGAllCriticals.h"
#include "RPGUnitMission.h"
#include "RPGItemSet.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS CCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
CCritical::CCritical( const SCritical &crit ): critical(crit), nTurn(0)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CCritical::NextTurn()
{
	if ( critical.nDuration >= 0 && ++nTurn > critical.nDuration )
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CCritical::GetRemainingTime() const
{
	return critical.nDuration - nTurn;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CCritical::GetModifier() const 
{ 
	if ( modifiers.empty() ) return 0; return modifiers.front()->Get(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CCritical::Compare( SCritical *pCritical ) const
{
	if ( pCritical->eCritical != critical.eCritical || pCritical->eCl != critical.eCl )
		return OTHER;
	const int nRTime = GetRemainingTime();
	bool bTime = !IsTemporarily() ? true : (pCritical->nDuration < 0 ? false : nRTime > pCritical->nDuration);
	if ( bTime && critical.fValue > pCritical->fValue && critical.nDC > pCritical->nDC )
		return WEAKER;
	if ( nRTime == pCritical->nDuration && fabs( critical.fValue - pCritical->fValue ) < FP_EPSILON )
		return EQUAL;
	pCritical->nDuration = !IsTemporarily() || pCritical->nDuration < 0 ? -1 : Max( nRTime, pCritical->nDuration );
	pCritical->fValue = Max( critical.fValue, pCritical->fValue );
	pCritical->nDC = Max( critical.nDC, pCritical->nDC );
	return STRONGER;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCritical* CreateCritical( const SCritical &critical )
{
	switch ( critical.eCritical )
	{
		case NDb::C_VP:
			return new CVPCritical( critical );
		case NDb::C_AP_REDUCTION:
			return new CAPCritical( critical );
		case NDb::C_WEAPONSKILL_REDUCTION:
			return new CWeaponSkillCritical( critical );
		case NDb::C_DEATH:
			return new CDeathCritical( critical );
		case NDb::C_MOTIONLESS:
			return new CMotionlessCritical( critical );
		case NDb::C_ACCIDENTAL_SHOT:
			return new CAccidentalShotCritical( critical );
		case NDb::C_LOST_WEAPON:
			return new CLostWeaponCritical( critical );
		case NDb::C_IDLE_HAND:
			return new CIdleHandCritical( critical );
		case NDb::C_DAMAGE_WEAPON:
			return new CDamageWeaponCritical( critical );
		case NDb::C_BLIND:
			return new CBlindCritical( critical );
		case NDb::C_STUN:
			return new CStunCritical( critical );
		case NDb::C_PATIENT:
			return new CPatientCritical( critical );
		case NDb::C_DEAF:
			return new CDeafCritical( critical );
	}
	ASSERT( 0 );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CVPCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVPCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGUnit );
	CDynamicSkill &vp = pRPGUnit->Skills(NDb::ST_VP);
	CDynamicSkill &ap = pRPGUnit->Skills(NDb::ST_AP);
	CDynamicSkill &interrupt = pRPGUnit->Skills(NDb::ST_INTERRUPT);
	CDynamicSkill &melee = pRPGUnit->Skills(NDb::ST_MELEE);

	const int nPercentage = 100.0f * vp / vp.GetMaxValue();
	//
	if ( nPercentage > 75 )
		return false;
	else if ( nPercentage > 50 )
	{
		PushModifier( new CSkillModifier( &ap, 0.95f ) );
		PushModifier( new CSkillModifier( &interrupt, 0.9f ) );
		PushModifier( new CSkillModifier( &melee, 0.9f ) );
	}
	else if ( nPercentage > 25 )
	{
		PushModifier( new CSkillModifier( &ap, 0.9f ) );
		PushModifier( new CSkillModifier( &interrupt, 0.75f ) );
		PushModifier( new CSkillModifier( &melee, 0.75f ) );
	}
	else
	{
		PushModifier( new CSkillModifier( &ap, 0.8f ) );
		PushModifier( new CSkillModifier( &interrupt, 0.5f ) );
		PushModifier( new CSkillModifier( &melee, 0.5f ) );
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CAPCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAPCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGUnit );
	CDynamicSkill &ap = pRPGUnit->Skills(NDb::ST_AP);
 	PushModifier( new CSkillModifier( &ap, 1.0f/critical.fValue ) );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CWeaponSkillCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
static NDb::ESkillType weaponSkills[] = 
{
	NDb::ST_MELEE,
	NDb::ST_SHOOTING,
	NDb::ST_THROWING,
	NDb::ST_BURST,
	NDb::ST_SNIPE,
};
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWeaponSkillCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGUnit );
	//
	for ( int i = 0; i < ARRAY_SIZE( weaponSkills ); ++i )
	{
		CDynamicSkill &s = pRPGUnit->Skills( weaponSkills[i] );
 		PushModifier( new CSkillModifier( &s, 1.0f/critical.fValue ) );
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CDeathCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDeathCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGUnit );
	pRPGUnit->Skills(NDb::ST_VP) -= pRPGUnit->Skills(NDb::ST_VP);
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CMotionlessCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMotionlessCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGUnit && pRPGMission );
	PushModifier( new CSkillModifier( &pRPGUnit->Skills(NDb::ST_AP), 1.0f/critical.fValue ) );
	pRPGMission->Seat();
	data.pRPGMission = pRPGMission;
	pRPGMission->SetLastCritical( GetCriticalType() );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CMotionlessCritical::SModif::~SModif()
{
	if ( IsValid( pRPGMission ) )
		pRPGMission->Stand();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CLostWeaponCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CLostWeaponCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGMission );
	pRPGMission->SetLastCritical( GetCriticalType() );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CIdleHandCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CIdleHandCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGUnit && pRPGMission );
	
	pRPGMission->UseTwoHanded( false );
	data.pRPGMission = pRPGMission;
	pRPGMission->SetLastCritical( GetCriticalType() );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CIdleHandCritical::SModif::~SModif()
{
	if ( IsValid( pRPGMission ) )
		pRPGMission->UseTwoHanded( true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CLostAccidentalShot
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAccidentalShotCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGMission );
	pRPGMission->SetLastCritical( GetCriticalType() );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CDamageWeaponCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDamageWeaponCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGMission );
	if ( CDynamicCast<CWeaponItem> pW( pRPGMission->GetInventory()->GetActive() ) )
	{
		pRPGMission->SetLastCritical( GetCriticalType() );
		pW->Damage();
	}
	else
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CBlindCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBlindCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGUnit && pRPGMission );
	pRPGMission->Blind( true );
	data.pRPGMission = pRPGMission;
	pRPGMission->SetLastCritical( GetCriticalType() );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CBlindCritical::SModif::~SModif()
{
	if ( IsValid( pRPGMission ) )
		pRPGMission->Blind( false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CStunCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CStunCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//                      CLASS CDeafCritical
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDeafCritical::SetModifiers( CUnit *pRPGUnit, IUnitMission *pRPGMission )
{
	ASSERT( pRPGUnit && pRPGMission );
	pRPGMission->Deaf( true );
	data.pRPGMission = pRPGMission;
	pRPGMission->SetLastCritical( GetCriticalType() );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CDeafCritical::SModif::~SModif()
{
	if ( IsValid( pRPGMission ) )
		pRPGMission->Deaf( false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NRPG;
REGISTER_SAVELOAD_CLASS( 0xA28C1160, CVPCritical );
REGISTER_SAVELOAD_CLASS( 0xA0812160, CAPCritical );
REGISTER_SAVELOAD_CLASS( 0xA0912140, CWeaponSkillCritical );
REGISTER_SAVELOAD_CLASS( 0xA1812120, CDeathCritical );
REGISTER_SAVELOAD_CLASS( 0xA2112180, CMotionlessCritical );
REGISTER_SAVELOAD_CLASS( 0xA2412180, CLostWeaponCritical );
REGISTER_SAVELOAD_CLASS( 0xA2412181, CIdleHandCritical );
REGISTER_SAVELOAD_CLASS( 0xA2412182, CAccidentalShotCritical );
REGISTER_SAVELOAD_CLASS( 0xA2512150, CDamageWeaponCritical );
REGISTER_SAVELOAD_CLASS( 0xA2812130, CBlindCritical );
REGISTER_SAVELOAD_CLASS( 0xA0132160, CStunCritical );
REGISTER_SAVELOAD_CLASS( 0x50642130, CPatientCritical );
REGISTER_SAVELOAD_CLASS( 0x50842160, CDeafCritical );
