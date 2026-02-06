#include "StdAfx.h"
#include "..\Misc\StrProc.h"
#include "..\MiscDll\LogStream.h"
#include "..\DBFormat\DataRPG.h"
#include "rpgGlobal.h"
#include "scScenarioTracker.h"
#include "wMain.h"
#include "wUICommands.h"
#include "wInterface.h"
#include "..\Script\lua.h"
#include "A5Script.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NScript
{
externA5 Script::SRegFunction pRegList[];
Script::SRegFunction pLuaPtrTagFuncList[] = { (0,0) };
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SharedInit( Script *scr )
{
	scr->Register( pRegList );
	int nTag = 0;
	nTag = scr->RegisterNewTag( pLuaPtrTagFuncList );
	ASSERT( nTag == tagLuaCPtr );
	nTag = scr->RegisterNewTag( pLuaPtrTagFuncList );
	ASSERT( nTag == tagLuaCObj );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandShowScriptError( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	if ( luaLastError.szError.empty() )
	{
		csSystem << "there are no script errors" << endl;
		return;
	}
	//
	csSystem << endl << "Script last error:" << endl;
	csSystem << CC_RED << "Script error: " << CC_GREY << luaLastError.szError << endl;
	vector< SLUAError::SLUAStackTrace >::iterator i;
	for ( i = luaLastError.stack.begin(); i != luaLastError.stack.end(); ++i )
	{
		csSystem << CC_RED << "\t" << (*i).nDepth;
		csSystem << CC_GREY << "\tfile: \"" << (*i).szSource << "\"";
		csSystem << ",   function \"" << (*i).szFunctionName << "\"";
		csSystem << ",   defined at line  " << (*i).nDefinedAtLine;
		csSystem << endl;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CScript::CScript():
	cmdShowError( "scripterror", CommandShowScriptError, this )
{
	SharedInit(this);
	interfaceActions.resize( NWorld::N_INTERFACE_ACTION_TYPE, 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScript::OnInterfaceActionStarted( NWorld::EInterfaceActionType type )
{
	ASSERT( type >= 0 && type < NWorld::N_INTERFACE_ACTION_TYPE );
	++interfaceActions[ type ];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScript::OnInterfaceActionFinished( NWorld::EInterfaceActionType type )
{
	ASSERT( type >= 0 && type < NWorld::N_INTERFACE_ACTION_TYPE );
	int &nAction = interfaceActions[ type ];
	--nAction;
	nAction = Max( 0 , nAction );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CScript::IsInterfaceAction( NWorld::EInterfaceActionType type ) const
{
	ASSERT( type >= 0 && type < NWorld::N_INTERFACE_ACTION_TYPE );
	return interfaceActions[ type ] > 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScript::RunScriptByID( int nID )
{
	CDBPtr<NDb::CScript> pDBScript = NDb::GetDBScript( nID );
	if ( IsValid( pDBScript ) )
		return DoBuffer( (const char *)pDBScript->strCode.c_str(), pDBScript->strCode.length() );
	return -1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CScript::RunScriptFile( const string &szFileName )
{
	try
	{
		CFileStream f;
		CMemoryStream m;
		f.OpenRead( szFileName.c_str() );
		m.WriteFrom( f );
		return DoBuffer( (const char *)m.GetBuffer(), m.GetSize(), szFileName.c_str() );
	}
	catch ( ... )
	{
		DebugTrace( "Can't find script file %s", szFileName.c_str() );
		ASSERT( 0 );
		return -1;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NScenario::CScenarioTracker* CScript::GetScenarioTracker()
{
	return pWorld->GetGlobalGame()->pScenarioTracker;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NRPG::CGlobalGame* CScript::GetGlobalGame()
{
	return pWorld->GetGlobalGame();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CScript::AddUICommand( NWorld::CUICmd *pCmd )
{
	pWorld->AddUICommand( pCmd );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptWarning( const string &message )
{
	csSystem << CC_RED << "Script warning: " << CC_GREY << message << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptError( const string &message )
{
	csSystem << CC_RED << "Script error: " << CC_GREY << message << endl;
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
		int nErr = pScr->DoString( expr.c_str() );
		if ( nErr != 0 )
			csSystem << CC_RED << "script error : " << ErrorToString(nErr) << endl;
	}
	else
		NGlobal::ProcessCommand( szCmd );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RunScriptFile( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
	{
		csSystem << "usage: cmd < filename | ID >" << endl;
		return;
	}
	//
	NScript::CScript *pScr = GetScript();
	if ( !pScr )
	{
		csSystem << "Error querying script: no script is allowed in that moment";
		return;
	}
	//
	const char *pszRez = 0;
	string szParam = NStr::ToAscii( szParams.front() );
	int nID = atoi( szParam.c_str() );
	if ( nID > 0 )
	{
		csSystem << "Executing script, ID = " << nID << " ..." << endl;
		pszRez = ErrorToString( pScr->RunScriptByID( nID ) );
	}
	else
	{
		szParam = "scripts\\" + szParam;
		csSystem << "Executing script, filename = '" << szParam << "' ..." << endl;
		pszRez = ErrorToString( pScr->RunScriptFile( szParam ) );
	}
	//
	csSystem << pszRez << "." << endl;
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