#ifndef __GSCENEINTERNAL_H_
#define __GSCENEINTERNAL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
/////////////////////////////////////////////////////////////////////////////////////
// hierarchy forming node
class CMSRNode: public CFuncBase<SFBTransform>
{
	OBJECT_BASIC_METHODS(CMSRNode);
protected:
	virtual void Update();
public:
	CDGPtr< CFuncBase<SFBTransform> > pAncestor;
	CDGPtr< CFuncBase<SHMatrix> > pPos;
	//
	CMSRNode() {}
	void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
// geometry loader from disk
class CCubeGeometry: public CPtrFuncBase<NGfx::CGeometry>
{
	OBJECT_BASIC_METHODS(CCubeGeometry);
protected:
	virtual void Update();
};
/////////////////////////////////////////////////////////////////////////////////////
class CCubeTriList: public CPtrFuncBase<NGfx::CTriList>
{
	OBJECT_BASIC_METHODS(CCubeTriList);
protected:
	virtual void Update();
};
/////////////////////////////////////////////////////////////////////////////////////
// node that can render something somewhere
class CRenderNode: public CBaseNode
{
	OBJECT_BASIC_METHODS(CRenderNode);
protected:
	virtual void Update();
public:
	CDGPtr< CFuncBase<SFBTransform> > pPlacement;
	CDGPtr< CPtrFuncBase<NGfx::CGeometry> > pGeometry;
	CDGPtr< CPtrFuncBase<NGfx::CTriList> > pTriList;
	//
	void DrawGeometry( NGfx::SEffect *pEffect ); // draw geometry with pEffect
	void DrawDiffuse(); // draw diffuse multiplier for node
	void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
class CLightBase: public CBaseNode
{
public:
	virtual void Draw() = 0;
};
/////////////////////////////////////////////////////////////////////////////////////
class CShadowedLight: public CLightBase
{
	OBJECT_BASIC_METHODS(CShadowedLight);
	CVec4 vecU, vecV;
	CVec4 vecDepth, vecDepthP;
	CVec4 vecLightDir;
	CTransformStack ts;
	CObj<NGfx::CTexture> pTexture;
	CObj<NGfx::CRenderTarget> pTarget;
protected:
	virtual void Update();
public:
	void SetupLight( const CVec3 &ptCenter, const CVec3 &ptLight, float fpHalfSize, float fpMaxHeight );
	virtual void Draw();
	virtual void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
class CHilight: public CLightBase
{
	OBJECT_BASIC_METHODS(CHilight);
	CObj<CRenderNode> pDest;
	CVec4 color;
public:
	void SetupLight( CRenderNode *_pDest, const CVec3 &ptColor );
	virtual void Draw();
	virtual void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
/*
class CBoundVolume: public CBaseNode
{
public:
	CVec3 ptCenter;
	float fpRadius;
	CDGPtr< CFuncBase<SHMatrix> > pBind;
	//
	vector< CPtr<CRenderNode> > objects;
};
/////////////////////////////////////////////////////////////////////////////////////
class CVolumeReqs
{
	list< CPtr<CBoundVolume> > objects;
};*/
/////////////////////////////////////////////////////////////////////////////////////
class CGScene: public CGSceneBase
{
FUNDAMENT_BASIC_METHODS(CGScene);
	CObj<CCFBTransform> pTransformRoot;
public:
	//CVolumeReqs v;
	//typedef list< CPtr<CBaseNode> > CForceSet;
	//CForceSet forceRecalc;
	list< CPtr<CRenderNode> > visible;
	list< CPtr<CLightBase> > lights;
	//
	CGScene() {	pTransformRoot = new CCFBTransform; }
	void SetRootTransform( const SFBTransform &t ) { pTransformRoot->Set( t ); UseUpdated(); }
	const SFBTransform& GetRootTransform() const { return pTransformRoot->GetValue(); }
	// some means to request from VS
	// outer space integration
	//
	virtual CRenderNode* CreateModel( int nModelID, CFuncBase<SHMatrix> *pPlacement ); // *pAnimation );
	virtual CBaseNode* AddHilight( CRenderNode *pDst, const CVec3 &ptColor );
	virtual void AddLight( const CVec3 &ptCenter, const CVec3 &ptLight, float fpHalfSize, float fpMaxHeight );
	virtual void Draw( CTransformStack *pTS );
	virtual void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
} // namespace
/////////////////////////////////////////////////////////////////////////////////////
#endif