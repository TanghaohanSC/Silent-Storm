#ifndef __IMISSION_INTERNAL_H__
#define __IMISSION_INTERNAL_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "..\Input\Bind.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
//////////////////////////////////////////////////////////////////////////////////////
// CVisibleTracker
//////////////////////////////////////////////////////////////////////////////////////
class CVisibleTracker: public CObjectBase
{
	OBJECT_BASIC_METHODS( CVisibleTracker );
private:
	ZDATA
	bool bUpdated;
	vector< CPtr<IUnitTracker> > selectedUnits;
	vector< CObj<NGScene::CRenderNode> > nodesSet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bUpdated); f.Add(3,&selectedUnits); f.Add(4,&nodesSet); return 0; }

public:
	CVisibleTracker() {}

	void Update( IMission* pMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CViewBuildingScheme
////////////////////////////////////////////////////////////////////////////////////////////////////
class CViewBuildingScheme: public CSyncDst<NWorld::IVisObj>, public NWorld::IRenderVisitor
{
	NGScene::IGameView *pScene;
	vector<CObj<CObjectBase> > *pRes;

	virtual void VisitObject( int nID, NWorld::IVisObj *p ) { if ( p ) p->Visit( this ); }
public:
	CViewBuildingScheme( CSyncSrc<NWorld::IVisObj> *pSrc, NGScene::IGameView *_pScene, vector<CObj<CObjectBase> > *_pRes );
	virtual void AddBuilding( const SMapBuilding &info );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUpdateBuildingStability
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUpdateBuildingStability: public CSyncDst<NWorld::IVisObj>, public NWorld::IRenderVisitor
{
	virtual void VisitObject( int nID, NWorld::IVisObj *p ) { if ( p ) p->Visit( this ); }
public:
	CUpdateBuildingStability( CSyncSrc<NWorld::IVisObj> *pSrc );
	virtual void AddBuilding( const SMapBuilding &info );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMission
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMission: public IMission
{
	OBJECT_BASIC_METHODS(CMission)
private:
	NInput::CBind bindStartOfTurn, bindEndOfTurn, bindNextEnemy; 
	NInput::CBind bindFocusUnit, bindAddFloor, bindSubFloor;

	NInput::CBind bindExit, bindCancel, bindContinue, bindPause, bindMainMenu, bindCluesMenu;
	NInput::CBind bindHero1, bindHero2, bindHero3, bindHero4, bindHero5, bindHero6, bindSelPrev, bindSelNext;
	NInput::CBind bindMove, bindAttack, bindFirstAid, bindDropCorpse, bindRotate;
	NInput::CBind bindNormalPose, bindCrawlPose, bindCrouchPose, bindRunPose, bindStrafe;
	NInput::CBind bindSnipeAttack, bindCollect1AP, bindCollect10AP, bindCollectMaxAP, bindCollectAllAP;
	NInput::CBind bindSnapShot, bindAimedShot, bindCarefulShot, bindShortBurst, bindLongBurst, bindSnipeShot, bindWeaponPrevMode, bindWeaponNextMode;
	NInput::CBind bindWeaponReload, bindPrevSlot, bindNextSlot, bindItemUnload;

	NInput::CBind bindSwitchLighting, bindHideInterface, bindSpecialHideInterface;
	NInput::CBind bindShadows, bindFog, bindHSR, bindShowParticles;
	NInput::CBind bindShowAI, bindExplode, bindCheatTeleport, bindRPGStats, bindShowSchema, bindCheckBuildings, bindShowVisibility, bindShowRAD;
	NInput::CBind bindTestIsect, bindClickOfDeath, bindExplosion;

	NGlobal::CCmd cmdTypeCamera, cmdSurrender;
	NGlobal::CVar varCheatGodMode, varCheatSeeAll, varCheatTeleport;
	NGlobal::CCmd cmdSetXPLevel;

	ZDATA
	int nTemplateID;
	CPtr<NRPG::CGlobalGame> pGlobalGame;
	//// graphics
	CObj<NGScene::IGameView> pScene;
	CObj<NSound::ISoundScene> pSoundScene;
	CObj<NRender::IRenderGame> pRender;
	CObj<NRender::IRenderSound> pRenderSound;
	//// world
	bool bPause;
	CObj<NWorld::IWorld> pWorld;
	//// players
	CPtr<IPlayerTracker> pActivePlayer;
	vector< CObj<IPlayerTracker> > playersSet;
	//// interface
	bool bHideInterface;
	bool bSpecialHideInterface;
	CObj<NUI::ICursor> pCursor;
	CObj<NUI::CInterface> pInterface;
	//// State
	bool bUpdated;
	CObj<IState> pState;
	CPtr<CObjectBase> pStateTarget;
	vector<CObj<IState> > updatedStatesSet;
	//// information needed to track changes
	bool bRealTime;
	bool bHasCommands;
	bool bActionExecuted;
	CPtr<CObjectBase> pTrackTarget;
	CPtr<CObjectBase> pPlayerInHand;
	vector< CPtr<NGame::IUnitTracker> > selectedUnits;
	//// Actions info
	vector<SActionInfo> actionsInfoSet;
	//// camera
	float fFOV;
	ECameraType eCameraType;
	ICamera::SCameraLimits cameraLimits;
	CObj<ICamera> pCamera;
	CPtr<IPlayerTracker> pCameraOwner;
	// freeze pose
	bool bFreezeCamera;
	ICamera::SCameraPos sFreezePose;
	// Camera result
	CVec3 vCameraCP;
	SHMatrix sCameraPos;
	CTransformStack sTransform;
	//// trace
	bool bTraceOk;
	CRay rTraceRay;
	NAI::SPosition sTraceTile;
	CPtr<NWorld::IVisObj> pTraceObject;
	//// camera control
	CVec3 vPlayerCamera;
	CPtr<NWorld::IPlayer> pLastActivePlayer;
	CPtr<NWorld::CActionLocator> pActionLocator;
	//// interface
	CObj<NUI::CMissionUI> pMissionUI;
	//// crap
	int nLightMode;
	CObj<NGScene::ILight> pLightSource;
	CObj<CVisibleTracker> pVisibleTracker;
	vector<CObj<CObjectBase> > buildingSchemas;
	CObj<NGScene::CRenderNode> pIntersectHolder;
	CObj<NGScene::CPolyline> pIntersectLineHolder;
	CVec3 vPrevCameraPosition;
	int nFramesSameCameraPosition;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nTemplateID); f.Add(3,&pGlobalGame); f.Add(4,&pScene); f.Add(5,&pSoundScene); f.Add(6,&pRender); f.Add(7,&pRenderSound); f.Add(8,&bPause); f.Add(9,&pWorld); f.Add(10,&pActivePlayer); f.Add(11,&playersSet); f.Add(12,&bHideInterface); f.Add(13,&pCursor); f.Add(14,&pInterface); f.Add(15,&bUpdated); f.Add(16,&pState); f.Add(17,&pStateTarget); f.Add(18,&updatedStatesSet); f.Add(19,&bRealTime); f.Add(20,&bHasCommands); f.Add(21,&bActionExecuted); f.Add(22,&pTrackTarget); f.Add(23,&pPlayerInHand); f.Add(24,&selectedUnits); f.Add(25,&actionsInfoSet); f.Add(26,&fFOV); f.Add(27,&eCameraType); f.Add(28,&cameraLimits); f.Add(29,&pCamera); f.Add(30,&pCameraOwner); f.Add(31,&bFreezeCamera); f.Add(32,&sFreezePose); f.Add(33,&vCameraCP); f.Add(34,&sCameraPos); f.Add(35,&sTransform); f.Add(36,&bTraceOk); f.Add(37,&rTraceRay); f.Add(38,&sTraceTile); f.Add(39,&pTraceObject); f.Add(40,&vPlayerCamera); f.Add(41,&pLastActivePlayer); f.Add(42,&pActionLocator); f.Add(43,&pMissionUI); f.Add(44,&nLightMode); f.Add(45,&pLightSource); f.Add(46,&pVisibleTracker); f.Add(47,&buildingSchemas); f.Add(48,&pIntersectHolder); f.Add(49,&pIntersectLineHolder); f.Add(50,&vPrevCameraPosition); f.Add(51,&nFramesSameCameraPosition); return 0; }

protected:
	bool TrackChanges();
	void UpdateState();
	void UpdateSound();
	void WeaponReload();
	void SelectSlot( int nInc );
	void NewWeaponMode( NDb::EShootMode eMode );
	void SelectWeaponMode( int nInc );
	void UpdateActionsInfo();
	void TraceCursor();
	void SetLightMode( int _nLightMode );
	void UnitCollectAP( NWorld::ECollectSnipeAP eAP );
	void UpdateLocators();

	void TestIntersection();
	void ClickOfDeath();
	void Explosion();

public:
	CMission();

	void Initialize( int nTemplateID, NRPG::CGlobalGame *pGlobalGame );

	void Command( NWorld::CCommand *pCmd );
	void Command( NWorld::CUnit *pUnit, NWorld::CCmd *pCmd, bool bInstantly = true );
	void StopAction();
	bool IsReady() const;
	bool IsActionExecuted() const;
	bool IsRealTime() const;

	void PauseGame( bool bState );
	bool IsGamePaused() const;

	IPlayerTracker* GetActivePlayer() const;
	void SetActivePlayer( IPlayerTracker *pPlayer );
	void GetPlayers( vector< CPtr<IPlayerTracker> > *pPlayersSet ) const;

	void GetUnits( vector< CPtr<IUnitTracker> > *pUnits ) const;
	void GetSelectedUnits( vector< CPtr<IUnitTracker> > *pUnits ) const;

	float GetGlobalVar( const string &szID, float fDefault = 0 ) const;
	void SetGlobalVar( const string &szID, float fValue );

	int CountSelected();
	void Select( NWorld::CUnit *pUnit, bool bAdditive );
	void SelectNext();
	void SelectPrev();

	bool IsUpdated() const;

	IState* GetState() const;
	bool CommandState( IState *pState );
	void ResetState();
	void SetUpdatedStates( const vector<CObj<IState> > &updatedStatesSet );
	CObjectBase* GetStateTarget() const;
	void SetStateTarget( CObjectBase* pObject );

	int GetUnitsState();
	NWorld::CUnit::EState GetUnitsWorldState();
	void GetActionInfo( EUnitAction eAction, SActionInfo *pInfo );
	void CanDoCommand( NWorld::CCmd *pCmd, bool bNoTarget, SActionInfo *pInfo );
	NWorld::EUnitCommandResult CanDoCommand( NWorld::CCmd *pCmd, bool bNoTarget = false, int *pnAP = 0 );

	ICamera* GetCamera() const;
	float GetCameraFOV() const;
	void GetCameraParams( ECameraType *pType, float *pFOV, ICamera::SCameraLimits *pLimits );
	void SetCameraParams( ECameraType eType, float fFOV, const ICamera::SCameraLimits &limits );
	void FreezeCamera( bool bState );
	void FocusCameraOnUnit( NWorld::CUnit *pUnit );
	const CVec3& GetCameraCP() const;
	const SHMatrix& GetCameraPos() const;
	const CTransformStack& GetCameraTransform() const;

	NWorld::IVisObj* GetTraceObject() const;
	bool GetTracePosition( NAI::SPosition *pPos ) const;
	virtual CRay GetTraceRay() const;

	NUI::ICursor* GetCursor() const;
	NUI::CInterface* GetInterface() const;
	NUI::CMissionUI* GetMissionUI() const;
	bool IsInterfaceHidden() const;

	NRPG::CGlobalGame *GetRPGGame() const;
	NWorld::IWorld* GetWorld() const;
	NGScene::IGameView* GetScene() const;
	NSound::ISoundScene* GetSoundScene() const;
	NRender::IRenderGame* GetRenderGame() const;

	void Step();
	void OnGetFocus();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void RenderFrame( int nMode, const STime &sTime, ICamera *pCamera = 0, bool bShowUnits = true );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
