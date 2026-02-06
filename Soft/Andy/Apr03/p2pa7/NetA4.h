#ifndef __NETA4_H_
#define __NETA4_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetLowest.h"
#include "Streams.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NNet
{
/////////////////////////////////////////////////////////////////////////////////////
class IA4NetDriver
{
public:
	enum EState
	{
		INACTIVE,
		ACTIVE,
		CONNECTING
	};
	enum EReject
	{
		NONE,
		TIMEOUT,
		BANNED,
		WRONG_VERSION,
		MAXPLAYERS_REACHED,
		PASSWORD_FAILED,
		ALREADY_CONNECTED,
		FORBIDDEN
	};
	enum EMessage
	{
		NEW_CLIENT,
		REMOVE_CLIENT,
		DIRECT,
		BROADCAST
	};
	struct SMessage
	{
		EMessage msg;
		int nClientID;
		std::vector<int> received;
		CMemoryStream pkt;
	};
	struct SServer
	{
		CNodeAddress addr;
		bool bWrongVersion;
		float fPing;
		CMemoryStream info;
	};
	
	virtual ~IA4NetDriver() {}
	virtual EState GetState() const = 0;
	virtual void Step() = 0;
	//
	virtual void ConnectGame( const CNodeAddress &addr, CMemoryStream &pwd ) = 0;
	virtual EReject GetRejectReason() const = 0;
	//
	virtual void StartGame() = 0;
	virtual void StartGameInfoSend( CMemoryStream &data ) = 0;
	virtual void StopGameInfoSend() = 0;
	virtual bool GetGameInfo( int nIdx, SServer *pData ) = 0;
	virtual void StartNewPlayerAccept() = 0;
	virtual void StopNewPlayerAccept() = 0;
	//
	virtual void SendBroadcast( CMemoryStream &pkt ) = 0;
	virtual void SendDirect( int nClient, CMemoryStream &pkt ) = 0;
	virtual void Kick( int nClient ) = 0;
	virtual bool GetMessage( SMessage *pRes ) = 0;
	virtual SOCKET GetSocket() = 0;
	virtual sockaddr *GetSockAddr() = 0;
};
/////////////////////////////////////////////////////////////////////////////////////
IA4NetDriver* MakeDriver( int nGamePort, bool bClientOnly = false );
/////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////
#endif