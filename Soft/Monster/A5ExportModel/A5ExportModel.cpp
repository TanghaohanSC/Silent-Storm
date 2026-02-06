#include "StdAfx.h"

#include <stdio.h>
#include <maya\MDagPathArray.h>
#include <maya\MSelectionList.h>
#include <maya\MGlobal.h>
#include <maya\MFnMesh.h>
#include <maya\MFloatVectorArray.h>
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

#include "Data.h"
#include "A5ExportModel.h"
#include "BasicChunk1.h"

void* A5ExportModel::creator()
{
	return new A5ExportModel();
}

MStatus A5ExportModel::reader( const MFileObject& file, const MString& optionsString, FileAccessMode mode )
{
	return MS::kFailure;
}

MPxFileTranslator::MFileKind A5ExportModel::identifyFile(
	const MFileObject& fileName, const char* buffer, short size ) const
{
	return kNotMyFileType;
}

MStatus A5ExportModel::writer( const MFileObject& file, const MString& options, FileAccessMode mode )
{
	MStatus status;

	fprintf( stderr, "Export started!\n" );

	MString mname = file.fullName();
	const char *fname = mname.asChar();

	if ( !fp.OpenWrite( fname ) )
	{
		fprintf( stderr, "Error: A5ExportModel::writer: The file %s could not be opened for writing.\n", fname );
		return MS::kFailure;
	}

	if( mode == MPxFileTranslator::kExportActiveAccessMode )
		status = Export();
	else
	{
		fprintf( stderr, "Error: A5ExportModel::writer: Select root node and export!\n" );
		return MS::kFailure;
	}

	fp.CloseFile();

	if( status == MS::kFailure )
	{
		fprintf( stderr, "Error: A5ExportModel::writer: Export has failed!\n" );
		return MS::kFailure;
	}

	fprintf( stderr, "Export successful!\n" );
	return MS::kSuccess;
}


