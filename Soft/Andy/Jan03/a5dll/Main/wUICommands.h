#ifndef __A5_UICOMMANDS_H_
#define __A5_UICOMMANDS_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//
#include "camera.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NScenario
{
	class CScenarioZone;
	class CScenarioClue;
}
//
namespace NWorld
{
class CUnit;
class CAckEvent;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUICmd
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EUICmdPriority
{
	UICP_HIGHEST			= 0,
	UICP_TURN					= 1,
	UICP_CAMERAMOVE		= 2,
	UICP_UNIT					= 3,
	UICP_INTERRUPT		= 4
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmd: public CObjectBase
{
	OBJECT_BASIC_METHODS(CUICmd);
public:
	ZDATA
	int nPriority;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nPriority); return 0; }

	CUICmd(): nPriority( 0x7FFF ) {}
	CUICmd( int _nPriority ): nPriority( _nPriority ) {}

	int GetPriority() const { return nPriority; } 
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// Script control
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdPartFinished: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdPartFinished );
	ZDATA_(CUICmd)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmd*)this); return 0; }
	//
	CUICmdPartFinished(): 
		CUICmd( 0 ) {}
}; 
////////////////////////////////////////////////////////////////////////////////////////////////////
// Camera control
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdMoveCamera: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdMoveCamera );
	ZDATA
public:
	ZPARENT( CUICmd );
	ICamera::SCameraPos pos;
	STime transitionTime;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUICmd *)this); f.Add(3,&pos); f.Add(4,&transitionTime); return 0; }
	//
	CUICmdMoveCamera() {}
	CUICmdMoveCamera(	const ICamera::SCameraPos &_pos, STime _transitionTime ): 
		CUICmd( UICP_CAMERAMOVE  ), pos( _pos ), transitionTime( _transitionTime ) {}
}; 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdTurn: public CUICmd
{
	OBJECT_BASIC_METHODS(CUICmdTurn);
public:
	ZDATA_(CUICmd)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmd*)this); return 0; }

	CUICmdTurn(): CUICmd( UICP_TURN ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdUnit: public CUICmd
{
	OBJECT_BASIC_METHODS(CUICmdUnit);
public:
	ZDATA_(CUICmd)
	CPtr<CUnit> pUnit;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmd*)this); f.Add(2,&pUnit); return 0; }

	CUICmdUnit() {}
	CUICmdUnit( CUnit *_pUnit ): CUICmd( UICP_UNIT ), pUnit( _pUnit ) {}
}; 
////////////////////////////////////////////////////////////////////////////////////////////////////
// Events
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdBeginSequence: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdBeginSequence );
private:
	ZDATA_(CUICmd)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmd*)this); return 0; }
	//
	CUICmdBeginSequence(): 
		CUICmd( 0 ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdEndSequence: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdEndSequence );
private:
	ZDATA_(CUICmd)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmd*)this); return 0; }
	//
	CUICmdEndSequence(): 
		CUICmd( 0 ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdPlayDialog: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdPlayDialog );
	ZDATA
public:
	ZPARENT( CUICmd );
	string szDialogCode;
	vector< CObj<NWorld::CUnit> > units;
	vector< CPtr<NWorld::CAckEvent> > phrases;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUICmd *)this); f.Add(3,&szDialogCode); f.Add(4,&units); f.Add(5,&phrases); return 0; }
	//
	CUICmdPlayDialog() {}
	CUICmdPlayDialog( const string &_szDialogCode, const vector< CObj<NWorld::CUnit> > &_units, const vector<CPtr<CAckEvent> > &_phrases ): 
		CUICmd( UICP_HIGHEST ), units( _units ), phrases( _phrases ), szDialogCode( _szDialogCode )	{}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdPlayAck: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdPlayAck );
	ZDATA
public:
	ZPARENT( CUICmd );
	vector< CPtr<NWorld::CAckEvent> > phrases;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUICmd *)this); f.Add(3,&phrases); return 0; }
	//
	CUICmdPlayAck() {}
	CUICmdPlayAck( const vector<CPtr<CAckEvent> > &_phrases ): CUICmd( UICP_HIGHEST ), phrases( _phrases ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdSetFloor: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdSetFloor );
	ZDATA
public:
	ZPARENT( CUICmd );
	int nFloor;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUICmd *)this); f.Add(3,&nFloor); return 0; }
	//
	CUICmdSetFloor() {}
	CUICmdSetFloor( int _nFloor ): CUICmd( UICP_HIGHEST ), nFloor( _nFloor ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdContinueChapter: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdContinueChapter );
	ZDATA
public:
	ZPARENT( CUICmd );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUICmd *)this); return 0; }
	//
	CUICmdContinueChapter(): CUICmd( UICP_HIGHEST ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdLoadTemplate: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdLoadTemplate );
	ZDATA
public:
	ZPARENT( CUICmd );
	int nTemplateID;
	CPtr<NScenario::CScenarioZone> pZone;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUICmd *)this); f.Add(3,&nTemplateID); f.Add(4,&pZone); return 0; }
	//
	CUICmdLoadTemplate() {}
	CUICmdLoadTemplate( NScenario::CScenarioZone *_pZone, int _nTemplateID = -1 ): 
		pZone( _pZone ), nTemplateID( _nTemplateID ), CUICmd( UICP_HIGHEST ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdShowStore: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdShowStore );
	ZDATA_(CUICmd)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmd*)this); return 0; }
	//
	CUICmdShowStore(): CUICmd( UICP_HIGHEST ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdShowTeamMng: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdShowTeamMng );
	ZDATA_(CUICmd)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmd*)this); return 0; }
	//
	CUICmdShowTeamMng(): CUICmd( UICP_HIGHEST ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdShowClue: public CUICmd
{
	OBJECT_BASIC_METHODS( CUICmdShowClue );
	ZDATA
	ZPARENT( CUICmd )
public:
	CPtr<NScenario::CScenarioClue> pClue;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUICmd *)this); f.Add(3,&pClue); return 0; }
	//
	CUICmdShowClue( NScenario::CScenarioClue *_pClue = 0 ): CUICmd( UICP_HIGHEST ), pClue( _pClue ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
