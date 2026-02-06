#include "StdAfx.h"
#include "wDebris.h"
#include "wInterface.h"
#include "GAnimParticles.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataGeometry.h"
#include "aiMap.h"
#include "GAnimation.h"
#include "RPGItemInfo.h"
#include "Transform.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\RandomGen.h"
#include "RPGAttackMech.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDFrozenItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CDFrozenItem::CDFrozenItem( CSyncSrc<IVisObj> *pShow, NDb::CModel *_pModel, const SHMatrix &_m,
	int _nFloor, NRPG::IInventoryItem *_pItem, CDebrisControllerTrash *_pTrash )
	: pModel(_pModel), m(_m), nFloor(_nFloor), pInvItem(_pItem), pTrash( _pTrash )
{
	bindGlobal.Link( pShow, this );
	//
	if ( IsValid( pModel ) )
		nMaxVP = pModel->GetMaxVP();
	else
		nMaxVP = 1;
	//
	nVP = nMaxVP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CModel* CDFrozenItem::GetModel() const 
{ 
	return pModel; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CDFrozenItem::ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor )
{
	if ( IsValid( pArmor ) && nVP > 0 )
	{
		int nDmg = pAttack->CalcStructDmg(pArmor);
		nVP = Max( 0, nVP - nDmg );
		if ( nVP == 0 )
			pTrash->itemsToRemove.push_back( this );
		return nDmg;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDFrozenItem::Visit( IRenderVisitor *p )
{
	SFBTransform pos;
	pos.forward = m;
	pos.backward.HomogeneousInverse( m );
	p->AddMesh( pModel, pos, 0, nFloor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDFrozenItem::Visit( IAIVisitor *p )
{
	SFBTransform pos;
	pos.forward = m;
	pos.backward.HomogeneousInverse( m );
	p->AddHull( pModel->pGeometry->pAIGeometry, pos, pModel->pRPGArmor, nFloor, TS_PICK|TS_FRAGMENTED|TS_GO_OVER|TS_VISION|TS_COVER|TS_WEAPON_BLOCKER );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDItem
////////////////////////////////////////////////////////////////////////////////////////////////////
CDItem::CDItem( CSyncSrc<IVisObj> *pShow, NDb::CModel *_pModel, CFuncBase<NAnimation::SSkeletonPose> *_pAnim,
	int _nFloor, NRPG::IInventoryItem *_pItem )
	: pModel(_pModel), pAnimation(_pAnim), nFloor(_nFloor), pInvItem(_pItem)
{
	bindGlobal.Link( pShow, this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDItem::Visit( IRenderVisitor *p )
{
	p->AddItemMesh( pModel, pAnimation );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDebrisController
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDebrisController::Add( CDItem *pItem, NAnimation::CASphereSet *pAnim )
{
	STrackItem d;
	d.pItem = pItem;
	d.pAnim = pAnim;
	items.push_back( d );
	showItems.push_back( pItem );
	InitAction();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDebrisController::GetStopped( list<STrackItem> *pRes )
{
	for ( list<STrackItem>::iterator i = items.begin(); i != items.end(); )
	{
		//i->pAnim.Refresh();
		if ( i->pAnim->HasStopped() )
		{
			list<STrackItem>::iterator k = i++;
			pRes->splice( pRes->end(), items, k );
		}
		else
			++i;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
struct SRangeTest
{
	const SSphere &s;
	float fR2;
	list<CObj<T> > &res;
	SRangeTest( const SSphere &_sp, list<CObj<T> > &_res ): s(_sp), res(_res) { fR2 = sqr( s.fRadius ); }
	bool operator()( T *p ) const 
	{
		if ( fabs2( s.ptCenter - p->GetPos() ) < fR2 )
		{
			res.push_back( p );
			return true;
		}
		return false;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDebrisController::GetInSphere( const SSphere &sphere, list< CObj<CDFrozenItem> > *pRes )
{
	SRangeTest<CDFrozenItem> testFI( sphere, *pRes );
	for ( list<CObj<CDFrozenItem> >::iterator i = showFrozenItems.begin(); i != showFrozenItems.end(); )
	{
		if ( testFI( *i ) )
			i = showFrozenItems.erase( i );
		else
			++i;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDebrisController::AddDebris( NDb::CModel *pModel, NAI::IAIMap *pAIMap, const CVec3 &ptCenter, const CQuat &q,
	const CVec3 &velocity, CFuncBase<STime> *pTime, NRPG::IInventoryItem *_pItem )
{
	NDb::CAIGeometry *pAIGeom = pModel->pGeometry->pAIGeometry;
	if ( !pAIGeom )
		NStr::DebugTrace( "Geometry %d has no AI geometry\n", pModel->pGeometry->GetRecordID() );
	vector<SMassSphere> spheres;
	CVec3 massCenter;
	NAI::GetSpheres( pModel, &spheres, &massCenter );
	NAnimation::CASphereSet *pSphere = new NAnimation::CASphereSet;
	pSphere->pTime = pTime;
	pSphere->pMap = pAIMap;
	pSphere->InitSpheres( spheres );
	pSphere->InitBound( pModel->pGeometry->boundCenter, pModel->pGeometry->boundSize );
	pSphere->Init( pTime->GetValue(), ptCenter, q, velocity );
	NAnimation::CSkeletonAnimator *pAnimator = new NAnimation::CSkeletonAnimator;
	pAnimator->pTime = pTime;
	pAnimator->AddAnimator( pTime->GetValue(), pSphere );

	CDItem *pI = new CDItem( GetShowList(), pModel, pAnimator, 0, _pItem ); // CRAP nFloor
	Add( pI, pSphere );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDebrisController::ActivateDebris( const SSphere &sphere, NAI::IAIMap *pAIMap, 
	CFuncBase<STime> *pTime )
{
	bc.Add( sphere.ptCenter, sphere.fRadius );
	list<CObj<CDFrozenItem> > affected;
	GetInSphere( sphere, &affected );
	for ( list<CObj<CDFrozenItem> >::iterator i = affected.begin(); i != affected.end(); ++i )
	{
		CDFrozenItem *p = *i;
		CVec3 pos = p->GetPos();
		CVec3 vel = pos - sphere.ptCenter;
		float fDist = fabs( vel );
		Normalize( &vel ); vel += CVec3(0,0,1);
		vel *= Max( 0.0f, 1 - sqr( fDist / sphere.fRadius ) );
		AddDebris( p->GetModel(), pAIMap, pos, QNULL, vel, pTime, p->GetInvItem() );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDebrisController::AddFrozenItem( const CVec3 &pos, const CQuat &rot, NRPG::IInventoryItem *pInvItem, int nFloor )
{
	CPtr<NRPG::IInventoryItem> pHold( pInvItem );
	if ( !IsValid( pInvItem->GetDBItem()->pModel ) )
	{
		ASSERT( 0 );
		return;
	}
	SRand rnd;
	SHMatrix m;
	MakeMatrix( &m, pos, rot );
	CDFrozenItem *pWorldItem = new CDFrozenItem( 
		GetShowList(), pInvItem->GetDBItem()->pModel->CreateModel( &rnd ), m, nFloor, pInvItem, pTrash ); // CRAP nFloor etc
	showFrozenItems.push_back( pWorldItem );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDebrisController::RemoveFrozenItem( NRPG::IInventoryItem *pInvItem )
{
	list<CObj<CDFrozenItem> >::iterator it;
	for ( it = showFrozenItems.begin(); it != showFrozenItems.end(); ++it )
	{
		if ( (*it)->GetInvItem() == pInvItem )
		{
			showFrozenItems.erase( it );
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDebrisController::Segment( SSphere *pInvalidate )
{
	bool bRes = false;
	if ( HasDynamicItems() )
	{
		list<STrackItem> stopped;
		GetStopped( &stopped );
		for ( list<STrackItem>::iterator i = stopped.begin(); i != stopped.end(); ++i )
		{
			CDItem *pI = i->pItem;
			CDGPtr< CFuncBase<NAnimation::SSkeletonPose> > pPos = pI->GetAnimation();
			pPos.Refresh();
			SHMatrix pos;
			const NAnimation::SBonePose &bone = pPos->GetValue()[0];
			MakeMatrix( &pos, bone.pos, bone.rot );
			CDFrozenItem *p = new CDFrozenItem( GetShowList(), 
				pI->GetModel(), pos, 0, pI->GetInvItem(), pTrash );
			showFrozenItems.push_back( p );
			showItems.remove( pI );
			bc.Add( bone.pos, 1 );
		}
	}
	if ( !HasDynamicItems() )
	{
		pDebrisAction = 0;
		if ( !bc.IsEmpty() )
		{
			bc.Make( pInvalidate );
			bc.Clear();
			bRes = true;
		}
	}
	//
	for ( list< CPtr<CDFrozenItem> >::iterator i = pTrash->itemsToRemove.begin();
		i != pTrash->itemsToRemove.end(); ++i )
	{
		CPtr<NRPG::IInventoryItem> pInventoryItem = (*i)->GetInvItem();
		if ( !IsValid( pInventoryItem ) )
			continue;
		//
		CDBPtr<NDb::CTEffect> pTEffect = pInventoryItem->GetDBItem()->pDestructionEffect;
		if ( IsValid( pTEffect ) )
		{
			SRand rand;
			CQuat rot = CQuat( random.GetFloat( 0, 10000 ), CVec3( 0, 0, 1 ) );
			CDBPtr<NDb::CEffect> pEffect = pTEffect->GetEffect( &rand );
			if ( IsValid( pEffect ) )
				CreateParticle( (*i)->GetPos(), rot, pEffect );
		}
		//
		//OnFrozenItemDestroyed( (*i)->GetInvItem()->GetDBItem()->pSuccessor->GetRecordID() );
		OnFrozenItemDestroyed( (*i)->GetInvItem()->GetDBItem()->GetRecordID() );
		RemoveFrozenItem( (*i)->GetInvItem() );
	}
	pTrash->itemsToRemove.clear();
	//
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NWorld;
//REGISTER_SAVELOAD_CLASS( 0x12781170, CDebrisController );
//REGISTER_SAVELOAD_CLASS( 0x018C1120, CRealDebrisPiece )
REGISTER_SAVELOAD_CLASS( 0x009b1140, CDFrozenItem )
REGISTER_SAVELOAD_CLASS( 0x130A1191, CDItem )
REGISTER_SAVELOAD_CLASS( 0x50692130, CDebrisControllerTrash )