#ifndef __WINTERFACE_H_
#define __WINTERFACE_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\FileIO\BasicDB.h"
#include "DG.h"
#include "GSkeleton.h"
#include "Time.h"
#include "Sync.h"
#include "wTSFlags.h"
#include "wUnitCommands.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCTime;
struct SRandomSeed;
class CTerrainPart;
namespace NRPG
{
	class CGlobalGame;
	class CGlobalPlayer;
	struct SUnitInfo;
	class CMerc;
	class IGame;
	class IInventoryItem;
	class IUnitMissionInfo;
	class CGlobalDiplomacy;
}
namespace NDb
{
	class CModel;
	class CEffect;
	class CRPGArmor;
	class CTexture;
	class CSound;
	class CAmbientLight;
	class CSkeleton;
	class CAIGeometry;
	class CDBAckInfo;
	class CDBAckSequence;
	class CComplexHead;
}
namespace NWorld
{
	class CUnit;
	class CPlayer;
	class CGlobalAck;
	enum ETBSEvent;
	class CUnitServer;
}
namespace NAI
{
	class CPath;
	class IAIMap;
	class IPathNetwork;
}
namespace NAnimation
{
	struct SSkeletonState;
}
namespace NGScene
{
	class CExplosionInfo;
}
namespace NBuilding
{
	class CBuildingInfoHold;
}
namespace NGScene
{
	class CLightGroup;
	struct SRoomInfo;
}
namespace NScenario
{
	class CScenarioClue;
}
class CMemObject;
struct STerrainInfo;
struct SMapBuilding;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int 
	CHEAT_GODMODE		= 0x00000001,
	CHEAT_SEEALL		= 0x00000002,
	CHEAT_TELEPORT	= 0x00000004;
