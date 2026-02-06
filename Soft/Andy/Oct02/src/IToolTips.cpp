#include "StdAfx.h"
#include "GSceneUtils.h"
#include "RectLayout.h"
#include "GView.h"
#include "G2DView.h"
#include "Interface.h"
#include "IDecorators.h"
#include "UIBaseCtrls.h"
#include "IToolTips.h"
#include "Cursor.h"
#include "..\Misc\StrProc.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataInterface.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
CToolTip::CToolTip( IWindow *pContainer, NDb::CUIContainer *_pToolTip, NDb::CUIContainer *_pBackground ):
	TBaseClass( pContainer ), pToolTip( _pToolTip ), pBackground( _pBackground )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CToolTip::ResizeBackground( const SPoint &sClientSize )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CToolTip::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_MOUSEMOVE:
		{
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CToolTip::Draw( const SScene &sScene )
{
	TBaseClass::Draw( sScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NUI;
//REGISTER_SAVELOAD_CLASS( 0x02841122, CInterface );
