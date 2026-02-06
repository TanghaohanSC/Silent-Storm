#ifndef __FLOWCHARTITEMS_H_
#define __FLOWCHARTITEMS_H_
//
namespace NDb
{
	class CDBScenarioZone;
	class CDBScenarioClue;
	class CDBScenarioObjective;
	enum EScenarioClueType;
	enum EScenarioObjectiveType;
}
//
namespace NScenario
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioClue;
class CScenarioObjective;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioZone
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioZone: public CObjectBase
{
public:
	struct STemplate
	{
		ZDATA
		SRandomSeed sSeed;
		int nVariantID;
		int nItemSlots;
		int nPersonSlots;
		int nInventorySlots;
		int nEmptyItemSlots;
		int nEmptyPersonSlots;
		int nEmptyInventorySlots;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&sSeed); f.Add(3,&nVariantID); f.Add(4,&nItemSlots); f.Add(5,&nPersonSlots); f.Add(6,&nInventorySlots); f.Add(7,&nEmptyItemSlots); f.Add(8,&nEmptyPersonSlots); f.Add(9,&nEmptyInventorySlots); return 0; }
		//
		STemplate(): sSeed( GetTickCount() ), nItemSlots( 0 ), nPersonSlots( 0 ), nInventorySlots( 0 ),
			nEmptyItemSlots( 0 ), nEmptyPersonSlots( 0 ), nEmptyInventorySlots( 0 ), nVariantID( 0 ) {}
	};
	OBJECT_BASIC_METHODS( CScenarioZone );
	ZDATA
	CDBPtr<NDb::CDBScenarioZone> pDBZone;
	int nInnerID;
public:
	vector< CPtr<CScenarioClue> > clues; // находящиеся улики
	CPtr<CScenarioZone> pParentZone;
	int nDifficulty;
	bool bDifCalculated;
	bool bProcessed;
	int nParentObj;
	bool bMarked;
	bool bPassed;
	bool bInaccessible;
	int nDistance;
	bool bBlocked;
	CPtr<CScenarioClue> pPassingParentClue;
	vector< CPtr<CScenarioObjective> > blockers;
	bool bProcessedAsFront;
	hash_map< int, STemplate > templates;
private:
	int nOpenOrder;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pDBZone); f.Add(3,&nInnerID); f.Add(4,&clues); f.Add(5,&pParentZone); f.Add(6,&nDifficulty); f.Add(7,&bDifCalculated); f.Add(8,&bProcessed); f.Add(9,&nParentObj); f.Add(10,&bMarked); f.Add(11,&bPassed); f.Add(12,&bInaccessible); f.Add(13,&nDistance); f.Add(14,&bBlocked); f.Add(15,&pPassingParentClue); f.Add(16,&blockers); f.Add(17,&bProcessedAsFront); f.Add(18,&templates); f.Add(19,&nOpenOrder); return 0; }
	//
private:
	int GetTemplateIDForClue( CScenarioClue *pClue );
	void GetVariantInfo( int nTemplateID, SRandomSeed sSeed, STemplate *templateData );
public:
	CScenarioZone() {}
	CScenarioZone( NDb::CDBScenarioZone *_pDBZone, int _nInnerID );
	//
	NDb::CDBScenarioZone *GetDBZone() const { return pDBZone; }
	bool CanPlaceClue( NDb::EScenarioClueType type );
	void PlaceClue( CScenarioClue *pClue );
	void RemoveClue( CScenarioClue *pClue );
	void DrawNode( fstream &file, int nID, bool bAvailable, bool bDrawBlockers = false );
	void DrawRelationship( fstream &file );
	void SetCluesInaccessible( bool _bInaccessible );
	int GetDifficulty() { return nDifficulty; }
	void SetPassingParentClue( CScenarioClue *pClue );
	void AddBlocker( CScenarioObjective *pBlocker );

	int GetDefaultTemplateID();
	int GetTemplateIDByVariantID( int nVariantID );
	int GetInnerID() { return nInnerID; }
	int GetOpenOrder() { return nOpenOrder; }
	void SetOpenOrder( int _nOpenOrder ) { nOpenOrder = _nOpenOrder; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioClue
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioClue: public CObjectBase
{
	OBJECT_BASIC_METHODS( CScenarioClue );
	ZDATA
	CDBPtr<NDb::CDBScenarioClue> pDBClue;
public:
	int nLock;
	bool bPlaced;
	bool bCompound;
	bool bPassed;
	bool bInaccessible;
	vector< CPtr<CScenarioObjective> > objectives;
	vector< CPtr<CScenarioObjective> > parentObjectives;
	vector< CPtr<CScenarioZone> > parentZones;
	CPtr<CScenarioZone> pPassingParentZone;
	vector< CPtr<CScenarioClue> > passingParentClues;
	bool bChildrenUnlocked;
	int nTemplateID;
private:
	bool bJustFound;
	int nOpenOrder;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pDBClue); f.Add(3,&nLock); f.Add(4,&bPlaced); f.Add(5,&bCompound); f.Add(6,&bPassed); f.Add(7,&bInaccessible); f.Add(8,&objectives); f.Add(9,&parentObjectives); f.Add(10,&parentZones); f.Add(11,&pPassingParentZone); f.Add(12,&passingParentClues); f.Add(13,&bChildrenUnlocked); f.Add(14,&nTemplateID); f.Add(15,&bJustFound); f.Add(16,&nOpenOrder); return 0; }
