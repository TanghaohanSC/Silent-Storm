#include "StdAfx.h"
#include "FilesPackage.h"
#include <iostream>
/////////////////////////////////////////////////////////////////////////////////////
const DWORD DW_PACKAGE_SIGNATURE = 0x95938921;
/////////////////////////////////////////////////////////////////////////////////////
// CFilesPackage
/////////////////////////////////////////////////////////////////////////////////////
struct SFileInfo
{
	unsigned int nStart;
	unsigned int nLength;
};
/////////////////////////////////////////////////////////////////////////////////////
class CFilesPackage: public CFundament
{
	typedef hash_map< int, SFileInfo > CFileInfoHash;
	CFileInfoHash files;
	CFileStream f;
	//
	bool ReadHeader( CMemoryStream *pHeader, int *pnPos );
	void WriteHeader( CMemoryStream *pHeader, int nPos );
public:
	bool Open( const char *pszFileName );
	bool Update( ostream *pErr, const char *pszFileName, const char *pszDir );
	//
	unsigned int Read( unsigned int nPos, void *pDest, unsigned int nSize ) { f.Seek( nPos ); return f.Read( pDest, nSize ); }
	unsigned int Write( unsigned int nPos, const void *pSrc, unsigned int nSize ) { f.Seek( nPos ); return f.Write( pSrc, nSize ); }
	SFileInfo* GetFileInfo( int nFileID )
	{
		CFileInfoHash::iterator it = files.find( nFileID );
		if ( it == files.end() )
			return 0;
		return &it->second;
	}
	//
	friend class CPackageStream;
};
/////////////////////////////////////////////////////////////////////////////////////
bool CFilesPackage::ReadHeader( CMemoryStream *pHeader, int *pnPos )
{
	int nSignature, nPos;
	f.Read( &nSignature, 4 );
	if ( nSignature != DW_PACKAGE_SIGNATURE )
		return false;
	f.Read( &nPos, 4 );
	if ( pnPos )
		*pnPos = nPos;
	f.Seek( nPos );
	pHeader->SetSizeDiscard( f.GetSize() - nPos );
	f.Read( pHeader->GetBufferForWrite(), pHeader->GetSize() );
	pHeader->Seek(0);
	return f.IsOk();
}
/////////////////////////////////////////////////////////////////////////////////////
void CFilesPackage::WriteHeader( CMemoryStream *pHeader, int nPos )
{
	f.Seek( 4 );
	f.Write( &nPos, 4 );
	f.Seek( nPos );
	f.Write( pHeader->GetBuffer(), pHeader->GetSize() );
}
/////////////////////////////////////////////////////////////////////////////////////
bool CFilesPackage::Open( const char *pszFileName ) 
{
	if ( !f.OpenRead( pszFileName ) )
		return false;
	CMemoryStream info;
	if ( !ReadHeader( &info, 0 ) )
		return false;
	{
		CStructureSaver ss( info, CStructureSaver::READ );
		ss.AddDataContainer( 1, &files );
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
static bool IsNumber( const char *pszFileName )
{
	char szBuf[16];
	int n = atoi( pszFileName );
	itoa( n, szBuf, 10 );
	return strcmp( szBuf, pszFileName ) == 0;
}
/////////////////////////////////////////////////////////////////////////////////////
inline bool operator==( const _FILETIME &a, const _FILETIME & b )
{
	return memcmp( &a, &b, sizeof(_FILETIME) ) == 0;
}
bool CFilesPackage::Update( ostream *pErr, const char *pszFileName, const char *pszDir )
{
	int nStartPos = 8;
	ostream &err = *pErr;
	CMemoryStream infoFile;
	hash_map< int, FILETIME > dates;
	// read info on them from .pkg
	if ( !f.Open( pszFileName ) )
	{
		if ( !f.OpenWrite( pszFileName ) )
		{
			err << "Can not open " << pszFileName << endl;
			ASSERT(0);
			return false;
		}
		int nSignature = DW_PACKAGE_SIGNATURE;
		f.Write( &nSignature, 4 );
	}
	else
	{
		if ( !ReadHeader( &infoFile, &nStartPos ) )
		{
			err << "File " << pszFileName << " is corrupted" << endl;
			ASSERT(0);
			return false;
		}
		{
			CStructureSaver ss( infoFile, CStructureSaver::READ );
			ss.AddDataContainer( 1, &files );
			ss.AddDataContainer( 2, &dates );
		}
	}
	// scan dir for files
	WIN32_FIND_DATA ff;
	HANDLE hf = FindFirstFile( (string(pszDir) + "\\*.*").c_str(), &ff );
	if ( hf != INVALID_HANDLE_VALUE )
	{
		for(;;)
		{
			if ( IsNumber( ff.cFileName ) )
			{
				bool bAdd = true;
				int nFileID = atoi( ff.cFileName );
				SFileInfo *pInfo = GetFileInfo( nFileID );
				if ( pInfo )
				{
					hash_map< int, FILETIME >::iterator k = dates.find( nFileID );
					if ( k != dates.end() && k->second == ff.ftLastWriteTime )
						bAdd = false;
				}
				if ( bAdd )
				{
					CMemoryStream file;
					CFileStream src;
					if ( !src.OpenRead( ( string(pszDir) + "\\" + ff.cFileName ).c_str() ) )
					{
						err << "Can not open " << ff.cFileName << endl;
					}
					else
					{
						if ( file.WriteFrom( src ) != src.GetSize() )
							err << "Failed to read " << ff.cFileName << endl;
						else
						{
							if ( pInfo && pInfo->nLength >= ff.nFileSizeLow )
							{
								f.Seek( pInfo->nStart );
								if ( f.Write( file.GetBuffer(), file.GetSize() ) != file.GetSize() )
								{
									err << "Failed to write into " << pszFileName << ", package is corrupted" << endl;
								}
								else
								{
									SFileInfo &fileInfo = files[nFileID];
									fileInfo.nLength = file.GetSize();
									dates[nFileID] = ff.ftLastWriteTime;
								}
							}
							else
							{
								f.Seek( nStartPos );
								if ( f.Write( file.GetBuffer(), file.GetSize() ) != file.GetSize() )
								{
									err << "Failed to write into " << pszFileName << ", package is corrupted" << endl;
								}
								else
								{
									SFileInfo &fileInfo = files[nFileID];
									fileInfo.nStart = nStartPos;
									fileInfo.nLength = file.GetSize();
									dates[nFileID] = ff.ftLastWriteTime;
									nStartPos += file.GetSize();
								}
							}
						}
					}
				}
			}
			else
			{
				if ( ff.cFileName[0] != '.' )
					err << "ignoring " << ff.cFileName << endl;
			}
			if ( !FindNextFile( hf, &ff ) )
				break;
		}
		FindClose( hf );
	}
	// write new header
	{
		infoFile.SetSize(0);
		CStructureSaver ss( infoFile, CStructureSaver::WRITE );
		ss.AddDataContainer( 1, &files );
		ss.AddDataContainer( 2, &dates );
	}
	WriteHeader( &infoFile, nStartPos );
	return f.IsOk();
}
/////////////////////////////////////////////////////////////////////////////////////
// CPackageStream
/////////////////////////////////////////////////////////////////////////////////////
unsigned int CPackageStream::DoRead( unsigned int nPos, void *pDest, unsigned int nSize )
{
	return pPack->Read( nPos + nOffset, pDest, nSize );
}
/////////////////////////////////////////////////////////////////////////////////////
unsigned int CPackageStream::DoWrite( unsigned int nPos, const void *pSrc, unsigned int nSize )
{
	ASSERT(0);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
CPackageStream::CPackageStream( CFilesPackage *_pPack, int nFileID ): pPack(_pPack)
{
	ASSERT( pPack->IsValid() );
	nFlags = F_CanRead;
	SFileInfo *pInfo = pPack->GetFileInfo( nFileID );
	if ( pInfo )
	{
		StartAccess( pInfo->nLength, 1024 );
		nOffset = pInfo->nStart;
	}
	else
	{
		SetFailed();
		nOffset = 0x7fffffff;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
CFilesPackage* OpenFilesPackage( const char *pszFileName )
{
	CFilesPackage *pRes = new CFilesPackage;
	if ( !pRes->Open( pszFileName ) )
		ASSERT(0); // throw?
	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////
bool UpdateFilesPackage( const char *pszFileName, const char *pszDir )
{
	CPtr<CFilesPackage> pRes = new CFilesPackage;
	return pRes->Update( &cerr, pszFileName, pszDir );
}
/////////////////////////////////////////////////////////////////////////////////////
