#ifndef __BASICCHUNK1_H_
#define __BASICCHUNK1_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "Streams.h"
#include "Basic1.h"
#include "BasicFactory.h"
/////////////////////////////////////////////////////////////////////////////////////
extern CClassFactory<CFundament> SSClasses;
/////////////////////////////////////////////////////////////////////////////////////
#define REGISTER_SAVELOAD_CLASS( N, name ) REGISTER_CLASS( SSClasses, N, name )                 
/////////////////////////////////////////////////////////////////////////////////////
// a) chunk structure
// b) ptr/ref storage
// system is able to store ref/ptr only for objectbase ancestors
// final save file structure
// -header section list of object types with pointers
// -object data separated in chunks one chunk per object
// c) can replace CMemoryStream with specialized objects to increase perfomance

// chunk with index 0 is used for system and should not be used in user code
typedef char chunk_id;
class CStructureSaver
{
public:
	typedef std::string stdString;
private:
	CDataStream &destStream;

	struct CChunkLevel
	{
		chunk_id idChunk;
		int nStart, nLength;
		int nChunkNumber; // номер чанка по порядку для считывания - используется при записи/считывании vector/list
		
		void Clear() { idChunk = (chunk_id)0xff; nChunkNumber = 1; nStart = 0; nLength = 0; }
		CChunkLevel() { Clear(); }
	};
	// objects descriptors
	CMemoryStream obj;
	// objects data
	CMemoryStream data;
	std::list<CChunkLevel> chunks;
	typedef std::list<CChunkLevel>::iterator CChunkLevelIterator;
	typedef std::list<CChunkLevel>::reverse_iterator CChunkLevelReverseIterator;
	bool bIsReading;
	// maps objects addresses during save(first) to addresses during load(second) - during loading
	// or serves as a sign that some object has been already stored - during storing
	typedef std::hash_map<void*,void*,SDefaultPtrHash> CObjectsHash;
	CObjectsHash objects;
	std::list<CFundament*> toStore;

	bool ReadShortChunk( CChunkLevel &src, int &nPos, CChunkLevel &res );
	bool WriteShortChunk( CChunkLevel &dst, chunk_id dwID, const unsigned char *pData, int nLength );
	bool GetShortChunk( CChunkLevel &src, chunk_id dwID, CChunkLevel &res, int nNumber = 1 );
	int CountShortChunks( CChunkLevel &src, chunk_id dwID );
	//
	bool StartChunk( const chunk_id idChunk );
	void FinishChunk();
	void AlignDataFileSize();
	int CountChunks( const chunk_id idChunk );
	void SetChunkCounter( int nCount ) { chunks.back().nChunkNumber = nCount; }
	//
	void DataChunk( const chunk_id idChunk, void *pData, int nSize );
	void RawData( void *pData, int nSize );
	void WriteRawData( const void *pData, int nSize );
	void DataChunkBLOB( CMemoryStream &file );
	void DataChunkString( stdString &data );
	// storing/loading pointers to objects
	void StoreObject( CFundament *pObject );
	CFundament* LoadObject();
	//
	void Start( bool bRead );
	void Finish();
public:
	enum EMode
	{
		READ,
		WRITE
	};
	CStructureSaver( CDataStream &res, EMode mode ): destStream(res) { Start( mode == READ ); }
	~CStructureSaver() { Finish(); }
	bool IsReading() { return bIsReading; }

