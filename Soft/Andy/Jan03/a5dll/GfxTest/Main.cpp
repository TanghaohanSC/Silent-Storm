#include "StdAfx.h"
#include "..\Main\Gfx.h"
#include "WinFrame.h"
#include "..\Input\Bind.h"
////#include "..\Main\GScene.h"
#include "..\Main\Transform.h"
#include "..\Main\GfxBuffers.h"
#include "..\Main\GfxEffects.h"
#include "..\Main\GfxRender.h"
#include "..\Main\GfxUtils.h"
#include "..\Main\GfxShaders.h"
#include <D3D9.h>
#include "..\Main\GfxInternal.h" // CRAP
#include "..\Misc\2DArray.h"
#include "..\MiscDll\Commands.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGfx
{
	externA5 bool bOutputFPS;
	externA5 NWin32Helper::com_ptr<IDirect3DDevice9> pDevice;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static CTransformStack ts;
static void InitTS()
{
	ts.MakeProjective( 1 );
	SHMatrix cameraPos;
	MakeMatrix( &cameraPos, CVec3( 0,0,0 ), CVec3(0,1,0) );
	ts.SetCamera( cameraPos );
}
typedef NGfx::SGeomVecFull SVertex;
//typedef NGfx::SGeomVecNT1 SVertex;
//typedef NGfx::SGeomVec SVertex;
static void InitQuads( 	CObj<NGfx::CGeometry> *ppGeom, CObj<NGfx::CTriList> *ppTris, int nQuads )
{
	static SVertex blah;
	memset( &blah, 0, sizeof(blah) );
	if ( !IsValid(*ppGeom) || !IsValid( *ppTris ) )
	{
		NGfx::CBufferLock<SVertex> g( ppGeom, nQuads * 4 + 4000, NGfx::STATIC );//DYNAMIC );//
		int q = 2;//8;//
		NGfx::CBufferLock<NGfx::S3DTriangle> t( ppTris, nQuads * q );
		for ( int k = 0; k < g.GetSize(); ++k )
		{
			blah.pos = CVec3( 5, 5, 5 );
			g[k] = blah;
			//g[k].tex = CVec2(0,0);
			//g[k].color.color = 0xffffffff;
		}
		/*for ( int i = 0; i < nQuads; ++i )
		{
			int nBase = i;
			t[i*2+0] = NGfx::S3DTriangle( nBase+0, nBase+64, nBase+1 );
			t[i*2+1] = NGfx::S3DTriangle( nBase+1, nBase+64, nBase+65 );
		}*/
		for ( int i = 0; i < nQuads; ++i )
		{
			//nBase = i * 4;
			//nBase = ( nBase + 8 * 4 ) % ( 41*4 );//nQuads * 4 );
			//nBase = ( nBase + 32 * 4 ) % ( nQuads * 4 ); // worst case, does not fit AGP cache, new page on each load
			//t[i*q+0] = NGfx::S3DTriangle( nBase+0, nBase+1114, nBase+3317 );
			//t[i*q+1] = NGfx::S3DTriangle( nBase+0, nBase+3317, nBase+2207 );
			int nBase = i % 20; //= ( (i * 3) % 9 ) + 3;
			if ( nBase & 1 )
				nBase = 20;
			nBase *= 6;
			
			t[i*q+0] = NGfx::S3DTriangle( nBase+0, nBase+1, nBase+2 );
			t[i*q+1] = NGfx::S3DTriangle( nBase+3, nBase+4, nBase+5 );
			for ( int m = 2; m < q; ++m )
				t[i*q+m] = NGfx::S3DTriangle( nBase+0, nBase+2, nBase+3 );
		}
	}
}
static void TestPerformance()
{
	InitTS();
	NGfx::bOutputFPS = true;
	//CObj<NGfx::CTexture> pRT = NGfx::MakeTexture( 1024, 1024, 1, NGfx::SPixel8888::ID, NGfx::TARGET );
	//CObj<NGfx::CZBuffer> pZB = NGfx::GetSharedZBuffer();
	// create fake triangle and render it in different ways
	const int N_BUFS = 1;
	const int N_QUADS_PER_BUF = 2500;
	const int N_POLYS_PER_FRAME = 1000000;
	static CObj<NGfx::CGeometry> pGeom[ N_BUFS ];
	static CObj<NGfx::CTriList> pTris[ N_BUFS ];
	for ( int k = 0; k < N_BUFS; ++k )
		InitQuads( &pGeom[k], &pTris[k], N_QUADS_PER_BUF );
	NGfx::CRenderContext ctx;
	ctx.ClearBuffers();
	ctx.SetTransform( ts.Get() );
	//ctx.SetLightParams( CVec3(0,0,0), CVec3(0,0,0), CVec3(0,0,1) );
	//NGfx::SEffConstLight l;
	//l.color = CVec3(1,1,0);
	//ctx.SetEffect( &l );
	NGfx::SEffPureGeometry pg;
	ctx.SetEffect( &pg );
	for ( int k = 0; k < N_POLYS_PER_FRAME / 2 / N_QUADS_PER_BUF / N_BUFS; ++k )
	{
		for ( int i = 0; i < N_BUFS; ++i )
			ctx.DrawPrimitive( pGeom[i], pTris[i] );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TestProjective()
{
	InitTS();
	NGfx::bOutputFPS = true;
	//CObj<NGfx::CTexture> pRT = NGfx::MakeTexture( 1024, 1024, 1, NGfx::SPixel8888::ID, NGfx::TARGET );
	//CObj<NGfx::CZBuffer> pZB = NGfx::GetSharedZBuffer();
	// create fake triangle and render it in different ways
	static CObj<NGfx::CGeometry> pGeom;
	static CObj<NGfx::CTriList> pTris;
	if ( !IsValid(pGeom) || !IsValid( pTris ) )
	{
		NGfx::CBufferLock<NGfx::SGeomVecFull> g( &pGeom, 3 );
		NGfx::CBufferLock<NGfx::S3DTriangle> t( &pTris, 2 );
		g[0].pos = CVec3( -2, 4, 2 );
		//g[0].tex = CVec2(0,0);
		//g[0].color.color = 0xffffffff;
		g[1].pos = CVec3( 2, 4, 2 );
		//g[1].tex = CVec2(0,1000);
		//g[1].tex = CVec2(0.01f,0);//CVec2(800,0);
		//g[1].color.color = 0xffffffff;
		g[2].pos = CVec3( 2, 4, -2 );
		//g[2].tex = CVec2(0.01f,0.01f);//CVec2(800,600);
		//g[2].color.color = 0xffffffff;
		t[0] = NGfx::S3DTriangle( 0, 1, 2 );
		t[1] = NGfx::S3DTriangle( 0, 2, 1 );
		/*NGfx::CBufferLock<NGfx::SGeomVecFull> g1( pGeom1 );
		g1[0].pos = CVec3( -1, 19, 1 );
		g1[1].pos = CVec3( 1, 19, 1 );
		g1[2].pos = CVec3( 1, 19, -1 );*/
	}
	NGfx::CRenderContext ctx;
	ctx.SetVirtualRT();
	ctx.ClearBuffers();
	ctx.SetRegister(1);
	ctx.Use();
	ctx.SetTransform( ts.Get() );
	NGfx::SEffConstLight l;
	l.color = CVec3(1,1,0);
	ctx.SetEffect( &l );
	ctx.DrawPrimitive( pGeom, pTris );

	ctx.SetRegister(0);
	//HRESULT hr = NGfx::pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED );
	//ASSERT( D3D_OK == hr );
	ctx.SetVertexShader( vsRegisterMap );
	//ctx.SetPixelShader( psProjectedTexture );
	ctx.SetPixelShader( psTextureCopyAlpha );
	ctx.SetTexture( 0, NGfx::GetRegisterTexture( 1 ) );
	ctx.DrawPrimitive( pGeom, pTris );

	NGfx::ShowTexture( NGfx::GetRegisterTexture(0), 1 );
	// render to first register
/*	{
		NGfx::CRenderContext ctx;
		ctx.SetRegister( 1 );
		//ctx.SetDepth( NGfx::DEPTH_NONE );
		ctx.SetAlphaCombine( NGfx::COMBINE_NONE );
		static DWORD dwStart = 0x101010;
		//dwStart += 0x202020;
		ctx.ClearBuffers();// dwStart );
		//D3DVIEWPORT8 vp;
		//Zero(vp);
		//vp.Height = 600;
		//vp.Width = 800;
		//vp.MaxZ = 1;
		//NGfx::pDevice->EndScene();
		//NGfx::pDevice->BeginScene();
		//NGfx::pDevice->SetViewport( &vp );

		ctx.SetTransform( ts.Get() );
		NGfx::SEffConstLight l;
		l.color = CVec3(1,1,0);
		ctx.SetEffect( &l );
		ctx.DrawPrimitive( pGeom, pTris );
		//ctx.SetRenderTarget(0);
		//ctx.DrawPrimitive( pGeom, pTris );
	}
	// combine to result
	{
		NGfx::CRenderContext ctx;
		//ctx.ClearBuffers( 0 );//x80808080 );
		ctx.SetTransform( ts.Get() );
		//ctx.SetDepth( NGfx::DEPTH_NONE );
		NGfx::SEffConstLight l;
		l.color = CVec3(0,0,1);
		ctx.SetEffect( &l );
		ctx.DrawPrimitive( pGeom, pTris );
		//
		//pDevice->SetTextureStageState( n, D3DTSS_MIPMAPLODBIAS, *(DWORD*)&fMipBias );
		//NGfx::pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_NONE );
		//NGfx::pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_NONE );
		//NGfx::pDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );//D3DTEXF_POINT );
		//NGfx::pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
		//NGfx::pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
		ctx.SetAlphaCombine( NGfx::COMBINE_ADD );
		//ctx.SetPixelShader( psTexture );//psProjectedTexture );
		//ctx.SetVertexShader( vsTexture );//vsRegisterMap );
		ctx.SetDepth( NGfx::DEPTH_EQUAL );
		ctx.SetPixelShader( psProjectedTexture );
		ctx.SetVertexShader( vsRegisterMap );
		ctx.SetVSConst( 14, ctx.GetRegisterMapScale() );
		ctx.SetTexture( 0, NGfx::GetRegisterTexture( 1 ) );
		ctx.DrawPrimitive( pGeom, pTris );
		//NGfx::SEffTexture tx;
		//tx.pTex = NGfx::GetRegisterTexture( 1 );
		//tx.SetGeometry( pGeom );
		//tx.Use( ctx );
		//NGfx::DrawPrimitive( pTris );
	}*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void DrawTriangle( NGfx::CRenderContext *pRC, int nX = 0 )
{
	static CObj<NGfx::CGeometry> pGeom;
	static CObj<NGfx::CTriList> pTris;
	if ( 1 )//!IsValid(pGeom) || !IsValid( pTris ) )
	{
		NGfx::CBufferLock<NGfx::SGeomVecFull> g( &pGeom, 4 );
		NGfx::CBufferLock<NGfx::S3DTriangle> t( &pTris, 2 );
		NGfx::SGeomVecFull v;
		v.pos = CVec3( -2.01f + nX * 0.001f, 3.9f, 2.07f );
		g[0] = v;
		v.pos = CVec3( 2.1f, 4, 2 );
		g[1] = v;
		v.pos = CVec3( 2, 4.3f, -2.05f );
		g[2] = v;
		v.pos = CVec3( -8, 16.3f, -8.05f );
		g[3] = v;
		//t[0] = NGfx::S3DTriangle( 0, 1, 2 );
		t[0] = NGfx::S3DTriangle( 0, 2, 1 );
		//t[2] = NGfx::S3DTriangle( 0, 2, 3 );
		t[1] = NGfx::S3DTriangle( 0, 3, 2 );
	}
/*	CObj<NGfx::CGeometry> pStupid[100];
	for ( int k = 0; k < 50 + (nX&31); ++k )
	{
		NGfx::CBufferLock<NGfx::SGeomVecNT1> g( &pStupid[k], 3 );
		NGfx::SGeomVecNT1 v;
		v.pos = CVec3(100,4,100);
		g[0] = v;
		g[1] = v;
		g[2] = v;
	}*/
	pRC->DrawPrimitive( pGeom, pTris );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TestTriangle()
{
	static int nX;
	nX = 105;//( nX + 1 ) & 2047;
	InitTS();
	NGfx::CRenderContext ctx;
	ctx.ClearBuffers();
	ctx.SetTransform( ts.Get() );
	NGfx::SEffConstLight l;
	l.color = CVec3(0,0,0);
	ctx.SetEffect( &l );
	DrawTriangle( &ctx, nX );
	ctx.SetDepth( NGfx::DEPTH_EQUAL );
	l.color = CVec3(1,1,0);
	ctx.SetEffect( &l );
	DrawTriangle( &ctx, nX );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_QUEUE_MAX = 50;
static int nFlyCheck;
static void TestQueue( int nFly )
{
	nFly = Clamp( nFly, 0, N_QUEUE_MAX - 1 );
	static CObj<NGfx::CGeometry> pGeom[N_QUEUE_MAX];
	static CObj<NGfx::CTriList> pTris;
	static NGfx::SGeomVecFull v[3];
	static int nInUse;
	if ( !IsValid(pTris) )
	{
		//NGfx::CBufferLock<NGfx::SGeomVecFull> g( &pGeom, 4 );
		NGfx::CBufferLock<NGfx::S3DTriangle> t( &pTris, 2, NGfx::STATIC );
		v[0].pos = CVec3( -2, 40, 2 );
		v[1].pos = CVec3( 2, 40, 2 );
		v[2].pos = CVec3( -2, 40, -2 );
		t[0] = NGfx::S3DTriangle( 0, 2, 1 );
		t[1] = NGfx::S3DTriangle( 0, 1, 2 );
		for ( int k = 0; k < N_QUEUE_MAX; ++k )
		{
			NGfx::CBufferLock<NGfx::SGeomVecFull> g( &pGeom[k], 3, NGfx::STATIC );
			g[0] = v[0];
			g[1] = v[1];
			g[2] = v[2];
		}
	}
	NGfx::CRenderContext ctx;
	ctx.ClearBuffers();
	InitTS();
	ctx.SetTransform( ts.Get() );
	NGfx::SEffConstLight l;
	l.color = CVec3(0,1,0);
	ctx.SetEffect( &l );
	ctx.DrawPrimitive( pGeom[ nInUse % N_QUEUE_MAX ], pTris );

	CDynamicCast<NGfx::ILinearBuffer> pClear( pGeom[ (nInUse + N_QUEUE_MAX - nFly) % N_QUEUE_MAX ] );
	NGfx::SGeomVecFull *p = (NGfx::SGeomVecFull*)pClear->Lock();
	memset( p, 0, sizeof(NGfx::SGeomVecFull) * 3 );
	pClear->Unlock();
	CDynamicCast<NGfx::ILinearBuffer> pFill( pGeom[ (nInUse + 1) % N_QUEUE_MAX ] );
	p = (NGfx::SGeomVecFull*)pFill->Lock();
	p[0] = v[0];
	p[1] = v[1];
	p[2] = v[2];
	pFill->Unlock();
	++nInUse;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//NGfx::SGeomVecFull gg[10240];
void TestVBFillSpeed()
{
	CObj<NGfx::CGeometry> pTest;
	NGfx::SGeomVecFull blah;
	//for(;;)
	{
		NGfx::CBufferLock<NGfx::SGeomVecFull> gg( &pTest, 10240, NGfx::STATIC );//DYNAMIC );//
		DWORD dwStart = GetTickCount();
		for ( int k = 0; k < 200; ++k )
		{
			for ( int i = 0; i < 10240; ++i )
				gg[i] = blah;
		}
		char szBuf[100];
		sprintf( szBuf, "%g MB/sec\n", 200 * 10240 * sizeof(NGfx::SGeomVecFull) * 1024.0f / ( GetTickCount() - dwStart ) / 1000000 );
		OutputDebugString( szBuf );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void FillTexture( NGfx::CTexture *pTex, int nLevel, NGfx::SPixel8888 pix )
{
	NGfx::CTextureLock<NGfx::SPixel8888> t( pTex, nLevel, NGfx::INPLACE );
	for ( int y = 0; y < t.GetYSize(); ++y )
		for ( int x = 0; x < t.GetXSize(); ++x )
			t[y][x] = pix;
}
void Test2DQuadRender()
{
	NGfx::SPixel8888 color;
	static NGfx::C2DQuadsRenderer quadRender;
	static CObj<NGfx::CTexture> pGreen, pRed;
	if ( !IsValid( pGreen ) )
	{
		NGfx::CRenderContext ctx;
		ctx.SetAlphaCombine( NGfx::COMBINE_ALPHA );
		quadRender.SetTarget( ctx, CVec2(640,480), NGfx::QRM_OVERWRITE );
		pGreen = NGfx::MakeTexture( 1024, 1024, 1, NGfx::SPixel8888::ID, NGfx::REGULAR, NGfx::CLAMP );
		pRed = NGfx::MakeTexture( 256, 256, 1, NGfx::SPixel8888::ID, NGfx::TEXTURE_2D, NGfx::CLAMP );
		color.color = 0xff00ff00;
		FillTexture( pGreen, 0, color );
		color.color = 0xffff0000;
		FillTexture( pRed, 0, color );
	}

	NGfx::CRenderContext rc;
	//rcClear.SetAlphaCombine( NGfx::COMBINE_ALPHA );
	rc.ClearBuffers();
	quadRender.SetTarget( rc, CVec2(640,480), NGfx::QRM_OVERWRITE );
	color.color = 0xffffffff;
	CTRect<float> rTarget( -100, -100, -100, -100 );
	CTRect<float> rSrc( 0, 0, 1, 1 );
	float fR = 0;
	rTarget = CTRect<float>( 0, 0, 640, 480 );
	quadRender.AddRect( rTarget, pGreen, rSrc, color );
	fR = 200;
	for ( int k = 0; k < 50; ++k )
	{
		fR = 200 + k * 4;
		rTarget = CTRect<float>( fR, 0, fR + 4, 200 );
		rSrc = CTRect<float>( k * 4, 0, k * 4 + 4, 100 );
		quadRender.AddRect( rTarget, pRed, rSrc, color );
	}
	quadRender.Flush();
	quadRender.AddRect( rTarget, 0, rSrc, color );
	quadRender.Flush();
	Sleep(1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void FillTexture( NGfx::CTexture *pTex, int nSize, int nLevel )
{
	NGfx::CTextureLock<NGfx::SPixel8888> t( pTex, nLevel, NGfx::WRITEONLY );
	for ( int y = 0; y < nSize; ++y )
		for ( int x = 0; x < nSize; ++x )
			t[y][x] = ((x+y)&1) ? NGfx::SPixel8888( 255,255,255,255 ) : NGfx::SPixel8888( 0, 0, 0, 0 );
			//t[y][x] = x >= (nSize/2) ? NGfx::SPixel8888( 255,255,255,255 ) : NGfx::SPixel8888( 0, 0, 0, 0 );
}
void DrawScene()
{
	InitTS();
	NGfx::bOutputFPS = true;
	// create fake triangle and render it in different ways
	static CObj<NGfx::CTexture> pRT;
	static CObj<NGfx::CGeometry> pGeom;
	static CObj<NGfx::CTriList> pTris;
	if ( !IsValid( pGeom ) || !IsValid( pTris ) )
	{
		NGfx::CBufferLock<NGfx::SGeomVecFull> g( &pGeom, 3 );
		NGfx::CBufferLock<NGfx::S3DTriangle> t( &pTris, 2 );
		g[0].pos = CVec3( -4, 8, 4 );
		//g[0].color = NGfx::SPixel8888(255,255,255);
		//g[0].tex = CVec2(0,1);
		g[1].pos = CVec3( 4, 8, 4 );
		//g[1].color = NGfx::SPixel8888(255,255,255);
		//g[1].tex = CVec2(1,1);
		g[2].pos = CVec3( 4, 8, -4 );
		//g[2].color = NGfx::SPixel8888(255,255,255);
		//g[2].tex = CVec2(1,0);
		g[3].pos = CVec3( -4, 8, -4 );
		//g[3].color = NGfx::SPixel8888(255,255,255);
		//g[3].tex = CVec2(0,0);
		t[0] = NGfx::S3DTriangle( 0, 1, 2 );
		t[1] = NGfx::S3DTriangle( 0, 2, 3 );
	}
	FillTexture( pRT.GetPtr(), 32, 0 );
	//FillTexture( pRT.GetPtr(), 32>>3, 3 );
	NGfx::CRenderContext ctx;
	ctx.SetVirtualRT();
	ctx.ClearBuffers(0);
	ctx.SetTransform( ts.Get() );
	//				ctx.SetRenderTarget( 1 );
	ctx.SetDepth( NGfx::DEPTH_NONE );
//ctx.SetAlphaCombine( NGfx::COMBINE_NONE );
	//ctx.SetRenderTarget( 0 );
	ctx.SetCulling( NGfx::CULL_NONE );
	NGfx::SEffTexture dt;
	dt.pTex = pRT;
	ctx.SetEffect( &dt );
	ctx.DrawPrimitive( pGeom, pTris );

	NGfx::ShowTexture( NGfx::GetRegisterTexture(0), 1 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void TestGetFrameData()
{
	int nTestSize = 32;
	const int N_BUFS = 1;
	InitTS();
	static int nTarget;
	static CObj<NGfx::CTexture> pTarget[N_BUFS], pSrc;
	if ( !pTarget[0] )
	{
		for ( int k = 0; k < N_BUFS; ++k )
			pTarget[k] = NGfx::MakeRenderTarget( nTestSize, nTestSize, NGfx::SPixel8888::ID );
		pSrc = NGfx::MakeTexture( nTestSize, nTestSize, 1, NGfx::SPixel8888::ID, NGfx::REGULAR, NGfx::CLAMP );
	}
	nTarget = (nTarget + 1) % N_BUFS;
	NGfx::SEffConstLight l;
	l.color = CVec3(1,1,0);


	NGfx::CRenderContext rc;
	rc.SetTextureRT( pTarget[nTarget] );
	rc.ClearBuffers();
	rc.SetEffect( &l );
	for ( int k = 0; k < 100; ++k )
		DrawTriangle( &rc );
	// can start thread that would lock stuff!
//	NGfx::pDevice->EndScene();
//	NGfx::pDevice->BeginScene();

	for ( int k = 0; k < 100000; ++k )
		pSrc = pSrc;

	{
		//NGfx::CTextureLock<NGfx::SPixel8888> tl( pSrc, 0, NGfx::INPLACE_READONLY );
		NGfx::CTextureLock<NGfx::SPixel8888> tl( pTarget[nTarget], 0, NGfx::READONLY );
	}

	rc.SetScreenRT();
	rc.ClearBuffers();
	rc.SetEffect( &l );
	rc.SetTransform( ts.Get() );
	for ( int k = 0; k < 500; ++k )
		DrawTriangle( &rc );
	NGfx::Flip();

	for ( int k = 0; k < 300000; ++k )
		pSrc = pSrc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void MainLoop()
{
	CTransformStack ts;
	ts.MakeProjective( 1 );
	SHMatrix cameraPos;
	MakeMatrix( &cameraPos, CVec3( 0,0,0 ), CVec3(0,1,0) );
	ts.SetCamera( cameraPos );
	NGfx::bOutputFPS = true;
	//CObj<NGScene::IGScene> pScene = NGScene::CreateScene( 0 );
	//NInput::CBind cmdExit("exit"), cmdPrintScreen("screenshot");
	for (;;)
	{
		NWinFrame::PumpMessages();
		bool bActive = NWinFrame::IsAppActive() && NGfx::Is3DActive();
		/*NInput::PumpMessages( bActive );
		NInput::SEvent msg;
		bool bExit = false;
		while ( NInput::GetEvent( &msg ) )
		{
			if ( cmdExit.ProcessEvent( msg ) )
				bExit = true;
			//if ( cmdPrintScreen.ProcessEvent( msg ) )
			//	NGScene::MakeScreenShot();
		}*/
		NWinFrame::SWindowsMsg msg;
		while ( NWinFrame::GetMessage( &msg ) ) ;
		if ( NWinFrame::IsExit() )//|| bExit )
			break;
		NGfx::SetGamma( bActive );
		if ( bActive )
		{
			//NGScene::Clear( CVec3(0.5f, 0.5f, 0.5f ) );
			//pScene->Draw( &ts, 0xffff );
			//Test2DQuadRender();
			TestTriangle();
			//TestQueue( nFlyCheck );
			//TestVBFillSpeed();
			//TestPerformance();
			//TestProjective();
			//DrawScene();
			//TestThirdProjective();
			NGfx::Flip();
			//TestGetFrameData();
		}
		if ( !bActive )
			Sleep( 40 );
	}
	//pScene = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
#ifdef _DEBUG
  int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
  tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag( tmpFlag );
	//_CrtSetBreakAlloc( 7439 );
#endif // _DEBUG
	nFlyCheck = atoi( lpCmdLine );
	//NGlobal::SetVar( "gfx_tnl_mode", 1 );
	//NGlobal::SetVar( "gfx_nopixelshaders", 1 );
	NGlobal::SetVar( "gfx_static_nooverwrite", 1 );
	//NGlobal::SetVar( "gfx_fix_ban_np2", 1 );
	//CalcKernel();
//	NGScene::AddResourceDir( ".\\res" );
	// process params
	int nModeXSize = 640, nModeYSize = 480, nRefreshLimit = 1000;
	//int nModeXSize = 800, nModeYSize = 600, nRefreshLimit = 1000;
	//int nModeXSize = 320, nModeYSize = 200, nRefreshLimit = 1000;
	//int nModeXSize = 256, nModeYSize = 256, nRefreshLimit = 1000;
	//int nModeXSize = 400, nModeYSize = 300, nRefreshLimit = 1000;
	NGfx::EFS fullScreen = NGfx::WINDOWED;//NGfx::FULL_SCREEN;//
	// init subsystems
	if ( !NWinFrame::InitApplication( hInstance, "A5", "A5" ) )
		return 0;
	//DWORD dwGfxFlags = NGfx::CF_NVIDIA_NP2_HACK;// | NGfx::CF_NOPS | NGfx::CF_NOVS;
	if ( !NGfx::Init3D( NWinFrame::GetWnd() ) )//, dwGfxFlags ) )
	{
		ASSERT(0); // DX8 not found
		MessageBox( 0, "Failed to initialize Direct3D8", "Error", MB_OK );
		return 0;
	}
	NGfx::SRenderTargetsInfo rtInfo;
	rtInfo.nRegisters = 2;
	if ( !NGfx::SetMode( NGfx::SVideoMode( nModeXSize, nModeYSize, 32, fullScreen, nRefreshLimit ), rtInfo ) )
	{
		ASSERT(0); // no mode found
		if ( fullScreen == NGfx::FULL_SCREEN )
			MessageBox( 0, "Failed to set display mode", "Error", MB_OK );
		else
			MessageBox( 0, "Need 32 bit color depth mode set on desktop to run in window", "Error", MB_OK );
		return 0;
	}
	/*if ( !NInput::InitInput( NWinFrame::GetWnd() ) )
	{
		ASSERT(0); // DX8input not found
		MessageBox( 0, "Failed to initialize DirectInput", "Error", MB_OK );
		return 0;
	}*/
	//NInput::LoadBindScript( "bind.cfg" );
	//NInput::SetBindSection( "default" );
	//
	MainLoop();
	//
	//NInput::DoneInput();
	NGfx::Done3D();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
