#ifndef __GDX_H_
#define __GDX_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "Geom.h"
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/*class CSkeleton
{
	std::vector<SHamatrix> bones;
public:
	const std::vector<SHamatrix>& GetBones() const { return bones; }
	void Calc();
};
*/
/////////////////////////////////////////////////////////////////////////////////////
/*
-what can be setup
 -texture warping, border color (hide in effect?)
 -zbias
 -alpha combine
 -rendertarget
 -wireframe mode
 */
namespace NGfx
{
	enum EFS
	{
		WINDOWED,
		FULL_SCREEN
	};
	enum EDynamic
	{
		DYNAMIC,
		STATIC
	};
	enum EAccess
	{
		READWRITE,
		WRITEONLY,
		READONLY,
		INPLACE,
		INPLACE_READONLY
	};
/////////////////////////////////////////////////////////////////////////////////////
// VERTEX/INDEX BUFFERS
/////////////////////////////////////////////////////////////////////////////////////
struct SColor
{
	enum { ID = 0 };
	union
	{
		struct{ unsigned char r,g,b,a; };
		DWORD dwColor;
	};
	SColor() {}
	SColor( unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 0xFF )
		: r(_r), g(_g), b(_b), a(_a) {}
};
/////////////////////////////////////////////////////////////////////////////////////
struct SColor16
{
	enum { ID = 1 };
	union
	{
		struct{ WORD r:5,g:6,b:5; };
		WORD wColor;
	};
};
/////////////////////////////////////////////////////////////////////////////////////
struct S3DTriangle
{
	enum { ID = 100 };
	WORD i1, i2, i3;

