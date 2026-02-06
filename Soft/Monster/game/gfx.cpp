#include "StdAfx.h"
#include "Win32Helper.h"
#include "HPTimer.h"
#include <D3D8.h>
// CRAP?
#include <D3Dx8.h>
#include "GfxInternal.h"
/////////////////////////////////////////////////////////////////////////////////////
using namespace NGfx;
namespace NGfx
{
/////////////////////////////////////////////////////////////////////////////////////
const int N_TEMPVB_SIZE = 20000;//2730;//65500;//10000;
const int N_TEMPIB_SIZE = 128 * 1024;//65500;//32768;
const int N_SYSMEM_TEXTURES = 4;
const int N_SYSMEM_TEXTURE_SIZE = 512;
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NWin32Helper::com_ptr<IDirect3D8> pD3D;
NWin32Helper::com_ptr<IDirect3DDevice8> pDevice;
static D3DPRESENT_PARAMETERS pp;
static D3DCAPS8 devCaps;
static HWND hWnd;
/////////////////////////////////////////////////////////////////////////////////////
// forward declarations
static void DestroyAllDXObjects();
static HRESULT InitDXObjects();
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
static NHPTimer::STime timeFrameStart;
static bool bDeviceCreated = false;
/////////////////////////////////////////////////////////////////////////////////////
static HRESULT ResetDevice()
{
	HRESULT hRes;
	if ( !bDeviceCreated )
	{
		hRes = pD3D->CreateDevice( 
			D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, 
			hWnd, 
			D3DCREATE_MIXED_VERTEXPROCESSING,//D3DCREATE_SOFTWARE_VERTEXPROCESSING,//D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, // CRAP, should be pure hardware
			&pp,
			pDevice.GetAddr() );
		if FAILED( hRes )
			OutputDebugString("Gdx::Failed to create device\n" );
	}
	else
	{
		DestroyAllDXObjects();
		hRes = pDevice->Reset( &pp );
	}
	SetWindowPos( 
		hWnd, 
		HWND_NOTOPMOST, 
		0, 0, pp.BackBufferWidth, pp.BackBufferHeight, 
		SWP_SHOWWINDOW );
	if ( hRes == D3D_OK )
	{
		hRes = InitDXObjects();
		bDeviceCreated = true;
	}
	return hRes;
}
/////////////////////////////////////////////////////////////////////////////////////
static void DeviceFinalRelease()
{
	DestroyAllDXObjects();
	pD3D = 0;
	pDevice = 0;
	bDeviceCreated = false;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////////////////////////////////////
static int GetBpp( D3DFORMAT format )
{
	switch ( format )
	{
		case D3DFMT_R8G8B8:   return 24;
    case D3DFMT_A8R8G8B8: return 32;
    case D3DFMT_X8R8G8B8: return 32;
    case D3DFMT_R5G6B5:   return 16;
    case D3DFMT_X1R5G5B5: return 16;
    case D3DFMT_A1R5G5B5: return 16;
    case D3DFMT_A4R4G4B4: return 16;
    case D3DFMT_R3G3B2:   return 8;
    case D3DFMT_A8:       return 8;
    case D3DFMT_A8R3G3B2: return 16;
    case D3DFMT_X4R4G4B4: return 16;

    case D3DFMT_A8P8:     return 16;
    case D3DFMT_P8:       return 8;

    case D3DFMT_L8:       return 8;
    case D3DFMT_A8L8:     return 16;
    case D3DFMT_A4L4:     return 8;

    case D3DFMT_V8U8:     return 16;
    case D3DFMT_L6V5U5:   return 16;
    case D3DFMT_X8L8V8U8: return 32;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
static int GetZBpp( D3DFORMAT format )
{
	switch ( format )
	{
		case D3DFMT_D16_LOCKABLE: return 16;
    case D3DFMT_D32:          return 32;
    case D3DFMT_D15S1:        return 16;
    case D3DFMT_D24S8:        return 32;
    case D3DFMT_D16:          return 16;
    case D3DFMT_D24X8:        return 32;
    case D3DFMT_D24X4S4:      return 32;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
static int D3DFormat2PixelID( D3DFORMAT format )
{
	switch ( format )
	{
    case D3DFMT_A8R8G8B8: return 0;
    case D3DFMT_X8R8G8B8: return 0;
    case D3DFMT_R5G6B5:   return 1;
	}
	ASSERT(0);
	return -1;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// DX initialisation/finalisation
/////////////////////////////////////////////////////////////////////////////////////
static bool TestZBufferFormat( D3DFORMAT screen, D3DFORMAT rTarget, D3DFORMAT zBuf )
{
	if ( D3D_OK != pD3D->CheckDeviceFormat( 
		D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		screen, 
		D3DUSAGE_DEPTHSTENCIL,
		D3DRTYPE_SURFACE,
		zBuf ) )
		return false;
	return D3D_OK == pD3D->CheckDepthStencilMatch( 
		D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		screen, 
		rTarget, 
		zBuf );
}
/////////////////////////////////////////////////////////////////////////////////////
static bool TestRTargetFormat( D3DFORMAT screen, D3DFORMAT rTarget )
{
	if ( D3D_OK != pD3D->CheckDeviceFormat( 
		D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		screen,
		D3DUSAGE_RENDERTARGET,
		D3DRTYPE_TEXTURE,
		rTarget ) )
		return false;
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
static D3DFORMAT GetZBufferFormat( D3DFORMAT screen, D3DFORMAT rTarget )
{
	if ( GetBpp( rTarget ) > 16 )
	{
		if ( TestZBufferFormat( screen, rTarget, D3DFMT_D32 ) )
			return D3DFMT_D32;
		if ( TestZBufferFormat( screen, rTarget, D3DFMT_D24S8 ) )
			return D3DFMT_D24S8;
		if ( TestZBufferFormat( screen, rTarget, D3DFMT_D24X8 ) )
			return D3DFMT_D24X8;
		if ( TestZBufferFormat( screen, rTarget, D3DFMT_D24X4S4 ) )
			return D3DFMT_D24X4S4;
	}
	if ( TestZBufferFormat( screen, rTarget, D3DFMT_D16 ) )
		return D3DFMT_D16;
	ASSERT(0);
	return D3DFMT_UNKNOWN;
}
/////////////////////////////////////////////////////////////////////////////////////
static bool FillPresent( int nSizeX, int nSizeY, int nBpp, EFS fullScreen )
{
	HRESULT hr;
	D3DPRESENT_PARAMETERS ppOld = pp;
	memset( &pp, 0, sizeof(pp) );
	if ( FULL_SCREEN == fullScreen )
	{
		// search through modes to find fitting
		bool bFound = false;
		D3DDISPLAYMODE best, tmp;
		for ( int i = 0; i < pD3D->GetAdapterModeCount( D3DADAPTER_DEFAULT ); i++ )
		{
			pD3D->EnumAdapterModes( D3DADAPTER_DEFAULT, i, &tmp );
			if ( nSizeX == tmp.Width && nSizeY == tmp.Height && GetBpp( tmp.Format ) == nBpp )
			{
				if FAILED( pD3D->CheckDeviceType(
					D3DADAPTER_DEFAULT, 
					D3DDEVTYPE_HAL, 
					tmp.Format, 
					tmp.Format, 
					FALSE ) )
					continue;
				if ( !bFound || tmp.RefreshRate > best.RefreshRate )
				{
					bFound = true;
					best = tmp;
				}
			}
		}
		if ( !bFound )
		{
			pp = ppOld;
			return false;
		}
		//
		pp.BackBufferWidth = nSizeX;
		pp.BackBufferHeight = nSizeY;
		pp.BackBufferFormat = best.Format;
		pp.BackBufferCount = 1;

		pp.MultiSampleType = D3DMULTISAMPLE_NONE;

		pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		pp.hDeviceWindow = hWnd;
		pp.Windowed = FALSE;
		pp.EnableAutoDepthStencil =	TRUE;
		pp.AutoDepthStencilFormat = GetZBufferFormat( pp.BackBufferFormat, pp.BackBufferFormat );//D3DFMT_UNKNOWN;//D3DFMT_UNKNOWN_D24S8;// D3DFMT_UNKNOWN;

		pp.FullScreen_RefreshRateInHz = best.RefreshRate;//D3DPRESENT_RATE_UNLIMITED;//100;//0;//D3DPRESENT_RATE_UNLIMITED;
		if ( devCaps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE )
			pp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		else
			pp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		return true;
	}
	//
	// Windowed mode
	D3DDISPLAYMODE desktop;
  // Get the current desktop display mode of the current adapters
  // to ensure that our back buffer bit depth is the same as that of 
  // windowed display depth to work in windowed mode
  hr = pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &desktop );
  if FAILED(hr) 
	{
		pp = ppOld;
		return false;
	}
	pp.BackBufferWidth = nSizeX;
	pp.BackBufferHeight = nSizeY;
	pp.BackBufferFormat = desktop.Format;//D3DFMT_UNKNOWN;//D3DFMT_X8R8G8B8;// D3DFMT_UNKNOWN;//D3DFMT_UNKNOWN_D24S8;
	pp.BackBufferCount = 1;

	pp.MultiSampleType = D3DMULTISAMPLE_NONE;

	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.hDeviceWindow = hWnd;
	pp.Windowed = TRUE;
	pp.EnableAutoDepthStencil =	TRUE;
	pp.AutoDepthStencilFormat = GetZBufferFormat( pp.BackBufferFormat, pp.BackBufferFormat );
	//pp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;//D3DFMT_UNKNOWN_D24S8;// D3DFMT_UNKNOWN;
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
static bool InitD3D()
{
	pD3D.Create( Direct3DCreate8( D3D_SDK_VERSION ) );
	if ( pD3D == 0 )
		return false;
	pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &devCaps );
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
bool NGfx::SetMode( int nSizeX, int nSizeY, int nBpp, EFS fullScreen )
{
	if ( !FillPresent( nSizeX, nSizeY, nBpp, fullScreen ) )
		return false;
	ResetDevice();
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
static void WalkLostable();
void NGfx::Flip()
{
	HRESULT hr = pDevice->EndScene();
	ASSERT( hr == D3D_OK );
	//
	pDevice->Present( 0, 0, hWnd, 0 );
	//
	double fFrameTime = NHPTimer::GetTimePassed( &timeFrameStart );
	float fFPS = 1 / fFrameTime;
	char szBuf[1024];
	sprintf( szBuf, "FPS = %f\n", fFPS );
	OutputDebugString( szBuf );
	//
	WalkLostable();
	//
	hr = pDevice->BeginScene();
	ASSERT( hr == D3D_OK );
}
/////////////////////////////////////////////////////////////////////////////////////
void NGfx::ClearBuffers( DWORD dwColor )
{
	HRESULT hr;
	hr = pDevice->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, dwColor, 1, 0 );
	ASSERT( hr == D3D_OK );
}
/////////////////////////////////////////////////////////////////////////////////////
// test cooperative level
bool NGfx::Is3DActive()
{
	HRESULT hr = pDevice->TestCooperativeLevel();
	if ( hr == D3DERR_DEVICELOST )
	{
		OutputDebugString("Device is lost but trying to restore\n" ); 
		//return false; // CRAP, commented due to DX bug
	}
	if ( hr == D3D_OK )
		return true;
	if ( pp.Windowed && !FillPresent( pp.BackBufferWidth , pp.BackBufferHeight, 16, WINDOWED ) )
		return false;
	ResetDevice();
	hr = pDevice->TestCooperativeLevel();
	if ( hr == D3DERR_DEVICELOST )
		return false;
	ASSERT( hr == D3D_OK );
	return hr == D3D_OK;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// LINEAR BUFFERS
/////////////////////////////////////////////////////////////////////////////////////
// CVB
/////////////////////////////////////////////////////////////////////////////////////
void CVB::Create( int _nSize, EDynamic eDynamic )
{
	nSize = _nSize;
	//	
	HRESULT hRes = pDevice->CreateVertexBuffer( 
		nSize, 
		D3DUSAGE_SOFTWAREPROCESSING | ( eDynamic == DYNAMIC ? D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY : D3DUSAGE_WRITEONLY ),
		0,
		D3DPOOL_SYSTEMMEM,// D3DPOOL_DEFAULT,//D3DPOOL_SYSTEMMEM,// 
		obj.GetAddr()
		);
	ASSERT( D3D_OK == hRes );
}
/////////////////////////////////////////////////////////////////////////////////////
// CIB
/////////////////////////////////////////////////////////////////////////////////////
void CIB::Create( int _nSize, EDynamic eDynamic )
{
	nSize = _nSize;
	//
	HRESULT hRes = pDevice->CreateIndexBuffer(
		nSize, 
		D3DUSAGE_SOFTWAREPROCESSING | (eDynamic == DYNAMIC ? D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY : D3DUSAGE_WRITEONLY ),
		D3DFMT_INDEX16,
		D3DPOOL_DEFAULT,
		obj.GetAddr()
		);
	ASSERT( D3D_OK == hRes );
}
/////////////////////////////////////////////////////////////////////////////////////
// CBufferBase
/////////////////////////////////////////////////////////////////////////////////////
int CBufferBase::GetBufSize() const { return nBufSize; }
/////////////////////////////////////////////////////////////////////////////////////
int CBufferBase::GetSize() const { return nSize; }
/////////////////////////////////////////////////////////////////////////////////////
void CBufferBase::SetSize( int _nSize )
{
	ASSERT( _nSize <= nBufSize ); 
	nSize = _nSize;
}
/////////////////////////////////////////////////////////////////////////////////////
// CGeometry
/////////////////////////////////////////////////////////////////////////////////////
CGeometry::CGeometry()
{
}
/////////////////////////////////////////////////////////////////////////////////////
int CGeometry::GetFormatID() const
{
	return nFormatID;
}
/////////////////////////////////////////////////////////////////////////////////////
void* CGeometry::Lock()
{
	ASSERT( pDX->IsValid() );
	int nOffset = GetStart() * GetStride();
	DxLock( *pDX );//, nOffset, obj.pObj->GetBufSize() * obj.nStride );
	return pDX->pLocked + nOffset;
}
/////////////////////////////////////////////////////////////////////////////////////
void CGeometry::Unlock()
{
	DxUnlock( *pDX );
}
/////////////////////////////////////////////////////////////////////////////////////
// CTriList
/////////////////////////////////////////////////////////////////////////////////////
CTriList::CTriList()
{
}
/////////////////////////////////////////////////////////////////////////////////////
int CTriList::GetFormatID() const
{
	return S3DTriangle::ID;
}
/////////////////////////////////////////////////////////////////////////////////////
void* CTriList::Lock()
{
	ASSERT( pDX->IsValid() );
	int nOffset = GetStart() * GetStride();
	DxLock( *pDX );//, nOffset, obj.pObj->GetBufSize() * obj.nStride );
	return pDX->pLocked + nOffset;
}
/////////////////////////////////////////////////////////////////////////////////////
void CTriList::Unlock()
{
	DxUnlock( *pDX );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// TEXTURES
/////////////////////////////////////////////////////////////////////////////////////
// CTexture
/////////////////////////////////////////////////////////////////////////////////////
CTexture::CTexture()
{
}
/////////////////////////////////////////////////////////////////////////////////////
CTexture::CTexture( int _nXSize, int _nYSize, int nLevels, D3DFORMAT _format, DWORD dwUsage ): 
	nXSize(_nXSize), nYSize(_nYSize), format(_format)
{
	HRESULT hRes = pDevice->CreateTexture( 
		nXSize, 
		nYSize, 
		nLevels, 
		dwUsage, 
		format, 
		D3DPOOL_DEFAULT,
		obj.GetAddr() );
	ASSERT( D3D_OK == hRes );
}
/////////////////////////////////////////////////////////////////////////////////////
int CTexture::GetPixelID() const
{
	return D3DFormat2PixelID( format );
}
/////////////////////////////////////////////////////////////////////////////////////
int CTexture::GetXSize() const
{
	return nXSize; 
}
/////////////////////////////////////////////////////////////////////////////////////
int CTexture::GetYSize() const 
{
	return nYSize; 
}
/////////////////////////////////////////////////////////////////////////////////////
// CSysTexture
/////////////////////////////////////////////////////////////////////////////////////
// system memory texture for update operations
class CSysTexture: public CObjectBase
{
	OBJECT_BASIC_METHODS(CSysTexture);
	bool bBusy;
public:
	NWin32Helper::com_ptr<IDirect3DSurface8> pSurface;
	NWin32Helper::com_ptr<IDirect3DTexture8> pTexture;
	//
	CSysTexture() { bBusy = false; }
	CSysTexture( D3DFORMAT _format );
	void MarkBusy() { bBusy = true; }
	void Free() { bBusy = false; }
	bool IsBusy() const { return bBusy; }
};
/////////////////////////////////////////////////////////////////////////////////////
CSysTexture::CSysTexture( D3DFORMAT _format )
{
	bBusy = false;
	HRESULT hRes = pDevice->CreateTexture( 
		N_SYSMEM_TEXTURE_SIZE,
		N_SYSMEM_TEXTURE_SIZE,
		1,
		0, 
		_format, 
		D3DPOOL_SYSTEMMEM,
		pTexture.GetAddr() );
	ASSERT( D3D_OK == hRes ); // if this fails no need to run further
	hRes = pTexture->GetSurfaceLevel( 0, pSurface.GetAddr() );
	ASSERT( D3D_OK == hRes );
}
/////////////////////////////////////////////////////////////////////////////////////
struct SSurfaceRing: public CFundament
{
	vector< CObj<CSysTexture> > textures;
	int nCurrent;
	//
	SSurfaceRing( D3DFORMAT _format, int nSize );
	void Switch() { nCurrent = ( nCurrent + 1 ) % textures.size(); }
	CSysTexture* GetTexture();
};
/////////////////////////////////////////////////////////////////////////////////////
SSurfaceRing::SSurfaceRing( D3DFORMAT _format, int nSize )
{
	textures.resize(0);
	textures.reserve( nSize );
	for ( int i = 0; i < nSize; i++ )
		textures.push_back( new CSysTexture( _format ) );
	nCurrent = 0;
}
/////////////////////////////////////////////////////////////////////////////////////
CSysTexture* SSurfaceRing::GetTexture()
{
	CSysTexture *pRes;
	for(;;)
	{
		pRes = textures[nCurrent];
		if ( !pRes->IsBusy() )
			break;
		Switch();
	}
	Switch();
	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////
typedef map<D3DFORMAT, CPtr<SSurfaceRing> > CFormatRingMap;
static CFormatRingMap sysTextures;
/////////////////////////////////////////////////////////////////////////////////////
// CTextureLockBase
/////////////////////////////////////////////////////////////////////////////////////
CTextureLockBase::CTextureLockBase( CTexture *pTexture, int _nLevel, 
																			 EAccess _access )
	: pObj( pTexture ), nLevel( _nLevel ), access( _access )
{
	static int nShitBuffer[1024];
	HRESULT hr;
	NWin32Helper::com_ptr<IDirect3DSurface8> pTempBuf;
	//
	DWORD dwLockFlags = D3DLOCK_NO_DIRTY_UPDATE;
	if ( access != INPLACE && access != INPLACE_READONLY )
	{
		CFormatRingMap::iterator i = sysTextures.find( pTexture->GetFormat() );
		ASSERT( i != sysTextures.end() );
		pLocker = i->second->GetTexture();
		pLocker->MarkBusy();
		ASSERT( pTexture->GetXSize() <= N_SYSMEM_TEXTURE_SIZE );
		ASSERT( pTexture->GetYSize() <= N_SYSMEM_TEXTURE_SIZE );
		//
		pTempBuf = pLocker->pSurface;
		if ( pDevice && pObj->IsValid() && access != WRITEONLY )
		{
			NWin32Helper::com_ptr<IDirect3DSurface8> pSrc;
			pObj->obj->GetSurfaceLevel( nLevel, pSrc.GetAddr() );
			hr = pDevice->CopyRects( pSrc, 0, 0, pTempBuf, 0 );
			ASSERT( D3D_OK == hr );
		}
	}
	else
	{
		if ( access == INPLACE_READONLY )
			dwLockFlags |= D3DLOCK_READONLY;
		if ( pObj->IsValid() )
			pObj->obj->GetSurfaceLevel( nLevel, pTempBuf.GetAddr() );
	}
	D3DLOCKED_RECT buf;
	if ( pTempBuf )
	{
		hr = pTempBuf->LockRect( &buf, 0, dwLockFlags );
		ASSERT( D3D_OK == hr );
	}
	else
	{
		ASSERT(0); // texture to be locked is unavailable
		buf.pBits = nShitBuffer;
		buf.Pitch = 0;
	}
	char *pData = (char*)buf.pBits;
	raws.resize( pTexture->GetYSize() );
	std::vector<void*>::iterator tek = raws.begin(), fin = raws.end();
	while ( tek != fin )
	{
		*tek++ = pData; 
		pData += buf.Pitch;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
CTextureLockBase::~CTextureLockBase()
{
	NWin32Helper::com_ptr<IDirect3DSurface8> pTempBuf;
	if ( access == INPLACE || access == INPLACE_READONLY )
	{
		if ( pObj->IsValid() )
			pObj->obj->GetSurfaceLevel( nLevel, pTempBuf.GetAddr() );
		if ( pTempBuf )
		{
			pTempBuf->UnlockRect();
			// mark as dirty changed regions
			if ( access == INPLACE )
			{
				int nRects = dirtyRects.size();
				if ( nRects > 0 )
				{
					for ( int i = 0; i < nRects; i++ )
						pObj->obj->AddDirtyRect( (RECT*)&dirtyRects[i] );
				}
				else
					pObj->obj->AddDirtyRect( 0 );
			}
		}
	}
	else
	{
		ASSERT( pLocker != 0 );
		pLocker->Free();
		if ( pLocker->IsValid() )
		{
			pTempBuf = pLocker->pSurface;
			pTempBuf->UnlockRect();
			if ( pDevice && pObj->IsValid() && access != READONLY )
			{
				NWin32Helper::com_ptr<IDirect3DSurface8> pDst;
				pObj->obj->GetSurfaceLevel( nLevel, pDst.GetAddr() );
				if ( N_SYSMEM_TEXTURE_SIZE > pObj->GetXSize() || N_SYSMEM_TEXTURE_SIZE > pObj->GetYSize() )
					dirtyRects.push_back( CTRect<int>( 0, 0, pObj->GetXSize(), pObj->GetYSize() ) );
				int nRects = dirtyRects.size();
				RECT *pRects = nRects == 0 ? 0 : (RECT*)&dirtyRects[0];
				HRESULT hRes = pDevice->CopyRects( 
					pTempBuf, 
					pRects,
					nRects,
					pDst,
					0 );
				ASSERT( hRes == D3D_OK );
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// RENDER TARGETS
/////////////////////////////////////////////////////////////////////////////////////
// CRenderTarget
/////////////////////////////////////////////////////////////////////////////////////
class CRenderTarget: public CObjectBase
{
OBJECT_BASIC_METHODS(CRenderTarget);
public:
	NWin32Helper::com_ptr<IDirect3DSurface8> pColor;
	NWin32Helper::com_ptr<IDirect3DSurface8> pDepth;
	//
	CRenderTarget();
	CRenderTarget( int ); // fake parameter to distinguish between it and simple constructor
	CRenderTarget( int nXSize, int nYSize, CTexture *_pTexture, IDirect3DSurface8 *_pDepth );
	CTexture* GetTexture() const;
	IDirect3DSurface8* GetColor() const { return pColor; }
	IDirect3DSurface8* GetDepth() const { return pDepth; }
private:
	CPtr<CTexture> pTexture;
};
/////////////////////////////////////////////////////////////////////////////////////
CRenderTarget::CRenderTarget()
{
}
/////////////////////////////////////////////////////////////////////////////////////
CRenderTarget::CRenderTarget( int )
{
	HRESULT hr;
	hr = pDevice->GetDepthStencilSurface( pDepth.GetAddr() );
	ASSERT( D3D_OK == hr );
	hr = pDevice->GetRenderTarget( pColor.GetAddr() );
	ASSERT( D3D_OK == hr );
	pTexture = 0;
}
/////////////////////////////////////////////////////////////////////////////////////
CRenderTarget::CRenderTarget( int nXSize, int nYSize, CTexture *_pTexture, IDirect3DSurface8 *_pDepth )
	:pTexture( _pTexture )
{
	D3DSURFACE_DESC sd;
	ASSERT( pTexture->IsValid() );
	if ( !pTexture->IsValid() )
		return;
	//
	pTexture->obj->GetSurfaceLevel( 0, pColor.GetAddr() );
	if ( _pDepth == 0 )
	{
		pColor->GetDesc( &sd );
		pDevice->CreateDepthStencilSurface( 
			nXSize, 
			nYSize, 
			GetZBufferFormat( pp.BackBufferFormat, sd.Format ), 
			D3DMULTISAMPLE_NONE, 
			pDepth.GetAddr() );
	}
	else
		pDepth = _pDepth;
}
/////////////////////////////////////////////////////////////////////////////////////
CTexture* CRenderTarget::GetTexture() const
{
	return pTexture;
}
/////////////////////////////////////////////////////////////////////////////////////
// CDynamicLock
/////////////////////////////////////////////////////////////////////////////////////
template< class TBase, class T >
class CDynamicLock: public TBase
{
public:
	CDynamicLock() {}
	~CDynamicLock() { if ( pObj->IsValid() ) DxUnlock( *pObj->pDX ); }
	void Bind( T *_pObj, DWORD dwFlags )
	{
		if ( pObj->IsValid() )
			DxUnlock( *pObj->pDX ); 
		pObj = _pObj;
		int nOffset = pObj->GetStart() * pObj->GetStride();
		DxLockDynamic( *pObj->pDX, dwFlags );//nOffset, pObj->GetBufSize() * nStride, dwFlags );
		pStart = (Element*)(pObj->pDX->pLocked + nOffset);
	}
	void Free()
	{
		if ( pObj->IsValid() )
			DxUnlock( *pObj->pDX ); 
		pObj = 0;
	}
};
typedef CDynamicLock< CBufferAccess<CGeometry, SEmpty>, CGeometry > CDynamiCGeometryLock;
typedef CDynamicLock< CTriListAccess, CTriList > CDynamiCTriListLock;
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// CORE DX FUNCTIONALITY
/////////////////////////////////////////////////////////////////////////////////////
// all DX objects
static std::list< CMObj<CObjectBase> > lostable;
static CPtr<CGeometry> tempGeometry;
static CDynamiCGeometryLock tempAGeometry;
static CPtr<CTriList> tempTriList;
static CDynamiCTriListLock tempATriList;
static bool bWireFrame = false;
static CPtr<CRenderTarget> pScreen;
int nVBGeomStart, nVBGeomSize;          // start & size of current vertex stream range
/////////////////////////////////////////////////////////////////////////////////////
static void DestroyAllDXObjects()
{
	int i;
	lostable.clear();
	sysTextures.clear();
	pDevice->SetIndices( 0, 0 );
	pDevice->SetStreamSource( 0, 0, 4 );
	for ( i = 0; i < 8; i++ )
		pDevice->SetTexture( i, 0 );
	pScreen = 0;
	EffectsShutdown();
}
/////////////////////////////////////////////////////////////////////////////////////
static void WalkLostable()
{
	for ( std::list< CMObj<CObjectBase> >::iterator i = lostable.begin(); i != lostable.end(); )
	{
		if ( (*i)->IsValid() )
			++i;
		else
			i = lostable.erase( i );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
static CGeometry* CreateVB( int nSize, int nFormatID, CVB *_pVB, int nStart, 
														 EDynamic eDynamic )
{
	CGeometry *pRes;
	CVB *pVB = _pVB;
	int nStride = GetGeomFormatSize( nFormatID );
	if ( pVB == 0 )
	{
		pVB = new CVB;
		pVB->Create( nSize * nStride, eDynamic );
	}
	else
	{
	}
	pRes = new CGeometry( pVB, nSize, nStart, nFormatID, nStride );
	lostable.push_back( pRes );
	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////
static CTriList* CreateIB( int nSize, CIB *_pIB, int nStart, EDynamic eDynamic )
{
	CTriList *pRes;
	CIB *pIB = _pIB;
	if ( pIB == 0 )
	{
		pIB = new CIB;
		pIB->Create( nSize * 6, eDynamic );
	}
	pRes = new CTriList( pIB, nSize, nStart );
	lostable.push_back( pRes );
	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////
static CTexture* CreateTB( int nXSize, int nYSize, int nLevels, D3DFORMAT format,
														DWORD dwUsage )
{
	CTexture *pRes;
	pRes = new CTexture( nXSize, nYSize, nLevels, format, dwUsage );
	lostable.push_back( pRes );
	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////
static CRenderTarget* CreateRT( int nXSize, int nYSize, IDirect3DSurface8 *_pDepth )
{
	CTexture *pTexture;
	//
	D3DFORMAT textureFormat = pp.BackBufferFormat; // CRAP do not know whom to blame either NVidia or MS
	if ( GetBpp( textureFormat ) == 32 )
		textureFormat = D3DFMT_A8R8G8B8;
	ASSERT( TestRTargetFormat( pp.BackBufferFormat, textureFormat ) );
	//
	pTexture = CreateTB( nXSize, nYSize, 1, textureFormat, D3DUSAGE_RENDERTARGET );
	CRenderTarget *pRes = new CRenderTarget( nXSize, nYSize, pTexture, _pDepth );
	lostable.push_back( pRes );
	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////
static CRenderTarget* CreateRTScreen()
{
	CRenderTarget *pRes = new CRenderTarget( 0 );
	lostable.push_back( pRes );
	return pRes;
}
/////////////////////////////////////////////////////////////////////////////////////
static void ApplyWireframe()
{
	if ( bWireFrame )
		pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	else
		pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}
static HRESULT InitDXObjects()
{
	HRESULT hr;
	// initialise temprorary buffers and buffers for data
	// temp VB buf, only size matters
	tempGeometry = CreateVB( N_TEMPVB_SIZE, SGeomVecT1C1::ID, 0, 0, DYNAMIC );
	// temp IB buf
	tempTriList = CreateIB( N_TEMPIB_SIZE, 0, 0, DYNAMIC );
	// buffers for dynamic textures
	sysTextures[D3DFMT_A8R8G8B8] = new SSurfaceRing( D3DFMT_A8R8G8B8, N_SYSMEM_TEXTURES );
	sysTextures[D3DFMT_R5G6B5] = new SSurfaceRing( D3DFMT_R5G6B5, N_SYSMEM_TEXTURES );
	//
	ApplyWireframe();
	// CRAP{ some initialisation
	pDevice->SetRenderState( D3DRS_SOFTWAREVERTEXPROCESSING, TRUE );
	//pDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
	pDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
	pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	//pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	//pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	// CRAP}
	pScreen = CreateRTScreen();
	//
	hr = EffectsInit();
	ASSERT( D3D_OK == hr );
	//
	hr = pDevice->BeginScene();
	ASSERT( D3D_OK == hr );
	return D3D_OK;
}
/////////////////////////////////////////////////////////////////////////////////////
void NGfx::SetWireframe( bool bWire )
{
	bWireFrame = bWire;
	ApplyWireframe();
}
/////////////////////////////////////////////////////////////////////////////////////
CGeometry* NGfx::MakeGeometry( int nSize, int nFormatID, EDynamic eDynamic )
{
	return CreateVB( nSize, nFormatID, 0, 0, eDynamic );
}
/////////////////////////////////////////////////////////////////////////////////////
CTriList* NGfx::MakeTriList( const int nTris, EDynamic eDynamic )
{
	return CreateIB( nTris, 0, 0, eDynamic );
}
/////////////////////////////////////////////////////////////////////////////////////
void NGfx::DrawPrimitive( const CTriList &tris )
{
	ASSERT( tris.IsValid() );
	pDevice->SetIndices( tris.pDX->obj, nVBGeomStart );
	HRESULT hRes = pDevice->DrawIndexedPrimitive( 
		D3DPT_TRIANGLELIST, 
		0, nVBGeomSize,
		tris.GetStart() * 3, 
		tris.GetSize()
		);
	ASSERT( hRes == D3D_OK );
}
/////////////////////////////////////////////////////////////////////////////////////
void NGfx::DrawPrimitive()
{
	tempATriList.Free();
	DrawPrimitive( *tempTriList );
}
/////////////////////////////////////////////////////////////////////////////////////
template<class T,class TDynamicLock>
static void AllocTempBuf( T &obj, TDynamicLock &a, int nSize, int nNewStride )
{
	int nStart = ( obj.GetStart() + obj.GetBufSize() ) * obj.GetStride();
	nStart += nNewStride - ( nStart % nNewStride ); // align on nNewStride border
	if ( obj.pDX->GetSize() >= nStart + nSize * nNewStride )
	{
		obj.Init( nSize, nStart / nNewStride );
		a.Bind( &obj, D3DLOCK_NOOVERWRITE );
	}
	else
	{
		obj.Init( nSize, 0 );
		a.Bind( &obj, D3DLOCK_DISCARD );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
CBufferAccess<CGeometry, SEmpty>& NGfx::MakeTempGeometry( int nSize, int nFormatID )
{
	int nElementSize = GetGeomFormatSize( nFormatID );
	ASSERT( tempGeometry->pDX->GetSize() >= nSize * nElementSize );
	AllocTempBuf( *tempGeometry, tempAGeometry, nSize, nElementSize );
	if ( nElementSize != tempGeometry->GetStride() )
		TempVertexStreamChanged();
	tempGeometry->SetStride( nFormatID, nElementSize );
	return tempAGeometry;
}
/////////////////////////////////////////////////////////////////////////////////////
CGeometry* NGfx::GetTempGeometry()
{
	tempAGeometry.Free();
	return tempGeometry;
}
/////////////////////////////////////////////////////////////////////////////////////
CTriListAccess& NGfx::MakeTempTriList( int nSize )
{
	ASSERT( tempTriList->pDX->GetSize() >= nSize * 6 );
	AllocTempBuf( *tempTriList, tempATriList, nSize, 6 );
	return tempATriList;
}
/////////////////////////////////////////////////////////////////////////////////////
CTexture* NGfx::MakeTexture( int nXSize, int nYSize, int nMipLevels )
{
	return CreateTB( nXSize, nYSize, nMipLevels, D3DFMT_A8R8G8B8, 0 );
}
/////////////////////////////////////////////////////////////////////////////////////
CRenderTarget* NGfx::GetScreen()
{
	return pScreen;
}
/////////////////////////////////////////////////////////////////////////////////////
CRenderTarget* NGfx::MakeRenderTarget( int nXSize, int nYSize, CRenderTarget *pZBufferSrc )
{
	IDirect3DSurface8 *pDepth = pZBufferSrc ? pZBufferSrc->GetDepth() : 0;
	return CreateRT( nXSize, nYSize, pDepth );
}
/////////////////////////////////////////////////////////////////////////////////////
void NGfx::SetRenderTarget( CRenderTarget *pTarget )
{
	HRESULT hr;
	hr = pDevice->EndScene();
	ASSERT( hr == D3D_OK );
	//
	if ( !pTarget->IsValid() )
		pTarget = GetScreen();
	ASSERT( pTarget->IsValid() );
	if ( !pTarget->IsValid() )
		return;
	hr = pDevice->SetRenderTarget( pTarget->GetColor(), pTarget->GetDepth() );
	ASSERT( D3D_OK == hr );
	//
	hr = pDevice->BeginScene();
	ASSERT( hr == D3D_OK );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
bool NGfx::Init3D( HWND _hWnd )
{
	hWnd = _hWnd;
	if ( !InitD3D() )
	{
		ASSERT(0);
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
void NGfx::Done3D()
{
	DeviceFinalRelease();
}
/////////////////////////////////////////////////////////////////////////////////////
}; // namespace NGfx
/////////////////////////////////////////////////////////////////////////////////////
