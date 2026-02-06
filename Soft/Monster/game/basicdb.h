#ifndef __BASICDB_H_
#define __BASICDB_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "basic1.h"
#include "BasicChunk1.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NDatabase {
	extern bool bIsDatabaseLoading;
	//
	extern void Import();
}
/////////////////////////////////////////////////////////////////////////////////////
class CDBTable;
class CDBRecord: public CFundament
{
	int nID;
public:
	int GetRecordID() const { return nID; }
	virtual void Serialize( CStructureSaver *pFile ) { pFile->AddData( 1, &nID ); }
	virtual void Import() = 0;
	friend class CDBTable;
};
/////////////////////////////////////////////////////////////////////////////////////
template <class T> 
class CDBPtr: public CPtrBase<CDBRecord, T, CDBRecord::SRef>
{
	typedef CPtrBase<CDBRecord, T, CDBRecord::SRef> CBase;
public:
	CDBPtr() {}
	CDBPtr( T *_ptr ): CBase( _ptr ) {}
	CDBPtr( const CDBPtr &a ): CBase( a.GetPtr() ) {}
	CDBPtr& operator=( T *_ptr ) { Set( _ptr ); return *this; }
	CDBPtr& operator=( const CDBPtr &a ) { Set( a.GetPtr() ); return *this; }
	//
};
/////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline void Serialize( CStructureSaver *pFile, CDBPtr<T> *pObj )
{
	if ( NDatabase::bIsDatabaseLoading )
		pFile->DoPtr( pObj );
	else
	{
		if ( pFile->IsReading() )
		{
			int nID;
			T examp;
			pFile->AddData( 1, &nID );
			int nTableID = NDatabase::GetTableID( &examp );
			CDBTable *pTable = NDatabase::GetTable( nTableID );
			ASSERT( pTable );
			if ( pTable )
				*pObj = static_cast<T*>( pTable->GetRecord( nID ) );
			ASSERT( pObj->GetPtr() );
		}
		else
		{
			CDBRecord *pRec = pObj->GetPtr();
			int nID = pRec ? pRec->GetRecordID() : -1;
			pFile->AddData( 1, &nID );
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
class CDBTable
{
	typedef std::hash_map<int, CPtr<CDBRecord> > CRecordHash;
	CRecordHash records;
	//
	void PreCreate( int nTypeID );
	void Import();
public:
	CDBRecord* GetRecord( int nID );
	friend void NDatabase::Import();
	friend void Serialize( CStructureSaver *pFile, CDBTable *pObj );
};
/////////////////////////////////////////////////////////////////////////////////////
inline void Serialize( CStructureSaver *pFile, CDBTable *pObj ) 
{
	pFile->AddContainer( 1, &pObj->records ); 
}
/////////////////////////////////////////////////////////////////////////////////////
namespace NDatabase
{
	typedef CDBRecord* (*RecordCreateFunc)();
	void SetSource( const char *pszSource );
	void AddTable( int nTableID, const char *pszTableName, RecordCreateFunc newf );
	void AddRelation( const char *pszTableName );
	CDBTable* GetTable( int nTableID );
	int GetTableID( CDBRecord *pExample );
	void Serialize( CStructureSaver &file );
	//
	void ImportField( const char *pszFieldName, int &nData );
	void ImportField( const char *pszFieldName, float &nData );
	void ImportField( const char *pszFieldName, std::string &nData );
	//void ImportField( const char *pszFieldName, std::string &nData ); // wide char string or smth like that
	// simple reference
	void ImportField( const char *pszFieldName, CDBRecord *&pRef, int nDestTable );
	template< class T >
	void ImportField( const char *pszFieldName, CDBPtr<T> &pRef )
	{
		CDBRecord *pRes;
		T test;
		int nTableID = GetTableID( &test );
		ASSERT( nTableID >= 0 );
		ImportField( pszFieldName, pRes, nTableID );
		pRef = (T*)pRes;
	}
	// array of refs
	void ImportField( const char *pszFieldName, CDBRecord *pSrc, std::vector< CDBPtr<CDBRecord> > &refs, int nDestTableID );
	template< class T >
	void ImportField( const char *pszFieldName, CDBRecord *pSrc, std::vector< CDBPtr<T> > &refs )
	{
		T test;
		int nTableID = GetTableID( &test );
		ASSERT( nTableID >= 0 );
		ImportField( pszFieldName, pSrc, *(std::vector<CDBPtr<CDBRecord> >*) &refs, nTableID );
	}
}
#define REGISTER_DATABASE_CLASS( N, table, name ) NDatabase::AddTable( N, table, \
(NDatabase::RecordCreateFunc)name##::New##name );
#define REGISTER_DATABASE_RELATION( table ) NDatabase::AddRelation( table );
/////////////////////////////////////////////////////////////////////////////////////
#endif