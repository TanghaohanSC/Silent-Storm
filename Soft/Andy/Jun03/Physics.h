#pragma once

namespace NPhysics
{
class CPos
{
public:
	int x, y, z;
	CPos() {}
	CPos( int _x, int _y, int _z ) : x(_x), y(_y), z(_z) {}
	CVec3 GetVec3() const { return CVec3(x,y,z); }
};
inline bool operator==( const CPos &a, const CPos &b ) { return a.x == b.x && a.y == b.y && a.z == b.z; }

struct SLink
{
	int n1, n2;
	int nSize;
	SLink() {}
	SLink( int _n1, int _n2, int _nSize ) : n1(_n1), n2(_n2), nSize(_nSize) {}
};

struct SDOF
{
	ZDATA
	vector<int> left;
	vector<int> right;
	int nCenter;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&left); f.Add(3,&right); f.Add(4,&nCenter); return 0; }
};

typedef vector<CVec3> Speed;

struct SContact
{
	ZDATA
	vector<float> speed;
	vector<float> speedX, speedY;
	float fFriction;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&speed); f.Add(3,&speedX); f.Add(4,&speedY); return 0; }
};

struct SLimitPlane
{
	CVec3 vDir;
	float fDist;

	SLimitPlane() {}
	SLimitPlane( const CVec3 &_vDir, float _fDist ) : vDir(_vDir), fDist(_fDist) {}
};
inline bool operator==( const SLimitPlane &a, const SLimitPlane &b ) { return a.vDir == b.vDir && a.fDist == b.fDist; }

struct SContactInfo
{
	SLimitPlane plane;
	int nPoint;
	int nPotential; // shift amount
	float fFriction;
};

class ICollider
{
public:
	virtual bool Collide( const CPos &pos, float fRadius, const CPos &delta, CVec4 *pPlane ) = 0;
};

struct SState
{
	ZDATA
	vector<CPos> pos;//, potential; //speed, 
	vector<float> speed;
	vector<SContactInfo> potential;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pos); f.Add(3,&speed); f.Add(4,&potential); return 0; }

	SState() {}
	void ShiftSinglePoint( CPos *pRes, const CPos &_delta, ICollider *pCollider, int nPoint, int nContactOffset, float fRadius );
	void ApplyDelta( const vector<CPos> &delta, ICollider *pCollider, int nContactOffset, const vector<float> &radiuses );
	void ApplyStringsReaction( vector<CPos> *pSpeed, const vector<SLink> &links );
	void GenerateDOFs( const vector<SDOF> &dofs, vector<Speed> *pRes ) const;
	void GenerateSpeed( const vector<SDOF> &dofGroups, const vector<float> &speeds, vector<CPos> *pRes );
	CVec3 CalcMassCenter();
};

class CAnimator
{
	ZDATA
	SState cur, prev;
	SState originalPos;
	vector<SLink> links;
	vector<SDOF> dofGroups;
	vector<float> radiuses;
	bool bPhrozen;
	int nGravity;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&cur); f.Add(3,&prev); f.Add(4,&originalPos); f.Add(5,&links); f.Add(6,&dofGroups); f.Add(7,&radiuses); f.Add(8,&bPhrozen); f.Add(9,&nGravity); return 0; }
public:
	void AddLink( int n1, int n2 );
	int AddPoint( int x, int y, int z, float fRadius );
	void StartNew();
	//void Relaxation();
	void Step( ICollider *pCollider );
	double CalcEnergy();
	CVec3 CalcMassCenter();
	void SetPosition( const CVec3 &vPos, const CQuat &q );
	void GetPosition( CVec3 *pRes, CQuat *pQ );
	void SetVelocity( const CVec3 &vMove, const CVec3 &vRot );
	bool HasStopped() const { return bPhrozen; }
	void SetGravity( int _n ) { nGravity = _n;}
};

}