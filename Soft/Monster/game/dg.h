#ifndef __DG_H_
#define __DG_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
namespace NDG
{
	extern int nCurrentFrame;
/////////////////////////////////////////////////////////////////////////////////////
// DG 
/////////////////////////////////////////////////////////////////////////////////////
// base node can be updated and store last updated mark, when update counter switches
// node has to recheck if its parameters changed and if so recalc its value
class CBaseNode: public CObjectBase
{
	int nFrameCalced;     // mark of last update frame, used to cut recalc time
	//
	void DoUpdate() { ASSERT( IsValid() ); Update(); nFrameCalced = NDG::nCurrentFrame; }
protected:
	virtual void Update();
public:
	CBaseNode() { nFrameCalced = 0; }
	//
	friend class CBaseScene;
	friend class CAccessForPtr;
};
/////////////////////////////////////////////////////////////////////////////////////
// any function should have version of its value to recalc only when function result
// has changed
class CVersioningBase: public CBaseNode
{
	int nVersion;         // current version number, used to determine if setParam needed
protected:
	void Updated() { nVersion++; } // should be called when function return recalc its value
public:
	CVersioningBase() { nVersion = 1; }
	void MarkDirty() { nVersion++; } // pScene->NextFrame(); }
	friend class CAccessForPtr;
};
/////////////////////////////////////////////////////////////////////////////////////
template <class TResult>
class CFuncBase: public CVersioningBase
{
protected:
	TResult value;
public:
	CFuncBase() {}
	CFuncBase( const TResult &_t ): value(_t) {}
	const TResult& GetValue() const { return value; }
};
/////////////////////////////////////////////////////////////////////////////////////
template <class TResult>
class CPtrFuncBase: public CVersioningBase
{
protected:
	CObj<TResult> pValue;
public:
	TResult* GetValue() const { return pValue; }
};
/////////////////////////////////////////////////////////////////////////////////////
#define DEFINE_DG_CONSTANT_NODE( ClassName, Type ) \
class ClassName: public CFuncBase<Type>            \
{                                                  \
	OBJECT_BASIC_METHODS(ClassName);                 \
public:                                            \
	ClassName() {}                                   \
	ClassName( const Type &_t ): CFuncBase<Type>(_t) { MarkDirty(); }\
	void Set( const Type &_t ) { value = _t; MarkDirty(); }\
	void Serialize( CStructureSaver *pFile ) { pFile->AddData( 1, &value ); }\
};
/////////////////////////////////////////////////////////////////////////////////////
// for use in CPtr only!
class CAccessForPtr
{
public:
	int GetFrameCalced( CBaseNode *pNode ) const { return pNode->nFrameCalced; }
	void DoUpdate( CBaseNode *pNode ) const { pNode->DoUpdate(); }
	int GetVersion( CVersioningBase *pNode ) const { return pNode->nVersion; }
	template <class T>
		bool IsStorageOk( CPtrFuncBase<T> &node ) const { return node.GetValue()->IsValid(); }
	template <class T>
		bool IsStorageOk( CFuncBase<T> &node ) const { return true; }
};
/////////////////////////////////////////////////////////////////////////////////////
class CBaseScene: public CFundament
{
	int nFrame;
	//
public:
	CBaseScene();
	// sets current frame counter to scene counter, after this update for nodes belonging
	// this scene work correctly, should be called before any node update because there
	// can be several scenes in program each with its own frame counter
	void Use() { NDG::nCurrentFrame = nFrame; }
	void UseUpdated() { nFrame++; NDG::nCurrentFrame = nFrame; }
	//
	void Update( CBaseNode *pNode )
	{
		if ( pNode->IsValid() )
			pNode->DoUpdate();
	}
	template <class TSet>
		void UpdateValidSet( const TSet &a )
		{
			for ( TSet::const_iterator i = a.begin(); i != a.end(); ++i )
			{
				CBaseNode *pNode = (CBaseNode*)i->GetPtr();
				pNode->DoUpdate();
			}
		}
	template <class TSet>
		void UpdateSet( TSet *a )
		{
			for ( TSet::iterator i = a->begin(); i != a->end(); )
			{
				CBaseNode *pNode = (CBaseNode*)i->GetPtr();
				if ( pNode->IsValid() )
				{
					pNode->DoUpdate();
					++i;
				}
				else
				{
					TSet::iterator k = i;
					++i;
					a->erase( k );
				}
			}
		}
	friend class CVersioningBase;
};
/////////////////////////////////////////////////////////////////////////////////////
} // namespace NDG
/////////////////////////////////////////////////////////////////////////////////////
template <class TFunc>
class CDGPtr
{
	CPtr<TFunc> pNode;
	int nVersion;
public:
	CDGPtr() { nVersion = 0; }
	CDGPtr( const CDGPtr<TFunc> &a ): pNode( a.pNode ) { nVersion = 0; }
	CDGPtr( TFunc *_pNode ): pNode(_pNode) { nVersion = 0; }
	CDGPtr& operator=( const CDGPtr<TFunc> &a ) { pNode = a.pNode; nVersion = 0; return *this; }
	CDGPtr& operator=( TFunc *_pNode ) { pNode = _pNode; nVersion = 0; return *this; }
	//
	operator TFunc*() const { return pNode; }
	TFunc* operator->() const { return pNode; }
	bool Refresh()
	{
		ASSERT( pNode->IsValid() );
		NDG::CAccessForPtr cppIsWeak;
		bool bFrameMismatch = cppIsWeak.GetFrameCalced( pNode ) != NDG::nCurrentFrame;
		if ( bFrameMismatch || !cppIsWeak.IsStorageOk( *pNode ) )
			cppIsWeak.DoUpdate( pNode );
		bool bResult = nVersion != cppIsWeak.GetVersion( pNode );
		nVersion = cppIsWeak.GetVersion( pNode );
		return bResult;
	}
	friend struct SDGAdd;
};
/////////////////////////////////////////////////////////////////////////////////////
struct SDGAdd
{
	template<class T>
		CPtr<T>& GetForSerialisation( CDGPtr<T> *p ) { return p->pNode; }
};
template <class T>
inline void Serialize( CStructureSaver *pFile, CDGPtr<T> *pObj )
{
	SDGAdd cppIsWeak;
	pFile->DoPtr( &cppIsWeak.GetForSerialisation( pObj ) );
}
/////////////////////////////////////////////////////////////////////////////////////
#endif