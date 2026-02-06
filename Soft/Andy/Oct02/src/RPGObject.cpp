#include "StdAfx.h"
#include "rpgobject.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataGeometry.h"
#include "RPGUnit.h"
#include "RPGAttackMech.h"
#include "..\Misc\LogStream.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CObject: public IObject, public IAttackable
{
	OBJECT_BASIC_METHODS(CObject);
public:
	ZDATA
	int nVP;
	int nDestroyStages, nStage;
	int nMaxVP;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nVP); f.Add(3,&nDestroyStages); f.Add(4,&nStage); f.Add(5,&nMaxVP); return 0; }

	CObject() : nVP(0), nDestroyStages(0), nStage(0) {}
	CObject( int nStages, NDb::CModel *pModel );
	virtual int ProcessAttack( int nUserID, CAttackPortion *pAttack, NDb::CRPGArmor *pArmor );
	virtual bool IsDead() const;
	virtual int GetDestroyStage();
	virtual void Kill();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CObject::CObject( int nStages, NDb::CModel *pModel ): nDestroyStages(nStages), nStage(0) 
{
	if ( IsValid( pModel ) )
		nMaxVP = pModel->GetMaxVP();
	else
		nMaxVP = 1;
	//
	nVP = nMaxVP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CObject::ProcessAttack( int nUserID, CAttackPortion *pAttack, NDb::CRPGArmor *pArmor )
{
	if ( pAttack->atkType == AT_CLICK_OF_DEATH && nStage < nDestroyStages )
	{
		int nOldVP = nVP;
		++nStage;
		nVP = Max( 0, nMaxVP - Float2Int( nMaxVP * (nStage + 0.5f) ) );
		ASSERT( nVP <= nOldVP );
		return nOldVP - nVP;
	}
	if ( nMaxVP <= 0 )
		return 0;
	int nDmg = pAttack->CalcStructDmg(pArmor);
	nVP = Max( 0, nVP - nDmg );
	nStage = ( ( nMaxVP - nVP ) * nDestroyStages ) / nMaxVP;
	csRPG << "Shoot hit object, result damage = " << nDmg << ", remain " << nVP << " of " << nMaxVP << "\n";
	return nDmg;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObject::IsDead() const
{
	return nStage == nDestroyStages;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CObject::GetDestroyStage()
{
	return nStage;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject::Kill()
{
	nStage = nDestroyStages;
	nVP = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IObject* CreateObject( int nStages, NDb::CModel *pModel )
{
	return new CObject( nStages, pModel );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*IObject* CreateTerrain()
{
	CObject *pRes = new CObject;
	pRes->pArmor = NDb::GetArmor(NDb::CRPGArmor::GROUND);
	pRes->nVP = -1;
	return pRes;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CRPGArmor* GetTerrainArmor()
{
	return NDb::GetArmor(NDb::CRPGArmor::GROUND);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
REGISTER_SAVELOAD_CLASS_NM( 0x02781180, CObject, NRPG )
using namespace NRPG;
BASIC_REGISTER_CLASS( IObject )