////////////////////////////////////////////////////////////////////////////////////////////////////
// when someone holds CObj on such object then action is in progress
class CActionCounter: public CObjectBase
{
	OBJECT_BASIC_METHODS(CActionCounter);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCommand: public CObjectBase
{
public:
	virtual bool IsSkippable() const { return true; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdCheat: public CCommand
{
	OBJECT_BASIC_METHODS(CCmdCheat);
public:
	ZDATA
	int nCheatMask;
	bool bState;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nCheatMask); f.Add(3,&bState); return 0; }

public:
	CCmdCheat() {}
	CCmdCheat( int _nCheatMask, bool _bState ): nCheatMask( _nCheatMask ), bState( _bState ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdEndOfTurn: public CCommand
{
	OBJECT_BASIC_METHODS(CCmdEndOfTurn);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// not implemented yet
class CCmdQuitGame: public CCommand
{
	OBJECT_BASIC_METHODS(CCmdQuitGame);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCommander: public CObjectBase
{
	OBJECT_BASIC_METHODS(CCommander);
	ZDATA
	list< CObj<CCommand> > cmds;
	bool bInterruptRequest, bStopAction;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&cmds); f.Add(3,&bInterruptRequest); f.Add(4,&bStopAction); return 0; }
	CCommander(): bInterruptRequest(false), bStopAction(false) {}
	bool IsRequestCancel() const { return bStopAction; }
	virtual bool IsRequestInterrupt() const { return bInterruptRequest; }
	void ClearRequests() { bInterruptRequest = false; bStopAction = false; }
	void Do( CCommand *pCmd ) { if ( !pCmd->IsSkippable() ) bInterruptRequest = true; cmds.push_back( pCmd ); }
	void ClearList() { cmds.clear(); }
	bool HasCommands() const { return !cmds.empty(); }
	void StopAction() { ClearList(); bStopAction = true; }
	CCommand* GetCommand()
	{
		if ( cmds.empty() )
			GenerateCommand();
		if ( cmds.empty() )
			return 0;
		CCommand *pRes = cmds.front().Extract();
		cmds.pop_front();
		return pRes;
	}
	virtual void GenerateCommand() {}
	virtual void OnTBSEvent( ETBSEvent event ) {}
	virtual void OnPassControl( CPlayer *pPlayer ) {}
	virtual void OnUnitWasKilled( CUnitServer *pUnit ) {}
	virtual void OnSeeUnit( CUnitServer *pWatcher, CUnitServer *pTarget ) {}
	virtual void Segment() {}
	virtual void ProcessAISignals() {}
	virtual void OnUnitAdded( CUnitServer *pUnit ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnit;
struct IRenderVisitor
{
	struct SBoundMesh
	{
		NDb::CModel *pModel;
		const char *pszBindBone;

		SBoundMesh() {}
		SBoundMesh( NDb::CModel *_pModel, const char *_pszBindBone ): pModel(_pModel), pszBindBone(_pszBindBone) {}
	};
	virtual NGScene::CLightGroup* MakeGroup() { return 0; }
	virtual void AddParticleEffect( STime tBegin, NDb::CEffect *pEffect, CFuncBase<SFBTransform> *pPosition ) {}
	virtual void AddPointLight( const CVec3 &ptColor, const CVec3 &ptOrigin, float fRadius, bool bLightmapOnly ) {}
	virtual void AddSpotLight( const CVec3 &ptColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, float fRadius, NDb::CTexture *pMask, bool bLightmapOnly ) {}
	virtual void AddMesh( NDb::CModel *pModel, const SFBTransform &position, NGScene::CLightGroup *pGroup, int nFloor ) {}
	virtual void AddMesh( CMemObject *pModel, const CVec4 &color, const SFBTransform &position ) {}
	virtual void AddPolyline( const vector<CVec3> &points, const CVec3 &cr ) {}
	virtual void AddItemMesh( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation ) {}
	virtual void AddMesh( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation, CFuncBase<NAnimation::SSkeletonState> *pState, const vector<SBoundMesh> &boundMeshes, NGScene::CLightGroup *pGroup, int nFloor, CUnit *pUnit = 0 ) {}
	virtual void AddBuilding( const SMapBuilding &info ) {}
	virtual void AddBuildingPart( int nPartID, const SMapBuilding &info, NBuilding::CBuildingInfoHold *pBI ) {}
	virtual void AddTerrainParts( const SRandomSeed &sSeed, const CTRect<int> &sRegion, const list<CObj<CPtrFuncBase<CTerrainPart> > > &partsList, CFuncBase<STerrainInfo> *pInfo, CVersioningBase *pUpdateRegion ) {}
	virtual void AddTerrainWallPart( CPtrFuncBase<CTerrainPart> *pPart, NDb::CTexture *pTexture ) {}
	virtual void AddGrass( CFuncBase<STerrainInfo> *pInfo ) {}
	virtual void AddGrassEvent( const CVec3 &ptPlace ) {}
	virtual void AddExplosion( NDb::CEffect *pEffect, CFuncBase<NGScene::CExplosionInfo> *pExplosion, const CVec3 &pos ) {}
	virtual void AddHead( CUnit *pUnit, CFuncBase<SFBTransform> *pPosition, const NGScene::SRoomInfo &room ) {}
	virtual void AddHead( NDb::CComplexHead *pHead, CFuncBase<SFBTransform> *pPosition, const NGScene::SRoomInfo &room ) {}
	virtual void AddOccluder( NDb::CAIGeometry *pAIGeom, const SFBTransform &pos ) {}
	virtual void AddOccluder( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, CFuncBase<NAnimation::SSkeletonPose> *pAnimation ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct IAIVisitor
{
	struct SPieceMap
	{
		int nPieceID, nUserID;
	};
	virtual void AddHull( NDb::CAIGeometry *pAIGeom, 
		const SFBTransform &pos, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask ) = 0;
	virtual void AddHull( CMemObject *pAIGeom, 
		const SFBTransform &pos, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask, int nUserID ) = 0;
	virtual void AddAnimatedHull( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, 
		CFuncBase<NAnimation::SSkeletonPose> *pAnimation, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask ) = 0;
	virtual void AddFlippingHull( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, 
		CFuncBase<NAnimation::SSkeletonPose> *pAn1, CFuncBase<NAnimation::SSkeletonPose> *pAn2, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask, bool bOpen ) = 0;
	virtual void AddPieces( NDb::CAIGeometry *pAIGeom, const vector<SPieceMap> &parts,
		const SFBTransform &pos, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask ) = 0;
	virtual void AddTerrainPart( CPtrFuncBase<CTerrainPart> *pPart, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct ISoundVisitor
{
	virtual void Add3DSound( STime tStart, NDb::CSound *pSound, CFuncBase<CVec3> *pPosition ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct IVisObj: public CObjectBase
{
	virtual void Visit( IRenderVisitor* ) {}
	virtual void Visit( IAIVisitor* ) {}
	virtual void Visit( ISoundVisitor* ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IItem: virtual public IVisObj
{
public:
	virtual CVec3 GetPos() const = 0;
	virtual NRPG::IInventoryItem* GetInvItem() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IObject: virtual public IVisObj
{
public:
	virtual bool IsTargetable() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IPassageObject
{
public:
	virtual bool IsBroken() const = 0;
	virtual bool UsePassageObject( CUnitServer *pUS ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IWindowDoor
{
public:
	virtual bool IsBroken() const = 0;
	virtual bool IsOpen() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnit;
class ICannon
{
public:
	virtual bool IsBroken() const = 0;
	virtual bool IsOccupied() const = 0;
	virtual CUnit* GetCurrentUnit() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IBuilding: virtual public IVisObj
{
public:
	virtual const SMapBuilding& GetInfo() const = 0;
	virtual void UpdateAllParts() = 0;
	virtual void Update() = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class ITerrain: virtual public IVisObj
{
public:
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// IPathViewer
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SPathPoint
{
	ZDATA
	int nAP;
	int nFloor;
	CVec3 vPoint;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nAP); f.Add(3,&nFloor); f.Add(4,&vPoint); return 0; }

	SPathPoint() {}
	SPathPoint( int _nAP, int _nFloor, CVec3 _vPoint ): nAP( _nAP ), nFloor( _nFloor ), vPoint( _vPoint ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IPathViewer: public CObjectBase
{
public:
	virtual void SetPath( NAI::CPath *pPath ) = 0;

	virtual int GetResult() const = 0;
	virtual void GetPoints( vector<SPathPoint> *pRes ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IPlayer;
class CUnit: virtual public IVisObj
{
public:
	enum EState
	{
		ST_NORMAL_DEFAULT,
		ST_NORMAL_PISTOL,
		ST_NORMAL_RIFLE,
		ST_NORMAL_SUB_MACHINE_GUN,
		ST_NORMAL_KNIFE,
		ST_NORMAL_HAND_MACHINE_GUN,
		ST_NORMAL_RLAUNCHER,
		ST_MACHINE_GUN,
		ST_CARRY_CORPSE,
		ST_NORMAL_MEDKIT,
		ST_NORMAL_MELEE,
		ST_NORMAL_GRENADE,
		ST_HEALER,
		ST_SNIPE
	};
	virtual bool IsMoving() const = 0;
	virtual bool IsDead() const = 0;
	virtual bool IsUnconscious() const = 0;
	virtual bool IsStrafing() const = 0;
	virtual bool IsCarryingCorpse() const = 0;
	virtual CUnit* GetCorpseCarrier() const = 0;
	virtual NDb::CModel* GetModel() const = 0;
	virtual void GetVisible( vector<CPtr<CUnit> > *pTarget ) const = 0;
	virtual void GetInfo( NRPG::SUnitInfo *pInfo ) const = 0;
	virtual IPlayer* GetPlayer() const = 0;
	virtual NAI::CPath* GetCurrentPath() = 0;
	virtual IPathViewer* CreatePathViewer() = 0;
	virtual NRPG::IUnitMissionInfo* GetRPG() const = 0;
	virtual const NAI::SUnitPosition& GetPosition() const = 0;
	virtual void GetRealPosition( CVec3 *pRes ) = 0;
	virtual void AddVisitableChildren( vector<IVisObj*> *pRes ) = 0;
	virtual bool GetCurrentCommandName( string *pName ) const = 0;
	virtual CVec3 GetAttackOrigin() const = 0;
	virtual CVec3 GetAttackOrigin( const NAI::SUnitPosition &from ) const = 0;
	virtual float GetMinClearDistance() const = 0;
	virtual const CObjectBase* GetAttackIgnore() const = 0;
	virtual EUnitCommandResult CanDo( CCmd *p, int *pnStartAP = 0, int *pnFullAP = 0 ) = 0; // destroys CCmd if it has zero references
	virtual bool HasEnoughAP() = 0; // to start execution of current command, return false if no command is set
	virtual EState GetState() = 0;
	virtual NDb::CComplexHead* GetDBHead() = 0;
	virtual bool IsCapPresent() = 0;
	virtual int GetCarefulShotExtraAP() = 0;
	////
	NAI::EPose GetPose() const { return GetPosition().GetPose(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAckEvent
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAckEvent: public CObjectBase
{
	OBJECT_BASIC_METHODS(CAckEvent);
public:
	ZDATA
	int nPriority; // приоритет
	CPtr<NWorld::CUnit> pUnit; // кто выполняет
	CDBPtr<NDb::CDBAckInfo> pAckInfo; // ack
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nPriority); f.Add(3,&pUnit); f.Add(4,&pAckInfo); return 0; }

	CAckEvent() {}
	CAckEvent( int _nPriority, NWorld::CUnit *_pUnit, NDb::CDBAckInfo *_pAckInfo ): nPriority( _nPriority ), pUnit( _pUnit ), pAckInfo( _pAckInfo ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CActionLocator: public CObjectBase
{
	OBJECT_BASIC_METHODS(CActionLocator);
public:
	enum EType
	{
		TYPE_TURN				= 0,
		TYPE_DIE				= 1,
		TYPE_INTERRUPT	= 2,
		TYPE_UNIT				= 3,
		TYPE_WORLD			= 4,
		TYPE_OBJECT			= 5
	};

	ZDATA
	EType eType;
	STime tTime;
	CVec3 vPosition;
	CPtr<CUnit> pUnit;
	CPtr<IObject> pObject;
	CPtr<IPlayer> pPlayer;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&eType); f.Add(3,&tTime); f.Add(4,&vPosition); f.Add(5,&pUnit); f.Add(6,&pObject); f.Add(7,&pPlayer); return 0; }

	CActionLocator(): eType( TYPE_WORLD ), tTime( 0 ) {}
	CActionLocator( EType _eType ): eType( _eType ), tTime( 0 ) {}
	CActionLocator( EType _eType, IPlayer* _pPlayer ): eType( _eType ), tTime( 0 ), pPlayer( _pPlayer ) {}
	CActionLocator( EType _eType, CUnit* _pUnit, const CVec3 &_vPosition, const STime &_tTime = 0 ): eType( _eType ), tTime( _tTime ), pUnit( _pUnit ), vPosition( _vPosition ) {}
	CActionLocator( EType _eType, IObject* _pObject, const CVec3 &_vPosition, const STime &_tTime = 0 ): eType( _eType ), tTime( _tTime ), pObject( _pObject ), vPosition( _vPosition ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHitLocator: public CObjectBase
{
	OBJECT_BASIC_METHODS(CHitLocator);
public:
	ZDATA
	int nHitValue;
	CVec3 vPosition;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nHitValue); f.Add(3,&vPosition); return 0; }

	CHitLocator(): nHitValue( 0 ), vPosition( 0, 0, 0 ) {}
	CHitLocator( int _nHitValue, const CVec3 &_vPosition ): nHitValue( _nHitValue ), vPosition( _vPosition ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdUnit: public CCommand
{
public:
	ZDATA
	CPtr<CUnit> pUnit;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pUnit); return 0; }
	CCmdUnit() {}
	CCmdUnit( CUnit *_pUnit ): pUnit(_pUnit) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdCancel: public CCmdUnit
{
	OBJECT_BASIC_METHODS(CCmdCancel);
public:
	CCmdCancel() {}
	CCmdCancel( CUnit *_pUnit ): CCmdUnit(_pUnit) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCmdSetCommand: public CCmdUnit
{
	OBJECT_BASIC_METHODS(CCmdSetCommand);
	ZDATA_(CCmdUnit)
	CObj<CCmd> pCmd;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CCmdUnit*)this); f.Add(2,&pCmd); return 0; }
public:
	CCmdSetCommand() {}
	CCmdSetCommand( CUnit *_pUnit, CCmd *_p ): CCmdUnit(_pUnit), pCmd(_p) {}
	CCmd* GetCmd() const { return pCmd; }
	bool IsSkippable() const { return pCmd->IsSkippable(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IPlayer: public CObjectBase
{
public:
	typedef vector< CPtr<CUnit> > CUnitSet;
	struct SItemInfo
	{
		ZDATA
		CPtr<CUnit> pUnit;
		CPtr<NRPG::IInventoryItem> pItem;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pUnit); f.Add(3,&pItem); return 0; }
	};

	virtual bool IsCheatEnabled( int nCheat ) const = 0;
	virtual void GetSounds( vector<IVisObj*> *pRes ) = 0;
	virtual void GetUnits( CUnitSet *pRes ) const = 0;
	virtual void GetVisible( list< CPtr<CUnit> > *pRes ) const = 0;
	virtual CCommander *GetCommander() = 0;	
	virtual const wstring& GetPlayerName() const = 0;
	virtual NRPG::CGlobalPlayer* GetGlobalPlayer() const = 0;
	virtual void GetDeploySpot( NAI::SPathPlace *pRes ) = 0;
	virtual bool GetInHandItem( SItemInfo *pInfo ) const = 0;
	bool IsControlling( CUnit *pUnit )
	{
		CUnitSet u;
		GetUnits( &u );
		return find( u.begin(), u.end(), pUnit ) != u.end();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPostWorldCreateInfo;
class IWorld: public CObjectBase
{
public:
	virtual CSyncSrc<IVisObj>* GetActive() const = 0;
	virtual CSyncSrc<IVisObj>* GetUnits() const = 0;
	virtual bool GetAcknowledgement( IPlayer *pPlayer, vector<CPtr<CAckEvent> > *pEventsSet ) = 0;	
	virtual CHitLocator* GetHitEvent() = 0;
	virtual CActionLocator* GetActionEvent( IPlayer *pViewFrom ) = 0;
	//
	virtual CCTime* GetAimTime() const = 0;
	virtual NRPG::IGame* GetGame() = 0;
	virtual NAI::IAIMap* GetAIMap() = 0;
	virtual NAI::IPathNetwork* GetPathNetwork() = 0;
	virtual CFuncBase<STerrainInfo>* GetTerrainInfo() const = 0;
	virtual NDb::CAmbientLight* GetDefaultLight() = 0;
	virtual void GetInterrupts( vector< CPtr<IPlayer> > *pInterrups ) const = 0;
	virtual CGlobalAck *GetGlobalAck() const = 0;
	//
	virtual const CTRect<float>& GetMapSafeZone() const = 0;
	//
	virtual void CreateRandom( int nTemplateID, bool bBuildingStability, 
		const list< CPtr<NScenario::CScenarioClue> > &clues, int nMobsLevel,
		CObj<CPostWorldCreateInfo> *pPostInfo, SRandomSeed sSeed ) = 0;
	virtual void RunPostInit( CPostWorldCreateInfo *pPostInfo ) = 0;
	virtual void CreateDefault() = 0;
	virtual IPlayer* AddPlayer( const wstring &wsName, NRPG::CGlobalPlayer *pGlobalPlayer, CCommander *pCommander ) = 0;
	virtual IPlayer* GetCurrentPlayer() const = 0;
	virtual bool IsUnitActive( CUnit *pTest ) const = 0;
	virtual void GetActiveUnits( IPlayer *pPlayer, list<CUnit*> *pRes ) = 0;
	virtual bool IsValidPath( const NAI::CPath &path, CUnit *pWho ) = 0;
	virtual bool IsFirstTurn() const = 0;
	virtual bool IsInterrupt() const = 0;
	virtual bool TraceTile( const CRay &ray, NAI::SPosition *pRes, int nMaxFloor = 100 ) = 0;
	virtual void TraceObjects( const CRay &ray, vector<IVisObj*> *pRes, int nMaxFloor = 100 ) = 0;
	virtual void ClickOfDeath( const CRay &ray, int nMaxFloor ) = 0;
	virtual CUnit* GetUnit( const NAI::SUnitPosition &pos ) = 0;
	virtual CUnit* GetUnitInTile( const NAI::SUnitPosition &pos ) = 0;
	virtual void FindCloseGroundItems( CUnit *pUnit, vector<SItem> *pRes ) = 0;
	virtual bool IsWinnerPlayer( IPlayer *pPlayer ) = 0;
	virtual int  GetEnemyWatchers( IPlayer *pPlayer ) const = 0;
	//
	virtual bool IsExecuting() const = 0;
	virtual bool CanSeeAction( IPlayer *pPlayer ) = 0;
	virtual void UpdateWorld( STime tScene, IPlayer *pPlayer ) = 0;
	virtual IPlayer* GetNextPlayerForScript( IPlayer *pPrev ) = 0;// pPrev = 0 returns first; pPrev = last player returns 0.
	virtual void MakeExplosion( const CRay &ray, int nMaxFloor ) = 0;
	virtual void InitCorpseCarrying() = 0;
	virtual void GetAllUnits( vector< CPtr<NWorld::CUnit> > *pUnits ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorldSyncSrc: public CSyncSrc<IVisObj>
{
	OBJECT_BASIC_METHODS(CWorldSyncSrc);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IWorld* CreateWorld( NRPG::CGlobalGame *_pGlobalGame );
////////////////////////////////////////////////////////////////////////////////////////////////////
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif