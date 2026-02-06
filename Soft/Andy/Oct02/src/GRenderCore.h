#ifndef __GRenderCore_H_
#define __GRenderCore_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
#include "GRenderModes.h"
#include "Pool.h"
#include "GPixelFormat.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline T* SafeCast( CObjectBase *p, T *pMSVC6Suck = 0 )
{
#ifdef _DEBUG
	T *pRes = dynamic_cast<T*>( p );
	ASSERT( pRes );
	return pRes;
#else
	return (T*)p;
#endif
};
////////////////////////////////////////////////////////////////////////////////////////////////////
inline CVec3 MulPerComp2( const CVec3 &a, const CVec3 &b ) { return CVec3(a.x*b.x*2, a.y*b.y*2, a.z*b.z*2); }
inline CVec3 MulPerComp( const CVec3 &a, const CVec3 &b ) { return CVec3(a.x*b.x, a.y*b.y, a.z*b.z); }
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTransformStack;
namespace NGfx
{
	class CRenderContext;
	class CGeometry;
	class CTexture;
	class CCubeTexture;
	struct SEffect;
}
namespace NGScene
{
class IPart;
class CSceneFragments;
class IMaterial;
typedef unsigned int TPartFlags;
const TPartFlags PF_ALL_PARTS = 0xffffffff;
const int PF_MAX_PARTS_PER_COMBINER = 32;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRenderPartSet
{
	CPtr<CObjectBase> pNode;
	const vector< CPtr<IPart> > *pParts;
	TPartFlags nParts;

	SRenderPartSet() {}
	SRenderPartSet( CObjectBase *_pNode, const vector< CPtr<IPart> > *_pParts ): pNode(_pNode), pParts(_pParts), nParts(0) {}
	//SRenderPartSet( CObjectBase *_pNode ): pNode(_pNode) {}
	IPart* GetPart( int nIndex ) const { return (*pParts)[ nIndex ]; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGroupSelect;
class IRender
{
public:
	enum EDepthType
	{
		DT_STATIC,
		DT_DYNAMIC,
		DT_ALL,
		DT_FAST
	};
	
	virtual void FormPartList( CTransformStack *pTS, list<SRenderPartSet> *pRes, EDepthType dt ) = 0;
	virtual void FormPartList( CTransformStack *pTS, list<SRenderPartSet> *pRes, const SGroupSelect &mask ) = 0;
	virtual void FormDepthList( CTransformStack *pTS, CTransformStack *pParticleTS, CSceneFragments *pRes, EDepthType dt ) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// node that can render something somewhere
enum ETrilistType
{
	TLT_POSITION,
	TLT_GEOM,
	TLT_LM,
	TLT_LMCALC,
	TLT_NUMBER
};
class IVBCombiner : public CPtrFuncBase<NGfx::CGeometry>
{
protected:
	ZDATA
	SBound bound;
	vector<SSphere> partBVs;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bound); f.Add(3,&partBVs); return 0; }
	virtual const SBound& GetBound() { ASSERT( IsFrameMatch() ); return bound; }
	virtual const vector<SSphere>& GetBounds() { ASSERT( IsFrameMatch() ); return partBVs; }
	int GetPartsNum() const { return partBVs.size(); }
};
struct SRenderGeometryInfo
{
	CDGPtr<IVBCombiner> pVertices;
	CDGPtr< CFuncBase<vector<NGfx::STriangleList> > > pTriLists[TLT_NUMBER];

	int operator&( CStructureSaver &f ) 
	{ 
		f.Add(1,&pVertices); 
		for ( int k = 0; k < TLT_NUMBER; ++k )
			f.Add(2,&pTriLists[k], k + 1 );
		return 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//! intermediate per fragment operations representation
enum EStencilBlendingOp
{
	//	STM_TEST = 1,
	STM_NONE = 0,
	STM_LIGHT = 1,
	STM_STENCIL_LIGHT = 2,
	STM_TEST_STENCIL_LIGHT = 3,
	STM_MARK = 4,
	STM_TEST_CLEAR_MARK = 5,
	STM_MASK = 7,

	DPM_EQUAL    = 8,
	DPM_TESTONLY = 16,
	DPM_NONE     = 24,
	DPM_MASK     = 24,

	ABM_NONE     = 0,
	ABM_ZERO     = 32,
	ABM_ADD      = 64,
	ABM_MUL      = 96,
	ABM_SRC_AMUL = 128,
	ABM_ALPHA_BLEND = 160,
	ABM_ALPHA_FOG = 192,
	ABM_MUL2     = 224,
	ABM_MASK     = 224
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SDirectionalDepthInfo
{
	CVec4 vChannelSelect;
	CVec4 vDepth;
	CVec4 vVecU, vVecV;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SPerspDirectionalDepthInfo
{
	CVec4 vChannelSelect;
	CVec4 vDepth;
	SHMatrix m;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSkyDepth3Info
{
	SDirectionalDepthInfo *channels[3];
	CVec3 vDirs[3];
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ERenderOperation
{
	RO_NOP,
	//
	// TnL path
	RO_TNL_DIR_AMB_LIT_DIFFUSE_SOLID,
	RO_TNL_DIR_AMB_LIT_DIFFUSE_TEXTURE,
	RO_TNL_DIR_AMB_LIT_DIFFUSE_TEXTURE_AT,
	RO_TNL_DIR_AMB_LIT_DIFFUSE_TEXTURE_DECAL,
	// general ops
	RO_TEXTURE_AT,
	RO_DIFFUSE_TEXTURE,
	RO_DIFFUSE_TEXTURE_DECAL,
	RO_DIFFUSE_TEXTURE_AT,
	RO_LIGHTMAP,
	RO_SKYMAP,
	RO_SOLID_COLOR,
	// dynamic lightmaps
	RO_DYNAMIC_LIGHTMAP,
	// lightmapped stuff
	RO_LMPD_SOLID,
	RO_LMPD_TEXTURE,
	RO_LMPD_TEXTURE_AT,
	RO_LMPD_TEXTURE_DECAL,
	RO_DYN_LMPD_SOLID,
	RO_DYN_LMPD_TEXTURE,
	RO_DYN_LMPD_TEXTURE_AT,
	RO_DYN_LMPD_TEXTURE_DECAL,
	// directional
	RO_DIFFUSE_DIR_LIT_TEXTURE, // copies alpha
	RO_DIFFUSE_DIR_LIT_TEXTURE_PP,
	RO_DIFFUSE_BUMP_DIR_LIT_TEXTURE,
	RO_DIFFUSE_BUMP_DIR_LIT_TEXTURE_PP,
	RO_SPECULAR_DIR,
	RO_SPECULAR_DIR_TEXTURE,
	RO_SOLID,
	RO_DIR_LIT_SOLID,
	RO_DIR_LIT_SOLID_PP,
	RO_DIR_DEPTH,
	RO_DIR_DEPTH_MAX, // not used
	RO_DIR_SHADOW_TEST,
	RO_DIR_SHADOW_TEST_SMOOTHED,
	RO_TEXTURE_AT_CONSERVATIVE,
	RO_DIR_PARTICLE_LM_SHADOW_TEST,
	RO_DIR_PARTICLE_LM_SOFT_SHADOW_TEST,
	// ambient + directional
	RO_DIR_AMB_LIT_SOLID,
	RO_DIR_AMB_LIT_DIFFUSE_TEXTURE,
	RO_DIR_AMB_LIT_DIFFUSE_TEXTURE_AT,
	RO_DIR_AMB_LIT_DIFFUSE_TEXTURE_DECAL,
	// fast full light ops
	RO_DIFFUSE_FULL_LIT_TEXTURE_PP,
	RO_DIFFUSE_FULL_LIT_BUMP_TEXTURE_PP,
	RO_PP_SPECULAR_FULL_COLOR_DIR,
	RO_PP_SPECULAR_FULL_TEXTURE_DIR,
	RO_FULL_LIT_SOLID_PP,
	RO_DYNLM_LIT_SOLID_PP,
	RO_DIFFUSE_DYNLM_LIT_BUMP_TEXTURE_PP,
	RO_DIFFUSE_DYNLM_LIT_TEXTURE_PP,
	// point light
	RO_PNT_LIT_SOLID,
	RO_PNT_LIT_TEXTURE, // copies alpha
	RO_PNT_LIT_TEXTURE_PBUMP,
	RO_PNT_LIT_SOLID16,
	RO_PNT_LIT_TEXTURE16, // copies alpha
	RO_PNT_LIT_TEXTURE_PBUMP16,
	RO_PNT_SPECULAR_COLOR,
	RO_PNT_SPECULAR_TEXTURE,
	// fog
	RO_FOG_STATIC,
	RO_FOG_DYNAMIC,
	// mirrors
	RO_MIRROR,
	RO_GLOSSED_MIRROR,
	RO_BUMPED_MIRROR,
	RO_BUMPED_FRESNEL,
	RO_REG_MUL_TEXTURE,
	RO_TEXTURE,
	// per pixel specular
	RO_NHCALC,
	RO_NHCALC_BUMP,
	RO_PP_SPECULAR_COLOR_DIR,
	RO_PP_SPECULAR_TEXTURE_DIR,
	RO_PP_SPECULAR_COLOR_PNT,
	RO_PP_SPECULAR_TEXTURE_PNT,
	// lightmap calcs
	RO_LM_MODULATE,
	RO_LM_SKY_SQRT_MODULATE,
	RO_LM_SKY_DIR_CHECK,
	RO_LM_SKY_LIGHT,
	RO_LM_SKY_3LIGHT,
	RO_LM_DIR_LIT_SHADOWED,
	RO_LM_SPOT_DEPTH_CHECK,
	RO_LM_SPOT_LIGHT,
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_DYNAMIC_AMBIENT_INFO_COMPONENTS = 6;
struct SDynamicAmbientInfo
{
	struct SPad
	{
		CVec3 v;
		float f;
	};
	SPad vXPos, vXNeg, vYPos, vYNeg, vZPos, vZNeg; // CVec4 to make vs registers load easier

	CVec4* GetVec4() const { return (CVec4*)&vXPos; }
	void Clear() { SPad z; z.v = CVec3(0,0,0); vXPos = vXNeg = vYPos = vYNeg = vZPos = vZNeg = z; }
	void AddLight( const CVec3 &vColor, const CVec3 &vDir )
	{
		vXPos.v += vColor * Max( vDir.x, 0.0f );
		vXNeg.v += vColor * Max( -vDir.x, 0.0f );
		vYPos.v += vColor * Max( vDir.y, 0.0f );
		vYNeg.v += vColor * Max( -vDir.y, 0.0f );
		vZPos.v += vColor * Max( vDir.z, 0.0f );
		vZNeg.v += vColor * Max( -vDir.z, 0.0f );
	}
	void Blend( SDynamicAmbientInfo &a, float fSrc )
	{
		vXPos.v = fSrc * vXPos.v + ( 1 - fSrc ) * a.vXPos.v;
		vXNeg.v = fSrc * vXNeg.v + ( 1 - fSrc ) * a.vXNeg.v;
		vYPos.v = fSrc * vYPos.v + ( 1 - fSrc ) * a.vYPos.v;
		vYNeg.v = fSrc * vYNeg.v + ( 1 - fSrc ) * a.vYNeg.v;
		vZPos.v = fSrc * vZPos.v + ( 1 - fSrc ) * a.vZPos.v;
		vZNeg.v = fSrc * vZNeg.v + ( 1 - fSrc ) * a.vZNeg.v;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRenderStaticInfo
{
	SRenderGeometryInfo *pGeometry;
	CPtr<IMaterial> pMaterial;
	CPtr<CObjectBase> pHandle;
	CPtr<NGfx::CTexture> pLightmap;
	const SDynamicAmbientInfo *pLM;
	bool bSkipPerPartTests;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRenderFragmentInfo
{
	SRenderStaticInfo *pStatic;
	TPartFlags nParts;
	// some user settable data for commands generation
	bool bSkipPerPartTests;

	SRenderFragmentInfo() {}
//	SRenderFragmentInfo( SRenderGeometryInfo *_pElement, const SBound &_bound, bool _bLightmapped )
//		: pElement(_pElement) {}//, bound(_bound), bLightmapped(_bLightmapped) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSelectFragments;
enum EFragmentsSplit
{
	FST_ACCEPT,
	FST_REJECT,
	FST_SPLIT
};
const int N_MAX_SCENE_FILTER_DEPTH = 2;
class CSceneFragments
{
private:
	int nSceneTris;
	CPool<SRenderStaticInfo,128> staticInfos;
	CPool<SDynamicAmbientInfo,128> dynamicLMinfos;
	CPool<SRenderFragmentInfo,128> fragments;
	CPool<SRenderGeometryInfo,128> geometryInfos;
	vector<SRenderFragmentInfo*> selected, *pRejected;
	vector<SRenderFragmentInfo*> litParticles;
	vector<SRenderFragmentInfo*> *pTarget;
	struct SRejectInfo
	{
		vector<SRenderFragmentInfo*> rejected;
		int nLastRejectedFragment;
	};
	SRejectInfo rejected[ N_MAX_SCENE_FILTER_DEPTH ];
	int nRejectedUsed;

	template<class T>
		void SelectSet( SSelectFragments &selector, const T &select )
	{
		int nRes = 0;
		for ( int k = 0; k < pTarget->size(); ++k )
		{
			switch ( select( (*pTarget)[k], selector ) )
			{
				case FST_ACCEPT: (*pTarget)[nRes++] = (*pTarget)[k]; break;
				case FST_REJECT: pRejected->push_back( (*pTarget)[k] ); break;
				case FST_SPLIT: break;
				default: ASSERT(0); break;
			}
		}
		pTarget->resize( nRes );
	}
	template<class T>
		void Select( SSelectFragments &selector, const T &select )
	{
		ASSERT( nRejectedUsed < N_MAX_SCENE_FILTER_DEPTH );
		SRejectInfo &ri = rejected[ nRejectedUsed ];
		ri.rejected.resize(0);
		pRejected = &ri.rejected;
		ASSERT( pRejected->empty() );
		pTarget = &selected;
		SelectSet( selector, select );
		ri.nLastRejectedFragment = pRejected->size();
		pTarget = &litParticles;
		SelectSet( selector, select );
		pRejected = 0;
		++nRejectedUsed;
	}
	void Merge()
	{
		--nRejectedUsed;
		SRejectInfo &ri = rejected[nRejectedUsed];
		selected.insert( selected.end(), ri.rejected.begin(), ri.rejected.begin() + ri.nLastRejectedFragment );
		litParticles.insert( litParticles.end(), ri.rejected.begin() + ri.nLastRejectedFragment, ri.rejected.begin() + ri.rejected.size() );
	}
public:
	CSceneFragments() : nSceneTris(0), pRejected(0), nRejectedUsed(0) {}
	void AddElement( CObjectBase *pHandle, SRenderGeometryInfo *pGeometry, TPartFlags nParts, IMaterial *pMaterial, 
		NGfx::CTexture *pLightmap, const SDynamicAmbientInfo *pLM, bool bSkipPerPartTests = false );
	int CountTris( SRenderGeometryInfo *pGeometry, TPartFlags nParts );
	SDynamicAmbientInfo* AllocDynamicAmbient() { return dynamicLMinfos.Alloc(); }
	void AddLitParticles( IVBCombiner *pCombiner, CFuncBase<vector<NGfx::STriangleList> > *pTris, int nPart, IMaterial *pMaterial,
		bool bSkipPerPartTests );
	int GetSceneTris() const { return nSceneTris; }
	const vector<SRenderFragmentInfo*>& GetSelected() { return selected; }
	const vector<SRenderFragmentInfo*>& GetLitParticles() { return litParticles; }
	friend struct SSelectFragments;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSelectFragments
{
	CSceneFragments *pScene;
	template<class T>
		SSelectFragments( CSceneFragments *_pScene, const T &select ) : pScene(_pScene) { pScene->Select( *this, select ); }
	~SSelectFragments() { pScene->Merge(); }
	EFragmentsSplit Split( const SRenderFragmentInfo &original, TPartFlags accepted, TPartFlags rejected ) const 
	{
		if ( accepted == 0 )
			return FST_REJECT;
		if ( rejected == 0 )
			return FST_ACCEPT;
		SRenderFragmentInfo *pRes = pScene->fragments.Alloc();
		*pRes = original;
		pRes->nParts = accepted;
		pScene->pTarget->push_back( pRes ); 
		pRes = pScene->fragments.Alloc();
		*pRes = original;
		pRes->nParts = rejected;
		pScene->pRejected->push_back( pRes ); 
		return FST_SPLIT;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// bit set to 1 means part is ignored
typedef hash_map<CPtr<CObjectBase>,TPartFlags,SPtrHash> CFilterPartsHash;
struct SLightmappedFilter
{
	EFragmentsSplit operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const
	{
		return pF->pStatic->pLightmap ? FST_ACCEPT : FST_REJECT;
	}
};
struct SBoundIntersectFilter
{
	const SBound &bv;
	SBoundIntersectFilter( const SBound &_bv ) : bv(_bv) {}
	EFragmentsSplit operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const;
};
struct SFrustrumFilter
{
	CTransformStack *pTS;
	SFrustrumFilter( CTransformStack *_pTS ) : pTS(_pTS) {}
	EFragmentsSplit operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const;
};
struct SSphereFilter
{
	const SSphere &sph;
	SSphereFilter( const SSphere &_sph ) : sph(_sph) {}
	EFragmentsSplit operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const;
};
struct SSphereAndIgnoredFilter
{
	const SSphere &sph;
	const CFilterPartsHash &ignoreList;
	SSphereAndIgnoredFilter( const SSphere &_sph, const CFilterPartsHash &_ignoreList )
		: sph(_sph), ignoreList(_ignoreList) {}
	EFragmentsSplit operator()( SRenderFragmentInfo *pF, const SSelectFragments &selector ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRenderCmdList
{
public:
	union UParameter
	{
		const CVec3 *pVec3;
		const CVec4 *pVec4;
		const SFogParams *pFog;
		NGfx::CTexture *pTex;
		NGfx::CCubeTexture *pCubeTex;
		const SDirectionalDepthInfo *pDirDepth;
		const SPerspDirectionalDepthInfo *pPDirDepth;
		const SDynamicAmbientInfo *pDynamicAmbientInfo;
		const SSkyDepth3Info *pSkyDepth3;
		float f;

		UParameter() : f(0) {}
		UParameter( const CVec3 *v3 ): pVec3(v3) {}
		UParameter( const CVec4 *v4 ): pVec4(v4) {}
		UParameter( const SFogParams *_p ): pFog(_p) {}
		UParameter( NGfx::CTexture *_pTex ): pTex(_pTex) {}
		UParameter( NGfx::CCubeTexture *_pTex ): pCubeTex(_pTex) {}
		UParameter( const SDirectionalDepthInfo *_pDirDepth ): pDirDepth(_pDirDepth) {}
		UParameter( const SPerspDirectionalDepthInfo *_pDirDepth ): pPDirDepth(_pDirDepth) {}
		UParameter( const SDynamicAmbientInfo *_p ): pDynamicAmbientInfo(_p) {}
		UParameter( const SSkyDepth3Info *_p ): pSkyDepth3(_p) {}
		UParameter( float _f ): f(_f) {}
	};
	struct SOperation
	{
		SRenderFragmentInfo *pFrag;
		ERenderOperation op;
		unsigned char nPass;
		unsigned char nStencilBlendMode; // stencil mode & blend mode
		char nDestRegister;
		UParameter p1, p2, p3;

		SOperation() {}
		SOperation( SRenderFragmentInfo *_pFrag, ERenderOperation _op, unsigned char _nPass, 
			unsigned char _nSBM, unsigned char _nRegister, UParameter _p1 = UParameter(float(0)), UParameter _p2 = UParameter(float(0)), UParameter _p3 = UParameter(float(0)) )
			: pFrag(_pFrag), op(_op), nPass(_nPass), nStencilBlendMode(_nSBM), nDestRegister(_nRegister),
			p1(_p1), p2(_p2), p3(_p3) {}
		bool IsSame( const SOperation &a ) const
		{
			ASSERT( sizeof(p1.f) == sizeof(p1) );
			return op == a.op && nPass == a.nPass && nStencilBlendMode == a.nStencilBlendMode && 
				nDestRegister == a.nDestRegister && p1.f == a.p1.f && p2.f == a.p2.f && p3.f == a.p3.f;
		}
	};
	vector<SOperation> ops;

	CRenderCmdList() {}
	bool IsEmpty() const { return ops.empty(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SOpGenContext
{
	vector<CRenderCmdList::SOperation> *pRes;
	SRenderFragmentInfo *pCurFragment;
	bool bHasAddedOps;

	void AddOperation(
		ERenderOperation _op, unsigned char _nPass, int nSBM, char nDestRegister, 
		CRenderCmdList::UParameter p1 = CRenderCmdList::UParameter(), 
		CRenderCmdList::UParameter p2 = CRenderCmdList::UParameter(), 
		CRenderCmdList::UParameter p3 = CRenderCmdList::UParameter() )
	{
		ASSERT( pCurFragment );
		pRes->push_back( 
			CRenderCmdList::SOperation( pCurFragment, _op, _nPass, nSBM, nDestRegister, p1, p2, p3 )
			);
		bHasAddedOps = true;
	}
	SOpGenContext( vector<CRenderCmdList::SOperation> *_pRes, SRenderFragmentInfo *_pTarget )
		: pRes(_pRes), pCurFragment(_pTarget), bHasAddedOps(false) {}
	bool HasAddedOps() const { return bHasAddedOps; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void MakeSingleOp( CRenderCmdList *pRes, CSceneFragments &src, bool bTakeLitParticles, ERenderOperation op,
	CRenderCmdList::UParameter _p1 = CRenderCmdList::UParameter(),
	CRenderCmdList::UParameter _p2 = CRenderCmdList::UParameter(),
	CRenderCmdList::UParameter _p3 = CRenderCmdList::UParameter(),
	int nStencilOp = 0 );
void AddFinalOps( CRenderCmdList *pRes, CSceneFragments &src, ESceneRenderMode rm, 
	ERenderOperation op, CRenderCmdList::UParameter _p1 );
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMaterialInfo
{
	enum EMaterialType
	{
		NORMAL,
		DECAL
	};
	enum EType
	{
		T_NONE,
		T_COLOR,
		T_TEXTURE,
		T_VSPEC  // variable spec power
	};
	struct SColorInfo
	{
		ZDATA
		EType type;
		CVec3 color;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&type); f.Add(3,&color); return 0; }
		NGfx::CTexture *pTex;

		SColorInfo(): type(T_NONE) {}
	};
	ZDATA
	EMaterialType mt;
	bool bAlphaTest;
	SColorInfo diffuse, specular;
	float fSpecPower;
	float fTransp; // for TREE type
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&mt); f.Add(3,&bAlphaTest); f.Add(4,&diffuse); f.Add(5,&specular); f.Add(6,&fSpecPower); f.Add(7,&fTransp); return 0; }
	NGfx::CTexture *pBump;

	SMaterialInfo(): mt(NORMAL), bAlphaTest(false), pBump(0), fSpecPower(0), fTransp(0) {}
	bool IsDecal() const { return mt == DECAL; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class IMaterial: public CObjectBase
{
public:
	enum EMaterialType
	{
		MT_NORMAL,
		MT_TRANSPARENT,
		MT_OCCLUDER
	};
	virtual EMaterialType GetType() const { return MT_NORMAL; }
	virtual bool IsDecal() const { return false; }
	virtual void ExecTransparent( SRenderGeometryInfo *pElement, NGfx::CRenderContext *pRC, NGfx::CTexture *pFog ) { ASSERT( 0 ); }
	virtual bool DoesCastShadow() const = 0;
	virtual void AddOperations( SOpGenContext *p, ESceneRenderMode rm ) {}
	virtual void AddATOperations( SOpGenContext *p ) {}
	virtual const SMaterialInfo& GetMaterialInfo() { return *(SMaterialInfo*)0; }
	virtual CVec3 GetAverageColor() const { return CVec3(1,1,1); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SParticleLMRenderTargetInfo
{
	SHMatrix rootTransform;
	CObj<NGfx::CTexture> pParticleLMs;
	CVec2 vParticleLMSize, vKernelSize;

	SParticleLMRenderTargetInfo() : vParticleLMSize(0,0), vKernelSize(0,0) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class ILight: public CObjectBase
{
public:
	virtual int GetPriority() const = 0;
	virtual void Render( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, ESceneRenderMode renderMode, 
		IRender *pRender, CSceneFragments &scene, const SParticleLMRenderTargetInfo &particleLM ) = 0;
	virtual bool CheckCulling( CTransformStack *pTS ) { return true; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
