#ifndef __BASIC1_H_
#define __BASIC1_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
class CStructureSaver;
/////////////////////////////////////////////////////////////////////////////////////
// single thread version
// базовые классы для системы подсчета ссылок на объекты с целью упрощения memory
// management
//   Поддерживаются копируемые объекты, 
// наследуются от CObjectBase, подразумевается, что корректные
// конструктор копирования и оператор присваивания генерируются автоматически
// при использовании макро OBJECT_BASIC_METHODS, для того,
// чтобы это происходило все members производных классов должны иметь корректные конструкторы
// копирования и операторы присваивания. Кроме того наследники от CObjectBase получают
// корректные методы Clear() и Duplicate() без каких либо усилий.
//   При наследовании от CObjectBase настойчиво рекомендуется использовать макрос
// OBJECT_BASIC_METHODS для определения всех методов, необходимых для функционирования
// системы подсчета ссылок.
//   Есть разные шаблоны указателей - простые указатели - CPtr с добавлением ссылки и
// указатели, обладающие правом собственности - CObj. После уничтожения CObj объект, на который
// он ссылался очищается и помечается как невалидный. Аналогично действует CMObj организуя
// паралелльное владение объектом (используется когда объект может быть уничточжен из-за
// влияния внешних обстоятельств)
//   Ограничения - при множественном наследовании наследник от CObjectBase должен быть первым 
// из-за реализации CObjectBase::operator new. Использование SRef* рассматривается 
// как читерство, так как позволяет обойти подсчет ссылок. Нельзя переопределять 
// operator new, так как удалятся они будут с помощью стандартного operator`a delete 
// (из-за delete this) и так как в operator new надо позвать InitRefCount()
/////////////////////////////////////////////////////////////////////////////////////
class CObjectBase
{
private:
	int nRefData;
	//
	void InitRefCount() { nRefData = 0; }
	//
	void AddRef() { nRefData++; }
	void DecRef() { nRefData--; }
	void Release() 
	{ 
		nRefData--; 
		if ( (nRefData & 0x7fffffff) == 0 ) 
			delete this; 
	}
	void AddRefO() { nRefData += 0x1000; }
	void DecRefO() { nRefData -= 0x1000; }
	void ReleaseO()
	{
		nRefData -= 0x1000; 
		if ( (nRefData & 0x7fffffff) == 0 ) 
			delete this;
		else if ( (nRefData & 0x00fff000) == 0 )
		{
			nRefData |= 0x80000000;
			AddRef(); 
			DestroyContents(); 
			Release();
		}
	}
	void AddRefM() { nRefData += 0x1000000; }
	void DecRefM() { nRefData -= 0x1000000; }
	void ReleaseM()
	{
		nRefData -= 0x1000000; 
		if ( (nRefData & 0x7fffffff) == 0 ) 
			delete this;
		else if ( (nRefData & 0x7f000000) == 0 )
		{
			nRefData |= 0x80000000;
			AddRef(); 
			DestroyContents(); 
			Release();
		}
	}
protected:
	// function should clear contents of object, easy to implement via consequent calls to
	// destructor and constructor, this function should not be called directly, use Clear()
	virtual void DestroyContents() = 0;
	virtual CObjectBase* MakeCopy() const = 0;
	virtual ~CObjectBase() {}
public:
	CObjectBase() {}
	// do not copy refcount when copy object
	CObjectBase( const CObjectBase &a ) {} // CFundament() is called
	CObjectBase& operator=( const CObjectBase &a ) { return *this; }
	//
	bool IsValid() const { return this != 0 && (nRefData & 0x80000000) == 0; }
	// reset data in class to default values, saves RefCount from destruction
	void Clear() { AddRef(); DestroyContents(); DecRef(); }
	// for serialization purposes
	virtual void Serialize( CStructureSaver *pFile ) {}
	//
	// clear reference counter when new object is created
	void* operator new( size_t nSize ) { void *pTemp = new char[nSize]; ((CObjectBase*)pTemp)->InitRefCount(); return pTemp; }
	void* operator new[]( size_t nSize ) { ASSERT(0); return 0; }
#if defined(_DEBUG) && defined(_AFX) 
	void* operator new( size_t nSize, LPCSTR pszFile, int nLine ) { void *pTemp = new(pszFile,nLine) char[nSize]; ((CObjectBase*)pTemp)->InitRefCount(); return pTemp; }
#endif
	//
	// due to absense of template friend classes in vc this class is needed
	struct SRefO
	{
		void AddRef( CObjectBase *pObj ) { pObj->AddRefO(); }
		void DecRef( CObjectBase *pObj ) { pObj->DecRefO(); }
		void Release( CObjectBase *pObj ) { pObj->ReleaseO(); }
	};
	struct SRefM
	{
		void AddRef( CObjectBase *pObj ) { pObj->AddRefM(); }
		void DecRef( CObjectBase *pObj ) { pObj->DecRefM(); }
		void Release( CObjectBase *pObj ) { pObj->ReleaseM(); }
	};

