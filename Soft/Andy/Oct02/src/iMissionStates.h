#ifndef __I_MISSIONSTATES_H__
#define __I_MISSIONSTATES_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iGameStates.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMissionState
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMissionState: public CStateBase
{
private:
	ZDATA_(CStateBase)
	CPtr<IGlobalGame> pMissionGame;
	CPtr<NUI::CMissionUI> pMissionUI;
	////
	CPtr<NWorld::IVisObj> pTarget;
	////
	bool bShowTileHilight;
	CObj<NGScene::CRenderNode> pTileHilight;
	CObj<NGScene::CCFBTransform> pTileHilightPos;
	CObj<NGScene::CLightGroup> pTileHilightLG;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&pMissionGame); f.Add(3,&pMissionUI); f.Add(4,&pTarget); f.Add(5,&bShowTileHilight); f.Add(6,&pTileHilight); f.Add(7,&pTileHilightPos); f.Add(8,&pTileHilightLG); return 0; }

public:
	CMissionState() {}
	CMissionState( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI );

	void Terminate();

	void ShowTileHilight();
	void HideTileHilight();

	void Step();

	IGlobalGame* GetMissionGame() const;
	NUI::CMissionUI* GetInterface() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// UPDATED STATES
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateWait
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateWait: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateWait)
private:
	ZDATA_(CStateBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); return 0; }

public:
	bool Initialize( IGlobalGame *pGame );

	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return UPDATED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateTeam
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateTeam: public CMissionState
{
	OBJECT_BASIC_METHODS(CStateTeam);
private:
	NInput::CBind bindModifier;
	ZDATA_(CMissionState)
	bool bModifier;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CMissionState*)this); f.Add(2,&bModifier); f.Add(3,&pTraceSelection); return 0; }

public:
	CStateTeam();
	CStateTeam( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI );

	bool Initialize( IGlobalGame *pGame );
	void Terminate();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void Step();

	bool OnLButtonUp();

	EType GetType() const { return UPDATED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateMove
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateMove: public CMissionState
{
	OBJECT_BASIC_METHODS(CStateMove);
private:
	NInput::CBind bindStrafe;
	ZDATA_(CMissionState)
	bool bForced;
	bool bAnchorSet;
	CVec2 vAnchor;
	NUI::SCursorInfo sCursorInfo;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CMissionState*)this); f.Add(2,&bForced); f.Add(3,&bAnchorSet); f.Add(4,&vAnchor); f.Add(5,&sCursorInfo); return 0; }

protected:
	void DoMove( bool bInstant );
	void UpdateCursor();

public:
	CStateMove();
	CStateMove( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI, bool bForced );

	bool Initialize( IGlobalGame *pGame );
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void Step();

	bool OnLButtonUp();
	bool OnLButtonDown();
	bool OnLButtonDblClk();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateAttack
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateAttack: public CMissionState
{
	OBJECT_BASIC_METHODS(CStateAttack);
private:
	NInput::CBind bindHitLocationHead, bindHitLocationBody, bindHitLocationLArm, bindHitLocationRArm, bindHitLocationLLeg, bindHitLocationRLeg;
	ZDATA_(CMissionState)
	int nActionAP;
	bool bForced;
	bool bActionUnavailable;
	NUI::SCursorInfo sCursorInfo;
	NAI::EHitLocation eHitLocation;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CMissionState*)this); f.Add(2,&nActionAP); f.Add(3,&bForced); f.Add(4,&bActionUnavailable); f.Add(5,&sCursorInfo); f.Add(6,&eHitLocation); f.Add(7,&pTraceSelection); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();
	void UpdateCursor();
	void UpdateCursorInfo();
	void UpdateBlockedState();
	void UpdateTraceSelection();

public:
	CStateAttack();
	CStateAttack( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI, bool bForced );

	bool Initialize( IGlobalGame *pGame );
	void Terminate();
	void Step();

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateUse
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateUse: public CMissionState
{
	OBJECT_BASIC_METHODS(CStateUse);
private:
	ZDATA_(CMissionState)
	NUI::SCursorInfo sCursorInfo;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CMissionState*)this); f.Add(2,&sCursorInfo); f.Add(3,&pTraceSelection); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();

public:
	CStateUse() {}
	CStateUse( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI ): CMissionState( pMissionGame, pMissionUI ) {}

	bool Initialize( IGlobalGame *pGame );
	void Terminate();

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return UPDATED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatePickItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStatePickItem: public CMissionState
{
	OBJECT_BASIC_METHODS(CStatePickItem);
private:
	ZDATA_(CMissionState)
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CMissionState*)this); f.Add(2,&pTraceSelection); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();

public:
	CStatePickItem() {}
	CStatePickItem( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI ): CMissionState( pMissionGame, pMissionUI ) {}

	bool Initialize( IGlobalGame *pGame );
	void Terminate();

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return UPDATED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// FORCED STATES
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateRotate
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateRotate: public CMissionState
{
	OBJECT_BASIC_METHODS(CStateRotate);
private:
	ZDATA_(CMissionState)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CMissionState*)this); return 0; }

public:
	CStateRotate() {}
	CStateRotate( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI ): CMissionState( pMissionGame, pMissionUI ) {}

	bool Initialize( IGlobalGame *pGame );

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return FORCED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateFirstAid
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateFirstAid: public CMissionState
{
	OBJECT_BASIC_METHODS(CStateFirstAid);
private:
	ZDATA_(CMissionState)
	NUI::SCursorInfo sCursorInfo;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CMissionState*)this); f.Add(2,&sCursorInfo); f.Add(3,&pTraceSelection); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();

public:
	CStateFirstAid() {}
	CStateFirstAid( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI ): CMissionState( pMissionGame, pMissionUI ) {}

	bool Initialize( IGlobalGame *pGame );

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return FORCED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// INSTANT STATES
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatePose
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStatePose: public CStateBase
{
	OBJECT_BASIC_METHODS(CStatePose);
private:
	ZDATA_(CStateBase)
	NAI::EPose ePose;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&ePose); return 0; }

public:
	CStatePose() {}
	CStatePose( NAI::EPose ePose );

	bool Initialize( IGlobalGame *pGame );

	EType GetType() const { return INSTANT; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// TEMPORARY STATES
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateSelection
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateSelection: public CMissionState
{
	OBJECT_BASIC_METHODS(CStateSelection);
private:
	ZDATA_(CMissionState)
	CVec2 vAnchor;
	CObj<NUI::CImage> pSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CMissionState*)this); f.Add(2,&vAnchor); f.Add(3,&pSelection); return 0; }

protected:
	bool OnLButtonUp();

public:
	CStateSelection() {}
	CStateSelection( IGlobalGame *pMissionGame, NUI::CMissionUI *pMissionUI, const CVec2 &vAnchor );

	bool Initialize( IGlobalGame *pGame );
	void Terminate();
	void Step();

	EType GetType() const { return TEMPORARY; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateVictory
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateVictory: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateVictory)
private:
	NInput::CBind bindEndMission, bindContinueMission;
	ZDATA_(CStateBase)
	bool bComplete;
	CObj<NUI::CWindow> pWindow;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&bComplete); f.Add(3,&pWindow); return 0; }

public:
	CStateVictory();

	bool Initialize( IGlobalGame *pGame );
	void Terminate();
	bool ProcessEvent( const NInput::SEvent &sEvent );

	EType GetType() const { return TEMPORARY; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
