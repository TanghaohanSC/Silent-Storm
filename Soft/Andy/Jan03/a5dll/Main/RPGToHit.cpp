#include "StdAfx.h"

#include "aiUnit.h"
#include "aiPosition.h"

#include "..\DBFormat\DataRPG.h"

#include "RPGGame.h"
#include "RPGUnit.h"
#include "RPGItemSet.h"
#include "RPGItemInfo.h"
#include "RPGUnitMission.h"

#include "..\MiscDll\LogStream.h"

#include "math.h"

#include "wOSBase.h"
#include "wUnitServer.h"
//
#include "RPGToHit.h"
//
namespace NRPG
{
inline float DistanceFunc( int nDistInTile, float fSlope )
{
	return ( (fSlope + 1.f) * 100.f ) / ( float(nDistInTile) + fSlope );
}
inline float Cos3DistanceFunc( int nDistInTile, float fSlope )
{
	return 100.f * pow( cos( float(nDistInTile) / fSlope ), 3 ) + 2;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CToHitCalcer::CToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
	CVec3 _ptAttacker, int _nExtraAP, int _nSnipeAP, int _nHitCover, bool _bFirstRound,	
	CVec3 _ptIllumination, int _nBullet, bool _bBackStab ):
		pUnitMission(_pUnitMission), eCurPose(_eCurPose), nDistance(_nDistance), ptAttacker(_ptAttacker),
		nExtraAP(_nExtraAP), nHitCover(_nHitCover), bFirstRound(_bFirstRound), ptIllumination(_ptIllumination),
		nSnipeAP(_nSnipeAP), nBullet(_nBullet), bBackStab( _bBackStab )
{
	pWeaponItem = pUnitMission->GetRPGUnit()->GetWeaponItem();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CToHitCalcer::FillWeaponInfo()
{
	sWeaponInfo.nQuality = 0;
	sWeaponInfo.nMinRange = 0;
	sWeaponInfo.nMaxRange = 0;
	sWeaponInfo.nShotAP = 0;
	sWeaponInfo.nTargetingAP = 0;
	sWeaponInfo.nRecoil = 0;
	sWeaponInfo.nDmgMin = 0;
	sWeaponInfo.nDmgMax = 0;
	sWeaponInfo.nArmorPiercingAbility = 0;
	sWeaponInfo.fScopeFactor = 0;
	fMovePenalty = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CToHitCalcer::Prepare()
{
	if ( IsValid( pWeaponItem ) )
	{
		pWeaponItem->GetInfo( &sWeaponInfo );
		fMovePenalty = pWeaponItem->GetDBWeapon()->pWeaponType->fMovePenalty;
	}
	else
		FillWeaponInfo();
	//
	nSkill = pUnitMission->GetRPGUnit()->Skills( NDb::ST_SHOOTING );
	//
	if ( IsValid( pWeaponItem ) )
	{
		NDb::EShootMode shootMode = pWeaponItem->GetShootMode();
		if ( shootMode == NDb::SM_Snipe )
			nExtraAP += nSnipeAP;
		if ( shootMode == NDb::SM_Careful || shootMode == NDb::SM_Snipe )
			nSkill += max( 0, int( nExtraAP - GetSMove() ) ) / 3;
	}
	//
	if ( nBullet > 0 )
	{
		float fBurstNonStab = nSkill * pow( (double)pUnitMission->GetRPGUnit()->Skills(NDb::ST_BURST) / N_MAX_SKILL, 
			nBullet );
		float fBurstStab = pUnitMission->GetRPGUnit()->Skills(NDb::ST_BURST) * 
			pUnitMission->GetToHitConstants()->nMaxBurstStabilize / N_MAX_SKILL;
		float fStabilized = fBurstStab * nSkill / 100;
		nSkill = Max( fBurstNonStab, fStabilized ) * sWeaponInfo.nRecoil / 100;
	}
	float fVPPenalty = GetVPPenalty( pUnitMission->GetRPGUnit()->Skills( NDb::ST_VP ), 
		pUnitMission->GetHealedVP(), pUnitMission->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue() );
	nSkill *= fVPPenalty;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetStance()
{
	int nStance = 0; 
	CPtr<CWeaponItem> pWeaponItem = pUnitMission->GetRPGUnit()->GetWeaponItem();
	if ( IsValid( pWeaponItem ) )
	{
		if ( NAI::CROUCH == eCurPose )
			nStance = int( pWeaponItem->GetDBWeapon()->pWeaponType->fCrouchBonus );
		else if ( NAI::CRAWL == eCurPose ) 
			nStance = int( pWeaponItem->GetDBWeapon()->pWeaponType->fCrawlBonus );
	}
	return nStance;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetD1()
{
	return float(nSkill) * ( sWeaponInfo.nMaxRange - sWeaponInfo.nMinRange ) / float(N_MAX_SKILL) + sWeaponInfo.nMinRange;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetD2()
{
	float fD1 = GetD1();
//	return nDistance <= sWeaponInfo.nRange ? DistanceFunc( nDistance, fD1 ) : -2 * (nDistance - sWeaponInfo.nRange);
	float fDTH = Cos3DistanceFunc( nDistance, fD1 );
	fDTH = Clamp( fDTH, 2.f, 100.f );
	if ( nDistance > fD1 * 2 )
		fDTH = 0;
//	return nDistance <= sWeaponInfo.nRange ? fDTH : fDTH * (2.f * sWeaponInfo.nRange - nDistance) / sWeaponInfo.nRange;
	return fDTH;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetRS()
{
	return Min( pUnitMission->GetLastActionTimes(), pUnitMission->GetToHitConstants()->nMaxShotsRepeat ) * 2;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetSMove()
{
	float fRes = 0;
	//
	if ( !bFirstRound )
		fRes = Min( pUnitMission->GetMoveInLastTurn(), 
			pUnitMission->GetToHitConstants()->nSMaxMove ) * fMovePenalty;
	//
	return fRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetLight()
{
	CVec3 ptLight = CVec3( Clamp(ptIllumination.x, 0.f, 1.f), 
		Clamp(ptIllumination.y, 0.f, 1.f), Clamp(ptIllumination.z, 0.f, 1.f) );
	return 0.5f * (1 + fabs( ptLight ) / fabs( CVec3(1,1,1) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetFRMult()
{
	return bFirstRound ? pUnitMission->GetToHitConstants()->fFirstRoundCoeff : 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetAllMult()
{
	return Max( 0.f, GetAllAdd() * GetLight() * GetFRMult() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetSnipeAdd()
{
	float fMaxSkillAdd = pUnitMission->GetRPGUnit()->Skills( NDb::ST_SNIPE ) * 
		1.f / pUnitMission->GetToHitConstants()->nSnipingCoeff;
	float fAPSnipeAdd = nSnipeAP * 1.f / pUnitMission->GetToHitConstants()->nSnipingCoeff;
	return ( fAPSnipeAdd <= fMaxSkillAdd ) ? fAPSnipeAdd : fMaxSkillAdd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetTArea()
{
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetCA()
{
	float fTArea = GetTArea();
	float fTCover = float(nHitCover) / 100.f;
	float fScopeFactor = sWeaponInfo.fScopeFactor; // 20 - обычное оружие // 100 - снайперская винтовка
	float fDistCoeff = DistanceFunc( nDistance, fScopeFactor );
	return ( (100-fDistCoeff)*(fTCover*fTArea)+fDistCoeff ) / 100.0f;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CToHitCalcer::GetAllAdd()
{
	float fRes = sWeaponInfo.nQuality + GetD2() + 
		GetStance() + GetSMove() + GetRS();
	//
	if ( IsValid( pWeaponItem ) )
	{
		NDb::EShootMode ShotMode = pWeaponItem->GetShootMode();
		if ( ShotMode == NDb::SM_Aimed || ShotMode == NDb::SM_Careful )
			fRes += sWeaponInfo.nTargetingAP;
		if ( ShotMode == NDb::SM_Careful || ShotMode == NDb::SM_Snipe )
			fRes += max( nExtraAP, int( GetSMove() ) );
	}
	//
	return fRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CToHitCalcer::GetToHit()
{
	Prepare();
	if ( nHitCover < 1 )
		return 0;
	fToHit = Clamp( GetAllMult(), 2.0f, 100.0f );
	return fToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CToHitCalcer::Log()
{
	csRPG << "<font size=16pt>";
	csRPG << CC_GREEN << "General ToHit:";
	csRPG << CC_ORANGE << "\tDist=" << CC_GREY << nDistance;
	csRPG << CC_ORANGE << "\tW+=" << CC_GREY << sWeaponInfo.nQuality;
	csRPG << CC_ORANGE << "\tMinRange=" << CC_GREY << sWeaponInfo.nMinRange;
	csRPG << CC_ORANGE << "\tMaxRange=" << CC_GREY << sWeaponInfo.nMaxRange;
	csRPG << CC_ORANGE << "\tBulletNumber=" << CC_GREY << nBullet << endl;
	csRPG << "<font size=16pt>";
	csRPG << CC_ORANGE << "\tStance=" << CC_GREY << GetStance();
//	csRPG << CC_ORANGE << "\tTMove=" << CC_GREY << GetTMove();
	csRPG << CC_ORANGE << "\tDerSkill=" << CC_GREY << nSkill;
	csRPG << CC_ORANGE << "\tD1=" << CC_GREY << GetD1();
	csRPG << CC_ORANGE << "\tD2=" << CC_GREY << GetD2();
	csRPG << CC_ORANGE << "\tZeroingIn(RS)=" << CC_GREY << GetRS();
	csRPG << CC_ORANGE << "\tAddAP=" << CC_GREY << nExtraAP;
	csRPG << CC_ORANGE << "\tTCover=0." << CC_GREY << nHitCover;
	csRPG << CC_ORANGE << "\tTArea=" << CC_GREY << GetTArea();
	csRPG << CC_ORANGE << "\tCA=" << CC_GREY << GetCA();
	csRPG << CC_ORANGE << "\tSMove=" << CC_GREY << GetSMove() << endl;
	csRPG << "<font size=16pt>";
	csRPG << CC_ORANGE << "\tLight=" << CC_GREY << GetLight();
	csRPG << CC_ORANGE << "\tFRmult=" << CC_GREY << GetFRMult();
//	csRPG << CC_ORANGE << "\tHL=" << CC_GREY << GetHLName( eHitLocation );
	csRPG << CC_ORANGE << "\tAllAdd=" << CC_GREY << GetAllAdd();
	csRPG << CC_ORANGE << "\tAllAddMult=" << CC_GREY << GetAllMult();
//	csRPG << CC_ORANGE << "\tBaseIC=" << CC_GREY << pTarget->GetIC();
	csRPG << CC_GREEN << "\tToHit = " << GetAllAdd() << "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitToHitCalcer::CUnitToHitCalcer(	IUnitMission *_pUnitMission, NAI::EPose _eCurPose,	
		int _nDistance,	CVec3 _ptAttacker, NAI::SPosition _sTargetPosition, int _nExtraAP, int _nSnipeAP, 
		int _nHitCover, bool _bFirstRound,	CVec3 _ptIllumination, NAI::EHitLocation _eHitLocation, 
		IUnitMissionInfo *_pTarget, int _nBullet, bool _bBackStab ) :
			CToHitCalcer( _pUnitMission, _eCurPose, _nDistance, _ptAttacker, _nExtraAP, _nSnipeAP, _nHitCover, 
			_bFirstRound,	_ptIllumination, _nBullet, _bBackStab ), sTargetPosition(_sTargetPosition), 
			eHitLocation(_eHitLocation), pTarget(_pTarget)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitToHitCalcer::GetToHit()
{
	Prepare();
	if ( nHitCover < 1 )
		return 0;
	float fTargetIC = 0;
	if ( !bBackStab )
		fTargetIC = pTarget->GetIC() / pUnitMission->GetToHitConstants()->fICModifier;
	fToHit = GetAllMult() * GetCA() - fTargetIC;
	fToHit = Clamp( fToHit, 2.0f, 100.0f );
	return fToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CUnitToHitCalcer::GetTArea()
{
	vector<CVec3> vCubes;
	GetOccupiedCubes( &vCubes, sTargetPosition );
	float fTArea = GetCubesArea( ptAttacker, &vCubes );
	switch ( eHitLocation )
	{
		case NAI::HL_BODY:
			fTArea *= 0.40f;
			break;
		case NAI::HL_HEAD:
			fTArea *= 0.06f;
			break;
		case NAI::HL_RHAND:
		case NAI::HL_LHAND:
			fTArea *= 0.09f;
			break;
		case NAI::HL_RLEG:
		case NAI::HL_LLEG:
			fTArea *= 0.18f;
			break;
	}
	return fTArea;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CUnitToHitCalcer::GetTMove()
{
	CVec3 ptTMove = FP_INV_GRID_STEP * ( sTargetPosition.GetCP() - pTarget->GetTurnStartCP() );
	CVec3 ptTDir  = sTargetPosition.GetCP() - ptAttacker;
	float fTargetMoveDist = fabs( ptTMove );
	Normalize( &ptTMove );
	Normalize( &ptTDir );
	float fCos = ptTDir * ptTMove;
	float fTargetMoveDir = sqrt(1 - fCos * fCos);

	float fRes = Min( fTargetMoveDist * fTargetMoveDir,
		(float)pUnitMission->GetToHitConstants()->nMaxMoveBonus );
	if ( bFirstRound )
		fRes = 0;
	return fRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CUnitToHitCalcer::GetAllAdd()
{
	return CToHitCalcer::GetAllAdd() - GetTMove();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitToHitCalcer::Log()
{
	CToHitCalcer::Log();
	csRPG << "<font size=16pt>";
	csRPG << CC_ORANGE << "Unit ToHit:\tTMove=" << CC_GREY << GetTMove();
	csRPG << CC_ORANGE << "\tHL=" << CC_GREY << GetHLName( eHitLocation );
	csRPG << CC_ORANGE << "\tBaseIC=" << CC_GREY << pTarget->GetIC() << "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CObjectToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CObjectToHitCalcer::CObjectToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose,
	int _nDistance,	CVec3 _ptAttacker, int _nExtraAP, int _nHitCover, bool _bFirstRound,	
	CVec3 _ptIllumination, int _nBullet ) :
		CToHitCalcer( _pUnitMission, _eCurPose, _nDistance, _ptAttacker, _nExtraAP, 0, _nHitCover, 
			_bFirstRound,	_ptIllumination, _nBullet, false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CObjectToHitCalcer::GetCA()
{
	return CToHitCalcer::GetCA();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTileToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CTileToHitCalcer::CTileToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose,
	int _nDistance,	CVec3 _ptAttacker, int _nExtraAP, int _nHitCover, bool _bFirstRound,	
	CVec3 _ptIllumination, NAI::ETileHitLocation _eHitLocation, CVec3 _ptTilePos, int _nBullet ) :
		CToHitCalcer( _pUnitMission, _eCurPose, _nDistance, _ptAttacker, _nExtraAP, 0, _nHitCover, 
			_bFirstRound,	_ptIllumination, _nBullet, false ), eHitLocation(_eHitLocation), ptTilePos(_ptTilePos)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CTileToHitCalcer::GetTArea()
{
	return 0.33f;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGrenadeToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CGrenadeToHitCalcer::CGrenadeToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
	CVec3 _ptAttacker, bool _bFirstRound,	CVec3 _ptIllumination, CVec3 _ptTilePos, NDb::CRPGGrenade *_pGrenade ) :
		CToHitCalcer( _pUnitMission, _eCurPose, _nDistance, _ptAttacker, 0, 0, 0, _bFirstRound,	_ptIllumination, 0, false ), 
		ptTilePos(_ptTilePos), pGrenade(_pGrenade)
{
	Prepare();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGrenadeToHitCalcer::Prepare()
{
	nSkill = pUnitMission->GetRPGUnit()->Skills( NDb::ST_THROWING );
	float fVPPenalty = GetVPPenalty( pUnitMission->GetRPGUnit()->Skills( NDb::ST_VP ), 
		pUnitMission->GetHealedVP(), pUnitMission->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue() );
	nSkill *= fVPPenalty;
	fMovePenalty = pGrenade->pWeaponType->fMovePenalty;
	FillWeaponInfo();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CGrenadeToHitCalcer::GetStance()
{
	int nStance = 0; 
	if ( eCurPose == NAI::CROUCH  )
		nStance = int( pGrenade->pWeaponType->fCrouchBonus );
	else if ( eCurPose == NAI::CRAWL ) 
		nStance = int( pGrenade->pWeaponType->fCrawlBonus );
	return nStance;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGrenadeToHitCalcer::FillWeaponInfo()
{
	CToHitCalcer::FillWeaponInfo();
	sWeaponInfo.nQuality = pGrenade->nQuality;
	sWeaponInfo.nMinRange = GetGrenadeMaxDistance();
	sWeaponInfo.nMaxRange = sWeaponInfo.nMinRange;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CGrenadeToHitCalcer::GetMaxImp()
{
	float fBaseStr = float( pUnitMission->GetRPGUnit()->Skills( NDb::ST_STR ) );
	if ( pUnitMission->GetPanzerklein() )
		fBaseStr = pUnitMission->GetPanzerklein()->nGrenadeStrength;
	float fStr = fBaseStr + float(nSkill) / 18.f;
	float fRes = pUnitMission->GetToHitConstants()->fGrenadeBaseCoeff;
	fRes += pUnitMission->GetToHitConstants()->fGrenadeSTRCoeff * fStr;
	fRes *= pow( float(pGrenade->pItem->nWeight), 0.74f );
	return fRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CGrenadeToHitCalcer::GetRelWeight()
{
/*	NDb::SToHitConstants *c = pUnitMission->GetToHitConstants();

	float fRes = Min( float(pGrenade->pItem->nWeight - c->fGrenadeBaseWeight),
		float(c->nGrenadeWeightScalingBase) ) / c->nGrenadeWeightScaling;
	fRes = pow( c->nGrenadeWeightScalingBase, fRes );
	return fRes;*/
	return float(pGrenade->pItem->nWeight) / 1000.f; // т.е. в киллограммах
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CGrenadeToHitCalcer::GetMaxGrenadeVelocity()
{
	return GetMaxImp() / GetRelWeight();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CGrenadeToHitCalcer::GetGrenadeMaxDistance()
{
	float fMaxImp = GetMaxImp();
	float fRelWeight = GetRelWeight();
	return int( pow(fMaxImp,2) / ( pUnitMission->GetToHitConstants()->fGravity * pow(fRelWeight,2) ) / FP_GRID_STEP );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CGrenadeToHitCalcer::GetAllAdd()
{
	return sWeaponInfo.nQuality + GetD2() + GetStance() - GetSMove();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CGrenadeToHitCalcer::GetToHit()
{
	Prepare();
	fToHit = GetAllMult();
	fToHit = Clamp( fToHit, 2.0f, 100.0f );
	return fToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGrenadeToHitCalcer::Log()
{
	csRPG << "<font size=16pt>";
	csRPG << CC_GREEN << " \tGrenade ToHit: ";
	csRPG << CC_ORANGE << "\tSkill=" << CC_GREY << pUnitMission->GetRPGUnit()->Skills( NDb::ST_THROWING );
	csRPG << CC_ORANGE << "\tDist=" << CC_GREY << nDistance;
	csRPG << CC_ORANGE << "\tMaxGrenadeRange=" << CC_GREY << GetGrenadeMaxDistance();
	csRPG << CC_ORANGE << "\tW+=" << CC_GREY << sWeaponInfo.nQuality;
	csRPG << CC_ORANGE << "\tStance=" << CC_GREY << GetStance();
	csRPG << CC_ORANGE << "\tSMvDist=" << CC_GREY << pUnitMission->GetMoveInLastTurn();
	csRPG << endl;
	csRPG << "<font size=16pt>";
	csRPG << CC_ORANGE << "\tDerSkill=" << CC_GREY << nSkill;
	csRPG << CC_ORANGE << "\tD1=" << CC_GREY << GetD1();
	csRPG << CC_ORANGE << "\tD2=" << CC_GREY << GetD2();
		csRPG << CC_ORANGE << "\tLight=" << CC_GREY << GetLight();
	csRPG << CC_ORANGE << "\tFRmult=" << CC_GREY << GetFRMult();
	csRPG << CC_ORANGE << "\tAllAdd=" << CC_GREY << GetAllAdd();
	csRPG << CC_ORANGE << "\tAllAddMult=" << CC_GREY << GetAllMult();
	csRPG << CC_GREEN << "\t ToHit = " << fToHit << "\n";
	csRPG << CC_ORANGE << "\tMaxImpulse = " << GetMaxImp() << " MaxVelocity = " << GetMaxGrenadeVelocity() << "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMeleeToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
inline float CMeleeToHitCalcer::GetShooterDerSkill()
{
	return pShooter->GetRPGUnit()->Skills( NDb::ST_MELEE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline float CMeleeToHitCalcer::GetTargetDerSkill()
{
	return pTarget->GetRPGUnit()->Skills( NDb::ST_MELEE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline float CMeleeToHitCalcer::GetShooterDWSkill()
{
	return pShooter->GetRPGUnit()->Skills( NDb::ST_MELEE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline float CMeleeToHitCalcer::GetTargetDWSkill()
{
	return pTarget->GetRPGUnit()->Skills( NDb::ST_MELEE );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CMeleeToHitCalcer::GetToHit()
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIUnitToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIUnitToHitCalcer::CAIUnitToHitCalcer(	NAI::IAIUnit *pShooter, const NAI::SUnitPosition &shooterPos, 
	NAI::IAIUnit *pTarget, int nHitCover, NAI::EHitLocation _eHitLocation, int _nBullet, IInventoryItem *_pWeapon, int _nExtraAP ):
		CUnitToHitCalcer( pShooter->GetUnitServer()->GetUnitRPG(),
		shooterPos.GetPose(), 
		fabs( shooterPos.GetCP() - pTarget->GetPosition().GetCP() ) / FP_GRID_STEP,
		shooterPos.GetEyePosition(), 
		pTarget->GetPosition(),
		_nExtraAP, 0, nHitCover, false, CVec3( 1, 1, 1 ),
		_eHitLocation,
		pTarget->GetUnitServer()->GetUnitRPG(),
		_nBullet, false ) 
{
	CDynamicCast<CWeaponItem> pWeapon( _pWeapon );
	pWeaponItem = pWeapon;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIUnitToHitCalcer::GetTArea()
{
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIUnitToHitCalcer::GetTMove()
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIUnitNoWeaponToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIUnitNoWeaponToHitCalcer::CAIUnitNoWeaponToHitCalcer( NAI::IAIUnit *pShooter, NAI::IAIUnit *pTarget	):
	CAIUnitToHitCalcer( pShooter, pShooter->GetUnitPosition(), pTarget, 100, NAI::HL_ANY, 0, 0, 0 )
{
	sWeaponInfo.nQuality = 0;
	sWeaponInfo.nMinRange = 0xFFFF - 1;
	sWeaponInfo.nMaxRange = 0xFFFF;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIUnitNoWeaponToHitCalcer::Prepare()
{
	nSkill = pUnitMission->GetRPGUnit()->Skills( NDb::ST_SHOOTING );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIUnitNoWeaponToHitCalcer::GetD1()
{
	return nSkill;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIUnitNoWeaponToHitCalcer::GetRS()
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIUnitNoWeaponToHitCalcer::GetTMove()
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIUnitNoWeaponToHitCalcer::GetTArea()
{
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CAIUnitNoWeaponToHitCalcer::GetLight()
{
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CRLauncherToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CRLauncherToHitCalcer::CRLauncherToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose,
	int _nDistance,	CVec3 _ptAttacker, int _nExtraAP, bool _bFirstRound,	
	CVec3 _ptIllumination, NAI::ETileHitLocation _eHitLocation, CVec3 _ptTilePos ) :
		CToHitCalcer( _pUnitMission, _eCurPose, _nDistance, _ptAttacker, _nExtraAP, 0, 100, 
			_bFirstRound,	_ptIllumination, 1, false ), eHitLocation(_eHitLocation), ptTilePos(_ptTilePos)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CRLauncherToHitCalcer::GetMaxDistance()
{
	return pWeaponItem->GetDBWeapon()->nMaxRange;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CThrowKnifeToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
int CThrowKnifeToHitCalcer::GetKnifeMaxDistance()
{
	return pUnitMission->GetRPGUnit()->Skills( NDb::ST_STR ) * 1.5f;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CThrowKnifeToHitCalcer::FillWeaponInfo()
{
	CToHitCalcer::FillWeaponInfo();
	sWeaponInfo.nQuality = 0;
	sWeaponInfo.nMinRange = 1;
	sWeaponInfo.nMaxRange = 15;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CThrowKnifeToHitCalcer::Prepare()
{
	FillWeaponInfo();
	nSkill = pUnitMission->GetRPGUnit()->Skills( NDb::ST_THROWING );
	float fVPPenalty = GetVPPenalty( pUnitMission->GetRPGUnit()->Skills( NDb::ST_VP ), 
		pUnitMission->GetHealedVP(), pUnitMission->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue() );
	nSkill *= fVPPenalty;
//	fMovePenalty = pGrenade->pWeaponType->fMovePenalty;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CThrowKnifeToHitCalcer::CThrowKnifeToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
	CVec3 _ptAttacker, bool _bFirstRound,	CVec3 _ptIllumination ):
CToHitCalcer( _pUnitMission, _eCurPose, _nDistance, _ptAttacker, 0, 0, 100, 
			_bFirstRound,	_ptIllumination, 1, false )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CThrowKnifeTileToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
CThrowKnifeTileToHitCalcer::CThrowKnifeTileToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose,
	int _nDistance,	CVec3 _ptAttacker, bool _bFirstRound,	CVec3 _ptIllumination, NAI::ETileHitLocation _eHitLocation,
	CVec3 _ptTilePos ) :
		CThrowKnifeToHitCalcer( _pUnitMission, _eCurPose, _nDistance, _ptAttacker,  
			_bFirstRound,	_ptIllumination ), eHitLocation(_eHitLocation), ptTilePos(_ptTilePos)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NRPG;
//
REGISTER_SAVELOAD_CLASS( 0x52132140, CUnitToHitCalcer );
REGISTER_SAVELOAD_CLASS( 0x52132180, CObjectToHitCalcer );
REGISTER_SAVELOAD_CLASS( 0x52232150, CTileToHitCalcer );
REGISTER_SAVELOAD_CLASS( 0x52232160, CGrenadeToHitCalcer );
REGISTER_SAVELOAD_CLASS( 0x51642170, CMeleeToHitCalcer );
REGISTER_SAVELOAD_CLASS( 0x51262180, CAIUnitToHitCalcer );
REGISTER_SAVELOAD_CLASS( 0x51462140, CAIUnitNoWeaponToHitCalcer );
REGISTER_SAVELOAD_CLASS( 0x72762140, CRLauncherToHitCalcer );
