#ifndef __LSHEAD_H_
#define __LSHEAD_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Time.h"
#include "GResource.h"
#include <LifeStudioHeadAPI.h>
#include <LifeStudioHeadAPIMMTS.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
	class CHead;
	class CSequence;
}
namespace NGScene
{
	class CObjectInfo;
}
namespace NLSHead
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CLSPtr
{
	T *ptr;
public:
	CLSPtr( T *_ptr = 0 ): ptr(_ptr) {}
	~CLSPtr() { if ( ptr ) ptr->Destroy(); }

	CLSPtr& operator=( T *_ptr ) { if ( ptr ) ptr->Destroy(); ptr = _ptr; return *this; }
	T* operator->() const { return ptr; }
	operator T*() const { return ptr; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeadMeshInfo: public CObjectBase
{
	OBJECT_BASIC_METHODS(CHeadMeshInfo);
public:
	vector< CLSPtr<LifeStudioHeadAPI::IAnimator> > pLSAnimators;
	vector< int > nVertices;
	vector< CTPoint<int> > copys;
	vector< CVec2 > UVs;
	vector< WORD > indices;
	vector< int > tris;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeadSequenceInfo: public CObjectBase
{
	OBJECT_BASIC_METHODS(CHeadSequenceInfo);
public:
	CLSPtr<LifeStudioHeadAPI::ISequencer> pLSSequence;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeadMeshLoader: public NGScene::CResourceLoader<int, CHeadMeshInfo>
{
	OBJECT_BASIC_METHODS(CHeadMeshLoader);
protected:
	virtual void Recalc();	
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeadSequenceLoader: public NGScene::CResourceLoader<int, CHeadSequenceInfo>
{
	OBJECT_BASIC_METHODS(CHeadSequenceLoader);
protected:
	virtual void Recalc();	
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeadBound: public CFuncBase<SBound>
{
	OBJECT_BASIC_METHODS(CHeadBound);
	ZDATA
	CDGPtr< CFuncBase<SFBTransform> > pParent;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pParent); return 0; }
protected:
	virtual bool NeedUpdate() { return pParent.Refresh(); }
	virtual void Recalc();
public:
	CHeadBound() {}
	CHeadBound( CFuncBase<SFBTransform> *_pParent ): pParent(_pParent) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SHeadFrame
{
	vector<CVec3> mesh;
	vector<CVec3> normals; // not normalized
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeadAnimator: public CFuncBase<SHeadFrame>
{
	OBJECT_BASIC_METHODS(CHeadAnimator);
	ZDATA
	CDBPtr< NDb::CHead > pDbHead;
	CDGPtr< CFuncBase<STime> > pTime;
	CDGPtr< CPtrFuncBase<CHeadMeshInfo> > pHead;
	// current sequence
	CDGPtr< CPtrFuncBase<CHeadSequenceInfo> > pSequence;
	STime tStart;
	bool bCycle;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pDbHead); f.Add(3,&pTime); f.Add(4,&pHead); f.Add(5,&pSequence); f.Add(6,&tStart); f.Add(7,&bCycle); return 0; }
protected:
	virtual bool NeedUpdate() { return pTime.Refresh() | pHead.Refresh(); }
	virtual void Recalc();
public:
	CHeadAnimator() {}
	CHeadAnimator( CFuncBase<STime> *_pTime, NDb::CHead *_pDbHead );

	void PlaySequence( NDb::CSequence *pDbSeq, STime tStart, bool bCycle = false );
	void StopSequence();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHead : public CPtrFuncBase<NGScene::CObjectInfo>
{
	OBJECT_BASIC_METHODS(CHead);
	ZDATA
	CDGPtr< CFuncBase<SFBTransform> > pParent;
	CDGPtr< CFuncBase<SHeadFrame> > pAnimator;
	CDGPtr< CPtrFuncBase<CHeadMeshInfo> > pHead;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pParent); f.Add(3,&pAnimator); f.Add(4,&pHead); return 0; }
protected:
	virtual bool NeedUpdate() { return pAnimator.Refresh() | pParent.Refresh() | pHead.Refresh(); }
	virtual void Recalc();
public:
	CHead() {}
	CHead( CFuncBase<SFBTransform> *_pParent, CFuncBase<SHeadFrame> *_pAnimator, CPtrFuncBase<CHeadMeshInfo> *_pHead ):
		pParent(_pParent), pAnimator(_pAnimator), pHead(_pHead) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif