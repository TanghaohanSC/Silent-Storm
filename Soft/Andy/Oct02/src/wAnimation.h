#ifndef __wAnimation_H_
#define __wAnimation_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Time.h"
#include "aiPosition.h"
#include "GAnimation.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
	class CGeometry;
	class CSkeleton;
	enum EWeaponType;
	enum ESlot;
	enum EItemPlace;
}
namespace NAnimation
{
	class CAnimator;
	class CAnimation;
	class CSkeletonAnimator;
	struct SBonePose;
}
namespace NAI
{
	class IAIMap;
	enum EBlowHeight;
}
namespace NRPG
{
	class IInventoryItem;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
class CUnitTerrain;
class CCannon;
class CUnit;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitAnimator
{
	ZDATA
	CObj<CUnitTerrain> pTerrainFunc;
	
	STime tEnd; // time when dynamic action will end
	int nAnimFlags; // current pose and weapon
	string szWeaponName;
	CVec3 hipShift;
	
	bool bIdle; 
	// movement vars
	bool bWalking;
	bool bStart;
	float fStartLeft;
	STime tAnim; // current animation time on tEnd
	CVec2 ptPrev;
	CVec2 ptPrevPrev; // :)
	// strafe vars
	bool bStrafing;
	bool bAimedStrafe;
	float fLookAngle;
	int nWalkAnim;
	// rotate vars
	float fRotateVel;
	// shoot vars
	bool bAimed;
	CVec3 aimDir;
	//
	bool bActiveItem;
	bool bInactivePose;
	bool bCorpse;
	bool bHealing;
	bool bLadder;
	
	// time labels
	STime tLabel1;
	vector<STime> tSteps;

	NDb::EWeaponType eAnimationWeaponType;
	NAI::EPose pose;
	CDBPtr<NDb::CSkeleton> pSkeleton;

	CObj<CFuncBase<STime> > pTime;
	CObj<NAnimation::CSkeletonAnimator> pAnimator;
	CObj<NAnimation::CSkeletonState> pAnimState;
	CPtr<CCannon> pCannon;
	CPtr<CUnit> pCarrier; // somebody who carries this unit
	CPtr<NAI::IAIMap> pAIMap;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pTerrainFunc); f.Add(3,&tEnd); f.Add(4,&nAnimFlags); f.Add(5,&szWeaponName); f.Add(6,&hipShift); f.Add(7,&bWalking); f.Add(8,&bStart); f.Add(9,&fStartLeft); f.Add(10,&tAnim); f.Add(11,&ptPrev); f.Add(12,&ptPrevPrev); f.Add(13,&bStrafing); f.Add(14,&bAimedStrafe); f.Add(15,&fLookAngle); f.Add(16,&nWalkAnim); f.Add(17,&fRotateVel); f.Add(18,&bAimed); f.Add(19,&aimDir); f.Add(20,&bActiveItem); f.Add(21,&bInactivePose); f.Add(22,&bCorpse); f.Add(23,&bHealing); f.Add(24,&bLadder); f.Add(25,&tLabel1); f.Add(26,&tSteps); f.Add(27,&eAnimationWeaponType); f.Add(28,&pose); f.Add(29,&pSkeleton); f.Add(30,&pTime); f.Add(31,&pAnimator); f.Add(32,&pAnimState); f.Add(33,&pCannon); f.Add(34,&pCarrier); f.Add(35,&pAIMap); return 0; }
private:
	void CalculateAnimFlags();
	//NAnimation::CAnimation* CreateAnimation( NDb::CAnimation::EType type, int nFlags, STime tStart );
	void PlayAnimation( const NAI::SUnitPosition &cmdPos, 
		int nType, const char *pszParams = 0, bool bInstantly = false );
	void DefaultAction( const NAI::SUnitPosition &cmdPos );
	NAnimation::CAnimator* PutOnTerrain( NAnimation::CAnimator *pAnim );
	void Move( const NAI::SUnitPosition &prevPos, 
		const NAI::SUnitPosition &cmdPos, const NAI::SUnitPosition &nextPos, bool bEnd, bool bInterGrid );
	void StandStill( CVec2 &pos, float fAngle, bool bNeedStrafe = false );
	void Stand( const NAI::SUnitPosition &cmdPos, bool bNeedStrafe = false );
	void HealOn( const NAI::SUnitPosition &cmdPos, const char *pszParams );
	void IdleOn( const NAI::SUnitPosition &cmdPos );
	void IdleOff();
	void AttackCannon( const NAI::SUnitPosition &cmdPos, const CRay &ray, bool bInstant = false );
	void ReloadCannon( const NAI::SUnitPosition &cmdPos );
	void ReturnCannonToDefault();
	void AttachHandsToCannon();
	bool CanStandAimed( const NAI::SUnitPosition &cmdPos );
	bool PlayLadderAnimation( const NAI::SUnitPosition &prevPos, int nType );