public:
	//
	CScenarioClue() {}
	CScenarioClue( NDb::CDBScenarioClue *_pDBClue );
	//
	NDb::CDBScenarioClue *GetDBClue() { return pDBClue; }
	bool CanPlaceObjective( CScenarioObjective *pObjective );
	void PlaceObjective( CScenarioObjective *pObjective );
	void SetPlaced( bool _bPlaced = true ) { bPlaced = _bPlaced; }
	bool IsPlaced() { return bPlaced; }
	void SetCompound( bool _bCompound = true ) { bCompound = _bCompound; }
	bool IsCompound() { return bCompound; }
	void CreateObjectives();
	void DrawNode( fstream &file, bool bAvailable );
	void DrawRelationship( fstream &file );
	void SetLock( int _nLock ) { nLock = _nLock; }
	void IncLock() { ++nLock; }
	void DecLock() { --nLock; }
	bool IsLocked() { return nLock > 0; }
	void RemoveParentObjective( CScenarioObjective *pObjective );
	void RemoveChildObjective( CScenarioObjective *pObjective );
	void ClearLinks();
	bool IsCorrect();
	CScenarioObjective *GetObjectiveByType( NDb::EScenarioObjectiveType type );
	void SetPassingParentZone( CScenarioZone *pZone );
	void SetPassingParentClue( CScenarioClue *pClue );
	bool IsJustFound() { return bJustFound; }
	void SetJustFound( bool _bJustFound ) { bJustFound = _bJustFound; }
	int GetOpenOrder() { return nOpenOrder; }
	void SetOpenOrder( int _nOpenOrder ) { nOpenOrder = _nOpenOrder; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioObjective
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioObjective: public CObjectBase
{
	OBJECT_BASIC_METHODS( CScenarioObjective );
	ZDATA
	CDBPtr<NDb::CDBScenarioObjective> pDBObjective;
	bool bPlaced;
public:
	vector< CPtr<CScenarioZone> > zones; // открываемые зоны
	vector< CPtr<CScenarioClue> > clues; // открываемые ключи
	vector< CPtr<CScenarioZone> > zonesToBlock; // блокируемые зоны
	CPtr<CScenarioClue> pParentClue;
	CPtr<CScenarioZone> pParentZone;
	bool bProcessedAsFront;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pDBObjective); f.Add(3,&bPlaced); f.Add(4,&zones); f.Add(5,&clues); f.Add(6,&zonesToBlock); f.Add(7,&pParentClue); f.Add(8,&pParentZone); f.Add(9,&bProcessedAsFront); return 0; }
	//
	CScenarioObjective() {}
	CScenarioObjective( NDb::CDBScenarioObjective *_pDBObjective );
	//
	void RemoveZone( CScenarioZone *pZone );
	void RemoveClue( CScenarioClue *pClue );
	//
	NDb::CDBScenarioObjective *GetDBObjective() { return pDBObjective; }
	void SetPlaced( bool _bPlaced = true ) { bPlaced = _bPlaced; }
	bool IsPlaced() { return bPlaced; }
	bool IsCorrect() { return IsPlaced() && ( !zones.empty() || !clues.empty() ); }
	bool IsZoneBlocker() { return !zonesToBlock.empty(); }
	void BlockZones( bool bBlock );
	bool IsZonesToBlockPassed();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioZone *CreateScenarioZone( NDb::CDBScenarioZone *pDBZone, int nInnerID );
CScenarioClue *CreateScenarioClue( NDb::CDBScenarioClue *pDBZone );
CScenarioObjective *CreateScenarioObjective( NDb::CDBScenarioObjective *pDBObjective );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __FLOWCHARTITEMS_H_