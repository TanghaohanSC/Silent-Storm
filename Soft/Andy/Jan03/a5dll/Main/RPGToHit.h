#ifndef __RPGTOHIT_H_
#define __RPGTOHIT_H_

namespace NAI
{
	class IAIUnit;
}

namespace NDb
{
	class CRPGGrenade;
}

namespace NWorld
{
	class CObjectServerBase;
}

namespace NRPG
{
class CWeaponItem;
class IInventoryItem;
////////////////////////////////////////////////////////////////////////////////////////////////////
// IToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class IToHitCalcer:	public CObjectBase
{
public:
	virtual int GetToHit() = 0;
	virtual void Log() = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CToHitCalcer: public IToHitCalcer
{
	ZDATA
public:
	int nSkill;
	float fToHit;
	float fMovePenalty;
	SWeaponInfo sWeaponInfo;
	CPtr<IUnitMission> pUnitMission;
	int nExtraAP; // для careful shot
	int nSnipeAP;
	int nHitCover;
	int nDistance;
	CVec3 ptAttacker;
	bool bFirstRound;
	NAI::EPose eCurPose;
	CVec3 ptIllumination;
	int nBullet;
	CPtr<CWeaponItem> pWeaponItem;
	bool bBackStab;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nSkill); f.Add(3,&fToHit); f.Add(4,&fMovePenalty); f.Add(5,&sWeaponInfo); f.Add(6,&pUnitMission); f.Add(7,&nExtraAP); f.Add(8,&nSnipeAP); f.Add(9,&nHitCover); f.Add(10,&nDistance); f.Add(11,&ptAttacker); f.Add(12,&bFirstRound); f.Add(13,&eCurPose); f.Add(14,&ptIllumination); f.Add(15,&nBullet); f.Add(16,&pWeaponItem); f.Add(17,&bBackStab); return 0; }

	// функции соответствуют столбцам в xls табличке
	virtual float GetStance();
	virtual float GetD1();
	virtual float GetD2();
	virtual float GetRS();
	virtual float GetSMove();	
	virtual float GetLight();
	virtual float GetFRMult();
	virtual float GetAllMult();
	virtual float GetSnipeAdd();
	virtual float GetTArea();
	virtual float GetCA();
	virtual float GetAllAdd();
	virtual int GetSkill() { return nSkill; }

	CToHitCalcer() {}
	CToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, 
		int _nDistance, CVec3 _ptAttacker, int _nExtraAP, int _nSnipeAP,
		int _nHitCover, bool _bFirstRound,	CVec3 _ptIllumination, int _nBullet, bool _bBackStab );

	virtual void FillWeaponInfo();
	virtual void Prepare(); 
	virtual int GetToHit(); // получить значение ToHit
	virtual void Log();		// вывести Log
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitToHitCalcer: public CToHitCalcer
{
	OBJECT_BASIC_METHODS(CUnitToHitCalcer)
	ZDATA_(CToHitCalcer)
public:
	CPtr<IUnitMissionInfo> pTarget;
	NAI::SPosition sTargetPosition;
	NAI::EHitLocation eHitLocation;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CToHitCalcer*)this); f.Add(2,&pTarget); f.Add(3,&sTargetPosition); f.Add(4,&eHitLocation); return 0; }

	virtual float GetTArea();
	virtual float GetTMove();
	virtual float GetAllAdd();

	CUnitToHitCalcer() {}
	CUnitToHitCalcer(	IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
		CVec3 _ptAttacker, NAI::SPosition _sTargetPosition, int _nExtraAP, int _nSnipeAP,
		int _nHitCover, bool _bFirstRound,	CVec3 _ptIllumination, NAI::EHitLocation _eHitLocation, 
		IUnitMissionInfo *_pTarget, int _nBullet, bool _bBackStab	);

	virtual int GetToHit();
	virtual void Log();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CObjectToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CObjectToHitCalcer: public CToHitCalcer
{
	OBJECT_BASIC_METHODS(CObjectToHitCalcer)
	ZDATA_(CToHitCalcer)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CToHitCalcer*)this); return 0; }

	virtual float GetCA();

	CObjectToHitCalcer() {}
	CObjectToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
		CVec3 _ptAttacker, int _nExtraAP, int _nHitCover, bool _bFirstRound,	CVec3 _ptIllumination, int _nBullet );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTileToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTileToHitCalcer: public CToHitCalcer
{
	OBJECT_BASIC_METHODS(CTileToHitCalcer)
	ZDATA_(CToHitCalcer)
	NAI::ETileHitLocation eHitLocation;
	CVec3 ptTilePos;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CToHitCalcer*)this); f.Add(2,&eHitLocation); f.Add(3,&ptTilePos); return 0; }

	virtual float GetTArea();

	CTileToHitCalcer() {}
	CTileToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
		CVec3 _ptAttacker, int _nExtraAP, int _nHitCover, bool _bFirstRound,	CVec3 _ptIllumination,
		NAI::ETileHitLocation _eHitLocation, CVec3 _ptTilePos, int _nBullet );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGrenadeToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGrenadeToHitCalcer: public CToHitCalcer
{
	OBJECT_BASIC_METHODS(CGrenadeToHitCalcer)
	ZDATA_(CToHitCalcer)
	CVec3 ptTilePos;
	CDBPtr<NDb::CRPGGrenade> pGrenade;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CToHitCalcer*)this); f.Add(2,&ptTilePos); f.Add(3,&pGrenade); return 0; }

	virtual void Prepare();
	virtual float GetMaxImp();
	virtual float GetRelWeight();
	int GetGrenadeMaxDistance();
	float GetMaxGrenadeVelocity();
	virtual void FillWeaponInfo();
	virtual float GetStance();

	virtual float GetAllAdd();
	virtual int GetToHit();
	virtual void Log();

	CGrenadeToHitCalcer() {}
	CGrenadeToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
		CVec3 _ptAttacker, bool _bFirstRound,	CVec3 _ptIllumination, CVec3 _ptTilePos, NDb::CRPGGrenade *_pGrenade );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMeleeToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMeleeToHitCalcer: public IToHitCalcer
{
	OBJECT_BASIC_METHODS(CMeleeToHitCalcer)

	ZDATA
	CPtr<NRPG::IUnitMission> pShooter;
	CPtr<NRPG::IUnitMission> pTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pShooter); f.Add(3,&pTarget); return 0; }

	inline float GetShooterDerSkill(); // skill с учетом ранений
	inline float GetTargetDerSkill();
	inline float GetShooterDWSkill(); // учет оружия, которым нападаем
	inline float GetTargetDWSkill();

