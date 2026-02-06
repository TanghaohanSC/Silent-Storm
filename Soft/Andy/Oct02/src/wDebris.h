#ifndef __WDEBRISCONTROLLER_H_
#define __WDEBRISCONTROLLER_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "wInterface.h"
#include "Bound.h"
#include "RPGAttackMech.h"
namespace NRPG
{
	class CAttackPortion;
}
namespace NDb
{
	class CRPGArmor;
}
namespace NAnimation
{
	class CASphereSet;
}
namespace NWorld
{
class	CDFrozenItem;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDebrisControllerTrash: public CObjectBase
{
	OBJECT_BASIC_METHODS( CDebrisControllerTrash );
	ZDATA
public:
	list< CPtr<CDFrozenItem> > itemsToRemove;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&itemsToRemove); return 0; }
	//
	CDebrisControllerTrash() {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDFrozenItem: public IItem, public NRPG::IAttackable
{
	OBJECT_NOCOPY_METHODS(CDFrozenItem);
	ZDATA
	CPtr<NDb::CModel> pModel;
	SHMatrix m;
	int nFloor;
	CObj<NRPG::IInventoryItem> pInvItem;
	CSyncSrcBind<IVisObj> bindGlobal;
	int nVP, nMaxVP;
	CPtr<CDebrisControllerTrash> pTrash;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pModel); f.Add(3,&m); f.Add(4,&nFloor); f.Add(5,&pInvItem); f.Add(6,&bindGlobal); f.Add(7,&nVP); f.Add(8,&nMaxVP); f.Add(9,&pTrash); return 0; }
public:
	CDFrozenItem() {}
	CDFrozenItem( CSyncSrc<IVisObj> *pShow, NDb::CModel *_pModel, const SHMatrix &m, 
		int _nFloor, NRPG::IInventoryItem *_pItem, CDebrisControllerTrash *_pTrash );
	const SHMatrix& GetMatrix() const { return m; }
	CVec3 GetPos() const { return m.GetTranslation(); }
	NDb::CModel* GetModel() const;
	NRPG::IInventoryItem* GetInvItem() const { return pInvItem; }
	virtual int ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor );
	virtual void Visit( IRenderVisitor *p );
	virtual void Visit( IAIVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDItem: public IVisObj
{
	OBJECT_NOCOPY_METHODS(CDItem);
	ZDATA
	CPtr<NDb::CModel> pModel;
	CObj< CFuncBase<NAnimation::SSkeletonPose> > pAnimation;
	int nFloor;
	CObj<NRPG::IInventoryItem> pInvItem;
	CSyncSrcBind<IVisObj> bindGlobal;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pModel); f.Add(3,&pAnimation); f.Add(4,&nFloor); f.Add(5,&pInvItem); f.Add(6,&bindGlobal); return 0; }
public:
	CDItem() {}
	CDItem( CSyncSrc<IVisObj> *pShow, NDb::CModel *_pModel, CFuncBase<NAnimation::SSkeletonPose> *_pAnim,
		int _nFloor, NRPG::IInventoryItem *_pItem );
	CFuncBase<NAnimation::SSkeletonPose>* GetAnimation() const { return pAnimation; }
	NDb::CModel* GetModel() const { return pModel; }
	NRPG::IInventoryItem* GetInvItem() const { return pInvItem; }
	//
	virtual void Visit( IRenderVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDebrisController
{
	struct STrackItem
	{
		ZDATA
		CPtr<CDItem> pItem;
		CPtr<NAnimation::CASphereSet> pAnim;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pItem); f.Add(3,&pAnim); return 0; }
	};
	ZDATA
	list<STrackItem> items;
	CObj<CActionCounter> pDebrisAction;
	SBoundCalcer bc;
protected:
	list<CObj<CDItem> > showItems;
	list<CObj<CDFrozenItem> > showFrozenItems;
	CObj<CDebrisControllerTrash> pTrash;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&items); f.Add(3,&pDebrisAction); f.Add(4,&bc); f.Add(5,&showItems); f.Add(6,&showFrozenItems); f.Add(7,&pTrash); return 0; }
private:
	void Add( CDItem *pD, NAnimation::CASphereSet *pAnim );
	void GetStopped( list<STrackItem> *pRes );
	void GetInSphere( const SSphere &sphere, list<CObj<CDFrozenItem> > *pRes );
	void InitAction() { if ( !IsValid( pDebrisAction ) ) pDebrisAction = CreateActionCounter(); }
protected:
	bool Segment( SSphere *pInvalidate );
	bool HasDynamicItems() { return !items.empty(); }
	virtual CActionCounter* CreateActionCounter() = 0;
	virtual CSyncSrc<IVisObj>* GetShowList() = 0;
	virtual void OnFrozenItemDestroyed( int nItemID ) = 0;
	virtual void CreateParticle( const CVec3 &ptPos, const CQuat &rot, NDb::CEffect *pEffect ) = 0;
public:
	CDebrisController() { pTrash = new CDebrisControllerTrash(); }
	//! add new piece of debris
	void AddDebris( NDb::CModel *pModel, NAI::IAIMap *pMap, const CVec3 &ptCenter, const CQuat &q, const CVec3 &velocity, 
		CFuncBase<STime> *pTime, NRPG::IInventoryItem *pItem = 0 );
	//! turn in radius frozen items into alive ones
	void ActivateDebris( const SSphere &b, NAI::IAIMap *pAIMap, CFuncBase<STime> *pTime );
	//! put RPG item into fixed position
	void AddFrozenItem( const CVec3 &pos, const CQuat &rot, NRPG::IInventoryItem *pInvItem, int nFloor = 0 );
	//! remove frozen item by RPG pointer
	void RemoveFrozenItem( NRPG::IInventoryItem *pInvItem );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif