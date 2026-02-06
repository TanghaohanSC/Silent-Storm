#include "StdAfx.h"
#include "Streams.h"
#include "NetAcks.h"
#include "NetStream.h"
#include "NetServerInfo.h"
#include "NetLogin.h"
#include "NetPeer2Peer.h"
#include "HPTimer.h"
#include "NetA4.h"

#ifdef _USE_GAMESPY
extern "C"
{
#include "peerA4Ext.h"
}
#endif
//#define LOG
#ifdef LOG
#include <iostream>
#endif

namespace NNet
{
const F_TIMEOUT = 60;
/////////////////////////////////////////////////////////////////////////////////////
class CA4NetDriver: public IA4NetDriver
{
public:
	CA4NetDriver( int _nGamePort, bool bClientOnly );
	~CA4NetDriver();
	virtual EState GetState() const { return state; }
	virtual void Step();
	virtual void ConnectGame( const CNodeAddress &addr, CMemoryStream &pwd );
	virtual EReject GetRejectReason() const { return lastReject; }

	virtual void SendBroadcast( CMemoryStream &pkt );
	virtual void SendDirect( int nClient, CMemoryStream &pkt );
	virtual void Kick( int nClient );
	virtual bool GetMessage( SMessage *pRes );

	virtual void StartGame();
	virtual void StartGameInfoSend( CMemoryStream &data );
	virtual void StopGameInfoSend();
	virtual bool GetGameInfo( int nIdx, SServer *pData );
	virtual void StartNewPlayerAccept();
	virtual void StopNewPlayerAccept();
	virtual SOCKET GetSocket();
	virtual sockaddr *GetSockAddr();
private:
	struct SClientAddressInfo
	{
		CNodeAddress inetAddress;
		CNodeAddressSet localAddress;
		
		SClientAddressInfo() {}
		SClientAddressInfo( const CNodeAddress &_inetAddress, const CNodeAddressSet &_localAddress ) 
			: inetAddress(_inetAddress), localAddress(_localAddress) {}
	};
	class CPeer
	{
	public:
		CP2PTracker::UCID clientID;
		CNodeAddress currentAddr;
		SClientAddressInfo addrInfo;
		CAckTracker acks;
		CStreamTracker data;
		bool bTryShortcut;
	};
	std::list<CPeer> clients;
	EState state;
	EReject lastReject;
	NHPTimer::STime lastTime;
	CServerInfoSupport serverInfo;
	CLoginSupport login;
	CP2PTracker p2p;
	std::list<SMessage> msgQueue;
	bool bAcceptNewClients;
	CLinksManager links;
	int nGamePort;
	CNodeAddress addr;

