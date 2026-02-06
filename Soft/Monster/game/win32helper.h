#ifndef __WIN32HELPER_H__
#define __WIN32HELPER_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
namespace NWin32Helper
{
/////////////////////////////////////////////////////////////////////////////////////
class CEvent
{
	HANDLE h;
	CEvent( const CEvent& ) {}
	CEvent& operator=( const CEvent& ) {}
public:
	CEvent( bool bInitState = false, bool bManualReset = true ) { h = CreateEvent(0, bManualReset, bInitState, 0 ); }
	~CEvent() { CloseHandle(h); }
	bool Set() { return SetEvent(h) != 0; }
	bool Pulse() { return SetEvent(h) != 0; }
	bool Reset() { return ResetEvent(h) != 0; }
	void Wait() { WaitForSingleObject(h, INFINITE ); }
	bool IsSet() { return WaitForSingleObject( h, 0 ) == WAIT_OBJECT_0; }
};
/////////////////////////////////////////////////////////////////////////////////////
class CCriticalSection
{
	CRITICAL_SECTION sect;
	CCriticalSection( const CCriticalSection & ) {}
	CCriticalSection& operator=( const CCriticalSection &) {}
	//
	void Enter() { EnterCriticalSection( &sect ); }
	void Leave() { LeaveCriticalSection( &sect ); }
public:
	CCriticalSection() { InitializeCriticalSection( &sect ); }
	~CCriticalSection() { DeleteCriticalSection( &sect ); }
	friend class CCriticalSectionLock;
};
/////////////////////////////////////////////////////////////////////////////////////
class CCriticalSectionLock
{
	CCriticalSection &lock;
public:
	CCriticalSectionLock( CCriticalSection &_lock ): lock(_lock) { lock.Enter(); }
	~CCriticalSectionLock() { lock.Leave(); }
};
/////////////////////////////////////////////////////////////////////////////////////
template<class T>
class com_ptr
{
	T *pData;
	void Assign( T *_pData ) { if ( _pData ) { _pData->AddRef(); } pData = _pData; }
	void Free() { if ( pData ) pData->Release(); }
public:
	com_ptr( T *_pData = 0 ) { Assign( _pData ); }
	~com_ptr() { Free(); }
	com_ptr( const com_ptr &a ) { Assign( a.pData ); }
	com_ptr& operator=( const com_ptr &a ) { if ( pData == a.pData ) return *this; Free(); Assign( a.pData ); return *this; }
	operator T*() const { return pData; }
	T* operator->() const { return pData; }
	// not fair play
	void Create( T *_pData ) { Free(); pData = _pData; }
	T** GetAddr() { return &pData; }
};
/////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////
#endif
