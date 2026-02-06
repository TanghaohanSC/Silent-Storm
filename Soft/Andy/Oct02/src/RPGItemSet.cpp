#include "StdAfx.h"
#include "RPGItem.h"
#include "RPGItemSet.h"
#include "aiPosition.h"
#include "RPGAttackMech.h"
#include "..\DBFormat\DataRPG.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CItem
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CItem::Join( IJoinSplit *_pItem )
{
	if ( typeid(this) != typeid(_pItem) )
		return false;
	CDynamicCast<CItem> pItem(_pItem);
	ASSERT( pItem );
	if ( pItem )
		nQuantity += pItem->GetQuantity();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IJoinSplit *CItem::Split( int nQuantityToGet )
{
	if ( nQuantityToGet > nQuantity - 1 )
		nQuantityToGet = nQuantity - 1;
	nQuantity -= nQuantityToGet;
	CItem *pNewItem = dynamic_cast<CItem*>(MakeCopy());
	pNewItem->nQuantity = nQuantityToGet;
	return pNewItem;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CInventoryItem
////////////////////////////////////////////////////////////////////////////////////////////////////
int CInventoryItem::GetWeight() const 
{ 
	return GetDBItem()->nWeight; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const CTPoint<int>& CInventoryItem::GetSize() const 
{ 
	return GetDBItem()->sSize; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CClipItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CClipItem::CClipItem( NDb::CRPGClip *pClip ): 
	CItemContainer<CAmmoItem>( pClip->pItem ), pDBClip( pClip ), nMaxAmmoQuantity( 0 )
{
	ASSERT( IsValid( pClip ) );
	if ( IsValid( pClip ) )
		SetMaxIncQuantity( pDBClip->nQuantity );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CClipItem::GetDBClipID() const 
{ 
	if ( IsValid( pDBClip ) ) 
		return pDBClip->GetRecordID(); 
	return -1; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CRPGAmmo* CClipItem::GetDBAmmo() const 
{ 
	return GetItem()->GetDBAmmo(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CClipItem::GetMaxIncQuantity() const
{
	return nMaxAmmoQuantity;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CClipItem::SetMaxIncQuantity( int _nMaxAmmoQuantity )
{
	nMaxAmmoQuantity = _nMaxAmmoQuantity;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClipItem::IsCompatible( CClipItem *pClip, bool bCheckSameColor )
{
	return ( GetDBClip()->nAmmoGroup == pClip->GetDBClip()->nAmmoGroup &&
		( !bCheckSameColor || GetDBAmmo()->color == pClip->GetDBAmmo()->color ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CClipItem::LoadAmmoFromClip( CClipItem *pClip )
{
	ASSERT( IsValid( pClip ) );
	if ( !IsValid( pClip ) )
		return;
	ASSERT( IsCompatible( pClip, false ) );
	if ( !IsCompatible( pClip, false ) )
		return;
	//
	if ( pClip->GetIncQuantity() <= 0 )
		return;
	//
	Load( new CAmmoItem( pClip->GetDBAmmo() ) );
	int nNeed = GetMaxIncQuantity() - GetIncQuantity();
	int nHave = pClip->GetIncQuantity();
	CPtr<IJoinSplit> pGet = pClip->SplitItem( min( nNeed, nHave ) );
	if ( IsValid( pGet ) )
		JoinItem( pGet );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CWeaponItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CWeaponItem::CWeaponItem( NDb::CRPGWeapon *_pWeapon )
	: CInventoryItem( _pWeapon->pItem ), pDBWeapon( _pWeapon ), bWorking(true)
{
	for ( int i = 0; i < NDb::SM_MAXVALUE; i++ )
	{
		if ( IsShootModeSupported( NDb::EShootMode( i ) ) )
		{
			eShootMode = NDb::EShootMode( i );
			break;
		}
	}
	//
	int nInnerClipAmmoQuantity = 
		min( pDBWeapon->nInnerClipAmmoQuantity, pDBWeapon->pInnerClip->nQuantity );
	if ( nInnerClipAmmoQuantity <= 0 )
		nInnerClipAmmoQuantity = pDBWeapon->pInnerClip->nQuantity;
	if ( CDynamicCast<CClipItem> pTmpClip( CreateClipItem( pDBWeapon->pInnerClip,
		0, nInnerClipAmmoQuantity ) ) )
	{
		pInnerClip = pTmpClip;
		pInnerClip->SetMaxIncQuantity( nInnerClipAmmoQuantity );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWeaponItem::SetShootMode( NDb::EShootMode _eShootMode )
{
	if ( !IsShootModeSupported( _eShootMode ) )
		return false;

	eShootMode = _eShootMode;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWeaponItem::IsShootModeSupported( NDb::EShootMode eMode ) const
{
	return pDBWeapon->shootModes[eMode];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWeaponItem::GetInfo( SWeaponInfo *pInfo ) const
{
	ASSERT( pInfo );
	if ( pInfo == 0 )
		return;
	//
	CPtr<NDb::CRPGAmmo> pAmmo = GetInnerClip()->GetDBAmmo();
	int nDmgMod = pDBWeapon->nDamageMod;
	pInfo->nRoF = Float2Int( float( pDBWeapon->nRoF ) / 6 );
	pInfo->nDmgMin = ( pAmmo->nDmgMin * pDBWeapon->nDamageMod ) / 100;
	pInfo->nDmgMax = ( pAmmo->nDmgMax * pDBWeapon->nDamageMod ) / 100;
	pInfo->nArmorPiercingAbility = int( float(pDBWeapon->nInitialVelocity) * pAmmo->fUnitWeight );
	pInfo->nRange = pDBWeapon->nRange;
	pInfo->nShotAP = pDBWeapon->nShotAP;
	pInfo->nTargetingAP = pDBWeapon->nTargetingAP;
	pInfo->nQuality = pDBWeapon->nQuality;
	pInfo->fMaxToHitCoeff = pDBWeapon->pWeaponType->fMaxToHitCoeff;
	pInfo->fMinToHitCoeff = pDBWeapon->pWeaponType->fMinToHitCoeff;
	pInfo->nRecoil = pDBWeapon->nRecoil;
	pInfo->fScopeFactor = (pDBWeapon->bScope) ? 100.f : 20.f;
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::ESkillType CWeaponItem::GetSkillIndex() const
{
	return (NDb::ESkillType)(NDb::ST_MELEE + pDBWeapon->pWeaponType->nSkillIndex);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CWeaponItem::GetShootAP() const
{
	return pDBWeapon->nShotAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CWeaponItem::GetReloadAP() const
{
	return pDBWeapon->nReloadAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CWeaponItem::GetAmmoQuantity() const
{
	ASSERT( IsValid( pInnerClip ) );
	if ( IsValid( pInnerClip ) )
		return pInnerClip->GetIncQuantity();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWeaponItem::HasAmmo() const
{
	return GetAmmoQuantity() > 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CWeaponItem::CreateNewAttackPortion( vector<CAttackPortion> *pRes, bool bSpendAmmo )
{
	if ( !IsWorking() )
		return;
	SWeaponInfo info;
	GetInfo( &info );

	if ( bSpendAmmo )
	{
		ASSERT( IsValid( pInnerClip ) );
		if ( pInnerClip->GetIncQuantity() == 0 )
			return;

		CObj<IJoinSplit> pSpent = pInnerClip->SplitItem( 1 );
	}

	float fUW = pInnerClip->GetDBAmmo()->fUnitWeight;
	int nK = (int)( fUW * pDBWeapon->nInitialVelocity );
	pRes->push_back( CAttackPortion( nK, pInnerClip->GetDBAmmo()->nBulletType, info.nDmgMin, info.nDmgMax,
	info.nArmorPiercingAbility, 0 ) ); // вероятность и сложность critical вычисляется в RPGUnitMission
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CWeaponItem::GetClipType() const
{
	if ( !IsValid( pInnerClip )  )
		return -1;
	return pInnerClip->GetDBClipID();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFindClipResult
{
	enum ESource
	{
		SLOT,
		BACKPACK
	};

	ESource eSource;
	NDb::ESlot eSlot;
	CPtr<CClipItem> pItem;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static void EraseFindResultItem( IInventory *pInventory, const SFindClipResult &sResult )
{
	switch( sResult.eSource )
	{
	case SFindClipResult::SLOT:
		{
			pInventory->TakeOff( sResult.eSlot );
			break;
		}
	case SFindClipResult::BACKPACK:
		{
			pInventory->Take( sResult.pItem );
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWeaponItem::FindProperClip( IInventoryInfo *pInventory, 
	SFindClipResult *pResult, bool bCheckSameColor ) const
{
	ASSERT( pResult );
	// если патроны в рожке не закончились, то туда можно сыпать только тот же тип патронов
	bool bSameColor = bCheckSameColor || pInnerClip->GetIncQuantity() > 0;
	// Ищем обойму такого же типа как сейчас в оружии
	// ищем в slot-ах
	for ( int i = 0; i < NDb::N_SLOTS; ++i )
	{
		if ( CDynamicCast<CClipItem> pClip( pInventory->Get( NDb::ESlot(i) ) ) )
		{
			if ( pInnerClip->IsCompatible( pClip, bSameColor ) )
			{
				pResult->eSource = SFindClipResult::SLOT;
				pResult->eSlot = NDb::ESlot(i);
				pResult->pItem = pClip;
				return true;
			}
		}
	}
	// ищем в рюкзаке
	const vector<SBackPackItem> &items = pInventory->GetItems();
	for ( int i = 0; i < items.size(); ++i )
	{
		if ( CDynamicCast<CClipItem> pClip( items[i].pItem ) )
		{
			if ( pInnerClip->IsCompatible( pClip, bSameColor ) )
			{
				pResult->eSource = SFindClipResult::BACKPACK;
				pResult->pItem = pClip;
				return true;
			}
		}
	}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWeaponItem::CanReload( IInventoryInfo *pInventory ) const
{
	SFindClipResult sResult;
	if ( GetInnerClip()->GetIncQuantity() > 0 )
		return FindProperClip( pInventory, &sResult, true );
	else
		return FindProperClip( pInventory, &sResult, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWeaponItem::Reload( IInventory *pInventory )
{
	SFindClipResult sResult;
	if ( !FindProperClip( pInventory, &sResult, false ) )
		return false;
	//
	bool bSameColor = FindProperClip( pInventory, &sResult, true );
	while ( pInnerClip->GetIncQuantity() < pInnerClip->GetMaxIncQuantity() && 
		FindProperClip( pInventory, &sResult, bSameColor ) )
	{
		pInnerClip->LoadAmmoFromClip( sResult.pItem );
		if ( sResult.pItem->GetIncQuantity() <= 0 )
			EraseFindResultItem( pInventory, sResult );
		bSameColor = true;
	}
	//
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWeaponItem::Unload( IInventory *pInventory )
{
	ASSERT( IsValid( pInventory ) );
	if ( !IsValid( pInventory ) || pInnerClip->GetIncQuantity() <= 0 )
		return false;
	//
	while ( pInnerClip->GetIncQuantity() > 0 )
	{
		if ( CDynamicCast<CClipItem> pUnloadedClip( CreateClipItem( pInnerClip->GetDBClip(), 
			pInnerClip->GetDBAmmo(), 0 ) ) )
		{
			int nUnload = min( pInnerClip->GetIncQuantity(), pInnerClip->GetDBClip()->nQuantity );
			CPtr<IJoinSplit> pGet = pInnerClip->SplitItem( nUnload );
			if ( IsValid( pGet ) )
				pUnloadedClip->JoinItem( pGet );
			pInventory->Place( CTPoint<int>( -1, -1 ), pUnloadedClip );
		}
		return true;
	}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGrenadeItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CGrenadeItem::CGrenadeItem( NDb::CRPGGrenade *_pDBGrenade )
	: CInventoryItem( _pDBGrenade->pItem ), pDBGrenade(_pDBGrenade) 
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMeleeWeaponItem
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMeleeWeaponItem::CreateNewAttackPortion( vector<CAttackPortion> *pRes )
{
	pRes->push_back( CAttackPortion( 
		110, 2, 
		pDBMelee->nDmgMin, pDBMelee->nDmgMax, 
		0, 0 ) ); // piercing ability(+str*10), вероятность и сложность critical вычисляется в RPGUnitMission
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CFirstAidItem::CFirstAidItem( NDb::CRPGFirstAid *_pDBFirstAid )
	:CInventoryItem( _pDBFirstAid->pItem ), pDBFirstAid(_pDBFirstAid) 
{
	pClip = new CPotionContainer();
	CPotionItem *pI = new CPotionItem();
	pI->nQuantity = _pDBFirstAid->nQuantity;
	pClip->Load( pI );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFirstAidItem::SpendPotion()
{
	ASSERT( !IsEmpty() );
	if ( !IsEmpty() )
		return;
	CObj<IJoinSplit> pSpent = pClip->SplitItem( 1 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFirstAidItem::IsEmpty()
{
	if ( !IsValid( pClip ) )
	{
		ASSERT(0);
		return true;
	}
	return pClip->GetIncQuantity() <= 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMeleeWeaponItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CMeleeWeaponItem::CMeleeWeaponItem( NDb::CRPGMeleeWeapon *_pDBMelee )
	: CInventoryItem( _pDBMelee->pItem ), pDBMelee(_pDBMelee) 
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUniformItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CUniformItem::CUniformItem( NDb::CRPGUniform *_pDBUniform )
	: CInventoryItem( _pDBUniform->pItem ), pDBUniform(_pDBUniform) 
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMineDetectorItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CMineDetectorItem::CMineDetectorItem( NDb::CRPGMineDetector *_pDBMineDetector )
	: CInventoryItem( _pDBMineDetector->pItem ), pDBMineDetector(_pDBMineDetector) 
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IInventoryItem *CreateClipItem( NDb::CRPGClip *pDBClip, NDb::CRPGAmmo *pDBAmmo, int nAmmoQuantity )
{
	ASSERT( IsValid( pDBClip ) );
	if ( !IsValid( pDBClip ) )
		return 0;
	//
	CDBPtr<NDb::CRPGAmmo> pTmpDBAmmo = pDBAmmo;
	if ( !IsValid( pTmpDBAmmo ) )
	{
		// ищем подходящие патроны
		CDBTable<NDb::CRPGAmmo> *pAmmoTable = NDatabase::GetTable<NDb::CRPGAmmo>();
		CDBIterator<NDb::CRPGAmmo> i(*pAmmoTable);
		while ( pAmmoTable && i.MoveNext() )
		{
			CDBPtr<NDb::CRPGAmmo> pTableRecord = i.Get();
			if ( pTableRecord->nAmmoGroup == pDBClip->nAmmoGroup )
			{
				pTmpDBAmmo = pTableRecord;
				break;
			}
		}
	}
	if ( IsValid( pTmpDBAmmo ) )
	{
		CClipItem *pClip = new CClipItem( pDBClip );
		if ( IsValid( pClip ) )
		{
			CPtr<NRPG::CAmmoItem> pAmmo = new CAmmoItem( pTmpDBAmmo );
			pAmmo->nQuantity = nAmmoQuantity < 0 ? pClip->GetMaxIncQuantity() : nAmmoQuantity;
			pClip->Load( pAmmo );
		}
		return pClip;
	}
	else
	{
		// В базе данных нет подходящего clip-а
		// Ошибка дизайнеров
		ASSERT( 0 );
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IWeaponItem *CreateWeaponItem( NDb::CRPGWeapon *pDBWeapon )
{
	ASSERT( IsValid( pDBWeapon ) );
	if ( !IsValid( pDBWeapon ) )
		return 0;
	//
	return new CWeaponItem( pDBWeapon );
}
IInventoryItem *CreateGrenadeItem( NDb::CRPGGrenade *pDBGrenade )
{
	CGrenadeItem *pGrenade = new CGrenadeItem(pDBGrenade);
	return pGrenade;
}
IInventoryItem *CreateUniformItem( NDb::CRPGUniform *pDBUniform )
{
	CUniformItem *pUniform = new CUniformItem(pDBUniform);
	return pUniform;
}
IInventoryItem* CreateFirstAidItem( NDb::CRPGFirstAid *pDBFirstAid )
{
	CFirstAidItem *pFA = new CFirstAidItem(pDBFirstAid);
	return pFA;
}
IInventoryItem* CreateMeleeWeaponItem( NDb::CRPGMeleeWeapon *pDBMelee )
{
	CMeleeWeaponItem *pMW = new CMeleeWeaponItem(pDBMelee);
	return pMW;
}
IInventoryItem* CreateMineDetectorItem( NDb::CRPGMineDetector *pDBDetector )
{
	CMineDetectorItem *pMD = new CMineDetectorItem(pDBDetector);
	return pMD;
}
IInventoryItem* CreateItem( const CDBRecord *pItem )
{
	IInventoryItem *pIItem = 0;
	if( CDynamicCast<NDb::CRPGWeapon> pWeapon(pItem) )
		pIItem = CreateWeaponItem( pWeapon );
	else if( CDynamicCast<NDb::CRPGClip> pClip(pItem) )
		pIItem = CreateClipItem( pClip );
	else if( CDynamicCast<NDb::CRPGGrenade> pGrenade(pItem) )
		pIItem = CreateGrenadeItem( pGrenade );
	else if( CDynamicCast<NDb::CRPGUniform> pUniform(pItem) )
		pIItem = CreateUniformItem( pUniform );
	else if( CDynamicCast<NDb::CRPGFirstAid> pFirstAid(pItem) )
		pIItem = CreateFirstAidItem( pFirstAid );
	else if( CDynamicCast<NDb::CRPGMeleeWeapon> pMelee(pItem) )
		pIItem = CreateMeleeWeaponItem( pMelee );
	else if( CDynamicCast<NDb::CRPGMineDetector> pMD(pItem) )
		pIItem = CreateMineDetectorItem( pMD );
	else
		ASSERT( "Unsupport item type" && 0 );
	return pIItem;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NRPG;
REGISTER_SAVELOAD_CLASS( 0xE10A1140, CWeaponItem );
REGISTER_SAVELOAD_CLASS( 0xE10A1141, CClipItem );
REGISTER_SAVELOAD_CLASS( 0xE10A1142, CAmmoItem );
REGISTER_SAVELOAD_CLASS( 0x101B1170, CGrenadeItem );
REGISTER_SAVELOAD_CLASS( 0x108B1120, CUniformItem );
REGISTER_SAVELOAD_CLASS( 0xA1112153, CFirstAidItem );
REGISTER_SAVELOAD_CLASS( 0xA1512132, CMeleeWeaponItem );
REGISTER_SAVELOAD_CLASS( 0xA2312140, CPotionItem );
REGISTER_SAVELOAD_CLASS( 0xA2312141, CPotionContainer );
REGISTER_SAVELOAD_CLASS( 0x11462170, CMineDetectorItem );
