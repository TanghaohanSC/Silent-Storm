#include "stdafx.h"
#include "NetA4.h"
#include <conio.h>
#include <iostream>
#include <winsock2.h>
//#include "HPTimer.h"
//using namespace std;
//using namespace std;
////////////////////////////////////////////////////////////////////////////////////////////////////
// for testing purposes both server and client has same functionality
////////////////////////////////////////////////////////////////////////////////////////////////////
/*class CTestServerInfo: public CCSServerInfo
{
	OBJECT_BASIC_METHODS( CTestServerInfo );
public:
	std::string szName;

	virtual void UpdateTo( CBitStream &bits ) { bits.WriteCString( szName.c_str() ); }
	virtual void UpdateFrom( CBitStream &bits ) { bits.ReadCString( szName ); }
};*/
namespace NNet
{
	extern bool bEmulateWeakNetwork;
	extern float fLostRate;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RunBase( NNet::IA4NetDriver *pNet )
{
	//NNet::bEmulateWeakNetwork = true;
	//NNet::fLostRate = 0.5f;
	char xz[10000];
	gethostname( xz,9999 );
	hostent *p = gethostbyname( xz );
	for ( int k = 0; k < 100; ++k )
	{
		if ( !p->h_addr_list[k] )
			break;
		unsigned char *addr = (unsigned char*)p->h_addr_list[k];
		char szBuf[100];
		sprintf( szBuf, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3] );
		NNet::CNodeAddress node;
		node.SetInetName( szBuf, 8888 );
		cout << node.GetFastName().c_str() << endl;
	}
	//addrinfo blah, res;
	//Zero( blah );
	//blah.ai_flags = AI_NUMERICHOST;
	//getaddrinfo( xz, "", &blah, &res );
	for (;;)
	{
		if ( kbhit() )
		{
			int c = getch();
			if ( c == 27 )
				break;
			if ( c == 8 )
			{
				pNet->StopGameInfoSend();
				pNet->StopNewPlayerAccept();
				continue;
			}
			cout << "Sending " << (char) c << endl;
			CMemoryStream pkt;
			pkt << c;
			pNet->SendBroadcast( pkt );
		}
		//Sleep( 50 );
		pNet->Step();
		NNet::IA4NetDriver::SMessage msg;
		while ( pNet->GetMessage( &msg ) )
		{
			if ( msg.msg == NNet::IA4NetDriver::DIRECT )
			{
				cout << "direct msg from " << msg.nClientID << " size=" << msg.pkt.GetSize() << " ";
				cout << msg.pkt.GetBuffer();
				cout << endl;
			}
			if ( msg.msg == NNet::IA4NetDriver::BROADCAST )
			{
				char c;
				msg.pkt.Seek( 0 );
				msg.pkt.Read( &c, 1 );
				cout << "received " << c << "; recved by: ";
				for ( int k = 0; k < msg.received.size(); ++k )
					cout << msg.received[k] << " ";
				cout << endl;
			}
			if ( msg.msg == NNet::IA4NetDriver::NEW_CLIENT )
			{
				cout << "new client " << msg.nClientID << endl;
			}
			if ( msg.msg == NNet::IA4NetDriver::REMOVE_CLIENT )
			{
				cout << "remove client " << msg.nClientID << endl;
			}
		}
	}
/*	double fpTime = NHPTimer::GetTimeSecond();
	CLetterList::iterator i;
	for(;;)
	{
		if ( kbhit() )
		{
			int c = getch();
			if ( c == 27 )
				break;
			if ( c == 8 )
			{
				if ( !base.letters.empty() )
					base.letters.pop_back();
				continue;
			}
			if ( c == 224 )
			{
				i = PickRandom( base.letters );
				if ( i != base.letters.end() )
					base.letters.erase( i );
				continue;
			}
			if ( c == ' ' )
			{
				i = PickRandom( base.letters );
				if ( i != base.letters.end() )
				{
					CLetter *pL = *i;
					pL->cLetter++;
					pL->SetUpdated();
				}
				continue;
			}
			CLetter *pTest = new CLetter;
			pTest->cLetter = c;
			base.letters.push_back( pTest );
		}
		double fpOld = fpTime;
		fpTime = NHPTimer::GetTimeSecond();
		base.TimePassed( fpTime - fpOld );
	}*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// TEST structure
void RunServer()
{
	cout << "Activating server" << endl;
	NNet::IA4NetDriver *pNet = NNet::MakeDriver( 8889 );
	delete pNet;
	pNet = NNet::MakeDriver( 8889 );
	pNet->StartGame();
	CMemoryStream gameInfo;
	pNet->StartGameInfoSend( gameInfo );
	RunBase( pNet );
	delete pNet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void RunClient()
{
	NNet::IA4NetDriver *pNet = NNet::MakeDriver( 8889, true );

	NNet::CNodeAddress test, connectTo;
	//connectTo.SetInetName( "localhost:8889", 0 ); //book.push_back( test );
	//test.SetInetName( "192.168.0.117:8888" ); book.push_back( test );
	cout << "enter the address: ";

	std::string szAddress;
	cin >> szAddress; cout << endl;
	connectTo.SetInetName( (szAddress + ":8889").c_str(), 0 );

/*	cout << "Start server search" << endl;
	for (;;)
	{
		Sleep(100);
		pNet->Step();
		for ( int i = 0; 1; i++ )
		{
			NNet::IA4NetDriver::SServer info;
			if ( !pNet->GetGameInfo( i, &info ) )
				break;
			cout << " " << i << ") " << info.addr.GetFastName() << endl;
		}
		if ( kbhit() )
		{
			int nChoice;
			int n = getch();
			if ( n == 27 )
				return;
			nChoice = n - '0';
			NNet::IA4NetDriver::SServer info;
			if ( pNet->GetGameInfo( nChoice, &info ) )
			{
				connectTo = info.addr;
				break;
			}
		}
	}*/
	cout << "Connecting to server" << endl;

	CMemoryStream pwd;
	pNet->ConnectGame( connectTo, pwd );
	for(;;)
	{
		if ( kbhit() && getch() == 27 )
			return;
		pNet->Step();
		if ( pNet->GetState() == NNet::IA4NetDriver::INACTIVE )
		{
			cout << "Connection failed, exit prog" << endl;
			return;
		}
		if ( pNet->GetState() == NNet::IA4NetDriver::ACTIVE )
			break;
		Sleep(10);
	}
	cout << "Activating client" << endl;
	RunBase( pNet );
	delete pNet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//using namespace std;
int __cdecl zhopa( size_t n )
{
	cout << "MEMORY PIZDOS" << n << endl;
	// Your code
	return 0;
}

#include "new.h"
int __cdecl  main(int argc, char* argv[])
{
	_set_new_handler( zhopa );
//	abort();
	//	pCSLog = &cout;
	cout << "server = 0, client = 1" << endl;
	if ( getch() == '0' )
		RunServer();
	else
		RunClient();
	return 0;
}
