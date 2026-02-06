#include "StdAfx.h"

#include "aiUnit.h"
#include "aiCommander.h"
#include "aiTacticalCommander.h"
#include "aiTaskCommander.h"

#include "aiControl.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIControl
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIControl: public IAIControl
{
	ZDATA
	EAIControlType Type;
	CPtr<IAIUnit> pAIUnit;
	CPtr<CAICommander> pAICommander;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&Type); f.Add(3,&pAIUnit); f.Add(4,&pAICommander); return 0; }
	//
	CAIControl() {}
	CAIControl( CAICommander *_pAICommander, IAIUnit *_pAIUnit, EAIControlType ControlType ):
		pAICommander( _pAICommander ), pAIUnit( _pAIUnit ), Type( ControlType )
	{
		ASSERT( IsValid( pAICommander ) );
		ASSERT( IsValid( pAIUnit ) );
	}
	//
	virtual void Activate() = 0;
	virtual void DeActivate() = 0;
	virtual IAIUnit *GetAIUnit() { return pAIUnit; }
	virtual CAICommander *GetAICommander() { return pAICommander; }
	virtual EAIControlType GetType() { return Type; }
	virtual void DebugOutput() {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAITacticalControl
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAITacticalControl: public CAIControl
{
	OBJECT_BASIC_METHODS( CAITacticalControl );
	ZDATA
	ZPARENT( CAIControl );
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIControl *)this); return 0; }
public:
	//
	CAITacticalControl() {}
	CAITacticalControl( CAICommander *_pAICommander, IAIUnit *_pAIUnit ):
		CAIControl( _pAICommander, _pAIUnit, AI_CONTROL_UNINTERRUPTABLE )
	{
	}
	//
	virtual void Activate();
	virtual void DeActivate();
	virtual void DebugOutput() { OutputDebugString( "[AI TACTICAL CONTROL]\n" ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalControl::Activate()
{
	GetAICommander()->GetAITacticalCommander()->AddAllyUnit( GetAIUnit() );
	if ( GetAICommander()->IsAITurn() )
		GetAICommander()->GetAITacticalCommander()->Think();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITacticalControl::DeActivate()
{
	GetAICommander()->GetAITacticalCommander()->DismissUnit( GetAIUnit() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAITaskControl
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAITaskControl: public CAIControl
{
	OBJECT_BASIC_METHODS( CAITaskControl );
	ZDATA
	ZPARENT( CAIControl );
	CPtr<CTask> pAITask;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CAIControl *)this); f.Add(3,&pAITask); return 0; }
public:
	//
	CAITaskControl() {}
	CAITaskControl( CAICommander *_pAICommander, CTask *_pAITask, EAIControlType ControlType ):
		CAIControl( _pAICommander, _pAICommander->GetAIUnit( _pAITask->GetUnitServer() ), ControlType ), 
		pAITask( _pAITask )
	{
		ASSERT( IsValid( pAITask ) );
		//
		if ( !IsValid( pAITask ) )
			return;
		//
		pAITask->DeActivate();
		GetAICommander()->GetAITaskCommander()->AddTask( pAITask );
	}
	//
	virtual void Activate();
	virtual void DeActivate();
	virtual void DebugOutput() { OutputDebugString( "[AI TASK CONTROL]\n" ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITaskControl::Activate()
{
	pAITask->Activate();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAITaskControl::DeActivate()
{
	pAITask->DeActivate();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIControl *CreateAITacticalControl( CAICommander *pAICommander, IAIUnit *pAIUnit )
{
	return new CAITacticalControl( pAICommander, pAIUnit );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIControl *CreateAITaskControl( CAICommander *pAICommander, 
	CTask *pAITask, EAIControlType ControlType )
{
	return new CAITaskControl( pAICommander, pAITask, ControlType );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NAI;
//
REGISTER_SAVELOAD_CLASS( 0x52662100, CAITacticalControl );
REGISTER_SAVELOAD_CLASS( 0x52662101, CAITaskControl );