	//
	template <class T> 
		void AddData( const chunk_id idChunk, T *pData ) { DataChunk( idChunk, pData, sizeof( *pData ) ); }
	template <class T>
		void AddObject( const chunk_id idChunk, T *pData )
		{
			if ( !StartChunk( idChunk ) )
				return;
			Serialize( this, pData );
			FinishChunk();
		}
	template <class T>
		void AddContainer( const chunk_id idChunk, T *pData )
		{
			if ( !StartChunk( idChunk ) )
				return;
			SerializeT( this, pData );
			FinishChunk();
		}
	template <class T>
		void AddDataContainer( const chunk_id idChunk, T *pData )
		{
			if ( !StartChunk( idChunk ) )
				return;
			SerializeData( this, pData );
			FinishChunk();
		}
	// structs for adding objects as templates and as ordinary objects
	template<class T> struct SAdd { void operator()( CStructureSaver *s, const chunk_id idChunk, T &data ) { s->AddData( idChunk, &data ); } };
	template<class T> struct SAddT { void operator()( CStructureSaver *s, const chunk_id idChunk, T &data ) { s->AddObject( idChunk, &data ); } };
	// some funcs for internal use (instead of these funcs use Addxxx()
	void DoString( stdString *pData ) { DataChunkString( *pData ); }
	void DoBLOB( CMemoryStream *pData ) { DataChunkBLOB( *pData ); }
	//
	template <class T1, class T2, class T3> 
		void DoPtr( CPtrBase<T1,T2,T3> *pData ) 
	{
		if ( IsReading() ) 
			pData->Set( (T2*)LoadObject() ); 
		else 
			StoreObject( pData->GetBarePtr() );
	}
	//
	// serializnig several standard containers
	// vector
	template <class T,class Adder> void DoVector( std::vector<T> &data, Adder adder )
	{
		int i, nSize;
		if ( IsReading() )
		{
			data.clear();
			data.resize( nSize = CountChunks( 1 ) );
		}
		else
			nSize = data.size();
		for ( i = 0; i < nSize; i++ )
		{
			SetChunkCounter( i + 1 );
			adder( this, 1, data[i] );
		}
	}
	template <class T> void DoDataVector( std::vector<T> &data )
	{
		int nSize = data.size();
		AddData( 1, &nSize );
		if ( IsReading() )
		{
			data.clear();
			data.resize( nSize );
		}
		if ( nSize > 0 )
			DataChunk( 2, &data[0], sizeof(T) * nSize );
	}
	// list
	template <class T,class Adder> void DoList( std::list<T> &data, Adder adder )
	{
		if ( IsReading() )
		{
			data.clear();
			data.insert( data.begin(), CountChunks( 1 ), T() );
		}
		int i = 1;
		for ( list<T>::iterator k = data.begin(); k != data.end(); ++k, ++i )
		{
			SetChunkCounter( i );
			adder( this, 1, *k );
		}
	}
	// map
	template <class T1, class T2> void DoMap( std::map<T1,T2> &data )
	{
		ASSERT(0);
		// TODO...
	}
	// hash_map
	template <class T1,class T2,class T3,class T4, class Adder> void DoHashMap( std::hash_map<T1,T2,T3,T4> &data, Adder adder )
	{
		if ( IsReading() )
		{
			data.clear();
			for ( int nSize = CountChunks( 1 ), i = 1; nSize > 0; nSize--, i++ )
			{
				T1 idx;
				SetChunkCounter( i );
				AddData( 1, &idx );
				adder( this, 2, data[idx] );
			}
		}
		else
		{
			for ( std::hash_map<T1,T2,T3,T4>::iterator pos = data.begin(); pos != data.end(); ++pos )
			{
				T1 idx = pos->first;
				AddData( 1, &idx );
				adder( this, 2, pos->second );
			}
		}
	}
};
inline void Serialize( CStructureSaver *pFile, CStructureSaver::stdString *pObj ) { pFile->DoString( pObj ); }
inline void Serialize( CStructureSaver *pFile, CMemoryStream *pObj ) { pFile->DoBLOB( pObj ); }
template <class T>
inline void Serialize( CStructureSaver *pFile, CPtr<T> *pObj )
{
	pFile->DoPtr( pObj );
}
template <class T>
inline void Serialize( CStructureSaver *pFile, CObj<T> *pObj )
{
	pFile->DoPtr( pObj );
}
template <class T>
inline void Serialize( CStructureSaver *pFile, CMObj<T> *pObj )
{
	pFile->DoPtr( pObj );
}
template <class T>
inline void SerializeData( CStructureSaver *pFile, std::vector<T> *pData )
{
	pFile->DoDataVector( *pData );
}
template <class T>
inline void SerializeData( CStructureSaver *pFile, std::list<T> *pData )
{
	pFile->DoList( *pData, CStructureSaver::SAdd<T>() );
}
template <class T1,class T2,class T3,class T4> 
inline void SerializeData( CStructureSaver *pFile, std::hash_map<T1,T2,T3,T4> *pData )
{ 
	pFile->DoHashMap( *pData, CStructureSaver::SAdd<T2>() );
}
/////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline void SerializeT( CStructureSaver *pFile, std::vector<T> *pData )
{
	pFile->DoVector( *pData, CStructureSaver::SAddT<T>() );
}
template <class T>
inline void SerializeT( CStructureSaver *pFile, std::list<T> *pData )
{
	pFile->DoList( *pData, CStructureSaver::SAddT<T>() );
}
template <class T1,class T2,class T3,class T4> 
inline void SerializeT( CStructureSaver *pFile, std::hash_map<T1,T2,T3,T4> *pData )
{ 
	pFile->DoHashMap( *pData, CStructureSaver::SAddT<T2>() );
}
/////////////////////////////////////////////////////////////////////////////////////
#endif