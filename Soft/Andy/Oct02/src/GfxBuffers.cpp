#include "StdAfx.h"
#include <D3D8.h>
#include "Gfx.h"
#include "GfxBuffers.h"
#include "GfxInternal.h"
#include "Cache.h"
#include "..\Misc\Commands.h"
#include "..\Misc\LogStream.h"

//#include "GfxUtils.h"
//#include "GfxRender.h"
#include "GfxBuffersInternal.h"

const int N_SYSMEM_TEXTURES = 2;
const int N_SYSMEM_TEXTURE_SIZE = 1024;

namespace NGfx
{
static void OnThrashing();
////////////////////////////////////////////////////////////////////////////////////////////////////
// INew2DTexAllocCallback
////////////////////////////////////////////////////////////////////////////////////////////////////
static list<INew2DTexAllocCallback*> alloc2Dcallback;
INew2DTexAllocCallback::INew2DTexAllocCallback()
{
	alloc2Dcallback.push_back( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
INew2DTexAllocCallback::~INew2DTexAllocCallback()
{
	alloc2Dcallback.remove( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void InformNew2DTextureAlloc()
{
	for ( list<INew2DTexAllocCallback*>::iterator i = alloc2Dcallback.begin(); i != alloc2Dcallback.end(); ++i )
		(*i)->NewTextureWasAllocated();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
static list<CPtr<CLockable> > lockableList;
bool bWasLinearBufferLock;
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
T* RegisterDXBuffer( T *p )
{
	lockableList.push_back( p );
	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FreeLinearBuffers()
{
	EraseInvalidRefs( &lockableList );
	for ( list<CPtr<CLockable> >::iterator i = lockableList.begin(); i != lockableList.end(); )
	{
		CLockable *p = *i;
		if ( IsValid( p ) )
		{
			p->Free();
			++i;
		}
		else
			i = lockableList.erase( i );
	}
	bWasLinearBufferLock = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RealSetVertexStream( IDirect3DVertexBuffer8 *pBuf, int nStride );
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_MAX_PRESENTS_IN_QUEUE = 4;
template<class TDXBuffer, class TUserObject>
class CLinearBuffer : public CObjectBase
{
	OBJECT_NOCOPY_METHODS(CLinearBuffer);
	typedef NCache::CGatheringCache<NCache::CShortPtrAllocator, NCache::CFibElement, TUserObject> CCache;
	CObj<TDXBuffer> pDX;
	CObj<CCache> pCache;
	int nFormatID;
	int nStride;
	DWORD dwNextLockFlags, dwLockFlags, dwFirstLockFlags;
	bool bIsDynamicBuffer, bIsThrashing;
	int nKeepObjectsFrames;
	struct SBuffersPerFrame
	{
		vector<CObj<TUserObject> > data;
	};
	list<SBuffersPerFrame> frames;
public:
	CLinearBuffer() {}
	CLinearBuffer( int _nSize, int _nStride, int _nFormatID, ETrueBufferUsage usage ) : nFormatID(_nFormatID), nStride(_nStride), bIsThrashing(false)
	{
		int nFibSize = NCache::GetMajorFib( _nSize );
		int nSize = NCache::fib(nFibSize);
		pDX = RegisterDXBuffer( new TDXBuffer( nSize * _nStride, usage ) );//DYNAMIC ) );
		pCache = new CCache( nCurrentFrame );
		NCache::CFibElement root;
		root.nSize = nFibSize;
		root.nShift = 0;
		pCache->AddRoot( root );
		frames.push_front();
		if ( usage == DYNAMIC )
		{
			bIsDynamicBuffer = true;
			dwLockFlags = D3DLOCK_NOOVERWRITE;
			dwFirstLockFlags = D3DLOCK_DISCARD;
			nKeepObjectsFrames = 1;
		}
		else
		{
			bIsDynamicBuffer = false;
			if ( bStaticNooverwrite && bHardwareVP )
			{
				dwLockFlags = D3DLOCK_NOOVERWRITE;
				nKeepObjectsFrames = N_MAX_PRESENTS_IN_QUEUE + 1;
			}
			else
			{
				dwLockFlags = 0;
				nKeepObjectsFrames = 1;
			}
			dwFirstLockFlags = dwLockFlags;
		}
		dwNextLockFlags = dwFirstLockFlags;
	}
	TDXBuffer* GetBuffer() const { return pDX; }
	int GetFormatID() const {	return nFormatID; }
	int GetStride() const { return nStride; }
	TUserObject* Alloc( int nSize )
	{
		ASSERT( IsValid( pCache ) );
		//ASSERT( nSize < 65536 );
		//NCache::MRU_TYPE nBestMRU = NCache::MRU_LAST;
		NCache::CFibElement el;
		el.nSize = NCache::GetMajorFib( nSize );
		CCache::SCachePlace best;
		if ( !pCache->GetPlace( el, &best ) )
		{
			ASSERT( 0 );
			return 0;
		}
		ASSERT( pCache->GetCurrentRU() == nCurrentFrame );
		if ( best.nMRU >= nCurrentFrame - N_MAX_PRESENTS_IN_QUEUE )
		{
			bIsThrashing = true;
			OnThrashing();
			// wait till rendered
			HRESULT hr;
			BYTE *pFake;
			hr = pDX->obj->Lock( 0, 0, &pFake, 0 );
			ASSERT( hr == D3D_OK );
			hr = pDX->obj->Unlock();
			ASSERT( hr == D3D_OK );
			// recalc place after shit happened
			if ( !pCache->GetPlace( el, &best ) )
			{
				ASSERT( 0 );
				return 0;
			}
		}
		TUserObject *pRes = new TUserObject( this );
		pCache->PerformAlloc( pRes, &best );
		pRes->nStart = best.resPlace.nShift;
		pRes->nBufSize = NCache::fib( best.resPlace.nSize );
		pRes->nSize = nSize;
		WasTouched( pRes );
		return pRes;
	}
	void NextFrame( bool bOnThrashing ) 
	{
		pCache->AdvanceFrameCounter();
		int nReserve = 10;
		if ( !frames.empty() )
			nReserve = frames.front().data.size() + 64;
		frames.insert( frames.begin() )->data.reserve( nReserve );
		// remove all elements for dynamic buffer
		if ( bIsDynamicBuffer )
			pCache->Clear();
		while ( frames.size() > nKeepObjectsFrames )
			frames.pop_back();
		if ( !bOnThrashing )
			bIsThrashing = false;
		dwNextLockFlags = dwFirstLockFlags;
	}
	void WasTouched( TUserObject *p ) { frames.front().data.push_back( p ); }
	void CalcStats( NCache::SStats *pStats ) 
	{ 
		if ( IsValid( pCache ) )
			pCache->CalcStats( pStats ); 
		pStats->bThrashing |= bIsThrashing;
	}
	void DrawRU( CTexture *pTarget )
	{
		CTextureLock<SPixel8888> tl( pTarget, 0, INPLACE );
		vector<CCache::SStatePlace> places;
		pCache->GetState( &places );
		for ( int y = 0; y < tl.GetYSize(); ++y )
			for ( int x = 0; x < tl.GetXSize(); ++x )
				tl[y][x] = SPixel8888( 255,255,255 );
		for ( int k = 0; k < places.size(); ++k )
		{
			const CCache::SStatePlace &p = places[k];
			SPixel8888 color;
			if ( p.pUser )
			{
				switch ( pCache->GetCurrentRU() - p.nMRU )
				{
				case 0: color = SPixel8888( 0,0,255 ); break;
				case 1: color = SPixel8888( 0,0,200 ); break;
				case 2: color = SPixel8888( 0,0,100 ); break;
				default: color = SPixel8888( 0,0,50 ); break;
				}
			}
			else
			{
				ASSERT( p.nMRU == 0 );
				color = SPixel8888( 255,0,0 );
			}
			const NCache::CFibElement &fe = p.place;
			const int N_XSTEP = 1;
			const int N_YSTEP = 1;
			int nPerScanline = tl.GetXSize() / N_XSTEP;
			for ( int i = 0; i < NCache::fib( fe.nSize ); ++i )
			{
				int n = fe.nShift + i, x1 = ( n & (nPerScanline-1) ) * N_XSTEP, y1 = (n/nPerScanline) * N_YSTEP;
				for ( int y = 0; y < N_YSTEP; ++y )
				{
					for ( int x = 0; x < N_XSTEP; ++x )
						tl[y1+y][x1+x] = color;
				}
			}
		}
	}

	unsigned char* Lock() { pDX->Lock( dwNextLockFlags ); dwNextLockFlags = dwLockFlags; return pDX->pLocked; }
	void Unlock() { pDX->Unlock(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TBuffer, class TDXBuffer, class TUserObject>
class CLinearBufferElement : 
	public NCache::CGatherElementBase<NCache::CShortPtrAllocator,NCache::CFibElement, TUserObject>,
	public ILinearBuffer
{
	OBJECT_NOCOPY_METHODS(CLinearBufferElement);
public:
	CPtr<TBuffer> pBuffer;
	int nStart, nBufSize;
	int nSize;
	int nLocked;

	CLinearBufferElement( TBuffer *_p = 0 ) : pBuffer(_p), nLocked(0) {}
	~CLinearBufferElement() { while ( nLocked-- ) pBuffer->Unlock(); }
	virtual int GetFormatID() const { return pBuffer->GetFormatID(); }
	virtual void SetSize( int _nSize ) { ASSERT( _nSize <= nBufSize ); nSize = _nSize; }
	virtual int GetSize() const { return nSize; }
	virtual int GetBufSize() const { return nBufSize; }
	virtual void* Lock()
	{
		++nLocked;
		return pBuffer->Lock() + pBuffer->GetStride() * nStart;
	}
	virtual void Unlock() { if ( nLocked == 0 ) return; --nLocked; pBuffer->Unlock(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUserGeometry;
typedef CLinearBuffer<CVB, CUserGeometry> CGeometryBuffer;
class CUserGeometry : 
	public CLinearBufferElement<CGeometryBuffer, CVB, CUserGeometry>,
	public CGeometry
{
	OBJECT_NOCOPY_METHODS(CUserGeometry);
	typedef CLinearBufferElement<CGeometryBuffer, CVB, CUserGeometry> TParent;
public:
	CUserGeometry( CGeometryBuffer *_p = 0 ) : TParent(_p) {}
	virtual void* GetVertexStream() { return pBuffer; }
	virtual int GetVBStart() const { return nStart; }
	virtual int GetVBSize() const { return nSize; }
	virtual void DoTouch()
	{
		ASSERT( IsValid( this ) );
		if ( Touch() )
			pBuffer->WasTouched( this );
	}
	virtual void SetVertexStream() 
	{
		ASSERT( IsValid( this ) );
		RealSetVertexStream( pBuffer->GetBuffer()->obj, pBuffer->GetStride() );
	}
	virtual int GetGeometryFormatID() { return GetFormatID(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTriListWrapper;
class CTriListWrapperHandle
{
	int nSlowCheck;
	list<CMObj<CTriListWrapper> > wrappers;
public:
	template<class T>
	CTriListWrapper* NewWrapper( T *pThis, int nTris )
	{
		if ( ( (++nSlowCheck) & 0xff ) == 0 && !wrappers.empty() )
			EraseInvalidRefs( &wrappers );
		ASSERT( nTris <= pThis->GetSize() );
		CTriListWrapper *pRes = new CTriListWrapper( pThis, nTris ); 
		wrappers.push_back( pRes ); 
		return pRes;  
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTriListCore: public ILinearBuffer, public CTriList, public CTriListWrapperHandle
{
	OBJECT_NOCOPY_METHODS(CTriListCore);
	vector<S3DTriangle> data;
	int nSize;
public:
	CTriListCore( int _nSize = 0 ) : data(_nSize), nSize(_nSize) {}
	virtual int GetFormatID() const { return S3DTriangle::ID; }
	virtual void SetSize( int _nSize ) { ASSERT( _nSize <= data.size() ); nSize = _nSize; }
	virtual int GetSize() const { return nSize; }
	virtual int GetBufSize() const { return data.size(); }
	virtual void* Lock() { return &data[0]; }
	virtual void Unlock() {}

	virtual const vector<S3DTriangle>& GetTris() const { return data; }
	virtual int GetTrisNumber() const { return GetSize(); }
	virtual CTriListWrapper* CreateWrapper( int nTris ) { return NewWrapper( this, nTris ); }
	void DrawPrimitive( int nVBStart, int nVBSize ) { ASSERT( 0 ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTriListCore16;
typedef CLinearBuffer<CIB16, CTriListCore16> CIndicesBuffer;
class CTriListCore16: 
	public CLinearBufferElement<CIndicesBuffer, CIB16, CTriListCore16>,
	public CTriList, public CTriListWrapperHandle
{
	OBJECT_NOCOPY_METHODS(CTriListCore16);
	typedef CLinearBufferElement<CIndicesBuffer, CIB16, CTriListCore16> TParent;
public:
	CTriListCore16( CIndicesBuffer *_pBuf = 0 ) : TParent( _pBuf ) {}
	virtual const vector<S3DTriangle>& GetTris() const { ASSERT(0); return *(vector<S3DTriangle>*)0; }
	virtual int GetTrisNumber() const { ASSERT(0); return 0; }
	virtual CTriListWrapper* CreateWrapper( int nTris ) { return NewWrapper( this, nTris ); }
	void RealDrawPrimitive( int nVBStart, int nVBSize, int _nTris )
	{
		if ( bWasLinearBufferLock )
			FreeLinearBuffers();
		if ( Touch() )
			pBuffer->WasTouched( this );
		renderStats.nVertices += nVBSize;
		renderStats.nTris += _nTris;
		pDevice->SetIndices( pBuffer->GetBuffer()->obj, nVBStart );
		HRESULT hRes = pDevice->DrawIndexedPrimitive( 
			D3DPT_TRIANGLELIST, 
			0, nVBSize,
			nStart * 3,
			_nTris
			);
		ASSERT( hRes == D3D_OK );
	}
	virtual void DrawPrimitive( int nVBStart, int nVBSize ) { RealDrawPrimitive( nVBStart, nVBSize, nSize ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTriListWrapper: public CTriList, public ISomeBuffer
{
	OBJECT_BASIC_METHODS( CTriListWrapper );

	CPtr<ISomeBuffer> pParent;
	int nTris;
public:
	CTriListWrapper() {}
	CTriListWrapper( ISomeBuffer *_p, int _nTris ): pParent(_p), nTris(_nTris) {}
	virtual const vector<S3DTriangle>& GetTris() const { return dynamic_cast<CTriList*>(pParent.GetPtr())->GetTris(); }
	virtual int GetTrisNumber() const { return nTris; }
	int GetFormatID() const { ASSERT( IsValid( pParent ) ); return pParent->GetFormatID(); }
	void SetSize( int _nSize ) { ASSERT( IsValid( pParent ) && _nSize <= pParent->GetSize() ); nTris = _nSize; }
	int GetSize() const { return nTris; }
	int GetBufSize() const { ASSERT( IsValid( pParent ) ); return pParent->GetBufSize(); }
	virtual CTriListWrapper* CreateWrapper( int nTris ) { return dynamic_cast<CTriListCore16*>(pParent.GetPtr())->CreateWrapper( nTris ); }
	virtual void DrawPrimitive( int nVBStart, int nVBSize ) { dynamic_cast<CTriListCore16*>(pParent.GetPtr())->RealDrawPrimitive( nVBStart, nVBSize, nTris ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURES support classes
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTB: public CObjectBase
{
	OBJECT_BASIC_METHODS(CTB);
public:
	NWin32Helper::com_ptr<IDirect3DTexture8> obj;
	//
	CTB() {}
	CTB( int _nXSize, int _nYSize, int _nLevels, D3DFORMAT _format, ETextureUsage usage )
		:format(_format), nXSize(_nXSize), nYSize(_nYSize), nLevels(_nLevels)
	{
		HRESULT hRes = pDevice->CreateTexture( 
			_nXSize, 
			_nYSize, 
			_nLevels, 
			usage == REGULAR ? 0 : D3DUSAGE_RENDERTARGET,
			_format, 
			usage == REGULAR ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT,
			obj.GetAddr() );
		if ( hRes == D3DERR_OUTOFVIDEOMEMORY )
			ASSERT( 0 );
		ASSERT( D3D_OK == hRes );
	}
	
	int GetXSize() const { return nXSize; }
	int GetYSize() const { return nYSize; }
	D3DFORMAT GetFormat() const { return format; }
	int GetNumMipLevels() const { return nLevels; }
	int GetRawSize() const
	{
		int nRes = 0;
		for ( int k = 0; k < nLevels; ++k )
			nRes += nXSize * nYSize * D3DFormat2PixelBitSize( format ) >> (k*2);
		return nRes / 8;
	}
private:
	int nXSize, nYSize, nLevels;
	D3DFORMAT format;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
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
	CSysTexture( D3DFORMAT _format )
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
	void MarkBusy() { bBusy = true; }
	void Free() { bBusy = false; }
	bool IsBusy() const { return bBusy; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// class for ring usage of system memory buffers for texture update operations
class CSurfaceRing: public CObjectBase
{
	OBJECT_NOCOPY_METHODS( CSurfaceRing );
public:
	vector< CObj<CSysTexture> > textures;
	int nCurrent;
	//
	CSurfaceRing() { ASSERT( 0 ); }
	CSurfaceRing( D3DFORMAT _format, int nSize );
	void Switch() { nCurrent = ( nCurrent + 1 ) % textures.size(); }
	CSysTexture* GetTexture();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CSurfaceRing::CSurfaceRing( D3DFORMAT _format, int nSize )
{
	textures.resize(0);
	textures.reserve( nSize );
	for ( int i = 0; i < nSize; i++ )
		textures.push_back( new CSysTexture( _format ) );
	nCurrent = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CSysTexture* CSurfaceRing::GetTexture()
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
////////////////////////////////////////////////////////////////////////////////////////////////////
// class responsible for texture updates
class CTextureLocker: public I2DBufferLock
{
	NWin32Helper::com_ptr<IDirect3DSurface8> pObj;
	EAccess access;
	CPtr<CSysTexture> pLocker;
	D3DLOCKED_RECT buf;
	CTRect<int> rect;
public:
	// rect is specified for level 0
	CTextureLocker( IDirect3DSurface8 *_pObj, const CTRect<int> &_rect, EAccess _access, D3DFORMAT format );
	~CTextureLocker();
	virtual void* GetBuffer() { return buf.pBits; }
	virtual int GetStride() { return buf.Pitch; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SD3DFormatHash
{
	int operator()( D3DFORMAT f ) const { return (int)f; }
};
typedef hash_map<D3DFORMAT, CPtr<CSurfaceRing>, SD3DFormatHash > CFormatRingMap;
static CFormatRingMap sysTextures;
////////////////////////////////////////////////////////////////////////////////////////////////////
CTextureLocker::CTextureLocker( IDirect3DSurface8 *_pObj, const CTRect<int> &_rect, EAccess _access, D3DFORMAT format )
: pObj(_pObj), rect(_rect), access( _access )
{
	static int nShitBuffer[1024];
	HRESULT hr;
	NWin32Helper::com_ptr<IDirect3DSurface8> pTempBuf;
	//
	DWORD dwLockFlags = 0;//D3DLOCK_NO_DIRTY_UPDATE;
	CTRect<int> lockRect;
	if ( access != INPLACE && access != INPLACE_READONLY )
	{
		lockRect.SetRect( 0, 0, rect.Width(), rect.Height() );
		CFormatRingMap::iterator i = sysTextures.find( format );
		ASSERT( i != sysTextures.end() );
		pLocker = i->second->GetTexture();
		pLocker->MarkBusy();
		ASSERT( rect.Width() <= N_SYSMEM_TEXTURE_SIZE );
		ASSERT( rect.Height() <= N_SYSMEM_TEXTURE_SIZE );
		//
		pTempBuf = pLocker->pSurface;
		if ( pDevice && pObj && access != WRITEONLY )
		{
			hr = pDevice->CopyRects( pObj, (RECT*)&rect, 1, pTempBuf, 0 );
			ASSERT( D3D_OK == hr );
		}
	}
	else
	{
		if ( access == INPLACE_READONLY )
			dwLockFlags |= D3DLOCK_READONLY;
		pTempBuf = pObj;
		lockRect = rect;
	}
	if ( pTempBuf )
	{
		hr = pTempBuf->LockRect( &buf, (RECT*)&lockRect, dwLockFlags );
		ASSERT( D3D_OK == hr );
	}
	else
	{
		ASSERT(0); // texture to be locked is unavailable
		buf.pBits = nShitBuffer;
		buf.Pitch = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTextureLocker::~CTextureLocker()
{
	NWin32Helper::com_ptr<IDirect3DSurface8> pTempBuf;
	if ( access == INPLACE || access == INPLACE_READONLY )
	{
		pTempBuf = pObj;
		if ( pTempBuf )
		{
			pTempBuf->UnlockRect();
			// mark as dirty changed regions (happens automatically)
			// if ( access == INPLACE ) pObj->obj->AddDirtyRect( 0 );
		}
	}
	else
	{
		ASSERT( pLocker != 0 );
		pLocker->Free();
		if ( IsValid( pLocker ) )
		{
			pTempBuf = pLocker->pSurface;
			pTempBuf->UnlockRect();
			if ( pDevice && pObj && access != READONLY )
			{
				CTRect<int> copyRect;
				copyRect.SetRect( 0, 0, rect.Width(), rect.Height() );
				POINT dst;
				dst.x = rect.x1;
				dst.y = rect.y1;
				HRESULT hRes = pDevice->CopyRects( 
					pTempBuf, 
					(RECT*)&copyRect,
					1,
					pObj,
					(POINT*)&dst );
				ASSERT( hRes == D3D_OK );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTexture: public I2DBuffer, 
	public NCache::CGatherElementBase<NCache::CShortPtrAllocator,NCache::CQuadTreeElement, CTexture>
{
	typedef NCache::CGatherElementBase<NCache::CShortPtrAllocator,NCache::CQuadTreeElement, CTexture> CBase;
	OBJECT_BASIC_METHODS(CTexture);
	int nFrameUsed;
public:
	CPtr<CTB> pTB;
	CTRect<int> region;
	EWrap wrap;
	bool bPointFiltered;
	//
	bool Touch() { bool bRes = CBase::Touch(); nFrameUsed = nCurrentFrame; return bRes; }
	CTexture() : bPointFiltered(false) {}
	CTexture( CTB *_pTB, EWrap _wrap ): pTB(_pTB), nFrameUsed(0), wrap(_wrap), bPointFiltered(false) { region.SetRect(0, 0, _pTB->GetXSize(), _pTB->GetYSize() ); }
	//CTexture( int _nXSize, int _nYSize, int nLevels, D3DFORMAT _format, DWORD dwUsage );
	virtual int GetPixelID() { return D3DFormat2PixelID( pTB->GetFormat() ); }
	virtual I2DBufferLock* Lock( int nLevel, EAccess access );
	virtual int GetXSize() const { return region.Width(); }///pTB->GetXSize(); }
	virtual int GetYSize() const { return region.Height(); }//pTB->GetYSize(); }
	virtual int GetNumMipLevels() const { return pTB->GetNumMipLevels(); }
	virtual int GetFrameMRU() const { return nFrameUsed; }
	virtual void UserTouch() { Touch(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
I2DBufferLock* CTexture::Lock( int nLevel, EAccess access ) 
{
	NWin32Helper::com_ptr<IDirect3DSurface8> pSurface;
	pTB->obj->GetSurfaceLevel( nLevel, pSurface.GetAddr() );
	CTRect<int> rect = region;
	rect.x1 >>= nLevel;
	rect.y1 >>= nLevel;
	rect.x2 >>= nLevel;
	rect.y2 >>= nLevel;
	return new CTextureLocker( pSurface, rect, access, pTB->GetFormat() ); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetSurface( CTexture *pTexture, int nLevel, NWin32Helper::com_ptr<IDirect3DSurface8> *pRes )
{
	ASSERT( IsValid( pTexture ) );
	pTexture->Touch();
	pTexture->pTB->obj->GetSurfaceLevel( nLevel, pRes->GetAddr() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsWrapped( CTexture *pTex )
{
	return pTex->wrap == WRAP;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCubeTexture: public ICubeBuffer
{
	OBJECT_BASIC_METHODS(CCubeTexture);
	int nSize, nMipLevels;
	D3DFORMAT format;
public:
	NWin32Helper::com_ptr<IDirect3DCubeTexture8> obj;
	//
	CCubeTexture() {}
	CCubeTexture( int _nSize, int _nMipLevels, D3DFORMAT _format, ETextureUsage usage )
		:format(_format), nSize(_nSize), nMipLevels(_nMipLevels)
	{
		if ( bNoCubeMapMipLevels )
			nMipLevels = 1;

		HRESULT hRes = pDevice->CreateCubeTexture( 
			_nSize,
			nMipLevels,	//_nLevels, 
			0, //usage == REGULAR ? 0 : D3DUSAGE_RENDERTARGET,
			_format, 
			D3DPOOL_MANAGED, //usage == REGULAR ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT,
			obj.GetAddr() );
		if ( hRes == D3DERR_OUTOFVIDEOMEMORY )
			ASSERT( 0 );
		ASSERT( D3D_OK == hRes );
	}
	virtual int GetPixelID() { return D3DFormat2PixelID( format ); }
	virtual I2DBufferLock* Lock( EFace face, int nLevel, EAccess access );
	virtual int GetSize() const { return nSize; }
	virtual int GetNumMipLevels() const { return nMipLevels; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
I2DBufferLock* CCubeTexture::Lock( EFace face, int nLevel, EAccess access ) 
{
	NWin32Helper::com_ptr<IDirect3DSurface8> pSurface;
	D3DCUBEMAP_FACES f;
	switch ( face )
	{
		case POSITIVE_X: f = D3DCUBEMAP_FACE_POSITIVE_X; break;
		case POSITIVE_Y: f = D3DCUBEMAP_FACE_POSITIVE_Y; break;
		case POSITIVE_Z: f = D3DCUBEMAP_FACE_POSITIVE_Z; break;
		case NEGATIVE_X: f = D3DCUBEMAP_FACE_NEGATIVE_X; break;
		case NEGATIVE_Y: f = D3DCUBEMAP_FACE_NEGATIVE_Y; break;
		case NEGATIVE_Z: f = D3DCUBEMAP_FACE_NEGATIVE_Z; break;
		default: ASSERT( 0 );
	}
	obj->GetCubeMapSurface( f, nLevel, pSurface.GetAddr() );
	CTRect<int> rect( 0, 0, nSize >> nLevel, nSize >> nLevel );
	return new CTextureLocker( pSurface, rect, access, format ); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTextureCache
{
	typedef NCache::CGatheringCache<NCache::CShortPtrAllocator, NCache::CQuadTreeElement, CTexture> CCache;
	CObj<CCache> pCache;
	CPtr<CTB> pBuffer;
	CObj<CTexture> pTexture;

public:
	void Init( CTB *_pBuffer )
	{
		pBuffer = _pBuffer;
		if ( pBuffer )
			pTexture = new CTexture( pBuffer, CLAMP );
		else
			pTexture = 0;
		pCache = 0;
		if ( !pBuffer )
			return;
		pCache = new CCache( nCurrentFrame );
		NCache::CQuadTreeElement root;
		root.nXSize = GetMSB( pBuffer->GetXSize() - 1 ) + 1;
		root.nYSize = GetMSB( pBuffer->GetYSize() - 1 ) + 1;
		root.nShiftX = 0;
		root.nShiftY = 0;
		pCache->AddRoot( root );
	}
	void NextFrame( bool bOnThrashing )
	{
		if ( IsValid( pCache ) )
			pCache->AdvanceFrameCounter();
	}
	CTB* GetTB() const { return pBuffer; }
	CTexture* GetTexture() const { return pTexture; }
	bool IsThrashing() const { return IsValid( pCache ) && pCache->IsThrashing(); }
	CTexture* Alloc( int nXSize, int nYSize )
	{
		if ( !IsValid(pCache) )
			return 0;
		NCache::CQuadTreeElement elem;
		elem.nXSize = GetMSB( nXSize - 1 ) + 1;
		elem.nYSize = GetMSB( nYSize - 1 ) + 1;
		CCache::SCachePlace place;
		if ( !pCache->GetPlace( elem, &place ) )
			return 0;
		CTexture *pRes = new CTexture( pBuffer, CLAMP );
		pCache->PerformAlloc( pRes, &place );
		pRes->region.SetRect( 
			place.resPlace.nShiftX, place.resPlace.nShiftY,
			place.resPlace.nShiftX + nXSize, place.resPlace.nShiftY + nYSize
			);
		//pRes->Touch();
		return pRes;
	}
	void DrawRU()
	{
		CTextureLock<SPixel8888> tl( pTexture, 0, INPLACE );
		vector<CCache::SStatePlace> places;
		pCache->GetState( &places );
		for ( int y = 0; y < tl.GetYSize(); ++y )
			for ( int x = 0; x < tl.GetXSize(); ++x )
				tl[y][x] = SPixel8888( 255,255,255 );
		for ( int k = 0; k < places.size(); ++k )
		{
			const CCache::SStatePlace &p = places[k];
			SPixel8888 color;
			if ( p.pUser )
			{
				switch ( pCache->GetCurrentRU() - p.nMRU )
				{
				case 0: color = SPixel8888( 0,0,255 ); break;
				case 1: color = SPixel8888( 0,0,200 ); break;
				case 2: color = SPixel8888( 0,0,100 ); break;
				default: color = SPixel8888( 0,0,50 ); break;
				}
			}
			else
			{
				ASSERT( p.nMRU == 0 );
				color = SPixel8888( 255,0,0 );
			}
			const NCache::CQuadTreeElement &te = p.place;
			if ( ( (te.nShiftX >> te.nXSize ) + (te.nShiftY >> te.nYSize ) ) & 1 )
				color.g = 30;
			for ( int y = te.nShiftY; y < te.nShiftY + (1<<te.nYSize); ++y )
			{
				for ( int x = te.nShiftX; x < te.nShiftX + (1<<te.nXSize); ++x )
					tl[y][x] = color;
			}
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTextureBuffersSet
{
	struct STex
	{
		CPtr<CTB> pTB;
		CMObj<CTexture> pTexture;

		STex( CTB *_pTB ): pTB(_pTB) {}
	};
	list<STex> textures;
public:
	void Init( int nSize, int nNumber )
	{
		for ( int i = 0; i < nNumber; ++i )
			textures.push_back( STex( new CTB( nSize, nSize, 1, PixelID2D3DFormat( SPixel8888::ID ), TARGET ) ) );
	}
	void Clear() { textures.clear(); }
	void Walk()
	{
		for ( list<STex>::iterator i = textures.begin(); i != textures.end(); ++i )
		{
			ASSERT( IsValid( i->pTB ) );
			if ( !IsValid( i->pTexture ) )
				i->pTexture = 0;
		}
	}
	CTexture* Alloc()
	{
		// pick best
		NCache::MRU_TYPE nBest = nCurrentFrame - 1; //MRU_LAST;
		STex *pBest = 0;
		for ( list<STex>::iterator i = textures.begin(); i != textures.end(); ++i )
		{
			if ( !IsValid( i->pTexture ) )
			{
				pBest = &(*i);
				break;
			}
			NCache::MRU_TYPE nTest = i->pTexture->GetFrameMRU();
			if ( nTest < nBest )
			{
				pBest = &(*i);
				nBest = nTest;
			}
		}
		if ( pBest == 0 )
			return 0;
		CTexture *pRes = new CTexture( pBest->pTB, CLAMP );
		pBest->pTexture = pRes;
		pRes->Touch();
		return pRes;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct S32Triangle
{
	int n1, n2, n3;
};
const int N_TRIS_BUFFER_SIZE = 65536; // in bytes
const int N_MIN_BATCH_SIZE = 500;
class CDynamicTrisIndices
{
	enum EPrim
	{
		TRILIST,
		LINESTRIP
	};
	CObj<CIB> pDynamicTrisBuffer;
	int nLast, nBuf;
	int nMaxVBIndex; // fake to keep DX happy
	int nStart; // start of queued triangles and number of them
	EPrim currentPrim;


public:
	void Clear() { pDynamicTrisBuffer = 0; nLast = 0; nBuf = 0;}
	CDynamicTrisIndices() { Clear(); }
	void Init() 
	{
		int nBufSize = N_TRIS_BUFFER_SIZE;
		int n = nBufSize / 4 / 3;
		n = n & (~1);
		nLast = n;
		nBuf = n;
		pDynamicTrisBuffer = new CIB( nBufSize, TBU_DYNAMIC );
		nMaxVBIndex = 0;
		nStart = n;
		//nQueuedTris = 0;
		currentPrim = TRILIST;
	}
	void AddPrimitiveGeometry( const S3DTriangle *pSrcTris, int nTris, int nVBStart, int nVBSize, EPrim _prim = TRILIST )
	{
		if ( _prim != currentPrim )
		{
			FlushPrimitive();
			currentPrim = _prim;
		}
		int nSrcStart = 0;//, nSrcInfo = 0;
		nMaxVBIndex = Max( nMaxVBIndex, nVBSize + nVBStart );
		while ( nTris > 0 )
		{
		// feed into current buffer until full
		//{
			int nToDraw = Min( nBuf - nLast, nTris );
			DWORD dwFlags = D3DLOCK_NOOVERWRITE;
			if ( nToDraw == 0 )//nBuf - nLast < nToDraw )
			{
				FlushPrimitive();
				nMaxVBIndex = nVBSize + nVBStart;
				nLast = 0;
				nStart = 0;
				dwFlags = D3DLOCK_DISCARD;
				nToDraw = Min( nBuf - nLast, nTris );
			}
			pDynamicTrisBuffer->Lock( dwFlags );
			S32Triangle *pTri = (S32Triangle*)pDynamicTrisBuffer->pLocked;
			pTri += nLast;
			// fill tris from source
			for ( int k = 0; k < nToDraw; ++k, ++pTri )
			{
				const S3DTriangle &src = pSrcTris[ k + nSrcStart ];
				pTri->n1 = ((int)src.i1) + nVBStart;
				pTri->n2 = ((int)src.i2) + nVBStart;
				pTri->n3 = ((int)src.i3) + nVBStart;
			}
			pDynamicTrisBuffer->Unlock();
			nLast += nToDraw;
			nSrcStart += nToDraw;
			nTris -= nToDraw;
		}
	}
	void AddLinestrip( int nVBStart, int nVBSize )
	{
		int nLines = nVBSize - 1;
		int nPacks = ( nLines + 2 ) / 3; // number of 3 line packs
		int nRem = nPacks * 3 - nLines;
		vector<S3DTriangle> tris( nPacks * 2 );
		WORD *p = (WORD*)&tris[0];
		for ( int k = 0; k < nRem * 2; ++k )
			*p++ = 0;
		for ( int k = 0; k < nLines; ++k )
		{
			*p++ = k; 
			*p++ = k + 1;
		}
		ASSERT( p == ((WORD*)&tris[0]) + nPacks * 6 );
		AddPrimitiveGeometry( &tris[0], tris.size(), nVBStart, nVBSize, LINESTRIP );
	}
	void FlushPrimitive()
	{
		if ( bWasLinearBufferLock )
			FreeLinearBuffers();
		if ( nLast - nStart > 0 )
		{
			HRESULT hr;
			switch ( currentPrim )
			{
				case TRILIST:
					hr = pDevice->DrawIndexedPrimitive( 
						D3DPT_TRIANGLELIST, 
						0, nMaxVBIndex,
						nStart * 3,
						nLast - nStart
						);
					ASSERT( hr == D3D_OK );
					break;
				case LINESTRIP:
					hr = pDevice->DrawIndexedPrimitive( 
						D3DPT_LINELIST, 
						0, nMaxVBIndex,
						nStart * 3,
						( nLast - nStart ) * 3 / 2
						);
					ASSERT( hr == D3D_OK );
					break;
				default:
					ASSERT(0);
					break;
			}
		}
		//nTris -= nToDraw;
		nLast = ( nLast + 1 ) & ~1;
		if ( nBuf - nLast < N_MIN_BATCH_SIZE )
			nLast = nBuf;
		nMaxVBIndex = 0;
		nStart = nLast;
	}
	CIB* GetBuffer() { return pDynamicTrisBuffer; }
};
struct SGeometryType
{
	EBufferUsage usage;
	int nID;
	SGeometryType( EBufferUsage _usage, int _nID ) : usage(_usage), nID(_nID) {}
	bool operator==( const SGeometryType &a ) const { return usage == a.usage && nID == a.nID; }
};
struct SGeometryTypeHash
{
	int operator()( const SGeometryType &s ) const { return s.usage ^ s.nID; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// all DX buffers
static list< CMObj<CObjectBase> > lostable;
static list< CMObj<CObjectBase> > managed;
typedef hash_map<CPtr<CTB>, CObj<CTexture>, SPtrHash> CTexContainerHash;
static CTexContainerHash texContainers;
typedef hash_map<int, CTextureBuffersSet> CRTCache;
static CRTCache rtCache;
static CTextureCache textureCache, transparentCache;
typedef hash_map<SGeometryType, CObj<CGeometryBuffer>,SGeometryTypeHash > CGeometryCacheHash;
static CObj<CIndicesBuffer> pIndexBuffer;
static CGeometryCacheHash geometries;
static CDynamicTrisIndices dynamicTris;
static NWin32Helper::com_ptr<IDirect3DVertexBuffer8> pCurrentVB;
static CObj<CTexture> pLinearBufferMRU;
int nCurrentFrame = NCache::N_START_RU;
////////////////////////////////////////////////////////////////////////////////////////////////////
static void OnThrashing()
{
	dynamicTris.FlushPrimitive();
	for ( int k = 0; k < N_MAX_PRESENTS_IN_QUEUE + 1; ++k )
		NextFrameBuffes( true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
T* AddLostable( T *p )
{
	lostable.push_back( p );
	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
T* AddManaged( T *p )
{
	managed.push_back( p );
	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DestroyLostableBuffers()
{
	FreeLinearBuffers();
	pLinearBufferMRU = 0;
	lostable.clear();
	lockableList.clear();
	texContainers.clear();
	dynamicTris.Clear();
	geometries.clear();
	pIndexBuffer = 0;
	pCurrentVB = 0;
	sysTextures.clear();
	rtCache.clear();
	pDevice->SetIndices( 0, 0 );
	pDevice->SetStreamSource( 0, 0, 4 );
	for ( int i = 0; i < 8; i++ )
		pDevice->SetTexture( i, 0 );
	transparentCache.Init( 0 );
	textureCache.Init( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DestroyManagedBuffers()
{
	managed.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool bFillTransp = false, bFill2d = false;
void NextFrameBuffes( bool bOnThrashing )
{
	// render caches RU
	if ( bFill2d )
		textureCache.DrawRU();
	if ( bFillTransp )
		transparentCache.DrawRU();

	++nCurrentFrame;
	EraseInvalidRefs( &lostable );
	EraseInvalidRefs( &managed );
	for ( CRTCache::iterator i = rtCache.begin(); i != rtCache.end(); ++i )
		i->second.Walk();
	for ( CGeometryCacheHash::iterator i = geometries.begin(); i != geometries.end(); ++i )
		i->second->NextFrame( bOnThrashing );
	if ( IsValid(pIndexBuffer) )
		pIndexBuffer->NextFrame( bOnThrashing );
	textureCache.NextFrame( bOnThrashing );
	transparentCache.NextFrame( bOnThrashing );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTexture* GetTextureContainer( CTexture *pTex, STexturePlaceInfo *pPlace )
{
	if ( !IsValid( pTex ) )
		return 0;
	pTex->Touch();
	pPlace->place = pTex->region;
	pPlace->size.x = pTex->pTB->GetXSize();
	pPlace->size.y = pTex->pTB->GetYSize();
	if ( pPlace->IsWhole() )
		return pTex;
	CTexContainerHash::iterator i = texContainers.find( pTex->pTB );
	if ( i != texContainers.end() && IsValid( i->second ) )
		return i->second;
	CTexture *pRes = new CTexture( pTex->pTB, CLAMP );
	texContainers[pTex->pTB] = pRes;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void AddGeometryCache( int nSize, EBufferUsage usage, ETrueBufferUsage trueUsage, T *p = 0 )
{
	geometries[SGeometryType(usage,T::ID)] = new CGeometryBuffer( nSize, sizeof(T), T::ID, trueUsage );
}
void InitBuffers()
{
	textureCache.Init( new CTB( 1024, 1024, 1, PixelID2D3DFormat(SPixel8888::ID), REGULAR ) );
	transparentCache.Init( new CTB( 1024, 1024, 4, PixelID2D3DFormat(SPixel8888::ID), REGULAR ) );
	// CRAP number of textures should be specified somehow from outside
	if ( !bTnLDevice )
	{
		if ( nDepthTexResolution == 512 )
		{
			rtCache[512].Init( 512, 3 );
			rtCache[1024].Init( 1024, 2 );
		}
		else
		{
			ASSERT( nDepthTexResolution == 1024 );
			rtCache[512].Init( 512, 2 );
			rtCache[1024].Init( 1024, 3 );
		}
	}
	// buffers for dynamic textures
	sysTextures[D3DFMT_A8R8G8B8] = new CSurfaceRing( D3DFMT_A8R8G8B8, N_SYSMEM_TEXTURES );
	//sysTextures[D3DFMT_R5G6B5] = new CSurfaceRing( D3DFMT_R5G6B5, N_SYSMEM_TEXTURES );
	//trilists.Init( 220000, sizeof(S3DTriangle), S3DTriangle::ID, STATIC );
	if ( !bTnLDevice )
	{
		if ( bHardwareVP )
		{
#ifdef _MAPEDIT
			AddGeometryCache<SGeomVecFull>( 700000, STATIC, TBU_STATIC );
#else
			AddGeometryCache<SGeomVecFull>( 300000, STATIC, TBU_STATIC );
#endif
			AddGeometryCache<SGeomVecFull>( 100000, DYNAMIC, TBU_DYNAMIC );
		}
		else
			AddGeometryCache<SGeomVecFull>( 500000, STATIC, TBU_STATIC );
	}
	else
	{
		if ( bHardwareVP )
		{
			AddGeometryCache<SGeomVecT1C1>( 100000, DYNAMIC, TBU_DYNAMIC );
			AddGeometryCache<SGeomVecNT1>( 300000, STATIC, TBU_STATIC );
			AddGeometryCache<SGeomVecNT1>( 100000, DYNAMIC, TBU_DYNAMIC );
		}
		else
		{
			AddGeometryCache<SGeomVecT1C1>( 100000, STATIC, TBU_STATIC );
			AddGeometryCache<SGeomVecNT1>( 500000, STATIC, TBU_STATIC );
		//AddGeometryCache<SGeomVecFull>( 500000, DYNAMIC, TBU_SOFTWARE );
		}
	}
	if ( bBan32BitIndices )
		pIndexBuffer = new CIndicesBuffer( 200000, sizeof(S3DTriangle), S3DTriangle::ID, TBU_DYNAMIC );
	else
	{
		dynamicTris.Init();
		pDevice->SetIndices( dynamicTris.GetBuffer()->obj, 0 );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static ILinearBuffer* MakeGeometry( int nFormatID, int nSize, EBufferUsage usage )
{
	if ( !bHardwareVP )
		usage = STATIC;
	ASSERT( !bTnLDevice || nFormatID != SGeomVecFull::ID );
	ASSERT( bTnLDevice || nFormatID == SGeomVecFull::ID );
	CGeometryCacheHash::iterator i = geometries.find( SGeometryType(usage, nFormatID) );
	ASSERT( i != geometries.end() );
	return i->second->Alloc( nSize );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static ILinearBuffer* MakeTriList( int nSize )// int nSize, EBufferUsage eUsage )
{
	if ( bBan32BitIndices )
		return pIndexBuffer->Alloc( nSize );
	CTriListCore *pRes = new CTriListCore( nSize );
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ILinearBuffer* CreateBuffer( int nFormatID, int nSize, EBufferUsage usage )
{
	if ( nFormatID == S3DTriangle::ID )
		return MakeTriList( nSize );
	return MakeGeometry( nFormatID, nSize, usage );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTriList* MakeWrapper( CTriList *pSrc, int nTris )
{
	if ( IsValid( pSrc ) )
		return pSrc->CreateWrapper( nTris );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTexture* GetTextureCache()
{
	return textureCache.GetTexture();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTexture* GetTransparentTextureCache()
{
	return transparentCache.GetTexture();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool HasSameContainer( CTexture *p1, CTexture *p2 )
{
	return p1->pTB == p2->pTB;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTexture* MakeTexture( int nXSize, int nYSize, int nMipLevels, int nPixelID, ETextureUsage eUsage, EWrap wrap )
{
	if ( eUsage == REGULAR  )
	{
		ASSERT( GetNextPow2( nXSize ) == nXSize && GetNextPow2( nYSize ) == nYSize );
		CTB *pTB = new CTB( nXSize, nYSize, nMipLevels, PixelID2D3DFormat( nPixelID ), eUsage );
		return AddManaged( new CTexture( pTB, wrap ) );
	}
	if ( eUsage == TEXTURE_2D )
	{
		InformNew2DTextureAlloc();
		D3DFORMAT fmt = PixelID2D3DFormat( nPixelID );
		ASSERT( fmt == D3DFMT_A8R8G8B8 );
		return textureCache.Alloc( nXSize, nYSize );
	}
	if ( eUsage == TRANSPARENT_TEXTURE )
	{
		D3DFORMAT fmt = PixelID2D3DFormat( nPixelID );
		ASSERT( fmt == D3DFMT_A8R8G8B8 );
		return transparentCache.Alloc( nXSize, nYSize );
	}
	ASSERT( wrap == CLAMP );
	ASSERT( eUsage == TARGET );
	ASSERT( nPixelID == SPixel8888::ID );
	ASSERT( nXSize == nYSize );
	ASSERT( nMipLevels == 1 );
	ASSERT( rtCache.find( nXSize ) != rtCache.end() );
	CTexture *pRes = rtCache[nXSize].Alloc();
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTexture* MakeRenderTarget( int nXSize, int nYSize, int nPixelID )
{
	CTB *pTB = new CTB( nXSize, nYSize, 1, PixelID2D3DFormat( nPixelID ), TARGET );
	CTexture *pRes = new CTexture( pTB, CLAMP );
	return pRes;// pRes->Touch(); AddLostable( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCubeTexture* MakeCubeTexture( int nSize, int nMipLevels, int nPixelID, ETextureUsage eUsage )
{
	ASSERT( eUsage == REGULAR );
	ASSERT( GetNextPow2(nSize) == nSize );
	CCubeTexture *pRes = new CCubeTexture( nSize, nMipLevels, PixelID2D3DFormat( nPixelID ), eUsage );
	return AddManaged( pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RealSetVertexStream( IDirect3DVertexBuffer8 *pBuf, int nStride )
{
	if ( pCurrentVB != pBuf )
	{
		pCurrentVB = pBuf;
		pDevice->SetStreamSource( 0, pBuf, nStride );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SetTexture( int nStage, CTexture *pTex )
{
	//ASSERT( pTex->IsValid() );
	if ( IsValid( pTex ) )
	{
		pTex->Touch();
		HRESULT hRes = pDevice->SetTexture( nStage, pTex->pTB->obj );
		ASSERT( hRes == D3D_OK );
	}
	else
	{
		HRESULT hRes = pDevice->SetTexture( nStage, 0 );
		ASSERT( hRes == D3D_OK );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SetTexture( int nStage, CCubeTexture *pTex )
{
	if ( IsValid( pTex ) )
	{
		//pTex->Touch();
		HRESULT hRes = pDevice->SetTexture( nStage, pTex->obj );
		ASSERT( hRes == D3D_OK );
	}
	else
	{
		HRESULT hRes = pDevice->SetTexture( nStage, 0 );
		ASSERT( hRes == D3D_OK );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddPrimitiveGeometry( CGeometry *pGeom, CTriList *pTriList, int nBaseVertex, int nVertices )
{
	pGeom->DoTouch();
	ASSERT( nBaseVertex >= 0 && nVertices > 0 );
	ASSERT( nBaseVertex + nVertices <= pGeom->GetVBSize());
	if ( bBan32BitIndices )
	{
		pGeom->SetVertexStream();
		pTriList->DrawPrimitive( pGeom->GetVBStart() + nBaseVertex, nVertices );
		return;
	}
	const vector<S3DTriangle> &tris = pTriList->GetTris();
	int nTris = pTriList->GetTrisNumber();
	int nVBStart = pGeom->GetVBStart() + nBaseVertex, nVBSize = nVertices;
	dynamicTris.AddPrimitiveGeometry( &tris[0], nTris, nVBStart, nVBSize );
	renderStats.nVertices += nVBSize;
	renderStats.nTris += nTris;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddPrimitiveGeometry( CGeometry *pGeom, CTriList *pTriList )
{
	AddPrimitiveGeometry( pGeom, pTriList, 0, pGeom->GetVBSize() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddPrimitiveGeometry( CGeometry *pGeom, const STriangleList *pTris, int nCount, unsigned nMask )
{
	ASSERT( sizeof(S3DTriangle) == sizeof(STriangle) ); // is required for this function to work
	if ( bBan32BitIndices )
	{
		// slow workaround
		CObj<CTriList> pTriList;
		int nTris = 0;
		for ( int k = 0; k < nCount; ++k )
		{
			if ( nMask & (1<<k) )
				nTris += pTris[k].nTris;
		}
		{
			ASSERT( sizeof(STriangle) == sizeof(S3DTriangle) ); // used below
			CBufferLock<S3DTriangle> triBuf( &pTriList, nTris );
			nTris = 0;
			for ( int k = 0; k < nCount; ++k )
			{
				if ( ( nMask & (1<<k) ) == 0 )
					continue;
				const STriangleList &t = pTris[k];
				int nOffset = t.nBaseIndex; // actually will fuck up on buffers more then 65K
				ASSERT( nOffset < 65536 );
				for ( int i = 0; i < t.nTris; ++i )
				{
					S3DTriangle &res = triBuf[ nTris++ ];
					const STriangle &src = t.pTri[i];
					res.i1 = nOffset + src.i1;
					res.i2 = nOffset + src.i2;
					res.i3 = nOffset + src.i3;
				}
			}
		}
		pGeom->DoTouch();
		pGeom->SetVertexStream();
		pTriList->DrawPrimitive( pGeom->GetVBStart(), pGeom->GetVBSize() );
		return;
	}
	pGeom->DoTouch();
	int nVBStart = pGeom->GetVBStart(), nVBSize = pGeom->GetVBSize();
	for ( int k = 0; k < nCount; ++k )
	{
		if ( ( nMask & (1<<k) ) == 0 )
			continue;
		const STriangleList &t = pTris[k];
		int nTris = t.nTris;
		dynamicTris.AddPrimitiveGeometry( (S3DTriangle*)t.pTri, nTris, nVBStart + t.nBaseIndex, nVBSize - t.nBaseIndex );
		renderStats.nVertices += nTris * 3;//nVBSize;
		renderStats.nTris += nTris;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FlushPrimitive()
{
	dynamicTris.FlushPrimitive();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DrawLineStrip( CGeometry *pGeom )
{
	pGeom->DoTouch();
	int nVBGeomSize = pGeom->GetVBSize();
	int nVBGeomStart = pGeom->GetVBStart();
	if ( bWasLinearBufferLock )
		FreeLinearBuffers();
	if ( nVBGeomSize < 2 )
		return;
	if ( bBan32BitIndices )
	{
		HRESULT hRes = pDevice->DrawPrimitive( 
			D3DPT_LINESTRIP, 
			nVBGeomStart, nVBGeomSize - 1
			);
		ASSERT( hRes == D3D_OK );
		return;
	}
	dynamicTris.AddLinestrip( nVBGeomStart, nVBGeomSize );
//	dynamicTris.FlushPrimitive();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CopyTexture( NGfx::CTexture *_pTarget, NGfx::CTexture *_pSrc )
{
	NWin32Helper::com_ptr<IDirect3DSurface8> pSrc, pDst;
	GetSurface( _pTarget, 0, &pDst );
	GetSurface( _pSrc, 0, &pSrc );
	HRESULT hr = pDevice->CopyRects( pSrc, 0, 0, pDst, 0 );
	ASSERT( D3D_OK == hr );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CalcTouchedTextureSize()
{
	int nRes = 0;
	for ( list< CMObj<CObjectBase> >::iterator i = managed.begin(); i != managed.end(); ++i )
	{
		CDynamicCast<CTexture> pTexture( *i );
		if ( IsValid( pTexture ) && pTexture->GetFrameMRU() > nCurrentFrame - N_MAX_PRESENTS_IN_QUEUE - 2 )
			nRes += pTexture->pTB->GetRawSize();
	}
	return nRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TBuf, class TElem>
static bool IsThrashing( CLinearBuffer<TBuf,TElem> *pLinearBuffer )
{
	NCache::SStats stats;
	pLinearBuffer->CalcStats( &stats );
	return stats.bThrashing;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsGeometryThrashing()
{
	bool bRes = false;
	for ( CGeometryCacheHash::iterator i = geometries.begin(); i != geometries.end(); ++i )
		bRes |= IsThrashing( i->second.GetPtr() );
	if ( IsValid( pIndexBuffer ) )
		bRes |= IsThrashing( pIndexBuffer.GetPtr() );
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Is2DTextureThrashing()
{
	return textureCache.IsThrashing();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsTransparentThrashing()
{
	return transparentCache.IsThrashing();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CanStreamGeometry()
{
	return !bBan32BitIndices;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTexture* GetLinearBufferMRU( EBufferUsage usage )
{
	if ( !IsValid( pLinearBufferMRU ) )
		pLinearBufferMRU = MakeTexture( 1024, 1024, 1, SPixel8888::ID, REGULAR, CLAMP );
	if ( geometries.find( SGeometryType(usage,SGeomVecFull::ID) ) == geometries.end() )
		return 0;
	geometries[SGeometryType(usage,SGeomVecFull::ID)]->DrawRU( pLinearBufferMRU );
	return pLinearBufferMRU;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TBuf, class TElem>
void TypeStats( CLinearBuffer<TBuf,TElem> *pLinearBuffer )
{
	NCache::SStats stats;
	pLinearBuffer->CalcStats( &stats );
	csSystem << "size=" << stats.nFree + stats.nUsed << "; blocks=" << stats.nBlocks <<"; used=" << stats.nUsed << endl;
	csSystem << "frame=" << (int)stats.nCurrentRU << "; eldest=" << (int)stats.nEldestEntry;
	if ( stats.bThrashing )
		csSystem << " thrashing" << endl;
	else 
		csSystem << " ok" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void ShowCacheStats( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	csSystem << "index buffer" << endl;
	if ( IsValid( pIndexBuffer ) )
		TypeStats( pIndexBuffer.GetPtr() );
	for ( CGeometryCacheHash::iterator i = geometries.begin(); i != geometries.end(); ++i )
	{
		csSystem << "type " << i->first.nID;
		if ( i->first.usage == STATIC )
			csSystem << " static geometry" << endl;
		else
			csSystem << " dynamic geometry" << endl;
		TypeStats( i->second.GetPtr() );
	}
}
static void ShowFill2D( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	bFill2d = !bFill2d;
}
static void ShowFillTransp( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	bFillTransp = !bFillTransp;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(GfxBuffers)
	REGISTER_CMD( "gfx_stats", ShowCacheStats )
	REGISTER_CMD( "gfx_fill_2d", ShowFill2D )
	REGISTER_CMD( "gfx_fill_transp", ShowFillTransp )
FINISH_REGISTER
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NGfx;
BASIC_REGISTER_CLASS( CTexture )
BASIC_REGISTER_CLASS( CGeometry )
BASIC_REGISTER_CLASS( CTriList )
BASIC_REGISTER_CLASS( CCubeTexture )
