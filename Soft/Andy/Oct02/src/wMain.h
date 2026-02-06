#ifndef __WMAIN_H_
#define __WMAIN_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "wInterface.h"
#include "wTurnBased.h"
#include "wDebris.h"
#include "TerrainInfo.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMapInfo;
struct SClueSlot;

namespace NAI
{
	class IAISignalManager;
	class IAIJobManager;
	class IPathNetwork;
	class CMultiMovesTable;
}
namespace NGScene
{
	class CBuildInfo;
}
namespace NRPG
{
	class CGlobalPlayer;
	class IAttackable;
	class IGame;
	class IObject;
	class CCoverInfo;
	class CAttackPortion;
	class IUnitMission;
	class IClipItem;
	enum EAction;
	class CUnit;
}
namespace NDb
{
	class CObject;
	class CDebrisMaterial;
	class CRPGGrenade;
	class CRPGMeleeWeapon;
	class CAISound;
	class CDBAckSequence;
	class CScript;
}
namespace NScript
{
	class CScript;
}
namespace NScenario
{
	class CScenarioClue;
}
struct STerrainInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
class CPlayer;
class CWorld;
class CUnitServer;
class CDumbUnitServer;
class CObjectServerBase;
class CObjectServer;
struct SObjectPlace;
class CBuilding;
class IDynamicObject;
class CTimedObject;
class CGlobalAck;
class CTerrain;
struct SInterfaceAck;
class CPassageObject;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPlayer: public IPlayer, public CPlayerBase<CUnitServer>
{
	OBJECT_BASIC_METHODS(CPlayer);
	typedef CPlayerBase<CUnitServer> TPlayerBase;
	ZDATA_(TPlayerBase)
	int nCheatFlags;
	wstring wsName;
	NAI::SPathPlace deploySpot;
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TPlayerBase*)this); f.Add(2,&nCheatFlags); f.Add(3,&wsName); f.Add(4,&deploySpot); f.Add(5,&pGlobalPlayer); return 0; }
public:
	CPlayer() {}
	CPlayer( const wstring &_wsName, NRPG::CGlobalPlayer *_pGlobalPlayer );

	virtual void SetCheat( int nCheat, bool bOn );
	virtual bool IsCheatEnabled( int nCheat ) const;
	virtual void GetSounds( vector<IVisObj*> *pRes );
	virtual void GetUnits( CUnitSet *pRes ) const;
	virtual void GetVisible( list< CPtr<CUnit> > *pRes ) const;
	virtual const wstring& GetPlayerName() const { return wsName; }
	virtual NRPG::CGlobalPlayer* GetGlobalPlayer() const { return pGlobalPlayer; }
	virtual CCommander* GetCommander() { return TPlayerBase::GetCommander(); }	
	virtual void GetDeploySpot( NAI::SPathPlace *pRes ) { *pRes = deploySpot; }
	void SetDeploySpot( const NAI::SPathPlace &p ) { deploySpot = p; }

	virtual bool GetInHandItem( SItemInfo *pInfo ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SInterruptInfo
{
	struct SNotice
	{
		CUnitServer *pWho, *pWhom;
		bool bIsMutual;
		bool bWasShot;
		float fStrength;
		
		SNotice() {}
		SNotice( CUnitServer *_pWho, CUnitServer *_pWhom, bool _bWasShot ): pWho(_pWho), 
			pWhom(_pWhom), bIsMutual(false), bWasShot(_bWasShot) {}
	};
	list<SNotice> events;

	void AddEvent( CUnitServer *pWho, CUnitServer *pWhom, bool bWasShot = false )
		{ events.push_back( SNotice( pWho, pWhom, bWasShot ) ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPostWorldCreateInfo : public CObjectBase
{
	OBJECT_NOCOPY_METHODS(CPostWorldCreateInfo);
public:
	list<CDBPtr<NDb::CScript> > scripts;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorld: public IWorld, public CTBSWorld<CUnitServer, CPlayer>, public CDebrisController
{
	typedef CTBSWorld<CUnitServer, CPlayer> TTBSWorld;
	typedef hash_map< CPtr<NScenario::CScenarioClue>, SClueSlot, SPtrHash > ClueToSlot;
	OBJECT_BASIC_METHODS(CWorld);
	ZDATA_(TTBSWorld)
	CObj<CWorldSyncSrc> pShow, pShowUnits;
	list< CPtr<CHitLocator> > eventHits;
	list< CPtr<CActionLocator> > eventActions;
	CObj< NWorld::CTerrain > pTerrain;
	CObj<CCTime> pTime, pAimTime;
	STime tPrev, tHiddenDelta;
	CObj<NAI::IAIMap> pAIMap;
	CObj<NAI::IPathNetwork> pPathNetwork;
	CObj<NRPG::IGame> pRPGGame;
	CDBPtr<NDb::CAmbientLight> pDefaultLight;
	list< CObj<CUnitServer> > units;
	list< CObj<CObjectServerBase> > objects;
	list< CPtr<CObjectServerBase> > segmentObjects;
	list< CObj<IDynamicObject> > miscObjects;
	list< CObj<CBuilding> > buildings;
	CObj< CGlobalAck > pGlobalAck;
	CObj<CTerrainInfoHolder> pTerrainInfo;
	
	vector<NAI::SPathPlace> deploySpots;
	
	int nRootLayersGroup, nPartiesAdded;
	CTRect<float> sMapSafeZone;
	ZPARENT(CDebrisController)
	CObj<NAI::IAIJobManager> pAIJobManager;
	CObj<NScript::CScript> pOwnScript;
	CObj<NAI::IAISignalManager> pAISignalManager;
	int nAIUnitsCreated;
	CPtr<NRPG::CGlobalGame> pGlobalGame;
	CObj<CPlayer> pDeployedDeadUnitsPlayer;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TTBSWorld*)this); f.Add(2,&pShow); f.Add(3,&pShowUnits); f.Add(4,&eventHits); f.Add(5,&eventActions); f.Add(6,&pTerrain); f.Add(7,&pTime); f.Add(8,&pAimTime); f.Add(9,&tPrev); f.Add(10,&tHiddenDelta); f.Add(11,&pAIMap); f.Add(12,&pPathNetwork); f.Add(13,&pRPGGame); f.Add(14,&pDefaultLight); f.Add(15,&units); f.Add(16,&objects); f.Add(17,&segmentObjects); f.Add(18,&miscObjects); f.Add(19,&buildings); f.Add(20,&pGlobalAck); f.Add(21,&pTerrainInfo); f.Add(22,&deploySpots); f.Add(23,&nRootLayersGroup); f.Add(24,&nPartiesAdded); f.Add(25,&sMapSafeZone); f.Add(26,(CDebrisController*)this); f.Add(27,&pAIJobManager); f.Add(28,&pOwnScript); f.Add(29,&pAISignalManager); f.Add(30,&nAIUnitsCreated); f.Add(31,&pGlobalGame); f.Add(32,&pDeployedDeadUnitsPlayer); return 0; }
	
	void AddObject( const SObjectPlace &pos, bool bLightmap, 
		NDb::CObject *pO, NRPG::IObject *pRPG, bool bOpen, int nPassageZoneID, int nPassageObjectID, int nAPRadius );
	CUnitServer *AddUnit( const NAI::SPathPlace &aiPos, NRPG::IUnitMission *_pRPG, CPlayer *pPlayer );
	void AddBuilding( const SMapBuilding &info );
	void CreateFakeTerrainInfo( STerrainInfo *pTerrain );
	void SelectFindPathUnits( CUnit *pWho, list<CObjectBase*> *pRes );
	void CheckStability();
	CUnitServer* GetUnit( CUnit *pUnit ) const;
	void PlaceAllUnits();
	void StartGame();
	void CreateSoundStuff( vector<CObj<CTimedObject> > *stuff, CVec3 ptPos );
	
	void Segment();
	virtual void OnNewPlayerTurn( CPlayer *pPlayer );
	virtual void OnRealTimeStarted();
	// TBSWorld<>	
	virtual void ExecuteCommand( CCommand *_pCmd );
	virtual bool IsTBSRealTimeModePossible() const { return nPartiesAdded <= 1; }
	virtual void OnAction( bool bStartAction );
	bool TraceRay( const CRay &ray, int nMaxFloor, NWorld::IVisObj **ppUserData, int *pUserID, CVec3 *pPoint );
	bool IsPersonSlotUsed( int nUnitID, const ClueToSlot &clueToSlot, int *pPersID );
	CUnitServer *GetUnitServerByPersID( int nPersID );
	void DistributeClues( int nTemplateID, const SMapInfo &mapInfo,
		const list< CPtr<NScenario::CScenarioClue> > &clues,
		ClueToSlot *personClueToSlot,	ClueToSlot *itemClueToSlot );
	void PlaceItemSlotsToMap( const ClueToSlot &clueToSlot );
	void PlaceItemSlotsToInventory( const ClueToSlot &clueToSlot );
private:
	// CDebrisController
	virtual CActionCounter* CreateActionCounter() { return GetActiveCounter(); }
	virtual CSyncSrc<IVisObj>* GetShowList() { return pShow; }
	virtual void OnFrozenItemDestroyed( int nItemID );
	void GetPassageObjects( int nPassageZoneID, list< CPtr<CPassageObject> > *pPassageObjects );
	bool GetPassageDeployPlace( CPassageObject *pPassage, 
		CUnitServer *pUS, const vector<NAI::SPathPlace> &lockedPlaces, NAI::SPathPlace *pPathPlace );
	void InitPlayerCorpseCarrying( CPlayer *pPlayer );
	CUnitServer *GetDeployedDeadUnit( const NAI::SPathPlace &aiPos, NRPG::CUnit *pRPGUnit );

public:
	CWorld() {}
	CWorld( NRPG::CGlobalGame *_pGlobalGame );

	virtual void UpdateVisible();
	virtual CSyncSrc<IVisObj>* GetActive() const { return pShow; }
	virtual CSyncSrc<IVisObj>* GetUnits() const { return pShowUnits; }
	virtual bool GetAcknowledgement( IPlayer *pPlayer, vector<CPtr<CAckEvent> > *pEventsSet );
	virtual CHitLocator* GetHitEvent();
	virtual CActionLocator* GetActionEvent( IPlayer *pViewFrom  );
	//
	virtual CCTime* GetAimTime() const { return pAimTime; }
	virtual NRPG::IGame* GetGame() { return pRPGGame; }
	virtual NAI::IAIMap* GetAIMap() { return pAIMap; }
	virtual NAI::IPathNetwork* GetPathNetwork() { return pPathNetwork; }
	virtual CFuncBase<STerrainInfo>* GetTerrainInfo() const { return pTerrainInfo; }
//	virtual NTerrain::CTerrain* GetTerrain() const { return pTerrain; }
	virtual NDb::CAmbientLight* GetDefaultLight() { return pDefaultLight; }
	virtual void GetInterrupts( vector< CPtr<IPlayer> > *pInterrups ) const { TTBSWorld::GetInterrupts( pInterrups ); }
	virtual void CheckInterrupt( SInterruptInfo *info );
	virtual CGlobalAck *GetGlobalAck() const { return pGlobalAck; }	
	//
	virtual const CTRect<float>& CWorld::GetMapSafeZone() const;
	//
	virtual void CreateRandom( int nTemplateID, bool bBuildingStability, 
		const list< CPtr<NScenario::CScenarioClue> > &clues, int nMobsLevel,
		CObj<CPostWorldCreateInfo> *pPostInfo, SRandomSeed sSeed );
	virtual void RunPostInit( CPostWorldCreateInfo *pPostInfo );
	virtual void CreateDefault();
	virtual IPlayer* AddPlayer( const wstring &wsName, NRPG::CGlobalPlayer *pGlobalPlayer, CCommander *pCommander );
	virtual IPlayer* GetCurrentPlayer() const { return GetTBSCurrentPlayer(); }
	virtual bool IsUnitActive( CUnit *pTest ) const { return IsTBSUnitActive( GetUnit(pTest) ); }
	virtual void GetActiveUnits( IPlayer *pPlayer, list<CUnit*> *pRes );
	virtual bool IsValidPath( const NAI::CPath &path, CUnit *pWho );
	virtual bool IsFirstTurn() const { return CTBSWorld<CUnitServer, CPlayer>::IsFirstTurn(); }
	virtual bool IsInterrupt() const { return CTBSWorld<CUnitServer, CPlayer>::IsInterrupt(); }
	virtual bool TraceTile( const CRay &ray, NAI::SPosition *pRes, int nMaxFloor );
	virtual void TraceObjects( const CRay &ray, vector<IVisObj*> *pRes, int nMaxFloor );
	virtual void ClickOfDeath( const CRay &ray, int nMaxFloor );
	virtual CUnit* GetUnit( const NAI::SUnitPosition &pos );
	virtual CUnit* GetUnitInTile( const NAI::SUnitPosition &pos );
	virtual int  GetEnemyWatchers( IPlayer *pPlayer ) const;
	//
	virtual bool IsExecuting() const { return IsAction(); }
	virtual bool CanSeeAction( IPlayer *pPlayer );
	virtual void UpdateWorld( STime tScene, IPlayer *pPlayer );
	//
	void GetAllUnits( list<CPtr<CUnitServer> > *pRes );
	virtual void GetAllUnits( vector< CPtr<NWorld::CUnit> > *pUnits );
	void GetUnitsNear( const CVec3 &pos, list<CPtr<CUnitServer> > *pRes, float fRadius );
	
	CCTime* GetTime() const { return pTime; }
	NRPG::CCoverInfo* CalcCovers( const CVec3 &ptSrc, const NRPG::CAttackPortion &attack, CUnit *pIgnore, CUnit *pTarget, int nTargetUserID, float fMinClearDistance );
	NRPG::CCoverInfo* CalcCoversForTile( const CVec3 &src, const NRPG::CAttackPortion &attack, CUnit *pIgnore,
		const CVec3 &ptTarget, float fMinClearDistance );
	void PerformMeleeAttack( const NRPG::CAttackPortion &ap, const CRay &ray, const vector<NRPG::IAttackable*> &ignores );
	void PerformRangedAttack( const NRPG::CAttackPortion &ap, const CRay &ray, const vector<NRPG::IAttackable*> &ignores, STime sCast, NDb::CModel *pTrailModel, float fTrailSpeed );
	virtual void Explode( const CVec3 &ptEpicentre, int nPower );
	virtual void CreateParticle( const CVec3 &ptPos, const CQuat &rot, NDb::CEffect *pEffect );
	void AttachMiscObject( CTimedObject *p );
	void AddHitLocator( CHitLocator* pLocator );
	void AddActionLocator( CActionLocator* pLocator, STime tShowTime = 0 );
	void ThrowGrenade( const CVec3 &vFrom, const CVec3 &vSpeed, STime tThrow, float fTFly, 
		NDb::CModel *pModel, NDb::CRPGGrenade *pRPGGrenade, CUnitServer *pUnitServer );
	void ThrowKnife( const CVec3 &vFrom, const CVec3 &vSpeed, STime tThrow, float fDistance,
		NDb::CModel *pModel, NRPG::CAttackPortion &attack, NRPG::IInventoryItem *pIItem );
	void LaunchRocket( const CVec3 &vFrom, const CVec3 &vSpeed,
		STime tThrow, float fDistance, NDb::CModel *pModel, NRPG::CAttackPortion &attack, 
		NRPG::IClipItem *pRocket, CUnitServer *pIgnored, NDb::CEffect *_pEffect = 0 );
	virtual void AddGrenadeExplosion( const CVec3 &vStartPosition, 
		NDb::CRPGGrenade *pRPGGrenade, CUnitServer *pUnitServer = 0 );
	void KillObject( CObjectServerBase *pOS );
	void PrepareAllPaths( NAI::CMultiMovesTable *pTable, list<NAI::SPathPlace> *pResult, CUnit *pWho,
		const NAI::SPathPlace &ptSrc, int nPriceLimit, CUnit *pIgnore, bool bCheckSuicide );
	NAI::CPath* FindPath( CUnit *pWho, const NAI::SPathPlace &ptSrc, 
		const vector<NAI::SPathPlace> &ptDst, CUnit *pIgnore, bool bCheckSuicide = false, 
		NAI::EFindPathParams eParams = NAI::PF_DEFAULT, bool bStrafe = false, 
		bool bCanFindNotExactPath = false, bool bIgnoreAllUnits = false );
	void FindCloseGroundItems( CUnit *pU, vector<SItem> *pRes );
	bool IsWinnerPlayer( IPlayer *pPlayer );
	NRPG::EAction GetMoveActionType( const NAI::SUnitPosition &src, const NAI::SUnitPosition &dst, bool bCorpse );
	void GenerateDebris( NDb::CDebrisMaterial *pDebrisMaterial, const CVec3 &ptCenter, const CVec3 &ptDir, int nDebris );
	void MakeAISound( NDb::CAISound *pAISound, CDumbUnitServer *pWho, int nSoundType = 0, NDb::CSound *pSound = 0 );
	void MakeSound( const CVec3 &ptCenter, NDb::CSound *pSound );
	list< CObj<IDynamicObject> > *GetMiscObjects() { return &miscObjects; }
	NAI::IAIJobManager *GetAIJobManager() { return pAIJobManager; }
	NAI::IAISignalManager *GetAISignalManager() { return pAISignalManager; }
	CUnitServer *GetUnitServer( NRPG::IUnitMissionInfo *pUnitMission );
	CUnitServer *GetUnitServer( NRPG::CUnit *pRPGUnit );
	CUnitServer *GetUnitServer( string szName );
	void SetAudible( CUnitServer *pHearer, CUnitServer *pSource );
	virtual IPlayer* GetNextPlayerForScript( IPlayer *pPrev )
		// pPrev = 0 returns first; pPrev = last player returns 0. user only in script functions which must check all players
	{
		return GetNextPlayer( (CPlayer*)pPrev );
	}
	virtual void MakeExplosion( const CRay &ray, int nMaxFloor );
	NRPG::CGlobalGame *GetGlobalGame() const { return pGlobalGame; }
	bool UsePassageObject( CUnitServer *pUS, int nPassageZoneID );
	void InitCorpseCarrying();
	void GetScenarioPlayerUnits( int nScenarioPlayer, vector< CPtr<CUnitServer> > *pUnits );
};
extern CWorld *pCurrentWorld;
////////////////////////////////////////////////////////////////////////////////////////////////////
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif