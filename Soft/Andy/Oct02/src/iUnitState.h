#ifndef __A5_I_UNITSTATE_H__
#define __A5_I_UNITSTATE_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
class CStateBase;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateNormal
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateTracker: public IStateTracker
{
	OBJECT_BASIC_METHODS(CStateTracker)
private:
	NInput::CBind bindExplode, bindCheatTeleport;
	//
	NInput::CBind bindCancel, bindNextEnemy;
	NInput::CBind bindMove, bindUse, bindAttack, bindFirstAid, bindDropCorpse, bindRotate, bindWeaponReload;
	NInput::CBind bindNormalPose, bindCrawlPose, bindCrouchPose, bindRunPose;

	typedef CObjectBase* (*TStateObjectCreate)();

	ZDATA
	CPtr<CMissionUI> pInterface;
	CPtr<NGame::IMissionGame> pGame;
	////
	EType eType;
	CObj<CStateBase> pState;
	TStateObjectCreate pStateCreator;
	////
	bool bUpdated;
	bool bSelectionUpdated;
	bool bStateUpdateBlocked;
	vector<SActionInfo> actionsInfoSet;
	//// information needed to track changes
	bool bRealTime;
	bool bHasCommands;
	bool bActionExecuted;
	CPtr<CObjectBase> pTraceObject;
	vector< CPtr<NGame::IUnitTracker> > selectedUnits;
	////
	CPtr<NWorld::IVisObj> pHilightedObject;
	CObj<NGScene::CRenderNode> pTileHilight;
	CObj<NGScene::CCFBTransform> pTileHilightPos;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pInterface); f.Add(3,&pGame); f.Add(4,&eType); f.Add(5,&pState); f.Add(6,&pStateCreator); f.Add(7,&bUpdated); f.Add(8,&bSelectionUpdated); f.Add(9,&bStateUpdateBlocked); f.Add(10,&actionsInfoSet); f.Add(11,&bRealTime); f.Add(12,&bHasCommands); f.Add(13,&bActionExecuted); f.Add(14,&pTraceObject); f.Add(15,&selectedUnits); f.Add(16,&pHilightedObject); f.Add(17,&pTileHilight); f.Add(18,&pTileHilightPos); return 0; }

public:
	CStateTracker();

	void Initialize( NGame::IMissionGame* pGame, CMissionUI *pInterface );

	void ShowTileHilight();
	void HideTileHilight();

	void NewPose( NAI::EPose ePos );
	void NewWeaponMode( NDb::EShootMode eMode );
	void SelectWeaponMode( int nInc );

	bool SetState( TStateObjectCreate pState, EType eType );
	void ResetState();
	void SetStateUpdateBlock( bool bMode ) { bStateUpdateBlocked = bMode; }

	void TrackChanges();
	void UpdateState();
	void UpdateGroupActionsInfo();

	bool IsUpdated() { return bUpdated; }
	bool IsSelectionUpdated() { return bSelectionUpdated; }

	void CanGroupDo( NWorld::CCmd *pCmd, bool bNoTarget, SActionInfo *pInfo );
	NWorld::EUnitCommandResult CanGroupDo( NWorld::CCmd *pCmd, bool bNoTarget = false, int *pnAP = 0 );

	int GetGroupState();
	EType GetStateType() const;
	NWorld::CUnit::EState GetGroupWorldState();
	const vector<SActionInfo>& GetGroupActionsInfo() const;

	CMissionUI* GetInterface() const { ASSERT( pInterface ); return pInterface; }
	NGame::IMissionGame* GetGame() const { ASSERT( pGame ); return pGame; }

	void Update();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateBase
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateBase: public CObjectBase
{
	OBJECT_BASIC_METHODS(CStateBase);
protected:
	ZDATA
	CPtr<CStateTracker> pTracker;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pTracker); return 0; }

public:
	virtual bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );

	virtual void OnLButtonUp() {}
	virtual void OnLButtonDown() {}
	virtual void OnLButtonDblClk() {}
	virtual void Step() {}

	CMissionUI* GetInterface() const;
	CStateTracker* GetTracker() const;
	NGame::IMissionGame* GetGame() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateDynamicAction
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateWait: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateWait);
private:
	ZDATA_(CStateBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); return 0; }
public:
	CStateWait() {}

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateEmpty
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateEmpty: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateEmpty);
private:
	ZDATA_(CStateBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); return 0; }

public:
	CStateEmpty() {}

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateTeam
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateTeam: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateTeam);
private:
	NInput::CBind bindModifier;
	ZDATA_(CStateBase)
	bool bModifier;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&bModifier); f.Add(3,&pTraceSelection); return 0; }

protected:
	void OnLButtonUp();

public:
	CStateTeam();

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
	void Step();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateMove
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateMove: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateMove);
private:
	NInput::CBind bindStrafe;
	ZDATA_(CStateBase)
	bool bSelection;
	CVec2 vSelectionAnchor;
	CObj<IImage> pSelectionZone;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&bSelection); f.Add(3,&vSelectionAnchor); f.Add(4,&pSelectionZone); return 0; }

protected:
	void UpdateCursor();

	void OnLButtonUp();
	void OnLButtonDown();
	void OnLButtonDblClk();

public:
	CStateMove();

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
	void Step();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateRotate
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateRotate: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateRotate);
private:
	ZDATA_(CStateBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); return 0; }

public:
	CStateRotate();

	void OnLButtonUp();

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateAttack
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateAttack: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateAttack);
private:
	NInput::CBind bindHitLocationHead, bindHitLocationBody, bindHitLocationLArm, bindHitLocationRArm, bindHitLocationLLeg, bindHitLocationRLeg;
	ZDATA_(CStateBase)
	int nActionAP;
	bool bActionUnavailable;
	wstring wsCursorText;
	NAI::EHitLocation eHitLocation;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&nActionAP); f.Add(3,&bActionUnavailable); f.Add(4,&wsCursorText); f.Add(5,&eHitLocation); f.Add(6,&pTraceSelection); return 0; }

protected:
	void UpdateCursor();
	void UpdateCursorInfo();
	void UpdateBlockedState();
	void UpdateTraceSelection();

public:
	CStateAttack();

	void OnLButtonUp();

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
	void Step();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateUse
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateUse: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateUse);
private:
	ZDATA_(CStateBase)
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&pTraceSelection); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();

public:
	CStateUse();

	void OnLButtonUp();

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateFirstAid
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateFirstAid: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateFirstAid);
private:
	ZDATA_(CStateBase)
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&pTraceSelection); return 0; }

protected:
	NWorld::CUnit* GetTarget();

public:
	CStateFirstAid();

	void OnLButtonUp();

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatePickItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStatePickItem: public CStateBase
{
	OBJECT_BASIC_METHODS(CStatePickItem);
private:
	ZDATA_(CStateBase)
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&pTraceSelection); return 0; }

public:
	CStatePickItem();

	void OnLButtonUp();

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateDropItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateDropItem: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateDropItem);
private:
	ZDATA_(CStateBase)
	CObj<NUI::IModel> pModel;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&pModel); return 0; }

public:
	CStateDropItem();

	void OnLButtonUp();

	bool Initialize( CStateTracker* pTracker, IStateTracker::EType eType );
	void Step();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
