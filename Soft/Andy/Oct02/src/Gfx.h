#ifndef __GFX_H_
#define __GFX_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GPixelFormat.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGfx
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EFS
{
	WINDOWED,
	FULL_SCREEN
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SVideoMode
{
	int nXSize, nYSize, nBpp, nRefreshLimit;
	EFS fullScreen;
	SVideoMode() { nXSize = 800, nYSize = 600; nBpp = 32; nRefreshLimit = 1000; fullScreen = WINDOWED; }
	SVideoMode( int _nXSize, int _nYSize, int _nBpp, EFS _fullScreen, int _nRefreshLimit = 1000 )
		:nXSize(_nXSize), nYSize(_nYSize), nBpp(_nBpp), nRefreshLimit(_nRefreshLimit), fullScreen(_fullScreen) {}
};
// general
bool Init3D( HWND hWnd );
void Done3D();
bool Is3DActive();
void SetGamma( bool bGamma );
bool SetMode( const SVideoMode &m_ );
bool SetModeFromConfig();
void GetModesList( list<SVideoMode> *pRes );
CVec2 GetScreenRect();
void Flip();
void MakeScreenShot( CArray2D<SPixel8888> *pRes, bool bCorrectGamma );
void CheckBackBufferSize();
//
struct SRenderStats
{
	int nVertices, nTris;
	SRenderStats(): nVertices(0), nTris(0) {}
	void Clear() { nVertices = 0; nTris = 0; }
};
extern SRenderStats renderStats;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif