#include "StdAfx.h"
#include "Camera.h"
#include "Transform.h"
/////////////////////////////////////////////////////////////////////////////////////
// CCamera
/////////////////////////////////////////////////////////////////////////////////////
CCamera::CCamera(): fwd("forward"), back("backward"), left("left"), right("right"),
	up("up"), down("down" )
{
	pos = CVec3( 0,0,10 );//100, 100, 1200 );
	fpTangazh = 0; 
	fpRisk = 0;
}
/////////////////////////////////////////////////////////////////////////////////////
void CCamera::Update()
{
	fpTangazh += ( up.GetDelta() - down.GetDelta() ) * 0.6f;
	fpRisk += ( left.GetDelta() - right.GetDelta() ) * 0.6f;
	float fpFwd = ( fwd.GetDelta() - back.GetDelta() ) * 20;
	CQuat q, qt,qr;
	qt.FromAngleAxis( fpTangazh, CVec3(1,0,0) );
	qr.FromAngleAxis( fpRisk, CVec3(0,0,1) );
	q = qr*qt;
	CVec3 forward;
	q.Rotate( CVec3( 0, 1, 0 ), forward );
	pos += forward * fpFwd;
}
/////////////////////////////////////////////////////////////////////////////////////
SHMatrix CCamera::GetPos()
{
	SHMatrix res;
	MakeMatrix( &res, fpTangazh, fpRisk, pos );
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////
