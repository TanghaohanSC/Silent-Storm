#include "StdAfx.h"
#include "wUICommands.h"
//
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0xB1011160, CUICmd )
REGISTER_SAVELOAD_CLASS( 0xB1011161, CUICmdTurn )
REGISTER_SAVELOAD_CLASS( 0xB1011162, CUICmdUnit )
REGISTER_SAVELOAD_CLASS( 0x51402130, CUICmdMoveCamera )
REGISTER_SAVELOAD_CLASS( 0x53102130, CUICmdPlayDialog )
REGISTER_SAVELOAD_CLASS( 0x50412160, CUICmdContinueChapter )
REGISTER_SAVELOAD_CLASS( 0x51312180, CUICmdLoadTemplate )
REGISTER_SAVELOAD_CLASS( 0xB1122080, CUICmdShowStore )
REGISTER_SAVELOAD_CLASS( 0xB1122081, CUICmdShowTeamMng )
REGISTER_SAVELOAD_CLASS( 0x52022180, CUICmdPlayAck )
REGISTER_SAVELOAD_CLASS( 0x52022200, CUICmdSetFloor )
REGISTER_SAVELOAD_CLASS( 0x52622200, CUICmdShowClue )
REGISTER_SAVELOAD_CLASS( 0x53115170, CUICmdPartFinished )
REGISTER_SAVELOAD_CLASS( 0x53115171, CUICmdBeginSequence )
REGISTER_SAVELOAD_CLASS( 0x53115172, CUICmdEndSequence )