	// due to absense of template friend classes in vc this class is needed
	struct SRef
	{
		void AddRef( CObjectBase *pObj ) { pObj->AddRef(); }
		void DecRef( CObjectBase *pObj ) { pObj->DecRef(); }
		void Release( CObjectBase *pObj ) { pObj->Release(); }
	};
	friend struct CObjectBase::SRef;
	friend struct CObjectBase::SRefO;
	friend struct CObjectBase::SRefM;
};
/////////////////////////////////////////////////////////////////////////////////////
class CFundament: public CObjectBase
{
	CFundament( const CFundament &a ) { ASSERT( 0 ); }
	CFundament& operator=( const CFundament &a ) { ASSERT( 0 ); return *this; }
	virtual void DestroyContents() { ASSERT( 0 ); }
	virtual CObjectBase* MakeCopy() const { ASSERT( 0 ); return 0; }
public:
	CFundament() {}
};
/////////////////////////////////////////////////////////////////////////////////////
// macro that helps to make CFundament derivative to be serializable
// and makes sure that class will not be copied or destroyed via obj references
/////////////////////////////////////////////////////////////////////////////////////
// macro that helps to create neccessary members for proper operation of refcount system
// if class needs special destructor, use CFundament
#define OBJECT_BASIC_METHODS(classname)                                              \
	public:                                                                            \
		static classname* New##classname() { return new classname(); }                   \
		classname* Duplicate() const { return (classname*)MakeCopy(); }                  \
	protected:                                                                         \
		virtual void DestroyContents() { classname::~classname(); ::new(this) classname;  }\
		CObjectBase* MakeCopy() const { return new classname(*this); }                   \
		virtual ~classname() {}                                                          \
	private:
