#include "StdAfx.h"
#include "A5Script.h"
//
#include "wMain.h"
#include "rpgUnit.h"
#include "..\MiscDll\LogStream.h"
#include "scriptPtr.h"
#include "scriptCommon.h"
#include "scriptUnitGroup.h"
#include "scriptUnit.h"
#include "scriptSequence.h"
#include "scriptDiplomacy.h"
#include "scriptVector.h"
#include "scriptRoute.h"
#include "scriptDialog.h"
#include "scriptObject.h"
#include "scriptScenario.h"
#include "scriptTemplate.h"
#include "scriptPosition.h"
//
namespace NScript
{
static int Error_out( lua_State* state )
{
	Script script( state );
	Script::Object obj = script.GetObject(script.GetTop());
	csSystem << CC_RED << "Script error: " << CC_GREY << obj.GetString() << endl;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#define REG_FUNCTION( Name ) { #Name, lua##Name }
Script::SRegFunction pRegList[] =
{
	// Common functions
	{ "_ERRORMESSAGE", Error_out },
	{ "out", luaOut },
	{ "Sleep", LuaCFuncSleep },
	{ "StartThread", LuaCFuncStartThread },
	{ "random", luaRandom },
	{ "HasInventoryItem", luaHasInventoryItem },
	REG_FUNCTION( IsRealTime ),
	REG_FUNCTION( Explosion ),
	REG_FUNCTION( GetTurn ),
	REG_FUNCTION( Difficulty ),
	// Ptr
	{ "Ptr", luaMakeCPtr },
	{ "ObjPtr", luaMakeCObj },
	{ "IsValid", luaIsValid },
	REG_FUNCTION( IsEqual ),
	// Diplomacy
	REG_FUNCTION( GetDiplomacy ),
	REG_FUNCTION( SetDiplomacy ),
	REG_FUNCTION( UnitGetDiplomacy ),
	REG_FUNCTION( UnitSetDiplomacy ),
	// Camera
	REG_FUNCTION( GetCamera ),
	REG_FUNCTION( CameraMove ),
	REG_FUNCTION( IsInterfaceAction ),
	// Sequence
	REG_FUNCTION( c_BeginSequence ),
	REG_FUNCTION( EndSequencePart ),
	REG_FUNCTION( EndSequence ),
	REG_FUNCTION( Floor ),
	// Unit
	REG_FUNCTION( GetUnit ),
	REG_FUNCTION( CreateUnit ),
	REG_FUNCTION( GetHero ),
	REG_FUNCTION( UnitApplyCritical ),
	REG_FUNCTION( UnitSetXPLevel ),
	REG_FUNCTION( UnitGetVisible ),
	REG_FUNCTION( UnitIsAction ),
	REG_FUNCTION( UnitMoveToWaypoint ),
	REG_FUNCTION( UnitSetToWaypoint ),
	REG_FUNCTION( UnitShoot ),
	REG_FUNCTION( UnitSetShootMode ),
	REG_FUNCTION( UnitSetPose ),
	REG_FUNCTION( UnitSetWishPose ),
	REG_FUNCTION( UnitSetDirection ),
	REG_FUNCTION( UnitIsDead ),
	REG_FUNCTION( UnitIsUnconscious ),
	REG_FUNCTION( UnitSayAck ),
	REG_FUNCTION( UnitIsSeeUnit ),
	REG_FUNCTION( UnitReload ),
	REG_FUNCTION( UnitCheat ),
	REG_FUNCTION( UnitSetRoute ),
	REG_FUNCTION( UnitKill ),
	REG_FUNCTION( UnitPlayAnimation ),
	REG_FUNCTION( UnitHide ),
	REG_FUNCTION( UnitDrawPerksTree ),
	REG_FUNCTION( UnitTakePerk ),
	REG_FUNCTION( UnitCancelAction ),
	REG_FUNCTION( UnitRemove ),
	REG_FUNCTION( UnitGetName ),
	REG_FUNCTION( UnitSetPlayer ),
	REG_FUNCTION( UnitSetDialog ),
	REG_FUNCTION( UnitSetCanTalk ),
	REG_FUNCTION( UnitRoaming ),
	REG_FUNCTION( UnitTakeCorpse ),
	REG_FUNCTION( UnitDropCorpse ),
	REG_FUNCTION( UnitMakeUnconscious ),
	REG_FUNCTION( UnitActivateWeapon ),
	REG_FUNCTION( UnitPlaceInPocket ),
	REG_FUNCTION( UnitRestoreFromPocket ),
	REG_FUNCTION( UnitGetRoute ),
	REG_FUNCTION( UnitAI ),
	// Inner Route
	REG_FUNCTION( RouteIsFinished ),
	// Player
	REG_FUNCTION( PlayerGetUnits ),
	// UnitGroup
	REG_FUNCTION( CreateGroup ),
	REG_FUNCTION( GetGroup ),
	REG_FUNCTION( GroupGetID ),
	REG_FUNCTION( GroupAddUnit ),
	REG_FUNCTION( GroupRemoveUnit ),
	REG_FUNCTION( GroupGetSize ),
	REG_FUNCTION( GroupGetUnit ),
	REG_FUNCTION( GroupIsContainUnit ),
	REG_FUNCTION( GroupMoveToWaypoint ),
	REG_FUNCTION( GroupGetVisible ),
	REG_FUNCTION( GroupSetRoute ),
	REG_FUNCTION( GroupCheat ),
	REG_FUNCTION( GroupAddGroup ),
	REG_FUNCTION( GroupGetCross ),
	// Dialog
	REG_FUNCTION( DialogPlay ),
	REG_FUNCTION( DialogPlayAsAcks ),
	// Route
	REG_FUNCTION( CreateRoute ),
	// Object
	REG_FUNCTION( GetObject ),
	REG_FUNCTION( CreateObject ),
	REG_FUNCTION( ObjectOpen ),
	REG_FUNCTION( ObjectClose ),
	REG_FUNCTION( ObjectIsOpened ),
	REG_FUNCTION( ObjectDestroy ),
	REG_FUNCTION( ObjectRemove ),
	REG_FUNCTION( ObjectSetToWaypoint ),
	REG_FUNCTION( ObjectPlayAnimation ),
	REG_FUNCTION( ObjectIsAction ),
	REG_FUNCTION( ObjectSetDestroyStage ),
	REG_FUNCTION( ObjectGetName ),
	REG_FUNCTION( ObjectCancelAction ),
	// Item
	REG_FUNCTION( GetItem ),
	REG_FUNCTION( ItemGetName ),
	REG_FUNCTION( ItemRemove ),
	// Scenario
	REG_FUNCTION( ScenarioGiveClue ),
	REG_FUNCTION( ScenarioOpenZone ),
	REG_FUNCTION( ScenarioBlockZone ),
	REG_FUNCTION( ExitToChapter ),
	REG_FUNCTION( ClueShow ),
	REG_FUNCTION( ClueIsFound ),
	REG_FUNCTION( GetCurrentZoneAILevel ),
	// Interface
	REG_FUNCTION( uiShowStore ),
	REG_FUNCTION( uiShowTeamMngMenu ),
	// Test
	REG_FUNCTION( LuaTest ),
	// Template
	REG_FUNCTION( PlaceTemplate ),
	// Distance
	REG_FUNCTION( GetPos ),
	REG_FUNCTION( GetWaypointPos ),
	REG_FUNCTION( GetDistance ),
	//
	{ 0, 0 } // End
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CScript *CreateScript( NWorld::IWorld *pWorld )
{
	ASSERT( IsValid( pWorld ) );
	if ( !IsValid( pWorld ) )
		return 0;
	//
	CScript *pRes = new NScript::CScript();
	CDynamicCast<NWorld::CWorld> pTmpWorld( pWorld );
	pRes->pWorld = pTmpWorld;
	return pRes;
}
//
}