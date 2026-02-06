#ifndef __I_GAMESTATES_H__
#define __I_GAMESTATES_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\Input\Bind.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
wstring GetErrorString( NWorld::EUnitCommandResult eResult );
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateBase
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateBase: public IState
{
private:
	ZDATA
	CPtr<IMission> pMission;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pMission); return 0; }

public:
	bool Initialize( IMission *pMission );
	void Terminate();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	bool ProcessMessage( const NUI::SEvent &sEvent );
	void Step();

	bool OnLButtonUp() { return false; }
	bool OnLButtonDown() { return false; }
	bool OnLButtonDblClk() { return false; }
	NUI::SCursorInfo GetCursorInfo() const { return NUI::SCursorInfo(); }

	IMission* GetMission() const;
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
	bool Initialize( IMission *pMission );

	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return UPDATED; }
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
	CPtr<IUnitTracker> pUnitTracker;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&bModifier); f.Add(3,&pUnitTracker); f.Add(4,&pTraceSelection); return 0; }

public:
	CStateTeam();

	bool Initialize( IMission *pMission );
	void Terminate();
	bool ProcessEvent( const NInput::SEvent &sEvent );
	void Step();

	bool OnLButtonUp();

	EType GetType() const { return UPDATED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateMove
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateMove: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateMove);
private:
	ZDATA_(CStateBase)
	bool bForced;
	bool bAnchorSet;
	CVec2 vAnchor;
	NUI::SCursorInfo sCursorInfo;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&bForced); f.Add(3,&bAnchorSet); f.Add(4,&vAnchor); f.Add(5,&sCursorInfo); return 0; }

protected:
	void DoMove( bool bInstant );
	void UpdateCursor();

public:
	CStateMove() {}
	CStateMove( bool bForced );

	bool Initialize( IMission *pMission );
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
class CStateAttack: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateAttack);
private:
	NInput::CBind bindHitLocationHead, bindHitLocationBody, bindHitLocationLArm, bindHitLocationRArm, bindHitLocationLLeg, bindHitLocationRLeg;
	ZDATA_(CStateBase)
	int nActionAP;
	bool bForced;
	bool bActionUnavailable;
	NUI::SCursorInfo sCursorInfo;
	NAI::EHitLocation eHitLocation;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&nActionAP); f.Add(3,&bForced); f.Add(4,&bActionUnavailable); f.Add(5,&sCursorInfo); f.Add(6,&eHitLocation); f.Add(7,&pTraceSelection); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();
	void UpdateCursor();
	void UpdateCursorInfo();
	void UpdateBlockedState();
	void UpdateTraceSelection();

public:
	CStateAttack();
	CStateAttack( bool bForced );

	bool Initialize( IMission *pMission );
	void Terminate();
	void Step();

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateUse
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateUse: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateUse);
private:
	ZDATA_(CStateBase)
	NUI::SCursorInfo sCursorInfo;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&sCursorInfo); f.Add(3,&pTraceSelection); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();

public:
	CStateUse() {}

	bool Initialize( IMission *pMission );
	void Terminate();

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return UPDATED; }
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

protected:
	NWorld::CCmd* GetTargetCmd();

public:
	CStatePickItem() {}

	bool Initialize( IMission *pMission );
	void Terminate();

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return UPDATED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateDropItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateDragItem: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateDragItem);
private:
	ZDATA_(CStateBase)
	CObj<NUI::CModel> pModel;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&pModel); return 0; }

public:
	bool Initialize( IMission *pMission );
	void Terminate();
	void Step();

	bool OnLButtonUp();

	EType GetType() const { return UPDATED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// TEMPORARY STATES
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateSelection
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateSelection: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateSelection);
private:
	ZDATA_(CStateBase)
	CVec2 vAnchor;
	CObj<NUI::CImage> pSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&vAnchor); f.Add(3,&pSelection); return 0; }

protected:
	bool OnLButtonUp();

public:
	CStateSelection() {}
	CStateSelection( const CVec2 &vAnchor );

	bool Initialize( IMission *pMission );
	void Terminate();
	void Step();

	EType GetType() const { return TEMPORARY; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateUnloadItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateUnloadItem: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateUnloadItem)
private:
	NInput::CBind bindCancel;
	ZDATA_(CStateBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();

public:
	CStateUnloadItem();

	bool Initialize( IMission *pMission );
	bool ProcessEvent( const NInput::SEvent &sEvent );

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return TEMPORARY; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// FORCED STATES
////////////////////////////////////////////////////////////////////////////////////////////////////
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
	CStateRotate() {}

	bool Initialize( IMission *pMission );

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return FORCED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateFirstAid
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateFirstAid: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateFirstAid);
private:
	ZDATA_(CStateBase)
	NUI::SCursorInfo sCursorInfo;
	CObj<CObjectBase> pTraceSelection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&sCursorInfo); f.Add(3,&pTraceSelection); return 0; }

protected:
	NWorld::CCmd* GetTargetCmd();

public:
	CStateFirstAid() {}

	bool Initialize( IMission *pMission );

	bool OnLButtonUp();
	NUI::SCursorInfo GetCursorInfo() const;

	EType GetType() const { return FORCED; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// INSTANT STATES
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateEmpty
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateEmpty: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateEmpty)
private:
	ZDATA_(CStateBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); return 0; }

public:
	bool Initialize( IMission *pMission );

	EType GetType() const { return INSTANT; }
};
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

	bool Initialize( IMission *pMission );

	EType GetType() const { return INSTANT; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateDropCannon
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateDropCorpse: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateDropCorpse);
private:
	ZDATA_(CStateBase)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); return 0; }

public:
	CStateDropCorpse() {}

	bool Initialize( IMission *pMission );

	EType GetType() const { return INSTANT; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateMoveItem
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateMoveItem: public CStateBase
{
	OBJECT_BASIC_METHODS(CStateMoveItem);
private:
	ZDATA_(CStateBase)
	CPtr<NWorld::CUnit> pUnit;
	NWorld::SItem sSource;
	NWorld::SItem sTarget;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CStateBase*)this); f.Add(2,&pUnit); f.Add(3,&sSource); f.Add(4,&sTarget); return 0; }

public:
	CStateMoveItem() {}
	CStateMoveItem( NWorld::CUnit *pUnit, const NWorld::SItem &sSource, const NWorld::SItem &sTarget );

	bool Initialize( IMission *pMission );

	EType GetType() const { return INSTANT; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
