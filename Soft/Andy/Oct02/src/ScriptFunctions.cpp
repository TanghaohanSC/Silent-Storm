#include "StdAfx.h"
#include "A5Script.h"
#include "..\Misc\LogStream.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataMap.h"
#include "rpgUnit.h"
#include "wInterface.h"
#include "rpgItemInfo.h"
#include "rpgUnitInfo.h"
#include "aiCommander.h"
#include "wAckBase.h"
#include "wUnitServer.h"
#include "wMain.h"
#include "rpgGlobal.h"
#include "rpgDiplomacy.h"
#include "rpgUnitMission.h"
#include "rpgCritical.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NScript
{
extern hash_map< int, CPtr<CContext> > scr_units;
static int Error_out( lua_State* state )
{
	Script script(state);
	Script::Object obj = script.GetObject(script.GetTop());
	csSystem << CC_RED << "Script error: " << CC_GREY << obj.GetString() << endl;
	return 0;
}
static int luaRandom( lua_State* state )
{
	Script script(state);
	if ( script.GetTop() == 0 )
		script.PushNumber( random.Get() );
	else if ( script.GetTop() > 1 )
		script.PushNumber( random.Get( script.GetObject(1).GetInteger(), script.GetObject(2).GetInteger() ) );
	else
		script.PushNumber( random.Get( script.GetObject(1).GetInteger() ) );
	return 1;
}
static int luaOut(lua_State* state)
{
	Script script(state);
	int nTop = script.GetTop();
	for ( int i = 1; i <= nTop; ++i )
	{
		Script::Object o = script.GetObject(i);
		if ( o.Tag() >= tagContext )
		{
			switch ( o.Tag() )
			{
				case tagVec:
					{
						CVec3 vec = ToVec(o);
						csScript << "Vec{ x=" << vec.x << " y=" << vec.y << " z=" << vec.z << "}";
					}
					break;
				case tagContext:
					{
						CContext *pContext = ContextPointer(o);
						csScript << "Unit " << pContext->nID << "";
					}
					break;
				default:
					csScript << "-Incorrect tag-";
					break;
			}
		}
		else if ( o.IsUserData() )
		{
			csScript << "-UserData-";
			break;
		}
		else if ( o.IsNil() )
			csScript << "-Nil-";
		else if ( o.IsString() )
			csScript << o.GetString();
		else
			ASSERT(0); // attempting to output an unknown tag?
	}
	csScript << endl;
	return 0;
}
static int luaMessage(lua_State* state)
{
	Script script(state);
	if ( !script.CheckArgs( "c6s.", "Message" ) )
		return 0;
	int nArgs = script.GetTop();
	CContext *pContext = ContextPointer( script.GetObject(1) );
	if ( !IsValid( pContext ) )
		return 0;
	if ( pContext->PrepareForMessage( script.GetObject(2).GetString() ) )
	{
		for ( int j = 3; j <= nArgs; ++j )
			pContext->script.PushObject( script.GetObject(j) );
		pContext->CallMessageHandler();
	}
	return 0;
}
const char *pUnitStatProp[] = {	"Melle","Throwing","Pistols","Rifle","Smg","HW","FirstAid","Mechanics","Mine","Sneak","Acrobatics","Disguise","Radio","VP","AP","IC","Interrupt","Lvl","Str","Dex","Agl","Int","Per", 0 };
static int FindIndex( const char **pList, const string &str )
{
	for ( int i = 0; pList[i]; ++i )
		if ( str == pList[i] )
			return i;
	return -1;
}
static int tagUnitProp(lua_State* state)
{
	Script script(state);
	if ( !script.CheckArgs( "u6s", "tagUnitIndex" ) )
		return 0;
	CContext *pContext = ContextPointer( script.GetObject(1) );
	if ( !IsValid( pContext ) )
		return 0;
	string sProp = script.GetObject(2);
	int nIndex = FindIndex( pUnitStatProp, sProp );
	if ( nIndex != -1 )
	{
		script.PushNumber( pContext->pRPG->Skills(nIndex) );
		return 1;
	}
/*	else if ( sProp == "Name" )
	{
		script.PushNumber( pContext->pRPG->p );
	} */
	return 0;
}
static int luaUnit(lua_State* state)
{
	Script script(state);
	if ( !script.CheckArgs( "n", "Unit" ) )
		return 0;
	CContext *pContext = scr_units[script.GetObject(1).GetInteger()];
	if ( !IsValid( pContext ) )
		return 0;
	script.PushUserTag( (void*)pContext, tagContext );
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Vec
static int luaVec( lua_State* state )	// Args:none 
{
	Script script(state);
	script.NewTable();
	script.SetTag( tagVec );
	return 1;
}
static void SetVec( Script &script, const CVec3 &vec )
{
	Script::Object o = script.GetTopObject();
	if ( o.Tag() == tagVec )
	{
		o.SetNumber( "x", vec.x );
		o.SetNumber( "y", vec.y );
		o.SetNumber( "z", vec.z );
	}
}
CVec3 ToVec( Script::Object &o )
{
	if ( o.Tag() != tagVec )
		return CVec3(0,0,0);
	return CVec3( o.GetByName("x").GetNumber(), o.GetByName("y").GetNumber(), o.GetByName("z").GetNumber() );
}
void CContext::PushVec( const CVec3 &vec )
{
	luaVec( script.GetState() );
	SetVec( script, vec );
}
static int tagVecAdd(lua_State* state)
{
	Script script(state);
	if ( !script.CheckArgs( "c7c7", "Vec::Add" ) )
		return 0;
	CVec3 vOrig = ToVec( script.GetObject(1) );
	Script::Object o = script.GetObject(2);
	ASSERT ( o.Tag() == tagVec );
	vOrig += ToVec(o);
	luaVec( script.GetState() );
	SetVec( script, vOrig );
	return 1;
}
static int tagVecSub(lua_State* state)
{
	Script script(state);
	if ( !script.CheckArgs( "c7c7", "Vec::Sub" ) )
		return 0;
	CVec3 vOrig = ToVec( script.GetObject(1) );
	Script::Object o = script.GetObject(2);
	ASSERT ( o.Tag() == tagVec );
	vOrig -= ToVec(o);
	luaVec( script.GetState() );
	SetVec( script, vOrig );
	return 1;
}
static int tagVecMul(lua_State* state)
{
	Script script(state);
	if ( !script.CheckArgs( "c7n", "Vec::Mul" ) )
		return 0;
	CVec3 vOrig = ToVec( script.GetObject(1) );
	vOrig *= script.GetObject(2).GetNumber();
	luaVec( script.GetState() );
	SetVec( script, vOrig );
	return 1;
}
static int tagVecDiv(lua_State* state)
{
	Script script(state);
	if ( !script.CheckArgs( "c7n", "Vec::Div" ) )
		return 0;
	CVec3 vOrig = ToVec( script.GetObject(1) );
	vOrig /= script.GetObject(2).GetNumber();
	luaVec( script.GetState() );
	SetVec( script, vOrig );
	return 1;
}
static CVec3 gVecThis;
static int tagVecLen(lua_State* state)
{
	Script script(state);
	script.PushNumber( fabs(gVecThis) );
	return 1;
}
static int tagVecMemb( lua_State* state )
{
	Script script(state);
	if ( !script.CheckArgs( "c7s", "tagVecIndex" ) )
		return 0;
	string str = script.GetObject(2).GetString();
	if ( str == "len" )
	{
		gVecThis = ToVec( script.GetObject(1) );
		script.PushCFunction( tagVecLen );
		return 1;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaSayAck( lua_State* pState )
{
	CScript *pScript = GetScript();
	Script &script = *pScript;
	ASSERT( script.m_state == pState );
	if ( script.m_state != pState )
		return 0;
	if ( !script.CheckArgs( "un", "SayAck" ) )
		return 0;
	CDynamicCast<NWorld::IWorld> pWorld( pScript->pWorld );
	if ( !pWorld )
		return 0;
	Script::Object &o = script.GetObject(1);
	CObjectBase *pUserData = (CObjectBase *)o.GetUserData();
	CDynamicCast<NWorld::CUnitServer> pUS( pUserData );
	if ( !pUS )
		return 0;
	pWorld->GetGlobalAck()->SayAck( pUS, script.GetObject(2).GetInteger() );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaHasInventoryItem( lua_State* pState )
{
	CScript *pScript = GetScript();
	Script &script = *pScript;
	ASSERT( script.m_state == pState );
	if ( script.m_state != pState )
	{
		script.PushNil();
		return 1;
	}
	if ( !script.CheckArgs( "n", "HasInventoryItem" ) )
	{
		script.PushNil();
		return 1;
	}
	CDynamicCast<NWorld::IWorld> pWorld( pScript->pWorld );
	NWorld::IPlayer::CUnitSet units;
	int nHasItem = 0;
	NWorld::CUnitServer *pWhoHas;
	if ( !pWorld )
	{
		script.PushNil();
		return 1;
	}
	NWorld::IPlayer *pPlayer = pWorld->GetNextPlayerForScript( 0 );
	while ( pPlayer )
	{
		if ( CDynamicCast<NAI::CAICommander>( pPlayer->GetCommander() ) )
		{
			pPlayer = pWorld->GetNextPlayerForScript( pPlayer );
			continue;
		}
		pPlayer->GetUnits( &units );
		for ( int i = 0; i < units.size(); ++i )
		{
			NWorld::CUnit *pWho = units[i];
			CDynamicCast<NWorld::CUnitServer> pServer( pWho );
			NRPG::IInventoryInfo *pInfo = pWho->GetRPG()->GetInventoryInfo();
			for ( int j = 0; j < pInfo->GetItems().size(); ++j )
			{
				const NRPG::SBackPackItem &item = (pInfo->GetItems())[ j ];
				const NDb::CRPGItem *pDBItem = item.pItem->GetDBItem();
				if ( pDBItem->GetRecordID() == script.GetObject(1).GetInteger() )
				{
					++nHasItem;
					pWhoHas = pServer;
				}
			}
			NRPG::IInventoryItem *pItem = pInfo->Get( NDb::SLOT_1 );
			if ( pItem )
			{
				const NDb::CRPGItem *pDBItem = pItem->GetDBItem();
				if ( pDBItem->GetRecordID() == script.GetObject(1).GetInteger() )
				{
					++nHasItem;
					pWhoHas = pServer;
				}
			}
			pItem = pInfo->Get( NDb::SLOT_2 );
			if ( pItem )
			{
				const NDb::CRPGItem *pDBItem = pItem->GetDBItem();
				if ( pDBItem->GetRecordID() == script.GetObject(1).GetInteger() )
				{
					++nHasItem;
					pWhoHas = pServer;
				}
			}
		}
		pPlayer = pWorld->GetNextPlayerForScript( pPlayer );
	}
	if ( nHasItem )
		script.PushNumber( nHasItem );
	else
		script.PushNil();
	script.PushUserData( pWhoHas );
	return 2;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaGetGlobalGameVar( lua_State* pState )
{
	CScript *pScript = GetScript();
	Script &script = *pScript;
	ASSERT( script.m_state == pState );
	if ( script.m_state != pState )
		return 0;
	if ( !script.CheckArgs( "sn", "GetGlobalVar" ) )
		return 0;
	if ( CDynamicCast<NWorld::CWorld> pWorld( pScript->pWorld ) )
	{
		float fValue = script.GetObject(2).GetNumber();
		string varName( script.GetObject(1).GetString() );	

		fValue = pWorld->GetGlobalGame()->GetGlobalVar( varName );
		script.PushNumber( fValue );
			return 1;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaSetGlobalGameVar( lua_State* pState )
{
	CScript *pScript = GetScript();
	Script &script = *pScript;
	ASSERT( script.m_state == pState );
	if ( script.m_state != pState )
		return 0;
	if ( !script.CheckArgs( "sn", "SetGlobalVar" ) )
		return 0;
	if ( CDynamicCast<NWorld::CWorld> pWorld( pScript->pWorld ) )
	{
		pWorld->GetGlobalGame()->SetGlobalVar( script.GetObject(1).GetString(), 
			script.GetObject(2).GetNumber() );
		return 1;
	}
	//
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool luaPrepareData( lua_State* pState, 
	string szFuncName, string szParams, CScript **ppScript, NWorld::CWorld **ppWorld )
{
	*ppScript = 0;
	*ppWorld = 0;
	ASSERT( pState != 0 );
	if ( pState == 0 )
		return false;
	*ppScript = GetScript();
	ASSERT( (*ppScript)->m_state == pState );
	if ( (*ppScript)->m_state != pState )
		return false;
	if ( !(*ppScript)->CheckArgs( szParams.c_str(), szFuncName.c_str() ) )
		return false;
	CDynamicCast<NWorld::CWorld> pWorld( (*ppScript)->pWorld );
	if ( !IsValid( pWorld ) )
		return false;
	*ppWorld = pWorld.GetPtr();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaGetUnitByName( lua_State* pState )
{
	CScript *pScript;
	NWorld::CWorld *pWorld;
	if ( !luaPrepareData( pState, "GetDiplomacy", "s", &pScript, &pWorld ) )
		return 0;
	//
	string szName = pScript->GetObject(1).GetString();
	pScript->PushUserData( pWorld->GetUnitServer( szName ) );
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaGetPlayerToPlayerDiplomacy( lua_State* pState )
{
	CScript *pScript;
	NWorld::CWorld *pWorld;
	if ( !luaPrepareData( pState, "GetDiplomacy", "nn", &pScript, &pWorld ) )
		return 0;
	//
	int nPlayer1 = pScript->GetObject(1).GetInteger();
	int nPlayer2 = pScript->GetObject(2).GetInteger();
	NDb::EDiplomacyState state = 
		pWorld->GetGlobalGame()->pGlobalDiplomacy->GetDiplomacyState( nPlayer1, nPlayer2 );
	pScript->PushNumber( (int)state );
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaSetPlayerToPlayerDiplomacy( lua_State* pState )
{
	CScript *pScript;
	NWorld::CWorld *pWorld;
	if ( !luaPrepareData( pState, "SetDiplomacy", "nnn", &pScript, &pWorld ) )
		return 0;
	//
	int nPlayer1 = pScript->GetObject(1).GetInteger();
	int nPlayer2 = pScript->GetObject(2).GetInteger();
	NDb::EDiplomacyState state = (NDb::EDiplomacyState)pScript->GetObject(3).GetInteger(); 
	pWorld->GetGlobalGame()->pGlobalDiplomacy->SetDiplomacyState( nPlayer1, nPlayer2, state );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaGetUnitToPlayerDiplomacy( lua_State* pState )
{
	CScript *pScript;
	NWorld::CWorld *pWorld;
	if ( !luaPrepareData( pState, "GetUnitDiplomacy", "un", &pScript, &pWorld ) )
		return 0;
	//
	CDynamicCast<NWorld::CUnitServer> pUS( (CObjectBase *)pScript->GetObject(1).GetUserData() );
	if ( !IsValid( pUS ) )
		return 0;
	int nPlayer = pScript->GetObject(2).GetInteger();
	NDb::EDiplomacyState state = 
		pUS->GetUnitRPG()->GetDiplomacy()->GetDiplomacyState( nPlayer );
	pScript->PushNumber( (int)state );
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaSetUnitToPlayerDiplomacy( lua_State* pState )
{
	CScript *pScript;
	NWorld::CWorld *pWorld;
	if ( !luaPrepareData( pState, "SetUnitDiplomacy", "unn", &pScript, &pWorld ) )
		return 0;
	//
	CDynamicCast<NWorld::CUnitServer> pUS( (CObjectBase *)pScript->GetObject(1).GetUserData() );
	if ( !IsValid( pUS ) )
		return 0;
	int nPlayer = pScript->GetObject(2).GetInteger();
	NDb::EDiplomacyState state = (NDb::EDiplomacyState)pScript->GetObject(3).GetInteger(); 
	pUS->GetUnitRPG()->GetDiplomacy()->SetDiplomacyState( nPlayer, state );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaSetUnitXPLevel( lua_State* pState )
{
	CScript *pScript;
	NWorld::CWorld *pWorld;
	if ( !luaPrepareData( pState, "SetUnitXPLevel", "un", &pScript, &pWorld ) )
		return 0;
	//
	CDynamicCast<NWorld::CUnitServer> pUS( (CObjectBase *)pScript->GetObject(1).GetUserData() );
	if ( !IsValid( pUS ) )
		return 0;
	int nLevel = pScript->GetObject(2).GetInteger();
	pUS->GetUnitRPG()->GetRPGUnit()->SetXPLevel( nLevel );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static int luaApplyCriticalToUnit( lua_State* pState )
{
	CScript *pScript;
	NWorld::CWorld *pWorld;
	if ( !luaPrepareData( pState, "SetUnitXPLevel", "un", &pScript, &pWorld ) )
		return 0;
	//
	CDynamicCast<NWorld::CUnitServer> pUS( (CObjectBase *)pScript->GetObject(1).GetUserData() );
	if ( !IsValid( pUS ) )
		return 0;
	NDb::ECriticalLocation location = (NDb::ECriticalLocation)pScript->GetObject(2).GetInteger();
	NDb::ECritical critical = (NDb::ECritical)pScript->GetObject(3).GetInteger();
	NRPG::SCritical rpgCritical( location, critical );
	pUS->GetUnitRPG()->ApplyCritical( rpgCritical );
	pUS->ProcessCriticalImmediately( critical );
	csRPG << "<color=red>Critical applied\n";
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
Script::SRegFunction pRegList[] =
{
	{ "_ERRORMESSAGE", Error_out },
	{ "out", luaOut },
	{ "Sleep", LuaCFuncSleep },
	{ "StartThread", LuaCFuncStartThread },
	{ "Message", luaMessage },
	{ "Unit", luaUnit },
	{ "Vector", luaVec },
	{ "random", luaRandom },
	{ "HasInventoryItem", luaHasInventoryItem },
	{ "SayAck", luaSayAck },
	{ "GetGlobalGameVar", luaGetGlobalGameVar },
	{ "SetGlobalGameVar", luaSetGlobalGameVar },
	{ "GetDiplomacy", luaGetPlayerToPlayerDiplomacy },
	{ "SetDiplomacy", luaSetPlayerToPlayerDiplomacy },
	{ "GetUnitDiplomacy", luaGetUnitToPlayerDiplomacy },
	{ "SetUnitDiplomacy", luaSetUnitToPlayerDiplomacy },
	{ "GetUnitByName", luaGetUnitByName },
	{ "SetUnitXPLevel", luaSetUnitXPLevel },
	{ "Critical", luaApplyCriticalToUnit },
	{ 0, 0 } // End
};
Script::SRegFunction pUnitTagFuncList[] =
{
	{ "gettable", tagUnitProp },
	{ "index", tagUnitProp },
	{ 0, 0 } // End
};
Script::SRegFunction pVecTagFuncList[] =
{
	{ "gettable", tagVecMemb },
	{ "add", tagVecAdd },
	{ "sub", tagVecSub },
	{ "mul", tagVecMul },
	{ "div", tagVecDiv },
	{ 0, 0 } // End
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
