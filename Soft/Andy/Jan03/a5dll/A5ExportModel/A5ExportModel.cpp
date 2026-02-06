#include "StdAfx.h"

#include <stdio.h>
#include <math.h>

#include <maya\MDagPathArray.h>
#include <maya\MSelectionList.h>
#include <maya\MGlobal.h>
#include <maya\MFnMesh.h>
#include <maya\MFloatVectorArray.h>
#include <maya\MFnIntArrayData.h>
#include <maya\MFnVectorArrayData.h>
#include <maya\MFnDoubleArrayData.h>
#include <maya\MPointArray.h>
#include <maya\MItMeshPolygon.h>
#include <maya\MItSelectionList.h>
#include <maya\MItDependencyGraph.h>
#include <maya\MItDependencyNodes.h>
#include <maya\MItDag.h>
#include <maya\MItGeometry.h>
#include <maya\MFnSkinCluster.h>
#include <maya\MFnTransform.h>
#include <maya\MQuaternion.h>
#include <maya\MFnIkJoint.h>
#include <maya\MMatrix.h>
#include <maya\MFnTypedAttribute.h>
#include <maya\MFnMatrixData.h>
#include <maya\MFnMeshData.h>
#include <maya\MFnSet.h>
#include <maya\MTime.h>

#include "A5ExportModel.h"
#include "Particles.h"
#include "Polygons.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
void* A5ExportModel::creator()
{
	return new A5ExportModel();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::reader( const MFileObject& file, const MString& optionsString, FileAccessMode mode )
{
	return MS::kFailure;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MPxFileTranslator::MFileKind A5ExportModel::identifyFile(
	const MFileObject& fileName, const char* buffer, short size ) const
{
	return kNotMyFileType;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::writer( const MFileObject& file, const MString& options, FileAccessMode mode )
{
	bool bSkin = false, bAnim = false, bAISkin = false, bMesh = false, bAIMesh = false,
		bSkel = false, bParticles = false, bLight = false;

	nFileNumber = 0;
	szBindsPath = file.path().asChar();
	fFrameRate = 30;
	bool bAllSceneFrames = true;
	fStartFrame = 0;
	fEndFrame = 0;
	bAnimWithScale = false;
	if ( options.length() > 0 )
	{
		int i, length;
		MStringArray optionList;
		MStringArray theOption;
		options.split(';', optionList); // break out all the options.
		length = optionList.length();
		for( i = 0; i < length; ++i )
		{
			theOption.clear();
			optionList[i].split( '=', theOption );
			if( theOption[0] == MString("skin") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bMesh = true;
			}
			if( theOption[0] == MString("animation") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bAnim = true;
			}
			if( theOption[0] == MString("ai_skin") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bAIMesh = true;
			}
			if( theOption[0] == MString("mesh") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bMesh = true;
			}
			if( theOption[0] == MString("ai_mesh") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bAIMesh = true;
			}
			if( theOption[0] == MString("skeleton") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bSkel = true;
			}
			if( theOption[0] == MString("particles") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bParticles = true;
			}
			if( theOption[0] == MString("light") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bLight = true;
			}
			if( theOption[0] == MString("fps") && theOption.length() > 1 )
				fFrameRate = theOption[1].asFloat();
			if( theOption[0] == MString("start") && theOption.length() > 1 )
				fStartFrame = theOption[1].asFloat();
			if( theOption[0] == MString("end") && theOption.length() > 1 )
				fEndFrame = theOption[1].asFloat();
			if( theOption[0] == MString("all_frames") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() == 0 )
					bAllSceneFrames = false;
			}
			if ( theOption[0] == MString("binds_path") && theOption.length() > 1 )
				szBindsPath = theOption[1].asChar();
			if ( theOption[0] == MString("eff_path") && theOption.length() > 1 )
				szEffPath = theOption[1].asChar();
			if ( theOption[0] == MString("scale") && theOption.length() > 1 )
			{
				if( theOption[1].asInt() > 0 )
					bAnimWithScale = true;
			}
		}
	}

	if ( bAllSceneFrames )
	{
		MGlobal::executeCommand( "playbackOptions -q -min", fStartFrame );
		MTime start( fStartFrame, MTime::uiUnit() );
		fStartFrame = start.as( MTime::kMilliseconds );
		MGlobal::executeCommand( "playbackOptions -q -max", fEndFrame );
		MTime end( fEndFrame, MTime::uiUnit() );
		fEndFrame = end.as( MTime::kMilliseconds );
	}
	else if ( fStartFrame > fEndFrame )
	{
		float tmp = fStartFrame;
		fStartFrame = fEndFrame;
		fEndFrame = tmp;
	}

	if ( bSkin && bAnim )
		bAnim = false;

	MString numberName = file.name();
	if ( numberName.asInt() > 0 )
		nFileNumber = numberName.asInt();
	szExportFilePath = file.path().asChar();

	MString mname = file.fullName();
	szExportFileName = mname.asChar();

	szCurrentFileName = szExportFileName;

	MStatus status = MS::kSuccess;
	try
	{
		if ( bMesh )
		{
			MObject object;
			if ( GetSelection( &object ) == MS::kSuccess )
			{
				if ( object.apiType() == MFn::kSkinClusterFilter )
					status = ExportSkin();
				else
					status = ExportMesh();
			}
			else
				status = MS::kFailure;
		}
		else if ( bAIMesh )
		{
			MObject object;
			if ( GetSelection( &object ) == MS::kSuccess )
			{
				if ( object.apiType() == MFn::kSet )
					status = ExportAISkin();
				else
					status = ExportAIMesh();
			}
			else
				status = MS::kFailure;
		}
		else if ( bAnim )
			status = ExportAnimation();			
		else if ( bSkel )
			status = ExportSkeleton();
		else if ( bParticles )
			status = ExportParticles();
		else if ( bLight )
			status = ExportLight();
		/*
		if ( bSkin )
			status = ExportSkin();
		else if ( bAISkin )
			status = ExportAISkin();
		else if ( bAnim )
			status = ExportAnimation();
		else if ( bMesh )
			status = ExportMesh();
		else if ( bAIMesh )
			status = ExportAIMesh();
		else if ( bSkel )
			status = ExportSkeleton();
		*/

		ClearAll();
	}
	catch(...)
	{
		fprintf( stderr, "Error: Writing file %s.\n", szCurrentFileName );
		status = MS::kFailure;
	}

	if( status == MS::kFailure )
	{
		fprintf( stderr, "Error: Export has failed!\n" );
		return MS::kFailure;
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void A5ExportModel::ClearAll()
{
	mxCurPoses.clear();
	mxBindPoses.clear();
	nJointIndices.clear();
	nSetIndices.clear();
	fWeightsArray.clear();
	addBones.clear();
	oJoints.clear();
	oSkin = MObject::kNullObj;
	oAISkins.clear();
	addMeshPaths.clear();
	addLocators.clear();
	addSpheres.clear();
	particleObjects.clear();
	nPieces.clear();
	dwColors.clear();
	NConverter::ClearAll();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetSelection( MObject *pObject )
{
	MSelectionList selection;
 	MGlobal::getActiveSelectionList( selection );
	MItSelectionList iterSL( selection );
	MObject object;
	for ( ; !iterSL.isDone(); iterSL.next() )
		iterSL.getDependNode( *pObject );
	if ( pObject->isNull() )
	{
		fprintf( stderr, "Error: No objects selected!\n" );
		return MS::kFailure;
	}
	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetExportedJoints()
{
	MObject object;
	if ( GetSelection( &object ) == MS::kFailure )
		return MS::kFailure;
	if ( object.apiType() == MFn::kSkinClusterFilter )
	{
		oSkin = object;

		MStatus status;
		MFnDependencyNode skin(object);
		MPlug plugMatrix = skin.findPlug( "matrix", &status );
		if ( status != MS::kSuccess || !plugMatrix.isArray() )
		{
			fprintf( stderr, "Error: Skin cluster %s has no \"matrix\" plug!\n", skin.name().asChar() );
			return MS::kFailure;
		}
		if ( plugMatrix.numElements() < 1 )
		{
			fprintf( stderr, "Error: Skin cluster %s has no matrices in \"matrix\" plug!\n", skin.name().asChar() );
			return MS::kFailure;
		}
		MPlugArray inputs;
		if ( !plugMatrix[0].connectedTo( inputs, true, false ) || inputs.length() == 0 )
		{
			fprintf( stderr, "Error: Skin cluster %s - \"matrix\" plug is not connected!\n", skin.name().asChar() );
			return MS::kFailure;
		}

		MObject oTempJoint;
		for ( int i = 0; i < inputs.length(); ++i )
		{
			if ( inputs[i].node().apiType() == MFn::kJoint )
			{
				oTempJoint = inputs[i].node();
				break;
			}
		}
		if ( oTempJoint.isNull() )
		{
			fprintf( stderr, "Error: Haven't found any input joint for skin cluster %s!\n", skin.name().asChar() );
			return MS::kFailure;
		}
		// climb up the tree
		while ( oTempJoint.apiType() == MFn::kJoint )
		{
			object = oTempJoint;
			MFnDagNode dag(object);
			oTempJoint = dag.parent(0);
		}
	}

	if ( object.apiType() != MFn::kJoint )
	{
		fprintf( stderr, "Error: One joint root node must be selected!\n" );
		return MS::kFailure;
	}

	MItDag itDAG;
	itDAG.reset( object, MItDag::kBreadthFirst, MFn::kJoint );
	for ( ; !itDAG.isDone(); itDAG.next() )
	{
		bool bEndJoint = true;
		MDagPath path;
		itDAG.getPath( path );
		for ( int i=0; i<path.childCount(); ++i )
			if ( path.child(i).apiType() == MFn::kJoint )
			{
				bEndJoint = false;
				break;
			}
		//if ( !bEndJoint )
		oJoints.append( itDAG.item() );
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetExportedSkin()
{
	/*
	int count = oJoints.length();
	for ( int i=0; i<count; ++i )
	{
		MFnDependencyNode node( oJoints[i] );
		MItDependencyGraph itDG( oJoints[i], MFn::kGeometryFilt );
		for ( ; !itDG.isDone(); itDG.next() )
		{
			MObject obj = itDG.thisNode();
			MFnDependencyNode child( obj );
			if ( obj.apiType() == MFn::kSkinClusterFilter )
			{
				oSkin = obj;
				break;
			}
		}
		if ( !oSkin.isNull() )
			break;
	}
*/
	if ( oSkin.isNull() )
	{
		fprintf( stderr, "Error: No skin to export!\n" );
		return MS::kFailure;
	}
	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetExportedAISkins()
{
	MObject object;
	if ( GetSelection( &object ) == MS::kFailure )
		return MS::kFailure;
	if ( object.apiType() != MFn::kSet )
	{
		fprintf( stderr, "Error: No set selected!\n" );
		return MS::kFailure;
	}
	MFnSet setFn( object );
	MSelectionList selList;
	setFn.getMembers( selList, false );
	for ( int i = 0; i < selList.length(); ++i )
	{
		MObject obj;
		selList.getDependNode( i, obj );
		if ( obj.apiType() == MFn::kSkinClusterFilter )
			oAISkins.append( obj );
		if ( obj.apiType() == MFn::kTransform )
		{
			MItDag itDAG;
			itDAG.reset( obj, MItDag::kBreadthFirst, MFn::kNurbsSurface );
			for ( ; !itDAG.isDone(); itDAG.next() )
			{
				if ( itDAG.depth() == 1 )
				{
					MDagPath tempPath;
					MDagPath::getAPathTo( obj, tempPath );
					addSpheres.append( tempPath );
				}
			}
		}
	}

	if ( oAISkins.length() == 0 )
	{
		fprintf( stderr, "Error: Set %s has no skin clusters!\n", setFn.name().asChar() );
		return MS::kFailure;
	}

	MStatus status;
	MFnDependencyNode skin( oAISkins[0] );
	MPlug plugMatrix = skin.findPlug( "matrix", &status );
	if ( status != MS::kSuccess || !plugMatrix.isArray() )
	{
		fprintf( stderr, "Error: Skin cluster %s has no \"matrix\" plug!\n", skin.name().asChar() );
		return MS::kFailure;
	}
	if ( plugMatrix.numElements() < 1 )
	{
		fprintf( stderr, "Error: Skin cluster %s has no matrices in \"matrix\" plug!\n", skin.name().asChar() );
		return MS::kFailure;
	}
	MPlugArray inputs;
	if ( !plugMatrix[0].connectedTo( inputs, true, false ) || inputs.length() == 0 )
	{
		fprintf( stderr, "Error: Skin cluster %s - \"matrix\" plug is not connected!\n", skin.name().asChar() );
		return MS::kFailure;
	}

	MObject oTempJoint;
	for ( i = 0; i < inputs.length(); ++i )
	{
		if ( inputs[i].node().apiType() == MFn::kJoint )
		{
			oTempJoint = inputs[i].node();
			break;
		}
	}
	if ( oTempJoint.isNull() )
	{
		fprintf( stderr, "Error: Haven't found any input joint for skin cluster %s!\n", skin.name().asChar() );
		return MS::kFailure;
	}
	// climb up the tree
	while ( oTempJoint.apiType() == MFn::kJoint )
	{
		object = oTempJoint;
		MFnDagNode dag(object);
		oTempJoint = dag.parent(0);
	}

	MItDag itDAG;
	itDAG.reset( object, MItDag::kBreadthFirst, MFn::kJoint );
	for ( ; !itDAG.isDone(); itDAG.next() )
	{
		bool bEndJoint = true;
		MDagPath path;
		itDAG.getPath( path );
		for ( int i=0; i<path.childCount(); ++i )
			if ( path.child(i).apiType() == MFn::kJoint )
			{
				bEndJoint = false;
				break;
			}
		//if ( !bEndJoint )
		oJoints.append( itDAG.item() );
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetExportedMesh()
{
	MObject object;
	MObject objectMesh;

	if ( GetSelection( &object ) == MS::kFailure )
		return MS::kFailure;

	if ( object.apiType() != MFn::kTransform )
	{
		fprintf( stderr, "Error: You must select mesh transform node!\n" );
		return MS::kFailure;
	}

	MFnDagNode dag( object );
	int nChildren = dag.childCount();
	if ( nChildren == 0 )
	{
		fprintf( stderr, "Error: No children of transform %s!\n", dag.name().asChar() );
		return MS::kFailure;
	}
	for ( int i = 0; i < nChildren; ++i )
	{
		MObject child = dag.child(i);
		if ( child.apiType() == MFn::kMesh )
		{
			MFnDagNode node(child);
			if ( !node.isIntermediateObject() )
				objectMesh = child;
		}
	}
	if ( objectMesh.isNull() )
	{
		fprintf( stderr, "Error: No mesh children of transform %s!\n", dag.name().asChar() );
		return MS::kFailure;
	}
	MDagPath::getAPathTo( objectMesh, meshPath );

	MStatus stat;
	MFnDependencyNode node( objectMesh );
	MPlug plug = node.findPlug( MString("destructibleBlock"), &stat );
	if ( stat == MS::kSuccess )
	{
		fprintf( stderr, "Destructible block...\n" );
		MItDag itDAG;
		itDAG.reset( object, MItDag::kBreadthFirst, MFn::kMesh );
		for ( ; !itDAG.isDone(); itDAG.next() )
		{
			MDagPath path;
			itDAG.getPath( path );
			MFnDagNode dagNode( path );
			if ( dagNode.isIntermediateObject() )
				continue;
			int nDepth = itDAG.depth();
			switch ( nDepth )
			{
				case 2: // sub block
					addMeshPaths.append( path );
					nPieces.push_back( 0 );
					break;
				case 3: // destruct piece
					addMeshPaths.append( path );
					nPieces.push_back( 1 );
					break;
			}
		}
	}
	else
	{
		fprintf( stderr, "Finding locators and spheres...\n" );
		MItDag itDAG;
		itDAG.reset( object, MItDag::kBreadthFirst, MFn::kLocator );
		for ( ; !itDAG.isDone(); itDAG.next() )
		{
			MDagPath path;
			itDAG.getPath( path );
			if ( itDAG.depth() == 2 )
			{
				MFnDagNode dagNode( path );
				MDagPath tempPath;
				MDagPath::getAPathTo( dagNode.parent(0), tempPath );
				addLocators.append( tempPath );
			}
		}
		itDAG.reset( object, MItDag::kBreadthFirst, MFn::kNurbsSurface );
		for ( ; !itDAG.isDone(); itDAG.next() )
		{
			MDagPath path;
			itDAG.getPath( path );
			if ( itDAG.depth() == 2 )
			{
				MFnDagNode dagNode( path );
				MDagPath tempPath;
				MDagPath::getAPathTo( dagNode.parent(0), tempPath );
				addSpheres.append( tempPath );
			}
		}
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetExportedParticles()
{
	MSelectionList selection;
 	MGlobal::getActiveSelectionList( selection );
	MItSelectionList iterSL( selection );
	MObject object;
	for ( ; !iterSL.isDone(); iterSL.next() )
	{
		iterSL.getDependNode( object );
		if ( object.apiType() == MFn::kSet )
			particleObjects.append( object );
	}
	if ( particleObjects.length() == 0 )
	{
		fprintf( stderr, "Error: Nothing to export.\n");
		return MS::kFailure;
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetJointsData()
{
	int i, j, nJoint, nTotalJoints = oJoints.length(), nAddBones = addBones.size();
	// names
	// skeleton
	for ( nJoint = 0; nJoint < nTotalJoints; ++nJoint )
	{
		SJoint joint;
		MFnDependencyNode node( oJoints[nJoint] );
		joint.szName = node.name().asChar();
		joint.nParent = -1;
		NConverter::joints.push_back( joint );
	}
	// additional bones
	for ( nJoint = 0; nJoint < nAddBones; ++nJoint )
	{
		SJoint joint;
		MFnDependencyNode node( addBones[nJoint].bonePath.node() );
		joint.szName = node.name().asChar();
		joint.nParent = -1;
		NConverter::joints.push_back( joint );
	}

	// hierarchy
	// skeleton
	for ( nJoint = 1; nJoint < nTotalJoints; ++nJoint )
	{
		MStatus stat;
		MFnDagNode parentNode( oJoints[nJoint] );
		MObject oTmpParent = parentNode.parent(0,&stat);
		if ( stat == MS::kSuccess )
		{
			parentNode.setObject( oTmpParent );
			while ( oTmpParent.apiType() != MFn::kJoint )
			{
				oTmpParent = parentNode.parent(0,&stat);
				if ( stat != MS::kSuccess )
					break;
				parentNode.setObject( oTmpParent );
			}
		}

		for ( int i = 0; i < nTotalJoints; ++i )
		{
			if ( oJoints[i] == parentNode.object() && i != nJoint )
			{
				NConverter::joints[nJoint].nParent = i;
				break;
			}
		}
	}
	
	// additional bones
	for ( nJoint = 0; nJoint < nAddBones; ++nJoint )
	{
		double fMaxWeight = -1000.0f;
		SAdditionalBone &bone = addBones[nJoint];
		for ( int i=0; i<bone.plugs.size(); ++i )
		{
			double w = 0;
			if ( bone.plugs[i].getValue(w) != MS::kSuccess )
			{
				fprintf( stderr, "Oops!\n" );
			}
			if ( w > fMaxWeight )
			{
				fMaxWeight = w;
				NConverter::joints[ nTotalJoints + nJoint ].nParent = bone.nJoints[i];
			}
		}
	}

//	FILE *f = fopen( "c:\\out.txt", "wt" );

	for ( nJoint = 0; nJoint < NConverter::joints.size(); ++nJoint )
	{
		MDagPath path;
		MFnTransform nodeJoint;

		SJoint &joint = NConverter::joints[nJoint];

//		fprintf( f, "%s\n", joint.szName.c_str() );

		if ( nJoint < nTotalJoints )
		{
			nodeJoint.setObject( oJoints[nJoint] );
			nodeJoint.getPath(path);
			double scale[3];
			nodeJoint.getScale( scale );
			joint.scale.x = scale[0];
			joint.scale.y = scale[1];
			joint.scale.z = scale[2];
		}
		else
		{
			path = addBones[ nJoint - nTotalJoints ].bonePath;
			joint.scale.x = joint.scale.y = joint.scale.z = 1.0f;
		}

		// current pose
		MMatrix mxCurPose = path.inclusiveMatrix();

		//OutputMatrix( "CurPose: ", mxCurPose, f );

		// convertion to root CS is not needed
		//if ( nJoint == 0 )
		//	mxRootInverse = mxCurPose.inverse();

		// scaling correction
		if ( nJoint < nTotalJoints )
		{
			MMatrix mxScale;
			mxScale[0][0] = 1 / joint.scale.x;
			mxScale[1][1] = 1 / joint.scale.y;
			mxScale[2][2] = 1 / joint.scale.z;
			mxCurPose = mxScale * mxCurPose;
		}
		//
		mxCurPoses.push_back( mxCurPose );

		// default position
		int nParent = joint.nParent;
		if ( nParent >= 0 )
			mxCurPose = mxCurPose * mxCurPoses[ nParent ].inverse();
		joint.pos.x = mxCurPose.matrix[3][0];
		joint.pos.y = mxCurPose.matrix[3][1];
		joint.pos.z = mxCurPose.matrix[3][2];
		mxCurPose.matrix[3][0] = 0;
		mxCurPose.matrix[3][1] = 0;
		mxCurPose.matrix[3][2] = 0;
		MQuaternion q;
		q = mxCurPose;
		joint.rot.x = q.x;
		joint.rot.y = q.y;
		joint.rot.z = q.z;
		joint.rot.w = q.w;

		// no bind pose for additional bones
		if ( nJoint >= nTotalJoints )
			continue;

		// bind pose
		SHMatrix m;
		MStatus stat;
		MPlug plug = nodeJoint.findPlug( MString("bindPose"), &stat );
		if ( stat != MS::kSuccess )
			fprintf( stderr, "Warning: No bindPose for joint %s", nodeJoint.name().asChar() );
		MObject res;
		plug.getValue( res );
		MFnMatrixData data( res );

		// convertion to root CS is not needed
		//MMatrix mx = data.matrix() * mxRootInverse;

		MMatrix mx = data.matrix();

		//OutputMatrix( "BindPose: ", mx, f );

		// scaling correction
		{
			MMatrix mxScale;
			mxScale[0][0] = 1 / joint.scale.x;
			mxScale[1][1] = 1 / joint.scale.y;
			mxScale[2][2] = 1 / joint.scale.z;
			mx = mxScale * mx;
		}
		//
		mxBindPoses.push_back( mx );
		mx = mx.inverse();
		//
		for ( i = 0; i < 4; ++i )
			for ( j = 0; j < 4; ++j )
				*( &m._11 + i * 4 + j ) = mx.matrix[j][i];
		NConverter::binds.push_back( m );
	}

	//fclose(f);

	// convertion to root CS is not needed
	//for ( nJoint = 0; nJoint < NConverter::joints.size(); ++nJoint )
	//	mxCurPoses[nJoint] = mxCurPoses[nJoint] * mxRootInverse;

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetGeometryData()
{
	int i, j, count;
	int nTotalInfs = nJointIndices.size();
	MFnMesh mesh( meshPath );
	count = mesh.numVertices();
	NConverter::gv.clear();
	for ( i = 0; i < count; i++ )
	{
		MPoint p;
		mesh.getPoint( i, p, MSpace::kWorld );
		// convertion to root CS is not needed
		//p *= mxRootInverse;
		// convert to bind pose mesh
		MMatrix mxRes;
		memset( &mxRes, 0, sizeof(MMatrix) );
		for ( j = 0; j < nTotalInfs; ++j )
		{
			float fW = fWeightsArray[ i * nTotalInfs + j ];
			if ( fW != 0 )
			{
				int index = nJointIndices[j];
				MMatrix mxInf = mxBindPoses[ index ].inverse() * mxCurPoses[ index ];
				mxInf *= fW;
				mxRes += mxInf;
			}
		}
		mxRes = mxRes.inverse();
		p *= mxRes;
		CVec3 plot(p.x, p.y, p.z);
		NConverter::gv.push_back( plot );
	}
	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetShaderData()
{
//	path.extendToShape();
	int instanceNum = 0;
	if ( meshPath.isInstanced() )
		instanceNum = meshPath.instanceNumber();
	MFnMesh mesh( meshPath );
	MItMeshPolygon itPoly( meshPath );
	MObjectArray sets;
	MIntArray indices;
	mesh.getConnectedShaders( instanceNum, sets, indices );
	if ( sets.length() < 1 )
	{
		fprintf( stderr, "Error: No shaders assigned to mesh %s!\n", mesh.name().asChar() );
		return MS::kFailure;
	}
	vector<int> replace;
	int i;
	for ( i = 0; i < sets.length(); ++i )
	{
		int nMat = 0;
		MStatus stat;
		MFnDependencyNode node( sets[i] );
		MPlug plug = node.findPlug( MString("materialIndex"), &stat );
		if ( stat != MS::kSuccess )
		{
			fprintf( stderr, "Warning: Shader %s does not have attribute \"materialIndex\"!\n", node.name().asChar() );
		}
		else
			plug.getValue( nMat );
		if ( nMat < 0 || nMat > NUM_MATERIALS )
			nMat = 0;
		replace.push_back( nMat );
	}
	if ( itPoly.count() != indices.length() )
	{
		fprintf( stderr, "Error: Different number of faces in MItMeshPolygon and getConnectedShaders for %s!\n", mesh.name().asChar() );
		return MS::kFailure;
	}
	for ( i = 0; i < indices.length(); ++i )
	{
		int nSet = indices[i];
		if ( nSet < 0 || nSet > replace.size() )
			nSetIndices.push_back(0);
		else
			nSetIndices.push_back( replace[nSet] );
	}
	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetAddMeshesData()
{
	int nXLimit = 2, nYLimit = 2, nZLimit = 4;
	for ( int i = 0; i < addMeshPaths.length(); ++i )
	{
		MFnDagNode mesh( addMeshPaths[i] );
		MObject obj = mesh.parent(0);
		if ( obj.apiType() != MFn::kTransform )
		{
			fprintf( stderr, "Error: Mesh %s has no transform parent!\n", mesh.name().asChar() );
			return MS::kFailure;
		}
		MDagPath tempPath;
		MDagPath::getAPathTo( obj, tempPath );
		MFnTransform transform( tempPath );
		MPoint pivot = transform.rotatePivot( MSpace::kWorld );
		MPoint pivotParent = pivot;
		if ( nPieces[i] )
		{
			obj = transform.parent(0);
			if ( obj.apiType() != MFn::kTransform )
			{
				fprintf( stderr, "Error: Transform %s has no transform parent!\n", transform.name().asChar() );
				return MS::kFailure;
			}
			MDagPath::getAPathTo( obj, tempPath );
			MFnTransform transformP( tempPath );
			pivotParent = transformP.rotatePivot( MSpace::kWorld );
		}
		MPoint snap;
		pivotParent.x /= 0.625f;
		pivotParent.y /= 0.625f;
		pivotParent.z /= 0.625f;
		snap.x = floor(pivotParent.x);
		snap.y = floor(pivotParent.y);
		snap.z = floor(pivotParent.z);
		if ( pivotParent.x - snap.x >= 0.5f )
			++snap.x;
		if ( pivotParent.y - snap.y >= 0.5f )
			++snap.y;
		if ( pivotParent.z - snap.z >= 0.5f )
			++snap.z;
		int nSubX = (int)(snap.x / 2);
		int nSubY = (int)(snap.y / 2);
		int nSubZ = (int)(snap.z / 4);
		if ( nPieces[i] )
		{
			pivot.x /= 0.625f;
			pivot.y /= 0.625f;
			pivot.z /= 0.625f;
			snap.x = floor(pivot.x);
			snap.y = floor(pivot.y);
			snap.z = floor(pivot.z);
			if ( pivot.x - snap.x >= 0.5f )
				++snap.x;
			if ( pivot.y - snap.y >= 0.5f )
				++snap.y;
			if ( pivot.z - snap.z >= 0.5f )
				++snap.z;
			snap.x -= nSubX * 2;
			snap.y -= nSubY * 2;
			snap.z -= nSubZ * 4;
			pivot.x -= nSubX * 2;
			pivot.y -= nSubY * 2;
			pivot.z -= nSubZ * 4;
			snap.x = __max( 0, __min(snap.x, nXLimit) );
			snap.y = __max( 0, __min(snap.y, nYLimit) );
			snap.z = __max( 0, __min(snap.z, nZLimit) );
			if ( (int(snap.x + snap.y + snap.z) & 1) == 0 )
			{
				MPointArray knots;
				if ( snap.x < nXLimit )
					knots.append( snap + MPoint(1,0,0) );
				if ( snap.x > 0 )
					knots.insert( snap + MPoint(-1,0,0), rand() % (knots.length()+1) );
				if ( snap.y < nYLimit )
					knots.insert( snap + MPoint(0,1,0), rand() % (knots.length()+1) );
				if ( snap.y > 0 )
					knots.insert( snap + MPoint(0,-1,0), rand() % (knots.length()+1) );
				if ( snap.z < nZLimit )
					knots.insert( snap + MPoint(0,0,1), rand() % (knots.length()+1) );
				if ( snap.z > 0 )
					knots.insert( snap + MPoint(0,0,-1), rand() % (knots.length()+1) );
				double fMinLength = 2.f;
				for ( int j=0; j<knots.length(); ++j )
					if ( knots[j].distanceTo(pivot) < fMinLength )
					{
						fMinLength = knots[j].distanceTo(pivot);
						snap = knots[j];
					}
			}
			nPieces[i] = (int(snap.x+1)) | (int(snap.y+1) << 2) | (int(snap.z+1) << 4);
		}
		nPieces[i] |= ((nSubX+1) << 7) | ((nSubY+1) << 10) | ((nSubZ+1) << 13);
		// ZYXzyx
		// 333322
		nPieces[i] <<= 4;
	}
	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSortWeight
{
	int n;
	float w;
};
bool CompareWeights( const SSortWeight &a, const SSortWeight &b )
{
	return a.w > b.w;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetSkinClusterData()
{
	MStatus stat;
	MFnSkinCluster cluster( oSkin );
	MDagPathArray infs;

	int nTotalInfs = cluster.influenceObjects( infs, &stat );
	if ( nTotalInfs == 0 )
	{
		fprintf( stderr, "Error: No influence objects found for cluster %s!\n", cluster.name().asChar() );
		return MS::kFailure;
	}

	nJointIndices.clear();
	for ( int nInf = 0; nInf < nTotalInfs; ++nInf )
	{
		MFnDependencyNode node( infs[ nInf ].node() );
		const char *pszName = node.name().asChar();
		for ( int i = 0; i < NConverter::joints.size(); ++i )
			if ( strcmp( pszName, NConverter::joints[i].szName.c_str() ) == 0 )
			{
				nJointIndices.push_back( i );
				break;
			}
	}
	if ( nJointIndices.size() != nTotalInfs )
	{
		fprintf( stderr, "Error: Found dependency on non-exported joint for cluster %s!\n", cluster.name().asChar() );
		return MS::kFailure;
	}

	int nTotalGeoms = cluster.numOutputConnections();
	if ( nTotalGeoms != 1 )
	{
		fprintf( stderr, "Error: There should be only one influenced geometry in cluster %s!\n", cluster.name().asChar() );
		return MS::kFailure;
	}

	int index = cluster.indexForOutputConnection( 0 );
	cluster.getPathAtIndex( index, meshPath );
	MItGeometry itGeom( meshPath );
	MFnMesh mesh( meshPath );
	if ( itGeom.count() != mesh.numVertices() )
	{
		fprintf( stderr, "Error: Different number of vertices in MFnMesh and MItGeometry for mesh %s!\n", mesh.name().asChar() );
		return MS::kFailure;
	}

	// getting skin cluster weights
	fWeightsArray.clear();
	for ( ; !itGeom.isDone(); itGeom.next() )
	{
		MObject comp = itGeom.component();
		MFloatArray fWeights;
		unsigned int nInfCount;
		stat = cluster.getWeights( meshPath, comp, fWeights, nInfCount );
		if ( fWeights.length() != nTotalInfs )
		{
			fprintf( stderr, "Error: MFnSkinCluster::getWeights returned wrong number of weights for mesh %s!\n", mesh.name().asChar() );
			return MS::kFailure;
		}
		// sorting weights and normalizing (drop all above 4)
		vector<SSortWeight> sorted;
		for ( int i = 0; i < fWeights.length(); ++i )
		{
			SSortWeight sw;
			sw.n = i;
			sw.w = fWeights[i];
			sorted.push_back( sw );
		}
		std::sort( sorted.begin(), sorted.end(), CompareWeights );
		float fSumWeights = 0;
		for ( i = 0; i < sorted.size(); ++i )
		{
			if ( i > 3 || sorted[i].w < 1e-3f )
				fWeights[ sorted[i].n ] = 0;
			else
				fSumWeights += sorted[i].w;
		}
		if ( fabs(fSumWeights) < 1e-3f )
		{
			fprintf( stderr, "Error: Sum of weights is about zero for mesh %s!\n", mesh.name().asChar() );
			return MS::kFailure;
		}
		for ( i = 0; i < fWeights.length(); ++i )
			fWeightsArray.push_back( fWeights[i] / fSumWeights );
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetAdditionalBonesData()
{
	MObject temp;
	if ( GetSelection( &temp ) != MS::kSuccess )
		return MS::kFailure;
	MFnDependencyNode tempNode( temp );

	MDagPathArray addBonesPaths;
	MString szSetName;
	szSetName = tempNode.name().asChar();
	szSetName += "_Effectors";
	
	MItDependencyNodes itDG( MFn::kSet ); // for each set in scene
	for ( ; !itDG.isDone(); itDG.next() )
	{
		MFnSet setFn( itDG.item() );
		if ( setFn.name() == szSetName )
		{
			MSelectionList selList;
			setFn.getMembers( selList, false );
			for ( int i = 0; i < selList.length(); ++i )
			{
				MDagPath path;
				selList.getDagPath( i, path );
				addBonesPaths.append( path );
			}
		}
	}

	for ( int nBone = 0; nBone < addBonesPaths.length(); ++nBone )
	{
		/*
		MFnDagNode nodeMesh( addBonesPaths[nBone] );
		if ( nodeMesh.parentCount() != 1 )
		{
			fprintf( stderr, "Error: Mesh %s has illegal number of parents!\n", nodeMesh.name().asChar() );
			return MS::kFailure;
		}
		if ( nodeMesh.parent(0).apiType() != MFn::kTransform )
		{
			fprintf( stderr, "Warning: Parent of mesh %s is not a transform node!\n", nodeMesh.name().asChar() );
			return MS::kFailure;
		}
		MFnDagNode nodeTransform( nodeMesh.parent(0) );
		*/
		MFnDagNode nodeTransform( addBonesPaths[nBone] );
		if ( nodeTransform.parentCount() != 1 )
		{
			fprintf( stderr, "Error: Transform %s has illegal number of parents!\n", nodeTransform.name().asChar() );
			return MS::kFailure;
		}
		if ( nodeTransform.parent(0).apiType() != MFn::kTransform )
		{
			fprintf( stderr, "Warning: Parent of transform %s is not a transform node!\n", nodeTransform.name().asChar() );
			return MS::kFailure;
		}
		addBones.push_back();
		SAdditionalBone &bone = addBones[ addBones.size() - 1 ];
		nodeTransform.getPath( bone.bonePath );
		MFnDependencyNode nodeLocator( nodeTransform.parent(0) );

		MStatus status;
		int i, j;
		MPlugArray plugs, inputs;
		if ( nodeLocator.getConnections( plugs ) != MS::kSuccess )
		{
			fprintf( stderr, "Error: Transform %s has no plugs!\n", nodeLocator.name().asChar() );
			return MS::kFailure;
		}
		MObject constr;
		for ( i=0; i<plugs.length(); ++i )
			if ( plugs[i].connectedTo( inputs, true, false ) && inputs.length() > 0 )
			{
				MFn::Type type = inputs[0].node().apiType();
				if ( type == MFn::kPointConstraint || type == MFn::kOrientConstraint )
				{
					constr = inputs[0].node();
					break;
				}
			}
		if ( constr.isNull() )
		{
			fprintf( stderr, "Error: Locator %s has neither point nor orient constraint!\n", nodeLocator.name().asChar() );
			return MS::kFailure;
		}
		MFnDependencyNode nodeConstr(constr);
		MPlug plugTarget = nodeConstr.findPlug( "target", &status );
		if ( status != MS::kSuccess || !plugTarget.isArray() )
		{
			fprintf( stderr, "Error: Constraint %s has no target plug!\n", nodeConstr.name().asChar() );
			return MS::kFailure;
		}
		int nTargets = plugTarget.numElements();
		for ( i=0; i<plugTarget.numElements(); ++i ) // for each target
		{
			if ( !plugTarget[i].isCompound() )
			{
				fprintf( stderr, "Error: Plug %s is not compound!\n", plugTarget[i].name().asChar() );
				return MS::kFailure;
			}

			int nChildren = plugTarget[i].numChildren();
			for ( j=0; j<nChildren; ++j )
			{
				MPlug child = plugTarget[i].child(j);
				MStringArray str;
				child.name().split( '.', str );
				if ( str[ str.length() - 1 ] == "targetWeight" ) // if "weight" plug
				{
					bone.plugs.push_back( child );
				}
				else if ( bone.nJoints.length() <= i )
				{
					if ( !child.connectedTo( plugs, true, false ) )
					{
						fprintf( stderr, "Error: Plug %s is not connected to anything!\n", child.name().asChar() );
						return MS::kFailure;
					}
					MObject target = plugs[0].node();
					if ( target.apiType() != MFn::kJoint ) // should be world effector
						bone.nJoints.append( -1 );
					else
					{
						int nResJoint = -1;
						for ( int nJoint = 0; nJoint < oJoints.length(); ++nJoint )
							if ( oJoints[ nJoint ] == target )
							{
								nResJoint = nJoint;
								break;
							}
						if ( nResJoint == -1 )
						{
							fprintf( stderr, "Error: Plug %s is connected to joint node not from skeleton!\n", child.name().asChar() );
							return MS::kFailure;
						}
						else
							bone.nJoints.append( nResJoint );
					}
				}
			}

			if ( bone.nJoints.length() != i+1 || bone.plugs.size() != i+1 )
			{
				fprintf( stderr, "Error: Plug %s has no targetWeight!\n", plugTarget[i].name().asChar() );
				return MS::kFailure;
			}
		}

	}
	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetColorsData()
{
	MFnMesh mesh( meshPath );
	MColorArray colors;
	mesh.getVertexColors( colors );
	int count = colors.length();
	if ( count != mesh.numVertices() )
	{
		fprintf( stderr, "Error: Different number of colors and vertices for mesh %s!\n", mesh.name().asChar() );
		return MS::kFailure;
	}
	for ( int i = 0; i < count; ++i )
	{
		if ( colors[i].r == -1 || colors[i].g == -1 || colors[i].b == -1 )
		{
			colors[i].r = 1;
			colors[i].g = 1;
			colors[i].b = 1;
			colors[i].a = 1;
		}
		int r, g, b, a;
		r = FindCloseValue( colors[i].r * 255, 0, 255 );
		g = FindCloseValue( colors[i].r * 255, 0, 255 );
		b = FindCloseValue( colors[i].r * 255, 0, 255 );
		a = FindCloseValue( colors[i].r * 255, 0, 255 );
		DWORD pixel = b | (g << 8) | (r << 16) | (a << 24);
		dwColors.push_back( pixel );
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void A5ExportModel::SolveDuDv( MVector &normal, MVector &side1, MVector &side2, float dU1, float dU2, MVector *pRes )
{
	// basic vectors in plane
	MVector b1;
	if ( fabs(normal.x) < 1e-6f )
		b1 = MVector( 1, 0, 0 );
	else
		b1 = MVector( normal.y, -normal.x, 0 );
	b1.normalize();
	MVector b2 = b1 ^ normal;
	// matrix 2x2 solving
	double fA = b1 * side1;
	double fB = b2 * side1;
	double fC = b1 * side2;
	double fD = b2 * side2;

//	OutputVector( "normal ", normal );
//	OutputVector( "b1 ", b1 );
//	OutputVector( "b2 ", b2 );
//	OutputVector( "side1 ", side1 );
//	OutputVector( "side2 ", side2 );

	double det = fA * fD - fB * fC;

//	fprintf( stderr, "det=%f   a=%f b=%f c=%f d=%f\n", det, fA, fB, fC, fD );

	double fAr = fD / det;
	double fBr = - fB / det;
	double fCr = - fC / det;
	double fDr = fA / det;
	double fCoeff1 = fAr * dU1 + fBr * dU2;
	double fCoeff2 = fCr * dU1 + fDr * dU2;
	*pRes = fCoeff1 * b1 + fCoeff2 * b2;

//	fprintf( stderr, "%f = %f; %f = %f\n", (*pRes) * side1, dU1, (*pRes) * side2, dU2 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void A5ExportModel::CalculateDuDv()
{
	const int n = 402;
	int i;
	int count = NConverter::verts.size();
	MFloatVectorArray dUs, dVs;
	for ( i = 0; i < count; i++ )
	{
		dUs.append( MFloatVector::zero );
		dVs.append( MFloatVector::zero );
	}
	
	for ( std::hash_map< int, SFacesVector >::const_iterator it = NConverter::faces.begin(); 
		it != NConverter::faces.end(); ++it )
	for ( i = 0; i < it->second.faces.size(); ++i )
	{
		const SFace &face = it->second.faces[i];
//		fprintf( stderr, "%d: %d %d %d\n", i, face.nA, face.nB, face.nC );

		SVertex &pp1 = NConverter::verts[ face.i1 ];
		SVertex &pp2 = NConverter::verts[ face.i2 ];
		SVertex &pp3 = NConverter::verts[ face.i3 ];
		MVector p12( pp2.gv.x - pp1.gv.x, pp2.gv.y - pp1.gv.y, pp2.gv.z - pp1.gv.z );
		MVector p23( pp3.gv.x - pp2.gv.x, pp3.gv.y - pp2.gv.y, pp3.gv.z - pp2.gv.z );
		MVector p31( pp1.gv.x - pp3.gv.x, pp1.gv.y - pp3.gv.y, pp1.gv.z - pp3.gv.z );
		// check special case
		MVector check = p12 ^ p23;
		if ( check.length() < 1e-6f )
			continue;
		float u12 = pp2.tv.x - pp1.tv.x;
		float u23 = pp3.tv.x - pp2.tv.x;
		float u31 = pp1.tv.x - pp3.tv.x;
		float v12 = pp2.tv.y - pp1.tv.y;
		float v23 = pp3.tv.y - pp2.tv.y;
		float v31 = pp1.tv.y - pp3.tv.y;
		MVector dU, dV;
		MVector norm;

		norm = MVector( pp1.nv.x, pp1.nv.y, pp1.nv.z );
		if ( norm.length() > 1e-6f )
		{
			SolveDuDv( norm, p12, p31, u12, u31, &dU );
			dUs[ face.i1 ] += dU;
			SolveDuDv( norm, p12, p31, v12, v31, &dV );
			dVs[ face.i1 ] += dV;
		}
		/*
		if ( face.nA == n )
		{
			OutputVector( "nA   normal ", norm );
			OutputVector( "side1 ", p12 );
			OutputVector( "side2 ", p31 );
			fprintf( stderr, "U1: %f  U2: %f\n", u12, u31 );
			fprintf( stderr, "V1: %f  V2: %f\n", v12, v31 );
			OutputVector( "dU ", dU );
			OutputVector( "dV ", dV );
		}
		*/
		norm = MVector( pp2.nv.x, pp2.nv.y, pp2.nv.z );
		if ( norm.length() > 1e-6f )
		{
			SolveDuDv( norm, p12, p23, u12, u23, &dU );
			dUs[ face.i2 ] += dU;
			SolveDuDv( norm, p12, p23, v12, v23, &dV );
			dVs[ face.i2 ] += dV;
		}
		/*
		if ( face.nB == n )
		{
			OutputVector( "nA   normal ", norm );
			OutputVector( "side1 ", p12 );
			OutputVector( "side2 ", p23 );
			fprintf( stderr, "U1: %f  U2: %f\n", u12, u23 );
			fprintf( stderr, "V1: %f  V2: %f\n", v12, v23 );
			OutputVector( "dU ", dU );
			OutputVector( "dV ", dV );
		}
		*/
		norm = MVector( pp3.nv.x, pp3.nv.y, pp3.nv.z );
		if ( norm.length() > 1e-6f )
		{
			SolveDuDv( norm, p23, p31, u23, u31, &dU );
			dUs[ face.i3 ] += dU;
			SolveDuDv( norm, p23, p31, v23, v31, &dV );
			dVs[ face.i3 ] += dV;
		}
		/*
		if ( face.nC == n )
		{
			OutputVector( "nA   normal ", norm );
			OutputVector( "side1 ", p23 );
			OutputVector( "side2 ", p31 );
			fprintf( stderr, "U1: %f  U2: %f\n", u23, u31 );
			fprintf( stderr, "V1: %f  V2: %f\n", v23, v31 );
			OutputVector( "dU ", dU );
			OutputVector( "dV ", dV );
		}
		*/
	}

	float fSum = 0;
	for ( i = 0; i < count; ++i )
	{
		MVector dU( dUs[i] );
		MVector dV( dVs[i] );
		if ( dU.length() < 1e-6f )
			dU = MVector(0,0,0);
		else
			dU.normalize();
		if ( dV.length() < 1e-6f )
			dV = MVector(0,0,0);
		else
			dV.normalize();
		float scal = dU * dV;
		NConverter::verts[i].du.x = dU.x;
		NConverter::verts[i].du.y = dU.y;
		NConverter::verts[i].du.z = dU.z;
		NConverter::verts[i].dv.x = dV.x;
		NConverter::verts[i].dv.y = dV.y;
		NConverter::verts[i].dv.z = dV.z;
		NConverter::verts[i].dwColor = dwColors[ NConverter::links[i] ];
		fSum += fabs(scal);
	}
	fSum /= count;
	fprintf( stderr, "Info: Average dU * dV: %f\n", fSum );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetSpheresData()
{ 
	// mass spheres for collision etc
	for ( int i = 0; i < addSpheres.length(); ++i )
	{
		MStatus stat;
		SSphereMass sphere;
		MFnDependencyNode node( addSpheres[i].node() );
		
		MFnTransform trans( addSpheres[i] );
		MPoint pt = trans.rotatePivot( MSpace::kWorld );
		//OutputPoint( "Sphere: ", pt );
		sphere.point.x = pt.x;
		sphere.point.y = pt.y;
		sphere.point.z = pt.z;

		/*
		MMatrix mx = addSpheres[i].inclusiveMatrix();
		sphere.point.x = mx.matrix[3][0];
		sphere.point.y = mx.matrix[3][1];
		sphere.point.z = mx.matrix[3][2];
		*/

		sphere.fRadius = 1;
		MPlug plugR = node.findPlug( MString("radius"), &stat );
		if ( stat != MS::kSuccess )
		{
			fprintf( stderr, "Error: Node %s does not have attribute \"radius\"!\n", node.name().asChar() );
			return MS::kFailure;
		}
		else
		{
			plugR.getValue( sphere.fRadius );
			if ( sphere.fRadius <= 0 )
			{
				fprintf( stderr, "Warning: Node %s have illegal value in attribute \"radius\"!\n", node.name().asChar() );
				sphere.fRadius = 1;
			}
		}

		sphere.fMass = 1;
		MPlug plugM = node.findPlug( MString("mass"), &stat );
		if ( stat != MS::kSuccess )
		{
			fprintf( stderr, "Error: Node %s does not have attribute \"mass\"!\n", node.name().asChar() );
			return MS::kFailure;
		}
		else
		{
			plugM.getValue( sphere.fMass );
			if ( sphere.fMass <= 0 )
			{
				fprintf( stderr, "Warning: Node %s have illegal value in attribute \"mass\"!\n", node.name().asChar() );
				sphere.fMass = 1;
			}
		}

		NConverter::spheres.push_back( sphere );
	}
	float fSumMass = 0;
	for ( i = 0; i < NConverter::spheres.size(); ++i )
		fSumMass += NConverter::spheres[i].fMass;
	if ( fSumMass == 0 )
		fSumMass = 1;
	for ( i = 0; i < NConverter::spheres.size(); ++i )
		NConverter::spheres[i].fMass /= fSumMass;
	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EExportFlags
{
	FULL_VERTEX = 1,
	ADD_MESHES = 2,
	SKIN_WEIGHTS = 4,
};
void A5ExportModel::ExportWithMaterialFilter( int nFlags )
{
	char pszName[1024];
	for ( int nMat = 0; nMat < NUM_MATERIALS; ++nMat )
	{
		hash_map< int, SMeshPiece<SVertex> > piecesFull;
		hash_map< int, SMeshPiece<CVec3> > piecesSimp;
		hash_map< int, SConvertedMeshPiece<SVertex> > piecesFullConv;

		int nFaces = 0;
		// filtering
		for ( std::hash_map< int, SFacesVector >::const_iterator it = NConverter::faces.begin(); 
			it != NConverter::faces.end(); ++it )
		{
			if ( (it->first & 0xF) != nMat )
				continue;
			hash_map< int, int > links;
			vector<int> linksRev;
			SMeshPiece<SVertex> &pieceFull = piecesFull[ it->first >> 4 ];
			SMeshPiece<CVec3> &pieceSimp = piecesSimp[ it->first >> 4 ];
			for ( int i = 0; i < it->second.faces.size(); ++i )
			{
				SFace face;
				int nV;
				nV = it->second.faces[i].i1;
				if ( links.find(nV) == links.end() )
				{
					links[nV] = (nFlags & FULL_VERTEX) ? pieceFull.verts.size() : pieceSimp.verts.size();
					linksRev.push_back(nV);
					if ( nFlags & FULL_VERTEX )
						pieceFull.verts.push_back( NConverter::verts[nV] );
					else
						pieceSimp.verts.push_back( NConverter::gv[nV] );
				}
				face.i1 = links[nV];
				nV = it->second.faces[i].i2;
				if ( links.find(nV) == links.end() )
				{
					links[nV] = (nFlags & FULL_VERTEX) ? pieceFull.verts.size() : pieceSimp.verts.size();
					linksRev.push_back(nV);
					if ( nFlags & FULL_VERTEX )
						pieceFull.verts.push_back( NConverter::verts[nV] );
					else
						pieceSimp.verts.push_back( NConverter::gv[nV] );
				}
				face.i2 = links[nV];
				nV = it->second.faces[i].i3;
				if ( links.find(nV) == links.end() )
				{
					links[nV] = (nFlags & FULL_VERTEX) ? pieceFull.verts.size() : pieceSimp.verts.size();
					linksRev.push_back(nV);
					if ( nFlags & FULL_VERTEX )
						pieceFull.verts.push_back( NConverter::verts[nV] );
					else
						pieceSimp.verts.push_back( NConverter::gv[nV] );
				}
				face.i3 = links[nV];
				if ( nFlags & FULL_VERTEX )
					pieceFull.faces.push_back( face );
				else
					pieceSimp.faces.push_back( face );
				++nFaces;
			}

			if ( nFlags & SKIN_WEIGHTS )
			{
				NConverter::weights.clear();
				// converting weights to file format
				int nTotalInfs = nJointIndices.size();
				for ( int i = 0; i < linksRev.size(); ++i )
				{
					int nOrigVertex = NConverter::links[ linksRev[i] ];
					for ( int j = 0; j < nTotalInfs; ++j )
					{
						float fW = fWeightsArray[ nOrigVertex * nTotalInfs + j ];
						if ( fW == 0 )
							continue;
						SVertexWeight w = { fW, i, nJointIndices[j] };
						NConverter::weights.push_back( w );
					}
				}
			}

			if ( (it->first >> 4) != 0 )
			{
				int nPiece = it->first >> 4;
				int nSubX = ((nPiece >> 7) & 0x7) - 1;
				int nSubY = ((nPiece >> 10) & 0x7) - 1;
				int nSubZ = ((nPiece >> 13) & 0x7) - 1;
				CVec3 shift( -nSubX * 1.25f, -nSubY * 1.25f, -nSubZ * 2.5f );
				if ( nFlags & FULL_VERTEX )
				{
					for ( int i = 0; i < pieceFull.verts.size(); ++i )
						pieceFull.verts[i].gv += shift;
				}
				else
				{
					for ( int i = 0; i < pieceSimp.verts.size(); ++i )
						pieceSimp.verts[i] += shift;
				}
				/*
				if ( (nPiece & 0x7F) )//&& pieceFull.faces.size() == 3 && (it->first & 0xF) == 3 )
				{
					int nX = ((nPiece >> 0) & 0x3) - 1;
					int nY = ((nPiece >> 2) & 0x3) - 1;
					int nZ = ((nPiece >> 4) & 0x7) - 1;
					fprintf( stderr, "Sub: %d %d %d;  Index: %d %d %d;  Mat: %d;  Faces: %d\n", nSubX, nSubY, nSubZ, nX, nY, nZ, it->first & 0xF, pieceFull.faces.size() );
				}
				*/
			}

			if ( nFlags & FULL_VERTEX )
			{
				SConvertedMeshPiece<SVertex> &pieceFullConv = piecesFullConv[ it->first >> 4 ];
				pieceFullConv.verts = pieceFull.verts;
				if ( nFlags & SKIN_WEIGHTS )
					ConvertTriangles( pieceFull.verts, pieceFull.faces, &pieceFullConv.indices, &pieceFullConv.polys );
				else
					ConvertPolygons( pieceFull.verts, pieceFull.faces, &pieceFullConv.indices, &pieceFullConv.polys, true );
			}
		}

		if ( nFaces == 0 )
			continue;

		sprintf( pszName, "%s%d", szExportFilePath.c_str(), nFileNumber + (nMat << 16) );
		szCurrentFileName = pszName;

		fp.OpenWrite( szCurrentFileName.c_str() );
		{		
			CStructureSaver file( fp, CStructureSaver::WRITE );
			if ( nFlags & FULL_VERTEX )
			{
				file.Add( 1, &piecesFullConv[0].verts );
				file.Add( 2, &piecesFullConv[0].indices );
				file.Add( 3, &piecesFullConv[0].polys );
			}
			else
			{
				file.Add( 1, &piecesSimp[0].verts );
				file.Add( 2, &piecesSimp[0].faces );
			}
			if ( nFlags & ADD_MESHES )
			{
				if ( nFlags & FULL_VERTEX )
					file.Add( 4, &piecesFullConv );
				else
					file.Add( 4, &piecesSimp );
			}
			if ( nFlags & SKIN_WEIGHTS )
				file.Add( 5, &NConverter::weights );
			if ( NConverter::spheres.size() )
				file.Add( 6, &NConverter::spheres );
		}
		fp.CloseFile();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// export skeleton
MStatus A5ExportModel::ExportSkeleton()
{
	if ( GetExportedJoints() == MS::kFailure )
		return MS::kFailure;

	if ( GetAdditionalBonesData() == MS::kFailure )
		return MS::kFailure;

	if ( GetJointsData() == MS::kFailure )
		return MS::kFailure;

	fp.OpenWrite( szExportFileName.c_str() );
	{
		CStructureSaver file( fp, CStructureSaver::WRITE );
		file.Add( 1, &NConverter::joints );
		file.Add( 2, &bAnimWithScale );
	}
	fp.CloseFile();

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// export skin cluster model
MStatus A5ExportModel::ExportSkin()
{
	int i, count;

	if ( GetExportedJoints() == MS::kFailure )
		return MS::kFailure;

	if ( GetExportedSkin() == MS::kFailure )
		return MS::kFailure;

	if ( GetJointsData() == MS::kFailure )
		return MS::kFailure;

	if ( GetSkinClusterData() == MS::kFailure )
		return MS::kFailure;

	if ( GetGeometryData() == MS::kFailure )
		return MS::kFailure;

	if ( GetShaderData() == MS::kFailure )
		return MS::kFailure;

	if ( GetColorsData() == MS::kFailure )
		return MS::kFailure;

	MFnMesh mesh( meshPath );
	// texture coords
	count = mesh.numUVs();
	for ( i = 0; i < count; i++ )
	{
		float u, v;
		mesh.getUV( i, u, v );
//		if ( u < 0 || u > 1 || v < 0 || v > 1 )
//			fprintf( stderr, "Warning: Texture coords of mesh %s are out of bounds!\n", mesh.name().asChar() );
		CVec3 plot(u, v, 0);
		NConverter::tv.push_back( plot );
	}

	// normals
	count = mesh.numNormals();
	MFloatVectorArray norms;
	//mesh.getNormals( norms, MSpace::kWorld );
	for ( i = 0; i < count; i++ )
		norms.append( MFloatVector::zero );

	MItMeshPolygon polyIter( meshPath );
	int nIndices[9];

	// we take normals to faces which are sharing this normal and take mean value (all in bind pose)
	for ( ; !polyIter.isDone(); polyIter.next() )
	{
		if ( polyIter.polygonVertexCount() != 3 )
		{
			fprintf( stderr, "Error: Mesh %s is needed to be triangulated - found %d vertices in one face!\n",
				mesh.name().asChar(), polyIter.polygonVertexCount() );
			return MS::kFailure;
		}
		for ( int vtx = 0; vtx < 3; vtx++ )
		{
			nIndices[ vtx * 3 ] = polyIter.vertexIndex( vtx );
			nIndices[ vtx * 3 + 1 ] = polyIter.normalIndex( vtx );
		}
		CVec3 &pp1 = NConverter::gv[ nIndices[0] ];
		CVec3 &pp2 = NConverter::gv[ nIndices[3] ];
		CVec3 &pp3 = NConverter::gv[ nIndices[6] ];
		MVector p1( pp1.x, pp1.y, pp1.z );
		MVector p2( pp2.x, pp2.y, pp2.z );
		MVector p3( pp3.x, pp3.y, pp3.z );
		MVector normal = (p2 - p1) ^ (p3 - p1);
		normal.normalize();
		norms[ nIndices[1] ] += normal;
		norms[ nIndices[4] ] += normal;
		norms[ nIndices[7] ] += normal;
	}

	for ( i = 0; i < count; i++ )
	{
		MVector p( norms[i] );
		p.normalize();
		CVec3 plot(p.x, p.y, p.z);
		NConverter::nv.push_back( plot );
	}

	// faces
	int nFace = 0;
	polyIter.reset( meshPath );
	for ( ; !polyIter.isDone(); polyIter.next(), ++nFace )
	{
		// don't have to check triangulancy
		for ( int vtx = 0; vtx < 3; vtx++ )
		{
			nIndices[ vtx * 3 ] = polyIter.vertexIndex( vtx );
			nIndices[ vtx * 3 + 1 ] = polyIter.normalIndex( vtx );
			if ( !polyIter.getUVIndex( vtx, nIndices[ vtx * 3 + 2 ] ) )
			{
				fprintf( stderr, "Error: Mesh %s is needed to be fully mapped!\n", mesh.name().asChar() );
				return MS::kFailure;
			}
		}
		NConverter::AddFace( nSetIndices[nFace], nIndices );
	}

	CalculateDuDv();

	ExportWithMaterialFilter( FULL_VERTEX | SKIN_WEIGHTS );

	char pszName[1024];
	sprintf( pszName, "%s%d", szBindsPath.c_str(), nFileNumber );
	szCurrentFileName = pszName;
	fp.OpenWrite( szCurrentFileName.c_str() );
	{
		CStructureSaver file( fp, CStructureSaver::WRITE );
		file.Add( 4, &NConverter::binds );
	}
	fp.CloseFile();

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// export single mesh and/or attached meshes in mesh(full) format
MStatus A5ExportModel::ExportMesh()
{
	if ( GetExportedMesh() == MS::kFailure )
		return MS::kFailure;	

	if ( GetAddMeshesData() == MS::kFailure )
		return MS::kFailure;	

	int nAllVertices = 0, nAllUVs = 0, nAllNormals = 0;
	int nFace = 0;

	for ( int nMesh = 0; nMesh < 1 + addMeshPaths.length(); ++nMesh )
	{
		int nPiece = 0;
		if ( nMesh > 0 )
		{
			nPiece = nPieces[ nMesh - 1 ];
			meshPath = addMeshPaths[ nMesh - 1 ];
		}

		if ( GetShaderData() == MS::kFailure )
			return MS::kFailure;

		if ( GetColorsData() == MS::kFailure )
			return MS::kFailure;

		MFnMesh mesh( meshPath );

		int i, count;
		count = mesh.numVertices();
		for ( i = 0; i < count; i++ )
		{
			MPoint p;
			mesh.getPoint( i, p, MSpace::kWorld );
			CVec3 plot(p.x, p.y, p.z);
			NConverter::gv.push_back( plot );
		}

		count = mesh.numUVs();
		for ( i = 0; i < count; i++ )
		{
			float u, v;
			mesh.getUV( i, u, v );
//			if ( u < 0 || u > 1 || v < 0 || v > 1 )
//				fprintf( stderr, "Warning: Texture coords of mesh %s are out of bounds!\n", mesh.name().asChar() );
			CVec3 plot(u, v, 0);
			NConverter::tv.push_back( plot );
		}

		count = mesh.numNormals();
		MFloatVectorArray norms;
		mesh.getNormals( norms, MSpace::kWorld );
		for ( i = 0; i < count; i++ )
		{
			if ( norms[i].length() < 1e-6f )
				fprintf( stderr, "Warning: Zero normals encountered for mesh %s!!!\n", mesh.name().asChar() );
			CVec3 plot(norms[i].x, norms[i].y, norms[i].z);
			NConverter::nv.push_back( plot );
		}

		MItMeshPolygon itPoly( meshPath.node() );
		int nIndices[9];
		for ( ; !itPoly.isDone(); itPoly.next(), ++nFace )
		{
			if ( itPoly.polygonVertexCount() != 3 )
			{
				fprintf( stderr, "Error: Mesh %s is needed to be triangulated - found %d vertices in one face!\n",
					mesh.name().asChar(), itPoly.polygonVertexCount() );
				return MS::kFailure;
			}
			for ( int vtx = 0; vtx < 3; vtx++ )
			{
				nIndices[ vtx * 3 ] = itPoly.vertexIndex( vtx ) + nAllVertices;
				nIndices[ vtx * 3 + 1 ] = itPoly.normalIndex( vtx ) + nAllNormals;
				if ( !itPoly.getUVIndex( vtx, nIndices[ vtx * 3 + 2 ] ) )
				{
					fprintf( stderr, "Error: Mesh %s is needed to be fully mapped!\n", mesh.name().asChar() );
					return MS::kFailure;
				}
				nIndices[ vtx * 3 + 2 ] += nAllUVs;
			}
			NConverter::AddFace( nPiece + nSetIndices[nFace], nIndices );
		}

		nAllVertices += mesh.numVertices();
		nAllNormals += mesh.numNormals();
		nAllUVs += mesh.numUVs();
	}

	CalculateDuDv();

	if ( addMeshPaths.length() > 0 )
		ExportWithMaterialFilter( FULL_VERTEX | ADD_MESHES );
	else
		ExportWithMaterialFilter( FULL_VERTEX );

	if ( addLocators.length() > 0 )
	{
		for ( int i=0; i<addLocators.length(); ++i )
		{
			SJoint joint;
			MFnDependencyNode node( addLocators[i].node() );
			joint.szName = node.name().asChar();
			joint.nParent = -1;

			// current pose
			MMatrix mxCurPose = addLocators[i].inclusiveMatrix();

			joint.pos.x = mxCurPose.matrix[3][0];
			joint.pos.y = mxCurPose.matrix[3][1];
			joint.pos.z = mxCurPose.matrix[3][2];
			mxCurPose.matrix[3][0] = 0;
			mxCurPose.matrix[3][1] = 0;
			mxCurPose.matrix[3][2] = 0;
			MQuaternion q;
			q = mxCurPose;
			joint.rot.x = q.x;
			joint.rot.y = q.y;
			joint.rot.z = q.z;
			joint.rot.w = q.w;

			joint.scale.x = joint.scale.y = joint.scale.z = 1.0f;
			NConverter::joints.push_back( joint );
		}
		char pszName[1024];
		sprintf( pszName, "%s%d", szEffPath.c_str(), nFileNumber );
		szCurrentFileName = pszName;
		fp.OpenWrite( szCurrentFileName.c_str() );
		{
			CStructureSaver file( fp, CStructureSaver::WRITE );
			file.Add( 1, &NConverter::joints );
		}
		fp.CloseFile();
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// export AI skin clusters
MStatus A5ExportModel::ExportAISkin()
{
	hash_map<int, SAISkinCluster> clusters;

	if ( GetExportedAISkins() == MS::kFailure )
		return MS::kFailure;

	if ( GetJointsData() == MS::kFailure )
		return MS::kFailure;

	for ( int nCluster = 0; nCluster < oAISkins.length(); ++nCluster )
	{
		oSkin = oAISkins[nCluster];
		MFnDependencyNode node( oSkin );

		MStatus stat;
		int nBodyPart;
		MPlug plugBP = node.findPlug( MString("bodyPart"), &stat );
		if ( stat != MS::kSuccess )
		{
			fprintf( stderr, "Error: Node %s does not have attribute \"bodyPart\"!\n", node.name().asChar() );
			return MS::kFailure;
		}
		else
			plugBP.getValue( nBodyPart );

		if ( clusters.find(nBodyPart) != clusters.end() )
		{
			fprintf( stderr, "Error: Two clusters with same ID %d for node %s!\n", nBodyPart, node.name().asChar() );
			return MS::kFailure;
		}

		SAISkinCluster &curCluster = clusters[nBodyPart];

		if ( GetSkinClusterData() == MS::kFailure )
			return MS::kFailure;

		// geometry
		if ( GetGeometryData() == MS::kFailure )
			return MS::kFailure;

		// faces
		MItMeshPolygon itPoly( meshPath );
		for ( ; !itPoly.isDone(); itPoly.next() )
		{
			if ( itPoly.polygonVertexCount() != 3 )
			{
				MFnMesh mesh( meshPath );
				fprintf( stderr, "Error: AI mesh %s is needed to be triangulated - found %d vertices in one face!\n",
					mesh.name().asChar(), itPoly.polygonVertexCount() );
				return MS::kFailure;
			}
			SFace face;
			face.i1 = itPoly.vertexIndex(0);
			face.i2 = itPoly.vertexIndex(1);
			face.i3 = itPoly.vertexIndex(2);
			curCluster.faces.push_back( face );
		}

		// converting weights to file format
		int i, j;
		int nTotalInfs = nJointIndices.size();
		for ( i = 0; i < NConverter::gv.size(); ++i )
			for ( j = 0; j < nTotalInfs; ++j )
			{
				float fW = fWeightsArray[ i * nTotalInfs + j ];
				if ( fW != 0 )
				{
					SVertexWeight w = { fW, i, nJointIndices[j] };
					curCluster.weights.push_back( w );
				}
			}
		curCluster.verts = NConverter::gv;
	}

	if ( GetSpheresData() == MS::kFailure )
		return MS::kFailure;	

	fp.OpenWrite( szExportFileName.c_str() );
	{
		CStructureSaver file( fp, CStructureSaver::WRITE );
		file.Add( 4, &clusters );
		if ( NConverter::spheres.size() )
			file.Add( 6, &NConverter::spheres );
	}
	fp.CloseFile();

	char pszName[1024];
	sprintf( pszName, "%s%d", szBindsPath.c_str(), nFileNumber );
	szCurrentFileName = pszName;
	fp.OpenWrite( szCurrentFileName.c_str() );
	{
		CStructureSaver file( fp, CStructureSaver::WRITE );
		file.Add( 4, &NConverter::binds );
	}
	fp.CloseFile();

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// export AI simple mesh
MStatus A5ExportModel::ExportAIMesh()
{
	if ( GetExportedMesh() == MS::kFailure )
		return MS::kFailure;

	if ( GetAddMeshesData() == MS::kFailure )
		return MS::kFailure;	

	int nAllVertices = 0;
	int nFace = 0;

	for ( int nMesh = 0; nMesh < 1 + addMeshPaths.length(); ++nMesh )
	{
		int nPiece = 0;
		if ( nMesh > 0 )
		{
			nPiece = nPieces[ nMesh - 1 ];
			meshPath = addMeshPaths[ nMesh - 1 ];
		}

		if ( GetShaderData() == MS::kFailure )
			return MS::kFailure;

		MFnMesh mesh( meshPath );

		int i, count;
		count = mesh.numVertices();
		for ( i = 0; i < count; i++ )
		{
			MPoint p;
			mesh.getPoint( i, p, MSpace::kWorld );
			CVec3 plot(p.x, p.y, p.z);
			NConverter::gv.push_back( plot );
		}

		// faces
		MItMeshPolygon itPoly( meshPath );
		for ( ; !itPoly.isDone(); itPoly.next(), ++nFace )
		{
			if ( itPoly.polygonVertexCount() != 3 )
			{
				fprintf( stderr, "Error: AI mesh %s is needed to be triangulated - found %d vertices in one face!\n",
					mesh.name().asChar(), itPoly.polygonVertexCount() );
				return MS::kFailure;
			}
			SFace face;
			face.i1 = itPoly.vertexIndex(0) + nAllVertices;
			face.i2 = itPoly.vertexIndex(1) + nAllVertices;
			face.i3 = itPoly.vertexIndex(2) + nAllVertices;
			NConverter::faces[ nPiece + nSetIndices[nFace] ].faces.push_back( face );
		}

		nAllVertices += mesh.numVertices();
	}

	if ( GetSpheresData() == MS::kFailure )
		return MS::kFailure;	

	int nExportFlags = 0;
	if ( addMeshPaths.length() > 0 )
		nExportFlags |= ADD_MESHES;
	ExportWithMaterialFilter( nExportFlags );

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// export animation
MStatus A5ExportModel::ExportAnimation()
{
	if ( GetExportedJoints() == MS::kFailure )
		return MS::kFailure;

	if ( GetAdditionalBonesData() == MS::kFailure )
		return MS::kFailure;

	if ( GetJointsData() == MS::kFailure )
		return MS::kFailure;
	
	int nJoint, nTotalJoints, nAddBones;
	SAnimationHeader hdr;
	hdr.nRoots = 1;
	hdr.nBones = oJoints.length() - 1;
	hdr.nAddBones = addBones.size();
	hdr.bScale = bAnimWithScale;

	nTotalJoints = oJoints.length();
	nAddBones = addBones.size();
	// CRAP{ should be subset of bones
	for ( int i = 0; i < nTotalJoints + nAddBones; ++i )
		hdr.indices.push_back( i );
	// CRAP}

	// all frame values in milliseconds
	double fStep = 1000.0 / fFrameRate;

	double fTotalFrames = (fEndFrame - fStartFrame) / fStep;
	int nTotalFrames = (int)fTotalFrames;
	// if fraction frame less than millisecond - skip
	if ( fTotalFrames - (double)nTotalFrames < 0.001 * fFrameRate ) 
		fTotalFrames = (double)nTotalFrames;
	else
		++nTotalFrames;

	hdr.fLength = (float)fTotalFrames;
	hdr.fFrameRate = (float)fFrameRate;
	for ( int nFrame = 0; nFrame < nTotalFrames + 1; ++nFrame )
	{
		double fTime;
		if ( nFrame == nTotalFrames )
			fTime = fEndFrame;
		else
			fTime = fStartFrame + nFrame * fStep;
		MTime time( fTime, MTime::kMilliseconds );
		//fprintf( stderr, "%d: %10.10f\n", nFrame, time.as( MTime::uiUnit() ) );
		MGlobal::viewFrame( time );

		vector<CVec3> scales;
		mxCurPoses.clear();
		for ( nJoint = 0; nJoint < nTotalJoints + nAddBones; ++nJoint )
		{
			MDagPath path;
			if ( nJoint < nTotalJoints )
			{
				MFnIkJoint joint( oJoints[nJoint] );
				joint.getPath(path);
			}
			else
				path = addBones[ nJoint - nTotalJoints ].bonePath;
			MMatrix mxCurPose = path.inclusiveMatrix();
			// scaling
			if ( nJoint < nTotalJoints )
			{
				MMatrix mxScale;
				double scale[3];
				MFnIkJoint nodeJoint( oJoints[nJoint] );
				nodeJoint.getScale( scale );
				if ( bAnimWithScale )
					scales.push_back( CVec3( scale[0], scale[1], scale[2] ) );
				mxScale[0][0] = 1 / scale[0];
				mxScale[1][1] = 1 / scale[1];
				mxScale[2][2] = 1 / scale[2];
				mxCurPose = mxScale * mxCurPose;
			}
			mxCurPoses.push_back( mxCurPose );
		}
		for ( nJoint = 0; nJoint < nTotalJoints + nAddBones; ++nJoint )
		{
			MMatrix mxCurPose = mxCurPoses[nJoint];
			int nParent = -1;
			if ( nJoint < nTotalJoints )
				nParent = NConverter::joints[nJoint].nParent;
			else
			{
				double fMaxWeight = -1000.0f;
				SAdditionalBone &bone = addBones[nJoint - nTotalJoints];
				for ( int i=0; i<bone.plugs.size(); ++i )
				{
					double w;
					bone.plugs[i].getValue(w);
					if ( w > fMaxWeight )
					{
						fMaxWeight = w;
						nParent = bone.nJoints[i];
					}
				}				
			}
			if ( nParent >= 0 )
				mxCurPose = mxCurPose * mxCurPoses[ nParent ].inverse();
			CVec3 pos;
			SQuat rot;
			pos.x = mxCurPose.matrix[3][0];
			pos.y = mxCurPose.matrix[3][1];
			pos.z = mxCurPose.matrix[3][2];
			mxCurPose.matrix[3][0] = 0;
			mxCurPose.matrix[3][1] = 0;
			mxCurPose.matrix[3][2] = 0;
			MQuaternion q;
			q = mxCurPose;
			rot.x = q.x;
			rot.y = q.y;
			rot.z = q.z;
			rot.w = q.w;

			if ( nJoint >= nTotalJoints )
			{
				SAddBoneAnimationKey key;
				key.nParent = nParent;
				key.pos = pos;
				key.rot = rot;
				NConverter::keysAddBones.push_back( key );
			}
			else if ( bAnimWithScale )
			{
				SMSRAnimationKey key;
				key.pos = pos;
				key.rot = rot;
				key.scale = scales[nJoint];
				NConverter::keysMSR.push_back( key );
			}
			else if ( nJoint == 0 )
			{
				SRootAnimationKey key;
				key.pos = pos;
				key.rot = rot;
				NConverter::keysRoot.push_back( key );
			}
			else
				NConverter::keysJoints.push_back( rot );
		}
	}

	fp.OpenWrite( szExportFileName.c_str() );
	{
		CStructureSaver file( fp, CStructureSaver::WRITE );
		file.Add( 1, &hdr );
		file.Add( 2, &NConverter::keysRoot );
		file.Add( 3, &NConverter::keysJoints );
		file.Add( 4, &NConverter::keysAddBones );
		file.Add( 5, &NConverter::keysMSR );
	}
	fp.CloseFile();

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::GetParticlesData()
{
	MStatus stat;

	int nMaxFrame, nMinFrame;
	MGlobal::executeCommand( "playbackOptions -q -max", nMaxFrame );
	MGlobal::executeCommand( "playbackOptions -q -min", nMinFrame );
	if ( nMinFrame > 0 )
		nMinFrame = 0;

	MObjectArray objects;

	MFnSet setFn;
	setFn.setObject( particleObjects[0] );
	MSelectionList selList;
	setFn.getMembers( selList, false );
	for ( int i = 0; i < selList.length(); ++i )
	{
		MDagPath transform;
		MDagPath particle;
		selList.getDagPath( i, transform );
		for ( int j = 0; j < transform.childCount(); ++j )
		if ( transform.child(j).apiType() == MFn::kParticle )
		{
			objects.append( transform.child(j) );
			break;
		}
	}
	
	for ( int nFrame = nMinFrame; nFrame <= nMaxFrame; ++nFrame )
	{
		MGlobal::viewFrame( nFrame );
		for ( int nObj = 0; nObj < objects.length(); ++nObj )
		{
			MObject tmp;
			MFnDagNode nodeFn;
			nodeFn.setObject( objects[nObj] );
			int nParticles;

			MPlug plug = nodeFn.findPlug( MString("count"), &stat );
			stat = plug.getValue(nParticles);
			if ( nParticles <= 0 )
				continue;

			plug = nodeFn.findPlug( MString("idMapping"), &stat );

			vector<int> sortedId, idIndex;
			sortedId.resize( nParticles );
			idIndex.resize( nParticles );

			MPlug chPlug = plug.child(0);
			stat = chPlug.getValue(tmp);
			{
				MFnIntArrayData tmpi(tmp, &stat);
				for ( int i = 0; i < nParticles; ++i )
					sortedId[i] = tmpi[i];
			}
			chPlug = plug.child(1);
			stat = chPlug.getValue(tmp);
			{
				MFnIntArrayData tmpi(tmp, &stat);
				for ( int i = 0; i < nParticles; ++i )
					idIndex[i] = tmpi[i];
			}
			
			vector<STempParticleFrame> particles;
			particles.resize( nParticles );
			
			// World position
			plug = nodeFn.findPlug( MString("worldPosition"), &stat );
			stat = plug.getValue(tmp);
			MFnVectorArrayData wpos(tmp, &stat);
			for ( int i = 0; i < nParticles; ++i )
			{
				STempParticleFrame &frame = particles[i];
				frame.nFrame = nFrame;
				frame.pos.val[0] = float( wpos[idIndex[i]].x );
				frame.pos.val[1] = float( wpos[idIndex[i]].y );
				frame.pos.val[2] = float( wpos[idIndex[i]].z );
				// default values
				frame.rot.val[0] = 0;
				frame.scale.val[0] = 1;
				frame.scale.val[1] = 1;
				frame.color.val[0] = 1;
				frame.color.val[1] = 1;
				frame.color.val[2] = 1;
				frame.color.val[3] = 1;
				frame.nSprite = 0;
			}
			// Rotation
			plug = nodeFn.findPlug( MString("spriteTwistPP"), &stat );
			if ( stat == MS::kSuccess )
			{
				stat = plug.getValue(tmp);
				MFnDoubleArrayData rot(tmp, &stat);
				for ( int i = 0; i < nParticles; ++i )
					particles[i].rot.val[0] = float( rot[idIndex[i]] * PI / 180 );
			}
			// ScaleX
			plug = nodeFn.findPlug( MString("spriteScaleXPP"), &stat );
			if ( stat == MS::kSuccess )
			{
				stat = plug.getValue(tmp);
				MFnDoubleArrayData scaleX(tmp, &stat);
				for ( int i = 0; i < nParticles; ++i )
					particles[i].scale.val[0] = float( scaleX[idIndex[i]] );
			}
			// ScaleY
			plug = nodeFn.findPlug( MString("spriteScaleYPP"), &stat );
			if ( stat == MS::kSuccess )
			{
				stat = plug.getValue(tmp);
				MFnDoubleArrayData scaleY(tmp, &stat);
				for ( int i = 0; i < nParticles; ++i )
					particles[i].scale.val[1] = float( scaleY[idIndex[i]] );
			}
			// Fase
			plug = nodeFn.findPlug( MString("spriteNumPP"), &stat );
			if ( stat == MS::kSuccess )
			{
				stat = plug.getValue(tmp);
				MFnDoubleArrayData sprite(tmp, &stat);
				for ( int i = 0; i < nParticles; ++i )
					particles[i].nSprite = short( sprite[idIndex[i]] - 1 );
			}
			// Color
			plug = nodeFn.findPlug( MString("rgbPP"), &stat );
			if ( stat == MS::kSuccess )
			{
				stat = plug.getValue(tmp);
				MFnVectorArrayData rgb(tmp, &stat);
				for ( int i = 0; i < nParticles; ++i )
				{
					particles[i].color.val[0] = float( rgb[idIndex[i]].x );
					particles[i].color.val[1] = float( rgb[idIndex[i]].y );
					particles[i].color.val[2] = float( rgb[idIndex[i]].z );
				}
			}
			// Opacity
			plug = nodeFn.findPlug(MString("opacityPP"),&stat);
			if ( stat == MS::kSuccess )
			{
				stat = plug.getValue(tmp);
				MFnDoubleArrayData opacity(tmp, &stat);
				for ( int i = 0; i < nParticles; ++i )
					particles[i].color.val[3] = float( opacity[idIndex[i]] );
			}

			for ( i = 0; i < nParticles; ++i )
				NConverter::particles[ (nObj << 16) + sortedId[i] ].frames.push_back( particles[i] );
		}
	}

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::ExportLight()
{
	MObject object;
	if ( GetSelection( &object ) == MS::kFailure )
		return MS::kFailure;
	MStatus status;
	MFnTransform trans( object, &status );
	if ( status != MS::kSuccess )
	{
		fprintf( stderr, "Error: Not transform selected!\n" );
		return MS::kFailure;
	}
	MDagPath path;
	MDagPath::getAPathTo( object, path );

	MPlug plugRadius = trans.findPlug( "radius", &status );
	if ( status != MS::kSuccess )
	{
		fprintf( stderr, "Error: Transform %s has no \"radius\" plug!\n", trans.name().asChar() );
		return MS::kFailure;
	}
	MPlug plugColorR = trans.findPlug( "fxColorR", &status );
	if ( status != MS::kSuccess )
	{
		fprintf( stderr, "Error: Transform %s has no \"fxColorR\" plug!\n", trans.name().asChar() );
		return MS::kFailure;
	}
	MPlug plugColorG = trans.findPlug( "fxColorG", &status );
	if ( status != MS::kSuccess )
	{
		fprintf( stderr, "Error: Transform %s has no \"fxColorG\" plug!\n", trans.name().asChar() );
		return MS::kFailure;
	}
	MPlug plugColorB = trans.findPlug( "fxColorB", &status );
	if ( status != MS::kSuccess )
	{
		fprintf( stderr, "Error: Transform %s has no \"fxColorB\" plug!\n", trans.name().asChar() );
		return MS::kFailure;
	}

	int nMaxFrame, nMinFrame;
	MGlobal::executeCommand( "playbackOptions -q -max", nMaxFrame );
	MGlobal::executeCommand( "playbackOptions -q -min", nMinFrame );
	if ( nMinFrame > 0 )
		nMinFrame = 0;

	int nFrames = nMaxFrame - nMinFrame + 1;
	vector< TVector<3> > pos;
	vector< TVector<3> > colors;
	vector< TVector<1> > radius;
	pos.resize( nFrames );
	colors.resize( nFrames );
	radius.resize( nFrames );
	for ( int nFrame = nMinFrame; nFrame <= nMaxFrame; ++nFrame )
	{
		int i = nFrame - nMinFrame;
		MGlobal::viewFrame( nFrame );
		float tmp;
		status = plugRadius.getValue(tmp);
		radius[i].val[0] = tmp;
		status = plugColorR.getValue(tmp);
		colors[i].val[0] = tmp;
		status = plugColorG.getValue(tmp);
		colors[i].val[1] = tmp;
		status = plugColorB.getValue(tmp);
		colors[i].val[2] = tmp;
		MMatrix mx = path.inclusiveMatrix();
		pos[i].val[0] = mx.matrix[3][0];
		pos[i].val[1] = mx.matrix[3][1];
		pos[i].val[2] = mx.matrix[3][2];
	}

	float fEpsilon = 0.005f;

	TKeyTrack< TVector<3> > posRes;
	TKeyTrack< TVector<3> > colorRes;
	TKeyTrack< TVector<1> > radiusRes;
	LinearSpline( pos, nMinFrame, fEpsilon, &posRes );
	LinearSpline( colors, nMinFrame, fEpsilon, &colorRes );
	LinearSpline( radius, nMinFrame, fEpsilon, &radiusRes );

	MTime second( 1, MTime::kSeconds );
	float fFrameRate = second.as( MTime::uiUnit() );
	float fTStart = nMinFrame / fFrameRate;
	float fTEnd = nMaxFrame / fFrameRate;
	fp.OpenWrite( szExportFileName.c_str() );
	{
		CStructureSaver file( fp, CStructureSaver::WRITE );
		file.Add( 1, &fFrameRate );
		file.Add( 2, &fTStart );
		file.Add( 3, &fTEnd );
		file.Add( 4, &posRes );
		file.Add( 5, &colorRes );
		file.Add( 6, &radiusRes );
	}
	fp.CloseFile();

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus A5ExportModel::ExportParticles()
{
	MGlobal::executeCommand( "dynPref -rt 0" );	

	if ( GetExportedParticles() == MS::kFailure )
		return MS::kFailure;

	if ( GetParticlesData() == MS::kFailure )
		return MS::kFailure;

	MTime second( 1, MTime::kSeconds );
	float fFrameRate = second.as( MTime::uiUnit() );
	float fEpsilon = 0.005f;

	int nKeyBytes = 0;

	SParticleObject object;
	object.fTEnd = 0;
	object.fFrameRate = fFrameRate;
	object.particles.resize( NConverter::particles.size() );

	int nP = 0;
	for ( std::hash_map< int, STempParticle >::const_iterator it = NConverter::particles.begin();
		it != NConverter::particles.end(); ++it, ++nP )
	{
		SParticle &out = object.particles[nP];

		const STempParticle &particle = it->second;
		int nStartFrame = particle.frames.front().nFrame;
		int nEndFrame = particle.frames.back().nFrame;
		int nFrames = nEndFrame - nStartFrame + 1;
		if ( nFrames != particle.frames.size() )
		{
			fprintf( stderr, "Error: Number of particle frames: %d vs %d!\n", nFrames, particle.frames.size() );
			return MS::kFailure;
		}
		int nTBegin = nStartFrame;
		out.nTStart = nStartFrame;
		out.nTEnd = nEndFrame;

		if ( object.fTEnd < out.nTEnd / fFrameRate )
			object.fTEnd = out.nTEnd / fFrameRate;

		vector< TVector<3> > tmpPos;
		vector< TVector<1> > tmpRot;
		vector< TVector<2> > tmpScale;
		vector< TVector<4> > tmpColor;
		vector< short > tmpSprite;
		tmpPos.resize( nFrames );
		tmpRot.resize( nFrames );
		tmpScale.resize( nFrames );
		tmpColor.resize( nFrames );
		tmpSprite.resize( nFrames );

		for ( int i = 0; i < nFrames; ++i )
			tmpPos[i] = particle.frames[i].pos;
		LinearSpline( tmpPos, nTBegin, fEpsilon, &out.pos );

		for ( i = 0; i < nFrames; ++i )
			tmpRot[i] = particle.frames[i].rot;
		LinearSpline( tmpRot, nTBegin, fEpsilon, &out.rot );

		for ( i = 0; i < nFrames; ++i )
			tmpScale[i] = particle.frames[i].scale;
		LinearSpline( tmpScale, nTBegin, fEpsilon, &out.scale );

		TKeyTrack< TVector<4> > tempTrack;
		for ( i = 0; i < nFrames; ++i )
			tmpColor[i] = particle.frames[i].color;
		LinearSpline( tmpColor, nTBegin, fEpsilon, &tempTrack );
		out.color.keys.resize( tempTrack.keys.size() );
		for ( i = 0; i < tempTrack.keys.size(); ++i )
		{
			int r, g, b, a;
			r = FindCloseValue( tempTrack.keys[i].value.val[0] * 255, 0, 255 );
			g = FindCloseValue( tempTrack.keys[i].value.val[1] * 255, 0, 255 );
			b = FindCloseValue( tempTrack.keys[i].value.val[2] * 255, 0, 255 );
			a = FindCloseValue( tempTrack.keys[i].value.val[3] * 255, 0, 255 );
			DWORD pixel = b | (g << 8) | (r << 16) | (a << 24);
			out.color.keys[i].value = pixel;
			out.color.keys[i].nT = tempTrack.keys[i].nT;
		}

		for ( i = 0; i < nFrames; ++i )
			tmpSprite[i] = particle.frames[i].nSprite;
		DropSame( tmpSprite, nTBegin, &out.sprite );
/*
		fprintf( stderr, "NP: %d, pos:%d, rot:%d, scaleX:%d, color:%d, sprite:%d\n",
			it->first, out.pos.keys.size(), out.rot.keys.size(), out.scale.keys.size(), 
			out.color.keys.size(), out.sprite.keys.size() );
*/
		if ( out.pos.keys.size() == 2 && out.pos.keys[0].value == out.pos.keys[1].value )
			out.pos.keys.pop_back();
		if ( out.rot.keys.size() == 2 && out.rot.keys[0].value == out.rot.keys[1].value )
			out.rot.keys.pop_back();
		if ( out.scale.keys.size() == 2 && out.scale.keys[0].value == out.scale.keys[1].value )
			out.scale.keys.pop_back();
		if ( out.color.keys.size() == 2 && out.color.keys[0].value == out.color.keys[1].value )
			out.color.keys.pop_back();
		if ( out.sprite.keys.size() == 2 && out.sprite.keys[0].value == out.sprite.keys[1].value )
			out.sprite.keys.pop_back();
		nKeyBytes += out.pos.keys.size() * 14;
		nKeyBytes += out.rot.keys.size() * 6;
		nKeyBytes += out.scale.keys.size() * 10;
		nKeyBytes += out.color.keys.size() * 6;
		nKeyBytes += out.sprite.keys.size() * 4;
	}

	fprintf( stderr, "Particles: %d, Key bytes: %d\n", NConverter::particles.size(), nKeyBytes );

	char *pData;
	int nOutBytes = ConvertToSingleFile( object, &pData, NConverter::particles.size(), nKeyBytes );
	fp.OpenWrite( szExportFileName.c_str() );
	{
		fp.Write( &nOutBytes, 4 );
		fp.Write( pData, nOutBytes );
	}
	fp.CloseFile();
	delete [] pData;

	return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void A5ExportModel::OutputMatrix( const char *pszText, MMatrix &mx, FILE *f )
{
	fprintf( f, pszText );
	fprintf( f, "\n" );
	int i, j;
	for ( i = 0; i < 4; ++i )
	{
		for ( j = 0; j < 4; ++j )
		{
			fprintf( f, "%.3f ", mx.matrix[j][i] );
		}
		fprintf( f, "\n" );
	}
}
void A5ExportModel::OutputVector( const char *pszText, MVector &v, FILE *f )
{
	fprintf( f, pszText );
	int i;
	for ( i = 0; i < 3; ++i )
		fprintf( f, " %.6f", v[i] );
	fprintf( f, "\n" );
}
void A5ExportModel::OutputPoint( const char *pszText, MPoint &p, FILE *f )
{
	fprintf( f, pszText );
	int i;
	for ( i = 0; i < 3; ++i )
		fprintf( f, " %.3f", p[i] );
	fprintf( f, "\n" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
