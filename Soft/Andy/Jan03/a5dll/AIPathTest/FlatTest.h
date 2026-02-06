#ifndef __GDX_H_
#define __GDX_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "Geom.h"
/////////////////////////////////////////////////////////////////////////////////////
class CEdge
{
public:
	CVec2 src, dst;

	CEdge( const CVec2 &_src, const CVec2 &_dst ): src(_src), dst(_dst) {}
};
/////////////////////////////////////////////////////////////////////////////////////
class CPolygon
{
public:
	vector<CVec2> points;
	//
	CPolygon& Add( const CVec2 &p ) { points.push_back( p ); return *this; }
	CPolygon& Add( float x, float y ) { return Add( CVec2( x, y ) ); }
	void Scale( float f );
	void Move( const CVec2 &move );
	void Rotate( float f );
};
/////////////////////////////////////////////////////////////////////////////////////
class CPointHorizon;
class CWorld
{
public:
	struct SPoint;
	struct SLink
	{
		float fLeng;
		SPoint *p;
		//
		SLink( float _fLeng, SPoint *_p ): fLeng(_fLeng), p(_p) {}
	};
	struct SPoint
	{
		CVec2 ptPlace;
		vector<SLink> links;
		float fEstimate, fDest;
		SPoint *pPrev;
		//
		SPoint( const CVec2 &p ): ptPlace(p) {}
	};
	//
	list<SPoint> points;
	//
	vector<CPolygon> polys;
	//
	void AddPolygon( const CPolygon &poly ) { polys.push_back( poly ); }
	void BuildGraph( const CVec2 &size, float fGridStep );
	bool SearchPath( vector<CVec2> *pRes, const CVec2 &src, const CVec2 &dst );
private:
	float fSearchRadius2;
	//
	float fBest;
	SPoint *pBest;
	//
	void TestSymmetry();
	void CheckTotal(SPoint *pNode );
	void PerformTrace( CPointHorizon *pRes );
	void OptimizePath( vector<CVec2> *pRes );
};
/////////////////////////////////////////////////////////////////////////////////////
// trace to bunch of points in batch
class CPointHorizon
{
public:
	struct STestPoint
	{
		float fAngle, fDist;
		vector<CEdge> res;
		CVec2 ptTest;
		void *pUserData;
	};
	vector<STestPoint> points;
	CVec2 ptCenter;
	int nInsideCounter;
	//
	void SetCenter( const CVec2 &_ptCenter ) { ptCenter = _ptCenter; nInsideCounter = 0; points.clear(); }
	void AddTestPoint( const CVec2 &ptTest, void *pUserData = 0 );
	void DoTrace( const CWorld &w );
};
/////////////////////////////////////////////////////////////////////////////////////
void TraceRay( const CWorld &w, vector<CEdge> *pRes, const CVec2 &src, const CVec2 &dst );
/////////////////////////////////////////////////////////////////////////////////////
#endif