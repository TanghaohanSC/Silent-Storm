#include "StdAfx.h"
#include "MEUserSettings.h"
#include "MEParams.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
string GetMEParamName( int nParamID )
{
	switch ( nParamID )
	{
		case ME_SUBTEMPLATE_ALPHA:
			return "SubtemplateAlpha";
		case ME_SWITCH_SYSCOLORS:
			return "SwitchSysColors";
		case ME_OBJECTS_SHOWGROUND:
			return "ObjectsShowGround";
		case ME_SHOW_AISPHERES:
			return "ShowAISpheres";
		case ME_DEF_ANIM_MODEL:
			return "DefAnimModel";
		case ME_INSTANT_TERRAIN:
			return "InstantTerrainPreview";
	}
	return "";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float GetMEParamDefValue( int nParamID )
{
	switch ( nParamID )
	{
		case ME_SUBTEMPLATE_ALPHA:
			return 0.2f;
		case ME_SWITCH_SYSCOLORS:
			return 0;
		case ME_OBJECTS_SHOWGROUND:
			return 0;
		case ME_SHOW_AISPHERES:
			return 1;
		case ME_DEF_ANIM_MODEL:
			return -1;
		case ME_INSTANT_TERRAIN:
			return 0;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
