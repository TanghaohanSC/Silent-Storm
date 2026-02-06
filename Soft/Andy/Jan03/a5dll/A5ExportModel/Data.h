#ifndef __DATA_H__
#define __DATA_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "BasicChunk1.h"
#include "Geom.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
template <int nCh>
struct TVector
{
	float val[nCh];
	bool operator==( const TVector<nCh> &a ) const
	{
		for ( int i=0; i<nCh; ++i )
			if ( val[i] != a.val[i] )
				return false;
		return true;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SVertex
{
	CVec3 gv, nv;
	CVec2 tv;
	CVec3 du, dv;
	DWORD dwColor;
	
	SVertex() {}
	SVertex( const CVec3 &g, const CVec3 &n, const CVec3 &t ) : gv(g), nv(n), tv(t.x, t.y)
		{ memset( &du, 0, sizeof(DWORD)*4 ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFace
{
	WORD i1, i2, i3;

	SFace() {}
	SFace( int a, int b, int c ) : i1(a), i2(b), i3(c) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SQuat
{
	float x, y, z, w;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSphereMass
{
	CVec3 point;
	float fRadius;
	float fMass;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SJoint
{
	string szName;
	int nParent;
	CVec3 pos;
	SQuat rot;
	CVec3 scale;

	int operator&( CStructureSaver &f )
	{
		f.Add( 1, &szName );
		f.Add( 2, &nParent );
		f.Add( 3, &pos );
		f.Add( 4, &rot );
		f.Add( 5, &scale );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSkeletonHeader
{
	int nBones;
	int nAdditionalBones;

	int operator&( CStructureSaver &f )
	{
		f.Add( 1, &nBones );
		f.Add( 2, &nAdditionalBones );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SVertexWeight
{
	float fWeight;
	int nVertex;
	int nJoint;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRootAnimationKey
{
	CVec3 pos;
	SQuat rot;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAddBoneAnimationKey
{
	int nParent;
	CVec3 pos;
	SQuat rot;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMSRAnimationKey
{
	CVec3 pos;
	SQuat rot;
	CVec3 scale;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAnimationHeader
{
	vector< int > indices;
	float fLength;
	float fFrameRate;
	int nRoots;
	int nBones;
	int nAddBones;
	bool bScale;
	int operator&( CStructureSaver &f )
	{
		f.Add( 1, &indices );
		f.Add( 2, &fLength );
		f.Add( 3, &fFrameRate );
		f.Add( 4, &nRoots );
		f.Add( 5, &nBones );
		f.Add( 6, &nAddBones );
		f.Add( 7, &bScale );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TVertex>
struct SMeshPiece
{
	vector<TVertex> verts;
	vector<SFace> faces;
	int operator&( CStructureSaver &f )
	{
		f.Add( 1, &verts );
		f.Add( 2, &faces );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TVertex>
struct SConvertedMeshPiece
{
	vector<TVertex> verts;
	vector<WORD> indices;
	vector<WORD> polys;
	int operator&( CStructureSaver &f )
	{
		f.Add( 1, &verts );
		f.Add( 2, &indices );
		f.Add( 3, &polys );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAISkinCluster
{
	vector<CVec3> verts;
	vector<SFace> faces;
	vector<SVertexWeight> weights;
	int operator&( CStructureSaver &f )
	{
		f.Add( 1, &verts );
		f.Add( 2, &faces );
		f.Add( 3, &weights );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFacesVector
{
	vector<SFace> faces;
	int operator&( CStructureSaver &f )
	{
		f.Add( 1, &faces );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct STempParticleFrame
{
	int nFrame;
	TVector<3> pos;
	TVector<1> rot;
	TVector<2> scale;
	TVector<4> color;
	short nSprite;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct STempParticle
{
	vector<STempParticleFrame> frames;	
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const int NUM_MATERIALS = 4;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NConverter
{
	// meshes
	externA5 vector<SJoint> joints;
	externA5 vector<SHMatrix> binds;
	externA5 vector<SVertexWeight> weights;
	externA5 vector<int> links;
	externA5 vector<CVec3> gv, nv, tv;
	externA5 vector<SVertex> verts;
	externA5 hash_map< int, SFacesVector > faces;
	// animation
	externA5 vector<SRootAnimationKey> keysRoot;
	externA5 vector<SQuat> keysJoints;
	externA5 vector<SAddBoneAnimationKey> keysAddBones;
	externA5 vector<SMSRAnimationKey> keysMSR;
	externA5 vector<SSphereMass> spheres;
	externA5 hash_map< int, STempParticle > particles;

	void ClearAll();
	void AddFace( int nSet, int *pIndices );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif