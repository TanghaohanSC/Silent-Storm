#include "StdAfx.h"
#include "Transform.h"

#include <D3Dx8.h> // CRAP, for matrix inverse
/////////////////////////////////////////////////////////////////////////////////////
void MakeMatrix( SHMatrix *pRes, float fpTangazh, float fpRiskanie, const CVec3 &pos )
{
	SHMatrix &res = *pRes;
	CQuat q, qt,qr;
	qt.FromAngleAxis( fpTangazh, CVec3(1,0,0) );
	qr.FromAngleAxis( fpRiskanie, CVec3(0,0,1) );
	q = qr*qt;
	Identity( &res );
	q.DecompEulerMatrix( res );
	res._14 = pos.x;
	res._24 = pos.y;
	res._34 = pos.z;
}
/////////////////////////////////////////////////////////////////////////////////////
void InvertMatrix( const SHMatrix &pos, SHMatrix *pRes )
{
	// CRAP - have to write our own?
	D3DXMATRIX *pOut = D3DXMatrixInverse( (D3DXMATRIX*)pRes, 0, (D3DXMATRIX*)&pos );
	ASSERT( pOut != 0 );
}
/////////////////////////////////////////////////////////////////////////////////////
// CTransformStack
/////////////////////////////////////////////////////////////////////////////////////
void CTransformStack::SetCamera( const SHMatrix& pos )
{
	SetToFirst();
	SHMatrix res, axisSwitch;
	Identity( &axisSwitch );
	axisSwitch._22 = 0;
	axisSwitch._23 = 1;
	axisSwitch._32 = 1;
	axisSwitch._33 = 0;
	Push43( axisSwitch );
	res.HomogeneousInverse( pos );
	Push43( res );
}
/////////////////////////////////////////////////////////////////////////////////////
void CTransformStack::PrepareClipPlanes()
{
	const SHMatrix &inv = Get().backward;
	for ( int i = 0; i < 4; i++ )
		inv.RotateHVectorTransposed( &viewFrustrum[i], viewFrustrum[i] );
}
/////////////////////////////////////////////////////////////////////////////////////
void CTransformStack::MakeProjective( float fFov )
{
	float h, w, Q;
	float far_plane = 10000;
	float near_plane = 1;
	fFov = ToRadian( fFov );
	w = (float)1/tan(fFov*0.5);  // 1/tan(x) == cot(x)
	h = (float)1/tan(fFov*0.5);   // 1/tan(x) == cot(x)
	Q = far_plane/(far_plane - near_plane);
 
	SHMatrix ret;
	ZeroMemory(&ret, sizeof(ret));
	ret._11 = w;
	ret._22 = h;
	ret._33 = Q;
	ret._34 = -Q*near_plane;
	ret._43 = 1;
	Init( ret );

	float fpCos = cos( fFov*0.5 );
	float fpSin = sin( fFov*0.5 );
	viewFrustrum[0] = CVec4(  fpCos, 0, fpSin, 0 );
	viewFrustrum[1] = CVec4( -fpCos, 0, fpSin, 0 );
	viewFrustrum[2] = CVec4( 0,  fpCos, fpSin, 0 );
	viewFrustrum[3] = CVec4( 0, -fpCos, fpSin, 0 );
	PrepareClipPlanes();
}
/////////////////////////////////////////////////////////////////////////////////////
void CTransformStack::MakeParallel( float fWidth, float fHeight )
{
	float far_plane = 10000;
	float near_plane = 1;
	SHMatrix ret;
	ZeroMemory(&ret, sizeof(ret));
	ret._11 = 2 / fWidth;
	ret._22 = 2 / fHeight;
	ret._33 = 1 / ( far_plane - near_plane );
	ret._34 = -near_plane * ret._33;
	ret._44 = 1;
	Init( ret );

	viewFrustrum[0] = CVec4(  1, 0, 0, -fWidth );
	viewFrustrum[1] = CVec4( -1, 0, 0, -fWidth );
	viewFrustrum[2] = CVec4(  0, 1, 0, -fHeight );
	viewFrustrum[3] = CVec4(  0,-1, 0, -fHeight );
	PrepareClipPlanes();
}
/////////////////////////////////////////////////////////////////////////////////////
inline float Dot( const CVec4 &a, const CVec4 &b ) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
/////////////////////////////////////////////////////////////////////////////////////
// scale independent variant
// can be optimized if take into account special structure viewFrustrum[] vertices 
// (lots of zeroes and same numbers with different signs)
bool CTransformStack::PushClipHint( const CVec3 &center, float fpRadius )
{
	CVec4 ptRes;
	const SHMatrix &trans = Get().forward;
	trans.RotateHVector( &ptRes, center );
	// replace sphere with Axis Aligned Bounding Cube
	for ( int i = 0; i < ARRAY_SIZE(viewFrustrum); i++ )
	{
		const CVec4 &test = viewFrustrum[i];
		float fpTest = 
			fabs( trans._11 * test.x + trans._21 * test.y + trans._31 * test.z + trans._41 * test.w ) + 
			fabs( trans._12 * test.x + trans._22 * test.y + trans._32 * test.z + trans._42 * test.w ) + 
			fabs( trans._13 * test.x + trans._23 * test.y + trans._33 * test.z + trans._43 * test.w );
		if ( Dot( ptRes, test ) + fpTest * fpRadius < 0 )
			return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
void CTransformStack::PopClipHint()
{
}
/////////////////////////////////////////////////////////////////////////////////////
