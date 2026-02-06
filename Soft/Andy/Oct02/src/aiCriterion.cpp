#include "StdAfx.h"

#include "aiGrid.h"
#include "aiUnit.h"
#include "aiState.h"
#include "aiPlayer.h"
#include "aiWeapon.h"
#include "aiPosition.h"
#include "aiInventory.h"
#include "aiScaleConverter.h"
#include "aiTacticalCommander.h"

#include "..\misc\RandomGen.h"

#include "wMain.h"
#include "wUnitServer.h"

#include "RPGItemInfo.h"
#include "RPGToHit.h"
#include "RPGGame.h"
//
#include "aiCriterion.h"
//
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// СAICriterionContainer
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAICriterion
{
	ZDATA
	float fWeight; // вес критерия
	CObj<IAICriterion> pAICriterion; // критерий
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&fWeight); f.Add(3,&pAICriterion); return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAICriterionContainer: public CAICriterion
{
	OBJECT_BASIC_METHODS(CAICriterionContainer)
	ZDATA
	ZPARENT(CAICriterion);
	vector< SAICriterion > vAICriterions;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAICriterion*)this); f.Add(3,&vAICriterions); return 0; }
public:
	//
	CAICriterionContainer( IAIState *_pAIState = 0 ): CAICriterion( _pAIState ) {}
	//
	void AddCriterion( IAICriterion *pAICriterion, float fWeight )
	{
		SAICriterion psCriterion;
		psCriterion.fWeight = fWeight;
		psCriterion.pAICriterion = pAICriterion;
		vAICriterions.push_back(psCriterion);
	}
	// IAICriterion
	virtual float GetExpediency()
	{
		float fRes = 0;
		for ( vector< SAICriterion >::iterator i = vAICriterions.begin(); i != vAICriterions.end(); ++i )
			fRes += (*i).fWeight * (*i).pAICriterion->GetExpediency();
		return fRes/vAICriterions.size();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIToHitCriterion
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIToHitCriterion: public CAICriterion
{
	OBJECT_BASIC_METHODS(CAIToHitCriterion);
	//
	ZDATA
	ZPARENT( CAICriterion );
	CObj<CScaleConverter> pConverter;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAICriterion *)this); f.Add(3,&pConverter); return 0; }
