#include "StdAfx.h"
#include <hash_map>
#include <vector>
#include <fstream>
#include <iostream>
#include "Basic2.h"
#include "Data.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TData>
class CSharedDesc
{
	// caching
	void *pManager; 
	CPtr< TData > pData;
	//
	int nHashValue;
protected:
	void SetHashValue( int nHash ) { nHashValue = nHash; }
public:
	CSharedDesc() { pManager = 0; nHashValue = -1; }
	int GetHashValue() const { return nHashValue; }
	friend class CSharedDescAccess;
};

class CSharedDescAccess
{
public:
template<class TData>
	static void SetCache( CSharedDesc<TData> &res, void *_pManager, TData *_pData ) { res.pManager = _pManager; res.pData = _pData; }
template<class TData>
	static bool CheckCache( CSharedDesc<TData> &res, void *_pManager ) { return res.pManager == _pManager && IsValid( res.pData ); }
template<class TData>
	static TData* GetCache( CSharedDesc<TData> &res ) { return res.pData; }
};

template <class TDesc, class TData>
class CSharedManager
{
	struct SSharedHash
	{
		int operator()( const TDesc &a ) const { return a.GetHashValue(); }
	};
	CSharedDescAccess descAccess;
	typedef std::hash_map<TDesc, CObj<TData>, SSharedHash> CSharedHash;
	CSharedHash objects;
protected:
	virtual TData* NewObject( TDesc &desc ) = 0;
public:
	virtual void Clear() { objects.clear(); }
	TData* GetData( TDesc &desc );
};

template <class TDesc, class TData>
TData* CSharedManager<TDesc,TData>::GetData( TDesc &desc )
{
	if ( descAccess.CheckCache( desc, this ) )
		return descAccess.GetCache( desc );
	TData *pRes;
	CSharedHash::iterator p = objects.find( desc );
	if ( p != objects.end() )
	{
		pRes = p->second;
		descAccess.SetCache( desc, this, pRes );
		return pRes;
	}
	pRes = NewObject( desc );
	objects[desc] = pRes;
	descAccess.SetCache( desc, this, pRes );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NConverter
{

class CVertex: public CObjectBase
{
OBJECT_BASIC_METHODS(CVertex);
public:
	int nIdx[3];
	int nNumber;
	CVertex() {}
	CVertex( int nNum, int *nNIdx ) { nNumber = nNum; memcpy( nIdx, nNIdx, sizeof(nIdx) ); }
};

class CVertexDesc: public CSharedDesc<CVertex>
{
public:
	int nIdx[3];
	CVertexDesc( int *nNIdx ) { memcpy( nIdx, nNIdx, sizeof(nIdx) ); SetHashValue( nIdx[0]+nIdx[1]+nIdx[2] ); }
	bool operator==( const CVertexDesc &a ) const { return memcmp( nIdx, a.nIdx, sizeof(nIdx) ) == 0; }
};

class CVertexManager: public CSharedManager<CVertexDesc,CVertex>
{
	int nTotalVertices;
protected:
	virtual CVertex* NewObject( CVertexDesc &desc )
	{
		verts.push_back( SVertex( gv[desc.nIdx[0]], nv[desc.nIdx[1]], tv[desc.nIdx[2]] ) );
		links.push_back( desc.nIdx[0] );
		return new CVertex( nTotalVertices++, desc.nIdx );
	}
public:
	CVertexManager() { nTotalVertices = 0; }
	void Clear() { nTotalVertices = 0; CSharedManager<CVertexDesc,CVertex>::Clear(); }
};

	vector<SJoint> joints;
	vector<SHMatrix> binds;
	vector<SVertexWeight> weights;

	CVertexManager vm;
	vector<CVec3> gv, nv, tv;
	vector<SVertex> verts;
	hash_map< int, SFacesVector > faces;

	vector<int> links;

	vector<SRootAnimationKey> keysRoot;
	vector<SQuat> keysJoints;
	vector<SAddBoneAnimationKey> keysAddBones;
	vector<SMSRAnimationKey> keysMSR;	
	vector<SSphereMass> spheres;
	hash_map< int, STempParticle > particles;


void ClearAll()
{
	joints.clear();
	binds.clear();
	weights.clear();
	links.clear();
	gv.clear();
	nv.clear();
	tv.clear();
	verts.clear();
	faces.clear();
	keysRoot.clear();
	keysJoints.clear();
	keysAddBones.clear();
	keysMSR.clear();
	spheres.clear();
	particles.clear();
	vm.Clear();
}

void AddFace( int nSet, int *pIndices )
{
	CPtr<CVertex> pV1 = vm.GetData( CVertexDesc( pIndices ) );
	CPtr<CVertex> pV2 = vm.GetData( CVertexDesc( pIndices + 3 ) );
	CPtr<CVertex> pV3 = vm.GetData( CVertexDesc( pIndices + 6 ) );
	faces[nSet].faces.push_back( SFace( pV1->nNumber, pV2->nNumber, pV3->nNumber ) );
}

};
////////////////////////////////////////////////////////////////////////////////////////////////////
