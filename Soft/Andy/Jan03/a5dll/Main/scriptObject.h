#ifndef __SCRIPTOBJECT_H_
#define __SCRIPTOBJECT_H_
//
namespace NScript
{
////////////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_SCRIPT_COMMAND( GetObject );
DECLARE_SCRIPT_COMMAND( CreateObject );
DECLARE_SCRIPT_COMMAND( ObjectOpen );
DECLARE_SCRIPT_COMMAND( ObjectClose );
DECLARE_SCRIPT_COMMAND( ObjectIsOpened );
DECLARE_SCRIPT_COMMAND( ObjectDestroy );
DECLARE_SCRIPT_COMMAND( ObjectRemove );
DECLARE_SCRIPT_COMMAND( ObjectSetToWaypoint );
DECLARE_SCRIPT_COMMAND( ObjectPlayAnimation );
DECLARE_SCRIPT_COMMAND( ObjectIsAction );
DECLARE_SCRIPT_COMMAND( ObjectSetDestroyStage );
DECLARE_SCRIPT_COMMAND( ObjectGetName );
DECLARE_SCRIPT_COMMAND( ObjectCancelAction );
DECLARE_SCRIPT_COMMAND( GetItem );
DECLARE_SCRIPT_COMMAND( ItemGetName );
DECLARE_SCRIPT_COMMAND( ItemRemove );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __SCRIPTOBJECT_H_