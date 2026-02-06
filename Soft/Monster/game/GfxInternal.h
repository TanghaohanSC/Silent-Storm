#ifndef __GDXINTERNAL_H_
#define __GDXINTERNAL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
// full description of buffers & textures for internal use & some internal data access
/////////////////////////////////////////////////////////////////////////////////////
namespace NGfx
{
// this define is required to ignore short descriptions from Gfx.h
#define __GXREALISATION__
/////////////////////////////////////////////////////////////////////////////////////
// Buffers
/////////////////////////////////////////////////////////////////////////////////////
class CBufferBase: public CObjectBase
{
public:
	CBufferBase() { nStart = 0; nBufSize = 0; nSize = 0; }
	CBufferBase( int _nBufSize, int _nStart )
		: nStart(_nStart), nBufSize(_nBufSize), nSize(_nBufSize) {}
	void Init( int _nBufSize, int _nStart ) { nStart = _nStart; nBufSize = nSize = _nBufSize; }
	int GetBufSize() const;
	int GetSize() const;
	void SetSize( int _nSize );
	int GetStart() const { return nStart; }
private:
	int nStart;       // number of first element
	int nBufSize;     // physical size in elements
	int nSize;        // logical size in elements
};
/////////////////////////////////////////////////////////////////////////////////////
class CVB;
class CGeometry: public CBufferBase
{
OBJECT_BASIC_METHODS(CGeometry);
public:
	CPtr<CVB> pDX;
	//
	CGeometry();
	CGeometry( CVB *_pDX, int _nBufSize, int _nStart, int _nFormatID, int _nStride )
		: CBufferBase(_nBufSize, _nStart ), pDX(_pDX), nStride(_nStride), nFormatID(_nFormatID) {}
	//
	int GetFormatID() const;
	int GetStride() const { return nStride; }
	void SetStride( int _nFormatID, int _nStride ) { nFormatID = _nFormatID; nStride = _nStride; }
private:
	int nFormatID;
	int nStride;
	//
	void* Lock();
	void Unlock();
};
/////////////////////////////////////////////////////////////////////////////////////
class CIB;
class CTriList: public CBufferBase
{
OBJECT_BASIC_METHODS(CTriList);
public:
	CPtr<CIB> pDX;
	//
	CTriList();
	CTriList( CIB *_pDX, int _nBufSize, int _nStart )
		: CBufferBase(_nBufSize, _nStart ), pDX(_pDX) {}
	//
	int GetFormatID() const;
	int GetStride() const { return 6; }
private:
	void* Lock();
	void Unlock();
};
/////////////////////////////////////////////////////////////////////////////////////
class CTexture: public CObjectBase
{
OBJECT_BASIC_METHODS(CTexture);
public:
	NWin32Helper::com_ptr<IDirect3DTexture8> obj;
	//
	CTexture();
	CTexture( int _nXSize, int _nYSize, int nLevels, D3DFORMAT _format, DWORD dwUsage );
	int GetPixelID() const;
	int GetXSize() const;
	int GetYSize() const;
	D3DFORMAT GetFormat() { return format; }
private:
	int nXSize, nYSize;
	D3DFORMAT format;
};
/////////////////////////////////////////////////////////////////////////////////////
// some defines reside in Gfx.h, use them
} // namespace
#include "Gfx.h"
namespace NGfx
{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// general actions linear DX buffers
/////////////////////////////////////////////////////////////////////////////////////
template< class T >
void DxLock( T &obj )//, int nOffset, int nSize )
{
	if ( ++obj.nLockCount == 1 )
	{
		obj.obj->Lock( 0, 0, &obj.pLocked, 0 );
		//obj.obj->Lock( nOffset, nSize, &obj.pLocked, 0 );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// bDiscard ? D3DLOCK_DISCARD: D3DLOCK_NOOVERWRITE
template< class T >
void DxLockDynamic( T &obj, DWORD dwFlags )//, int nOffset, int nSize, DWORD dwFlags )
{
	ASSERT( obj.nLockCount == 0 );
	if ( ++obj.nLockCount == 1 )
	{
		//HRESULT hr = obj.obj->Lock( nOffset, nSize, &obj.pLocked, dwFlags );
		HRESULT hr = obj.obj->Lock( 0, 0, &obj.pLocked, dwFlags );
		ASSERT( hr == D3D_OK );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
template< class T >
void DxUnlock( T &obj )
{
	ASSERT( obj.nLockCount > 0 );
	if ( --obj.nLockCount == 0 )
	{
		HRESULT hr = obj.obj->Unlock();
		ASSERT( hr == D3D_OK );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// common data for DX resources
template <class TDXobj>
class CRBase: public CFundament
{
public:
	NWin32Helper::com_ptr< TDXobj > obj;
	int nLockCount;
	unsigned char *pLocked;
	int nSize;  // size in bytes

	CRBase() { nLockCount = 0; }
	~CRBase() { while ( nLockCount ) DxUnlock( *this ); }
	int GetSize() const { return nSize; }
};
/////////////////////////////////////////////////////////////////////////////////////
class CVB: public CRBase<IDirect3DVertexBuffer8>
{
public:
	void Create( int _nSize, EDynamic eDynamic );
};
/////////////////////////////////////////////////////////////////////////////////////
class CIB: public CRBase<IDirect3DIndexBuffer8>
{
public:
	void Create( int _nSize, EDynamic eDynamic );
};
/////////////////////////////////////////////////////////////////////////////////////
// Vertex info
/////////////////////////////////////////////////////////////////////////////////////
struct SGeomFormatInfo
{
	int nFormatID;
	int nSize;
	DWORD *pdwVSD;
};
extern SGeomFormatInfo geometryFormatInfo[4];
/////////////////////////////////////////////////////////////////////////////////////
inline int GetGeomFormatSize( int nFormatID )
{
	ASSERT( nFormatID >= 0 && nFormatID < ARRAY_SIZE( geometryFormatInfo ) );
	ASSERT( nFormatID == geometryFormatInfo[nFormatID].nFormatID );
	return geometryFormatInfo[nFormatID].nSize;
}
/////////////////////////////////////////////////////////////////////////////////////
inline const DWORD* GetVertexLayout( int nFormatID )
{
	ASSERT( nFormatID >= 0 && nFormatID < ARRAY_SIZE( geometryFormatInfo ) );
	ASSERT( nFormatID == geometryFormatInfo[nFormatID].nFormatID );
	return geometryFormatInfo[nFormatID].pdwVSD;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// main interfaces
/////////////////////////////////////////////////////////////////////////////////////
extern NWin32Helper::com_ptr<IDirect3D8> pD3D;
extern NWin32Helper::com_ptr<IDirect3DDevice8> pDevice;
extern int nVBGeomStart, nVBGeomSize; // start & size of current vertex stream range
/////////////////////////////////////////////////////////////////////////////////////
// some callbacks called from Gfx
HRESULT EffectsInit();
void EffectsShutdown();
void TempVertexStreamChanged();
// gain access to temprorary geometry buffer
CGeometry* GetTempGeometry();
/////////////////////////////////////////////////////////////////////////////////////
} // namespace
/////////////////////////////////////////////////////////////////////////////////////
#endif