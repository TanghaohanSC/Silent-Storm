#ifndef __BASICFACTORY_H_
#define __BASICFACTORY_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
struct SDefaultPtrHash
{
	int operator()( const void *pData ) const { return (int)pData; }
};
/////////////////////////////////////////////////////////////////////////////////////
// factory for objects with virtual funcs and vft at first dword of object`s data
// multiple inheritance is prohibited, objects should inherit T and 
// T must have at least 1 virtual function
template <class T>
class CClassFactory
{
	typedef T* (*newFunc)();
	typedef std::hash_map<int, newFunc> CTypeNewHash;                  // typeID->newFunc()
	typedef std::hash_map<void*, int, SDefaultPtrHash> CTypeIndexHash; // vftable->typeID

	CTypeIndexHash typeIndex;
	CTypeNewHash typeInfo;

	void RegisterTypeBase( int nTypeID, newFunc func );
	static void* GetObjectType( void *pObject ) { return ((void**)pObject)[0]; }
public:
	template < class TT >
		void RegisterType( int nTypeID, TT* (*func)() ) { RegisterTypeBase( nTypeID, (newFunc) func ); }
	T* CreateObject( int nTypeID ) { newFunc f = typeInfo[ nTypeID ]; if ( f ) return f(); return 0; }
	int GetObjectTypeID( T *pObject );
};
/////////////////////////////////////////////////////////////////////////////////////
template <class T>
int CClassFactory<T>::GetObjectTypeID( T *pObject )
{
	CTypeIndexHash::const_iterator i = typeIndex.find( GetObjectType( pObject ) );
	if ( i != typeIndex.end() )
		return i->second;
	return -1;
}
/////////////////////////////////////////////////////////////////////////////////////
template <class T>
void CClassFactory<T>::RegisterTypeBase( int nTypeID, newFunc func )
{
	CPtr<T> pObj = func();
	ASSERT( typeInfo.find( nTypeID ) == typeInfo.end() );
	ASSERT( typeIndex.find( GetObjectType( pObj ) ) == typeIndex.end() );
	typeIndex[ GetObjectType( pObj ) ] = nTypeID;
	typeInfo[ nTypeID ] = func;
}
/////////////////////////////////////////////////////////////////////////////////////
// macro for registering CFundament derivatives
#define REGISTER_CLASS( factory, N, name ) factory.RegisterType( N, name##::New##name );
/////////////////////////////////////////////////////////////////////////////////////
#endif // __BASICFACTORY_H_
