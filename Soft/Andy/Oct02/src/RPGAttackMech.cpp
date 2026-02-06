#include "StdAfx.h"
#include "RPGAttackMech.h"
#include "..\DBFormat\DataRPG.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetDmg2Armor( int nDmg, int nArmor )
{
	if ( nArmor < 0 || nDmg < 0 )
		return 0;
	NDb::CRPGDmgToArmor *pAr = NDb::GetDBDmg2Armor(nDmg);
	if ( !pAr )
		return 0;
	return pAr->armors[nArmor];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAttackPortion::CanDealDmg( const NDb::CRPGArmor *pArmor ) const
{
	ASSERT( pArmor != 0 );
	if ( pArmor == 0 )
		return false;
	//
	return GetDmg2Armor( nDmgType, pArmor->nDR ) > 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAttackPortion::IsArmorIgnored( const NDb::CRPGArmor *pArmor ) const
{
	ASSERT( pArmor != 0 );
	if ( pArmor == 0 )
		return false;
	//
	int nDmg = GetDmg2Armor( nDmgType, pArmor->nDR );
	return nDmg < 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAttackPortion::CalcStructDmg( const NDb::CRPGArmor *pArmor ) const
{
	ASSERT( pArmor != 0 );
	if ( pArmor == 0 )
		return 0;
	//
	int nDmg = GetDmg2Armor( nDmgType, pArmor->nDR );
	int nResDmg = !nDmg ? nDmg : ( pArmor->nVP / nDmg );
	return nResDmg * fDamageCoeff;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAttackPortion::MakeClickOfDeath( const CRay &ray )
{
	nK = 1000000;
	nDmgType = 0;
	nDmgMin = 10000000; nDmgMax = 10000001;
	nCrtical = 0;
	nCrticalDifficulty = 0;
	rTtrajectory = ray;
	pAttacker = 0;
	pTarget = 0;
	fDamageCoeff = 1e6;
	atkType = AT_CLICK_OF_DEATH;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////