/////////////////////////////////////////////////////////////////////////////////////
// TObject - base object for reference counting, TUserObj - user object name
// TRef - struct with AddRef/DecRef/Release methods for refcounting to use
template< class TObject, class TUserObj, class TRef>
class CPtrBase
{
private:
	TObject *ptr;
	//
	void AddRef( TObject *_ptr ) { TRef p; if ( _ptr ) p.AddRef( _ptr ); }
	void DecRef( TObject *_ptr ) { TRef p; if ( _ptr ) p.DecRef( _ptr ); }
	void Release( TObject *_ptr ) { TRef p; if ( _ptr ) p.Release( _ptr ); }
protected:
	TObject* Get() const { return ptr; }
	void SetObject( TObject *_ptr ) { TObject *pOld = ptr; ptr = _ptr; AddRef( ptr ); Release( pOld ); }
public:
	CPtrBase(): ptr( 0 ) {}
	CPtrBase( TObject *_ptr ): ptr( _ptr ) { AddRef( ptr ); }
	CPtrBase( const CPtrBase &a ): ptr( a.ptr ) { AddRef( ptr ); }
	//template<class TRef1>
		//CPtrBase( const CPtrBase<TObject,TUserObj,TRef1> &a ): ptr( a.ptr ) { AddRef( ptr ); }
	~CPtrBase() { Release( ptr ); }
	//
	void Set( TUserObj *_ptr ) { SetObject( (TObject*)_ptr ); }
	TUserObj* Extract() { TObject *pRes = ptr; DecRef(ptr); ptr = 0; return static_cast<TUserObj*>( pRes ); }
	//
	// assignment operators
	CPtrBase& operator=( TUserObj *_ptr ) { Set( _ptr ); return *this; }
	//template<class TRef1>
		//CPtrBase& operator=( const CPtrBase<TObject,TUserObj,TRef1> &a ) { Set( a.Get() ); return *this; }
	CPtrBase& operator=( const CPtrBase &a ) { Set( a.Get() ); return *this; }
	// access
	TUserObj* operator->() const { return (TUserObj*)( Get() ); }
	operator TUserObj*() const { return (TUserObj*)( Get() ); }
	TUserObj* GetPtr() const { return (TUserObj*)( Get() ); }
	TObject* GetBarePtr() const { return Get(); }
	bool IsValid() const { return Get()->IsValid(); }
	void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
#define BASIC_PTR_DECLARE( TPtrName, TBase, TRef )                                   \
template<class T>                                                                    \
class TPtrName: public CPtrBase< TBase, T, TRef >                                    \
{                                                                                    \
	typedef CPtrBase< TBase, T, TRef > CBase;                                          \
public:                                                                              \
	typedef T CDestType;                                                               \
	TPtrName() {}                                                                      \
	TPtrName( T *_ptr ): CBase( (TBase*)_ptr ) {}                                      \
	TPtrName( const TPtrName &a ): CBase( a ) {}                                       \
	TPtrName& operator=( T *_ptr ) { Set( _ptr ); return *this; }                      \
	TPtrName& operator=( const TPtrName &a ) { SetObject( a.Get() ); return *this; }   \
	bool operator==( const TPtrName &a ) const { return Get() == a.Get(); }            \
	bool operator==( const T *a ) const { return static_cast<T*>(Get()) == a; }        \
	bool operator!=( const TPtrName &a ) const { return Get() != a.Get(); }            \
	bool operator!=( const T *a ) const { return static_cast<T*>(Get()) != a; }        \
	bool operator< ( const TPtrName &a ) const { return Get() < a.Get(); }             \
	bool operator> ( const TPtrName &a ) const { return Get() > a.Get(); }             \
	bool operator<=( const TPtrName &a ) const { return Get() <= a.Get(); }            \
	bool operator>=( const TPtrName &a ) const { return Get() >= a.Get(); }            \
};
#ifdef STUPID_VISUAL_ASSIST
template<class T> class CPtr {};
template<class T> class CObj {};
template<class T> class CMObj {};
#endif
//
BASIC_PTR_DECLARE( CPtr, CObjectBase, CObjectBase::SRef );
BASIC_PTR_DECLARE( CObj, CObjectBase, CObjectBase::SRefO );
BASIC_PTR_DECLARE( CMObj, CObjectBase, CObjectBase::SRefM );
/*
/////////////////////////////////////////////////////////////////////////////////////
// для использования только в MakeObj, нужен для реализации операций с STL структурами
// данных типа list<CObj<T>>, например, push_back( new T ) без копирования T, с 
// использованием этого метода запись будет такой: push_back( MakeObj( new T ) )
template < class T >
struct CObjFaker: public CObj<T>
{
	CObjFaker( T *_pData ) { ASSERT(sizeof(*this)==4); *((void**)(this)) = _pData; }
	~CObjFaker() { *((void**)(this)) = 0; }
};
/////////////////////////////////////////////////////////////////////////////////////
template < class T >
inline CObjFaker<T> MakeObj( T *o ) { return CObjFaker<T>( o ); }
/////////////////////////////////////////////////////////////////////////////////////
// для использования только в MakePtr, нужен для реализации операций с STL структурами
// данных типа vector<CPtr<T>>, например, push_back( pItem ) без излишних AddRef/Release,
// с использованием этого метода запись будет такой: push_back( MakePtr( pItem ) )
template < class T >
struct CPtrFaker: public CPtr<T>
{
	CPtrFaker( T *_pData ) { ASSERT(sizeof(*this)==4); *((void**)(this)) = _pData; }
	~CPtrFaker() { *((void**)(this)) = 0; }
};
/////////////////////////////////////////////////////////////////////////////////////
template < class T >
inline CPtrFaker<T> MakePtr( T *o ) { return CPtrFaker<T>( o ); }
*/
/////////////////////////////////////////////////////////////////////////////////////
// functor for STL tests
struct SPtrTest
{
	CObjectBase *pTest;
	SPtrTest( CObjectBase *_pTest ): pTest(_pTest) {}
	template <class T,class T1, class T2> 
		bool operator()( const CPtrBase<T,T1,T2> &a ) const { return a == pTest; }
};
/////////////////////////////////////////////////////////////////////////////////////
struct SPtrHash
{
	template <class T,class T1, class T2> 
		int operator()( const CPtrBase<T,T1,T2> &a ) const { return (int)a.GetBarePtr(); }
};
/////////////////////////////////////////////////////////////////////////////////////
// walks container of pointers and erases references on invalid entries
template<class TContainer>
inline void EraseInvalidRefs( TContainer *pData )
{
	for ( TContainer::iterator i = pData->begin(); i != pData->end(); )
	{
		if ( (*i)->IsValid() )
			++i;
		else
			i = pData->erase( i );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// class for convinient handling of framework`s contexts
class CFWContext
{
	void **pFrameworkPtr;
public:
	template<class T>
		CFWContext( T **pF, T *pData ): pFrameworkPtr((void**)pF) { *pF = pData; }
	~CFWContext() { *pFrameworkPtr = 0; }
};
/////////////////////////////////////////////////////////////////////////////////////
// assumes single inheritance and base class is CObjectBase
template<class T>
class CDynamicCast
{
	T *ptr;
public:
	template<class TT>
		CDynamicCast( TT *_ptr ) { ptr = dynamic_cast<T*>((CObjectBase*)_ptr); }
	template<class T1, class T2, class T3>
		CDynamicCast( const CPtrBase<T1,T2,T3> &_ptr ) { ptr = dynamic_cast<T*>(_ptr.GetBarePtr()); }
	operator T*() const { return ptr; }
	T* operator->() const { return ptr; }
};
/////////////////////////////////////////////////////////////////////////////////////
#endif