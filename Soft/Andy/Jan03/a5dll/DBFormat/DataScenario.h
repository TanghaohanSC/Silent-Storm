#ifndef __DATASCENARIO_H_
#define __DATASCENARIO_H_
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\ADOImport\BasicDB.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CString;
class CDBScenarioClue;
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EScenarioClueType
{
	CT_PERSON = 0,
	CT_ITEM,
	CT_CONCLUSION,
	N_CT_COUNT
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EScenarioObjectiveType
{
	OT_CAPTURE = 0,
	OT_DESTROY,
	N_OT_COUNT
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBScenario
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBScenario: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBScenario );
	ZDATA
public:
	ZPARENT( CDBRecord );
	string szName;
	string szDescription;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&szName); f.Add(4,&szDescription); return 0; }
	//
	CDBScenario() {}
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBScenarioZone
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBScenarioZone: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBScenarioZone );
	ZDATA
public:
	ZPARENT( CDBRecord );
	vector<int> templatesIDs;
	int nItemSlots;
	int nPersonSlots;
	string sSmallDescription;
	CPtr<CDBScenario> pScenario;
	int nCluesMaxNumber;
	bool bCanBeRevealed;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&templatesIDs); f.Add(4,&nItemSlots); f.Add(5,&nPersonSlots); f.Add(6,&sSmallDescription); f.Add(7,&pScenario); f.Add(8,&nCluesMaxNumber); f.Add(9,&bCanBeRevealed); return 0; }
	//
	CDBScenarioZone() {}
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBScenarioState
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBScenarioState: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBScenarioState );
	ZDATA
public:
	ZPARENT( CDBRecord );
	string sDescription;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&sDescription); return 0; }
	//
	CDBScenarioState() {}
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBScenarioObjective
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBScenarioObjective: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBScenarioObjective );
	ZDATA
public:
	ZPARENT( CDBRecord );
	EScenarioObjectiveType type;
	vector< CPtr<CDBScenarioZone> > zonesToOpen;
	vector< CPtr<CDBScenarioZone> > zonesToBlock;
	CPtr<CString> pDescription;
	CPtr<CDBScenario> pScenario;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&type); f.Add(4,&zonesToOpen); f.Add(5,&zonesToBlock); f.Add(6,&pDescription); f.Add(7,&pScenario); return 0; }
	//
	CDBScenarioObjective() {}
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBScenarioClue
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBScenarioClue: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBScenarioClue );
	ZDATA
public:
	ZPARENT( CDBRecord );
	EScenarioClueType clueType;
	CPtr<CDBScenarioState> pState;
	vector< CPtr<CDBScenarioZone> > zonesToPlace;
	vector< CPtr<CDBScenarioObjective> > objectives;
	string sSmallDescription;
	CPtr<CDBScenario> pScenario;
	int nItemID;
	int nPersID;
	bool bPermanent;
	int nMinParentToOpen;
	CPtr<CString> pDescription;
	bool bGiveImmediately;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&clueType); f.Add(4,&pState); f.Add(5,&zonesToPlace); f.Add(6,&objectives); f.Add(7,&sSmallDescription); f.Add(8,&pScenario); f.Add(9,&nItemID); f.Add(10,&nPersID); f.Add(11,&bPermanent); f.Add(12,&nMinParentToOpen); f.Add(13,&pDescription); f.Add(14,&bGiveImmediately); return 0; }
	//
	CDBScenarioClue() {}
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBScenarioObjective2Clue: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBScenarioObjective2Clue );
	ZDATA
public:
	ZPARENT( CDBRecord );
	CPtr<CDBScenarioObjective> pObjective;
	CPtr<CDBScenarioClue> pClue;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&pObjective); f.Add(4,&pClue); return 0; }
	//
	CDBScenarioObjective2Clue() {}
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __DATASCENARIO_H_