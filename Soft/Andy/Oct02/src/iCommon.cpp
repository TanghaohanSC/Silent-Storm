#include "StdAfx.h"
#include "GSceneUtils.h"
#include "RectLayout.h"
#include "GView.h"
#include "G2DView.h"
#include "Interface.h"
#include "IDecorators.h"
#include "wInterface.h"
#include "iCommon.h"
#include "..\DBFormat\DataFormat.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CBackground
////////////////////////////////////////////////////////////////////////////////////////////////////
CBackground::CBackground( IWindow *pContainer, float _fTransparency ):
	TBaseClass( pContainer ), fTransparency( _fTransparency )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBackground::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
		case EVENT_TEMPLATELOAD:
		{
			list<CPtr<IWindow> > childrenList;
			GetChildrenList( &childrenList );

			int nAlpha = 0xFF * fTransparency;
			for ( list<CPtr<IWindow> >::iterator iTemp = childrenList.begin(); iTemp != childrenList.end(); iTemp++ )
			{
				CPtr<IImage> pImage = dynamic_cast<IImage*>( iTemp->GetPtr() );
				if ( IsValid( pImage ) )
					pImage->SetColor( NGfx::SPixel8888( 0xFF, 0xFF, 0xFF, nAlpha ) );
			}
			break;
		}
	}

	return TBaseClass::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB2243954, CBackground );
