#include "stdafx.h"

#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataConst.h"

#include "aiInventory.h"
#include "aiPosition.h"
#include "aiWeapon.h"
#include "aiUnit.h"
#include "aiLog.h"

#include "RPGUnitMission.h"
#include "RPGUnitInfo.h"
#include "RPGItemSet.h"
#include "RPGToHit.h"

#include "wUnitAttack.h"
#include "wUnitServer.h"
#include "wMain.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_SHORT_DISTANCE = 3; // в метрах
const int N_MIDDLE_DISTANCE = 8;
const int N_LONG_DISTANCE = 15;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIWeaponClip
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIWeaponClip: public IAIWeaponClip
{
	ZDATA
public:
	int nAmmoCount; // количество патронов в обойме
	NDb::ESlot eSlot; // в каком слоте лежит
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nAmmoCount); f.Add(3,&eSlot); return 0; }
	//
	CAIWeaponClip(): nAmmoCount(0), eSlot( NDb::ESlot(0) ) {}
	//
	virtual int GetAmmoCount() { return nAmmoCount; }
	virtual void SetAmmoCount( int nCount ) { nAmmoCount = nCount; }
	virtual void SpendAmmo( int nCount ) { nAmmoCount -= nCount; nAmmoCount = max( 0, nAmmoCount ); }
	virtual bool IsEmpty() { return nAmmoCount <= 0; }
	virtual NRPG::IInventoryItem *GetInventoryItem() { return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIFireArmsWeaponClip
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIFireArmsWeaponClip: public CAIWeaponClip
{
	OBJECT_BASIC_METHODS(CAIFireArmsWeaponClip);
	ZDATA
	ZPARENT(CAIWeaponClip);
public:
	CPtr<NRPG::CClipItem> pClipItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIWeaponClip*)this); f.Add(3,&pClipItem); return 0; }

	CAIFireArmsWeaponClip() {}
	CAIFireArmsWeaponClip( NRPG::CClipItem *_pClipItem );

	virtual NRPG::IInventoryItem *GetInventoryItem() { return pClipItem; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIFireArmsWeaponClip::CAIFireArmsWeaponClip( NRPG::CClipItem *_pClipItem ): 
	CAIWeaponClip(), pClipItem(_pClipItem)
{ 
	if ( !IsValid(pClipItem) )
	{
		ASSERT(0);
		nAmmoCount = 0;
	}
	else
		nAmmoCount = pClipItem->GetIncQuantity();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIGrenadeWeaponClip
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIGrenadeWeaponClip: public CAIWeaponClip
{
	OBJECT_BASIC_METHODS(CAIGrenadeWeaponClip)
	ZDATA
	ZPARENT( CAIWeaponClip )
	CPtr<NRPG::CGrenadeItem> pGrenadeItem;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIWeaponClip *)this); f.Add(3,&pGrenadeItem); return 0; }
