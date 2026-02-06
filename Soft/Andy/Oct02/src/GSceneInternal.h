#ifndef __GSCENEINTERNAL_H_
#define __GSCENEINTERNAL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GScene.h"
#include "OcTree.h"
#include "GSceneUtils.h"
#include "GRenderCore.h"
#include "GCombiner.h"
#include "GRenderExecute.h"
#include "GTransparent.h"
#include "GLightmapCalc.h"
#include "DiscretePos.h"
namespace NDb
{
	class CMaterial;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
struct SDynamicLightGrouping;
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_OCCLUDER_FLAG = 0x2000;
struct SCombinedKey
{
	ZDATA
	CPtr<IMaterial> pMaterial;
	SGroupInfo groupInfo;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pMaterial); f.Add(3,&groupInfo); return 0; }
	
	SCombinedKey() {}
	SCombinedKey( IMaterial *_pMaterial, const SGroupInfo &_gInfo )
		: pMaterial(_pMaterial), groupInfo(_gInfo)
	{
		if ( !pMaterial->DoesCastShadow() )
			groupInfo.nObjectGroup &= 0x7fff;
		if ( pMaterial->GetType() == IMaterial::MT_OCCLUDER )
			groupInfo.nObjectGroup = N_OCCLUDER_FLAG;
		else
			groupInfo.nObjectGroup &= ~N_OCCLUDER_FLAG;
	}
	//
	bool operator==( const SCombinedKey &k ) const { return pMaterial == k.pMaterial && groupInfo == k.groupInfo; }
};
struct SCombinedKeyHash
{
	int operator()( const SCombinedKey &s ) const 
	{ 
		return ((int)s.pMaterial.GetPtr()) ^ s.groupInfo.nLightGroup ^ s.groupInfo.nObjectGroup;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVolumeNode;
class CCombinedPart;
class CNonePart : public IPart
{
	OBJECT_NOCOPY_METHODS(CNonePart);
	static CTRect<int> zeroRect;
public:
	ZDATA_(IPart)
	CPtr<CCombinedPart> pOwner;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(IPart*)this); f.Add(2,&pOwner); return 0; }
	CObj<CLMRegion> pLightmapInfo;
	CNonePart() {}
	CNonePart( CPtrFuncBase<CObjectInfo> *pData ) : IPart( pData, 0 ) {}
	virtual const CTRect<int>& GetLMRegion() const { if ( pLightmapInfo ) return pLightmapInfo->lmRegion; return zeroRect; }
	virtual int GetLMLOD() const { if ( pLightmapInfo ) return pLightmapInfo->nLOD; return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSimplePart : public CNonePart
{
	OBJECT_NOCOPY_METHODS(CSimplePart);
	ZDATA_(CNonePart)
	SFBTransform pos;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CNonePart*)this); f.Add(2,&pos); return 0; }
public:
	CSimplePart() {}
	CSimplePart( CPtrFuncBase<CObjectInfo> *pData, const SFBTransform &_pos ) : CNonePart( pData ), pos(_pos) {}
	virtual ETransformType GetTransformType() const { return TT_SIMPLE; }
	virtual const SFBTransform& GetSimplePos() { return pos; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDiscretePart : public CNonePart
{
	OBJECT_NOCOPY_METHODS(CDiscretePart);
	ZDATA_(CNonePart)
	SDiscretePos dPos;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CNonePart*)this); f.Add(2,&dPos); return 0; }
public:
	CDiscretePart() {}
	CDiscretePart( CPtrFuncBase<CObjectInfo> *pData, const SDiscretePos &_pos ) : CNonePart( pData ), dPos(_pos) {}
	virtual ETransformType GetTransformType() const { return TT_SIMPLE_DISCRETE; }
	virtual const SDiscretePos& GetDiscretePos() { return dPos; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGenericDynamicPart : public CNonePart
{
protected:
	ZDATA_(CNonePart)
	int nStillCounter;
	CPtr<IMaterial> pMaterial;
	SGroupInfo groupInfo;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CNonePart*)this); f.Add(2,&nStillCounter); f.Add(3,&pMaterial); f.Add(4,&groupInfo); return 0; }
public:
	CGenericDynamicPart() {}
	CGenericDynamicPart( CPtrFuncBase<CObjectInfo> *pData, IMaterial *_pMaterial, const SGroupInfo &_gInfo );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDynamicPart : public CGenericDynamicPart
{
	OBJECT_BASIC_METHODS( CDynamicPart );
private:
	ZDATA_(CGenericDynamicPart)
	SBound bound;
	CDGPtr<CFuncBase<SFBTransform> > pTransform;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CGenericDynamicPart*)this); f.Add(2,&bound); f.Add(3,&pTransform); return 0; }

