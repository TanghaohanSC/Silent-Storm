#ifndef __A5EXPORTMODEL_H__
#define __A5EXPORTMODEL_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <maya/MPxFileTranslator.h>
#include <maya/MDagPath.h>
#include <maya/MObjectArray.h>
#include <maya/MPlugArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MIntArray.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include "Data.h"
#include "Streams.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAdditionalBone
{
	MDagPath bonePath; // dag path to trasnform of the bone
	vector<MPlug> plugs; // all weight plugs for this bone
	// why I cannot use 'MPlugArray' instead of 'vector<MPlug>' remains mysterious for me
	MIntArray nJoints; // corresponding joints
};
////////////////////////////////////////////////////////////////////////////////////////////////////
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
	CFileStream fp; // currently opened file
	string szCurrentFileName; // currently opened file

	string szExportFileName;
	string szExportFilePath;
	int nFileNumber;

	string szBindsPath, szEffPath;

	double fStartFrame, fEndFrame, fFrameRate;
	bool bAnimWithScale;

	// convertion to root CS is not needed
	//MMatrix mxRootInverse;
	vector<MMatrix> mxCurPoses, mxBindPoses;
	vector<int> nJointIndices;
	vector<float> fWeightsArray;
	vector<DWORD> dwColors;
	vector<int> nSetIndices;
	vector<SAdditionalBone> addBones;
	vector<int> nPieces;
	MObjectArray oJoints;
	MObject oSkin;
	MObjectArray oAISkins;
	MDagPath meshPath;
	MDagPathArray addMeshPaths; // pieces for destructible blocks
	MDagPathArray addLocators; // special locators attached to mesh (eg point of flame for weapons etc)
	MDagPathArray addSpheres; // mass spheres attached to AI mesh (for collisions and stability computation)
	MObjectArray particleObjects; // particle sets to export

	void ClearAll();

	MStatus ExportMesh();
	MStatus ExportAIMesh();
	MStatus ExportSkin();
	MStatus ExportAISkin();
	MStatus ExportAnimation();
	MStatus ExportSkeleton();
	MStatus ExportParticles();
	MStatus ExportLight();

	void ExportWithMaterialFilter( int nFlags );

	MStatus GetSelection( MObject *pObject );
	MStatus GetExportedJoints();
	MStatus GetExportedSkin();
	MStatus GetExportedMesh();
	MStatus GetExportedParticles();
	MStatus GetExportedAISkins();

	MStatus GetAdditionalBonesData();
	MStatus GetJointsData();
	MStatus GetGeometryData();
	MStatus GetSkinClusterData();
	MStatus GetShaderData();
	MStatus GetAddMeshesData();
	MStatus GetParticlesData();
	MStatus GetColorsData();
	MStatus GetSpheresData();
	
	void SolveDuDv( MVector &normal, MVector &side1, MVector &side2, float dU1, float dU2, MVector *pRes );
	void CalculateDuDv();

	void OutputMatrix( const char *pszText, MMatrix &mx, FILE *f = stderr );
	void OutputVector( const char *pszText, MVector &v, FILE *f = stderr );
	void OutputPoint( const char *pszText, MPoint &p, FILE *f = stderr );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif