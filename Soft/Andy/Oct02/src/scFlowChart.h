#ifndef __FLOWCHART_H_
#define __FLOWCHART_H_
//
namespace NDb
{
	class CDBScenarioClue;
	class CDBScenarioZone;
	class CDBScenarioObjective;
}
//
namespace NScenario
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioZone;
class CScenarioClue;
class CScenarioObjective;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioFlowChart
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioFlowChart: public CObjectBase
{
	OBJECT_BASIC_METHODS( CScenarioFlowChart );
	ZDATA
	bool bFull;
public:
	int nScenarioID;
	vector< CObj<CScenarioZone> > zones;
	vector< CObj<CScenarioClue> > clues;
	vector< CObj<CScenarioObjective> > objectives;
	int nCurrentDistance;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bFull); f.Add(3,&nScenarioID); f.Add(4,&zones); f.Add(5,&clues); f.Add(6,&objectives); f.Add(7,&nCurrentDistance); return 0; }
	//
private:
	CScenarioClue *GetClueByDBClue( NDb::CDBScenarioClue *pDBClue );
	CScenarioObjective *GetObjectiveByDBObjective( NDb::CDBScenarioObjective *pDBObjective );
	void PlaceClue( CScenarioClue *pClue );
	void RemoveClue( CScenarioClue *pClue );
	void PlaceObjective( CScenarioObjective *pObjective );
	void SetupLinks( CScenarioObjective *pObjective );
	void LoadItems();
	void ClearItems();
	void RemoveIncorrectClues();
	void InitCompoundLock();
	void GetChildZones( CScenarioObjective *pObjective, 
		CScenarioZone *pParentZone, CScenarioClue *pParentClue, list< CPtr<CScenarioZone> > *pChildZones, 
		bool bCheckCompound, bool bUseProcessed, bool bSetDistance, bool bUseBlocks );
	void GetChildZones( CScenarioClue *pClue, 
		CScenarioZone *pParentZone,  CScenarioClue *pParentClue, list< CPtr<CScenarioZone> > *pChildZones, 
		bool bCheckCompound, bool bUseProcessed, bool bSetDistance, bool bUseBlocks );
	void GetChildZones( CScenarioZone *pZone, list< CPtr<CScenarioZone> > *pChildZones, 
		bool bCheckCompound, bool bUseProcessed, bool bSetDistance, bool bUseBlocks );
	void InitPathFinding();
	void InitDifficulties();
	void SetPathDifficulty( const list< CPtr<CScenarioZone> > &path, bool bMark, bool bCheckBegin = true );
	bool GetUnprocessedEdge( CScenarioZone **ppBegin, CScenarioZone **ppEnd );
	void Header( fstream &file, string szFileName );
	void Footer( fstream &file, string szFileName );
	void DrawZones( fstream &file, const list< CPtr<CScenarioZone> > &availableZones );
	void DrawClues( fstream &file, const list< CPtr<CScenarioClue> > &availableClues );
	void DrawZonesRelationships( fstream &file );
	void DrawCluesRelationships( fstream &file );
	void MarkInaccessible();
	void PassFlowChart( fstream &file, const list< CPtr<CScenarioZone> > &availableZones,
		void ( * RegularNode )( fstream &file, CScenarioZone *pParent, CScenarioZone *pZone, bool bAvailable ), 
		void ( * RefNode )( fstream &file, CScenarioZone *pParent, CScenarioZone *pZone, int nID, bool bAvailable ) );
	void InitClueLock( CScenarioClue *pClue );
	//
	bool GetPath( CScenarioZone *pBegin, 
		list< CPtr<CScenarioZone> > *pPath, bool (* IsFinal)( CScenarioZone *pZone ), 
		bool bCheckCompound, bool bSetDistance, bool bUseBlocks );
	void GenerateFlowChart();
	bool CheckCorrectness();
	bool CheckDataCorrectness();
	void CalculateDifficulties();
	void SetDistance();
	void GetObjectives( vector< CPtr<CScenarioObjective> > *pObjectives );
	void MarkPassed();
	void BlockZones( bool bBlock );
	//
