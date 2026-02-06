#ifndef __SCRIPTUNIT_H_
#define __SCRIPTUNIT_H_
//
namespace NScript
{
////////////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_SCRIPT_COMMAND( GetUnit );
DECLARE_SCRIPT_COMMAND( CreateUnit );
DECLARE_SCRIPT_COMMAND( GetHero );
DECLARE_SCRIPT_COMMAND( HasInventoryItem );
DECLARE_SCRIPT_COMMAND( UnitSayAck );
DECLARE_SCRIPT_COMMAND( UnitSetXPLevel );
DECLARE_SCRIPT_COMMAND( UnitApplyCritical );
DECLARE_SCRIPT_COMMAND( UnitIsAction );
DECLARE_SCRIPT_COMMAND( UnitShoot );
DECLARE_SCRIPT_COMMAND( UnitSetShootMode );
DECLARE_SCRIPT_COMMAND( UnitSetPose );
DECLARE_SCRIPT_COMMAND( UnitSetWishPose );
DECLARE_SCRIPT_COMMAND( UnitSetDirection );
DECLARE_SCRIPT_COMMAND( UnitIsDead );
DECLARE_SCRIPT_COMMAND( UnitIsUnconscious );
DECLARE_SCRIPT_COMMAND( UnitGetVisible );
DECLARE_SCRIPT_COMMAND( UnitIsSeeUnit );
DECLARE_SCRIPT_COMMAND( UnitReload );
DECLARE_SCRIPT_COMMAND( UnitCheat );
DECLARE_SCRIPT_COMMAND( UnitKill );
DECLARE_SCRIPT_COMMAND( UnitMakeUnconscious );
DECLARE_SCRIPT_COMMAND( UnitPlayAnimation );
DECLARE_SCRIPT_COMMAND( UnitHide );
DECLARE_SCRIPT_COMMAND( UnitDrawPerksTree ); // DEBUG
DECLARE_SCRIPT_COMMAND( UnitTakePerk );
DECLARE_SCRIPT_COMMAND( UnitCancelAction );
DECLARE_SCRIPT_COMMAND( UnitRemove );
DECLARE_SCRIPT_COMMAND( UnitGetName );
DECLARE_SCRIPT_COMMAND( UnitSetPlayer );
DECLARE_SCRIPT_COMMAND( UnitSetDialog );
DECLARE_SCRIPT_COMMAND( UnitSetCanTalk );
DECLARE_SCRIPT_COMMAND( UnitTakeCorpse );
DECLARE_SCRIPT_COMMAND( UnitDropCorpse );
DECLARE_SCRIPT_COMMAND( UnitActivateWeapon );
DECLARE_SCRIPT_COMMAND( UnitPlaceInPocket );
DECLARE_SCRIPT_COMMAND( UnitRestoreFromPocket );
DECLARE_SCRIPT_COMMAND( UnitGetRoute );
DECLARE_SCRIPT_COMMAND( RouteIsFinished );
DECLARE_SCRIPT_COMMAND( UnitAI );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __SCRIPTUNIT_H_