MStatus A5ExportModel::Export()
{
	MSelectionList selection;
 	MGlobal::getActiveSelectionList( selection );

	MItSelectionList iterSL( selection );
	MObject object;
	for ( ; !iterSL.isDone(); iterSL.next() )
		iterSL.getDependNode( object );
	if ( object.isNull() || object.apiType() != MFn::kJoint )
	{
		fprintf( stderr, "Error: A5ExportModel::Export: One joint root node must be selected!\n" );
		return MS::kFailure;
	}
/*	
//	MItDependencyGraph iterDG( object, MFn::kJoint,
//		MItDependencyGraph::kDownstream, MItDependencyGraph::kDepthFirst, MItDependencyGraph::kNodeLevel );

	MItDependencyNodes iterDG;
	for ( ; !iterDG.isDone(); iterDG.next() )
	{
		MObject obj;
		//obj = iterDG.thisNode();
		obj = iterDG.item();
		MFnDependencyNode node( obj );
		if ( obj.hasFn( MFn::kMesh ) )
		{
			meshes.append( obj );
			fprintf( stderr, "Mesh: %s\n", node.name().asChar() );
		}
		else if ( obj.hasFn( MFn::kJoint ) )
		{
			joints.append( obj );
			fprintf( stderr, "Joint: %s\n", node.name().asChar() );
		}
		else if ( obj.hasFn( MFn::kJointCluster ) )
		{
			clusters.append( obj );
			fprintf( stderr, "Cluster: %s\n", node.name().asChar() );
		}
		else if ( obj.hasFn( MFn::kSkinClusterFilter ) )
		{
			fprintf( stderr, "SkinCluster: %s\n", node.name().asChar() );
		}
		else
		{
			fprintf( stderr, "Unknown: %s\n", node.name().asChar() );
		}
	}
*/
	MObjectArray joints, meshes, clusters, skins;

	MItDag itDAG;
	itDAG.reset( object, MItDag::kDepthFirst, MFn::kJoint );
	for ( ; !itDAG.isDone(); itDAG.next() )
		joints.append( itDAG.item() );

	int count = joints.length();
	for ( int i=0; i<count; ++i )
	{
		MFnDependencyNode node( joints[i] );
		fprintf( stderr, "Joint: %s\n", node.name().asChar() );

		MItDependencyGraph itDG( joints[i], MFn::kGeometryFilt );
		for ( ; !itDG.isDone(); itDG.next() )
		{
			MObject obj = itDG.thisNode();
			if ( obj.apiType() == MFn::kSkinClusterFilter && !skins.length() )
				skins.append( obj );
			else if ( obj.apiType() == MFn::kJointCluster )
				clusters.append( obj );
		}
	}

	count = skins.length();
	for ( i=0; i<count; ++i )
	{
		MFnSkinCluster skin( skins[i] );
		fprintf( stderr, "SkinCluster: %s\n", skin.name().asChar() );

		MObjectArray geoms;
		skin.getOutputGeometry( geoms );
		if ( geoms.length() != 1 || geoms[0].apiType() != MFn::kMesh )
		{
			fprintf( stderr, "Error: A5ExportModel::Export: One mesh must be connected to skin cluster!\n" );
			return MS::kFailure;
		}
		ExportSkinCluster( skins[i] );
		meshes.append( geoms[0] );
	}
/*
	count = clusters.length();
	for ( i=0; i<count; ++i )
	{
		MFnDependencyNode node( clusters[i] );
		fprintf( stderr, "Cluster: %s\n", node.name().asChar() );

		MItDependencyGraph itDG( clusters[i], MFn::kMesh );
		for ( ; !itDG.isDone(); itDG.next() )
		{
			MObject obj = itDG.thisNode();
			if ( obj.apiType() == MFn::kMesh )
				meshes.append( obj );
		}
	}
*/


	NConverter::ClearAll();

	count = meshes.length();
	for ( i=0; i<count; ++i )
	{
		MFnDependencyNode node( meshes[i] );
		fprintf( stderr, "Mesh: %s\n", node.name().asChar() );
		if ( ExportMesh( meshes[i] ) == MS::kFailure )
			return MS::kFailure;
	}

	count = joints.length();
	for ( i=0; i<count; ++i )
	{
		MFnDependencyNode node( joints[i] );
		fprintf( stderr, "Joint: %s\n", node.name().asChar() );
	}

	if ( ExportJoints( joints ) == MS::kFailure )
		return MS::kFailure;

	count = skins.length();
	for ( i=0; i<count; ++i )
	{
		MFnDependencyNode node( skins[i] );
		fprintf( stderr, "SkinCluster: %s\n", node.name().asChar() );
		if ( ExportSkinCluster( skins[i] ) == MS::kFailure )
			return MS::kFailure;
	}

	CStructureSaver file( fp, CStructureSaver::WRITE );
	file.AddDataContainer( 1, &NConverter::verts );
	file.AddDataContainer( 2, &NConverter::faces );
	file.AddContainer( 3, &NConverter::joints );
	file.AddDataContainer( 4, &NConverter::weights );

	return MS::kSuccess;
}

MStatus A5ExportModel::ExportJoints( const MObjectArray &objects )
{
	for ( int nJoint = 0; nJoint < objects.length(); ++nJoint )
	{
		SJoint joint;
		MFnDependencyNode node( objects[nJoint] );
		joint.szName = node.name().asChar();
		NConverter::joints.push_back( joint );
	}
	NConverter::joints[0].nParent = -1;
	for ( nJoint = 1; nJoint < objects.length(); ++nJoint )
	{
		MFnDagNode node( objects[nJoint] );
		MFnDagNode parent( node.parent(0) );
		MString szParentName = parent.name();

		for ( int i = 0; i < objects.length(); ++i )
		{
			MFnDagNode temp( objects[i] );
			if ( temp.name() == szParentName )
			{
				NConverter::joints[nJoint].nParent = i;
				break;
			}
		}
	}

	MVector trans2;
	for ( nJoint = 0; nJoint < objects.length(); ++nJoint )
	{
		MFnIkJoint node( objects[nJoint] );
		fprintf( stderr, "%s\n", node.name().asChar() );
		MVector trans = node.translation( MSpace::kTransform );
		NConverter::joints[nJoint].translation.x = trans.x;
		NConverter::joints[nJoint].translation.y = trans.y;
		NConverter::joints[nJoint].translation.z = trans.z;

		MDagPath path;
		node.getPath(path);
		MTransformationMatrix transMat = path.inclusiveMatrixInverse();
		MMatrix mt = transMat.asMatrix();

		SHMatrix m;

		/*
		MDagPath path = node.dagPath();
		MTransformationMatrix transMat = path.inclusiveMatrix();
		MQuaternion quat = transMat.rotation();
		MMatrix matrix = quat.asMatrix();

		*/
		
		int i, j;
		for ( i = 0; i < 4; ++i )
			for ( j = 0; j < 4; ++j )
			{
				*( &m._11 + i * 4 + j ) = mt.matrix[j][i];
				fprintf( stderr, "%g ", mt.matrix[j][i] );
			}
		fprintf( stderr, "\n" );

		NConverter::joints[nJoint].bindPoseMatrix = m;
		//fprintf( stderr, "rot: %g %g %g %g\n", quat[0], quat[1], quat[2], quat[3] );
	}

	return MS::kSuccess;
}

