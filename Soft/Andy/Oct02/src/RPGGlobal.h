#ifndef __RPGGLOBAL_H_
#define __RPGGLOBAL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "ChapterInfo.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NScenario
{
	class CScenarioTracker;
	class CScenarioZone;
}
//
namespace NRPG
{
//
enum EChapterMapMode
{
	GGCM_NULL,
	GGCM_FIRSTTIME,
	GGCM_CONTINUE
};
//
class CMerc;
class CUnit;
class CGlobalDiplomacy;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SUnitDeployData
{
	ZDATA
	int nPassageObjectID;
	CObj<NRPG::CUnit> pCorpse; // CObj для правильного переноса AI unit-ов между template-ми
	bool bCorpseAlive;
	bool bCorpseEnemy;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nPassageObjectID); f.Add(3,&pCorpse); f.Add(4,&bCorpseAlive); f.Add(5,&bCorpseEnemy); return 0; }
	//
	SUnitDeployData(): nPassageObjectID( 0 ), pCorpse( 0 ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SDeployData
{
	ZDATA
	bool bPassage;
	int nPassageZoneID;
	hash_map< CPtr<NRPG::CUnit>, SUnitDeployData, SPtrHash > unitsDeployData;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bPassage); f.Add(3,&nPassageZoneID); f.Add(4,&unitsDeployData); return 0; }
	//
	SDeployData(): bPassage( false ), nPassageZoneID( 0 ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalPlayer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGlobalPlayer: public CObjectBase
{
	OBJECT_BASIC_METHODS( CGlobalPlayer )
	ZDATA
public:
	vector< CPtr<CMerc> > mercs;
	vector< CPtr<CMerc> > totalMercs;
	SDeployData deployData;
private:
	vector< CPtr<NRPG::CUnit> > deadUnits;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&mercs); f.Add(3,&totalMercs); f.Add(4,&deployData); f.Add(5,&deadUnits); return 0; }
	//
	CGlobalPlayer() {}
	//
	void AddMerc( CMerc *pMerc );
	//
	void CaptureUnit( NRPG::CUnit *pCarrier, NRPG::CUnit *pUnit );
	void RescueUnit( NRPG::CUnit *pCarrier, NRPG::CUnit *pUnit );
	void TakeUnitCorpse( NRPG::CUnit *pCarrier, NRPG::CUnit *pUnit );
	void FreeUnit( NRPG::CUnit *pCarrier );
	//
	void MarkUnitAsDead( NRPG::CUnit *pUnit );
	void MarkUnitAsAlive( NRPG::CUnit *pUnit );
	bool IsUnitMarkedDead( NRPG::CUnit *pUnit );
	//
	void GetAliveUnits( vector< CPtr<NRPG::CUnit> > *pUnits );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalGame
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGlobalGame: public CObjectBase
{
	OBJECT_BASIC_METHODS(CGlobalGame);
public:
	ZDATA
	hash_map<string,float> globalVars;

	bool bGlobalMapSet;
	int nGlobalMapID;

	bool bChapterMapSet;
	int nChapterMapID;
	CVec2 vChapterPos;

	CObj<NScenario::CScenarioTracker> pScenarioTracker;
	vector< CObj<CGlobalPlayer> > players;
	CPtr<NScenario::CScenarioZone> pCurrentZone;
	int nCurrentTemplateID;
	CObj<CGlobalDiplomacy> pGlobalDiplomacy;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&globalVars); f.Add(3,&bGlobalMapSet); f.Add(4,&nGlobalMapID); f.Add(5,&bChapterMapSet); f.Add(6,&nChapterMapID); f.Add(7,&vChapterPos); f.Add(8,&pScenarioTracker); f.Add(9,&players); f.Add(10,&pCurrentZone); f.Add(11,&nCurrentTemplateID); f.Add(12,&pGlobalDiplomacy); return 0; }
	//
	CGlobalGame(): bGlobalMapSet( false ), bChapterMapSet( false ) {}
	//
	float GetGlobalVar( const string &szID, float fDefault = 0 ) const;
	void SetGlobalVar( const string &szID, float fValue );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CGlobalGame* CreateGlobalGame( int nScenarioID = -1 );
NRPG::CGlobalPlayer* CreateGlobalPlayer();
NRPG::CGlobalPlayer* CreateGlobalPlayer( const vector<int> &personages );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif