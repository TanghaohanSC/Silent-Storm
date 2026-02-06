#ifndef __DATA_H__
#define __DATA_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include "BasicChunk1.h"
/////////////////////////////////////////////////////////////////////////////////////
struct SPlot
{
	float x, y, z;
};
struct SPlot2
{
	float x, y;

	SPlot2() {}
	SPlot2( const SPlot &t ) { x = t.x; y = t.y; }
};
struct SVertex
{
	SPlot gv, nv;
	SPlot2 tv;

	SVertex() {}
	SVertex( const SPlot &g, const SPlot &n, const SPlot &t ) : gv(g), nv(n), tv(t) {}
};
struct SFace
{
	WORD nA, nB, nC;

	SFace() {}
	SFace( int a, int b, int c ) : nA(a), nB(b), nC(c) {}
};
struct SHMatrix
{
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
};

struct SJoint
{
	std::string szName;
	int nParent;
	SPlot translation;
	SHMatrix bindPoseMatrix;
};
inline void Serialize( CStructureSaver *pFile, SJoint *pJoint )
{
	pFile->AddObject( 1, &pJoint->szName );
	pFile->AddData( 2, &pJoint->nParent );
	pFile->AddData( 3, &pJoint->translation );
	pFile->AddData( 4, &pJoint->bindPoseMatrix );
}

struct SVertexWeight
{
	float fWeight;
	int nVertex;
	int nJoint;
};
/////////////////////////////////////////////////////////////////////////////////////
namespace NConverter
{
	extern std::vector<SJoint> joints;
	extern std::vector<SVertexWeight> weights;
	extern std::vector<int> links;
	extern std::vector<SPlot> gv, nv, tv;
	extern std::vector<SVertex> verts;
	extern std::vector<SFace> faces;
	void ClearAll();
	void AddFace( int *pIndices );
};
/////////////////////////////////////////////////////////////////////////////////////
#endif