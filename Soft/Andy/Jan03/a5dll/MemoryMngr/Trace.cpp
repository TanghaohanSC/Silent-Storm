#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>
#include "GetFuncName.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool bInternal = false;
struct SAlloc
{
	DWORD dwAddress;
	int nSize;
};
struct SPtrHash
{
	int operator()( void *p ) const { return (int)p; }
};
static std::hash_map<void*, SAlloc, SPtrHash> *pAllocs = 0;
static std::hash_map<DWORD, bool> *pIgnored = 0;
static CRITICAL_SECTION block;
static bool bHasCreatedBlock;
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool AddressFits( DWORD dwAddress )
{
	std::hash_map<DWORD, bool>::iterator i = pIgnored->find( dwAddress );
	if ( i == pIgnored->end() )
	{
		const char *pszFileName = 0;
		int nSourceLine;
		if ( !GetSourceLine( dwAddress, &pszFileName, &nSourceLine ) || pszFileName == 0 )
		{
			(*pIgnored)[dwAddress] = false;
			return false;
		}
		// analyze source file
		bool bOk = true;
		bOk &= ( strnicmp( pszFileName, "c:\\program", 10 ) != 0 );
		bOk &= ( stricmp( pszFileName, "c:\\code\\a5\\misc\\basic1.h" ) != 0 );
		return (*pIgnored)[dwAddress] = bOk;
	}
	return i->second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DebugRegister( size_t n, void *pRes )
{
	int *pBuf;
	_asm { mov pBuf, ebp }
	if ( !bHasCreatedBlock )
	{
		bHasCreatedBlock = true;
		InitializeCriticalSection( &block );
	}
	EnterCriticalSection( &block );
	if ( bInternal )
	{
		LeaveCriticalSection( &block );
		return;
	}
	bInternal = true;
	if ( !pIgnored )
	{
		LeaveCriticalSection( &block );
		pIgnored = new std::hash_map<DWORD, bool>;
		pAllocs = new std::hash_map<void*, SAlloc, SPtrHash>;
		EnterCriticalSection( &block );
	}
	DWORD dwAddress = 0;
	pBuf = (int*)pBuf[0];
	for (;;)
	{
		if ( IsBadReadPtr( pBuf, 8 ) )
			break;
		dwAddress = pBuf[1];
		if ( AddressFits( dwAddress ) )
			break;
		pBuf = (int*)pBuf[0];
	}
	SAlloc &a = (*pAllocs)[pRes];
	a.dwAddress = dwAddress;
	a.nSize = n;
	bInternal = false;
	LeaveCriticalSection( &block );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DebugFree( void *p )
{
	EnterCriticalSection( &block );
	if ( bInternal )
	{
		LeaveCriticalSection( &block );
		return;
	}
	bInternal = true;
	std::hash_map<void*, SAlloc, SPtrHash>::iterator i = pAllocs->find( p );
	if ( i != pAllocs->end() )
		pAllocs->erase( i );
	else
		assert( 0 );
	if ( pAllocs->empty() )
	{
		LeaveCriticalSection( &block );
		delete pAllocs;
		delete pIgnored;
		pAllocs = 0;
		pIgnored = 0;
		EnterCriticalSection( &block );
	}
	bInternal = false;
	LeaveCriticalSection( &block );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAllocStats
{
	int nMax, nTotal, nNumber;
	SAllocStats() { nMax = 0; nTotal = 0; nNumber = 0; }
};
void DumpMemoryStats()
{
	if ( pAllocs == 0 )
		return;
	bInternal = true;
	{
		std::hash_map<DWORD, SAllocStats> info;
		for ( std::hash_map<void*, SAlloc, SPtrHash>::iterator i = pAllocs->begin(); i != pAllocs->end(); ++i )
		{
			SAllocStats &s = info[ i->second.dwAddress ];
			s.nMax = max( s.nMax, i->second.nSize );
			s.nTotal += i->second.nSize;
			s.nNumber++;
		}
		int nTotal = 0;
		char szBuf[1024];
		for ( std::hash_map<DWORD, SAllocStats>::iterator k = info.begin(); k != info.end(); ++k )
		{
			const char *pszFileName = 0;
			int nLine;
			GetSourceLine( k->first, &pszFileName, &nLine );  // silent-storm-port: args 2,3 are out-pointers
			if ( pszFileName )
				sprintf( szBuf, "%s(%d): max block = %d, blocks = %d, total = %d\n", 
					pszFileName, nLine, k->second.nMax, k->second.nNumber, k->second.nTotal );
			else
				sprintf( szBuf, "unknown: max block = %d, blocks = %d, total = %d\n",
					k->second.nMax, k->second.nNumber, k->second.nTotal);
			OutputDebugString( szBuf );
			nTotal += k->second.nTotal;
		}
		sprintf( szBuf, "allocated %d bytes\n", nTotal );
		OutputDebugString( szBuf );
	}
	bInternal = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
