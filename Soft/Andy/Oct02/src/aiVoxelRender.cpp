#include "StdAfx.h"
//
#include "memObject.h"
#include "gView.h"
#include "aiObject.h"
#include "aiInterval.h"
//
#include "aiVoxelRender.h"
//
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVoxelRenderer
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVoxelRenderer::TestSphere( const CVec3 &ptCenter, float fR ) 
{
	return ts.IsIn( SSphere( ptCenter, fR ) ); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVoxelRenderer::AddObject( SExplObject object )
{
	if ( *nObjectsEnd >= objects->size() )
		objects->resize( objects->size() + 500 );
	//
	(*objects)[*nObjectsEnd] = object;
	++(*nObjectsEnd);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVoxelRenderer::Init( const CVec3 &_vCenter, 
	float _fCubeSize, int _nResolution, vector<SExplObject> *_objects, int *_nObjectsEnd )
{
	nObjectsEnd = _nObjectsEnd;
	nCurrentObjectID = 0;
	vCenter = _vCenter;
	fCubeSize = _fCubeSize;
	nResolution = _nResolution;
	objects = _objects;
	voxels.SetSizes( nResolution + 1, nResolution + 1, nResolution + 1 );
	voxels.FillEvery( SExplVoxel( 0, 0 ) );
	if ( objects->empty() )
	{
		AddObject( SExplObject( 0, 0, 0, false ) );
		AddObject( SExplObject( 0, 0, 0, true ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVoxelRenderer::InitParallel( EAxis _axis )
{
	axis = _axis;
	ts.MakeParallel( fCubeSize, fCubeSize, -fCubeSize * 0.5f, fCubeSize * 0.5f );
	SHMatrix cam;
	switch ( axis )
	{
		case AXIS_X:
			MakeMatrix( &cam, vCenter, CVec3(1,0,0) );
			break;
		case AXIS_Y:
			MakeMatrix( &cam, vCenter, CVec3(0,1,0) );
			break;
		case AXIS_Z:
			MakeMatrix( &cam, vCenter, CVec3(0,0,1) );
			break;
	}
	ts.SetCamera( cam );
	transform = ts.Get().forward;
	int nHalfSize = nResolution * 0.5f;
	region = CTRect<int>( -nHalfSize, -nHalfSize, nHalfSize, nHalfSize );
	transform.x *= nHalfSize;
	transform.y *= nHalfSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVoxelRenderer::TraceEntity( const SConvexHull &e, bool bTerrain )
{
	nCurrentObjectID = N_VOXEL_TERRAIN;
	//
	if ( !bTerrain )
	{
		int nObjectID = 0;
		for ( vector<SExplObject>::iterator obj = objects->begin(); obj != objects->end(); ++obj, ++nObjectID )
			if ( obj->pUserData == e.src.pUserData && obj->nUserID == e.nUserID )
			{
				nCurrentObjectID = nObjectID;
				break;
			}
		//
		if ( nCurrentObjectID == N_VOXEL_TERRAIN )
		{
			AddObject( SExplObject( e.src.pUserData, e.nUserID, e.src.pArmor, false ) );
			nCurrentObjectID = *nObjectsEnd - 1;
		}
	}
	//
	RealTraceEntity( e );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVoxelRenderer::RealTraceEntity( const SConvexHull &e )
{
	// находим проекцию в CameraSpace
	static vector<CVec3> flatProjected;
	if ( e.points.size() > flatProjected.size() )
		flatProjected.resize( e.points.size() );
	//
	SHMatrix xform;
	Multiply( &xform, transform, e.trans.forward );
	for ( int i = 0; i < e.points.size(); ++i )
	{
		const CVec3 &src = e.points[i];
		CVec3 &dst = flatProjected[i];
		xform.RotateHVector( &dst, src );
	}
	// rasterize каждый треугольник ConvexHull-а
	const vector<SEdge> &edges = e.tris.edges;
	const vector<STriangle> &mesh = e.tris.mesh;
	for ( int i = 0; i < mesh.size(); ++i )
	{
		int i1, i2, i3;
		const STriangle &t = mesh[i];
		if ( t.i1 & 0x8000 )
		{
			i1 = edges[ t.i1 & 0x7fff ].wFinish; 
			i2 = edges[ t.i1 & 0x7fff ].wStart;
		}
		else
		{
			i1 = edges[ t.i1 & 0x7fff ].wStart; 
			i2 = edges[ t.i1 & 0x7fff ].wFinish;
		}
		if ( t.i2 & 0x8000 )
			i3 = edges[ t.i2 & 0x7fff ].wStart;
		else
			i3 = edges[ t.i2 & 0x7fff ].wFinish;
		//
		RasterNoClip( flatProjected[i1], flatProjected[i2], flatProjected[i3] );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NAI;
