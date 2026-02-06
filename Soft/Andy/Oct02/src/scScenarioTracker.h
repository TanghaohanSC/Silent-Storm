#ifndef __SCENARIOTRACKER_H_
#define __SCENARIOTRACKER_H_
//
#include "..\Misc\Commands.h"
//
struct SRandomSeed;
namespace NRPG
{
	class CMerc;
	class CUnit;
}
namespace NWorld
{
	class CUnitServer;
}
//
namespace NDb
{
	enum EScenarioObjectiveType;
	class CString;
	class CDBScenarioZone;
}
//
namespace NGlobal
{
	class CCmd;
}
//
namespace NScenario
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioClue;
class CScenarioZone;
class CScenarioObjective;
class CScenarioFlowChart;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CScenarioTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScenarioTracker: public CObjectBase
{
	OBJECT_BASIC_METHODS( CScenarioTracker );
	//
	NGlobal::CCmd cmdScenario;
	//
	ZDATA
	bool bScenarioAvailable;
	CObj<CScenarioFlowChart> pScenarioFlowChart;
	list< CPtr<CScenarioZone> > availableZones; // доступные зоны
	list< CPtr<CScenarioObjective> > finishedObjectives; // выполненные objectives
	list< CPtr<CScenarioZone> > blockedZones; // заблокированные зоны
	list< CPtr<CScenarioClue> > takenClues;
	list< CPtr<CScenarioClue> > destroyedClues;
	int nZonesOpenOrder;
	int nCluesOpenOrder;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bScenarioAvailable); f.Add(3,&pScenarioFlowChart); f.Add(4,&availableZones); f.Add(5,&finishedObjectives); f.Add(6,&blockedZones); f.Add(7,&takenClues); f.Add(8,&destroyedClues); f.Add(9,&nZonesOpenOrder); f.Add(10,&nCluesOpenOrder); return 0; }
	//
	void PostCreateScenario();
	bool IsObjectiveFinished( CScenarioObjective *pObjective );
	bool IsClueFound( CScenarioClue *pClue );
	bool IsZoneContainsSomeClue( CScenarioZone *pZone );
	bool IsZoneAvailable( CScenarioZone *pZone );
	bool IsZoneBlocked( CScenarioZone *pZone );
	void ExpandFlowChart();
	void BlockZone( CScenarioZone *pZone );
	void ProcessCluesList( const list< CPtr<CScenarioClue> > &clues,
		NDb::EScenarioObjectiveType type );
	void JustFoundClue( CScenarioClue *pClue );
	void JustOpenZone( CScenarioZone *pZone );
	//
public:
	CScenarioTracker();
	//
	void CreateScenario( int nScenarioID );
	void CreateScenario( string szScenarioName );
	bool IsScenarioAvailable() { return bScenarioAvailable; }
	void DrawScenario();
	void PrintScenarioList();
	int GetTemplateIDByVariantID( int nVariantID );
	CScenarioZone *GetZone( int nTemplateID );
	CScenarioZone *GetZoneByDBZone( NDb::CDBScenarioZone *pDBZone );
	//
	void GetPlacedClues( int nTemplateID, list< CPtr<CScenarioClue> > *clues );
	void GetAvailableZones( list< CPtr<CScenarioZone> > *pZones );
	void GetAvailableClues( list< CPtr<CScenarioClue> > *pClues );
	CScenarioZone *GetRecommendedZone();
	void OpenZone( CScenarioZone *pZone );
	void RevealZone( CScenarioZone *pZone ); // аналог OpenZone при случайном обнаружении зоны
	void CheatOpenZone( CScenarioZone *pZone );
	//
	NDb::CString *GetClueDescriptionFromObjective( CScenarioClue *pClue );
	//
	void OnObjectiveComplete( CScenarioObjective *pObjective );
	void CheatTakeClue( string szName );
	void CheatDestroyClue( string szName );
	void OnScenarioClueTaken( int nID, bool bUnit );
	void OnScenarioClueDestroyed( int nID, bool bUnit );
	void ProcessScenario( const vector< CPtr<NRPG::CUnit> > &units );
	SRandomSeed GetRandomSeedForTemplate( int nTemplateID );
	CScenarioZone *GetZoneInWhichClueWasFound( CScenarioClue *pClue );
	void GetZonesWhichCanBeOpened( CScenarioClue *pClue, list< CPtr<CScenarioZone> > *pZones );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CScenarioTracker *CreateScenarioTracker( int nID );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __SCENARIOTRACKER_H_