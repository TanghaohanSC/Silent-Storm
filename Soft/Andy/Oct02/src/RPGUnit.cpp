#include "StdAfx.h"
#include "RPGUnit.h"
#include "RPGItemSet.h"
#include "A5Script.h"
#include "..\Misc\RandomGen.h"
#include "..\Misc\LogStream.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
const int N_DEFAULT_WEAPON_ID = 1;  // MeleeWeapon - "Unarmed"
const float LN161 = 0.476234179f;
int GetSkillByCap( int nCap, int nXP )
{
	if ( nXP <= 0 )
		return 0;
	const float C = 1.f / ( 20.f * LN161 / float(nCap) );
	return log( 1.f + 0.02f * float(nXP) ) * C;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetXPBySkill( int nCap, int nLvl )
{
	if ( nLvl <= 0 )
		return 0;
	const float C = 1.f / ( 20.f * LN161 / float(nCap) );
	return pow( 2.7296f, float(nLvl) / C - 1.f ) / 0.02;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDynamicSkill::Upgrade( float fAddToProgress )
{
	fProgress += fAddToProgress;
	int nModif = int(fProgress);
	if ( nModif > 0 )
	{
		fProgress -= float(nModif);
		Modify( nModif );
		if ( nValue > fMultiplier * nMaxValue )
			nValue = fMultiplier * nMaxValue;
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDynamicSkill::Multiply( float fValue ) 
{ 
	fMultiplier *= fValue; 
	if ( nValue > fMultiplier * nMaxValue )
		nValue = fMultiplier * nMaxValue;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnit
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnit::CUnit()
{
	nXP = 0;
	nHealedVP = 0;
	skills.resize(NDb::SKILL_TYPE_NUMBERS);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnit::GetSkillCap( NDb::ESkillType eSkill, int nXP )
{
	return GetSkillByCap( pClass->skills[eSkill], nXP );
}
int CUnit::GetXPForSkill( NDb::ESkillType eSkill, int nLvl )
{
	return GetXPBySkill( pClass->skills[eSkill], nLvl );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnit::CUnit( NDb::CRPGPers *_pPers )
{
	nHealedVP = 0;
	pPers = _pPers;
	nRPGPersID = pPers->nRPGPersID;
	nXP = 0; skills.resize(NDb::SKILL_TYPE_NUMBERS);
	pInventory = CreateInventory(this);
	NDb::CRPGBaseValue &baseValues = *pPers->pBaseValue;
	skills[NDb::ST_STR] = new CDynamicSkill( baseValues.skills[NDb::ST_STR] );
	skills[NDb::ST_DEX] = new CDynamicSkill( baseValues.skills[NDb::ST_DEX] );
	skills[NDb::ST_INT] = new CDynamicSkill( baseValues.skills[NDb::ST_INT] );
	nXP = baseValues.nBaseXP;
	pClass = pPers->pClass;
	SRand rand;
	pModel = pPers->pModel->CreateModel( &rand );

	pDefaultWeapon = dynamic_cast<CMeleeWeaponItem*>( CreateMeleeWeaponItem( NDb::GetMeleeWeapon( N_DEFAULT_WEAPON_ID ) ) );

	//pInventory->Equip( NDb::SLOT_WEAPON, CreateWeaponItem( pPers->pWeapon ) );
	vector< CPtr<IInventoryItem> > items;
	CPtr<IInventoryItem> pMainWeapon;
	if ( IsValid( pPers->pWeapon ) )
	{
		pMainWeapon = CreateWeaponItem( pPers->pWeapon );
		if ( IsValid( pMainWeapon ) )
			pInventory->Equip( NDb::SLOT_1, pMainWeapon );			
	}
	for ( vector<NDb::SItemAssign>::const_iterator it = pPers->items.begin(); it != pPers->items.end(); ++it )
	{
		const NDb::SItemAssign &assign = *it;
		for( int i = 0; i < assign.nQuantity; ++i )
		{
			if ( IsValid( assign.pItem ) )
			{
				CPtr<NRPG::IInventoryItem> pItem;
				if ( !IsValid( assign.pAmmo ) )
					pItem = CreateItem( assign.pItem );
				else
					if ( CDynamicCast<NDb::CRPGClip> pRPGClip( assign.pItem ) )
						pItem = CreateClipItem( pRPGClip, assign.pAmmo );

				if ( IsValid( pItem ) )
					pInventory->Place( CTPoint<int>( -1, -1 ), pItem );
			}
			else
				ASSERT( 0 && "wrong item was assigned to unit" );
		}
	}
	pInventory->Wear( CreateUniformItem( pPers->pUniform ) );

	for ( int i = 0; i < NDb::ST_STR; ++i )
	{
		skills[i] = new CDynamicSkill( baseValues.skills.skills[i], GetSkillBaseStatValue( NDb::ESkillType(i) ) );
		Skills(i).Reset();
	}
	for ( int i = NDb::ST_MELEE; i < NDb::ST_STR; ++i )
	{
		if ( baseValues.skills.skills[i] == 0 )
			Skills(i).SetXPPart( GetSkillCap( NDb::ESkillType(i), nXP ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnit::AddXP( int nXPToAdd )
{
	ASSERT( IsValid( pPers ) && IsValid( pPers->pName ) );
	if ( !IsValid( pPers ) || !IsValid( pPers->pName ) )
		return;
	//
	csRPG << CC_YELLOW << pPers->pName->szStr << " gained " << nXPToAdd << " EXP" << endl;
	int nLvlEXP = GetXPForSkill( NDb::ST_LEVEL, Skills(NDb::ST_LEVEL) );
	int nLvlUPEXP = GetXPForSkill( NDb::ST_LEVEL, Skills(NDb::ST_LEVEL) + 1 );
	nXP += nXPToAdd;
	Skills(NDb::ST_LEVEL).SetProgress( float(nXP - nLvlEXP) / float(nLvlUPEXP - nLvlEXP) );
	if ( nXP >= nLvlUPEXP )
		csRPG << CC_YELLOW << pPers->pName->szStr << " level up ( " << Skills(NDb::ST_LEVEL) << "-th level )" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnit::UseSkill( int eSkill )
{
	CDynamicSkill &skill = Skills(eSkill);
	float sSkillDiff = GetSkillCap( NDb::ESkillType(eSkill), nXP ) - skill.GetXPPart();
	float fAdd = Clamp( (1.f - skill.GetProgress()) * sSkillDiff / 20.f, 0.005f, 0.1f );
	return skill.Upgrade(fAdd);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnit::GetSkillBaseStatValue( const int eSkill )
{
	switch ( eSkill )
	{
		case NDb::ST_MELEE:
			return Skills(NDb::ST_STR) + 2 * Skills(NDb::ST_DEX);
		case NDb::ST_SHOOTING:
			return 20 + Skills(NDb::ST_DEX);
		case NDb::ST_THROWING:
			return 2 * Skills(NDb::ST_STR) + Skills(NDb::ST_DEX);
		case NDb::ST_BURST:
			return 2 * Skills(NDb::ST_STR) + Skills(NDb::ST_DEX);
		case NDb::ST_SNIPE:
			return 2 * Skills(NDb::ST_DEX) + Skills(NDb::ST_INT);
		//
		case NDb::ST_STEALTH:
			return 5 + 2 * Skills(NDb::ST_DEX);
		case NDb::ST_SPOT:
			return 5 + 2 * Skills(NDb::ST_INT);
		case NDb::ST_MEDICINE:
			return Skills(NDb::ST_DEX) + 2 * Skills(NDb::ST_INT);
		case NDb::ST_ENGINEERING:
			return Skills(NDb::ST_DEX) + 2 * Skills(NDb::ST_INT);
		//
		case NDb::ST_VP:
			return 50 + 10 * Skills(NDb::ST_STR);
		case NDb::ST_INTERRUPT:
			return 25; //CRAP
		case NDb::ST_AP:
			return 40 + 2 * Skills(NDb::ST_DEX);
		case NDb::ST_IC:
			return 5;
		case NDb::ST_LEVEL:
			return 1;
		default:
			ASSERT( 0 && "Unknow skill" );
			break;
	}
	return -1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CRPGPers* CUnit::GetPers() const
{
	return pPers;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::EWeaponType CUnit::GetWeaponType() const
{
	if ( CDynamicCast<NRPG::CMineDetectorItem> pMD( pInventory->GetActive() ) )
		return NDb::WT_MINE_DETECTOR;
	NDb::CAnimWeaponType* pType = GetAnimWeaponType();
	if ( pType )
		return pType->type;
	return NDb::WT_DEFAULT;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CAnimWeaponType* CUnit::GetAnimWeaponType() const
{
	CWeaponItem *pWeapon = GetWeaponItem();
	if ( pWeapon )
		return pWeapon->GetDBWeapon()->pAnimWeaponType;
	CMeleeWeaponItem *pMeleeWeapon = GetMeleeWeaponItem();
	if ( pMeleeWeapon )
		return pMeleeWeapon->GetDBMeleeWeapon()->pAnimWeaponType;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::ESkillType CUnit::GetWeaponSkill() const
{
	CWeaponItem *pWeapon = GetWeaponItem();
	if ( pWeapon )
		return pWeapon->GetSkillIndex();
	return NDb::ST_MELEE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnit::GetWeaponAP() const
{
	int nRes = 0;
	CWeaponItem *pWeapon = GetWeaponItem();
	if ( IsValid( pWeapon ) )
	{
		nRes = pWeapon->GetShootAP();
		NDb::EShootMode ShotMode = pWeapon->GetShootMode();
		if ( ShotMode == NDb::SM_Aimed || ShotMode == NDb::SM_Careful )
			nRes += pWeapon->GetDBWeapon()->nTargetingAP;		
	}
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnit::GetWeaponBurstAP() const
{
	CWeaponItem *pWeapon = GetWeaponItem();
	if ( pWeapon )
	{
		if ( 0 == pWeapon->GetDBWeapon()->nRoF )
		{
			ASSERT( 0 );
			return 0;
		}
		return skills[NDb::ST_AP]->GetMaxValue() / pWeapon->GetDBWeapon()->nRoF;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnit::GetWeaponReloadAP() const
{
	CWeaponItem *pWeapon = GetWeaponItem();
	if ( pWeapon )
		return pWeapon->GetReloadAP();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CWeaponItem* CUnit::GetWeaponItem() const
{
	if ( pCannonItem )
		return pCannonItem;
	if ( CDynamicCast<NRPG::CWeaponItem> pWeapon( pInventory->GetActive() ) )
		return pWeapon;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CMeleeWeaponItem* CUnit::GetMeleeWeaponItem() const
{
	IInventoryItem *pItem = pInventory->GetActive();
	if ( !pItem )
		return pDefaultWeapon;
	if ( CDynamicCast<NRPG::CMeleeWeaponItem> pWeapon( pItem ) )
		return pWeapon;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnit::IsDead()
{
	return Skills(NDb::ST_VP) <= 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnit::Kill()
{
	Skills(NDb::ST_VP) -= N_MAX_VP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnit::SetXPLevel( int nLevel )
{
	int nXP = GetXPForSkill( NDb::ST_LEVEL, nLevel );
	for ( int i = NDb::ST_MELEE; i < NDb::ST_STR; ++i )
		Skills(i).SetXPPart( GetSkillCap( NDb::ESkillType( i ), nXP ) );
	csRPG << "Unit level changed: " << Skills( NDb::ST_LEVEL ) << "-th level" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NRPG;
REGISTER_SAVELOAD_CLASS( 0x24051150, CUnit );
REGISTER_SAVELOAD_CLASS( 0x24051151, CDynamicSkill );
REGISTER_SAVELOAD_CLASS( 0x24051152, CSkillModifier );
