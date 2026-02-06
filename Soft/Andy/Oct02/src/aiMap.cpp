#include "StdAfx.h"
#include "aiTerrain.h"
#include "aiTrace.h"
#include "aiCollider.h"
#include "..\Misc\BasicShare.h"
#include "GAnimFormat.h"
#include "..\DBFormat\DataGeometry.h"
#include "..\DBFormat\DataAnimation.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataRPG.h"
#include "GSceneUtils.h"
#include "Transform.h"
#include "ocTree.h"
#include "GMesh.h"
#include "aiRender.h"
#include "aiObjectLoader.h"
#include "GBind.h"
#include "aiPosition.h"
#include "wInterface.h"
#include "Bound.h"
#include "MemObject.h"
#include "BSPTree.h"
#include "aiVoxelRender.h"
#include "aiMap.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_MIN_FLOOR = -3;
////////////////////////////////////////////////////////////////////////////////////////////////////
static NGScene::CResourceTracker aiGeometryCheckers( "AIGeometries" );
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAnimation
{
	extern CBasicShare<int, CFileSkeleton> shareSkeletons;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
CBasicShare<int, CLoadGeometryInfo> shareAIModel(110); // используется также в MakeBuilding для определения какие куски присутсвуют в геометрии
CBasicShare<int, CFileSkinPointsLoad> shareSkinPoints(111);
CBasicShare<int, NGScene::CFileAIBind> shareAIBinds(118);
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVolumeNode;
class CConvexHull: public CObjectBase
{
public:
	struct SMap
	{
		int nPieceID, nUserID;
		SMap() {}
		SMap( int _nPieceID, int _nUserID ): nPieceID(_nPieceID), nUserID(_nUserID) {}
	};
	ZDATA
	CPtr<CVolumeNode> pNode;
	CDGPtr<CPtrFuncBase<CGeometryInfo> > pGeometry;
	SFBTransform pos;
	vector<SMap> pieces;
	SSourceInfo src;
	int nIndexInNode;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pNode); f.Add(3,&pGeometry); f.Add(4,&pos); f.Add(5,&pieces); f.Add(6,&src); f.Add(7,&nIndexInNode); return 0; }
	//
	CConvexHull() {}
	CConvexHull( CPtrFuncBase<CGeometryInfo> *_pGeometry, const SFBTransform &_pos,
		NDb::CRPGArmor *_pArmor, NWorld::IVisObj *_pSrc, int _nMask, int _nFloor ) 
		: pGeometry(_pGeometry), pos(_pos), src( _pSrc, _pArmor, _nFloor, _nMask ) {}//, nFloor(_nFloor) {}
	~CConvexHull();
	bool SetNode( CVolumeNode *_p, const SBound &_bound );
	const SBound& GetLinkedBound();
	void SetLinkedBound( const SBound &b );
	void AssignUserID( int nUserID );
	void EstimateBound( SBound *pRes );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStaticConvexHull: public CConvexHull
{
	OBJECT_NOCOPY_METHODS(CStaticConvexHull);
public:
	CStaticConvexHull() {}
	CStaticConvexHull( CPtrFuncBase<CGeometryInfo> *_pGeometry, const SFBTransform &_pos,
		NDb::CRPGArmor *_pArmor, NWorld::IVisObj *_pSrc, int _nMask, int _nFloor ) 
		: CConvexHull( _pGeometry, _pos, _pArmor, _pSrc, _nMask, _nFloor )
	{
	}
//	~CStaticConvexHull();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDynamicConvexHull: public CConvexHull
{
	OBJECT_NOCOPY_METHODS(CDynamicConvexHull);
public:
	ZDATA_(CConvexHull)
	CDGPtr<CFuncBase<SBound> > pBound;
	CDGPtr<CFuncBase<NAnimation::SSkeletonPose> > pAnimationTracker;
	//SBound linkedBound;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CConvexHull*)this); f.Add(2,&pBound); f.Add(3,&pAnimationTracker); return 0; }
	//
	CDynamicConvexHull() {}
	CDynamicConvexHull( CPtrFuncBase<CGeometryInfo> *_pGeometry, const SFBTransform &_pos,
		NDb::CRPGArmor *_pArmor, NWorld::IVisObj *_pSrc, int _nMask, int _nFloor,
		CFuncBase<SBound> *_pBound, CFuncBase<NAnimation::SSkeletonPose> *_pAnimation ) 
		: CConvexHull( _pGeometry, _pos, _pArmor, _pSrc, _nMask, _nFloor ),
		pBound( _pBound), pAnimationTracker(_pAnimation)
	{
	}
//	~CDynamicConvexHull();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static void Convert( SObjectInfo *pRes, const SConvexHull &h )
{
	pRes->points.resize( h.points.size() );
	for ( int i = 0; i < h.points.size(); ++i )
		h.trans.forward.RotateHVector( &pRes->points[i], h.points[i] );
	h.tris.BuildTriangleList( &pRes->tris );
	pRes->nPieceID = h.nUserID;
	pRes->nArmorID = h.src.pArmor ? h.src.pArmor->GetRecordID() : 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
static void Convert( list<SObjectInfo> *pRes, const SHullSet &h )
{
	SObjectInfo r;
	for ( vector<SConvexHull>::const_iterator i = h.objects.begin(); i != h.objects.end(); ++i )
	{
		Convert( &r, *i );
		pRes->push_back( r );
	}
	for ( vector<SConvexHull>::const_iterator i = h.terrain.begin(); i != h.terrain.end(); ++i )
	{
		Convert( &r, *i );
		pRes->push_back( r );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TTest>
static void SelectPieces( SHullSet *pRes, const TTest &f, CConvexHull *pHull, const SBound &b )
{
	//const SHMatrix &fwd = trans.forward;
	//CVec3 pos;
	//fwd.RotateHVector( &pos, pGeom->bound.s.ptCenter );
	//float fR = sqrt( CalcRadius2( pGeom->bound, fwd ) );
	if ( f( b.s.ptCenter, b.s.fRadius ) )//pos, fR ) )
	{
		const SFBTransform &trans = pHull->pos;
		pHull->pGeometry.Refresh();
		CGeometryInfo *pGeom = pHull->pGeometry->GetValue();
		if ( pHull->pieces.empty() )
		{
			for ( CGeometryInfo::CPieceMap::const_iterator i = pGeom->pieces.begin(); i != pGeom->pieces.end(); ++i )
			{
				//if ( !i->second.pBSPTree && bBreak )
				//	__debugbreak();
				SConvexHull ch( i->second.points, i->second.edges, trans, pHull->src, i->first, i->second.pBSPTree );
				if ( pHull->src.pUserData )
					pRes->objects.push_back( ch );
				else
					pRes->terrain.push_back( ch );
			}
		}
		else
		{
			for ( int i = 0; i < pHull->pieces.size(); ++i )
			{
				const CConvexHull::SMap &m = pHull->pieces[i];
				CGeometryInfo::CPieceMap::const_iterator k = pGeom->pieces.find( m.nPieceID );
				if ( k != pGeom->pieces.end() )
				{
					//if ( !k->second.pBSPTree && bBreak )
					//	__debugbreak();
					SConvexHull ch( k->second.points, k->second.edges, trans, pHull->src, m.nUserID, k->second.pBSPTree );
					if ( pHull->src.pUserData )
						pRes->objects.push_back( ch );
					else
						pRes->terrain.push_back( ch );
				}
				else
					ASSERT( 0 && "specified piece not found in geometry" );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAlwaysTrue
{
	bool operator()( const CVec3 &p, float f ) const { return true; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetGeometry( list<SObjectInfo> *pRes, vector<SMassSphere> *pSpheres, int nAIGeometryID, bool *pbClosed )
{
	CDGPtr<CPtrFuncBase<CGeometryInfo> > pGeom = shareAIModel.Get( nAIGeometryID );
	SFBTransform trans;
	Identity( &trans.forward );
	Identity( &trans.backward );
	pGeom.Refresh();
	CGeometryInfo *pGInfo = pGeom->GetValue();
	if ( !pGInfo )
		return;
	*pSpheres = pGInfo->spheres;
	CObj<CConvexHull> pHull = new CStaticConvexHull( pGeom, trans, 0, 0, 0, 0 );
	if ( pGInfo->pieces.size() > 6 )
		pHull->pieces.push_back( CConvexHull::SMap( 0, 0 ) );
	SHullSet res;
	SelectPieces( &res, SAlwaysTrue(), pHull, SBound() );
	Convert( pRes, res );
	if ( pbClosed )
	{
		*pbClosed = true;
		for ( CGeometryInfo::CPieceMap::iterator i = pGInfo->pieces.begin(); i != pGInfo->pieces.end(); ++i )
			*pbClosed = pbClosed && i->second.edges.IsClosed();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetSpheres( NDb::CModel *pModel, vector<SMassSphere> *pRes, CVec3 *pMassCenter )
{
	if ( !pModel || !pModel->pGeometry )
		return;
	NDb::CAIGeometry *pAIGeom = pModel->pGeometry->pAIGeometry;
	if ( !pAIGeom )
		return;
	if ( pModel->pSkeleton )
	{
		CDGPtr< CPtrFuncBase<CFileSkinPoints> > pGeom = shareSkinPoints.Get( pAIGeom->GetRecordID() );
		pGeom.Refresh();
		CFileSkinPoints *pInfo = pGeom->GetValue();
		for ( int i = 0; i < pInfo->spheres.size(); ++i )
			pRes->push_back( pInfo->spheres[i] );
		*pMassCenter = pInfo->massCenter;
	}
	else
	{
		CDGPtr< CPtrFuncBase<CGeometryInfo> > pGeom = shareAIModel.Get( pAIGeom->GetRecordID() );
		pGeom.Refresh();
		CGeometryInfo *pInfo = pGeom->GetValue();
		for ( int i = 0; i < pInfo->spheres.size(); ++i )
			pRes->push_back( pInfo->spheres[i] );
		*pMassCenter = pInfo->massCenter;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CHGSLayer
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFloorsSelector
{
	vector<char> take;

	bool IsTaken( int _nFloor ) const 
	{ 
		unsigned int n = _nFloor - N_MIN_FLOOR;
		if ( n >= take.size() ) 
			return true;
		return take[n] != 0; 
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//! node size
const int N_MINIMAL_OCTREE_NODE = 4; 
class CVolumeNode : public COcTreeNode<CVolumeNode, N_MINIMAL_OCTREE_NODE>
{
	OBJECT_BASIC_METHODS( CVolumeNode );
	typedef COcTreeNode<CVolumeNode, N_MINIMAL_OCTREE_NODE> CParent;
	void InformLowerTrackers( const SBound &b, int nMask, bool bDoorFlipped );
	void InformCurrentTrackers( const SBound &b, int nMask, bool bDoorFlipped );
public:
	struct STrackerDescr
	{
		ZDATA
		CPtr<IAIMapTracker> pTracker;
		SBound bound;
		int nMask;
		bool bInformOnDoorFlip;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pTracker); f.Add(3,&bound); f.Add(4,&nMask); f.Add(5,&bInformOnDoorFlip); return 0; }
		STrackerDescr() {}
		STrackerDescr( IAIMapTracker *_p, const SBound &_b, int _nMask, bool _bInform ): 
			pTracker(_p), bound(_b), nMask(_nMask), bInformOnDoorFlip( _bInform ) {}
	};
	struct SElementInfo
	{
		ZDATA
		CPtr<CConvexHull> pHull;
		int nFlags, nFloor;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pHull); f.Add(3,&nFlags); f.Add(4,&nFloor); return 0; }
	};
	typedef vector<SElementInfo> CElemList;
	ZDATA_(CParent)
	//CElemList hulls;
	vector<SElementInfo> hulls;
	vector<SBound> hullBounds;
	list<STrackerDescr> trackers;
	int nFree;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CParent*)this); f.Add(2,&hulls); f.Add(3,&hullBounds); f.Add(4,&trackers); f.Add(5,&nFree); return 0; }

	CVolumeNode() : nFree(-1) {}
	int AddHull( CConvexHull *pHull, const SBound &bound );
	void RemoveHull( int nIndex );//CConvexHull *pHull );
	void SetLinkedBound( int nIndex, const SBound &b );
	CConvexHull* GetHull( NWorld::IVisObj *pSrc, SBound *pBound );
	void InformTrackers( const SBound &b, int nMask, bool bDoorFlipped = false, bool bTraverseUp = true );
	void AddTracker( IAIMapTracker *_pTracker, const SBound &_bound, int nMask, bool bInformOnDoorFlip );
	virtual bool IsEmpty();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIMap: public IAIMap, public COrdinarySyncDst<NWorld::IVisObj,CAIMap>, public NWorld::IAIVisitor
{
	OBJECT_BASIC_METHODS(CAIMap);
	//typedef hash_map<int, CObj<CVolumeNode> > CVolumeNodesHash;
	typedef COrdinarySyncDst<NWorld::IVisObj,CAIMap> TParent;
	//
	ZDATA_(TParent)
	CObj<CVolumeNode> pRoot;
	list<CPtr<CDynamicConvexHull> > dynamicHulls;
	int nAllTrackersMask; // for fast checks
	int nMaxFloor;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TParent*)this); f.Add(2,&pRoot); f.Add(3,&dynamicHulls); f.Add(4,&nAllTrackersMask); f.Add(5,&nMaxFloor); return 0; }
	//
	CVolumeNode* GetNode( const CVec3 &ptCenter, float fRadius );
	CVolumeNode* GetNode( CConvexHull *pHull, SBound *pBound );
	void InsertHull( CConvexHull *pHull );
	void RegisterFloor( int nFloor );
	bool InitFloorsSelector( SFloorsSelector *pRes, const CFloorsSet &fs );
	void CallInform( CVolumeNode *pNode, CConvexHull *pHull, const SBound &b, bool bDoorFlipped = false );
	//
	// adding new hull 
	void AddHull( NDb::CAIGeometry *pAIGeom, 
		const SFBTransform &pos, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask );
	void AddHull( CMemObject *pAIGeom, 
		const SFBTransform &pos, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask, int nUserID );
	void AddAnimatedHull( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, 
		CFuncBase<NAnimation::SSkeletonPose> *pAnimation, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask );
	void AddFlippingHull( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, 
	CFuncBase<NAnimation::SSkeletonPose> *pAn1, CFuncBase<NAnimation::SSkeletonPose> *pAn2, 
	NDb::CRPGArmor *pArmor, int nFloor, int nMask, bool bOpen );
	void AddPieces( NDb::CAIGeometry *pAIGeom, const vector<SPieceMap> &parts,
		const SFBTransform &pos, 
		NDb::CRPGArmor *pArmor, int nFloor, int nMask );
	void AddTerrainPart( CPtrFuncBase<CTerrainPart> *pPart, NDb::CRPGArmor *pArmor, int nFloor, int nMask );
	//
	CConvexHull* GetHull( NWorld::IVisObj *pSrc, SBound *pBound );
	template<class TTest>
		void SelectHulls( SHullSet *pRes, const TTest &f, CVolumeNode *pNode, const SFloorsSelector &fSelect, int nMask, bool bSelect2DoorHulls = false )
		{
			if ( pNode == 0 )
				return;
			SSphere t;
			pNode->GetBound( &t );
			if ( !f( t.ptCenter, t.fRadius ) )
				return;
			for ( CVolumeNode::CElemList::iterator i = pNode->hulls.begin(); i != pNode->hulls.end(); ++i )
			{
				CConvexHull *pHull = i->pHull;
				if ( pHull )
				{
					ASSERT( IsValid(pHull) );
					int nFlags = i->nFlags;
					if ( ( nFlags & nMask ) != 0 && fSelect.IsTaken( i->nFloor ) )
					{
						if ( bSelect2DoorHulls || 
								( ( nFlags & ( NWorld::TS_STATE_OPEN | NWorld::TS_STATE_CLOSED ) ) == 0 ) ||
								( nFlags & NWorld::TS_DOOR_HULL_VALID ) )
						{
							SelectPieces( pRes, f, pHull, pNode->hullBounds[ i - pNode->hulls.begin() ] );
						}
					}
				}
			}
			for ( int i = 0; i < 8; ++i )
				SelectHulls( pRes, f, pNode->GetNode(i), fSelect, nMask, bSelect2DoorHulls );
		}
	template<class TTest>
		void SelectFloorSet( SHullSet *pRes, const CFloorsSet &fs, int nMask, const TTest &f, bool bSelect2DoorHulls = false )
		{
			SFloorsSelector fSelect;
			if ( !InitFloorsSelector( &fSelect, fs ) )
				return;
			SelectHulls( pRes, f, pRoot, fSelect, nMask, bSelect2DoorHulls );
		}
	void FlipDoorWindow( CObjectBase *pWhat, bool bOpen, CVolumeNode *pNode );
public:
	CAIMap(): nMaxFloor(0), nAllTrackersMask(0) {}
	CAIMap( NWorld::IWorld* );
	virtual void Sync();
	virtual void GetEntities( list<SObjectInfo> *pRes, int nMask, const CFloorsSet &fs, 
		vector<CPtr<CBSPTree> > *pTrees );
	virtual void Trace( const CRay &, vector<SInterval> *pIntersections, int nMask, const CFloorsSet &fs, ESplitTerrainHGroups shg );
	virtual void TraceUnit( const CRay &, vector<SInterval> *pIntersections, NWorld::IVisObj *pTarget );
	virtual void TraceUnit( CFastRenderer *pRes, NWorld::IVisObj *pTarget );
	virtual void TraceGrid( CFastRenderer *pRes, int nMask, ESort sort, const CFloorsSet &fs, 
		ESplitTerrainHGroups shg, bool bSelect2DoorHulls = false );
	virtual void TraceVoxelGrid( CVoxelRenderer *pRes, int nMask, const CFloorsSet &hg = CFloorsSet(),
		bool bSelect2DoorHulls = false );	
	virtual void GetUnitHLPos( CVec3 *pRes, NWorld::IVisObj *pTarget, int nUserID );
	virtual void GetAccessibleUnitHL( vector<int> *pRes, const CVec3 &ptFrom, NWorld::IVisObj *pTarget, float fMaxDistance );
	virtual bool CalcIntersection( const CVec3 &ptCenter, float fRadius, int s, NWorld::IVisObj *pIgnore );
	virtual void PrepareCollider( CCollider *pRes, const SBound &bound, float fElementSize, 
		const int flags, bool bSelect2DoorHulls = false  );
	virtual void AddTracker( IAIMapTracker *pTracker, const SBound &b, int nMask, bool bInformOnDoorFlip = false );
	virtual void FlipDoorWindow( CObjectBase *pWhat, bool bOpen ) { FlipDoorWindow( pWhat, bOpen, pRoot ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CConvexHull
////////////////////////////////////////////////////////////////////////////////////////////////////
void CConvexHull::AssignUserID( int nUserID )
{
	pGeometry.Refresh();
	CGeometryInfo &g = *pGeometry->GetValue();
	for ( CGeometryInfo::CPieceMap::const_iterator i = g.pieces.begin(); i != g.pieces.end(); ++i )
		pieces.push_back( SMap( i->first, nUserID ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CConvexHull::EstimateBound( SBound *pRes )
{
	CVec3 ptCenter;
	pGeometry.Refresh();
	const SHMatrix &fwd = pos.forward;
	const CGeometryInfo &g = *pGeometry->GetValue();
	float fRadius = sqrt( CalcRadius2( g.bound, fwd ) );
	fwd.RotateHVector( &ptCenter, g.bound.s.ptCenter );
	pRes->SphereInit( ptCenter, fRadius );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CConvexHull::SetNode( CVolumeNode *_p, const SBound &_bound )
{
	if ( pNode == _p )
		return false;
	if ( IsValid(pNode) )
		pNode->RemoveHull( nIndexInNode );
	pNode = _p;
	nIndexInNode = pNode->AddHull( this, _bound );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const SBound& CConvexHull::GetLinkedBound() 
{ 
	ASSERT( pNode ); 
	return pNode->hullBounds[nIndexInNode]; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CConvexHull::SetLinkedBound( const SBound &b )
{
	pNode->SetLinkedBound( nIndexInNode, b );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CConvexHull::~CConvexHull()
{
	if ( !IsValid(pNode) )
		return;
	pNode->RemoveHull( nIndexInNode );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CStaticConvexHull
////////////////////////////////////////////////////////////////////////////////////////////////////
/*CStaticConvexHull::~CStaticConvexHull() 
{
	if ( !IsValid(pNode) )
		return;
	SBound b;
	EstimateBound( &b );
	pNode->InformTrackers( b, src.nTSFlags );
	pNode->Remove( this ); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDynamicConvexHull
////////////////////////////////////////////////////////////////////////////////////////////////////
CDynamicConvexHull::~CDynamicConvexHull() 
{ 
	if ( !IsValid(pNode) )
		return;
	pNode->InformTrackers( linkedBound, src.nTSFlags );
	pNode->Remove( this ); 
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVolumeNode
////////////////////////////////////////////////////////////////////////////////////////////////////
static int nSlowVolumeWalk;
bool CVolumeNode::IsEmpty()
{
	bool bEmpty = true;
	if ( ( nSlowVolumeWalk & 0xff ) == 0 )
	{
		for ( int k = 0; k < hulls.size(); ++k )
		{
			if ( hulls[k].pHull )
			{
				ASSERT( IsValid( hulls[k].pHull ) );
				bEmpty = false;
				break;
			}
		}
		/*for ( CElemList::iterator i = hulls.begin(); i != hulls.end(); )
		{
			if ( !IsValid( *i ) )
				i = hulls.erase( i );
			else
				++i;
		}*/
		for ( list<STrackerDescr>::iterator i = trackers.begin(); i != trackers.end(); )
		{
			if ( !IsValid( i->pTracker ) )
				i = trackers.erase( i );
			else
				++i;
		}
		return bEmpty;
	}
	return false;//hulls.empty();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*void CVolumeNode::Remove( CConvexHull *pHull )
{
	for ( CElemList::iterator i = hulls.begin(); i != hulls.end(); )
	{
		if ( pHull == *i )
		{
			pHull->Attach( 0 );
			i = hulls.erase( i );
		}
		else
			++i;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVolumeNode::Add( CConvexHull *pHull )
{
	ASSERT( IsValid( this ) );
	pHull->Attach( this );
	hulls.push_back( pHull );
}*/
int CVolumeNode::AddHull( CConvexHull *pHull, const SBound &bound )
{
	// find place
	int nPlace;
	if ( nFree < 0 )
	{
		hulls.resize( hulls.size() + 1 );
		hullBounds.resize( hullBounds.size() + 1 );
		nPlace = hulls.size() - 1;
	}
	else
	{
		nPlace = nFree;
		nFree = hulls[nFree].nFlags;
	}
	// store data
	SElementInfo &info = hulls[nPlace];
	info.pHull = pHull;
	info.nFlags = pHull->src.nTSFlags;
	info.nFloor = pHull->src.nFloor;
	hullBounds[nPlace] = bound;
	InformTrackers( bound, info.nFlags );
	return nPlace;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVolumeNode::RemoveHull( int nIndex )
{
	InformTrackers( hullBounds[nIndex], hulls[nIndex].nFlags );
	hulls[nIndex].pHull = 0;
	hulls[nIndex].nFlags = nFree;
	nFree = nIndex;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVolumeNode::SetLinkedBound( int nIndex, const SBound &b )
{
	hullBounds[nIndex] = b;
	InformTrackers( hullBounds[nIndex], hulls[nIndex].nFlags );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVolumeNode::InformTrackers( const SBound &b, int nMask, bool bDoorFlipped, bool bTraverseUp )
{
	for ( list<STrackerDescr>::iterator i = trackers.begin(); i != trackers.end(); )
	{
		if ( (nMask & i->nMask) != 0 )
		{
			if ( !IsValid( i->pTracker ) )
				i = trackers.erase( i );
			else
			{
				if ( DoesIntersect( i->bound, b ) && ( (!bDoorFlipped) || i->bInformOnDoorFlip	) )
					i->pTracker->OnChange();
				++i;
			}
		}
		else
			++i;
	}
	if ( bTraverseUp )
	{
		CVolumeNode *pUp = GetUpLink();
		if ( pUp )
			pUp->InformTrackers( b, nMask, bDoorFlipped );
	}
	for ( int k = 0; k < 8; ++k )
		if ( GetNode(k) ) 
			GetNode(k)->InformTrackers( b, nMask, bDoorFlipped, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVolumeNode::AddTracker( IAIMapTracker *_pTracker, const SBound &_bound, int nMask, bool bInformOnDoorFlip )
{
	trackers.push_back( STrackerDescr( _pTracker, _bound, nMask, bInformOnDoorFlip ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CConvexHull* CVolumeNode::GetHull( NWorld::IVisObj *pSrc, SBound *pBound )
{
	if ( this == 0 )
		return 0;
	for ( int i = 0; i < hulls.size(); ++i )
	{
		CConvexHull *pHull = hulls[i].pHull;
		if ( IsValid( pHull ) && pSrc == pHull->src.pUserData )
		{
			if ( pBound )
				*pBound = hullBounds[i];
			return pHull;
		}
	}
	for ( int k = 0; k < 8; ++k )
	{
		CConvexHull *pRes = GetNode(k)->GetHull( pSrc, pBound );
		if ( pRes )
			return pRes;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAIMap
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIMap::CAIMap( NWorld::IWorld *_pWorld )
: COrdinarySyncDst<NWorld::IVisObj,CAIMap>( 
	new CBoolSyncSrc<NWorld::IVisObj, CUnionFunc>( _pWorld->GetActive(), _pWorld->GetUnits() ) ),
	nMaxFloor(0), nAllTrackersMask(0)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVolumeNode* CAIMap::GetNode( const CVec3 &ptCenter, float fRadius )
{
	if ( !pRoot )
	{
		pRoot = new CVolumeNode;
		pRoot->SetSize( CVec3( -128, -128, -128 ), 1024 );
	}
	return pRoot->GetNode( ptCenter, fRadius );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVolumeNode* CAIMap::GetNode( CConvexHull *pHull, SBound *pBound )
{
	pHull->EstimateBound( pBound );
	return GetNode( pBound->s.ptCenter, pBound->s.fRadius );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::InsertHull( CConvexHull *pHull )
{
	if ( !IsValid( pHull ) )
		return;
	SBound bTest;
	CVolumeNode *pNode = GetNode( pHull, &bTest );
	pHull->SetNode( pNode, bTest );
	//pNode->Add( pHull );
	//CallInform( pNode, pHull, bTest );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::RegisterFloor( int nFloor )
{
	nMaxFloor = Max( nFloor, nMaxFloor );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIMap::InitFloorsSelector( SFloorsSelector *pRes, const CFloorsSet &fs )
{
	if ( !fs.floors.empty() )
	{
		bool bTaken = false;
		pRes->take.resize( nMaxFloor - N_MIN_FLOOR + 1, 0 );
		for ( int k = 0; k < fs.floors.size(); ++k )
		{
			int nFloor = fs.floors[k];
			if ( nFloor <= nMaxFloor )
			{
				pRes->take[ nFloor - N_MIN_FLOOR ] = 1;
				bTaken = true;
			}
		}
		if ( !bTaken )
			return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::AddHull( NDb::CAIGeometry *pAIGeom, 
	const SFBTransform &pos, 
	NDb::CRPGArmor *pArmor, int nFloor, int nMask )
{
	if ( !IsValid( pAIGeom ) )
		return;
	if ( !NGScene::CResourceFileOpener::DoesExist( "AIGeometries", pAIGeom->GetRecordID() ) )
		return;
	RegisterFloor( nFloor );
	CConvexHull *pRes = new CStaticConvexHull( shareAIModel.Get( pAIGeom->GetRecordID() ), pos, pArmor, 
		GetCurrentSrcObject(), nMask, nFloor );
	InsertHull( pRes );
	Register( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::AddHull( CMemObject *pModel, const SFBTransform &pos, 
	NDb::CRPGArmor *pArmor, int nFloor, int nMask, int nUserID )
{
	if ( pModel->IsPolyLine() )
		return;
	RegisterFloor( nFloor );
	CConvexHull *pRes = new CStaticConvexHull( new CMemGeometryInfo( pModel ), pos, 0,
		GetCurrentSrcObject(), nMask, nFloor );
	pRes->AssignUserID( nUserID );
	InsertHull( pRes );
	Register( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::AddTerrainPart( CPtrFuncBase<CTerrainPart> *pPart, NDb::CRPGArmor *pArmor, int nFloor, int nMask )
{
	SFBTransform matrix;
	Identity( &matrix.forward );
	Identity( &matrix.backward );
	RegisterFloor( nFloor );
	CConvexHull *pRes = new CStaticConvexHull( new CTerrainGeometry( pPart ), matrix, pArmor,
		0, nMask,
		nFloor );
	InsertHull( pRes );
	Register( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::AddPieces( NDb::CAIGeometry *pAIGeom, const vector<SPieceMap> &parts,
	const SFBTransform &pos, 
	NDb::CRPGArmor *pArmor, int nFloor, int nMask )
{
	if ( !IsValid( pAIGeom ) )
		return;
	int key = pAIGeom->GetRecordID();
	if ( !aiGeometryCheckers.DoesExist( key ) )
		return;
	RegisterFloor( nFloor );
	CObj<CConvexHull> pHull = new CStaticConvexHull( shareAIModel.Get(key), pos, pArmor,
		GetCurrentSrcObject(), nMask, nFloor );
	pHull->pGeometry.Refresh();
	CGeometryInfo &g = *pHull->pGeometry->GetValue();
	vector<CConvexHull::SMap> pieces;
	for ( int k = 0; k < parts.size(); ++k )
	{
		if ( !g.HasPiece( parts[k].nPieceID ) )
			continue;
		pieces.push_back( CConvexHull::SMap( parts[k].nPieceID, parts[k].nUserID ) ); 
	}
	if ( pieces.empty() )
		return;
	pHull->pieces = pieces;
	InsertHull( pHull );
	Register( pHull.GetPtr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::AddAnimatedHull( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, 
	CFuncBase<NAnimation::SSkeletonPose> *pAnimation, 
	NDb::CRPGArmor *pArmor, int nFloor, int nMask )
{
	if ( !IsValid( pAIGeom ) )
		return;
	if ( !NGScene::CResourceFileOpener::DoesExist( "AIGeometries", pAIGeom->GetRecordID() ) )
		return;
	ASSERT( IsValid( pAnimation ) );
	if ( !IsValid( pSkeleton ) )
	{
		ASSERT( 0 ); // animation for non skinned model
		return;
	}
	NGScene::CBind *pBind = new NGScene::CBind;
	pBind->pBinds = shareAIBinds.Get( pAIGeom->GetRecordID() );
	pBind->pAnimation = pAnimation;
	pBind->pSkeleton = NAnimation::shareSkeletons.Get( pSkeleton->GetRecordID() );
	CSkinner *pSkin = new CSkinner( 
		shareSkinPoints.Get( pAIGeom->GetRecordID() ),
		pBind );
	
	SFBTransform id;
	Identity( &id.forward );
	Identity( &id.backward );
	CDynamicConvexHull *pRes = new CDynamicConvexHull( pSkin, id, pArmor, 
		GetCurrentSrcObject(), nMask, nFloor, 
		new NGScene::CMeshBound( pBind ), pAnimation );

	dynamicHulls.push_back( pRes );
	Register( pRes );
	// CRAP - animated objects on AI map cannot change destroy stages etc. with this ASSERT
	//ASSERT( ( nAllTrackersMask & nMask ) == 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::AddFlippingHull( NDb::CAIGeometry *pAIGeom, NDb::CSkeleton *pSkeleton, 
	CFuncBase<NAnimation::SSkeletonPose> *pAn1, CFuncBase<NAnimation::SSkeletonPose> *pAn2, 
	NDb::CRPGArmor *pArmor, int nFloor, int nMask, bool bOpen )
{
	if ( !IsValid( pAIGeom ) )
		return;
	if ( !NGScene::CResourceFileOpener::DoesExist( "AIGeometries", pAIGeom->GetRecordID() ) )
		return;
	ASSERT( IsValid( pAn1 ) );
	ASSERT( IsValid( pAn2 ) );
	if ( !IsValid( pSkeleton ) )
	{
		ASSERT( 0 ); // animation for non skinned model
		return;
	}

	NGScene::CBind *pBind1 = new NGScene::CBind;
	pBind1->pBinds = shareAIBinds.Get( pAIGeom->GetRecordID() );
	pBind1->pAnimation = pAn1;
	pBind1->pSkeleton = NAnimation::shareSkeletons.Get( pSkeleton->GetRecordID() );
	CSkinner *pSkin1 = new CSkinner( 
		shareSkinPoints.Get( pAIGeom->GetRecordID() ),
		pBind1 );
	pSkin1->CreateBSPTrees();

	NGScene::CBind *pBind2 = new NGScene::CBind;
	pBind2->pBinds = shareAIBinds.Get( pAIGeom->GetRecordID() );
	pBind2->pAnimation = pAn2;
	pBind2->pSkeleton = NAnimation::shareSkeletons.Get( pSkeleton->GetRecordID() );
	CSkinner *pSkin2 = new CSkinner( 
		shareSkinPoints.Get( pAIGeom->GetRecordID() ),
		pBind2 );
	pSkin2->CreateBSPTrees();

	SFBTransform id;
	Identity( &id.forward );
	Identity( &id.backward );
	CStaticConvexHull *pRes;
	int nMaskCur = nMask | NWorld::TS_STATE_CLOSED;
	if ( !bOpen )
		nMaskCur |= NWorld::TS_DOOR_HULL_VALID;
	pRes = new CStaticConvexHull( pSkin1, id,	pArmor, GetCurrentSrcObject(), nMaskCur, nFloor ); 
	InsertHull( pRes );
	Register( pRes );
	nMaskCur = nMask | NWorld::TS_STATE_OPEN;
	if ( bOpen )
		nMaskCur |= NWorld::TS_DOOR_HULL_VALID;
	pRes = new CStaticConvexHull( pSkin2, id,	pArmor, GetCurrentSrcObject(), nMaskCur, nFloor ); 
	InsertHull( pRes );
	Register( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CConvexHull* CAIMap::GetHull( NWorld::IVisObj *pSrc, SBound *pBound )
{
	return pRoot->GetHull( pSrc, pBound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::Sync()
{
	TParent::Sync();
	++nSlowVolumeWalk;
	if ( IsValid( pRoot ) )
		pRoot->Walk();
	for ( list<CPtr<CDynamicConvexHull> >::iterator i = dynamicHulls.begin(); i != dynamicHulls.end(); )
	{
		CDynamicConvexHull *pHull = *i;
		if ( !IsValid( pHull ) )
			i = dynamicHulls.erase( i );
		else
		{
			bool bInform = pHull->pAnimationTracker.Refresh(), bBoundChanged = pHull->pBound.Refresh();
			if ( bBoundChanged )
			{
				const SBound &b = pHull->pBound->GetValue();
				CVolumeNode *pNode = GetNode( b.s.ptCenter, b.s.fRadius );
				bInform = !pHull->SetNode( pNode, b );
/*				if ( pHull->pNode != pNode )
				{
					if ( pHull->pNode )
					{
						CallInform( pHull->pNode, pHull, pHull->linkedBound );
						pHull->pNode->Remove( pHull );
					}
					pNode->Add( pHull );
					pHull->linkedBound = b;
					CallInform( pHull->pNode, pHull, pHull->linkedBound );
					bInform = false;
				}
				else
					bInform = true;*/
			}
			if ( bInform && IsValid( pHull->pNode ) )
			{
				CallInform( pHull->pNode, pHull, pHull->GetLinkedBound() );
				if ( bBoundChanged )
				{
					const SBound &b = pHull->pBound->GetValue();
					pHull->SetLinkedBound( b );
/*					pHull->linkedBound = b;
					CallInform( pHull->pNode, pHull, GetBound( pHull->GetLinkedBound() ) );*/
				}
			}
			++i;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::CallInform( CVolumeNode *pNode, CConvexHull *pHull, const SBound &b, bool bDoorFlipped )
{
	if ( pHull->src.nTSFlags & nAllTrackersMask )
		pNode->InformTrackers( b, pHull->src.nTSFlags, bDoorFlipped );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::AddTracker( IAIMapTracker *pTracker, const SBound &b, int nMask, bool bInformOnDoorFlip )
{
	CVolumeNode *pNode = GetNode( b.s.ptCenter, b.s.fRadius );
	pNode->AddTracker( pTracker, b, nMask, bInformOnDoorFlip );
	nAllTrackersMask |= nMask;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
struct STestSphere
{
	T *p;
	STestSphere( T *_p ): p(_p) {}
	bool operator()( const CVec3 &ptCenter, float fRadius ) const { return p->TestSphere( ptCenter, fRadius ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void TraceEntities( SHullSet &res, T *pRes, IAIMap::ESplitTerrainHGroups shg )
{
	if ( shg == IAIMap::STH_SPLIT_TERR_HG )
	{
		for ( vector<SConvexHull>::iterator i = res.terrain.begin(); i != res.terrain.end(); ++i )
			pRes->TraceEntity( *i, true );
	}
	else
		pRes->TraceEntity( res.terrain, true );
	for ( vector<SConvexHull>::iterator i = res.objects.begin(); i != res.objects.end(); ++i )
	{
		pRes->TraceEntity( *i, false );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::Trace( const CRay &r, vector<SInterval> *pIntersections, int nMask, const CFloorsSet &fs, ESplitTerrainHGroups shg )
{
	SHullSet res;
	CTracer trace( *pIntersections );
	trace.InitProjection( r );
	SelectFloorSet( &res, fs, nMask, STestSphere<CTracer>( &trace ) );
	TraceEntities( res, &trace, shg );
	SortIntervals( pIntersections );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*static SCompareHull
{
	bool operator()( SConvexHull &a, SConvexHull &b ) const {}
}*/
void CAIMap::TraceGrid( CFastRenderer *pRes, int nMask, ESort sort, const CFloorsSet &fs, ESplitTerrainHGroups shg,
	bool bSelect2DoorHulls )
{
	SHullSet res;
	SelectFloorSet( &res, fs, nMask, STestSphere<CFastRenderer>( pRes ), bSelect2DoorHulls );
	/*if ( sort == STH_SORT_INTERVALS )
	{
		sort( res.objects.begin(), res.objects.end(), SCompareHull() );
		sort( res.
	}*/
	TraceEntities( res, pRes, shg );
	if ( sort == STH_SORT_INTERVALS )
		pRes->SortIntervals();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::TraceVoxelGrid( CVoxelRenderer *pRes, int nMask, const CFloorsSet &fs, bool bSelect2DoorHulls )
{
	SHullSet res;
	pRes->InitParallel( CVoxelRenderer::AXIS_X );
	SelectFloorSet( &res, fs, nMask, STestSphere<CVoxelRenderer>( pRes ), bSelect2DoorHulls );
	TraceEntities( res, pRes, STH_SPLIT_TERR_HG );
	pRes->InitParallel( CVoxelRenderer::AXIS_Y );
	TraceEntities( res, pRes, STH_SPLIT_TERR_HG );
	pRes->InitParallel( CVoxelRenderer::AXIS_Z );
	TraceEntities( res, pRes, STH_SPLIT_TERR_HG );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::TraceUnit( const CRay &r, vector<SInterval> *pIntersections, NWorld::IVisObj *pTarget )
{
	SBound bound;
	CConvexHull *pHull = GetHull( pTarget, &bound );
	if ( pHull )
	{
		SHullSet res;
		CTracer trace( *pIntersections );
		trace.InitProjection( r );
		SelectPieces( &res, STestSphere<CTracer>(&trace), pHull, bound );
		for ( vector<SConvexHull>::iterator i = res.objects.begin(); i != res.objects.end(); ++i )
			trace.TraceEntity( *i, false );
		SortIntervals( pIntersections );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::TraceUnit( CFastRenderer *pRes, NWorld::IVisObj *pTarget )
{
	SBound bound;
	CConvexHull *pHull = GetHull( pTarget, &bound );
	if ( pHull )
	{
		SHullSet res;
		SelectPieces( &res, STestSphere<CFastRenderer>(pRes), pHull, bound );
		for ( vector<SConvexHull>::iterator i = res.objects.begin(); i != res.objects.end(); ++i )
			pRes->TraceEntity( *i, false );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::GetUnitHLPos( CVec3 *pRes, NWorld::IVisObj *pTarget, int nUserID )
{
	CConvexHull *pHull = GetHull( pTarget, 0 );
	CVec3 tmp(0,0,0);
	if ( !pHull || !IsValid( pHull ) )
	{
		*pRes = tmp;
		return;
	}
	pHull->pGeometry.Refresh();
	CGeometryInfo *pGeom = pHull->pGeometry->GetValue();
	tmp = pGeom->bound.s.ptCenter;
	CGeometryInfo::SPiece *pPiece = pGeom->GetPiece( nUserID );
	if ( pPiece )
	{
		SSphere s;
		CalcBound( &s, pPiece->points, SGetSelf<CVec3>() );
		tmp = s.ptCenter;
	}
	pHull->pos.forward.RotateHVector( pRes, tmp );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::GetAccessibleUnitHL( vector<int> *pRes, const CVec3 &ptFrom, NWorld::IVisObj *pTarget, float fMaxDistance )
{
	CConvexHull *pHull = GetHull( pTarget, 0 );
	if ( !pHull || !IsValid( pHull ) )
		return;
	pHull->pGeometry.Refresh();
	CGeometryInfo *pGeom = pHull->pGeometry->GetValue();
	for ( CGeometryInfo::CPieceMap::const_iterator i = pGeom->pieces.begin(); i != pGeom->pieces.end(); ++i )
	{
		const CGeometryInfo::SPiece &piece = i->second;
		SSphere s;
		CalcBound( &s, piece.points, SGetSelf<CVec3>() );
		CVec3 tmp;
		pHull->pos.forward.RotateHVector( &tmp, s.ptCenter );
		if ( fabs( tmp - ptFrom ) < fMaxDistance )
			pRes->push_back( i->first );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSphereSphere
{
	CVec3 ptCenter;
	float fRadius;

	SSphereSphere( const CVec3 &_ptCenter, float _fRadius ): ptCenter(_ptCenter), fRadius(_fRadius) {}
	bool operator()( const CVec3 &_ptCenter, float _fRadius ) const 
	{ 
		return fabs2( ptCenter - _ptCenter ) < sqr( fRadius + _fRadius ); 
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool CalcModelSphereIntersection( const SConvexHull &h, const CVec3 &ptCenter, float fRadius )
{
	//if ( !h.pBSPTree )
	{
		// SLOW implementation
		const SHMatrix &rot = h.trans.forward;
		vector<CVec3> pts;
		pts.resize( h.points.size() );
		for ( int i = 0; i < pts.size(); ++i )
			rot.RotateHVector( &pts[i], h.points[i] );
		vector<STriangle> tris;
		h.tris.BuildTriangleList( &tris );
		for ( int i = 0; i < tris.size(); ++i )
		{
			const STriangle &t = tris[i];
			if ( NCollider::DoesTriSphereIntersect( pts[t.i1], pts[t.i2], pts[t.i3], ptCenter, fRadius ) )
				return true;
		}
		return false;
	}
	const SHMatrix &rot = h.trans.backward;
	CVec3 newCenter; 
	rot.RotateHVector( &newCenter, ptCenter );
	float fR = Max( sqr( rot._11 ) + sqr( rot._21 ) + sqr( rot._31 ), sqr( rot._12 ) + sqr( rot._22 ) + sqr( rot._32 ) );
	fR = Max( fR, sqr( rot._13 ) + sqr( rot._23 ) + sqr( rot._33 ) );
	fR = sqrt( fR ) * fRadius;
#ifdef _BSP_DEBUG
	vector<CVec3> fake;vector<bool> fake2;
	return h.pBSPTree->DoesIntersect( newCenter, fR, &fake, &fake2 );
#else
	return h.pBSPTree->DoesIntersect( newCenter, fR );
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIMap::CalcIntersection( const CVec3 &vCenter, float fRadius, int nMask, NWorld::IVisObj *pIgnore )
{
	SHullSet res;
	SelectFloorSet( &res, CFloorsSet(), nMask, SSphereSphere( vCenter, fRadius ) );
	for ( vector<SConvexHull>::iterator i = res.objects.begin(); i != res.objects.end(); ++i )
	{
		if ( pIgnore && pIgnore == i->src.pUserData )
			continue;
		if ( CalcModelSphereIntersection( *i, vCenter, fRadius ) )
			return true;
	}
	for ( vector<SConvexHull>::iterator i = res.terrain.begin(); i != res.terrain.end(); ++i )
	{
		if ( pIgnore && pIgnore == i->src.pUserData )
			continue;
		if ( CalcModelSphereIntersection( *i, vCenter, fRadius ) )
			return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::GetEntities( list<SObjectInfo> *pRes, int nMask, const CFloorsSet &fs, 
	vector<CPtr<CBSPTree> > *pTrees )
{
	SHullSet res;
	pRes->clear();	
	SelectFloorSet( &res, fs, nMask, SAlwaysTrue() );
	Convert( pRes, res );
	if ( pTrees )
	{
		for ( vector<SConvexHull>::const_iterator i = res.objects.begin(); i != res.objects.end(); ++i )
			pTrees->push_back( i->pBSPTree );
		for ( vector<SConvexHull>::const_iterator i = res.terrain.begin(); i != res.terrain.end(); ++i )
			pTrees->push_back( i->pBSPTree );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::PrepareCollider( CCollider *pRes, const SBound &bound, float fElementSize, 
	const int flags, bool bSelect2DoorHulls )
{
	pRes->PrepareCollider( bound, fElementSize );
	SHullSet res;
	SelectFloorSet( &res, CFloorsSet(), flags, SSphereSphere( bound.s.ptCenter, bound.s.fRadius ), bSelect2DoorHulls );
	for ( vector<SConvexHull>::iterator i = res.objects.begin(); i != res.objects.end(); ++i )
		pRes->AddConvexHull( *i );
	for ( vector<SConvexHull>::iterator i = res.terrain.begin(); i != res.terrain.end(); ++i )
		pRes->AddConvexHull( *i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIMap::FlipDoorWindow( CObjectBase *pWhat, bool bOpen, CVolumeNode *pNode )
{
	if (!pNode)
		return;
	int nDoorFlag = bOpen? NWorld::TS_STATE_OPEN : NWorld::TS_STATE_CLOSED,
			nFullFlag = NWorld::TS_STATE_OPEN | NWorld::TS_STATE_CLOSED;
	for ( CVolumeNode::CElemList::iterator i = pNode->hulls.begin(); i != pNode->hulls.end(); ++i )
	{
		CConvexHull *pHull = i->pHull;
		if ( IsValid( pHull ) )
		{
			int nFlags = i->nFlags;
			if ( ( nFlags & nFullFlag ) == 0 )
				continue;
			if ( pHull->src.pUserData != pWhat )
				continue;
			if ( nFlags & nDoorFlag )
				pHull->src.nTSFlags |= NWorld::TS_DOOR_HULL_VALID;
			else
				pHull->src.nTSFlags &= ~NWorld::TS_DOOR_HULL_VALID;
			i->nFlags = pHull->src.nTSFlags;
			SBound bTest;
			GetNode( pHull, &bTest );
			CallInform( pNode, pHull, bTest, true );
		}
	}
	for ( int i = 0; i < 8; ++i )
		FlipDoorWindow( pWhat, bOpen, pNode->GetNode(i) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIMap* CreateAIMap( NWorld::IWorld *pWorld )
{
	return new CAIMap( pWorld );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NAI;
REGISTER_SAVELOAD_CLASS( 0x02911000, CAIMap )
REGISTER_SAVELOAD_CLASS( 0x01071140, CVolumeNode )
BASIC_REGISTER_CLASS( IAIMap )
REGISTER_SAVELOAD_CLASS( 0x02942160, CStaticConvexHull )
REGISTER_SAVELOAD_CLASS( 0x02942161, CDynamicConvexHull )
