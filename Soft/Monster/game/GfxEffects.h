#ifndef __GDXEFFECTS_H_
#define __GDXEFFECTS_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "Gfx.h"
namespace NGfx
{
/////////////////////////////////////////////////////////////////////////////////////
// EFFECTS
/////////////////////////////////////////////////////////////////////////////////////
struct SEffect
{
private:
	enum { N_MAXVB_COUNT = 1 };
	//
	int nVB;
	CGeometry *vbs[N_MAXVB_COUNT];
	int nVBFormat[N_MAXVB_COUNT];
protected:
	SHMatrix transform;
	//
	void ApplyVSConst( const void *pData, int nCount ) const;
	void ApplyGeometry() const;
	void ApplyTexture( int nStage, CTexture *pTex ) const;
protected:
	SEffect() { nVB = 0; }
	SEffect( const SEffect &a ) { ASSERT(0); }
public:
	void ResetGeometry() { nVB = 0; }
	void AddTempGeometry() { ASSERT( nVB < N_MAXVB_COUNT ); vbs[nVB] = 0; nVBFormat[nVB] = 0; nVB++; }
	void AddGeometry( CGeometry *pGeom ) { ASSERT( nVB < N_MAXVB_COUNT ); vbs[nVB] = pGeom; nVBFormat[nVB] = pGeom->GetFormatID(); nVB++; }
	void AddGeometry( CGeometry *pGeom, int nFormatID ) { ASSERT( nVB < N_MAXVB_COUNT ); vbs[nVB] = pGeom; nVBFormat[nVB] = nFormatID; nVB++; }
	//
	virtual void SetTransform( const SFBTransform &_transform );
	virtual void Use() const = 0;
	//
};
/////////////////////////////////////////////////////////////////////////////////////
struct SEffDiffuse: public SEffect
{
	virtual void Use() const;
};
/////////////////////////////////////////////////////////////////////////////////////
struct SEffTexture: public SEffect
{
	CTexture *pTex;
	//
	virtual void Use() const;
};
/////////////////////////////////////////////////////////////////////////////////////
struct SEffDepth: public SEffect
{
	CVec4 vecDepth;
	//
	virtual void Use() const;
};
/////////////////////////////////////////////////////////////////////////////////////
struct SEffShadowedLight: public SEffect
{
	CVec4 vecDepth, vecU, vecV, vecLightDir, color;
	CVec4 vecLightDirP;
	CTexture *pDepth;
	//
	virtual void SetTransform( const SFBTransform &_transform );
	virtual void Use() const;
};
/////////////////////////////////////////////////////////////////////////////////////
struct SEffConstLight: public SEffect
{
	CVec4 color;
	//
	virtual void Use() const;
};
/////////////////////////////////////////////////////////////////////////////////////
} // namespace
/////////////////////////////////////////////////////////////////////////////////////
#endif