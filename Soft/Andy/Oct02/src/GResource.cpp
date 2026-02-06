#include "StdAfx.h"
#include "GResource.h"
#include "..\Misc\Win32Helper.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
static list<CResourceTracker*>& GetTrackers()
{
	static list<CResourceTracker*> trackers;
	return trackers;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static string szDir;
void AddResourceDir( const char *pszName )
{
	szDir = pszName;
	if ( !szDir.empty() && szDir[ szDir.length() - 1 ] != '\\' )
		szDir += "\\";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef hash_map<string, CPtr<CFilesPackage> > CPackHash;
static CPackHash packages;
static CFilesPackage* GetPackage( const char *pszResName )
{
	CPackHash::iterator i = packages.find( pszResName );
	CFilesPackage *pRes;
	if ( i == packages.end() )
	{
		pRes = OpenFilesPackage( ( szDir + pszResName + ".res" ).c_str() );
#ifndef _MAPEDIT
		ASSERT( pRes );
#endif
		packages[pszResName] = pRes;
		if ( !pRes )
			return 0;
	}
	else
		pRes = i->second;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CloseAllResources()
{
	packages.clear();
	list<CResourceTracker*> &t = GetTrackers();
	for ( list<CResourceTracker*>::iterator i = t.begin(); i != t.end(); ++i )
		(*i)->Clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int GetID( const SPartKey &key )
{
	return key.nID + (key.nPart << 16);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline string GetFileResourceName( const char *pszResName, FILE_ID nFileID )
{
	char szBuf[1024];
	sprintf( szBuf, "%s\\%d", pszResName, nFileID );
	return szBuf;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFileResource
////////////////////////////////////////////////////////////////////////////////////////////////////
CFileResource::CFileResource( const char *pszResName, FILE_ID nFileID )
{
	f.OpenRead( GetFileResourceName( pszResName, nFileID ).c_str() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CResourceFileOpener
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool DoesPackageFileExist( const char *pszResName, int nID )
{
	CFilesPackage *pPack = GetPackage( pszResName );
	return DoesFileExist( pPack, nID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool DoesPackageFileExist( const char *pszResName, const SPartKey &key )
{
	CFilesPackage *pPack = GetPackage( pszResName );
	return DoesFileExist( pPack, GetID( key ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CResourceFileOpener::CResourceFileOpener( const char *pszResName, int nID )
{
	if ( DoesPackageFileExist( pszResName, nID ) )
		pf = new CPackageResource( GetPackage( pszResName ), nID );
	else
		pf = new CFileResource( pszResName, nID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CResourceFileOpener::CResourceFileOpener( const char *pszResName, const SPartKey &key ) 
{
	const int nID = GetID( key );
	if ( DoesPackageFileExist( pszResName, nID ) )
		pf = new CPackageResource( GetPackage( pszResName ), nID );
	else
		pf = new CFileResource( pszResName, nID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CResourceFileOpener::DoesExist( const char *pszResName, int nID )
{
	if ( DoesPackageFileExist( pszResName, nID ) )
		return true;
#ifdef _MAPEDIT
	HANDLE h = CreateFile( GetFileResourceName( pszResName, nID ).c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
	CloseHandle( h );
	return h != INVALID_HANDLE_VALUE;
//	CFileStream file;
//	return file.TryOpenRead( GetFileResourceName( pszResName, nID ).c_str() );
#else
	return false;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CResourceFileOpener::DoesExist( const char *pszResName, const SPartKey &key )
{
	if ( DoesPackageFileExist( pszResName, key ) )
		return true;
#ifdef _MAPEDIT
	string szName = GetFileResourceName( pszResName, GetID( key ) ).c_str();
	HANDLE h = CreateFile( szName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
	CloseHandle( h );
	return h != INVALID_HANDLE_VALUE;
	//CFileStream file;
	//return file.TryOpenRead( GetFileResourceName( pszResName, GetID( key ) ).c_str() );
#else
	return false;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CResourceTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
CResourceTracker::CResourceTracker( const char *pszResourceName ): szResourceName(pszResourceName)
{
	GetTrackers().push_back( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CResourceTracker::~CResourceTracker()
{
	GetTrackers().remove( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CResourceTracker::DoesExist( const SPartKey &k )
{
	bool bRes;
	CCheckHash::iterator i = check.find( k );
	if ( i == check.end() )
	{
		bRes = CResourceFileOpener::DoesExist( szResourceName.c_str(), k );
		check[k] = bRes;
	}
	else
		bRes = i->second;
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFileRequest
////////////////////////////////////////////////////////////////////////////////////////////////////
CFileRequest::CFileRequest( const char *_pszResName, int _nID ) 
	: pszResName(_pszResName), nID(_nID), bIsReady(false)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CFileRequest::CFileRequest( const char *_pszResName, const SPartKey &_key )
	: pszResName(_pszResName), nID( GetID(_key) ), bIsReady(false)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static NWin32Helper::CCriticalSection readResource;
static bool bIsFileReading = false;
void CFileRequest::Read()
{
	ASSERT(!bIsReady);
	if ( bIsReady )
		return;
	NWin32Helper::CCriticalSectionLock l( readResource );
	bIsFileReading = true;
	try
	{
		CResourceFileOpener f( pszResName, nID );
		f.GetStream()->ReadTo( data, f.GetStream()->GetSize() );
		data.Seek(0);
	}
	catch (...) 
	{
	}
	bIsFileReading = false;
	bIsReady = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Resource loading thread
////////////////////////////////////////////////////////////////////////////////////////////////////
static NWin32Helper::CCriticalSection reqQueue;
static NWin32Helper::CEvent newRequest;
static HANDLE hLoaderThread;
static list<CPtr<CFileRequest> > holdRequests;
static list<CFileRequest*> requests;
static DWORD WINAPI LoaderThread( void* )
{
	for (;;)
	{
		newRequest.Wait();
		newRequest.Reset();
		// process new requests
		CFileRequest *pRes;
		for(;;)
		{
			{
				NWin32Helper::CCriticalSectionLock l( reqQueue );
				if ( requests.empty() )
					break;
				pRes = requests.front();
				requests.pop_front();
			}
			if ( pRes == 0 )
				return 0;
			if ( !IsValid(pRes) )
				continue;
			pRes->Read();
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddFileRequest( CFileRequest *pReq )
{
	if ( pReq->IsReady() )
		return;
	NWin32Helper::CCriticalSectionLock l( reqQueue );
	holdRequests.push_front( pReq );
	requests.push_front( pReq );
	newRequest.Set();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool HasFileRequestsInFly()
{
	NWin32Helper::CCriticalSectionLock l( reqQueue );
	return !bIsFileReading && !requests.empty();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ReleaseFileRequestHolder()
{
	NWin32Helper::CCriticalSectionLock l( reqQueue );
	if ( requests.empty() )
		holdRequests.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void RunResourceLoadingThread()
{
	DWORD dwThread;
	hLoaderThread = CreateThread( 0, 102400, LoaderThread, 0, 0, &dwThread );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SKillLoaderThread
{
	~SKillLoaderThread()
	{
		if ( hLoaderThread )
		{
			{
				NWin32Helper::CCriticalSectionLock l( reqQueue );
				newRequest.Set();
				requests.push_front( 0 );
			}
			WaitForSingleObject( hLoaderThread, INFINITE );
		}
	}
} killLoaderThread;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