MStatus A5ExportModel::ExportSkinCluster( const MObject &object )
{
	MFnSkinCluster skinCluster( object );
	MDagPathArray infs;
	MStatus stat;

	int nTotalInfs = skinCluster.influenceObjects( infs, &stat );
	if ( nTotalInfs == 0 )
	{
		fprintf( stderr, "Error: A5ExportModel::ExportSkinCluster: No influence objects found!\n" );
		return MS::kFailure;
	}

	std::vector<int> nJointIndices;
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
		fprintf( stderr, "Error: A5ExportModel::ExportSkinCluster: Found dependency on non-exported joint!\n" );
		return MS::kFailure;
	}

	int nTotalGeoms = skinCluster.numOutputConnections();
	if ( nTotalGeoms != 1 )
	{
		fprintf( stderr, "Error: A5ExportModel::ExportSkinCluster: There should be only one influenced geometry in skin cluster!\n" );
		return MS::kFailure;
	}

	for ( int nGeom = 0; nGeom < nTotalGeoms; ++nGeom )
	{
		int index = skinCluster.indexForOutputConnection( nGeom );

		MDagPath path;
		skinCluster.getPathAtIndex( index, path );
		MItGeometry itGeom( path );

		float *fWeightsArray = new float[ itGeom.count() * nTotalInfs ];

		int nVertex = 0;
		for ( ; !itGeom.isDone(); itGeom.next(), ++nVertex )
		{
			MObject comp = itGeom.component();

			MFloatArray fWeights;
			unsigned int nInfCount;
			stat = skinCluster.getWeights( path, comp, fWeights, nInfCount );

			for ( int i = 0; i < fWeights.length(); ++i )
				fWeightsArray[ nVertex * nInfCount + i ] = fWeights[i];
		}

		int i, j;
		for ( i = 0; i < NConverter::verts.size(); ++i )
		{
			int nOrigVertex = NConverter::links[i];
			for ( j = 0; j < nTotalInfs; ++j )
			{
				float fW = fWeightsArray[ nOrigVertex * nTotalInfs + j ];
				if ( fW != 0 )
				{
					SVertexWeight w = { fW, i, nJointIndices[j] };
					NConverter::weights.push_back( w );
				}
			}
		}	
		
		delete [] fWeightsArray;
	}

	return MS::kSuccess;
}

MStatus A5ExportModel::ExportMesh( const MObject &object )
{
	MStatus stat;
	MFnMesh mesh( object );
	MDagPath path;
	MDagPath::getAPathTo( object, path );
	MSpace::Space space = MSpace::kWorld;

	int pID, count;

	count = mesh.numVertices();
	for ( pID = 0; pID < count; pID++ )
	{
		MPoint p;
		mesh.getPoint( pID, p, space );
		SPlot plot = { p.x, p.y, p.z };
		NConverter::gv.push_back( plot );
	}

	count = mesh.numUVs();
	for ( pID = 0; pID < count; pID++ )
	{
		float u, v;
		mesh.getUV( pID, u, v );
		SPlot plot = { u, v, 0 };
		NConverter::tv.push_back( plot );
	}

	count = mesh.numNormals();
	MFloatVectorArray norms;
	mesh.getNormals( norms, space );
	for ( pID = 0; pID < count; pID++ )
	{
		SPlot plot = { norms[pID].x, norms[pID].y, norms[pID].z };
		NConverter::nv.push_back( plot );
	}

	MItMeshPolygon polyIter( path );
	int nIndices[9];
	for ( ; !polyIter.isDone(); polyIter.next() )
	{
		if ( polyIter.polygonVertexCount() != 3 )
		{
			fprintf( stderr, "Error: Mesh %s is needed to be triangulated!\n", mesh.name().asChar() );
			return MS::kFailure;
		}
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
		NConverter::AddFace( nIndices );
	}

	return MS::kSuccess;
}