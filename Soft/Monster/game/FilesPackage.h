#ifndef __FILEPACKAGE_H_
#define __FILEPACKAGE_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "Streams.h"
/////////////////////////////////////////////////////////////////////////////////////
// read only stream
class CFilesPackage;
class CPackageStream: public CBufferedStream
{
	CPtr<CFilesPackage> pPack;
	int nOffset;
	//
	virtual unsigned int DoRead( unsigned int nPos, void *pDest, unsigned int nSize );
	virtual unsigned int DoWrite( unsigned int nPos, const void *pSrc, unsigned int nSize );
	CPackageStream( const CPackageStream &a ) { ASSERT(0); }
	CPackageStream& operator=( const CPackageStream &a ) { ASSERT(0); return *this;}
public:
	CPackageStream( CFilesPackage *_pPack, int nFileID );
};
/////////////////////////////////////////////////////////////////////////////////////
CFilesPackage* OpenFilesPackage( const char *pszFileName );
bool UpdateFilesPackage( const char *pszFileName, const char *pszDir );
/////////////////////////////////////////////////////////////////////////////////////
#endif