	CPeer* GetClientByAddr( const CNodeAddress &addr );
	CPeer* GetClient( CP2PTracker::UCID nID );
	void AddClient( const SClientAddressInfo &addr, CP2PTracker::UCID clientID );
	void AddNewP2PClient( const SClientAddressInfo &addr, CP2PTracker::UCID clientID );
	void RemoveClient( CP2PTracker::UCID nID );
	void StepInactive();
	void StepConnecting();
	void StepActive( float fDeltaTime );
	void ProcessIncomingMessages();
	void ProcessLogin( const CNodeAddress &addr, CBitStream &bits );
	void ProcessNormal( const CNodeAddress &addr, CBitStream &bits );
	void AddOutputMessage( EMessage msg, const CP2PTracker::UCID &_from, 
		CMemoryStream &data, const std::vector<CP2PTracker::UCID> &received );
	void PollMessages( CPeer *pPeer );
};
// interaction with master server is accomplished with different object
// if so then it should be possible to start
/////////////////////////////////////////////////////////////////////////////////////
enum EPacket
{
	NORMAL,
	LOGIN,
	REQUEST_SERVER_INFO,
	SERVER_INFO,
	ACCEPTED,
	REJECTED,
	LOGOUT,
	TRY_SHORTCUT,
	NOP,
	GAMESPY = '\\'
};
/////////////////////////////////////////////////////////////////////////////////////
const int N_APPLICATIONID = 0x45143100;
class CSendPacket
{
	const CLinksManager &links;
	const CNodeAddress &addr;
	static CMemoryStream pkt;
	static CBitLocker bits;
	static bool bLastPacket;
public:
	CSendPacket( const CNodeAddress &_addr, EPacket packet, const CLinksManager &_links ): addr(_addr), links(_links)
	{
		pkt.Seek(0);
		bits.LockWrite( pkt, N_MAX_PACKET_SIZE + 1024 );
		bits.Write( &packet, 1 );
	}
	CSendPacket( const CNodeAddress &_addr, EPacket packet, CP2PTracker::UCID clientID, const CLinksManager &_links ): addr(_addr), links(_links)
	{
		pkt.Seek(0);
		bits.LockWrite( pkt, N_MAX_PACKET_SIZE + 1024 );
		bits.Write( &packet, 1 );
		bits.Write( &clientID, sizeof(clientID) );
	}
	~CSendPacket()
	{
		bits.Free();
		bLastPacket = links.Send( addr, pkt );
	}
	CBitStream* GetBits() { return &bits; }
	static bool GetResult() { return bLastPacket; }
};
CMemoryStream CSendPacket::pkt;
CBitLocker CSendPacket::bits;
bool CSendPacket::bLastPacket;
/////////////////////////////////////////////////////////////////////////////////////
// packet to/from stream
/////////////////////////////////////////////////////////////////////////////////////
static bool CanReadPacket( CRingBuffer<N_STREAM_BUFFER> &buf )
{
	if ( buf.GetSize() < 4 )
		return false;
	int nSize;
	buf.Peek( &nSize, 4 );
	if ( nSize & 1 )
		nSize &= 0xff;
	nSize >>= 1;
//	if ( nSize > 7100 ) cout << "SIZE PIZDOS " << nSize << endl;
	if ( buf.GetSize() >= nSize + (nSize >= 128 ? 4 : 1) )
		return true;
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////
static void WritePacket( std::list<CMemoryStream> *pDst, CMemoryStream &pkt )
{
	ASSERT( pkt.GetSize() < N_STREAM_BUFFER - 1000 );
	pDst->push_back();
	CMemoryStream &b = pDst->back();
	int nSize = pkt.GetSize();
	if ( nSize >= 128 )
	{
		nSize <<= 1;
		b.Write( &nSize, 4 );
	}
	else
	{
		nSize <<= 1;
		nSize |= 1;
		b.Write( &nSize, 1 );
	}
	b.Write( pkt.GetBuffer(), pkt.GetSize() );
}
static void ReadPacket( CRingBuffer<N_STREAM_BUFFER> &src, CMemoryStream *pDst )
{
	ASSERT( CanReadPacket( src ) );
	int nSize = 0;
	src.Read( &nSize, 1 );
	if ( (nSize & 1) == 0 )
		src.Read( ((char*)&nSize) + 1, 3 );
	nSize >>= 1;
//	if ( nSize > 7100 )	cout << "SIZE PIZDOS " << nSize << endl;
	pDst->SetSizeDiscard( nSize );
	src.Read( pDst->GetBufferForWrite(), nSize );
}
/////////////////////////////////////////////////////////////////////////////////////
// CA4NetDriver
/////////////////////////////////////////////////////////////////////////////////////
CA4NetDriver::CA4NetDriver( int _nGamePort, bool bClientOnly ): serverInfo(N_APPLICATIONID), login(N_APPLICATIONID),
	nGamePort(_nGamePort)
{
	state = INACTIVE;
	lastReject = NONE;
	bAcceptNewClients = true;
	NHPTimer::GetTime( &lastTime );
	bool bLinksOk;
	if ( bClientOnly )
		bLinksOk = links.Start( 0 );
	else
		bLinksOk = links.Start( nGamePort );
	ASSERT( bLinksOk );
}
/////////////////////////////////////////////////////////////////////////////////////
CA4NetDriver::~CA4NetDriver()
{
	switch ( state )
	{
		case ACTIVE:
			{
				for ( std::list<CPeer>::iterator i = clients.begin(); i != clients.end(); ++i )
				{
					CSendPacket p( i->currentAddr, LOGOUT, i->clientID, links );
				}
				break;
			}
		case CONNECTING:
			{
				CSendPacket p( login.GetLoginTarget(), LOGOUT, -1, links );
			}
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
CA4NetDriver::CPeer* CA4NetDriver::GetClientByAddr( const CNodeAddress &addr )
{
	for ( std::list<CPeer>::iterator i = clients.begin(); i != clients.end(); ++i )
	{
		if ( i->currentAddr == addr )
			return &(*i);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
CA4NetDriver::CPeer* CA4NetDriver::GetClient( CP2PTracker::UCID nID )
{
	for ( std::list<CPeer>::iterator i = clients.begin(); i != clients.end(); ++i )
	{
		if ( i->clientID == nID )
			return &(*i);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::AddClient( const SClientAddressInfo &addr, CP2PTracker::UCID clientID )
{
	clients.push_back();
	CPeer &peer = clients.back();
	peer.currentAddr = addr.inetAddress;
	peer.clientID = clientID;
	peer.addrInfo = addr;
	CNodeAddress test;
	addr.localAddress.GetAddress( 0, &test );
	peer.bTryShortcut = !addr.inetAddress.SameIP( test );
	peer.bTryShortcut |= !addr.localAddress.GetAddress( 1, &test );
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::AddNewP2PClient( const SClientAddressInfo &addr, CP2PTracker::UCID clientID )
{
	CMemoryStream addrInfo;
	addrInfo.Write( &addr, sizeof( addr ) );
	p2p.AddNewClient( clientID, addrInfo );
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::RemoveClient( CP2PTracker::UCID nID )
{
	for ( std::list<CPeer>::iterator i = clients.begin(); i != clients.end(); )
	{
		if ( i->clientID == nID )
			i = clients.erase( i );
		else
			++i;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::SendBroadcast( CMemoryStream &pkt )
{
	ASSERT( state == ACTIVE );
	p2p.SendBroadcast( pkt );
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::SendDirect( int nClient, CMemoryStream &pkt )
{
	CPeer *pDst = GetClient( nClient );
	ASSERT( pDst );
	ASSERT( state == ACTIVE );
	if ( pDst )
		p2p.SendDirect( pDst->clientID, pkt );
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::Kick( int nClient )
{
	CPeer *pDst = GetClient( nClient );
	ASSERT( pDst );
	ASSERT( state == ACTIVE );
	if ( pDst )
		p2p.KickClient( nClient );
}
/////////////////////////////////////////////////////////////////////////////////////
bool CA4NetDriver::GetMessage( SMessage *pRes )
{
	if ( msgQueue.empty() )
		return false;
	*pRes = msgQueue.front();
	msgQueue.pop_front();
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::ProcessLogin( const CNodeAddress &addr, CBitStream &bits )
{
	// if can accept login requests only
	CLoginSupport::SLoginInfo info;
	if ( login.ProcessLogin( addr, bits, &info ) )
	{
		bool bAddp2pClient = false;
		EReject reject = NONE;
		if ( info.bWrongVersion )
			reject = WRONG_VERSION;
		else
		{
			CPeer *pPeer = GetClientByAddr( addr );
			if ( pPeer )
			{
				if ( !login.HasAccepted( addr, info ) )
					return; // ignore obsolete or too new request
			}
			else if ( info.pwd.GetSize() ) // wrong password
				reject = PASSWORD_FAILED;
			else
			{
				if ( !bAcceptNewClients )
					reject = FORBIDDEN;
				else
					bAddp2pClient = true;
			}
		}
		if ( reject != NONE )
		{
			CSendPacket pkt( addr, REJECTED, links );
			login.RejectLogin( pkt.GetBits(), info, (int)reject );
		}
		else
		{
			int nClientID;
			CSendPacket pkt( addr, ACCEPTED, links );
			CNodeAddressSet localAddr;
			bool bGetSelf = links.GetSelfAddress( &localAddr );
			ASSERT( bGetSelf );
			login.AcceptLogin( addr, pkt.GetBits(), info, &nClientID, localAddr );
			if ( bAddp2pClient )
				AddNewP2PClient( SClientAddressInfo( addr, info.localAddr ), nClientID );
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::ProcessNormal( const CNodeAddress &addr, CBitStream &bits )
{
	CP2PTracker::UCID clientID = -1;
	bits.Read( &clientID, sizeof(clientID) );
	if ( !p2p.IsActive( clientID ) )
		return;
	CPeer *pPeer = GetClient( clientID );
	if ( pPeer )
	{
		pPeer->currentAddr = addr;
		pPeer->bTryShortcut = false;
		std::vector<PACKET_ID> acked;
		ASSERT( pPeer->data.CanReadMsg() ); // data polling is not perfect
		if ( pPeer->data.CanReadMsg() && pPeer->acks.ReadAcks( &acked, bits ) )
		{
			pPeer->data.ReadMsg( bits );
			pPeer->data.Commit( acked );
			PollMessages( pPeer );			
		}
	}
	else
	{
		//if (pCSLog) (*pCSLog) << "normal packet from non client received from " << addr.GetFastName() << endl;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::ProcessIncomingMessages()
{
	// process incoming packets
//	CNodeAddress addr;
	static CMemoryStream pkt;
	while ( links.Recv( &addr, &pkt ) )
	{
		if ( pkt.GetSize() == 0 )
		{
			//if (pCSLog) (*pCSLog) << "ZERO length packet received from " << addr.GetFastName() << endl;
			continue;
		}
		EPacket cmd = (EPacket)0;
		pkt.Read( &cmd, 1 );
		CBitStream bits( pkt.GetBufferForWrite() + 1, CBitStream::read, pkt.GetSize() - 1 );
		switch ( cmd )
		{
			case NORMAL:
				ProcessNormal( addr, bits );
				break;
			case LOGIN:
				ProcessLogin( addr, bits );
				break;
			case REQUEST_SERVER_INFO:
				if ( serverInfo.DoReplyRequest() )
				{
					CSendPacket pkt( addr, SERVER_INFO, links );
					serverInfo.ReplyServerInfoRequest( bits, pkt.GetBits() );
				}
				break;
			case SERVER_INFO:
				serverInfo.ProcessServerInfo( addr, bits );
				break;
			case ACCEPTED:
				login.ProcessAccepted( addr, bits );
				break;
			case REJECTED:
				login.ProcessRejected( addr, bits );
				break;
			case LOGOUT:
				{
					CP2PTracker::UCID clientID = -1;
					bits.Read( &clientID, sizeof( clientID ) );
					p2p.KickClient( clientID );
					CPeer *pPeer = GetClientByAddr( addr );
					if ( pPeer )
						p2p.KickClient( pPeer->clientID );
				}
				break;
			case TRY_SHORTCUT:
				{
					CP2PTracker::UCID clientID = -1;
					bits.Read( &clientID, sizeof(clientID) );
					CLoginSupport::TServerID uniqueServerID;
					bits.Read( uniqueServerID );
					if ( uniqueServerID == login.GetUniqueServerID() )
					{
						if ( !p2p.IsActive( clientID ) )
							return;
						CPeer *pPeer = GetClient( clientID );
						if ( pPeer )
						{
							pPeer->currentAddr = addr;
							pPeer->bTryShortcut = false;
						}
					}
				}
				break;
			case NOP:
				break;
#ifdef _USE_GAMESPY
			case GAMESPY:
			{
				g_szGameSpyPeerMessage[0] = GAMESPY;
				pkt.Read( &g_szGameSpyPeerMessage[1], pkt.GetSize() - 1 );
				g_szGameSpyPeerMessage[pkt.GetSize()] = 0;
				break;
			}
#endif
			default:
				ASSERT( 0 && "Unknown command" );
				break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::StepInactive()
{
	std::vector<CNodeAddress> dest;
	CNodeAddress broadcast;
	links.MakeBroadcastAddr( &broadcast, nGamePort );
	if ( serverInfo.CanSendRequest( broadcast, &dest ) )
	{
		for ( int i = 0; i < dest.size(); ++i )
		{
			CSendPacket pkt( dest[i], REQUEST_SERVER_INFO, links );
			serverInfo.WriteRequest( pkt.GetBits() );
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::StepConnecting()
{
	switch ( login.GetState() )
	{
		case CLoginSupport::INACTIVE:
			lastReject = TIMEOUT;
			state = INACTIVE;
			break;
		case CLoginSupport::LOGIN:
			if ( login.CanSend() )
			{
				CNodeAddressSet localAddr;
				if ( !links.GetSelfAddress( &localAddr ) )
				{
					CSendPacket pkt( login.GetLoginTarget(), NOP, links );
				}
				bool bGetSelf = links.GetSelfAddress( &localAddr );
				ASSERT( bGetSelf );
				CSendPacket pkt( login.GetLoginTarget(), LOGIN, links );
				login.WriteLogin( pkt.GetBits(), localAddr );
			}
			break;
		case CLoginSupport::ACCEPTED:
			AddNewP2PClient( SClientAddressInfo( login.GetLoginTarget(), login.GetTargetLocalAddr() ), 0 );
			state = ACTIVE;
			break;
		case CLoginSupport::REJECTED:
			lastReject = (EReject)login.GetRejectReason();
			state = INACTIVE;
			break;
		default:
			ASSERT( 0 );
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::AddOutputMessage( EMessage msg, const CP2PTracker::UCID &_from, 
		CMemoryStream &data, const std::vector<CP2PTracker::UCID> &received )
{
	CPeer *pPeer = GetClient( _from );
	ASSERT( pPeer );
	if ( !pPeer )
		return;
	msgQueue.push_back();
	SMessage &res = msgQueue.back();
	res.msg = msg;
	res.pkt = data;
	res.nClientID = pPeer->clientID;
	for ( int i = 0; i < received.size(); ++i )
	{
		CPeer *pTest = GetClient( received[i] );
		ASSERT( pTest );
		if ( pTest )
			res.received.push_back( pTest->clientID );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::PollMessages( CPeer *pPeer )
{
	// seek for packets through incoming traffic
	while ( CanReadPacket( pPeer->data.channelInBuf ) )
	{
		CMemoryStream pkt;
		ReadPacket( pPeer->data.channelInBuf, &pkt );
#ifdef LOG
		cout << "receive packet from " << pPeer->addr.GetFastName() << " size=" << pkt.GetSize() << endl;
#endif
		p2p.ProcessPacket( pPeer->clientID, pkt );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::StepActive( float fDeltaTime )
{
	// rollback outdated packets
	for ( std::list<CPeer>::iterator i = clients.begin(); i != clients.end(); ++i )
	{
		std::vector<PACKET_ID> rolled, erased;
		i->acks.Step( &rolled, &erased, fDeltaTime );
		i->data.Rollback( rolled );
		i->data.Erase( erased );
		PollMessages( &(*i) );
	}
	// process peer2peer messages
	CP2PTracker::SMessage msg;
	while ( p2p.GetMessage( &msg ) )
	{
		switch ( msg.msg )
		{
			case CP2PTracker::NEW_CIENT:
				{
					SClientAddressInfo addr;
					msg.pkt.Seek(0);
					msg.pkt.Read( &addr, sizeof(addr) );
					AddClient( addr, msg.from );
					AddOutputMessage( NEW_CLIENT, msg.from, msg.pkt, msg.received );
				}
				break;
			case CP2PTracker::REMOVE_CLIENT:
				AddOutputMessage( REMOVE_CLIENT, msg.from, msg.pkt, msg.received );
				RemoveClient( msg.from );
				break;
			case CP2PTracker::DIRECT:
				AddOutputMessage( DIRECT, msg.from, msg.pkt, msg.received );
				break;
			case CP2PTracker::BROADCAST:
				AddOutputMessage( BROADCAST, msg.from, msg.pkt, msg.received );
				break;
		}
	}
	// form output packets
	for ( int pi = 0; pi < p2p.packets.size(); ++pi )
	{
		CP2PTracker::SPacket &p = p2p.packets[pi];
		CPeer *pPeer = GetClient( p.addr );
		if ( pPeer )
		{
			WritePacket( &pPeer->data.outList, p.pkt );
#ifdef LOG
			cout << "output packet to " << p.addr.GetFastName() << " size=" << p.pkt.GetSize() << endl;
#endif
		}
		else
		{
#ifdef LOG
			cout << "DISCARD packet to " << p.addr.GetFastName() << " size=" << p.pkt.GetSize() << endl;
#endif
		}
	}
	p2p.packets.resize( 0 );
	// send updates & check for timeouts
	for ( std::list<CPeer>::iterator it = clients.begin(); it != clients.end(); ++it )
	{
		if ( it->acks.GetTimeSinceLastRecv() > F_TIMEOUT )
			p2p.KickClient( it->clientID );
		if ( p2p.IsActive( it->clientID ) )
		{
/*
			if ( !it->data.HasOutData() )
			{
				// самый свободный? получи
				CMemoryStream shit;
				static int nShit = 0;
				shit.SetSize( 7000 );//15000 );
				sprintf( (char*)shit.GetBufferForWrite(), "info %d", nShit++ );
				p2p.SendDirect( it->addr, shit );
#ifdef LOG
				cout << "add direct msg for " << it->addr.GetFastName() << " msg " << nShit << endl;
#endif
			}*/
			while ( ( it->acks.CanSend() && it->data.HasOutData() ) || it->acks.NeedSend() )
			{
				if ( it->bTryShortcut )
				{
					for ( int k = 0; 1; ++k )
					{
						CNodeAddress dest;
						if ( !it->addrInfo.localAddress.GetAddress( k, &dest ) )
							break;
						CSendPacket pkt( dest, TRY_SHORTCUT, login.GetSelfClientID(), links );
						pkt.GetBits()->Write( login.GetUniqueServerID() );
					}
				}
				PACKET_ID pktID;
				{
					CSendPacket pkt( it->currentAddr, NORMAL, login.GetSelfClientID(), links );
					pktID = it->acks.WrtieAcks( pkt.GetBits(), 220 ); // CRAP - packet size limits??
					it->data.WriteMsg( pktID, pkt.GetBits(), 220 );
				}
				if ( !CSendPacket::GetResult() )
				{
					it->acks.PacketLost( pktID );
					std::vector<PACKET_ID> roll;
					roll.push_back( pktID );
					it->data.Rollback( roll );
					break;
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::Step()
{
	float fSeconds = NHPTimer::GetTimePassed( &lastTime );
	serverInfo.Step( fSeconds );
	login.Step( fSeconds );
	ProcessIncomingMessages();
	//
	switch ( state )
	{
		case INACTIVE:
			StepInactive();
			break;
		case ACTIVE:
			StepActive( fSeconds );
			break;
		case CONNECTING:
			StepConnecting();
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::StartGame()
{
	ASSERT( state == INACTIVE );
	state = ACTIVE;
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::ConnectGame( const CNodeAddress &addr, CMemoryStream &pwd )
{
	state = CONNECTING;
	login.StartLogin( addr, pwd );
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::StartGameInfoSend( CMemoryStream &data )
{
	serverInfo.StartReply( data );
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::StopGameInfoSend()
{
	serverInfo.StopReply();
}
/////////////////////////////////////////////////////////////////////////////////////
bool CA4NetDriver::GetGameInfo( int nIdx, SServer *pData )
{
	const CServerInfoSupport::CServerInfoList &servers = serverInfo.GetServers();
	CServerInfoSupport::CServerInfoList::const_iterator k = servers.begin();
	for ( ; k != servers.end(); ++k, --nIdx )
	{
		if ( nIdx == 0 )
		{
			pData->addr = k->addr;
			pData->bWrongVersion = k->bWrongVersion;
			pData->fPing = k->fPing;
			pData->info = k->info;
			return true;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::StartNewPlayerAccept() 
{
	bAcceptNewClients = true;
}
/////////////////////////////////////////////////////////////////////////////////////
void CA4NetDriver::StopNewPlayerAccept()
{
	bAcceptNewClients = false;
}
/////////////////////////////////////////////////////////////////////////////////////
SOCKET CA4NetDriver::GetSocket()
{
	return links.GetSocket();
}
/////////////////////////////////////////////////////////////////////////////////////
sockaddr *CA4NetDriver::GetSockAddr()
{
	return addr.GetSockAddr();
//	return links.GetSockAddr();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
IA4NetDriver* MakeDriver( int nGamePort, bool bClientOnly )
{
	CA4NetDriver *pRes = new CA4NetDriver( nGamePort, bClientOnly );
	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////
}