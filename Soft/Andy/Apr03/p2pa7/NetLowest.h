#ifndef __NETLOWEST_H_
#define __NETLOWEST_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock.h>
#include <string>
/////////////////////////////////////////////////////////////////////////////////////
class CMemoryStream;
/////////////////////////////////////////////////////////////////////////////////////
namespace NNet
{
/////////////////////////////////////////////////////////////////////////////////////
class CNodeAddress
{
	sockaddr addr;
public:
	CNodeAddress() { memset( &addr, 0, sizeof(addr) ); }

	void Clear() { memset( &addr, 0, sizeof(addr) ); }
	//
	bool SetInetName( const char *pszHost, int nDefaultPort );
	std::string GetName( bool bResolve = true ) const;
	std::string GetFastName() const { return GetName( false ); }
	//
	bool SameIP( const CNodeAddress &a ) const { return memcmp( ((char*)&a.addr) + 4, ((char*)&addr) + 4, 4 ) == 0; }
	unsigned int GetIP() const { return ((unsigned int*)(&addr))[1]; }

	bool operator == ( const CNodeAddress &a ) const { return memcmp( &addr, &a.addr, sizeof(addr) ) == 0; }
	bool operator != ( const CNodeAddress &a ) const { return memcmp( &addr, &a.addr, sizeof(addr) ) != 0; }

	sockaddr *GetSockAddr() { return &addr; }

	friend class CLinksManager;
};
/////////////////////////////////////////////////////////////////////////////////////
const int N_MAX_HOST_HOMES = 4;
class CNodeAddressSet
{
	unsigned short nPort;
	int ips[N_MAX_HOST_HOMES];
public:
	void Clear() { Zero(*this); }
	bool GetAddress( int n, CNodeAddress *pRes ) const;
	friend class CLinksManager;
};
/////////////////////////////////////////////////////////////////////////////////////
// abstraction from messaging level
class CLinksManager
{
	SOCKET s;
	CNodeAddress broadcastAddr;
public:
	CLinksManager();
	~CLinksManager();
	bool Start( int nPort );
	void Finish();
	bool MakeBroadcastAddr( CNodeAddress *pRes, int nPort ) const;
	bool IsLocalAddr( const CNodeAddress &test ) const;
	bool Send( const CNodeAddress &dst, CMemoryStream &pkt ) const;
	bool Recv( CNodeAddress *pSrc, CMemoryStream *pPkt ) const;
	SOCKET GetSocket() const;
	bool GetSelfAddress( CNodeAddressSet *pRes ) const;
};
/////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////
#endif
