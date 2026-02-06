#ifndef __RPGATTACKMECH_H_
#define __RPGATTACKMECH_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
	class CRPGArmor;
}
namespace NRPG
{
class IUnitMissionInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EAttackType
{
	AT_NORMAL,
	AT_BLAST_WAVE,
	AT_CLICK_OF_DEATH
};
class CAttackPortion
{
public:
	ZDATA
	int nK; // "кинетическая энергия" - мера пробивающей способности
	int nDmgType;			// Тип наносимого damage-а
	int nDmgMin, nDmgMax;	// вред по юнитам
	int nCrtical;  // вероятность критикала
	int nCrticalDifficulty;
	CRay rTtrajectory;
	CPtr<IUnitMissionInfo> pAttacker;
	CPtr<IUnitMissionInfo> pTarget;
	float fDamageCoeff;
	EAttackType atkType;
	int nUnconsciousProbability;
	bool bBackStab;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nK); f.Add(3,&nDmgType); f.Add(4,&nDmgMin); f.Add(5,&nDmgMax); f.Add(6,&nCrtical); f.Add(7,&nCrticalDifficulty); f.Add(8,&rTtrajectory); f.Add(9,&pAttacker); f.Add(10,&pTarget); f.Add(11,&fDamageCoeff); f.Add(12,&atkType); f.Add(13,&nUnconsciousProbability); f.Add(14,&bBackStab); return 0; }
	CAttackPortion() {}
	CAttackPortion( int _nK, int _nDmgType, int _nDmgMin, int _nDmgMax, 
		int _nCrtical, int nCritDifficulty = 0, IUnitMissionInfo *_pAttacker = 0, 
		IUnitMissionInfo *_pTarget = 0, float _fDamageCoeff = 1, 
		int _nUnconsciousProbability = 0, bool _bBackStab = false ):
			nK(_nK), nDmgType(_nDmgType), nDmgMin(_nDmgMin), nDmgMax(_nDmgMax), nCrtical(_nCrtical), 
			nCrticalDifficulty(nCritDifficulty), pTarget(_pTarget), pAttacker(_pAttacker), 
			fDamageCoeff( _fDamageCoeff ), atkType( AT_NORMAL ), 
			nUnconsciousProbability( _nUnconsciousProbability ), bBackStab( _bBackStab ) {}

	bool CanDealDmg( const NDb::CRPGArmor *pArmor ) const;
	bool IsArmorIgnored( const NDb::CRPGArmor *pArmor ) const;
	bool CanRicochet() const;
	int  CalcStructDmg( const NDb::CRPGArmor *pArmor ) const;
	void MakeClickOfDeath( const CRay &r );
};
float GetAPASubstraction( float fEnter, float fExit, const NDb::CRPGArmor *pArmor );
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAttackable
{
public:
	virtual int ProcessAttack( int nUserID, CAttackPortion *pAttack, NDb::CRPGArmor *pArmor ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif