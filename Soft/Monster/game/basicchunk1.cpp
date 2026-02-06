#include "StdAfx.h"
#include "BasicChunk1.h"

/////////////////////////////////////////////////////////////////////////////////////
// class factory
CClassFactory<CFundament> SSClasses;
/////////////////////////////////////////////////////////////////////////////////////
// chunks operations with whole saves
/////////////////////////////////////////////////////////////////////////////////////
static bool ReadShortChunkSave( CDataStream &file, chunk_id &dwID, CMemoryStream &chunk )
{
	DWORD dwLeng = 0, dwRead;
	file.Read( &dwID, sizeof( dwID ) );
	file.Read( &dwLeng, 1 );
	if ( dwLeng & 1 )
		file.Read( ((char*)&dwLeng)+1, 3 );
	dwLeng >>= 1;
	if ( !file.IsOk() )
		return false;
	if ( dwLeng > 10000000 )
		return false;
	chunk.SetSizeDiscard( dwLeng );
	dwRead = file.Read( chunk.GetBufferForWrite(), dwLeng );
	return dwRead == dwLeng;
}
/////////////////////////////////////////////////////////////////////////////////////
static bool WriteShortChunkSave( CDataStream &file, chunk_id dwID, CMemoryStream &chunk )
{
	DWORD dwLeng;
	file.Write( &dwID, sizeof( dwID ) );
	dwLeng = chunk.GetSize();
	dwLeng <<= 1;
	if ( dwLeng >= 256 )
	{
		dwLeng |= 1;
		file.Write( &dwLeng, sizeof( dwLeng ) );
	}
	else
		file.Write( &dwLeng, 1 );
	file.Write( chunk.GetBuffer(), chunk.GetSize() );
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
static bool GetShortChunkSave( CDataStream &file, chunk_id dwID, CMemoryStream &chunk, int nNumber = 1 )
{
	chunk_id dwRid;
	file.SetOk();
	file.Seek( 0 );
	while( ReadShortChunkSave( file, dwRid, chunk ) )
	{
		if ( dwRid == dwID )
		{
			if ( nNumber == 1 )
				return true;
			nNumber--;
		}
	}
	chunk.Clear();
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////
// chunks operations with ChunkLevels
/////////////////////////////////////////////////////////////////////////////////////
static void ReadPtrData( const unsigned char *pData, void *pDst, int &nPos, int nSize )
{
	memcpy( pDst, pData + nPos, nSize );
	nPos += nSize;
}
// should copy data from start
static void WritePtrData( unsigned char *pDst, const void *pSrc, int *nPos, int nSize )
{
	memcpy( pDst + *nPos, pSrc, nSize );
	*nPos += nSize;
}
/////////////////////////////////////////////////////////////////////////////////////
bool CStructureSaver::ReadShortChunk( CChunkLevel &src, int &nPos, CChunkLevel &res )
{
	const unsigned char *pSrc = data.GetBuffer() + src.nStart;
	DWORD dwLeng = 0;
	if ( nPos + 2 > src.nLength )
		return false;
	ReadPtrData( pSrc, &res.idChunk, nPos, sizeof( res.idChunk ) );
	ReadPtrData( pSrc, &dwLeng, nPos, 1 );
	if ( dwLeng & 1 )
		ReadPtrData( pSrc, ((char*)&dwLeng)+1, nPos, 3 );
	dwLeng >>= 1;
	if ( nPos + dwLeng > src.nLength )
		return false;
	res.nStart = nPos + src.nStart;
	res.nLength = dwLeng;
	nPos += dwLeng;
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
bool CStructureSaver::WriteShortChunk( CChunkLevel &dst, chunk_id dwID, 
																			const unsigned char *pData, int nLength )
{
	DWORD dwLeng;
	data.SetSize( dst.nStart + dst.nLength + 1 + 4 + nLength );
	unsigned char *pDst = data.GetBufferForWrite() + dst.nStart;
	WritePtrData( pDst, &dwID, &dst.nLength, sizeof( dwID ) );
	dwLeng = nLength;
	dwLeng <<= 1;
	if ( dwLeng >= 256 )
	{
		dwLeng |= 1;
		WritePtrData( pDst, &dwLeng, &dst.nLength, sizeof( dwLeng ) );
	}
	else
		WritePtrData( pDst, &dwLeng, &dst.nLength, 1 );
	// prevent copying to itself
	if ( pDst + dst.nLength != pData )
		WritePtrData( pDst, pData, &dst.nLength, nLength );
	else
		dst.nLength += nLength;
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
bool CStructureSaver::GetShortChunk( CChunkLevel &src, chunk_id dwID, CChunkLevel &res, int nNumber )
{
	int nPos = 0;
	while ( ReadShortChunk( src, nPos, res ) )
	{
		if ( res.idChunk == dwID )
		{
			if ( nNumber == 1 )
				return true;
			nNumber--;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////
int CStructureSaver::CountShortChunks( CChunkLevel &src, chunk_id dwID )
{
	int nPos = 0, nRes = 0;
	CChunkLevel temp;
	while ( ReadShortChunk( src, nPos, temp ) )
	{
		if ( temp.idChunk == dwID )
			nRes++;
	}
	return nRes;
}
/////////////////////////////////////////////////////////////////////////////////////
// CStructureSaver main methods
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::DataChunk( const chunk_id idChunk, void *pData, int nSize )
{
	CChunkLevel &last = chunks.back();
	if ( IsReading() )
	{
		CChunkLevel res;
		if ( GetShortChunk( last, idChunk, res, last.nChunkNumber ) )
		{
			ASSERT( res.nLength == nSize );
			memcpy( pData, data.GetBuffer() + res.nStart, nSize );
		}
		else
			memset( pData, 0, nSize );
	}
	else
		WriteShortChunk( last, idChunk, (const unsigned char*) pData, nSize );
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::WriteRawData( const void *pData, int nSize )
{
	CChunkLevel &res = chunks.back();
	data.SetSize( res.nStart + nSize );
	unsigned char *pDst = data.GetBufferForWrite() + res.nStart;
	WritePtrData( pDst, pData, &res.nLength, nSize );
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::RawData( void *pData, int nSize )
{
	if ( IsReading() )
	{
		CChunkLevel &res = chunks.back();
		ASSERT( res.nLength == nSize );
		memcpy( pData, data.GetBuffer() + res.nStart, nSize );
	}
	else
	{
		WriteRawData( pData, nSize );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::DataChunkString( stdString &str )
{
	if ( IsReading() )
	{
		CChunkLevel &res = chunks.back();
		const unsigned char *pStr = data.GetBuffer() + res.nStart;
		ASSERT( pStr[ res.nLength - 1 ] == 0 );
		str = (const char*) pStr;
	}
	else
	{
		WriteRawData( (const unsigned char*)str.c_str(), strlen(str.c_str()) + 1 );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::DataChunkBLOB( CMemoryStream &file )
{
	int nLeng = file.GetSize();
	AddData( 1, &nLeng );
	if ( IsReading() )
	{
		file.Clear();
		file.SetSizeDiscard( nLeng );
	}
	DataChunk( 2, file.GetBufferForWrite(), nLeng );
	file.Seek( 0 );
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::StoreObject( CFundament *pObject )
{
	if ( pObject != 0 && objects.find( pObject ) == objects.end() )
	{
		toStore.push_back( pObject );
		objects[pObject] = this; // важно присвоить хоть что-нибудь
	}
	RawData( &pObject, 4 );
}
/////////////////////////////////////////////////////////////////////////////////////
CFundament* CStructureSaver::LoadObject()
{
	void *pServerPtr = 0;
	RawData( &pServerPtr, 4 );
	if ( pServerPtr != 0 )
	{
		CObjectsHash::iterator pFound = objects.find( pServerPtr );
		if ( pFound != objects.end() )
			return (CFundament*)pFound->second;
		ASSERT(0);
		// here  we are in very crappy position - stored object does not exist
		// actually i think we got to throw the exception
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
bool CStructureSaver::StartChunk( const chunk_id idChunk )
{
	CChunkLevel &last = chunks.back();
	if ( IsReading() ) 
	{
		chunks.push_back();
		return GetShortChunk( last, idChunk, chunks.back(), last.nChunkNumber );
	}
	else 
	{
		chunks.push_back();
		CChunkLevel &newChunk = chunks.back();
		newChunk.idChunk = idChunk;
		newChunk.nStart = last.nStart + last.nLength + sizeof( chunk_id ) + 4;
		return true;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::FinishChunk()
{
	if ( IsReading() ) 
	{
		chunks.pop_back();
	}
	else 
	{
		CChunkLevelReverseIterator it = chunks.rbegin(), it1;
		it1 = it; ++it1;
		WriteShortChunk( *it1, it->idChunk, data.GetBuffer() + it->nStart, it->nLength );
		chunks.pop_back();
		AlignDataFileSize();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::AlignDataFileSize()
{
	CChunkLevel &last = chunks.back();
	data.SetSize( last.nStart + last.nLength );
}
/////////////////////////////////////////////////////////////////////////////////////
int CStructureSaver::CountChunks( const chunk_id idChunk )
{
	return CountShortChunks( chunks.back(), idChunk );
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::Start( bool bRead )
{
	CDataStream &res = destStream;
	//
	chunks.clear();
	obj.Clear();
	data.Clear();
	chunks.push_back();
	bIsReading = bRead;
	if ( bRead )
	{
		// read chunk with objects description
		GetShortChunkSave( res, 0, obj );
		GetShortChunkSave( res, 2, data );
		chunks.back().nLength = data.GetSize();
		// create all objects from obj
		while ( obj.GetPosition() < obj.GetSize() )
		{
			int nTypeID = 0;
			void *pServer = 0;
			bool bValid;
			obj.Read( &nTypeID, 4 );
			obj.Read( &pServer, 4 );
			obj.Read( &bValid,1 );
			CFundament *pObject = SSClasses.CreateObject( nTypeID );
			if ( !bValid )
			{
				// make object invalid
				CPtr<CFundament> pTemp( pObject );
				{
					CObj<CObjectBase> pTempObj( (CObjectBase*)pObject );
				}
				pTemp.Extract();
			}
			toStore.push_back( pObject );
			objects[pServer] = pObject;
		}
		// read information about every created object
		int nCount = CountChunks( (chunk_id) 1 );
		for ( int i = 0; i < nCount; i++ )
		{
			void *pServer = 0;
			CFundament *pObject;
			SetChunkCounter( i + 1 );
			StartChunk( (chunk_id) 1 );
			DataChunk( 0, &pServer, 4 );
			pObject = (CFundament*)objects[pServer];
			ASSERT( pObject );
			if ( pObject )
			{
				if ( StartChunk( 1 ) )
				{
					pObject->Serialize( this );
					FinishChunk();
				}
			}
			FinishChunk();
		}
		SetChunkCounter( 1 );

		// read main objects data
		chunks.back().Clear();
		GetShortChunkSave( res, 1, data );
		chunks.back().nLength = data.GetSize();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CStructureSaver::Finish()
{
	CDataStream &res = destStream;
	// CRAP check if chunks list has only one layer
	if ( !IsReading() )
	{
		// save standard data
		AlignDataFileSize();
		WriteShortChunkSave( res, 1, data );
		// store referenced objects
		data.Clear();
		chunks.back().Clear();
		while ( !toStore.empty() )
		{
			CFundament *pObject = toStore.front();
			toStore.pop_front();
			// save object type and its server pointer
			int nTypeID = SSClasses.GetObjectTypeID( pObject );
			bool bValid = ((CObjectBase*)pObject)->IsValid();
			ASSERT( nTypeID != -1 );
			obj.Write( &nTypeID, 4 );
			obj.Write( &pObject, 4 );
			obj.Write( &bValid, 1 );
			// save object data
			StartChunk( (chunk_id) 1 );
			DataChunk( 0, &pObject, 4 );
			//
			if ( StartChunk( 1 ) )
			{
				pObject->Serialize( this );
				FinishChunk();
			}
			FinishChunk();
		}
		// save data into resulting file
		WriteShortChunkSave( res, 0, obj );
		AlignDataFileSize();
		WriteShortChunkSave( res, 2, data );
	}
	else
	{
		// remove unreferenced objects
		for ( CObjectsHash::iterator i = objects.begin(); i != objects.end(); i++ )
		{
			CPtr<CFundament> ref( (CFundament*)i->second );
		}
	}
	obj.Clear();
	data.Clear();
	objects.clear();
	toStore.clear();
	chunks.clear();
}
/////////////////////////////////////////////////////////////////////////////////////
