#ifndef __DATAACK_H_
#define __DATAACK_H_

#include "..\ADOImport\BasicDB.h"

namespace NDb
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_ACKINFO_MAX_COUNT = 3;
const int N_ACK_MAX_PARAM_COUNT = 3;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRPGPers;
class CSound;
class CString;
class CSequence;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBAckInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAckVoice
{
	ZDATA
	CPtr<CSound> pSound; // звук
	CPtr<CSequence> pSequence; // для головы
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pSound); f.Add(3,&pSequence); return 0; }
	//
	SAckVoice() {}
	SAckVoice( CSound *_pSound, CSequence *_pSequence ): pSound( _pSound ), pSequence( _pSequence ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBAckInfo: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBAckInfo );
	ZDATA
	ZPARENT( CDBRecord );
public:
	int nRPGPersID; // кто выполняет ack
	CPtr<CString> pText; // текстовый вариант ack
	vector< SAckVoice > voices;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&nRPGPersID); f.Add(4,&pText); f.Add(5,&voices); return 0; }

	virtual void Import();
	const SAckVoice& GetVoice( int nVoice ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBAckSequence
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBAckSequence: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBAckSequence );
public:
	CPtr<CDBAckInfo> pDBAckInfo[N_ACKINFO_MAX_COUNT]; // ack-и
	int nPriority; // приоритет

	int operator&( CStructureSaver &f );
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBAck
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBAck: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBAck );
public:
	float fProbability; // вероятность выполнения
	int nRPGPersID; // кто отслеживает условие
	CPtr<CDBAckSequence> pAckSequence; // последовательность ack	
	string sParam[N_ACK_MAX_PARAM_COUNT]; // параметры условия
	int nConditionID; // номер проверяемого условия ( идентирикатор класса )

	int operator&( CStructureSaver &f ); 
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBDialogSeq
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBDialogSeq: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBDialogSeq );
	ZDATA
	ZPARENT( CDBRecord );
public:
	int nDialogID;
	CPtr<CDBAckInfo> pAckInfo;
	int nAckInfoOrder;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&nDialogID); f.Add(4,&pAckInfo); f.Add(5,&nAckInfoOrder); return 0; }
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBDialog
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBDialog: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBDialog );
	ZDATA
	ZPARENT( CDBRecord );
public:
	string szCode;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&szCode); return 0; }
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBDialogPers
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBDialogPers: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBDialogPers );
	ZDATA
	ZPARENT( CDBRecord );
public:
	int nPersID; // we don't need CPtr<CRPGPers>
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&nPersID); return 0; }
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CDBAck* GetDBAck( int nID );
CDBDialog* GetDBDialog( int nID );
NDb::CDBDialog* GetDBDialogByCode( const string &szCode );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif