#ifndef __A5EXPORTMODEL_H__
#define __A5EXPORTMODEL_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include <maya/MPxFileTranslator.h>
#include <maya/MDagPath.h>
#include <maya/MObjectArray.h>
#include "Streams.h"
/////////////////////////////////////////////////////////////////////////////////////
class A5ExportModel : public MPxFileTranslator
{
public:
	A5ExportModel() {};
	virtual ~A5ExportModel() {};

	static void* creator();

	MStatus reader( const MFileObject& file, const MString& optionsString, FileAccessMode mode );
	MStatus writer( const MFileObject& file, const MString& optionsString, FileAccessMode mode );

	bool haveReadMethod() const { return false; }
	bool haveWriteMethod() const { return true; }

	bool canBeOpened() const { return false; }
	MFileKind identifyFile( const MFileObject& fileName, const char* buffer, short size ) const;

private:
	CFileStream fp;
	//FILE *fp;
	MStatus Export();

	MStatus ExportJoints( const MObjectArray &objects );
	MStatus ExportSkinCluster( const MObject &object );
	MStatus ExportMesh( const MObject &object );

};
/////////////////////////////////////////////////////////////////////////////////////
#endif