#ifndef __COMBINER_H_
#define __COMBINER_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
#include "GPixelFormat.h"
#include "GRenderCore.h"

namespace NGfx
{
	class CGeometry;
}
struct SDiscretePos;
struct SBoundCalcer;
namespace NGScene
{
class CObjectInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ETransformType
{
	TT_NONE,
	TT_SIMPLE,
	TT_SIMPLE_DISCRETE,
	TT_SINGLE_SKIN
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ELightmapType
{
	LT_TNL,
	LT_TNL_SELECTION,
	LT_NONE,
	LT_NORMAL,
	LT_POSITION
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ECombinerType
{
	CT_STATIC,
	CT_DYNAMIC
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPerMaterialCombiner;
class IPart: public CObjectBase
{
	ZDATA
	CPtr<CPerMaterialCombiner> pCombiner;
	CDGPtr< CPtrFuncBase<CObjectInfo> > pObjInfo;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCombiner); f.Add(3,&pObjInfo); return 0; }
	IPart() {}
	IPart( CPtrFuncBase<CObjectInfo> *pData, CPerMaterialCombiner *_pCombiner );
	~IPart();
	void SetCombiner( CPerMaterialCombiner *_pCombiner, bool bForceUpdate, bool bAnimated );
	bool RefreshObjectInfo() { return pObjInfo.Refresh(); }
	CObjectInfo* GetObjectInfo() { return pObjInfo->GetValue(); }
	virtual ELightmapType GetLightmapType() const { return LT_NONE; }
	virtual ETransformType GetTransformType() const { return TT_NONE; }
	virtual const SDiscretePos& GetDiscretePos() { return *(SDiscretePos*)0; }
	virtual const SFBTransform& GetSimplePos() { return *(SFBTransform*)0; }
	virtual const vector<SHMatrix>& GetAnimation() { return *(vector<SHMatrix>*)0; }
	virtual const CTRect<int>& GetLMRegion() const { return *(CTRect<int>*)0; }
	virtual int GetLMLOD() const { return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void TransformPart( IPart *p, vector<CVec3> *pRes, vector<STriangle> *pTris );
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPerMaterialCombiner
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPerMaterialCombiner: public CFuncBase< vector< CPtr<IPart> > >
{
	OBJECT_NOCOPY_METHODS(CPerMaterialCombiner);
	CPtr<CVersioningBase> pTracker;
	CObj<CVersioningBase> pAnimation;
public:
	CPerMaterialCombiner() {}
	CPerMaterialCombiner( CVersioningBase *_pTracker );
	~CPerMaterialCombiner();
	// for use with IPart like objects only
	void AddPart( IPart *pPart );
	void RemovePart( IPart *pPart );
	void MarkWasted();
	void Animated() { if ( pAnimation ) pAnimation->Updated(); }
	CVersioningBase* GetAnimation() const { return pAnimation; }
	int GetSize() const { return value.size(); }
	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAutomaticCombiner
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAutomaticCombiner : public CFuncBase< vector< CPtr<IPart> > >
{
	OBJECT_NOCOPY_METHODS(CAutomaticCombiner);
protected:
	virtual bool NeedUpdate();
	virtual void Recalc() {}
public:
	void AddPart( IPart *pPart ) { 	ASSERT( value.size() < PF_MAX_PARTS_PER_COMBINER ); value.push_back( pPart ); Updated(); }
	int operator&( CStructureSaver &f ) { f.Add( 1, &value ); return 0; }
	friend class IPart;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
/*class CVBPositionCombiner : public CFuncBase<vector<vector<CVec3> > >
{
	OBJECT_NOCOPY_METHODS(CVBPositionCombiner);
	ZDATA
	CDGPtr< CFuncBase< vector< CPtr<IPart> > > > pCombiner;
	CDGPtr<CVersioningBase> pAnimation;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCombiner); f.Add(3,&pAnimation); return 0; }
protected:
	virtual bool NeedUpdate() { if ( pAnimation ) return pAnimation.Refresh() | pCombiner.Refresh(); return pCombiner.Refresh(); }
	virtual void Recalc();
public:
	CVBPositionCombiner() {}
	CVBPositionCombiner( CFuncBase< vector< CPtr<IPart> > > *_pCombiner, CVersioningBase *_pAnimation )
		: pCombiner(_pCombiner), pAnimation(_pAnimation) {}
};*/
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVBCombiner
////////////////////////////////////////////////////////////////////////////////////////////////////
class CVBCombiner: public IVBCombiner
{
	OBJECT_BASIC_METHODS(CVBCombiner);
	ZDATA_(IVBCombiner)
	CDGPtr< CFuncBase< vector< CPtr<IPart> > > > pCombiner;
	ELightmapType lt;
	ECombinerType ct;
	CDGPtr<CVersioningBase> pAnimation;
	float fOffset;
	CDGPtr<CVersioningBase> pLMMapping;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(IVBCombiner*)this); f.Add(2,&pCombiner); f.Add(3,&lt); f.Add(4,&ct); f.Add(5,&pAnimation); f.Add(6,&fOffset); f.Add(7,&pLMMapping); return 0; }
	vector<vector<CVec3> > xformedPositions;
	bool bNeedXForm, bNeedRecalc;
	template<class TTrans>
		void SimpleTransform( TTrans *p );
protected:
	void XFormPosition();
	bool RealNeedUpdate() { if ( pAnimation ) return pAnimation.Refresh() | pCombiner.Refresh(); return pCombiner.Refresh(); }
	bool NeedXForm() { bool bRes = RealNeedUpdate(); bNeedXForm |= bRes; return bNeedXForm; }
	virtual bool NeedUpdate() { if ( pLMMapping ) return NeedXForm() | bNeedRecalc | pLMMapping.Refresh(); return NeedXForm() | bNeedRecalc; }
	virtual void Recalc();
public:
	CVBCombiner() {}
	CVBCombiner( CFuncBase< vector< CPtr<IPart> > > *_pCombiner, ELightmapType _lt, ECombinerType _ct, CVersioningBase *_pAnimation )
		: pCombiner(_pCombiner), lt(_lt), ct(_ct), pAnimation(_pAnimation), fOffset(0) {}
	void SetOffset( float _f ) { fOffset = _f; }
	void SetLMMappingTracker( CVersioningBase *pLMChange ) { pLMMapping = pLMChange; }
	virtual const SBound& GetBound() { if ( NeedXForm() ) XFormPosition(); return bound; }
	virtual const vector<SSphere>& GetBounds() { if ( NeedXForm() ) XFormPosition(); return partBVs; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CIBCombiner
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EIBTargetType
{
	IBTT_POSITIONS,
	IBTT_VERTICES,
	IBTT_LM,
	IBTT_LMCALC
};
class CIBCombiner : public CFuncBase<vector<NGfx::STriangleList> >
{
	OBJECT_BASIC_METHODS(CIBCombiner);
	ZDATA
	CDGPtr< CFuncBase< vector< CPtr<IPart> > > > pCombiner;
	ELightmapType lt;
	EIBTargetType ibt;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCombiner); f.Add(3,&lt); f.Add(4,&ibt); return 0; }
	vector<STriangle> triBuffer;
protected:
	virtual bool NeedUpdate() { return pCombiner.Refresh(); }
	virtual void Recalc();
public:
	CIBCombiner() {}
	CIBCombiner( CFuncBase< vector< CPtr<IPart> > > *_pCombiner, ELightmapType _lt, EIBTargetType _ibt )
		: pCombiner(_pCombiner), lt(_lt), ibt(_ibt) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif // __COMBINER_H_