public:
	//
	CAIGrenadeWeaponClip() {}
	CAIGrenadeWeaponClip( NRPG::CGrenadeItem *_pGrenadeItem );
	//
	virtual NRPG::IInventoryItem *GetInventoryItem() { return pGrenadeItem; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIGrenadeWeaponClip::CAIGrenadeWeaponClip( NRPG::CGrenadeItem *_pGrenadeItem ):
	CAIWeaponClip(), pGrenadeItem(_pGrenadeItem)
{
	nAmmoCount = 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIWeapon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIWeapon: public IAIWeapon
{
	ZDATA
public:
	bool bInHands; // находится-ли в руках
	NDb::ESlot eSlot; // в каком слоте лежит
	CObj<IAIWeaponClip> pCurrentClip; // обойма заряженная в оружие
	vector< CObj<IAIWeaponClip> > Clips; // имеющиеся обоймы ( заряженная не входит )
	CPtr<IAIUnit> pOwner; // у кого в inventory лежит
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bInHands); f.Add(3,&eSlot); f.Add(4,&pCurrentClip); f.Add(5,&Clips); f.Add(6,&pOwner); return 0; }
	//
	CAIWeapon( IAIUnit *_pOwner = 0 ): 
		bInHands(false), eSlot( NDb::ESlot(0) ), pOwner(_pOwner) {}

	// IAIWeapon
	virtual int GetDamage() { return 0; }
	virtual int GetClipCount() { return Clips.size(); }
	virtual IAIWeaponClip *GetCurrentClip() { return pCurrentClip; }
	virtual void SetCurrentClip( IAIWeaponClip *pClip ) { pCurrentClip = pClip; }
	virtual IAIWeaponClip *GetNextClip();
	virtual void RemoveClip( IAIWeaponClip *pClip );
	virtual int GetAmmoCountPerShot( int nUnitAP, int nUnitMaxAP ) { return 0; }
	virtual int GetAmmoCountPerAP( int nAP ) { return 0; }
	virtual int GetDamage( IAIUnit *pTarget, int nHitCover,
		NAI::EPose ePose, int nAP, NDb::EShootMode eShootMode, int *nMaxToHit ) { return 0; }
	virtual bool IsInHands() { return bInHands; }
	virtual void TakeInHands( bool _bInHands ) { bInHands = _bInHands; }
	virtual NRPG::IInventoryItem *GetInventoryItem() { return 0; }
	virtual bool IsSuitableClip( IAIWeaponClip *pClip ) { return false; }
	virtual void AddClip( IAIWeaponClip *pClip );
	virtual void RemoveClips();
	virtual void GetShotParameters( int nMaxAP, int nHitCover,
		int *nAP, int *nAmmo, IAIUnit *pTarget, int *nDamage, bool *bNeedReload ) {}
	virtual int GetReloadAP() { return 0xFFFF; }
	virtual int GetShotAP() { return 0xFFFF; }
	virtual int GetBurstAP() { return 0xFFFF; }
	virtual bool IsSameWeapon( IAIWeapon *pAIWeapon ) { return false; }
	virtual CAILogRecord *GetAttackCommand( IAIUnit *pAIUnit, IAIUnit *pAITarget ) { return 0; }
	virtual void DebugOutput();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIWeapon::DebugOutput()
{
	char szStr[128];
	OutputDebugString( " Weapon clips: " );
	for ( vector< CObj<IAIWeaponClip> >::iterator i = Clips.begin(); i != Clips.end(); ++i )
	{
		sprintf( szStr, " (%d) ", (*i)->GetAmmoCount() );
		OutputDebugString( szStr );
	}
	OutputDebugString( "\n" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeaponClip *CAIWeapon::GetNextClip() 
{ 
	if ( !Clips.empty() )
		return Clips.front();
	else
		return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIWeapon::AddClip( IAIWeaponClip *pClip )
{
	if ( IsSuitableClip( pClip ) )
		Clips.push_back( pClip );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIWeapon::RemoveClip( IAIWeaponClip *pClip )
{
	if ( IsValid( pClip ) )
	{
		vector< CObj<IAIWeaponClip> >::iterator c = find( Clips.begin(), Clips.end(), pClip );
		if ( c != Clips.end() )
			Clips.erase( c );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIWeapon::RemoveClips()
{
	Clips.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIFireArmsWeapon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIFireArmsWeapon: public CAIWeapon
{
	OBJECT_BASIC_METHODS(CAIFireArmsWeapon);
	//
	ZDATA
	ZPARENT(CAIWeapon);
	CPtr<NRPG::CWeaponItem> pWeaponItem; // соответствующий элемент inventory
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIWeapon*)this); f.Add(3,&pWeaponItem); return 0; }
	//
	bool IsBurstMode( NDb::EShootMode eShotMode );
	int GetMinAPToShoot( int nUnitAP, int nUnitMaxAP );
	int GetLongBurstAmmoCountPerShot( int nUnitAP, int nUnitMaxAP );
public:
	//
	CAIFireArmsWeapon() {}
	CAIFireArmsWeapon( IAIUnit *_pOwner, NRPG::CWeaponItem *_pWeaponItem );
	// IAIWeapon
	virtual int GetDamage(); // повреждение по людям
	virtual int GetAmmoCountPerShot( int nUnitAP, int nUnitMaxAP );
	virtual int GetDamage( IAIUnit *pTarget, int nHitCover, 
		NAI::EPose ePose, int nAP, NDb::EShootMode eShootMode, int *nMaxToHit );
	virtual NRPG::IInventoryItem *GetInventoryItem() { return pWeaponItem; }
	virtual bool IsSuitableClip( IAIWeaponClip *pClip );
	virtual int GetReloadAP();
	virtual int GetShotAP();
	virtual int GetBurstAP();
	virtual bool IsSameWeapon( IAIWeapon *pAIWeapon );
	virtual CAILogRecord *GetAttackCommand( IAIUnit *pAIUnit, IAIUnit *pAITarget );
	virtual void GetShotParameters( int nAvailableAP, int nHitCover,
		int *nAP, int *nAmmo, IAIUnit *pTarget, int *nDamage, bool *bNeedReload );
	virtual int GetAmmoCountPerAP( int _nAP );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIFireArmsWeapon::CAIFireArmsWeapon( IAIUnit *_pOwner, NRPG::CWeaponItem *_pWeaponItem ) :
	CAIWeapon( _pOwner ), pWeaponItem(_pWeaponItem)
{
	ASSERT( IsValid( pOwner ) );
	ASSERT( IsValid( pWeaponItem ) );
	//
	if ( CDynamicCast<NRPG::CClipItem> pClipItem( pWeaponItem->GetInnerClip() ) )
	{
		CPtr<IAIWeaponClip> pClip = CreateAIFireArmsWeaponClip( pClipItem );
		SetCurrentClip( pClip );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetAmmoCountPerAP( int _nAP )
{
	int nRes = 0;
	int nAP = _nAP;
	//
	int nMinAPToShoot = GetMinAPToShoot( _nAP, pOwner->GetMaxAP() );
	// запоминаем кол-во патронов в каждой обойме
	list<int> AmmoCount;
	AmmoCount.push_back( GetCurrentClip()->GetAmmoCount() );
	for ( vector< CObj<IAIWeaponClip> >::iterator i = Clips.begin(); i != Clips.end(); ++i )
		AmmoCount.push_back( (*i)->GetAmmoCount() );
	//
	int nAmmoCount = AmmoCount.front();
	// считаем сколько патронов можем выстрелить
	while ( nAP > 0 )
	{
		// выстрелы
		if ( nAmmoCount > 0 )
		{
			// проверяем хватит-ли AP на очередь
			if ( nAP < nMinAPToShoot ) 
			{
				nAP = 0;
				break;
			}
			// отстреливаем патроны
			int nAmmoToShoot = min( nAmmoCount, GetAmmoCountPerShot( _nAP, pOwner->GetMaxAP() ) );
			for ( int k = 0; k < nAmmoToShoot; ++k )
			{
				if ( k == 0 )
					nAP -= nMinAPToShoot;
				else
					nAP -= GetBurstAP();
	
				if ( nAP < 0 )
					break;

				++nRes;
				--nAmmoCount;				
			}
		}
		// перезарядка
		if ( nAmmoCount <= 0 )
		{
			nAP -= GetReloadAP();
			if ( nAP <= 0 )
				break;

			AmmoCount.pop_front();
			if ( AmmoCount.empty() )
				break;
			nAmmoCount += AmmoCount.front();
		}
	}

	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIFireArmsWeapon::GetShotParameters( int nAvailableAP, int nHitCover, 
	int *nAP, int *nAmmo, IAIUnit *pTarget, int *nDamage, bool *bNeedReload )
{
	*nAP = 0;
	*nAmmo = 0;
	*nDamage = 0;
	*bNeedReload = false;
	//
	if ( GetCurrentClip()->GetAmmoCount() <= 0 && !IsValid( GetNextClip() ) || !IsValid( pTarget ) )
		return;
	//
	int nAmmoInClip = GetCurrentClip()->GetAmmoCount();
	if ( nAmmoInClip <= 0 )
	{
		*nAP += GetReloadAP();
		nAmmoInClip = GetNextClip()->GetAmmoCount();
		*bNeedReload = true;
	}
	*nAmmo = min( nAmmoInClip, GetAmmoCountPerShot( nAvailableAP - *nAP, pOwner->GetMaxAP() ) );
	//
	int nBulletDamage = GetDamage();
	int nExtraAP = 0;
	if ( pWeaponItem->GetShootMode() == NDb::SM_Careful )
		nExtraAP = nAvailableAP - GetShotAP()- *nAP;
	//
	for ( int n = 0; n < *nAmmo; ++n )
	{
		if ( n == 0 )
			*nAP += GetMinAPToShoot( nAvailableAP - *nAP, pOwner->GetMaxAP() );
		else
			*nAP += GetBurstAP();
		//
		CPtr<NRPG::CAIUnitToHitCalcer> pToHit = new NRPG::CAIUnitToHitCalcer( pOwner, 
			pTarget, nHitCover, NAI::HL_ANY, n, GetInventoryItem(), nExtraAP );
		*nDamage += pToHit->GetToHit() / 100.f * nBulletDamage;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIFireArmsWeapon::IsBurstMode( NDb::EShootMode eShotMode )
{
	return ( eShotMode == NDb::SM_ShortBurst ) || ( eShotMode == NDb::SM_LongBurst );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetMinAPToShoot( int nUnitAP, int nUnitMaxAP )
{
	int nRes = 0;
	//
	nRes = GetShotAP();
	nRes += pOwner->GetUnitServer()->GetActionAP( NRPG::AC_PREPARE );
	//
	if ( pWeaponItem->GetShootMode() == NDb::SM_Careful )
		nRes = max( nUnitAP, GetShotAP() );
	//
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetDamage()
{
	NRPG::SWeaponInfo info;
	pWeaponItem->GetInfo( &info );
	return ( info.nDmgMax + info.nDmgMin ) / 2.f;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetDamage( IAIUnit *pTarget, int nHitCover, 
	NAI::EPose ePose, int nAP, NDb::EShootMode eShootMode, int *nMaxToHit )
{
	int nRes = 0;
	NDb::EShootMode eTmpShootMode = pWeaponItem->GetShootMode();
	pWeaponItem->SetShootMode( eShootMode );
	int nDamage = GetDamage();
	int nAmmoCount = GetAmmoCountPerAP( nAP );
	//
	int nExtraAP = 0;
	int nTmpExtraAP = 0;
	if ( eShootMode == NDb::SM_Careful )
	{
		nExtraAP = max( 0, nAP - GetShotAP() );
		nTmpExtraAP = max( 0, pOwner->GetMaxAP() - GetShotAP() );
	}
	//
	CPtr<NRPG::CAIUnitToHitCalcer> pTmpToHit = new NRPG::CAIUnitToHitCalcer( pOwner, 
		pTarget, nHitCover, NAI::HL_ANY, 0, GetInventoryItem(), nTmpExtraAP );
	*nMaxToHit = pTmpToHit->GetToHit();
	//
	for ( int n = 0; n < nAmmoCount; ++n )
	{
		CPtr<NRPG::CAIUnitToHitCalcer> pToHit = new NRPG::CAIUnitToHitCalcer( pOwner, 
			pTarget, nHitCover, NAI::HL_ANY, n % GetAmmoCountPerShot( nAP, pOwner->GetMaxAP() ), GetInventoryItem(), nExtraAP );
		nRes += pToHit->GetToHit() / 100.f * nDamage;
	}
	pWeaponItem->SetShootMode( eTmpShootMode );
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAILogRecord *CAIFireArmsWeapon::GetAttackCommand( IAIUnit *pAIUnit, IAIUnit *pAITarget )
{
	return new CAILogShot( pAIUnit, pAITarget, HL_ANY );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetReloadAP() 
{ 
	return pWeaponItem->GetReloadAP(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetShotAP()
{ 
	int nRes = pWeaponItem->GetShootAP();
	NDb::EShootMode ShotMode = pWeaponItem->GetShootMode();
	if ( ShotMode == NDb::SM_Aimed || ShotMode == NDb::SM_Careful )
		nRes += pWeaponItem->GetDBWeapon()->nTargetingAP;		
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetBurstAP()
{
	NRPG::SUnitInfo sInfo;
	pOwner->GetUnitServer()->GetUnitRPG()->GetInfo( NAI::WALK, &sInfo );
	int nRoF = pWeaponItem->GetDBWeapon()->nRoF;
	return sInfo.nMaxAP / pWeaponItem->GetDBWeapon()->nRoF;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetLongBurstAmmoCountPerShot( int nUnitAP, int nUnitMaxAP )
{
	int nRes = 0;
	int nAP = nUnitAP;
	nAP -= GetMinAPToShoot( nUnitAP, nUnitMaxAP );
	while ( nAP >= 0 )
	{
		++nRes;
		nAP -= GetBurstAP();
	}
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIFireArmsWeapon::GetAmmoCountPerShot( int nUnitAP, int nUnitMaxAP )
{
	switch( pWeaponItem->GetShootMode() )
	{
		case NDb::SM_Snap:
		case NDb::SM_Aimed:
		case NDb::SM_Careful:
		case NDb::SM_Snipe:
			return 1;
		case NDb::SM_ShortBurst:
			return pWeaponItem->GetDBWeapon()->nRoF / 6;
		case NDb::SM_LongBurst:
			return GetLongBurstAmmoCountPerShot( nUnitAP, nUnitMaxAP );
	}
	//
	ASSERT( 0 && " Unknown shoot mode " );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIFireArmsWeapon::IsSuitableClip( IAIWeaponClip *pClip )
{
	if ( IsValid( pClip ) )
	{
		if ( CDynamicCast<NRPG::CClipItem> pClipItem( pClip->GetInventoryItem() ) )
			if ( CDynamicCast<NRPG::CClipItem> pInnerClipItem( pWeaponItem->GetInnerClip() ) )
				return pInnerClipItem->IsCompatible( pClipItem, false );
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIFireArmsWeapon::IsSameWeapon( IAIWeapon *pAIWeapon )
{
	CDynamicCast<CAIFireArmsWeapon> pFireArmsWeapon( pAIWeapon );
	if ( IsValid( pFireArmsWeapon ) )
	{
		CDynamicCast<NRPG::CWeaponItem> pFireArmsWeaponItem( pFireArmsWeapon->GetInventoryItem() );
		CDynamicCast<NRPG::CWeaponItem> pThisFireArmsWeaponItem( GetInventoryItem() );
		int nID = pFireArmsWeaponItem->GetDBWeapon()->GetRecordID();
		int nThisID = pThisFireArmsWeaponItem->GetDBWeapon()->GetRecordID();
		if ( nID == nThisID )
			return true;
	}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIGrenadeWeapon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIGrenadeWeapon: public CAIWeapon
{
	OBJECT_BASIC_METHODS(CAIGrenadeWeapon);
	ZDATA
	ZPARENT(CAIWeapon)
	CDBPtr<NDb::CRPGGrenade> pDBGrenade;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIWeapon*)this); f.Add(3,&pDBGrenade); return 0; }
public:
	//
	CAIGrenadeWeapon() {}
	CAIGrenadeWeapon( IAIUnit *_pOwner, NRPG::CGrenadeItem *_pGrenadeItem );
	//
	int GetGrenadeID() { return pDBGrenade->GetRecordID(); }
	// IAIWeapon
	virtual int GetDamage() { return 500; }
	virtual int GetAmmoCountPerShot( int nUnitAP, int nUnitMaxAP ) { return 1; }
	virtual int GetDamage( IAIUnit *pTarget, int nHitCover,
		NAI::EPose ePose, int nAP, NDb::EShootMode eShootMode, int *nMaxToHit );
	virtual NRPG::IInventoryItem *GetInventoryItem();
	virtual bool IsSuitableClip( IAIWeaponClip *pClip );
	virtual int GetReloadAP() { return 0; }
	virtual int GetShotAP();
	virtual int GetBurstAP() { return 0; }
	virtual bool IsSameWeapon( IAIWeapon *pAIWeapon );
	virtual void GetShotParameters( int *nAP, int *nAmmo, 
		IAIUnit *pTarget, int *nDamage, bool *bNeedReload );
	CAILogRecord *GetAttackCommand( IAIUnit *pAIUnit, IAIUnit *pAITarget );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIGrenadeWeapon::CAIGrenadeWeapon( IAIUnit *_pOwner, NRPG::CGrenadeItem *_pGrenadeItem ):
	CAIWeapon(_pOwner)
{
	CDynamicCast<NRPG::CGrenadeItem> pGrenadeWeaponItem( _pGrenadeItem );
	pDBGrenade = pGrenadeWeaponItem->GetDBGrenade();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIGrenadeWeapon::GetShotAP()
{ 
	return pOwner->GetUnitServer()->GetUnitRPG()->GetActionAP( NAI::WALK, NRPG::AC_THROW_GRENADE ); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIGrenadeWeapon::IsSameWeapon( IAIWeapon *pAIWeapon )
{
	CDynamicCast<CAIGrenadeWeapon> pGrenadeWeapon( pAIWeapon );
	if ( IsValid( pGrenadeWeapon ) )
	{
		if ( GetGrenadeID() == pGrenadeWeapon->GetGrenadeID() )
			return true;
	}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIGrenadeWeapon::IsSuitableClip( IAIWeaponClip *pClip )
{
	CDynamicCast<CAIGrenadeWeaponClip> pGrenadeClip( pClip );
	if ( IsValid( pGrenadeClip ) )
	{
		CDynamicCast<NRPG::CGrenadeItem> pGrenadeClipItem( pGrenadeClip->GetInventoryItem() );
		int nClipID = pGrenadeClipItem->GetDBGrenade()->GetRecordID();
		if ( GetGrenadeID() == nClipID )
			return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CAIGrenadeWeapon::GetDamage( IAIUnit *pTarget, int nHitCover, 
	NAI::EPose ePose, int nAP, NDb::EShootMode eShootMode, int *nMaxToHit )
{
	if ( IsValid( GetCurrentClip() ) )
	{
		if ( CDynamicCast<NRPG::IGrenadeItem> pItem( GetCurrentClip()->GetInventoryItem() ) )
		{
			if ( pOwner->GetUnitServer()->CanDo( new NWorld::CCmdShootObject( pTarget->GetUnitServer(), 0 ) ) == NWorld::UCR_OK )
				return 5000; // CRAP;
		}
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::IInventoryItem *CAIGrenadeWeapon::GetInventoryItem()
{
	if ( IsValid( GetCurrentClip() ) )
		return GetCurrentClip()->GetInventoryItem();
	else 
		return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIGrenadeWeapon::GetShotParameters( int *nAP, 
	int *nAmmo, IAIUnit *pTarget, int *nDamage, bool *bNeedReload )
{
	if ( IsValid( GetCurrentClip() ) )
	{
		*nAP = GetShotAP();
		*nAmmo = 1;
		*nDamage = 5000;
		*bNeedReload = true;
	}
	else
	{
		*nAP = 0;
		*nAmmo = 0;
		*nDamage = 0;
		*bNeedReload = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CAILogRecord *CAIGrenadeWeapon::GetAttackCommand( IAIUnit *pAIUnit, IAIUnit *pAITarget )
{
	return new CAILogThrowGrenade( pAIUnit, pAITarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeaponClip *CreateAIFireArmsWeaponClip( NRPG::CClipItem *pClip )
{
	return new CAIFireArmsWeaponClip( pClip );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeaponClip *CreateAIGrenadeWeaponClip( NRPG::CGrenadeItem *_pGrenade )
{
	return new CAIGrenadeWeaponClip( _pGrenade );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeapon *CreateFireArmsAIWeapon( IAIUnit *_pOwner, NRPG::CWeaponItem *_pWeapon )
{
	return new CAIFireArmsWeapon( _pOwner, _pWeapon );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIWeapon *CreateGrenadeAIWeapon( IAIUnit *_pOwner, NRPG::CGrenadeItem *_pGrenade )
{
	return new CAIGrenadeWeapon( _pOwner, _pGrenade );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}

using namespace NAI;

REGISTER_SAVELOAD_CLASS( 0x52642101, CAIFireArmsWeapon );
REGISTER_SAVELOAD_CLASS( 0x52642100, CAIFireArmsWeaponClip );
REGISTER_SAVELOAD_CLASS( 0x52942120, CAIGrenadeWeaponClip );
REGISTER_SAVELOAD_CLASS( 0x52942121, CAIGrenadeWeapon );