public:
	CDynamicPart() {}
	CDynamicPart( CPtrFuncBase<CObjectInfo> *pData, CFuncBase<SFBTransform> *pPos, IMaterial *_pMaterial, const SGroupInfo &_gInfo );
	virtual ETransformType GetTransformType() const { return TT_SIMPLE; }
	virtual const SFBTransform& GetSimplePos() { return pTransform->GetValue(); }
	bool Update( CVolumeNode *pVolume, CVersioningBase *pStaticTracker, CLightmapTracker *plmTracker );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAnimatedPart : public CGenericDynamicPart
{
	OBJECT_NOCOPY_METHODS( CAnimatedPart );
	ZDATA_(CGenericDynamicPart)
	CDGPtr<CFuncBase< vector<SHMatrix> > > pAnimation;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CGenericDynamicPart*)this); f.Add(2,&pAnimation); return 0; }

	void EstimateBound( SBound *pRes );
public:
	CAnimatedPart() {}
	CAnimatedPart( CPtrFuncBase<CObjectInfo> *pData, CFuncBase< vector<SHMatrix> > *pAnim, IMaterial *_pMaterial, const SGroupInfo &_gInfo );
	virtual ETransformType GetTransformType() const { return TT_SINGLE_SKIN; }
	virtual const vector<SHMatrix>& GetAnimation() { return pAnimation->GetValue(); }
	bool Update( CVolumeNode *pVolume, CVersioningBase *pStaticTracker, CLightmapTracker *plmTracker );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDynamicGeometryPart : public CGenericDynamicPart
{
	OBJECT_NOCOPY_METHODS( CDynamicGeometryPart );
	ZDATA_(CGenericDynamicPart)
	CDGPtr<CFuncBase<SBound> > pBound;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CGenericDynamicPart*)this); f.Add(2,&pBound); return 0; }
public:
	CDynamicGeometryPart() {}
	CDynamicGeometryPart( CPtrFuncBase<CObjectInfo> *pData, CFuncBase<SBound> *pAnim, IMaterial *_pMaterial, const SGroupInfo &_gInfo );
	virtual ETransformType GetTransformType() const { return TT_NONE; }
	bool Update( CVolumeNode *pVolume, CVersioningBase *pStaticTracker, CLightmapTracker *plmTracker );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CParticles: public IParticles
{
	OBJECT_NOCOPY_METHODS(CParticles);
	ZDATA
	SBound bound;
	CDGPtr< CPtrFuncBase<CParticleEffect> > pParticles;
	CDGPtr< CFuncBase<SFBTransform> > pPlacement;
	CPtr<CVolumeNode> pNode;
	SGroupInfo groupInfo;
	bool bIsLit;
	SBound transformedBound;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bound); f.Add(3,&pParticles); f.Add(4,&pPlacement); f.Add(5,&pNode); f.Add(6,&groupInfo); f.Add(7,&bIsLit); f.Add(8,&transformedBound); return 0; }
public:
	CParticles() {}
	CParticles( CPtrFuncBase<CParticleEffect> *_pParticles, CFuncBase<SFBTransform> *_pPlacement, 
		const SBound &_bound, const SGroupInfo &_g, bool _bIsLit )
		: pParticles(_pParticles), pPlacement(_pPlacement), bound(_bound), groupInfo(_g), bIsLit(_bIsLit) {}
	const SGroupInfo& GetGroup() const { return groupInfo; }
	const SBound& GetBound() const { return transformedBound; }
	CParticleEffect* GetEffect();
	bool Update( CVolumeNode *pVolume );
	bool IsLit() const { return bIsLit; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//! selection
class CSelection: public CObjectBase
{
	OBJECT_BASIC_METHODS( CSelection );
private:
	ZDATA
	CVec4 vColor;
	CPtr<CNonePart> pTarget;
	CObj<CAutomaticCombiner> pCombo;
	CDGPtr<CVBCombiner> pOffsetVertices;
	CDGPtr< CFuncBase<vector<NGfx::STriangleList> > > pTriList;
	CObj<CVersioningBase> pAnimation;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&vColor); f.Add(3,&pTarget); f.Add(4,&pCombo); f.Add(5,&pOffsetVertices); f.Add(6,&pTriList); f.Add(7,&pAnimation); return 0; }
public:
	CSelection() {}
	bool Initialize( CObjectBase *pPart, const CVec4 &vColor );
	void Render( CTransformStack *pTS, NGfx::CRenderContext *pRC, bool bOffset );
	bool Update( IGScene *pScene );
	const CVec4& GetColor() const { return vColor; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCombinedPart: public ILightmappedElement
{
	OBJECT_BASIC_METHODS(CCombinedPart);
	ZDATA
	CObj<IMaterial> pMaterial;
	SRenderGeometryInfo geometryInfo;
	SGroupInfo groupInfo;
	CDGPtr<CPerMaterialCombiner> pCombiner;
	int nIgnoreMark;
	TPartFlags ignoredParts;
	CObj<CVersioningBase> pLMMapping;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pMaterial); f.Add(3,&geometryInfo); f.Add(4,&groupInfo); f.Add(5,&pCombiner); f.Add(6,&nIgnoreMark); f.Add(7,&ignoredParts); f.Add(8,&pLMMapping); return 0; }
public:
	CCombinedPart() : nIgnoreMark(0) {}
	CCombinedPart( const SCombinedKey &k, CVersioningBase *pTracker, bool bIsLightmapped );
	CPerMaterialCombiner* GetCombiner() const { return pCombiner; }
	virtual CVersioningBase* GetOnChangeNode() const { return pCombiner; }
	void AddForLightmapRender( CSceneFragments *pRes );
	IVBCombiner* GetVBCombiner() const { return geometryInfo.pVertices; }
	IMaterial* GetMaterial() const { return pMaterial; }
	const SGroupInfo& GetGroup() const { return groupInfo; }
	SRenderGeometryInfo* GetGeometryInfo() { return &geometryInfo; }
	void SetIgnored( int _nIgnoreMark, TPartFlags nParts );
	int GetIgnoreMark() const { return nIgnoreMark; }
	TPartFlags GetIgnoreFlags() const { return ignoredParts; }
	void UpdateLMMapping() { pLMMapping->Updated(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTransElementWrapper: public ITransparent
{
	OBJECT_BASIC_METHODS( CTransElementWrapper );
	CObj<CCombinedPart> pElement;
	CVec3 ptCenter;
public:
	CTransElementWrapper() {}
	CTransElementWrapper( CCombinedPart *_pElem, const CVec3 &_ptCenter ): pElement(_pElem), ptCenter(_ptCenter) {}
	void Render( NGfx::CRenderContext *pRC, NGfx::CTexture *pFog );
	float GetDepth( IParticleOutput *pInfo );
	int operator&( CStructureSaver &f ) { ASSERT( 0 ); return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//! node size
const int N_MINIMAL_OCTREE_NODE = 16; 
class CVolumeNode: public COcTreeNode<CVolumeNode, N_MINIMAL_OCTREE_NODE>
{
	OBJECT_BASIC_METHODS( CVolumeNode );
public:
	struct SPerMaterialHolder
	{
		typedef std::hash_map<SCombinedKey, vector<CObj<CCombinedPart> >, SCombinedKeyHash> CMaterialMap;
		CMaterialMap data;
		list<CPtr<CCombinedPart> > elements;

		CCombinedPart* GetCombinerPartForAdd( const SCombinedKey &k, CVersioningBase *pTracker, bool bIsLightmapped )
		{
			vector<CObj<CCombinedPart> > &nodes = data[k];
			for ( int k = 0; k < nodes.size(); ++k )
			{
				if ( nodes[k]->GetCombiner()->GetSize() < PF_MAX_PARTS_PER_COMBINER )
					return nodes[k];
			}
			CCombinedPart *pRes = new CCombinedPart( k, pTracker, bIsLightmapped );
			nodes.push_back( pRes );
			elements.push_back( pRes );
			return pRes;
		}
		void Walk()
		{
			if ( elements.empty() )
				return;
			for ( CMaterialMap::iterator i = data.begin(); i != data.end(); )
			{
				CMaterialMap::iterator tek = i++;
				vector<CObj<CCombinedPart> > &nodes = tek->second;
				bool bEmpty = true;
				for ( int k = 0; k < nodes.size(); ++k )
				{
					if ( nodes[k]->GetCombiner()->GetSize() > 0 )
					{
						bEmpty = false;
						break;
					}
				}
				if ( bEmpty )
					data.erase( tek );
			}
			EraseInvalidRefs( &elements );
		}
		bool IsEmpty() const { return elements.empty(); }
		int operator&( CStructureSaver &f )
		{
			f.Add( 1, &data );
			f.Add( 2, &elements );
			return 0;
		}
	};
	//
	ZDATA_(CParent)
	SPerMaterialHolder staticParts, dynamicParts;
	list<CPtr<CParticles> > particles;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CParent*)this); f.Add(2,&staticParts); f.Add(3,&dynamicParts); f.Add(4,&particles); return 0; }

	CVolumeNode* SelectNode( CFuncBase<SFBTransform> *pTransform, const SBound &bound )
	{
		const SFBTransform &pos = pTransform->GetValue();
		CVec3 ptCenter;
		pos.forward.RotateHVector( &ptCenter, bound.s.ptCenter );
		float fR = sqrt( CalcRadius2( bound, pos.forward ) );
		return GetNode( ptCenter, fR );
	}
	virtual bool IsEmpty();
private:	
	typedef COcTreeNode<CVolumeNode, N_MINIMAL_OCTREE_NODE> CParent;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPolyline: public CObjectBase
{
	OBJECT_BASIC_METHODS(CPolyline);
public:
	CDGPtr< CPtrFuncBase<NGfx::CGeometry> > pGeometry;
	CVec3 color;
	//
	void Render( NGfx::CRenderContext *pRC );
	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRayInfo
{
	CVec3 vOrigin, vDir, vDirOrt;
	float fLength;

	SRayInfo( const CRay &r ) 
		: vOrigin( r.ptOrigin ), vDir( r.ptDir ), vDirOrt( r.ptDir ), fLength( fabs( r.ptDir ) )
	{
		Normalize( &vDirOrt );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGScene;
class CDynamicLightCache
{
	typedef hash_map<CVec3, SDynamicAmbientInfo, SVec3Hash> CHash;
	ZDATA
	CDGPtr<CVersioningBase> pStaticChanged;
	CHash data;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pStaticChanged); f.Add(3,&data); return 0; }
	CDynamicLightCache() {}
	CDynamicLightCache( CVersioningBase *p ) : pStaticChanged(p) {}
	const SDynamicAmbientInfo& Calc( const SBound &bv, CGScene *pScene );
	bool CheckStatic();
	void Reset() { data.clear(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFakeParticleLMTexture : public CPtrFuncBase<NGfx::CTexture>
{
	OBJECT_NOCOPY_METHODS(CFakeParticleLMTexture);
	ZDATA
	CVec3 vAmbient;
	CDGPtr<CFuncBase<CVec3> > pColor;
	DWORD dwNormalColor, dwParticleColor;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&vAmbient); f.Add(3,&pColor); f.Add(4,&dwNormalColor); f.Add(5,&dwParticleColor); return 0; }
	bool NeedUpdate() { return pColor.Refresh(); }
	void Recalc();
public:
	void SetAmbient( const CVec3 &_v ) { vAmbient = _v; Updated(); }
	void SetColor( CFuncBase<CVec3> *_pColor ) { pColor = _pColor; }
	DWORD GetNormalColor() const { return dwNormalColor; }
	DWORD GetParticleColor() const { return dwParticleColor; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ELMCalcType
{
	LM_CALC_NONE,
	LM_CALC_ALL,
	LM_CALC_COLOR,
	LM_CALC_SKY,
	LM_CALC_CLEAR
};
class CGScene: public IGScene
{
OBJECT_BASIC_METHODS(CGScene);
	enum ERLRequest
	{
		RN_STATIC = 1,
		RN_DYNAMIC = 2,
		RN_ALL = 3,
		RN_LIGHTMAPS = 4,
		RN_DEPTH = 8,
		RN_LIT_PARTICLES = 16,
		RN_OCCLUDERS = 32
	};

	ZDATA
	CDGPtr<CVersioningBase> pStaticTracker;
	CObj<CVolumeNode> pVolume;
	list< CPtr<CDynamicGeometryPart> > dynamicFrags;
	list< CPtr<CAnimatedPart> > animatedParts;
	list< CPtr<CDynamicPart> > movingParts;
	list< CPtr<ILight> > lights;
	list< CPtr<CPolyline> > lines;
	list< CPtr<CSelection> > selections;
	list< CPtr<CParticles> > particles;
	ESceneRenderMode renderMode;
	CObj<CCVec3> pCamera;
	CDGPtr<CVersioningBase> pIgnoreStaticTrack;
	SHMatrix mHoldTransform;
	int nCurrentIgnoreMark;
	int nIgnoreListWasCalced;
	SGroupSelect holdMask;
	CObj<CCVec3> pAmbient;
	CObj<CLightmapTracker> pLMTracker;
	CLightState currentLightState;
	vector<int> freeLightGroups;
	vector<CPtr<CLightGroup> > lightGroups;
	CDynamicLightCache dynamicLightCache;
	CObj<IMaterial> pTransparentMaterial;
	CDGPtr<CFakeParticleLMTexture> pFakeParticleLM;
	bool bLightmapsEnable;
	bool bLightStateCalced;
	int nFrameCounter;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pStaticTracker); f.Add(3,&pVolume); f.Add(4,&dynamicFrags); f.Add(5,&animatedParts); f.Add(6,&movingParts); f.Add(7,&lights); f.Add(8,&lines); f.Add(9,&selections); f.Add(10,&particles); f.Add(11,&renderMode); f.Add(12,&pCamera); f.Add(13,&pIgnoreStaticTrack); f.Add(14,&mHoldTransform); f.Add(15,&nCurrentIgnoreMark); f.Add(16,&nIgnoreListWasCalced); f.Add(17,&holdMask); f.Add(18,&pAmbient); f.Add(19,&pLMTracker); f.Add(20,&currentLightState); f.Add(21,&freeLightGroups); f.Add(22,&lightGroups); f.Add(23,&dynamicLightCache); f.Add(24,&pTransparentMaterial); f.Add(25,&pFakeParticleLM); f.Add(26,&bLightmapsEnable); f.Add(27,&bLightStateCalced); f.Add(28,&nFrameCounter); return 0; }
	int nSlowVolumeWalk;
	int nWantFirstLM;
	SParticleLMRenderTargetInfo particleLM;

	struct SDynamicLightGroup
	{
		int nGroup;
		SBound bv;
		SDynamicAmbientInfo *pAmbient;

		SDynamicLightGroup() {}
		SDynamicLightGroup( SDynamicAmbientInfo *_pAmbient, const SBound &_bv, int _nGroup ) 
			: pAmbient(_pAmbient), bv(_bv), nGroup(_nGroup) {}
	};
	struct SSceneFragmentGroupInfo
	{
		CSceneFragments *pList;
		CSceneFragments *pNewLMList;
		CTransparentRenderer *pTransp;
		CLightmapTracker *pLMTracker;
		vector<SDynamicLightGroup> groups;
		bool bHasNewLightmaps;
		int nCurrentFrame;
		CVec3 vCamera;

		SSceneFragmentGroupInfo( const CVec3 &_vCamera, CSceneFragments *_pList, CTransparentRenderer *_pTransp ) 
			: pList(_pList), pTransp(_pTransp), pLMTracker(0), bHasNewLightmaps(false), nCurrentFrame(0), pNewLMList(0), vCamera(_vCamera) {}
		SSceneFragmentGroupInfo( const CVec3 &_vCamera, CSceneFragments *_pList, CTransparentRenderer *_pTransp, 
			CLightmapTracker *_pLMTracker, int _nFrame, CSceneFragments *_pNewLMList ) 
			: pList(_pList), pTransp(_pTransp), pLMTracker(_pLMTracker), bHasNewLightmaps(false), nCurrentFrame(_nFrame), pNewLMList(_pNewLMList), vCamera(_vCamera) {}
		void AddElement( CTransformStack *pTS, CCombinedPart *p, ERLRequest req, int _nIgnoreMark );
		void CalcLightmaps( CGScene *pScene );
	};

	void AddLight( ILight *pGroup );
	void AddNodeParts( CTransformStack *pTS, list<SRenderPartSet> *pRes, CVolumeNode *pNode, ERLRequest eReq, const SGroupSelect &mask );
	void MakePartList( CTransformStack *pTS, list<SRenderPartSet> *pRes, ERLRequest req, const SGroupSelect &mask );
	void AddNode( CTransformStack *pTS, SSceneFragmentGroupInfo *pFragments, CVolumeNode *pNode, ERLRequest req, const SGroupSelect &mask, int nIgnoreMark );
	void MakeRenderList( CTransformStack *pTS, SSceneFragmentGroupInfo *pTarget, ERLRequest req, const SGroupSelect &mask, int nIgnoreMark );
	//void MakeTransparentList( CTransparentRenderer *pRes, CTransformStack *pTS, const SGroupSelect &mask );
	void RecalcRenderStats( int nSceneTris, int nParticles, int nLitParticles );
	void RecalcCullingInfo();
	void UpdateIgnoreMark( IRender *pRender, CTransformStack *pTS, const SGroupSelect &mask );
	bool TraceStatic( CVolumeNode *pNode, const SRayInfo &r, float *pfT, CVec3 *pNormal, CVec3 *pColor );
	void ResetDynamicLightmapsCache();
	void CalcNewLightState();
	void UpdateLightmaps( CTransformStack *pTS, CSceneFragments *pScene, CSceneFragments *pNewLMScene, bool bHasNewLightmaps );
	void CheckDynamicLightmapCache();
	void DrawSelection( CTransformStack *pTS, NGfx::CRenderContext *pRC );
	void DrawLines( NGfx::CRenderContext *pRC );
	void RefreshParticleLMTarget();
	CLightmapTracker* GetLMTracker();
public:
	//
	CGScene();
	// outer space integration
	virtual CLightGroup* CreateLightGroup();
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		const SGroupInfo &_ginfo );
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		const SFBTransform &trans, const SGroupInfo &_ginfo );
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		const SDiscretePos &trans, const SGroupInfo &_ginfo );
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		CFuncBase<SFBTransform> *pPlacement, const SGroupInfo &_ginfo );
	virtual CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		CFuncBase<vector<SHMatrix> > *pPlacement, const SGroupInfo &_ginfo );
	virtual CObjectBase* CreateDynamicGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, 
		CFuncBase<SBound> *pBound, const SGroupInfo &_ginfo );
	//
	virtual CObjectBase* CreateParticles( CPtrFuncBase<CParticleEffect> *pInfo,
		CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SGroupInfo &_ginfo, bool bIsLit = false );
	virtual CObjectBase* CreateLitParticles( CPtrFuncBase<CParticleEffect> *pInfo, 
		IMaterial *pMat, CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SGroupInfo &_ginfo );
	virtual CPolyline* CreatePolyline( CPtrFuncBase<NGfx::CGeometry> *pGeometry, const CVec3 &color );
	virtual CObjectBase* CreateSelection( CObjectBase *pRenderNode, const CVec4 &vColor );
	virtual void SetAmbient( const CVec3 &ambientColor );
	virtual ILight* AddDirectionalLight( CFuncBase<CVec3> *pColor, CFuncBase<CVec3> *pGlossColor, const CVec3 &vShadowColor, const CVec3 &ptLight, const CVec3 &ptOrigin, const CVec2 &ptSize, float fMaxHeight, bool bLightmapOnly );
	virtual ILight* AddPointLight( const CVec3 &_vColor, const CVec3 &ptOrigin, float fR, bool bLightmapOnly );
	virtual ILight* AddPointLight( CPtrFuncBase<CAnimLight> *pLight );
	virtual ILight* AddSpotLight( CFuncBase<CVec3> *pColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, float fRadius, CPtrFuncBase<NGfx::CTexture> *pMask, bool bLightmapOnly );
	virtual void Draw( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, const SGroupSelect &mask, EFogMode fogMode, const SFogParams &fog, EHSRMode hsrMode );
	virtual CVec2 GetScreenRect();
	virtual ESceneRenderMode GetRenderMode();
	virtual void SetRenderMode( ESceneRenderMode mode );
	virtual CFuncBase<CVec3>* GetCamera();
	virtual bool TraceScene( const CRay &r, float *pfT, CVec3 *pNormal, CVec3 *pColor );
	void TraceDynamicAmbient( SDynamicAmbientInfo *pRes, const SBound &bv );
	const SDynamicAmbientInfo& GetDynamicAmbient( int nGroup, const SBound &bv );
	void FreeLightGroup( int n ) { lightGroups[n] = 0; freeLightGroups.push_back( n ); }
	virtual void SetLightmaps( bool bEnable );
	friend class CRenderWrapper;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRenderWrapper: public IRender
{
	CGScene *pScene;
public:
	CRenderWrapper( CGScene *_pScene ): pScene(_pScene) {}

	virtual void FormPartList( CTransformStack *pTS, list<SRenderPartSet> *pRes, EDepthType dt );
	virtual void FormPartList( CTransformStack *pTS, list<SRenderPartSet> *pRes, const SGroupSelect &mask );
	virtual void FormDepthList( CTransformStack *pTS, CTransformStack *pParticleTS, CSceneFragments *pRes, EDepthType dt );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
