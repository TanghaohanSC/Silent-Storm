#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__
/////////////////////////////////////////////////////////////////////////////////////
#include "geom.h"
/////////////////////////////////////////////////////////////////////////////////////
void MakeMatrix( SHMatrix *pRes, float fpTangazh, float fpRiskanie, const CVec3 &pos );
void InvertMatrix( const SHMatrix &pos, SHMatrix *pRes );
/////////////////////////////////////////////////////////////////////////////////////
template< int nMaxNumMatrices, class TElement>
class CBaseMatrixStack
{
protected:
	TElement matrices[nMaxNumMatrices];
	int nCurrentMatrix;
public :
	CBaseMatrixStack() : nCurrentMatrix( -1 ) {}
	//
	bool IsEmpty() const { return nCurrentMatrix == -1; }
	void Clear() { nCurrentMatrix = -1; }
	void Pop( int nAmount = 1 ) { ASSERT( nCurrentMatrix >= nAmount - 1 ); nCurrentMatrix -= nAmount; }
	const TElement& Get() const { return matrices[nCurrentMatrix]; }
	void Init( const TElement &matrix ) { matrices[nCurrentMatrix = 0] = matrix; }
};
/////////////////////////////////////////////////////////////////////////////////////
// matrix stack 4x3
// template parameter == max stack size, second parameter - is stack 4x4
template <int nMaxNumMatrices>
class CMatrixStack43: public CBaseMatrixStack<nMaxNumMatrices, SHMatrix>
{
public :
	CMatrixStack43() { for ( int i = 0; i < nMaxNumMatrices; i++ ) Identity( matrices + i ); }
	void Push43( const SHMatrix &matrix );
	void Push33( const SHMatrix &matrix );
	void Push( const CVec3 &pos );
	void Push( const CQuat &rot );
	void Push( const CVec3 &pos, const CQuat &rot );
	void PushScale( float x, float y, float z );
	void PushScale( float val );
};
/////////////////////////////////////////////////////////////////////////////////////
// matrix stack 4x4 FBTransforms
template <int nMaxNumMatrices>
class CFBMatrixStack: public CBaseMatrixStack<nMaxNumMatrices, SFBTransform>
{
protected:
	void SetToFirst() { ASSERT( nCurrentMatrix >= 0 ); nCurrentMatrix = 0; }
public :
	CFBMatrixStack() {}
	//
	void Init( const SHMatrix &matrix ) { nCurrentMatrix = 0; matrices[0].forward = matrix; InvertMatrix( matrix, &matrices[0].backward ); }
	//
	void Push43( const SHMatrix &matrix );
	void Push33( const SHMatrix &matrix );
	void Push33Orthonormal( const SHMatrix &matrix );
	void Push33( const SHMatrix &matrix, const SHMatrix &invMatrix );
	void Push( const CVec3 &pos );
	void Push( const CQuat &rot );
	void Push( const CVec3 &pos, const CQuat &rot );
	void PushScale( float x, float y, float z );
	void PushScale( float val );
};
/////////////////////////////////////////////////////////////////////////////////////
class CTransformStack: public CFBMatrixStack<8>
{
	CVec4 viewFrustrum[4];
	//
	void PrepareClipPlanes();
public:
	void SetCamera( const SHMatrix& pos );
	void MakeProjective( float fFov = 90 );
	void MakeParallel( float fWidth, float fHeight );
	bool PushClipHint( const CVec3 &center, float fpRadius );
	void PopClipHint();
};
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// some inlined functions
/////////////////////////////////////////////////////////////////////////////////////
// a&b 43
inline void Multiply43( SHMatrix *p, const SHMatrix &b, const SHMatrix &a )
{
	p->_11 = a._11*b._11 + a._12*b._21 + a._13*b._31;
	p->_12 = a._11*b._12 + a._12*b._22 + a._13*b._32;
	p->_13 = a._11*b._13 + a._12*b._23 + a._13*b._33;
	p->_14 = a._11*b._14 + a._12*b._24 + a._13*b._34 + a._14;

	p->_21 = a._21*b._11 + a._22*b._21 + a._23*b._31;
	p->_22 = a._21*b._12 + a._22*b._22 + a._23*b._32;
	p->_23 = a._21*b._13 + a._22*b._23 + a._23*b._33;
	p->_24 = a._21*b._14 + a._22*b._24 + a._23*b._34 + a._24;

	p->_31 = a._31*b._11 + a._32*b._21 + a._33*b._31;
	p->_32 = a._31*b._12 + a._32*b._22 + a._33*b._32;
	p->_33 = a._31*b._13 + a._32*b._23 + a._33*b._33;
	p->_34 = a._31*b._14 + a._32*b._24 + a._33*b._34 + a._34;
}
/////////////////////////////////////////////////////////////////////////////////////
// b 3x3 a 4x3
inline void Multiply33( SHMatrix *p, const SHMatrix &b, const SHMatrix &a )
{
	p->_11 = a._11*b._11 + a._12*b._21 + a._13*b._31;
	p->_12 = a._11*b._12 + a._12*b._22 + a._13*b._32;
	p->_13 = a._11*b._13 + a._12*b._23 + a._13*b._33;
	p->_14 = a._14;

	p->_21 = a._21*b._11 + a._22*b._21 + a._23*b._31;
	p->_22 = a._21*b._12 + a._22*b._22 + a._23*b._32;
	p->_23 = a._21*b._13 + a._22*b._23 + a._23*b._33;
	p->_24 = a._24;

	p->_31 = a._31*b._11 + a._32*b._21 + a._33*b._31;
	p->_32 = a._31*b._12 + a._32*b._22 + a._33*b._32;
	p->_33 = a._31*b._13 + a._32*b._23 + a._33*b._33;
	p->_34 = a._34;
}
/////////////////////////////////////////////////////////////////////////////////////
// b 4x3 a 4x4
inline void MultiplyF43( SHMatrix *p, const SHMatrix &b, const SHMatrix &a )
{
	p->_11 = a._11*b._11 + a._12*b._21 + a._13*b._31;
	p->_12 = a._11*b._12 + a._12*b._22 + a._13*b._32;
	p->_13 = a._11*b._13 + a._12*b._23 + a._13*b._33;
	p->_14 = a._11*b._14 + a._12*b._24 + a._13*b._34 + a._14;

	p->_21 = a._21*b._11 + a._22*b._21 + a._23*b._31;
	p->_22 = a._21*b._12 + a._22*b._22 + a._23*b._32;
	p->_23 = a._21*b._13 + a._22*b._23 + a._23*b._33;
	p->_24 = a._21*b._14 + a._22*b._24 + a._23*b._34 + a._24;

	p->_31 = a._31*b._11 + a._32*b._21 + a._33*b._31;
	p->_32 = a._31*b._12 + a._32*b._22 + a._33*b._32;
	p->_33 = a._31*b._13 + a._32*b._23 + a._33*b._33;
	p->_34 = a._31*b._14 + a._32*b._24 + a._33*b._34 + a._34;

	p->_41 = a._41*b._11 + a._42*b._21 + a._43*b._31;
	p->_42 = a._41*b._12 + a._42*b._22 + a._43*b._32;
	p->_43 = a._41*b._13 + a._42*b._23 + a._43*b._33;
	p->_44 = a._41*b._14 + a._42*b._24 + a._43*b._34 + a._44;
}
/////////////////////////////////////////////////////////////////////////////////////
// b 3x3 a 4x4
inline void MultiplyF33( SHMatrix *p, const SHMatrix &b, const SHMatrix &a )
{
	p->_11 = a._11*b._11 + a._12*b._21 + a._13*b._31;
	p->_12 = a._11*b._12 + a._12*b._22 + a._13*b._32;
	p->_13 = a._11*b._13 + a._12*b._23 + a._13*b._33;
	p->_14 = a._14;

	p->_21 = a._21*b._11 + a._22*b._21 + a._23*b._31;
	p->_22 = a._21*b._12 + a._22*b._22 + a._23*b._32;
	p->_23 = a._21*b._13 + a._22*b._23 + a._23*b._33;
	p->_24 = a._24;

	p->_31 = a._31*b._11 + a._32*b._21 + a._33*b._31;
	p->_32 = a._31*b._12 + a._32*b._22 + a._33*b._32;
	p->_33 = a._31*b._13 + a._32*b._23 + a._33*b._33;
	p->_34 = a._34;

	p->_41 = a._41*b._11 + a._42*b._21 + a._43*b._31;
	p->_42 = a._41*b._12 + a._42*b._22 + a._43*b._32;
	p->_43 = a._41*b._13 + a._42*b._23 + a._43*b._33;
	p->_44 = a._44;
}
/////////////////////////////////////////////////////////////////////////////////////
// a 4x4
inline void MultiplyFScale( SHMatrix *p, float x, float y, float z, const SHMatrix &a )
{
	p->_11 = a._11*x;
	p->_12 = a._12*y;
	p->_13 = a._13*z;
	p->_14 = a._14;

	p->_21 = a._21*x;
	p->_22 = a._22*y;
	p->_23 = a._23*z;
	p->_24 = a._24;

	p->_31 = a._31*x;
	p->_32 = a._32*y;
	p->_33 = a._33*z;
	p->_34 = a._34;

	p->_41 = a._41*x;
	p->_42 = a._42*y;
	p->_43 = a._43*z;
	p->_44 = a._44;
}
/////////////////////////////////////////////////////////////////////////////////////
// b 3x3 a 4x4
inline void MultiplyF33Inv( SHMatrix *p, const SHMatrix &b, const SHMatrix &a )
{
	p->_11 = a._11*b._11 + a._12*b._12 + a._13*b._13;
	p->_12 = a._11*b._21 + a._12*b._22 + a._13*b._23;
	p->_13 = a._11*b._31 + a._12*b._32 + a._13*b._33;
	p->_14 = a._14;

	p->_21 = a._21*b._11 + a._22*b._12 + a._23*b._13;
	p->_22 = a._21*b._21 + a._22*b._22 + a._23*b._23;
	p->_23 = a._21*b._31 + a._22*b._32 + a._23*b._33;
	p->_24 = a._24;

	p->_31 = a._31*b._11 + a._32*b._12 + a._33*b._13;
	p->_32 = a._31*b._21 + a._32*b._22 + a._33*b._23;
	p->_33 = a._31*b._31 + a._32*b._32 + a._33*b._33;
	p->_34 = a._34;

	p->_41 = a._41*b._11 + a._42*b._12 + a._43*b._13;
	p->_42 = a._41*b._21 + a._42*b._22 + a._43*b._23;
	p->_43 = a._41*b._31 + a._42*b._32 + a._43*b._33;
	p->_44 = a._44;
}
/////////////////////////////////////////////////////////////////////////////////////
inline void MultiplyTranslate( SHMatrix *p, const SHMatrix &a, const CVec3 &b )
{
	p->_11 = a._11;
	p->_12 = a._12;
	p->_13 = a._13;
	p->_14 = a._11*b.x + a._12*b.y + a._13*b.z + a._14;

	p->_21 = a._21;
	p->_22 = a._22;
	p->_23 = a._23;
	p->_24 = a._21*b.x + a._22*b.y + a._23*b.z + a._24;

	p->_31 = a._31;
	p->_32 = a._32;
	p->_33 = a._33;
	p->_34 = a._31*b.x + a._32*b.y + a._33*b.z + a._34;

	p->_41 = a._41; 
	p->_42 = a._42; 
	p->_43 = a._43; 
	p->_44 = a._41*b.x + a._42*b.y + a._43*b.z + a._44;
}
/////////////////////////////////////////////////////////////////////////////////////
inline void MultiplyInvTranslate( SHMatrix *p, const SHMatrix &a, const CVec3 &b )
{
	p->_11 = a._11 - b.x * a._14;
	p->_12 = a._12 - b.y * a._14;
	p->_13 = a._13 - b.z * a._14;
	p->_14 = a._14;

	p->_21 = a._21 - b.x * a._24;
	p->_22 = a._22 - b.y * a._24;
	p->_23 = a._23 - b.z * a._24;
	p->_24 = a._24;

	p->_31 = a._31 - b.x * a._34;
	p->_32 = a._32 - b.y * a._34;
	p->_33 = a._33 - b.z * a._34;
	p->_34 = a._34;

	p->_41 = a._41 - b.x * a._14;
	p->_42 = a._42 - b.y * a._14;
	p->_43 = a._43 - b.z * a._14;
	p->_44 = a._44;
}
/////////////////////////////////////////////////////////////////////////////////////
inline void MultiplyTranslate43( SHMatrix *p, const SHMatrix &a, const CVec3 &b )
{
	p->_11 = a._11;
	p->_12 = a._12;
	p->_13 = a._13;
	p->_14 = a._11*b.x + a._12*b.y + a._13*b.z + a._14;

	p->_21 = a._21;
	p->_22 = a._22;
	p->_23 = a._23;
	p->_24 = a._21*b.x + a._22*b.y + a._23*b.z + a._24;

	p->_31 = a._31;
	p->_32 = a._32;
	p->_33 = a._33;
	p->_34 = a._31*b.x + a._32*b.y + a._33*b.z + a._34;
}
/////////////////////////////////////////////////////////////////////////////////////
// CMatrixStack43
/////////////////////////////////////////////////////////////////////////////////////
template <int nMaxNumMatrices>
inline void CMatrixStack43<nMaxNumMatrices>::Push43( const SHMatrix &matrix ) 
{
	ASSERT( nCurrentMatrix >= 0 );
	Multiply43( &matrices[nCurrentMatrix + 1], matrix, matrices[nCurrentMatrix] );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CMatrixStack43<nMaxNumMatrices>::Push33( const SHMatrix &matrix ) 
{
	ASSERT( nCurrentMatrix >= 0 );
	Multiply33( &matrices[nCurrentMatrix + 1], matrix, matrices[nCurrentMatrix] );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CMatrixStack43<nMaxNumMatrices>::Push( const CVec3 &pos ) 
{
	ASSERT( nCurrentMatrix >= 0 );
	MultiplyTranslate43( &matrices[nCurrentMatrix + 1], matrices[nCurrentMatrix], pos );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CMatrixStack43<nMaxNumMatrices>::Push( const CQuat &rot )
{
	SHMatrix matrix;
	rot.DecompEulerMatrix( matrix );
	Push33( matrix );
}
template <int nMaxNumMatrices>
inline void CMatrixStack43<nMaxNumMatrices>::Push( const CVec3 &pos, const CQuat &rot )
{
	SHMatrix matrix;
	rot.DecompEulerMatrix( matrix );
	matrix._14 = pos.x; matrix._24 = pos.y; matrix._34 = pos.z;
	Push43( matrix );
}
template <int nMaxNumMatrices>
inline void CMatrixStack43<nMaxNumMatrices>::PushScale( float x, float y, float z )
{
	SHMatrix matrix;
	Zero( matrix );
	matrix._11 = x; matrix._22 = y; matrix._33 = z;
	Push33( matrix );
}
template <int nMaxNumMatrices>
inline void CMatrixStack43<nMaxNumMatrices>::PushScale( float val ) 
{
	PushScale( val, val, val ); 
}
/////////////////////////////////////////////////////////////////////////////////////
// CFBMatrixStack
/////////////////////////////////////////////////////////////////////////////////////
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::Push43( const SHMatrix &matrix ) 
{
	ASSERT( nCurrentMatrix >= 0 );
	MultiplyF43( &matrices[nCurrentMatrix + 1].forward, matrix, matrices[nCurrentMatrix].forward );
	InvertMatrix( matrices[nCurrentMatrix + 1].forward, &matrices[nCurrentMatrix + 1].backward );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::Push33( const SHMatrix &matrix ) 
{
	ASSERT( nCurrentMatrix >= 0 );
	MultiplyF33( &matrices[nCurrentMatrix + 1].forward, matrix, matrices[nCurrentMatrix].forward );
	InvertMatrix( matrices[nCurrentMatrix + 1].forward, &matrices[nCurrentMatrix + 1].backward );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::Push33Orthonormal( const SHMatrix &matrix ) 
{
	ASSERT( nCurrentMatrix >= 0 );
	MultiplyF33( &matrices[nCurrentMatrix + 1].forward, matrix, matrices[nCurrentMatrix].forward );
	MultiplyF33Inv( &matrices[nCurrentMatrix + 1].backward, matrix, matrices[nCurrentMatrix].backward );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::Push33( const SHMatrix &matrix, const SHMatrix &invMatrix ) 
{
	ASSERT( nCurrentMatrix >= 0 );
	MultiplyF33( &matrices[nCurrentMatrix + 1].forward, matrix, matrices[nCurrentMatrix].forward );
	MultiplyF33( &matrices[nCurrentMatrix + 1].backward, invMatrix, matrices[nCurrentMatrix].backward );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::Push( const CVec3 &pos ) 
{
	ASSERT( nCurrentMatrix >= 0 );
	MultiplyTranslate( &matrices[nCurrentMatrix + 1].forward, matrices[nCurrentMatrix].forward, pos );
	MultiplyInvTranslate( &matrices[nCurrentMatrix + 1].backward, matrices[nCurrentMatrix].backward, pos );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::Push( const CQuat &rot )
{
	SHMatrix matrix;
	rot.DecompEulerMatrix( matrix );
	Push33Orthonormal( matrix );
}
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::Push( const CVec3 &pos, const CQuat &rot )
{
	SHMatrix matrix;
	rot.DecompEulerMatrix( matrix );
	Push( pos );
	Push33Orthonormal( matrix );
}
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::PushScale( float x, float y, float z )
{
	ASSERT( nCurrentMatrix >= 0 );
	MultiplyF33( &matrices[nCurrentMatrix + 1].forward, x, y, z, matrices[nCurrentMatrix].forward );
	MultiplyF33( &matrices[nCurrentMatrix + 1].backward, 1/x, 1/y, 1/z, invMatrix, matrices[nCurrentMatrix].backward );
	nCurrentMatrix++;
}
template <int nMaxNumMatrices>
inline void CFBMatrixStack<nMaxNumMatrices>::PushScale( float val ) 
{
	PushScale( val, val, val ); 
}
/////////////////////////////////////////////////////////////////////////////////////
#endif