public:
	virtual int GetToHit();
	virtual void Log() {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIUnitToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIUnitToHitCalcer: public CUnitToHitCalcer
{
	OBJECT_BASIC_METHODS(CAIUnitToHitCalcer)
	ZDATA
	ZPARENT(CUnitToHitCalcer);
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUnitToHitCalcer*)this); return 0; }

	virtual float GetTArea();
	virtual float GetTMove();

	CAIUnitToHitCalcer() {}
	CAIUnitToHitCalcer(	NAI::IAIUnit *pShooter, const NAI::SUnitPosition &shooterPos,
		NAI::IAIUnit *pTarget, int nHitCover,	NAI::EHitLocation _eHitLocation, int _nBullet, IInventoryItem *_pWeapon, int _nExtraAP );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIUnitNoWeaponToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIUnitNoWeaponToHitCalcer: public CAIUnitToHitCalcer
{
	OBJECT_BASIC_METHODS(CAIUnitNoWeaponToHitCalcer)
	ZDATA
	ZPARENT(CAIUnitToHitCalcer);
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIUnitToHitCalcer*)this); return 0; }
	//
	virtual void Prepare();
	virtual float GetD1();
	virtual float GetRS();
	virtual float GetTMove();
	virtual float GetTArea();
	virtual float GetLight();
	//
	CAIUnitNoWeaponToHitCalcer() {}
	CAIUnitNoWeaponToHitCalcer( NAI::IAIUnit *pShooter, NAI::IAIUnit *pTarget	);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CThrowKnifeToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CThrowKnifeToHitCalcer: public CToHitCalcer
{
	OBJECT_BASIC_METHODS(CThrowKnifeToHitCalcer)
	ZDATA_(CToHitCalcer)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CToHitCalcer*)this); return 0; }

	virtual void FillWeaponInfo();
	int GetKnifeMaxDistance();
	virtual void Prepare();

	CThrowKnifeToHitCalcer() {}
	CThrowKnifeToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
		CVec3 _ptAttacker, bool _bFirstRound,	CVec3 _ptIllumination );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CThrowKnifeTileToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CThrowKnifeTileToHitCalcer: public CThrowKnifeToHitCalcer
{
	OBJECT_BASIC_METHODS(CThrowKnifeTileToHitCalcer)
	ZDATA_(CThrowKnifeToHitCalcer)
	NAI::ETileHitLocation eHitLocation;
	CVec3 ptTilePos;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CThrowKnifeToHitCalcer*)this); f.Add(2,&eHitLocation); f.Add(3,&ptTilePos); return 0; }

	CThrowKnifeTileToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
		CVec3 _ptAttacker, bool _bFirstRound,	CVec3 _ptIllumination,
		NAI::ETileHitLocation _eHitLocation, CVec3 _ptTilePos );
	CThrowKnifeTileToHitCalcer() {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CRLauncherToHitCalcer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRLauncherToHitCalcer: public CToHitCalcer
{
	OBJECT_BASIC_METHODS(CRLauncherToHitCalcer)
	ZDATA_(CRLauncherToHitCalcer)
	NAI::ETileHitLocation eHitLocation;
	CVec3 ptTilePos;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CRLauncherToHitCalcer*)this); f.Add(2,&eHitLocation); f.Add(3,&ptTilePos); return 0; }

	CRLauncherToHitCalcer() {}
	CRLauncherToHitCalcer( IUnitMission *_pUnitMission, NAI::EPose _eCurPose, int _nDistance,
		CVec3 _ptAttacker, int _nExtraAP, bool _bFirstRound,	CVec3 _ptIllumination,
		NAI::ETileHitLocation _eHitLocation, CVec3 _ptTilePos );

	int GetMaxDistance();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif