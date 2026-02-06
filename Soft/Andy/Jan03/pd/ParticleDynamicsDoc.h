// ParticleDynamicsDoc.h : interface of the CParticleDynamicsDoc class
//


#pragma once

#include "Geom.h"
#include "Tools.h"
#include "Transform.h"

class CPos
{
public:
	int x, y, z;
	CPos() {}
	CPos( int _x, int _y, int _z ) : x(_x), y(_y), z(_z) {}
};

struct SLink
{
	int n1, n2;
	int nSize;
	SLink() {}
	SLink( int _n1, int _n2, int _nSize ) : n1(_n1), n2(_n2), nSize(_nSize) {}
};

struct SDOF
{
	vector<int> left;
	vector<int> right;
	int nCenter;
};

/*class CVec2
{
public:
	float x, y;
	CVec2() {}
	CVec2( float _x, float _y ) : x(_x), y(_y) {}
	CVec2 operator-() { return CVec2(-x,-y); }
};
inline CVec2 operator-( const CVec2 &a, const CVec2 &b ) { return CVec2( a.x - b.x, a.y - b.y ); }
*/

typedef vector<CVec3> Speed;

struct SState
{
	vector<CPos> pos, potential; //speed, 
	vector<float> speed;

	SState() {}
	void ApplyDelta( const vector<CPos> &delta );
	void ApplyGravity();
	void ApplyReaction( const vector<SDOF> &dofGroups );
	void ApplyStringsReaction( vector<CPos> *pSpeed, const vector<SLink> &links );
	void GenerateDOFs( const vector<SDOF> &dofs, vector<Speed> *pRes );
	void GenerateSpeed( vector<CPos> *pRes, const vector<SDOF> &dofGroups );
};

class CParticleDynamicsDoc : public CDocument
{
protected: // create from serialization only
	CParticleDynamicsDoc() {}
	DECLARE_DYNCREATE(CParticleDynamicsDoc)

// Attributes
public:
	SState cur;//, prev;
	vector<int> radiuses;
	vector<SLink> links;
	vector<SDOF> dofGroups;
//	int nEnergy;

// Operations
public:
	void AddLink( int n1, int n2 );
	void AddPoint( int x, int y, int z, int r );
	void StartNew();
	//void Relaxation();
	void Collide();
	void Step();
	double CalcEnergy();
	CVec3 CalcMassCenter();

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CParticleDynamicsDoc() {}
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


