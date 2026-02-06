#ifndef __A5Script_H_
#define __A5Script_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//
#include "..\Script\Script.h"
#include "..\MiscDll\Commands.h"
//
namespace NScenario
{
	class CScenarioTracker;
}
namespace NRPG
{
	class CUnit;
	class CGlobalGame;
}
//
namespace NAI
{
	class IUnit;
}
//
namespace NWorld
{
	class CWorld;
	class IWorld;
	class CUICmd;
	enum EInterfaceActionType;
}
//
namespace NGame
{
	class IMission;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NScript
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScript: public Script, public CObjectBase 
{
	NGlobal::CCmd cmdShowError;
	OBJECT_NOCOPY_METHODS(CScript);
	ZDATA_(Script)
public:
	CPtr<NWorld::CWorld> pWorld;
private:
	vector<int> interfaceActions;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(Script*)this); f.Add(2,&pWorld); f.Add(3,&interfaceActions); return 0; }
	CScript();
	int RunScriptFile( const string &szFileName );
	int RunScriptByID( int nID );
	void OnInterfaceActionStarted( NWorld::EInterfaceActionType type );
	void OnInterfaceActionFinished( NWorld::EInterfaceActionType type );
	NScenario::CScenarioTracker *GetScenarioTracker();
	NRPG::CGlobalGame* GetGlobalGame();
	void AddUICommand( NWorld::CUICmd *pCmd );
	bool IsInterfaceAction( NWorld::EInterfaceActionType type ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CScript *CreateScript( NWorld::IWorld *pWorld );
void ScriptWarning( const string &message );
void ScriptError( const string &message );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
NScript::CScript *GetScript();
void ProcessCommand( const wstring &szCmd );
#endif