#include "StdAfx.h"
//
// Copyright (C) 2001 Nival Interactive 
// 
// File: pluginMain.cpp
//
// Author: Maya SDK Wizard
//

#include <maya/MFnPlugin.h>

#include "A5ExportModel.h"

__declspec(dllexport) MStatus initializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is loaded into Maya.  It 
//		registers all of the services that this plug-in provides with 
//		Maya.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{ 
	MFnPlugin plugin( obj, "Nival Interactive", "3.0", "Any" );

	return plugin.registerFileTranslator(
		"A5ExportModel", "none", A5ExportModel::creator,
		"A5ExportOptions", "" );
}
// model=0;animation=0;ai_model=0;mesh=0;skeleton=1;ai_mesh=0;

__declspec(dllexport) MStatus uninitializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is unloaded from Maya. It 
//		deregisters all of the services that it was providing.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{
	MFnPlugin plugin( obj );

	return plugin.deregisterFileTranslator( "A5ExportModel" );
}