public:
	//
	CScenarioFlowChart() {}
	CScenarioFlowChart( int _nScenarioID, bool _bFull );
	//
	void DrawFlowChart( const list< CPtr<CScenarioZone> > &availableZones, 
		const list< CPtr<CScenarioClue> > &availableClues, const char *pszOutputFile = 0 );
	void DrawPassingFlowChart( const list< CPtr<CScenarioZone> > &availableZones );
	void PrintScenarioList();
	//
	CScenarioZone *GetZoneByDBZone( NDb::CDBScenarioZone *pDBZone );
	CScenarioZone *GetZoneByTemplateID( int nTemplateID );
	int GetTemplateIDByVariantID( int nVariantID );
	CScenarioZone *GetZoneByName( string szName );
	CScenarioClue *GetClueByItemID( int nItemID );
	CScenarioClue *GetClueByName( string szName );
	CScenarioClue *GetClueByPersID( int nPersID );
	int GetMinParentToOpen( CScenarioClue *pClue );
	void GetParentClues( CScenarioClue *pClue, 
		const vector< CPtr<CScenarioObjective> > _objectives, vector< CPtr<CScenarioClue> > *pClues );
	void GetZonesWhichCanBeOpened( CScenarioClue *pClue, list< CPtr<CScenarioZone> > *pZones );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioFlowChartState
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioFlowChartState: public CObjectBase
{
public:
	OBJECT_BASIC_METHODS( CScenarioFlowChartState );
	ZDATA
private:
	CPtr<CScenarioFlowChart> pFlowChart;
	list< CPtr<CScenarioZone> > zones;
	list< CPtr<CScenarioZone> > branchingZones;
	CPtr<CScenarioZone> pFrontZone;
	vector<DWORD> signature;
	bool bCalculateSignature;	
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pFlowChart); f.Add(3,&zones); f.Add(4,&branchingZones); f.Add(5,&pFrontZone); f.Add(6,&signature); f.Add(7,&bCalculateSignature); return 0; }
	//
	void CopyState( CScenarioFlowChartState state );
	void CalculateSignature();
	void UpdateSignature();
	//
public:
	CScenarioFlowChartState() {}
	CScenarioFlowChartState( CScenarioFlowChart *_pFlowChart );
	CScenarioFlowChartState( CScenarioFlowChartState *_pState );
	//
	bool IsContain( CScenarioZone *pZone );
	bool IsContainState( CScenarioFlowChartState *pState );
	void operator=( CScenarioFlowChartState &state );
	bool operator==( CScenarioFlowChartState &state );
	void operator+=( CScenarioZone *pZone );
	void GetSignature( vector<DWORD> *pSignature );
	void GetZones( list< CPtr<CScenarioZone> > *pZones ) const ;
	CScenarioFlowChart *GetFlowChart() const;
	CScenarioZone *GetFrontZone() const;
	void AddBranchingZone( CScenarioZone *pZone );
	int GetHashKey();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioFlowChartPathFinder
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioFlowChartPathFinder: public CObjectBase
{
	OBJECT_BASIC_METHODS( CScenarioFlowChartPathFinder );
	ZDATA
	CPtr<CScenarioFlowChart> pFlowChart;
	hash_map< int, list< CPtr<CScenarioFlowChartState> > > passedStates;
	hash_map< int, list< CPtr<CScenarioFlowChartState> > > finalStates;
	list< CPtr<CScenarioFlowChartState> > states;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pFlowChart); f.Add(3,&passedStates); f.Add(4,&finalStates); f.Add(5,&states); return 0; }
	//
	bool IsStatePassed( CScenarioFlowChartState *pState );
	void PassState( CScenarioFlowChartState *pState );
	void AddFinalState( CScenarioFlowChartState *pState );
	//
public:
	CScenarioFlowChartPathFinder() {}
	CScenarioFlowChartPathFinder(	CScenarioFlowChart *_pFlowChart );
	//
	bool FindPath( CScenarioFlowChartState *pState );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioFlowChart *CreateScenarioFlowChart( int nScenarioID, bool bFull );
CScenarioFlowChart *CreateScenarioFlowChart( string szScenarioName, bool bFull );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __FLOWCHART_H_