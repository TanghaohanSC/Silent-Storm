#include "stdafx.h"
//
#include "A5Script.h"
#include "scriptCommon.h"
#include "wMain.h"
#include "wDialog.h"
#include "..\DBFormat\DataAck.h"
//
#include "scriptDialog.h"
//
namespace NScript
{
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( DialogPlay, "s" )
	CDBPtr<NDb::CDBDialog> pDBDialog = NDb::GetDBDialogByCode( luaParams[ 0 ].s );
	if ( !IsValid( pDBDialog ) )
		return 0;
	//
	pScript->OnInterfaceActionStarted( NWorld::IAT_DIALOG );
	NWorld::PlayDialog( pScript->pWorld, pDBDialog->GetRecordID() );
	return 0;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_SCRIPT_COMMAND( DialogPlayAsAcks, "s" )
	CDBPtr<NDb::CDBDialog> pDBDialog = NDb::GetDBDialogByCode( luaParams[ 0 ].s );
	if ( !IsValid( pDBDialog ) )
		return 0;
	//
	NWorld::PlayDialogAsAcks( pScript->pWorld, pDBDialog->GetRecordID() );
	return 0;
END_SCRIPT_COMMAND
////////////////////////////////////////////////////////////////////////////////////////////////////
}