#include "StdAfx.h"
#include "GfxEffects.h"
#include "GfxUtils.h"
#include "Transform.h"
/////////////////////////////////////////////////////////////////////////////////////
using namespace NGfx;
/////////////////////////////////////////////////////////////////////////////////////
void ShowTexture( NGfx::CTexture *pTex, float fpMag )
{
	CTransformStack ts;
	SHMatrix pos;
	Identity( &pos );
	ts.MakeParallel( 800, 600 );
	NGfx::SEffTexture tex;
	tex.pTex = pTex;
	tex.SetTransform( ts.Get() );
	tex.AddTempGeometry();
	//
	CGeomAccess<SGeomVecT1> &geom = NGfx::MakeTempGeometry( 4, SGeomVecT1() );
	CTriListAccess &tris = NGfx::MakeTempTriList( 2 );
	geom[0].pos = CVec3( -0.5f * fpMag, -0.5f * fpMag, 2 );
	geom[0].tex = CVec2(0,0);
	geom[1].pos = CVec3( pTex->GetXSize() * fpMag - 0.5f * fpMag, -0.5f * fpMag, 2 );
	geom[1].tex = CVec2(1,0);
	geom[2].pos = CVec3( pTex->GetXSize() * fpMag - 0.5f * fpMag, pTex->GetYSize() * fpMag - 0.5f * fpMag, 2 );
	geom[2].tex = CVec2(1,1);
	geom[3].pos = CVec3( -0.5f * fpMag, pTex->GetYSize() * fpMag - 0.5f * fpMag, 2 );
	geom[3].tex = CVec2(0,1);
	//
	tris[0] = S3DTriangle( 0, 1, 2 );
	tris[1] = S3DTriangle( 0, 2, 3 );
	//
	tex.Use();
	NGfx::DrawPrimitive();
}
/////////////////////////////////////////////////////////////////////////////////////
