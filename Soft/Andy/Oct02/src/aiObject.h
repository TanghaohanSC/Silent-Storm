#ifndef __AIOBJECT_H_
#define __AIOBJECT_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
namespace NDb
{
	class CRPGArmor;
}
namespace NWorld
{
	struct IVisObj;
}
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SEdge
{
	WORD wStart, wFinish;
	//
	SEdge() {}
	SEdge( WORD _wS, WORD _wF ): wStart(_wS), wFinish(_wF) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CEdgesInfo
{
	WORD InsertEdge( WORD i1, WORD i2, const vector<CVec3> &pts );
public:
	vector<SEdge> edges;
	vector<STriangle> mesh;
	bool bClosed;
	//
	CEdgesInfo() { bClosed = true; }
	void BuildTriangleList( vector<STriangle> *pRes ) const;
	void GenerateEdgeList( const vector<STriangle> &tris, const vector<CVec3> &pts );
	bool IsClosed() const; // checks geometry
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SJunction
{
	CVec3 pt;
	bool  bGround;

	SJunction() {}
	SJunction( const CVec3 &_pt, bool _bGr = false ): pt(_pt), bGround(_bGr) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CBSPTree;
class CGeometryInfo: public CObjectBase
{
	OBJECT_BASIC_METHODS(CGeometryInfo);
public:
	struct SPiece
	{
		vector<CVec3> points;
		CEdgesInfo edges;
		float fVolume;
		vector<SJunction> juncs;
		CPtr<CBSPTree> pBSPTree;
	};
	typedef std::hash_map<int, SPiece> CPieceMap;

	SBound bound;
	vector<SMassSphere> spheres;
	CVec3 massCenter;
	CPieceMap pieces;
	//
	SPiece* GetPiece( int nPieceID );
	void AddPiece( int nPieceID, const vector<CVec3> &_points, const vector<STriangle> &_tris, 
		float fVolume, vector<SJunction> juncs = vector<SJunction>(), bool _bClosed = true, CBSPTree* _pBSPTree = 0 );
	void CalcBound();
	void CalcBSPTrees( bool bTerrain = false );
	bool HasPiece( int nID ) const { return pieces.find( nID ) != pieces.end(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//! structure describing object for different tracers
struct SSourceInfo;
struct SConvexHull
{
	const vector<CVec3> &points;
	const CEdgesInfo &tris;
	const SFBTransform &trans;
	const SSourceInfo &src;
	int nUserID;
	const CPtr<CBSPTree> pBSPTree;
	//
	SConvexHull( const vector<CVec3> &_points, const CEdgesInfo &_tris, const SFBTransform &_trans,
		SSourceInfo &_src, int _nUserID, CBSPTree *_pBSPTree )
		: points(_points), tris(_tris), trans(_trans), src(_src), nUserID(_nUserID), pBSPTree(_pBSPTree) {}
	int operator&( CStructureSaver &f ) { ASSERT(0&&"This struct could not be serialized!"); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//! group of entities; entity terrain is represented with several SConvexHull
struct SHullSet
{
	vector<SConvexHull> objects, terrain;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif