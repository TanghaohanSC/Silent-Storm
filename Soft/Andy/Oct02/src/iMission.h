#ifndef __IMISSION_H__
#define __IMISSION_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
// CONST
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_FOV = 35;
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Time.h"
#include "iMain.h"
#include "Camera.h"
#include "Interface.h"
#include "wInterface.h"
#include "..\Misc\RandomGen.h"

namespace NGScene
{
	class ILight;
	class IGameView;
}
namespace NRPG
{
	class CGlobalGame;
}
namespace NRender
{
	class IRenderGame;
}
namespace NSound
{
	class ISoundScene;
}
namespace NAI
{
	struct SPosition;
}
namespace NUI
{
	class CMissionUI;
}
namespace NWorld
{
	class CUnit;
	class IPlayer;
	class IWorld;
	class CCommander;
	class CCmd;
	class CCommand;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// Render modes
const int
	N_RENDERMODE_2D	     = 1,
	N_RENDERMODE_3D	     = 2,
	N_RENDERMODE_SHOWALL = 4;
////////////////////////////////////////////////////////////////////////////////////////////////////
// State flags
const int
	N_UNITSTATE_DEFAULT					= 0x00000001,
	N_UNITSTATE_CANNON					= 0x00000002,
	N_UNITSTATE_SNIPE						= 0x00000003;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Actions
enum EUnitAction
{
	UA_DEFAULT,
	UA_STOP,
	UA_CONTINUE,
	//// moves
	UA_MOVE,
	UA_LOOK,
	//// actions
	UA_USE,
	UA_HEAL,
	UA_ATTACK,
	UA_DROPCORPSE,
	UA_WEAPONRELOAD,
	//// poses
	UA_POSERUN,
	UA_POSEWALK,
	UA_POSECROUCH,
	UA_POSECRAWL,
	//// snipe
	UA_SNIPE_ATTACK,
	UA_COLLECTAP_1AP,
	UA_COLLECTAP_10AP,
	UA_COLLECTAP_MAX,
	UA_COLLECTAP_ALL,
	////
	UA_MAXVALUE
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SActionInfo
{
	int nActionAP;

	bool bOk;
	bool bEnoughAP;
	bool bAvailable;
	NWorld::EUnitCommandResult eResult;

	SActionInfo(): nActionAP( -1 ), bOk( false ), bAvailable( false ), bEnoughAP( false ), eResult( NWorld::UCR_UNAVAILABLE ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IMission;
////////////////////////////////////////////////////////////////////////////////////////////////////
// IUnitTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
class IUnitTracker: public CObjectBase
{
public:
	virtual void SetTargetPosition( const NAI::SPosition &sPos, bool bInstantly = false ) = 0;
	virtual NAI::SPosition GetTargetPosition() const = 0;

	virtual bool IsPathComplete() const = 0;
	virtual void CancelPath() = 0;

	virtual bool IsStrafeMove() const = 0;
	virtual void SetStrafeMove( bool bState ) = 0;

	virtual bool IsActive() const = 0;
	virtual bool IsSelected() const = 0;

	virtual bool IsHilighted() const = 0;
	virtual void SetHilighted( bool bState ) = 0;

	virtual void Update() = 0;
	
	virtual NWorld::CUnit* GetUnit() const = 0;
	virtual NWorld::CUnit* GetNextEnemy() = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// IPlayerTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
class IPlayerTracker: public CObjectBase
{
public:
	virtual void GetUnits( vector< CPtr<IUnitTracker> > *pUnits ) const = 0;
	virtual void GetSelectedUnits( vector< CPtr<IUnitTracker> > *pUnits ) const = 0;

	virtual int CountSelected() = 0;
	virtual void Select( NWorld::CUnit *pUnit, bool bAdditive = false ) = 0;
	virtual void SelectNext() = 0;
	virtual void SelectPrev() = 0;

	virtual void Activate() = 0;
	virtual void Deactivate() = 0;
	
	virtual void Update( bool bActive ) = 0;

	virtual const ICamera::SCameraPos& GetCamera() const = 0;
	virtual void SetCamera( const ICamera::SCameraPos &sPosition ) = 0;

	virtual NWorld::IPlayer* GetPlayer() const = 0;
	virtual NWorld::CCommander* GetCommander() const = 0;
	virtual NRPG::CGlobalPlayer* GetGlobalPlayer() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// IState
////////////////////////////////////////////////////////////////////////////////////////////////////
class IState: public CObjectBase
{
public:
	enum EType
	{
		FORCED,
		UPDATED,
		INSTANT,
		TEMPORARY
	};

public:
	virtual bool Initialize( IMission *pGame ) = 0;
	virtual void Terminate() = 0;
	virtual bool ProcessEvent( const NInput::SEvent &sEvent ) = 0;
	virtual bool ProcessMessage( const NUI::SEvent &sEvent ) = 0;
	virtual void Step() = 0;

	virtual bool OnLButtonUp() = 0;
	virtual bool OnLButtonDown() = 0;
	virtual bool OnLButtonDblClk() = 0;
	virtual NUI::SCursorInfo GetCursorInfo() const = 0;

	virtual EType GetType() const = 0;
	virtual IMission* GetMission() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// IMission
////////////////////////////////////////////////////////////////////////////////////////////////////
class IMission: public NMainLoop::IInterfaceBase
{
public:
	virtual void Command( NWorld::CCommand *pCmd ) = 0;
	virtual void Command( NWorld::CUnit *pUnit, NWorld::CCmd *pCmd, bool bInstantly = true ) = 0;
	virtual void StopAction() = 0;
	virtual bool IsReady() const = 0;
	virtual bool IsActionExecuted() const = 0;
	virtual bool IsRealTime() const = 0;

	virtual void PauseGame( bool bState ) = 0;
	virtual bool IsGamePaused() const = 0;

	virtual IPlayerTracker* GetActivePlayer() const = 0;
	virtual void SetActivePlayer( IPlayerTracker *pPlayer ) = 0;
	virtual void GetPlayers( vector< CPtr<IPlayerTracker> > *pPlayersSet ) const = 0;

	virtual void GetUnits( vector< CPtr<IUnitTracker> > *pUnits ) const = 0;
	virtual void GetSelectedUnits( vector< CPtr<IUnitTracker> > *pUnits ) const = 0;

	virtual float GetGlobalVar( const string &szID, float fDefault = 0 ) const = 0;
	virtual void SetGlobalVar( const string &szID, float fValue ) = 0;

	virtual int CountSelected() = 0;
	virtual void Select( NWorld::CUnit *pUnit, bool bAdditive = false ) = 0;
	virtual void SelectNext() = 0;
	virtual void SelectPrev() = 0;

	virtual bool IsUpdated() const = 0;

	virtual IState* GetState() const = 0;
	virtual bool CommandState( IState *pState ) = 0;
	virtual void ResetState() = 0;
	virtual void SetUpdatedStates( const vector<CObj<IState> > &updatedStatesSet ) = 0;
	virtual CObjectBase* GetStateTarget() const = 0;
	virtual void SetStateTarget( CObjectBase* pObject ) = 0;

	virtual int GetUnitsState() = 0;
	virtual NWorld::CUnit::EState GetUnitsWorldState() = 0;
	virtual void GetActionInfo( EUnitAction eAction, SActionInfo *pInfo ) = 0;
	virtual void CanDoCommand( NWorld::CCmd *pCmd, bool bNoTarget, SActionInfo *pInfo ) = 0;
	virtual NWorld::EUnitCommandResult CanDoCommand( NWorld::CCmd *pCmd, bool bNoTarget = false, int *pnAP = 0 ) = 0;

	virtual ICamera* GetCamera() const = 0;
	virtual void GetCameraParams( ECameraType *pType, float *pFOV, ICamera::SCameraLimits *pLimits ) = 0;
	virtual void SetCameraParams( ECameraType eType, float fFOV, const ICamera::SCameraLimits &limits ) = 0;
	virtual void FreezeCamera( bool bState ) = 0;
	virtual void FocusCameraOnUnit( NWorld::CUnit *pUnit ) = 0;
	virtual const CVec3& GetCameraCP() const = 0;
	virtual const SHMatrix& GetCameraPos() const = 0;
	virtual const CTransformStack& GetCameraTransform() const = 0;

	virtual NWorld::IVisObj* GetTraceObject() const = 0;
	virtual CRay GetTraceRay() const = 0;
	virtual bool GetTracePosition( NAI::SPosition *pPos ) const = 0;

	virtual NUI::ICursor* GetCursor() const = 0;
	virtual NUI::CInterface* GetInterface() const = 0;
	virtual NUI::CMissionUI* GetMissionUI() const = 0;
	virtual bool IsInterfaceHidden() const = 0;

	virtual NRPG::CGlobalGame *GetRPGGame() const = 0;
	virtual NWorld::IWorld* GetWorld() const = 0;
	virtual NGScene::IGameView* GetScene() const = 0;
	virtual NSound::ISoundScene* GetSoundScene() const = 0;
	virtual NRender::IRenderGame* GetRenderGame() const = 0;

	virtual void Step() = 0;
	virtual bool ProcessEvent( const NInput::SEvent &sEvent ) = 0;
	virtual void RenderFrame( int nMode, const STime &sTime, ICamera *pCamera = 0, bool bShowUnits = true ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface commands
////////////////////////////////////////////////////////////////////////////////////////////////////
class CICBeginMission: public NMainLoop::CInterfaceCommand
{
	OBJECT_BASIC_METHODS(CICBeginMission);
protected:
	int nVariantID;
	CPtr<NRPG::CGlobalGame> pGlobalGame;

public:
	CICBeginMission() {}
	CICBeginMission( int _nVariantID, NRPG::CGlobalGame *_pGlobalGame );
	virtual void Exec();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CICMapEditBeginMission: public CICBeginMission
{
	OBJECT_BASIC_METHODS(CICMapEditBeginMission);
public:
	CICMapEditBeginMission() {}
	CICMapEditBeginMission( int nVariantID, NRPG::CGlobalGame *pGlobalGame );
	virtual void Exec();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetVariantForTemplate( int nTemplate, NRPG::CGlobalGame *pGlobalGame );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif