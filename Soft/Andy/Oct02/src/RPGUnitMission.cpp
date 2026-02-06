#include "StdAfx.h"
#include "RPGGlobal.h"
#include "RPGUnitInfo.h"
#include "RPGMerc.h"
#include "RPGItemSet.h"
#include "RPGAttackMech.h"
#include "Grid.h"
#include "A5Script.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataAI.h"
#include "..\Misc\RandomGen.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\LogStream.h"
#include "aiPosition.h"
#include "RPGCritical.h"
#include "RPGToHit.h"
#include "wAckBase.h"
#include "RPGDiplomacy.h"

#include "RPGUnitMission.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SCriticalType
{
	int nStartPos;					// начало диапазона в котором ролится данное критическое повреждение
	CDBPtr<NDb::CRPGCritical> pCritical;
	SCriticalType( int nStart, NDb::CRPGCritical *p ): nStartPos(nStart), pCritical(p) {}
};
typedef vector<SCriticalType> TCriticalSet;
static vector<TCriticalSet> criticalBar( NDb::N_CL ); //[NDb::N_CL];
static bool bCBarInitialized = false;
int Round( float f ) { return int( f + 0.5f ); }
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitMission
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitMission: public IUnitMission, public IAttackable
{
	friend class CUnitToHitCalcer;
	friend class CObjectToHitCalcer;
	friend class CTileToHitCalcer;
	friend class CGrenadeToHitCalcer;
	friend class CRLauncherToHitCalcer;

	OBJECT_BASIC_METHODS(CUnitMission);
private:
	void UseSkill( NDb::ESkillType eSkill );
	bool ApplyCritical( NDb::CRPGCritical *pCritical, int nDC );
	bool ProcessCriticals(); // every turn
	float GetCriticalDmgModifier( NAI::EHitLocation eHL, int nCriticalProbability, int nCriticalDifficulty );
	int  GetTotalVP() const { return pRPGUnit->Skills( NDb::ST_VP ) + GetHealedVP(); }
	int  GetMeleeToHit( const CVec3 &ptAttacker, const NAI::SPosition &posTarget, IUnitMissionInfo *pTarget, NAI::EHitLocation hl, const vector<int> &accessibleHLs ) const;
	int CalcInterruptProbability( const IUnitMission *pEnemy,	bool bIsMutual, bool bWasShot );

	virtual int& GetHealedVP() const { return pRPGUnit->nHealedVP; }
	virtual int GetLastActionTimes() const { return nLastActionTimes; }
	virtual int GetMoveInLastTurn() const { return nMoveInLastTurn; }
	virtual int RollCritical( NAI::EHitLocation eHL, int nCriticalProbability, 
		int nCriticalDifficulty, NDb::CRPGCritical **pCritical );
	void SaveAck( int nAckID, IUnitMissionInfo *pAttacker );
	void ResetUnitParameters();
	int GetSnipeAP( IUnitMissionInfo *pTarget ) const;
	int GetUnconsciousProbability( IUnitMissionInfo *pAttacker, 
		IUnitMissionInfo *pTarget, int nBaseProbability ) const;

	ZDATA
	int nMoveInLastTurn;
	EAction eLastAction;
	int nLastActionTimes;
	vector<CPtr<CCritical> > criticals;
	int _nShameOnEpik;
	bool bLogActive;
	bool bSitting;
	int  nBullet;
	SSnipeAP savedSnipeAP;
	float fLastToHit; // for Burst ToHit
	NDb::ECritical eLastCritical;
	bool bUseTwoHanded;
	float fLightPerception;
	float fSoundPerception;
	NDb::SToHitConstants tohit;
	NDb::SAISoundConstants sAISoundConstants; // константы AI sound
	NDb::SInterruptsConstants SInterruptsConstants; // константы Interrupt-ов
	CVec3 ptLastCP;
	vector<ECriticalState> vCriticalsState; // для блокировки критикалов и reroll-а
public:
	CPtr<NDb::CModel> pModel;
	CObj<CUnit> pRPGUnit;
	wstring sID;
private:
	int nBulletHitThisTurn; // for criticals
	list< CPtr<IUnitMissionInfo> > AckAttackers;
	list<int> AckIDs;
	int nScenarioPlayer;
	CObj<CDiplomacy> pDiplomacy;
	bool bUnconscious;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nMoveInLastTurn); f.Add(3,&eLastAction); f.Add(4,&nLastActionTimes); f.Add(5,&criticals); f.Add(6,&_nShameOnEpik); f.Add(7,&bLogActive); f.Add(8,&bSitting); f.Add(9,&nBullet); f.Add(10,&savedSnipeAP); f.Add(11,&fLastToHit); f.Add(12,&eLastCritical); f.Add(13,&bUseTwoHanded); f.Add(14,&fLightPerception); f.Add(15,&fSoundPerception); f.Add(16,&tohit); f.Add(17,&sAISoundConstants); f.Add(18,&SInterruptsConstants); f.Add(19,&ptLastCP); f.Add(20,&vCriticalsState); f.Add(21,&pModel); f.Add(22,&pRPGUnit); f.Add(23,&sID); f.Add(24,&nBulletHitThisTurn); f.Add(25,&AckAttackers); f.Add(26,&AckIDs); f.Add(27,&nScenarioPlayer); f.Add(28,&pDiplomacy); f.Add(29,&bUnconscious); return 0; }
		//	CPtr<CMerc> pMerc;
		
	//
	CUnitMission( int _nScenarioPlayer = 0 );
	// EOCRAP
	//
	virtual void GetInfo( NAI::EPose pose, SUnitInfo *pInfo ) const;
	virtual bool IsDead() const;
	virtual NDb::EWeaponType GetWeaponType() const;
	virtual NDb::CAnimWeaponType* GetAnimWeaponType() const;
	virtual int  GetActionAP( NAI::EPose pose, EAction action ) const;
	virtual int GetAP() const;
	virtual bool CanSpendAP( int nAP ) const;
	virtual void SpendAP( int nAP );
	virtual void RegisterAction( EAction action );
	virtual const SSnipeAP& GetSavedAP() const { return savedSnipeAP; }
	virtual void SaveAP( const SSnipeAP &ap );
	virtual float GetSightDistance( NAI::EPose pose ) const;
	virtual void StartNewTurn( const CVec3 &ptCP );

	virtual bool CreateAttack( vector<CAttackPortion> *pRes, bool bSpendAmmo, 
		IUnitMissionInfo *pAttacker, IUnitMissionInfo *pTarget ) const;
	virtual int ProcessAttack( int nUserID, CAttackPortion *pAttack, NDb::CRPGArmor *pArmor );

	virtual int GetToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, const NAI::SPosition &posTarget, 
		NAI::EHitLocation hl, int nExtraAP, IUnitMissionInfo *pTarget, 
		const vector<int> &accessibleHLs, int nHitCover, bool bFirstRound, const CVec3 &ptIllumination ) const;
	virtual int GetTileToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, CVec3 ptTilePos, 
		NAI::ETileHitLocation eHitLocation, int nExtraAP, int nHitCover, bool bFirstRound, const CVec3 &ptIllumination );
	virtual int GetObjectToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, 
		int nExtraAP, int nHitCover, bool bFirstRound, const CVec3 &ptIllumination ) ;
	virtual int GetGrenadeToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, 
		bool bFirstRound, CVec3 ptTilePos, NDb::CRPGGrenade *pGrenade, const CVec3 &ptIllumination );
	virtual int GetRLauncherToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, CVec3 ptTilePos, 
		NAI::ETileHitLocation eHitLocation, int nExtraAP, bool bFirstRound, const CVec3 &ptIllumination );


	virtual int  GetBulletsQuantityInShot() const;
	virtual void CreateFirstAid( SFirstAid *pRes, int nSpentAP ) const;
	virtual void Heal( const SFirstAid &fa );
	virtual bool CanHeal( IUnitMissionInfo *pTarget ) const;
	virtual int  GetIC() const;
	virtual bool CheckIC();
	virtual int CheckInterrupt( const IUnitMission *pEnemy, bool bIsMutual, bool bWasShot );
	virtual int  GetInterrupt() const { return pRPGUnit->Skills(NDb::ST_INTERRUPT); }
	virtual NDb::CRPGArmor* GetRPGArmor() const;
	virtual NDb::CModel* GetModel() const { return pRPGUnit->pModel; }
	virtual wstring GetName() const;
	virtual void Kill();
	virtual IInventory* GetInventory() const { return pRPGUnit->GetInventory(); };
	virtual IInventoryInfo* GetInventoryInfo() const { return pRPGUnit->GetInventory(); };
	virtual int GetMaxExtraAP() const;
	virtual int GetSkillValue( NDb::ESkillType skill ) const { return pRPGUnit->Skills( skill ); }
	virtual int GetSkillMaxValue( NDb::ESkillType skill ) const { return pRPGUnit->Skills( skill ).GetMaxValue(); }
	virtual float GetSkillProgress( NDb::ESkillType skill ) const { return pRPGUnit->Skills( skill ).GetProgress(); }
	virtual void DumpStats() const;
	virtual void PrintLog( bool bPrint ) { bLogActive = bPrint; }
	virtual void Seat() { bSitting = true; }
	virtual void Stand() { bSitting = false; }
	virtual bool CanMove() const { return !bSitting; }
	virtual void Reload();
	virtual void Unload( IInventoryItem *pItem );
	virtual void StartAttack() { nBullet = 0; }
	virtual void NextBullet() { ++nBullet; }
	virtual int  GetNBullets() const { return nBullet; }
	virtual void SetLastCritical( NDb::ECritical eCA ) { eLastCritical = eCA; }
	virtual NDb::ECritical GetLastCritical();
	virtual void UseTwoHanded( bool bUse ) { bUseTwoHanded = bUse;}
	virtual bool CanUseTwoHanded() const { return bUseTwoHanded; }
	virtual void Blind( bool bBlind ) { bBlind ? fLightPerception = 0 : fLightPerception = 1; }
	virtual void Deaf( bool bDeaf ) { bDeaf ? fSoundPerception = 0 : fSoundPerception = 1; }
	virtual int GetRPGPersID() const { return pRPGUnit->nRPGPersID; }
	virtual void SetCannonItem( IWeaponItem *pItem );
	virtual IWeaponItem* GetCannonItem() const { return pRPGUnit->GetCannonItem(); }
	virtual IWeaponItem* GetWeaponItem() const { return pRPGUnit->GetWeaponItem(); }
	virtual IWeaponItemInfo* GetWeaponItemInfo() const { return pRPGUnit->GetWeaponItem(); }
	virtual IWeaponItemInfo* GetCannonItemInfo() const { return pRPGUnit->GetCannonItem(); }
	virtual bool CanHearSound( const CVec3 &ptSoundPosition, const CVec3 &ptListenerPosition,
		NDb::CAISound *pSound, int nAISoundType, IUnitMission *pSource );
	virtual CVec3 GetTurnStartCP() const { return ptLastCP; }
	virtual NDb::CRPGPers* GetRPGPers() { return pRPGUnit->pPers; }
	virtual CUnit *GetRPGUnit() const { return pRPGUnit; }
	virtual NDb::SToHitConstants *GetToHitConstants() { return &tohit; }
	virtual NDb::SAISoundConstants *GetAISoundConstants() { return &sAISoundConstants; }
	virtual NDb::SInterruptsConstants *GetInterruptsConstants() { return &SInterruptsConstants; }
	virtual bool HasCritical( NDb::ECritical eCritical ) const;
	virtual bool ApplyCritical( SCritical &critical );
	virtual void RemoveCritical( NDb::ECritical eCritical );
	virtual void MakeDirectDamage( int nDmg );
	virtual void EnableCriticals();
	virtual void DisableCriticals();
	virtual void DisableCritical( NDb::ECritical eC, ECriticalState eState  );
	virtual void BulletHit() { ++nBulletHitThisTurn; }
	virtual bool GetAck( int *pAckID, IUnitMissionInfo ** ppAttacker );
	virtual int  GetXP( int nHowManyPerson ) const;
	virtual void StartRealTime();
	virtual int GetScenarioPlayer() const { return nScenarioPlayer; }
	virtual CDiplomacy *GetDiplomacy() const { return pDiplomacy; }
	virtual bool IsUnconscious() { return bUnconscious; }
	virtual void InitAsCorpse( bool bDead );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool DBCriticalCmp( const SCriticalType &a, const SCriticalType &b )
{
	return a.nStartPos < b.nStartPos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// размещаем критикалы в списке в соотвествии с заданной пользователем очередностью
void RangeCriticals( vector<SCriticalType> *pCriticals )
{
	if ( pCriticals->empty() )
		return;
	sort( pCriticals->begin(), pCriticals->end(), DBCriticalCmp );
	//
	int pos = 0;
	for ( vector<SCriticalType>::iterator i = pCriticals->begin(); i != pCriticals->end(); ++i )
	{
		i->nStartPos = pos;
		pos += i->pCritical->nRange;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void InitializeCriticals()
{
	CDBTable<NDb::CRPGCritical> *pDTable = NDatabase::GetTable<NDb::CRPGCritical>();
	CDBIterator<NDb::CRPGCritical> it(*pDTable);
	while ( it.MoveNext() )
	{
		NDb::CRPGCritical *pC = it.Get();
		if ( !pC )
			continue;
		ASSERT( pC->hl < NDb::N_CL );
		if ( pC->hl >= NDb::N_CL )
			continue;
		criticalBar[pC->hl].push_back( SCriticalType( pC->nWeight, pC ) );
	}
	for ( int i = 0; i < NDb::N_CL; ++i )
		RangeCriticals( &criticalBar[i] );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitMission::CUnitMission( int _nScenarioPlayer ) 
	: nMoveInLastTurn(0), nLastActionTimes(0), bLogActive(true), 
		bSitting(false), nBullet(-1), fLastToHit(0), eLastCritical(NDb::C_NONE), 
		bUseTwoHanded(true), fLightPerception(1), fSoundPerception(1), nBulletHitThisTurn(0),
		nScenarioPlayer( _nScenarioPlayer ), bUnconscious( false )
{
	NDb::CRPGToHit *pToHit = NDb::GetToHitConstants( 1 );
	if ( pToHit )
		tohit = pToHit->constants;
	NDb::CRPGAISoundConstants *pAISoundConstants = NDb::GetAISoundConstants( 1 );
	if ( pAISoundConstants )
		sAISoundConstants = pAISoundConstants->constants;
	NDb::CRPGInterruptsConstants *pInterruptsConstants = NDb::GetInterruptsConstants( 1 );
	if ( pInterruptsConstants )
		SInterruptsConstants = pInterruptsConstants->constants;

	vCriticalsState.resize( NDb::N_CRIT_TYPES );
	EnableCriticals();
	pDiplomacy = new CDiplomacy();
}
wstring CUnitMission::GetName() const
{
	NDb::CString *pDisplayName = pRPGUnit->pPers->pName;
	if ( pDisplayName )
		return pDisplayName->szStr;
	else
		return sID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::InitAsCorpse( bool bDead )
{
	bUnconscious = !bDead;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::SaveAck( int nAckID, IUnitMissionInfo *pAttacker )
{
	AckAttackers.push_back( pAttacker );
	AckIDs.push_back( nAckID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::GetAck( int *pAckID, IUnitMissionInfo **ppAttacker )
{
	if ( AckAttackers.empty() )
		return false;
	//
	*pAckID = AckIDs.front();
	AckIDs.pop_front();
	*ppAttacker = AckAttackers.front().Extract();
	AckAttackers.pop_front();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::UseSkill( NDb::ESkillType eSkill )
{
	if ( !pRPGUnit->UseSkill(eSkill) )
		return;
	SaveAck( NWorld::N_ACK_SKILL, 0 );
	csRPG << "<font size=16pt>";
	csRPG << "<color=yellow>" << GetName() << " improve skill N" << eSkill << " to " << pRPGUnit->Skills(eSkill) << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::GetInfo( NAI::EPose pose, SUnitInfo *pInfo ) const
{
	pInfo->nAP = pRPGUnit->Skills(NDb::ST_AP);
	pInfo->nMaxAP = pRPGUnit->Skills(NDb::ST_AP).GetMaxValue();
	pInfo->nHP = pRPGUnit->Skills(NDb::ST_VP);
	pInfo->nMaxHP = pRPGUnit->Skills(NDb::ST_VP).GetMaxValue();
	pInfo->nSightDistance = int( GetSightDistance( pose ) );
	pInfo->nHealedHP = GetHealedVP();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetSnipeAP( IUnitMissionInfo *pTarget ) const
{
	if ( savedSnipeAP.pTarget == pTarget )
		return savedSnipeAP.nAP;
	else
		return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::IsDead() const
{
	return pRPGUnit->Skills(NDb::ST_VP) <= 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::MakeDirectDamage( int nDmg )
{
	pRPGUnit->Skills(NDb::ST_VP) -= nDmg;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::RegisterAction( EAction action )
{
	if ( action == AC_PREPARE_AND_SHOOT )
	{
		action = AC_SHOOT;
		nLastActionTimes = 0;
	}
    eLastAction = action;

	if ( action != AC_END_SHOOT && action != AC_SHOOT && action != AC_BURST )
		nLastActionTimes = 0;

	switch( action )
	{
		case AC_END_SHOOT:
			nLastActionTimes++;
			break;
		case AC_MELEE:
			pRPGUnit->UseSkill( NDb::ST_MELEE );
			break;
		case AC_THROW_GRENADE:
		case AC_THROW_KNIFE:
			pRPGUnit->UseSkill( NDb::ST_THROWING );
			break;
		case AC_SHOOT:
		case AC_PREPARE_AND_SHOOT:
			pRPGUnit->UseSkill( NDb::ST_SHOOTING );
			break;
		case AC_BURST:
			pRPGUnit->UseSkill( NDb::ST_BURST );
			break;
		case AC_MOVE_DIAGONAL:
			nMoveInLastTurn += 1;
		case AC_MOVE_SIDE:
			nMoveInLastTurn += 2;
			pRPGUnit->UseSkill( NDb::ST_AP );
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::SpendAP( int nAP )
{
	ASSERT( pRPGUnit->Skills(NDb::ST_AP) >= nAP );
	pRPGUnit->Skills(NDb::ST_AP) -= nAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::CanSpendAP( int nAP ) const
{
	return pRPGUnit->Skills(NDb::ST_AP) >= nAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetAP() const
{
	return pRPGUnit->Skills(NDb::ST_AP);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CUnitMission::GetSightDistance( NAI::EPose pose ) const
{
	float fRetDist = 0;
	//NAI::EPose pose = pUnit->GetPosition().GetPose();
	switch ( pose )
	{
		case NAI::CRAWL: fRetDist = 25; break;
		case NAI::CROUCH: fRetDist = 27; break;
		case NAI::WALK: fRetDist = 30; break;
		case NAI::RUN: fRetDist = 30; break;
	}
	return fRetDist * FP_GRID_STEP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::ResetUnitParameters()
{
	nBulletHitThisTurn = 0;
	pRPGUnit->Skills(NDb::ST_AP).Reset();
	nMoveInLastTurn = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::StartNewTurn( const CVec3 &ptCP )
{
	ResetUnitParameters();
	ProcessCriticals();
	ptLastCP = ptCP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::StartRealTime()
{
	ResetUnitParameters();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetActionAP( NAI::EPose curPose, EAction action ) const
{
	// действия стоимость которых не зависит от позы
	switch ( action )
	{
		case AC_NONE: return 0;
		case AC_PREPARE: return 2;
		case AC_SHOOT:   return pRPGUnit->GetWeaponAP();
		case AC_PREPARE_AND_SHOOT:	return GetActionAP( curPose, AC_PREPARE ) + GetActionAP( curPose, AC_SHOOT );
		case AC_EXPLODE: return 2;
		case AC_ROTATE: return 2;
		case AC_THROW_GRENADE: return 20; // CRAP
		case AC_CLIMB_1: return 10;
		case AC_CLIMB_2: return 12;
		case AC_CLIMB_3: return 14;
		case AC_CLIMB_4: return 15;
		case AC_JUMP: return 4;
		case AC_TAKE_CORPSE: return 12;
		case AC_THROW_KNIFE: return 16; // CRAP
		case AC_FIRSTAID: 
			return 15;
		case AC_MELEE:
			{
				CMeleeWeaponItem *pMelee = pRPGUnit->GetMeleeWeaponItem();
				if ( !pMelee )
					return 0;
				NDb::CRPGMeleeWeapon *pW = pMelee->GetDBMeleeWeapon();
				return pW->nMaxAP - (pW->nMaxAP - pW->nMinAP) * pRPGUnit->Skills(NDb::ST_MELEE) / N_MAX_SKILL;
			}
		case AC_BURST:
			return pRPGUnit->GetWeaponBurstAP();
		case AC_RELOAD:
			return pRPGUnit->GetWeaponReloadAP();

		case AC_OPEN_CLOSE:
			return 4;
		case AC_APPROACH_CANNON:
			return 4;

		case AC_POSE_CRAWL:
		case AC_POSE_CROUCH:
		case AC_POSE_WALK:
		case AC_POSE_RUN:
		{
			if ( action == AC_POSE_RUN )
				action = AC_POSE_WALK;
			if ( curPose == NAI::RUN )
				curPose = NAI::WALK;
			return abs( curPose - ( action - AC_POSE_CRAWL ) ) * 4;
		}
		case AC_LADDER:
			return 6;
		case AC_LADDER_MOVE:
			return 6;
		case AC_END_SHOOT:
			return 0;
	}
	// действия стоимость которых зависит от позы
	switch ( curPose )
	{
		case NAI::CRAWL:
		case NAI::CROUCH:
		case NAI::WALK:
		case NAI::RUN:
		{
			int nPose = ( NAI::RUN - curPose + 1 ) * 2;
			switch ( action )
			{
				case AC_MOVE_SIDE:		return nPose;
				case AC_MOVE_DIAGONAL:	return ( nPose * 15 ) / 10;
				case AC_MOVE_CORPSE_SIDE:		return ( nPose * 15 ) / 10;
				case AC_MOVE_CORPSE_DIAGONAL:	return ( nPose * 225 ) / 100;
			}
		}
		break;
	}
	ASSERT(0); // unknown action
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CAnimWeaponType* CUnitMission::GetAnimWeaponType() const
{
	return pRPGUnit->GetAnimWeaponType();
}
NDb::EWeaponType CUnitMission::GetWeaponType() const 
{
	return pRPGUnit->GetWeaponType();
}
static void DumpAttackPortion( const CAttackPortion &ap )
{
	csRPG << "<font size=16pt>";
	csRPG << CC_ORANGE << " \tK="  << CC_GREY << ap.nK;
	csRPG << CC_ORANGE << " \tType="    << CC_GREY << ap.nDmgType;
	csRPG << CC_ORANGE << " \tDmgMin=" << CC_GREY << ap.nDmgMin;
	csRPG << CC_ORANGE << " \tDmgMax=" << CC_GREY << ap.nDmgMax;
	csRPG << CC_ORANGE << " \tCrit="  << CC_GREY << ap.nCrtical;
	csRPG << CC_ORANGE << " \tCritSev=" << CC_GREY << ap.nCrticalDifficulty;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetUnconsciousProbability( IUnitMissionInfo *pAttacker, 
	IUnitMissionInfo *pTarget, int nBaseProbability ) const
{
	if ( !IsValid( pTarget ) )
		return nBaseProbability;
	//
	int nHits = pTarget->GetSkillValue( NDb::ST_VP );
	int nMaxHits = pTarget->GetSkillMaxValue( NDb::ST_VP );
	return ( - 1.f / ( 2.f * nMaxHits ) * nHits + 1 ) * nBaseProbability;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Возвр. true, если все выстрел завершен (во время очереди он может быть завершен не сразу)
bool CUnitMission::CreateAttack( vector<CAttackPortion> *pRes, bool bSpendAmmo, 
		IUnitMissionInfo *pAttacker, IUnitMissionInfo *pTarget ) const
{
	bool bRet = true;
	CWeaponItem *pWeapon  = pRPGUnit->GetWeaponItem();
	CMeleeWeaponItem *pMW = pRPGUnit->GetMeleeWeaponItem();
	//
	if ( IsValid( pWeapon ) )
	{
		if ( pWeapon->GetDBWeapon()->pWeaponType->bTwoHanded && !CanUseTwoHanded() )
		{
			// мы сейчас не можем использовать двуручное оружие
			return true;
		}
		switch ( pWeapon->GetShootMode() )
		{
		case NDb::SM_ShortBurst:
				{
					const int nShoots = pWeapon->GetDBWeapon()->nRoF / 6;
					if ( nBullet < nShoots )
						bRet = false;
				}
				break;
		case NDb::SM_LongBurst:
				bRet = false;
				break;
		}
		pWeapon->CreateNewAttackPortion( pRes, bSpendAmmo );
		for ( int i = 0; i < pRes->size(); ++i )
		{
			int nSnipeSkill = pRPGUnit->Skills( NDb::ST_SNIPE );
			CAttackPortion &a = (*pRes)[i];
			a.pAttacker = pAttacker;
			a.pTarget = pTarget;
			a.nCrtical = 10.f * ( nBulletHitThisTurn + 1 );
			a.nCrticalDifficulty = nSnipeSkill / 10.f;
			a.nUnconsciousProbability = GetUnconsciousProbability( pAttacker, 
				pTarget, pWeapon->GetInnerClip()->GetDBAmmo()->nUnconsciousProbability );
			//
			if ( IsValid(savedSnipeAP.pTarget) && pTarget == savedSnipeAP.pTarget )
			{
				// снайперский выстрел
				a.nCrtical += savedSnipeAP.nAP / 5.f * 3;
				a.nCrticalDifficulty += savedSnipeAP.nAP / 5.f * 3;
				if ( bSpendAmmo )
					pRPGUnit->UseSkill( NDb::ST_SNIPE );
			}
			else
			{
				// обычный выстрел
				a.nCrtical += nSnipeSkill / 10;
			}
		}
	}
	else if ( pMW )
	{
		NDb::CRPGMeleeWeapon *pW = pMW->GetDBMeleeWeapon();
				pRes->push_back( CAttackPortion( 110, 2, 0, 0, 0, 0 ) ); // константы сказал Epik
		CAttackPortion &a = pRes->front();
		const int nStr = pRPGUnit->Skills( NDb::ST_STR );
		const int nMelee = pRPGUnit->Skills( NDb::ST_MELEE );
		//
		a.nDmgMin = pW->nDmgMin + nStr + nMelee * (pW->nDmgMax - pW->nDmgMin) / (N_MAX_SKILL * 2);
		a.nDmgMax = pW->nDmgMax + nStr;
		a.nK = 160 + 10 * pRPGUnit->Skills( NDb::ST_STR );
		a.nCrtical = 10 + 0.4f * (nMelee - 25) + pMW->GetDBMeleeWeapon()->nCriticalBonus;
		a.nCrtical = Max( 0, a.nCrtical );
		a.nCrticalDifficulty = 0.5f * a.nCrtical;
		a.pAttacker = pAttacker;
		a.pTarget = pTarget;
		a.nUnconsciousProbability = GetUnconsciousProbability( pAttacker, 
			pTarget, pMW->GetDBMeleeWeapon()->nUnconsciousProbability );
	}
	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CRPGArmor* CUnitMission::GetRPGArmor() const 
{
	return NDb::GetArmor( NDb::CRPGArmor::HUMAN_BODY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetXP( int nHowManyPerson ) const
{
	nHowManyPerson = Clamp( nHowManyPerson, 0, 7 );
	float EpikCheatCoeff[] = { 0, 0.8f, 0.5f, 0.5f, 0.4f, 0.3f, 0.2f, 0.2f };
	int nLvl = pRPGUnit->Skills(NDb::ST_LEVEL);
	if ( nLvl == 0 )
		nLvl = 1;
	int nXPDiff = pRPGUnit->GetXPForSkill( NDb::ST_LEVEL, nLvl ) - pRPGUnit->GetXPForSkill( NDb::ST_LEVEL, nLvl - 1 );
	return Round( EpikCheatCoeff[nHowManyPerson] * float(nXPDiff) / 5.f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::ProcessAttack( int nUserID, CAttackPortion *pAttack, NDb::CRPGArmor *pArmor )
{
	int nTotalDmg = 0;
	bool bAlive = pRPGUnit->Skills(NDb::ST_VP) > 0;
	if ( pAttack->CanDealDmg(pArmor) )
	{
		csRPG << "<font size=16pt>";
		// А может я увернулся?
		if ( this != pAttack->pTarget && CheckIC() )
		{
			csRPG << CC_RED << " damage avoided!" << endl;
			return -1;
		}
		int nDmg = 0;
		if ( pAttack->nDmgMax > pAttack->nDmgMin )
			nDmg = random.Get( pAttack->nDmgMin, pAttack->nDmgMax );
		else
			nDmg = pAttack->nDmgMax;
		nDmg *= pAttack->fDamageCoeff;
//	Get
		float fDmgModifier = 0;
		switch ( nUserID )
		{
			case NAI::HL_HEAD:
				fDmgModifier = 1.2f;
				break;
			case NAI::HL_RHAND:
			case NAI::HL_LHAND:
				fDmgModifier = 0.7f;
				break;
			case NAI::HL_RLEG:
			case NAI::HL_LLEG:
				fDmgModifier = 0.85f;
				break;
			case NAI::HL_BODY:
				fDmgModifier = 1.0f;
				break;
			default:
				ASSERT(0);
				break;
		}
		// в GetCriticalDmgModifier может возникнуть крит. повреждение
		float fCriticalDmgModifier = 
			GetCriticalDmgModifier( (NAI::EHitLocation)nUserID, pAttack->nCrtical, pAttack->nCrticalDifficulty );
		// Acks
		if ( bAlive && fCriticalDmgModifier > 0 )
			SaveAck( NWorld::N_ACK_CRITICAL, pAttack->pAttacker );
		//
		fDmgModifier += fCriticalDmgModifier;
		csRPG << CC_GREY << "\tHL=" << GetHLName( (NAI::EHitLocation)nUserID );
		csRPG << CC_GREY << " \tDmgModifier=" << fDmgModifier;
		int nDamage = fDmgModifier * nDmg;
		int ndBVP = nDamage * (float)GetHealedVP() / GetTotalVP();
		ASSERT( ndBVP >= 0 );
		GetHealedVP() -= ndBVP;
		pRPGUnit->Skills(NDb::ST_VP) -= nDamage - ndBVP;
		// Acks
		if ( bAlive )
		{
			SUnitInfo UnitInfo;
			GetInfo( NAI::WALK, &UnitInfo );
			if ( ( nDamage - ndBVP ) >= 3.f / 4.f * UnitInfo.nMaxHP )
				SaveAck( NWorld::N_ACK_CRITICAL, pAttack->pAttacker );
			if ( pRPGUnit->Skills(NDb::ST_VP) <= 0 )
				SaveAck( NWorld::N_ACK_DEATH, pAttack->pAttacker );
		}
		//
		csRPG << " \t" << CC_YELLOW << GetName() << CC_WHITE << " damaged on " << nDamage << "hp" << " \tPiercingAbility = " << pAttack->nK << " HP:" << pRPGUnit->Skills(NDb::ST_VP) << "\n";
		pRPGUnit->UseSkill( NDb::ST_VP );
		nTotalDmg = nDamage;
		//
		if ( random.Get( 0, 100 ) <= pAttack->nUnconsciousProbability )
		{
			bUnconscious = true;
			csRPG << " \t" << CC_YELLOW << GetName() << CC_WHITE << " made unconscious\n";
		}
	}
	else
	{
		csRPG << "<font size=16pt>";
		csRPG << "<color=blue>" << " Bullet can`t penetrate target armor, BulletAPA = " << pAttack->nK << endl;
	}
	ApplyCritical( SCritical( NDb::CL_ANY, NDb::C_VP ) );
	return nTotalDmg;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline CVec3 Projection( const CVec3 &pt, const SPlane &plane )
{
	float t = -( pt * plane.n + plane.d ) / fabs2( plane.n );
	return pt + t * plane.n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline float IntersectionArea( const CVec3 &ptA, const CVec3 &ptB, float fRadius )
{
	const float fL = fabs( ptB - ptA );
	const float fAng = acos( Min( 1.0f, fL / (2 * fRadius) ) );
	return sqr( fRadius ) * (2 * fAng - sin( 2*fAng ));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_RADIUS = 0.25f;
float GetCubesArea( const CVec3 &ptPos, vector<CVec3> *pCubes )
{
	if ( pCubes->size() < 2 )
		return 0;
	CVec3 ptCeneter( VNULL3 );
	for ( vector<CVec3>::iterator i = pCubes->begin(); i != pCubes->end(); ++i )
		ptCeneter += *i;
	ptCeneter /= pCubes->size();
	//
	CVec3 nrm( ptCeneter - ptPos );
	Normalize( &nrm );
	SPlane plane( nrm, 0 );
	plane.RecalcDist( ptCeneter );
	for ( vector<CVec3>::iterator i = pCubes->begin(); i != pCubes->end(); ++i )
		*i = Projection( *i, plane );
	//
	float fSum = 0;
	switch ( pCubes->size() )
	{
		case 2:
			fSum += IntersectionArea( pCubes->front(), pCubes->back(), F_RADIUS );
			break;
		case 3:
			fSum += IntersectionArea( (*pCubes)[0], (*pCubes)[1], F_RADIUS );
			fSum += IntersectionArea( (*pCubes)[1], (*pCubes)[2], F_RADIUS );
			fSum += IntersectionArea( (*pCubes)[0], (*pCubes)[2], F_RADIUS );
			break;
	}
	const float fOne = FP_PI * sqr( F_RADIUS );
	return (fOne * pCubes->size() - fSum) / (fOne * 3);
}
/*
////////////////////////////////////////////////////////////////////////////////////////////////////
float GetHLPenalty( NAI::EHitLocation hl )
{
	switch ( hl )
	{
		case NAI::HL_BODY:
			return 0.9f;
		case NAI::HL_HEAD:
			return 0.6f;
		case NAI::HL_RHAND:
		case NAI::HL_LHAND:
			return 0.7f;
		case NAI::HL_RLEG:
		case NAI::HL_LLEG:
			return 0.8f;
	}
	return 1;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetHLPenalty( NAI::EHitLocation hl )
{
	switch ( hl )
	{
		case NAI::HL_BODY:
			return 6;
		case NAI::HL_HEAD:
			return 9;
		case NAI::HL_RHAND:
		case NAI::HL_LHAND:
			return 8;
		case NAI::HL_RLEG:
		case NAI::HL_LLEG:
			return 7;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float GetVPPenalty( int nVP, int nHealedVP, int nMaxVP )
{
	const int nVPPercentage = 100.0f * ( nVP + nHealedVP / 2.f ) / nMaxVP;
	if ( nVPPercentage > 75 )
		return 1.0f;
	else if ( nVPPercentage > 50 )
		return 0.9f;
	else if ( nVPPercentage > 25 )
		return 0.75f;
	else
		return 0.5f;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, const NAI::SPosition &posTarget, 
	NAI::EHitLocation hl, int nExtraAP, IUnitMissionInfo *pTarget, const vector<int> &accessibleHLs, int nHitCover, 
	bool bFirstRound, const CVec3 &ptIllumination ) const
{
	CPtr<IToHitCalcer> pToHitCalcer;
	switch ( GetWeaponType() )
	{
		case NDb::WT_KNIFE:
			pToHitCalcer = new CThrowKnifeToHitCalcer( (CUnitMission *)this, curPose, nDistance, ptAttacker, bFirstRound, CVec3(1,1,1) );
			break;
		case NDb::WT_PISTOL:
		case NDb::WT_RIFLE:
		case NDb::WT_SUB_MACHINE_GUN:
		case NDb::WT_MACHINE_GUN:
			pToHitCalcer = new CUnitToHitCalcer( (CUnitMission *)this, curPose, nDistance, ptAttacker, posTarget, nExtraAP, GetSnipeAP( pTarget ),
									nHitCover, bFirstRound, ptIllumination, hl, pTarget, GetNBullets() );
			break;
		case NDb::WT_RLAUNCHER:
			pToHitCalcer = new CRLauncherToHitCalcer( (CUnitMission *)this, curPose, nDistance,	ptAttacker, nExtraAP, bFirstRound,	
				ptIllumination, NAI::THL_MIDDLE, CVec3(1,1,1) );
			break;
		default:
			ASSERT( 0 );
			return 0;
	}

	int nToHit = pToHitCalcer->GetToHit();
	if ( bLogActive )
		pToHitCalcer->Log();
	return nToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetTileToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, 
	CVec3 ptTilePos, NAI::ETileHitLocation eHitLocation, int nExtraAP, int nHitCover, 
	bool bFirstRound, const CVec3 &ptIllumination ) 
{
	CPtr<IToHitCalcer> pToHitCalcer;
	switch ( GetWeaponType() )
	{
		case NDb::WT_KNIFE:
			pToHitCalcer = new CThrowKnifeTileToHitCalcer( (CUnitMission *)this, curPose, nDistance, ptAttacker, bFirstRound,
					ptIllumination, eHitLocation, ptTilePos );
			break;
		case NDb::WT_PISTOL:
		case NDb::WT_RIFLE:
		case NDb::WT_SUB_MACHINE_GUN:
		case NDb::WT_MACHINE_GUN:
			pToHitCalcer = new CTileToHitCalcer( (CUnitMission *)this, curPose, nDistance,	ptAttacker, nExtraAP, nHitCover, bFirstRound,	
					ptIllumination, eHitLocation, ptTilePos, GetNBullets() );
			break;
		case NDb::WT_RLAUNCHER:
			pToHitCalcer = new CRLauncherToHitCalcer( (CUnitMission *)this, curPose, nDistance,	ptAttacker, nExtraAP, bFirstRound,	
					ptIllumination, eHitLocation, ptTilePos );
			break;
	}
	int nToHit = pToHitCalcer->GetToHit();
	if ( bLogActive )
		pToHitCalcer->Log();
	return nToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetRLauncherToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker,
	CVec3 ptTilePos, NAI::ETileHitLocation eHitLocation, int nExtraAP,
	bool bFirstRound, const CVec3 &ptIllumination )
{
	CPtr<CRLauncherToHitCalcer> pToHitCalcer =
		new CRLauncherToHitCalcer( (CUnitMission *)this, curPose, nDistance,	ptAttacker, nExtraAP, bFirstRound,	
					ptIllumination, eHitLocation, ptTilePos );

	int nToHit = pToHitCalcer->GetToHit();

	if ( bLogActive )
		pToHitCalcer->Log();

	return nToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetObjectToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, 
	int nExtraAP, int nHitCover, bool bFirstRound, const CVec3 &ptIllumination ) 
{
	CPtr<CObjectToHitCalcer> pToHitCalcer =
		new CObjectToHitCalcer( (CUnitMission *)this, curPose, nDistance,	ptAttacker, nExtraAP,
			nHitCover, bFirstRound,	ptIllumination, GetNBullets() );

	int nToHit = pToHitCalcer->GetToHit();

	if ( bLogActive )
		pToHitCalcer->Log();

	return nToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetGrenadeToHit( NAI::EPose curPose, int nDistance, const CVec3 &ptAttacker, 
	bool bFirstRound, CVec3 ptTilePos, NDb::CRPGGrenade *pGrenade, const CVec3 &ptIllumination ) 
{
	CPtr<CGrenadeToHitCalcer> pToHitCalcer =
		new CGrenadeToHitCalcer( (CUnitMission *)this, curPose, nDistance,	ptAttacker, 
			bFirstRound,	ptIllumination, ptTilePos, pGrenade );

	int nToHit = pToHitCalcer->GetToHit();

	if ( bLogActive )
		pToHitCalcer->Log();

	return nToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetBulletsQuantityInShot() const
{
	if ( GetWeaponType() == NDb::WT_KNIFE )
		return 1;
    CWeaponItem *pW = pRPGUnit->GetWeaponItem();
	if ( !pW )
	{
		ASSERT(0);
		return 1;
	}
	NDb::CRPGWeapon *pDBW = pW->GetDBWeapon();
	if ( !pDBW )
	{
		ASSERT(0);
		return 1;
	}
	switch ( pW->GetShootMode() )
	{
		case NDb::SM_Snap:
		case NDb::SM_Aimed:
		case NDb::SM_Careful:
		case NDb::SM_Snipe:
			return 1;
		case NDb::SM_ShortBurst:
		case NDb::SM_LongBurst:
			return pDBW->nRoF / 6;
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static float GetAccessibleHLsPenalty( const vector<int> &hls )
{
	static const int TOTAL_VISIBLE = 5 + 10 + 2 * 5 + 2 * 7;
	int nVisible = 0;
	for ( int i = 0; i < hls.size(); ++i )
		switch ( hls[i] )
		{
			case NAI::HL_HEAD:
				nVisible += 5;
				break;
			case NAI::HL_BODY:
				nVisible += 10;
				break;
			case NAI::HL_RHAND:
			case NAI::HL_LHAND:
				nVisible += 5;
				break;
			case NAI::HL_RLEG:
			case NAI::HL_LLEG:
				nVisible += 7;
				break;
		}
	int nInvisible = TOTAL_VISIBLE - nVisible;
	return 0.01f * (100 - nInvisible);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetMeleeToHit( const CVec3 &ptAttacker, const NAI::SPosition &posTarget, 
	IUnitMissionInfo *pTarget, NAI::EHitLocation hl, const vector<int> &accessibleHLs ) const
{
	CMeleeWeaponItem *pW = pRPGUnit->GetMeleeWeaponItem();
	if ( !pTarget || !pW )
		return 0;
	NDb::CRPGMeleeWeapon *pDBW = pW->GetDBMeleeWeapon();
	if ( !pDBW )
		return 0;
	const int   nSkill = GetSkillValue( NDb::ST_MELEE );
	const int   nTargetSkill = pTarget->GetSkillValue( NDb::ST_MELEE );
	const float nWBonus = pDBW->nToHitBonus;
	const float fMovePenalty = nMoveInLastTurn * pDBW->pWeaponType->fMovePenalty;
	const float fVPPenalty = GetVPPenalty( pRPGUnit->Skills(NDb::ST_VP), pRPGUnit->nHealedVP, pRPGUnit->Skills(NDb::ST_VP).GetMaxValue() );
	const float fAccessibleHLsPenalty = GetAccessibleHLsPenalty( accessibleHLs );
	//
	float fToHit = 50.0f + 0.4f * (nSkill - nTargetSkill) + nWBonus + fMovePenalty;
	fToHit *= fAccessibleHLsPenalty;
	fToHit *= fVPPenalty;
	if ( bLogActive )
	{
		csRPG << "<font size=16pt>";
		csRPG << CC_GREEN << " \tToHit: ";
		csRPG << CC_ORANGE << " \tSkill=" << CC_GREY << nSkill;
		csRPG << CC_ORANGE << " \tTarget skill=" << CC_GREY << nTargetSkill;
		csRPG << CC_ORANGE << " \tMove bonus=" << CC_GREY << (int)fMovePenalty;
		csRPG << CC_ORANGE << " \tWeapon bonus=" << CC_GREY << (int)nWBonus;
		csRPG << CC_ORANGE << " \tHL penalty=" << CC_GREY << int( 100 * (1.0f-fAccessibleHLsPenalty) ) << "%";
		csRPG << CC_ORANGE << " \tVPPenalty=" << CC_GREY << int(100 * (1.0f - fVPPenalty)) << "%";
		csRPG << CC_GREEN << " \tToHit=" << CC_GREY << fToHit << endl;
	}
	return fToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetIC() const
{
	//const float fMoveCoeff = 0.65f;
	//return ( 100 - pRPGUnit->Skills(NDb::ST_IC) ) * int( 100 - float(nMoveInLastTurn) * fMoveCoeff ) / 100;
	return pRPGUnit->Skills(NDb::ST_IC);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::CheckIC()
{
	UseSkill(NDb::ST_IC);
	// Движение цели???
	bool isCheck = random.Check(GetIC());
	csRPG << "\t" << GetName() << " Dodge:" << isCheck << endl;
	return isCheck;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::CalcInterruptProbability( const IUnitMission *pEnemy,
	bool bIsMutual, bool bWasShot )
{
	int nRes = 0; // вероятность interrupt-а
	NDb::SInterruptsConstants *pConst = GetInterruptsConstants();

	SUnitInfo sUnitInfo;
	GetInfo( NAI::WALK, &sUnitInfo );
	if ( sUnitInfo.nAP <= pConst->nMinInterruptAP )
		return nRes;

	int nASkill = pRPGUnit->Skills(NDb::ST_INTERRUPT); 	// A - кто 
	int nBSkill = pEnemy->GetRPGUnit()->Skills(NDb::ST_INTERRUPT); 	// B - кого

	if ( bWasShot )
	{
		nRes = pConst->nMissedShotInterruptsBase + ( nASkill - nBSkill );
	} 
	else
	{
		if ( bIsMutual )
		{
			nRes = pConst->nInterruptsBase + ( nASkill - nBSkill );
		}
		else
		{
			nRes = pConst->nBackInterruptsBase + ( nASkill - nBSkill );
		}
	}

	// уменьшаем вероятность за потраченные AP
	int nAPPenalty = 0;
	nAPPenalty = pConst->fAPInterruptReduction * ( sUnitInfo.nMaxAP - sUnitInfo.nAP );

	// считаем результат
	nRes -= nAPPenalty;
	nRes = Clamp( nRes, 5, 95 );

	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::CheckInterrupt( const IUnitMission *pEnemy, bool bIsMutual, bool bWasShot )
{
	int nProbability = CalcInterruptProbability( pEnemy, bIsMutual, bWasShot );
	int nCheck = random.Get(100);
	csRPG << CC_YELLOW << GetName() << CC_WHITE << " interrupt "<< CC_YELLOW << pEnemy->GetName() << CC_WHITE
		  << " probability " << nProbability << "% Check:" << nCheck << endl;
	pRPGUnit->UseSkill( NDb::ST_INTERRUPT );
	return nProbability - nCheck;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::Kill()
{
	bUnconscious = false;
	pRPGUnit->Skills(NDb::ST_VP).SetValue( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::RemoveCritical( NDb::ECritical eCritical )
{
	for ( vector<CPtr<CCritical> >::iterator i = criticals.begin(); i != criticals.end(); )
		if ( (*i)->GetCritical().eCritical == eCritical )
		{
			i = criticals.erase( i ); // удаляем только один критикал такого типа
			return;
		}
		else
			++i;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::ApplyCritical( SCritical &cr )
{
	SCritical critical = cr;
	for ( vector<CPtr<CCritical> >::iterator i = criticals.begin(); i != criticals.end(); ++i )
	{
		switch( (*i)->Compare( &critical ) )
		{
			case CCritical::WEAKER:
				return false;
			case CCritical::EQUAL:
			case CCritical::STRONGER:
				*i = CreateCritical( critical );
				if ( !IsValid( *i ) || !(*i)->SetModifiers( pRPGUnit, this ) )
					criticals.erase( i );
				return true;
		}
	}
	//
	CPtr<CCritical> p = CreateCritical( cr );
	if ( IsValid( p ) && p->SetModifiers( pRPGUnit, this ) )
	{
		criticals.push_back( p );
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::ApplyCritical( NDb::CRPGCritical *p, int nDC )
{
	int nDuration = -1;
	if ( p->nMinDuration > 0 )
	{
		static SRand rand;
		nDuration = p->nMinDuration + rand.Get( p->nMaxDuration - p->nMinDuration );
	}
	csRPG << "<font size=16pt>";
	csRPG << "<color=red>" << "\tCritical: " << "<color=yeloow>" << "\"" << p->szName << "\", difficulty=" << nDC;
	csRPG << "  Duration=" << nDuration << "(" << p->nMinDuration << " : " << p->nMaxDuration;
	csRPG << ")  HL=" << GetCLName( p->hl ) << "\n";
	return ApplyCritical( SCritical( p->hl, p->type, nDuration, p->fValue, nDC ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::ProcessCriticals()
{
	for ( vector<CPtr<CCritical> >::iterator i = criticals.begin(); i != criticals.end();  )
		if ( !(*i)->NextTurn() )
			i = criticals.erase( i );
		else
			++i;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::GetMaxExtraAP() const
{
	CWeaponItem *pW = pRPGUnit->GetWeaponItem();
	if ( !pW )
		return 0;
	SWeaponInfo info;
	pW->GetInfo( &info );
	return info.nTargetingAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static NDb::CRPGCritical* GetCritical( int nProbability, 
	const vector<SCriticalType> &criticals, const vector<ECriticalState> &criticalsStates )
{
	vector<SCriticalType> enabledCriticals;
	for ( vector<SCriticalType>::const_iterator i = criticals.begin(); i != criticals.end(); ++i )
		if ( criticalsStates[ (*i).pCritical->type ] == CS_ENABLED )
			enabledCriticals.push_back( *i );
	//
	if ( enabledCriticals.empty() )
		return 0;
	//
	if ( nProbability >= enabledCriticals.back().nStartPos )
		return enabledCriticals.back().pCritical;
	//
	for ( int i = 1; i < enabledCriticals.size(); ++i )
		if ( nProbability < enabledCriticals[i].nStartPos )
			return enabledCriticals[i-1].pCritical;
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CUnitMission::RollCritical( NAI::EHitLocation eHL, int nCriticalProbability, 
	int nCriticalDifficulty, NDb::CRPGCritical **pCritical )
{
	int nRand = 100;
	while ( nRand > nCriticalProbability )
		nRand = random.Get( 1, 100 );
	nRand += nCriticalDifficulty;
	switch ( eHL )
	{
		case NAI::HL_HEAD:
			*pCritical = GetCritical( nRand, criticalBar[NDb::CL_HEAD], vCriticalsState );
			break;
		case NAI::HL_BODY:
			*pCritical = GetCritical( nRand, criticalBar[NDb::CL_TORSO], vCriticalsState );
			break;
		case NAI::HL_RHAND:
		case NAI::HL_LHAND:
			*pCritical = GetCritical( nRand, criticalBar[NDb::CL_ARMS], vCriticalsState );
			break;
		case NAI::HL_RLEG:
		case NAI::HL_LLEG:
			*pCritical = GetCritical( nRand, criticalBar[NDb::CL_LEGS], vCriticalsState );
			break;
	}
	return nRand;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CUnitMission::GetCriticalDmgModifier( NAI::EHitLocation eHL, 
	int nCriticalProbability, int nCriticalDifficulty )
{
	ASSERT( nCriticalProbability );
	if ( nCriticalProbability == 0 )
		return 0;

	static SRand rand;
	float fRet = 0;

	if ( !bCBarInitialized )
	{
		InitializeCriticals();
		bCBarInitialized = true;
	}

	int nRand = rand.Get( 100 );
	if ( nRand > nCriticalProbability )
		return fRet;

	csRPG << "<font size=16pt>";
	csRPG << "<color=grey>" << "\tcrit prob=" << nCriticalProbability << " (check=" << nRand << ") \tseverity=" << nCriticalDifficulty;

	NDb::CRPGCritical *pCritical = 0;
	nRand = RollCritical( eHL, nCriticalProbability, nCriticalDifficulty, &pCritical );
	if ( !IsValid( pCritical ) )
		return 0;
	//
	float fDC = (float)nRand / N_MAX_SKILL;

	switch ( eHL )
	{
		case NAI::HL_HEAD:
			fRet = 2.5f * fDC;
			fDC *= 300;
			break;
		case NAI::HL_BODY:
			fRet = 3.0f * fDC;
			fDC *= 300;
			break;
		case NAI::HL_RHAND:
		case NAI::HL_LHAND:
			fRet = 1.5f * fDC;
			fDC *= 200;
			break;
		case NAI::HL_RLEG:
		case NAI::HL_LLEG:
			fRet = 2.0f * fDC;
			fDC *= 250;
			break;
	}
	ApplyCritical( pCritical, fDC );
	return fRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::DumpStats() const
{
	csRPG << "\t " << GetName() << endl;
	NRPG::DumpStats( pRPGUnit, GetHealedVP() );
	csRPG << "\n";
	for ( int i = 0; i < criticals.size(); ++i )
		DumpCritical( criticals[i] );
	csRPG << "\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::SaveAP( const SSnipeAP &ap )
{
	savedSnipeAP.pTarget = ap.pTarget;
	savedSnipeAP.nAP = ap.nAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float fVPScale = (float)N_MAX_VP / N_MAX_SKILL;
const float fDCScale = (float)N_MAX_DC / N_MAX_SKILL;
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::CreateFirstAid( SFirstAid *pRes, int nSpentAP ) const
{
	const int nSkill = pRPGUnit->Skills( NDb::ST_MEDICINE );
	pRes->nMaxVP = fVPScale * nSkill;
	pRes->fdVP = nSpentAP * nSkill / 60.f;
	pRes->nDC = fDCScale * nSkill;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::Heal( const SFirstAid &fa )
{
	const int nMaxVP = Min( pRPGUnit->Skills( NDb::ST_VP ).GetMaxValue(), pRPGUnit->Skills( NDb::ST_VP ) + fa.nMaxVP );
	const int ndVP = Max( 0, Min( int(fa.fdVP), nMaxVP - GetTotalVP() ) );
	GetHealedVP() += ndVP;
	csRPG << "<font size=16pt>";
	csRPG << "\tMaxVP=" << fa.nMaxVP << " \t dVP=" << fa.fdVP << " \t MaxDC=" << fa.nDC << endl;
	csRPG << "<font size=16pt>";
	csRPG << "\t" << GetName() << ": \tVP += " << ndVP << endl;
	for ( vector<CPtr<CCritical> >::iterator i = criticals.begin(); i != criticals.end(); )
	{
		const SCritical &c = (*i)->GetCritical();
		if ( c.nDC <= fa.nDC )
		{
			csRPG << "\tCured: \t";
			DumpCritical( *i );
			i = criticals.erase( i );
		}
		else
			++i;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::CanHeal( IUnitMissionInfo *pTarget ) const
{
	ASSERT( pTarget );
	SUnitInfo info;
	pTarget->GetInfo( NAI::WALK, &info );
	return int( fVPScale * pRPGUnit->Skills( NDb::ST_MEDICINE ) ) > info.nHealedHP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::Reload()
{
	CWeaponItem *pW = pRPGUnit->GetWeaponItem();
	if ( pW )
		pW->Reload( pRPGUnit->GetInventory() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::Unload( IInventoryItem *pItem )
{
	if ( CDynamicCast<CWeaponItem> pWeaponItem( pItem ) )
		pWeaponItem->Unload( pRPGUnit->GetInventory() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::ECritical CUnitMission::GetLastCritical()
{
	NDb::ECritical tmp = eLastCritical;
	eLastCritical = NDb::C_NONE;
	return tmp;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::SetCannonItem( IWeaponItem *pItem )
{
	pRPGUnit->SetCannonItem( dynamic_cast<CWeaponItem*>(pItem) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::CanHearSound( const CVec3 &ptSoundPosition, const CVec3 &ptListenerPosition, 
		NDb::CAISound *pSound, int nAISoundType, IUnitMission *pSource )
{
	float fRadius = pSound->GetRadiusFromAISoundType( nAISoundType );
	NDb::SAISoundConstants *c = GetAISoundConstants();

	float fHearingDistance = c->nPrecisePositionRadius + 
		( fRadius - c->nPrecisePositionRadius ) * 
		GetRPGUnit()->Skills(NDb::ST_SPOT) / NRPG::N_MAX_SKILL;

	float fDistance = fabs( ptSoundPosition - ptListenerPosition ) / FP_GRID_STEP;

	if ( !pSource || fRadius >= c->nLoudSound )
	{
		if ( fHearingDistance >= fDistance )
			return true;
		else
			return false;
	}
	else
	{
		float fHearingProbability = c->nBaseProbability + ( GetRPGUnit()->Skills(NDb::ST_SPOT) +
			( c->nPrecisePositionRadius - fDistance ) * c->nDistanceCoeff -
			pSource->GetRPGUnit()->Skills( NDb::ST_STEALTH ) ) * 100 / NRPG::N_MAX_SKILL;
		int nHearingProbability = Clamp( fHearingProbability, 0.0f, 95.0f );

		if ( random.Get( 1, 100) <= nHearingProbability )
			return true;
		else
			return false;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUnitMission::HasCritical( NDb::ECritical eCritical ) const
{
	for ( vector<CPtr<CCritical> >::const_iterator i = criticals.begin(); i != criticals.end(); ++i )
		if ( (*i)->GetCriticalType() == eCritical )
			return true;
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::EnableCriticals()
{
	for ( int i = 0; i < NDb::N_CRIT_TYPES; ++i )
		vCriticalsState[i] = CS_ENABLED;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::DisableCriticals()
{
	for ( int i = 0; i < NDb::N_CRIT_TYPES; ++i )
		vCriticalsState[i] = CS_DISABLED;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUnitMission::DisableCritical( NDb::ECritical eC, ECriticalState eState  )
{
	vCriticalsState[eC] = eState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Вариант для создания юнита из NRPG::Unit-а, для персонажей игрока
IUnitMission* CreateUnit( CUnit *pSrc, int nScenarioPlayer )
{
	static int nUnitN = 0;

	CUnitMission *pRes = new CUnitMission( nScenarioPlayer );
	pRes->pRPGUnit = pSrc;
	wstring szDotString;
	NStr::ToDotString( &szDotString, ++nUnitN );
	pRes->sID = L"Pers#" + szDotString;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Вариант для создания монстра
IUnitMission* CreateUnit( NDb::CRPGPers *pSrc, int nScenarioPlayer )
{
	static int nUnitN = 0;
	CUnitMission *pRes = new CUnitMission( nScenarioPlayer );
	pRes->pRPGUnit = new CUnit(pSrc);
/*	pRes->pRPGUnit->pScript = new NScript::CContext( pRes->pRPGUnit );

// CRAP
	if ( nUnitN % 2 == 0 )
	{	
		try
		{
			CFileStream f;
			CMemoryStream m;
			const char szFileName[] = "scripts\\enemy.l";
			f.OpenRead( szFileName );
			m.WriteFrom( f ); m << '\0';
			const char *pbuf = (const char *)m.GetBuffer();
			pRes->pRPGUnit->pScript->AddScriptChunk( pbuf, szFileName );
		}
		catch (...)
		{
		}
	}*/
// END OF CRAP

	NStr::ToDotString( &pRes->sID, ++nUnitN );
	pRes->sID = L"Enemy#" + pRes->sID;
	pRes->GetRPGUnit()->Skills(NDb::ST_VP).Multiply( 0.4f );
	pRes->GetRPGUnit()->Skills(NDb::ST_AP).Multiply( 0.7f );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IUnitMission* CreateUnit( CMerc *pSrc, int nScenarioPlayer )
{
	return CreateUnit( pSrc->pRPGUnit, nScenarioPlayer );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NRPG;
REGISTER_SAVELOAD_CLASS( 0x02511016, CUnitMission );
BASIC_REGISTER_CLASS( IUnitMission );
BASIC_REGISTER_CLASS( IUnitMissionInfo );