public:
	CUnitAnimator() {}
	CUnitAnimator( CFuncBase<STime> *_pTime, const NAI::SUnitPosition &pos, NDb::CSkeleton *_pSkeleton, NAI::IAIMap *pAIMap );
	//
	NAnimation::CSkeletonAnimator* GetSkeletonAnimator() const { return pAnimator; }
	CFuncBase<NAnimation::SSkeletonState>* GetSkeletonState() const { return pAnimState; }
	//
	void PlaceUnit( const NAI::SUnitPosition &cmdPos );
	void SetWeaponAnimation( NDb::EWeaponType eAWT ) { eAnimationWeaponType = eAWT; }
	void SetWeaponName( const char *pszName ) { szWeaponName = pszName;	}
	void SetActiveItem( bool _bActive ) { bActiveItem = _bActive;	}
	void SetPose( NAI::EPose _pose ) { pose = _pose; }
	// accessors
	void AlignTime( STime tDelay = 0 );
	STime GetTimeEnd() const { return tEnd; }
	STime GetTimeLabel1() const { return tLabel1; }
	bool GetHipPos( CVec3 *pRes );
	bool GetBarrelPos( NDb::CGeometry *pWeaponGeometry, NAnimation::SBonePose *pRes );
	bool IsAiming() const { return bAimed || bAimedStrafe; }
	bool IsStrafing() const { return bStrafing; }
	bool IsActiveItem() const { return bActiveItem; }
	bool IsCarryingCorpse() const { return bCorpse; }
	bool CanActivateItem() const { return !bCorpse && !bLadder && !bHealing && !bInactivePose; }
	CCannon* GetCannon() const { return pCannon; }
	CUnit* GetCorpseCarrier() const { return pCarrier; }
	// steps
	bool GetStepTime( STime *pT ) const;
	void DropStep();
	// commands
	void StartNewTurn( const NAI::SUnitPosition &cmdPos );
	void StartMove( bool bWalkStrafe );
	void EndMove( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos, bool bFreeze, bool bInterGrid );
	void Move( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos, const NAI::SUnitPosition &nextPos, bool bInterGrid );
	void Rotate( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos, bool bStart );
	void EndRotate( const NAI::SUnitPosition &cmdPos );
	void Attack( const NAI::SUnitPosition &cmdPos, const CRay &ray, bool bInstant = false, bool bNoShoot = false );
	void Snipe( const NAI::SUnitPosition &cmdPos, const CRay &ray );
	void CloseAttack( const NAI::SUnitPosition &cmdPos, NAI::EBlowHeight eHeight );
	void ChangePose( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos );
	void Climb( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos, bool bRealClimb );
	void Jump( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos, bool bRealJump );
	void Die( const NAI::SUnitPosition &cmdPos, const CVec3 &ptDir );
	void ActivateItem( const NAI::SUnitPosition &cmdPos, 
		bool bHeavy, bool bBackpack, NDb::EItemPlace place, 
		NDb::EWeaponType eAWT, bool bInstantly = false );
	void DeactivateItem( const NAI::SUnitPosition &cmdPos, bool bHeavy, 
		bool bBackpack, NDb::EItemPlace place, bool bInstantly = false );
	void ThrowGrenade( const NAI::SUnitPosition &cmdPos, const CVec3 &target, int nSide, int nGrenadeSize = 1 );
	void ThrowKnife( const NAI::SUnitPosition &cmdPos, const CVec3 &target );
	void OpenWindowDoor( const NAI::SUnitPosition &cmdPos );
	void Reload( const NAI::SUnitPosition &cmdPos );
	void StartHealing( const NAI::SUnitPosition &cmdPos, NAI::EBlowHeight eHeight );
	void FinishHealing( const NAI::SUnitPosition &cmdPos );
	// corpses
	void TakeCorpse( const NAI::SUnitPosition &cmdPos );
	void DropCorpse( const NAI::SUnitPosition &cmdPos );
	void BeTaken( CUnit *pCarrier, CUnitAnimator *pTaker );
	void BeDropped();
	// cannons
	void EnterCannon( const NAI::SUnitPosition &cmdPos, CCannon *pCannon );
	void LeaveCannon( const NAI::SUnitPosition &cmdPos );
	// ladders
	void EnterLadder( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos, bool bUp );
	void LeaveLadder( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos, bool bUp );
	void MoveLadder( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos );
	void JumpFromLadder( const NAI::SUnitPosition &prevPos, const NAI::SUnitPosition &cmdPos );
	//
	void Wound();
	void InitAsCorpseCarrier( const NAI::SUnitPosition &pos );
	void InitAsCorpse( const NAI::SUnitPosition &pos );
	//
	void IdleBan( char cBanSourceFlag, bool bBan ); 
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