	S3DTriangle() {}
	S3DTriangle( WORD _i1, WORD _i2, WORD _i3 ): i1(_i1), i2(_i2), i3(_i3) {}
};
/////////////////////////////////////////////////////////////////////////////////////
// geometry builder represents set of points on which mesh can be created
struct SGeomVec
{
	enum { ID = 0 };
	CVec3 pos;
};
/////////////////////////////////////////////////////////////////////////////////////
struct SGeomVecT1
{
	enum { ID = 1 };
	CVec3 pos;
	CVec2 tex;
};
/////////////////////////////////////////////////////////////////////////////////////
struct SGeomVecT1C1
{
	enum { ID = 2 };
	CVec3 pos;
	SColor color;
	CVec2 tex;
};
struct SGeomVecN
{
	enum { ID = 3 };
	CVec3 pos;
	CVec3 normal;
	CVec2 tex;
};
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// базовый объект ресурса
#ifndef __GXREALISATION__
class CBufferLock;
class CBufferBase: public CObjectBase
{
public:
	int GetBufSize() const;
	int GetSize() const;
	void SetSize( int nSize );
};
/////////////////////////////////////////////////////////////////////////////////////
class CGeometry: public CBufferBase
{
	void* Lock();
	void Unlock();
protected:
	~CGeometry();
public:
	CGeometry();
	int GetFormatID() const;
	friend class CBufferLock;
};
/////////////////////////////////////////////////////////////////////////////////////
class CTriList: public CBufferBase
{
	void* Lock();
	void Unlock();
protected:
	~CTriList();
public:
	CTriList();
	friend class CBufferLock;
};
/////////////////////////////////////////////////////////////////////////////////////
class CTexture: public CObjectBase
{
protected:
	~CTexture();
public:
	CTexture();
	int GetPixelID() const;
	int GetXSize() const;
	int GetYSize() const;
};
/////////////////////////////////////////////////////////////////////////////////////
class CRenderTarget: public CObjectBase
{
protected:
	~CRenderTarget();
public:
	CRenderTarget();
	CTexture* GetTexture() const;
};
/////////////////////////////////////////////////////////////////////////////////////
#else
class CRenderTarget;
class CTexture;
#endif
/////////////////////////////////////////////////////////////////////////////////////
// базовый объект доступа, T - наследник CBase<>
class CBufferLock
{
protected:
	template <class T>
		void* Lock( T &obj ) { return obj.Lock(); }
	template <class T>
		void Unlock( T &obj ) { obj.Unlock(); }
};
/////////////////////////////////////////////////////////////////////////////////////
// Access
/////////////////////////////////////////////////////////////////////////////////////
template <class T, class TElement>
class CBufferAccess
{
protected:
	CPtr<T> pObj;
	TElement *pStart;
	typedef TElement Element;
public:
	CBufferAccess( T *p = 0 ): pObj(p) {}
	//
	void SetSize( int nSize ) { pObj->SetSize( nSize ); }
	int GetStride() const { return sizeof( Element ); }
	TElement& operator[]( int n ) const
	{
		ASSERT( n >= 0 && n < pObj->GetBufSize() );
		return ((TElement*)pStart)[n];
	}
};
/////////////////////////////////////////////////////////////////////////////////////
template<class TElement>
class CGeomAccess: public CBufferAccess< CGeometry, TElement >
{
public:
	CGeomAccess( CGeometry *p = 0 ): CBufferAccess< CGeometry, TElement >( p )
	{
		ASSERT( p == 0 || p->GetFormatID() == TElement::ID );
	}
};
typedef CBufferAccess<CTriList, S3DTriangle> CTriListAccess;
/////////////////////////////////////////////////////////////////////////////////////
// Locks
/////////////////////////////////////////////////////////////////////////////////////
template<class T, class TElement>
class CLock: public CBufferAccess<T,TElement>, public CBufferLock
{
public:
	CLock( T *p ): CBufferAccess<T,TElement>( p ) { pStart = (TElement*) Lock(*p); }
	~CLock() { Unlock( *pObj ); }
};
/////////////////////////////////////////////////////////////////////////////////////
template <class TElement>
class CGeomLock: public CLock< CGeometry, TElement >
{
public:
	CGeomLock( CGeometry *obj ): CLock< CGeometry, TElement >( obj ) {}
};
/////////////////////////////////////////////////////////////////////////////////////
typedef CLock< CTriList, S3DTriangle > CTriListLock;
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// TEXTURE
/////////////////////////////////////////////////////////////////////////////////////
class CSysTexture;
class CTextureLockBase
{
public:
	vector< CTRect<int> > dirtyRects;
	//
	CTextureLockBase( CTexture *pTexture, int _nLevel, NGfx::EAccess _access );
	~CTextureLockBase();
private:
	CPtr<CTexture> pObj;
	NGfx::EAccess access;
	CPtr<CSysTexture> pLocker;
	int nLevel;
protected:
	std::vector<void*> raws;
};
/////////////////////////////////////////////////////////////////////////////////////
template<class TPixel>
class CTextureLock: public CTextureLockBase
{
public:
	CTextureLock( CTexture *pTexture, int nLevel, NGfx::EAccess access )
		: CTextureLockBase( pTexture, nLevel, access )
	{ 
		ASSERT( pTexture->GetPixelID() == TPixel::ID ); 
	}
	//
	TPixel* operator[]( int nY ) { return (TPixel*)raws[nY]; }
};
/////////////////////////////////////////////////////////////////////////////////////
	// for MakeTempGeometry template only
	struct SEmpty {};
	CBufferAccess<CGeometry, SEmpty>& MakeTempGeometry( int nSize, int nFormatID );
/////////////////////////////////////////////////////////////////////////////////////
// general
	//
	bool SetMode( int nSizeX, int nSizeY, int nBpp, EFS fullScreen );
	void Flip();
	void ClearBuffers( DWORD dwColor = 0x808080 );
	//
	void SetWireframe( bool bWire );
	//
	CGeometry* MakeGeometry( int nSize, int nFormatID, EDynamic eDynamic );
	CTriList* MakeTriList( int nTris, EDynamic eDynamic );
	//
	template<class TRes>
	CGeomAccess<TRes>& MakeTempGeometry( int nSize, const TRes & )
	{
		sizeof(TRes);
		return *(CGeomAccess<TRes>*)&MakeTempGeometry( nSize, (int)TRes::ID );
	}
	CTriListAccess& MakeTempTriList( int nSize );
	//
	CTexture* MakeTexture( int nXSize, int nYSize, int nMipLevels );
	// BPP of rendertarget is always same as screen
	CRenderTarget* GetScreen();
	CRenderTarget* MakeRenderTarget( int nXSize, int nYSize, CRenderTarget *pZBufferSrc );
	void SetRenderTarget( CRenderTarget *pTarget = 0 );
	//
	void DrawPrimitive( const CTriList& );
	void DrawPrimitive();
	//
	bool Is3DActive();
	bool Init3D( HWND hWnd );
	void Done3D();
};
/////////////////////////////////////////////////////////////////////////////////////
#endif