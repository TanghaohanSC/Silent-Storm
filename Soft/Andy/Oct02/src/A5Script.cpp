#include "StdAfx.h"
#include "A5Script.h"
#include "..\Misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "..\Misc\LogStream.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NScript
{
const char *V_THIS = "this";
extern Script::SRegFunction pRegList[];
extern Script::SRegFunction pUnitTagFuncList[],pVecTagFuncList[];
////////////////////////////////////////////////////////////////////////////////////////////////////
void CContext::CallMessageHandlerContext( const string &sName, const CContext *pContext )
{
	if ( PrepareForMessage( sName.c_str() ) )
	{
		PushContextPointer(pContext);
		CallMessageHandler();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CContext::PrepareForMessage( string sName )
{
	ASSERT( pMessageToCall == 0 );
	CScriptRefHash::iterator it = refs.find(sName);
	if ( it == refs.end() )
		return false;
	pMessageToCall = &it->second;
	nFirstArg = script.GetTop() + 1;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CContext::CallMessageHandler( const char *pszHandlerName )
{
	if ( !pMessageToCall )
	{
		if ( !pszHandlerName )
			return;
		if ( !PrepareForMessage(pszHandlerName) )
			return;
	}
	ASSERT(pMessageToCall);
	int nArg = script.GetTop() - nFirstArg + 1;
	for ( CObjRefs::iterator i = pMessageToCall->begin(); i != pMessageToCall->end(); ++i )
	{
		Script::ObjectReference value = *i;
		script.PushByRef(value);
		for ( int i = 0; i < nArg; ++i )
			script.PushValue( nFirstArg + i );
		script.Call( nArg, 0 );
	}
	for ( int i = 0; i < nArg; ++i )
		script.Remove( nFirstArg );
	pMessageToCall = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CContext::SetNewMessages( Script::Object &table )
{
	Script::AutoBlock fix(script);
	if ( !table.IsTable() )
		return;
	csSystem << "Add message handler\n";
	for ( Script::TableIterator i = table.First(); i != 0; i = table.Next() )
	{
		Script::Object obj = table.GetEntryValue(i);
		string name = table.GetEntryName(i);
		if ( obj.IsFunction() )
		{
			csSystem << "\t" << name << endl;
			refs[name].push_back( obj.GetRef() );
		}
		if ( obj.IsTable() )
		{
			int nSize = obj.GetTableSize();
			for ( int i = 1; i <= nSize; ++i )
			{
				Script::Object func = obj.GetByTableIndex(i);
				if ( func.IsFunction() )
				{
					csSystem << "\t" << name << endl;
					refs[name].push_back( script.PopAsRef() );
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CContext::AddScriptChunk( string strScriptText, string strChunkName )
{
	int nErr = script.DoBuffer( strScriptText.c_str(), strScriptText.length(), strChunkName.c_str() );
	if ( nErr != LUA_NOERR )
	{
		csSystem << CC_RED << "AddScriptChunk error: " << ErrorToString(nErr) << endl;
	}
	else
	{
		Script::AutoBlock fix(script);
		script.GetGlobal("MessagesToSet");
		Script::Object table = script.GetTopObject();
		SetNewMessages(table);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CContext* ContextPointer( const Script::Object &o )
{
	if ( o.Tag() != tagContext )
		return 0;
	return reinterpret_cast<CContext*>(o.GetUserData());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CContext *GetContext( lua_State* state )
{
	Script script(state);
	Script::AutoBlock fix(script);
	return ContextPointer( script.GetGlobal(V_THIS) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CContext::PushContextPointer( const CContext *p )
{
	script.PushUserTag( (void*)p, tagContext );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SharedInit( Script *scr )
{
	scr->Register( pRegList );
	int nTag = scr->RegisterNewTag( pUnitTagFuncList );
	ASSERT( nTag == tagContext );
	nTag = scr->RegisterNewTag( pVecTagFuncList );
	ASSERT( nTag == tagVec );
}
hash_map< int, CPtr<CContext> > scr_units;
CContext::CContext( NRPG::CUnit *pRPGUnit )
{
	static int nContextNumb = 1;
	nFirstArg = -1; pMessageToCall = 0;
	SharedInit(&script);
	Script::AutoBlock fix(script);
	// Create 'this'
	PushContextPointer( this );
	script.SetGlobal( V_THIS );

	nID = nContextNumb++;
	scr_units[nID] = this;

	pRPG = pRPGUnit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScript::CScript()
{
	SharedInit(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
CPtr<NScript::CScript> pScript;
////////////////////////////////////////////////////////////////////////////////////////////////////
NScript::CScript *GetScript()
{
	if ( !IsValid( pScript ) )
		return 0;
	return pScript;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Command proccessing
////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessCommand( const wstring &szCmd )
{
	if ( szCmd.empty() || ( szCmd.length() >= 2 && szCmd[0] == '-' && szCmd[1] == '-') ) // Skip comments
		return;
	if ( szCmd[0] == '@' )
	{
		NScript::CScript *pScr = GetScript();
		if ( !pScr )
		{
			csSystem << "Error executing script: no script is allowed in that moment";
			return;
		}
		string expr = NStr::ToAscii( szCmd );
		expr = expr.substr( 1, expr.length() );
		pScr->DoString( expr.c_str() );
	}
	else
		NGlobal::ProcessCommand( szCmd );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RunScriptFile( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
	{
		csSystem << "usage: cmd filename" << endl;
		return;
	}

//	try
	{
		NScript::CScript *pScr = GetScript();
		if ( !pScr )
		{
			csSystem << "Error executing script: no script is allowed in that moment";
			return;
		}
		CFileStream f;
		CMemoryStream m;
		string strFileName = NStr::ToAscii( szParams.front() );
		f.OpenRead( strFileName.c_str() );
		m.WriteFrom( f );
		csSystem << "Executing script '" << szParams.front() << "'..." << endl;
		const char *pszRez = ErrorToString( pScr->DoBuffer( (const char *)m.GetBuffer(), m.GetSize(), strFileName.c_str() ) );
		csSystem << pszRez << "." << endl;
	}
/*	catch(...)
	{
		csSystem << "Can't open file '" << szParams.front() << "'." << endl;
		return;
	}*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void PrintScriptState( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
	{
		csSystem << "usage: cmd all|ObjectName";
		return;
	}

	string strParam = NStr::ToAscii( szParams.front() );
	NScript::CScript *pScr = GetScript();
	if ( !pScr )
	{
		csSystem << "Error querying script: no script is allowed in that moment";
		return;
	}
	if ( strParam == "all" )
		csSystem << pScr->GetStateAsText();
	else
		csSystem << pScr->GetObjectAsText( strParam.c_str() );
}
#include "..\Misc\RandomGen.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TestRnd( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
	{
		csSystem << "usage: rnd number";
		return;
	}
	int nMaxC = NStr::ToInt( NStr::ToAscii( szParams.front() ) );
	int nCount = 0;
	int nLast = 200;
	for( int i = 0; i < nMaxC; ++i )
	{
		int nN = random.Get(100);
		if ( abs(nN-nLast) < 10 )
			nCount++;
		nLast = nN;
	}
	csSystem << "Rnd test:" << (100*nCount)/nMaxC << "%" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*static void ExecuteScriptThreads( const vector<wstring> &szParams, void *pContext )
{
	NScript::CScript *pScr = GetScript();
	if ( !pScr )
	{
		csSystem << "Error executing script: no script is allowed in that moment";
		return;
	}
  pScr->ExecuteThreads();
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(A5Script)
	REGISTER_CMD( "script_run", RunScriptFile )
	REGISTER_CMD( "script_show", PrintScriptState )
	REGISTER_CMD( "script_rnd", TestRnd )
//	REGISTER_CMD( "continue", ExecuteScriptThreads )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NScript;
REGISTER_SAVELOAD_CLASS( 0x70652130, CScript );