public:
	//
	CAIToHitCriterion() {}
	CAIToHitCriterion( IAIState *pAIState );
	//
	virtual float GetExpediency();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIToHitCriterion::CAIToHitCriterion( IAIState *pAIState ):
	CAICriterion(pAIState)
{
	pConverter = new CScaleConverter();
	pConverter->AddPoint( -0xFFFF, 0.f );
	pConverter->AddPoint( 0.0f, 0.f );
	pConverter->AddPoint( 0.4f, 0.75f );
	pConverter->AddPoint( 1.f, 1.f );
	pConverter->AddPoint( 0xFFFF, 1.f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIToHitCriterion::GetExpediency()
{
	IAIUnit *pUnit = pAIState->GetCurrentAIUnit();
	IAIUnit *pEnemy = pAIState->GetCurrentAIEnemy();
	//
	if ( !IsValid(pUnit) || !IsValid(pEnemy) )
		return 0;
	// должны видеть врага
	/*
	CPtr<NWorld::CWorld> pWorld = pUnit->GetUnitServer()->GetWorld(); 
	NAI::SPosition targetPos = pEnemy->GetPosition();
	NAI::SUnitPosition unitPos = pUnit->GetUnitPosition();
	int nEnemyDir = pWorld->GetPathNetwork()->GetClosestDir( unitPos.pos.p, targetPos.p );
	unitPos.pos.p.SetDirection( nEnemyDir );
	if ( !pWorld->GetGame()->CheckAIPositionVisibility( unitPos, targetPos ) )
		return 0;
	*/
	// на момент вычисления оружие неизвестно
	CPtr<NRPG::CAIUnitNoWeaponToHitCalcer> pToHit = new NRPG::CAIUnitNoWeaponToHitCalcer( pUnit, pEnemy );
	return pConverter->Convert( pToHit->GetToHit() / 100.f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIHideCriterion
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIHideCriterion: public CAICriterion
{
	OBJECT_BASIC_METHODS(CAIHideCriterion);
	//
	ZDATA
	ZPARENT( CAICriterion );
	CObj<CScaleConverter> pToHitConverter;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAICriterion *)this); f.Add(3,&pToHitConverter); return 0; }
public:
	//
	CAIHideCriterion() {}
	CAIHideCriterion( IAIState *pAIState );
	//
	virtual float GetExpediency();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIHideCriterion::CAIHideCriterion( IAIState *pAIState ): 
	CAICriterion(pAIState)
{
	pToHitConverter = new CScaleConverter();
	pToHitConverter->AddPoint( -0xFFFF, 1.f );
	pToHitConverter->AddPoint( 0.0f, 1.f );
	pToHitConverter->AddPoint( 0.5f, 0.25f );
	pToHitConverter->AddPoint( 1.f, 0.f );
	pToHitConverter->AddPoint( 0xFFFF, 0.f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIHideCriterion::GetExpediency()
{
	float fToHit = 0;
	IAIUnit *pUnit = pAIState->GetCurrentAIUnit();
	if ( !IsValid( pUnit ) )
		return 0;
	//
	vector< CPtr<IAIUnit> > *pEnemyUnits = pAIState->GetEnemyAIPlayer()->GetUnits();
	for ( vector< CPtr<IAIUnit> >::iterator i = pEnemyUnits->begin(); i != pEnemyUnits->end(); ++i )
		fToHit = max( fToHit, (*i)->GetToHit( pUnit ) / 100.f );
	//
	return pToHitConverter->Convert( fToHit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIDamageCriterion
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIDamageCriterion: public CAICriterion
{
	OBJECT_BASIC_METHODS(CAIDamageCriterion);
	//
	ZDATA
	ZPARENT( CAICriterion );
	CObj<CScaleConverter> pHPConverter;
	CObj<CScaleConverter> pAPConverter;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAICriterion *)this); f.Add(3,&pHPConverter); f.Add(4,&pAPConverter); return 0; }
public:
	//
	CAIDamageCriterion() {}
	CAIDamageCriterion( IAIState *pAIState );
	//
	virtual float GetExpediency();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIDamageCriterion::CAIDamageCriterion( IAIState *pAIState ): 
	CAICriterion(pAIState)
{
	pHPConverter = new CScaleConverter();
	pHPConverter->AddPoint( -0xFFFF, 0.f );
	pHPConverter->AddPoint( -10000.f, 0.f );
	pHPConverter->AddPoint( 10000.f, 1.f );
	pHPConverter->AddPoint( 0xFFFF, 1.f );
	//
	pAPConverter = new CScaleConverter();
	pAPConverter->AddPoint( 0.0f, 1.f );
	pAPConverter->AddPoint( 1.5f, 0.8f );
	pAPConverter->AddPoint( 3.f, 0.5f );
	pAPConverter->AddPoint( 10.f, 0.15f );
	pAPConverter->AddPoint( 0xFFFF, 0.f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIDamageCriterion::GetExpediency()
{
	CPtr<IAIUnit> pUnit = pAIState->GetCurrentAIUnit();
	CPtr<IAIUnit> pEnemy = pAIState->GetCurrentAIEnemy();
	if ( !IsValid( pUnit ) || !IsValid( pEnemy ) )
		return 0;
	// повреждения нанесенные врагам
	int nEnemyDamage = pUnit->GetHurtHP() + pUnit->GetAdditionalExpediency();
	// CRAP{
	// если даже не попадаем должно быть немного выгоднее подойти ближе
	if ( pUnit->GetHurtHP() <= 0 && pUnit->GetMaxToHit() <= 0 )
	{
		float fDistance = max( 10, fabs2( pUnit->GetPosition().GetCP() - pEnemy->GetPosition().GetCP() ) );
		nEnemyDamage = max( 1, 15 * ( 100.f - fDistance ) / 100.f ) + pUnit->GetAdditionalExpediency();
		return pHPConverter->Convert( nEnemyDamage - 1000 ); // -1000: если попадаем, то это лучше
	}
	// CRAP}
	// повреждения нанесенные нам
	int nAllyDamage = 0;
	vector< CPtr<IAIUnit> > *pEnemies = pAIState->GetEnemyAIPlayer()->GetUnits();
	for ( vector< CPtr<IAIUnit> >::iterator e = pEnemies->begin(); e != pEnemies->end(); ++e )
	{
		if ( (*e)->GetHP() > -50 && pAIState->GetAITacticalCommander()->GetDangerousAttackableEnemy( *e ) == pUnit )
		{
			int nTmp;
			int nTmpMaxToHit, nHitCover;
			NDb::EShootMode eTmpShootMode;
			float fRange = fabs( pUnit->GetPosition().GetCP() - (*e)->GetPosition().GetCP() );
			int nAPToShot = (*e)->GetMaxAP() * pAPConverter->Convert( fRange );
			CPtr<IAIWeapon> pWeapon = (*e)->GetAIInventory()->GetBestWeapon( pUnit, nAPToShot,
				&nHitCover, &nTmp, &eTmpShootMode, &nTmpMaxToHit );
			if ( IsValid( pWeapon ) )
				nAllyDamage += pWeapon->GetDamage( pUnit, nHitCover, 
					(NAI::EPose)(*e)->GetPosition().p.GetPose(), nAPToShot, eTmpShootMode, &nTmpMaxToHit );
		}
	}
	//
	return pHPConverter->Convert( nEnemyDamage - nAllyDamage );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAICriterion *CreateAIDamageCriterion( IAIState *pAIState )
{
	return new CAIDamageCriterion( pAIState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAICriterion *CreateAIToHitCriterion( IAIState *pAIState )
{
	return new CAIToHitCriterion( pAIState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAICriterion *CreateAIHideCriterion( IAIState *pAIState )
{
	return new CAIHideCriterion( pAIState );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAICriterion *CreateAICautiousToHitCriterion( IAIState *pAIState )
{
	CAICriterionContainer *pAICriterion = new CAICriterionContainer( pAIState );
	pAICriterion->AddCriterion( CreateAIToHitCriterion( pAIState ), 1.f );
	pAICriterion->AddCriterion( CreateAIHideCriterion( pAIState ), 1.f );
	return pAICriterion;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}

using namespace NAI;

REGISTER_SAVELOAD_CLASS( 0x50422100, CAICriterionContainer );
REGISTER_SAVELOAD_CLASS( 0x53152160, CAIToHitCriterion );
REGISTER_SAVELOAD_CLASS( 0x53152161, CAIHideCriterion );
REGISTER_SAVELOAD_CLASS( 0x50562140, CAIDamageCriterion );