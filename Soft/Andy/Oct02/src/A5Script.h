#ifndef __A5Script_H_
#define __A5Script_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\Script\Script.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
	class CUnit;
}
namespace NAI
{
	class IUnit;
}
namespace NWorld
{
	class CWorld;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NScript
{
enum A5Tags
{
	tagContext = 6,
	tagVec
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScript: public Script, public CObjectBase 
{
	OBJECT_BASIC_METHODS(CScript);
public:
	ZDATA_(Script)
	CPtr<NWorld::CWorld> pWorld;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(Script*)this); f.Add(2,&pWorld); return 0; }
	CScript();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CContext: public CObjectBase
{
	OBJECT_BASIC_METHODS(CContext)
	typedef vector<Script::ObjectReference> CObjRefs;
	typedef hash_map<string,CObjRefs> CScriptRefHash;

	CScriptRefHash refs;
	int nFirstArg;
	CObjRefs *pMessageToCall;

	void SetNewMessages( Script::Object &table );

public:
	CPtr<NRPG::CUnit> pRPG;
//	CPtr<NAI::IUnit> pAI;
	int nID;
	Script script;
	CContext( NRPG::CUnit *pRPGUnit = 0 );

	void AddScriptChunk( string strScriptText, string strChunkName );

	// Messages
	bool PrepareForMessage( string sName );
	void CallMessageHandler( const char *pszHandlerName = 0 );
	// Service message call functions
	void CallMessageHandlerContext( const string &sName, const CContext *pContext );

	// A5 data type
	void PushContextPointer( const CContext *p );
	void PushVec( const CVec3 &vec );
};
CContext* ContextPointer( const Script::Object &o );
CVec3 ToVec( Script::Object &o );
CContext *GetContext( lua_State* state );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
NScript::CScript *GetScript();
void ProcessCommand( const wstring &szCmd );
#endif
