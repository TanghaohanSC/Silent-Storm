#include "StdAfx.h"
#include "GSceneGraph.h"
#include "Transform.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
/////////////////////////////////////////////////////////////////////////////////////
// CMSRConvert
/////////////////////////////////////////////////////////////////////////////////////
void CMSRConvert::Update()
{
	if ( pMove.Refresh() | pRotate.Refresh() | pScale.Refresh() )
	{
		// CRAP{ - third angle is not accounted & slow implementation
		SHMatrix s;
		CMatrixStack43<4> m;
		const CVec3 &rotate = pRotate->GetValue();
		const CVec3 &scale = pScale->GetValue();
		MakeMatrix( &s, rotate.x, rotate.y, pMove->GetValue() );
		m.Init( s );
		m.PushScale( scale.x, scale.y, scale.z );
		value = m.Get();
		// CRAP}
		Updated();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CMSRConvert::Serialize( CStructureSaver *pFile )
{
	pFile->AddObject( 1, &pMove ); 
	pFile->AddObject( 2, &pRotate );
	pFile->AddObject( 3, &pScale );
}
/////////////////////////////////////////////////////////////////////////////////////
// CVec3Convert
/////////////////////////////////////////////////////////////////////////////////////
void CVec3Convert::Update()
{
	if ( pX.Refresh() | pY.Refresh() | pZ.Refresh() )
	{
		value.x = pX->GetValue();
		value.y = pY->GetValue();
		value.z = pZ->GetValue();
		Updated();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CVec3Convert::Serialize( CStructureSaver *pFile )
{
	pFile->AddObject( 1, &pX ); 
	pFile->AddObject( 2, &pY );
	pFile->AddObject( 3, &pZ );
}
/////////////////////////////////////////////////////////////////////////////////////
// CSinus
/////////////////////////////////////////////////////////////////////////////////////
void CSinus::Update()
{
	if ( pTime.Refresh() )
	{
		value = fpAdd + fpMn * sin( float( pTime->GetValue() / 1000 ) * fpFreq * FP_2PI);
		Updated();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CSinus::Serialize( CStructureSaver *pFile )
{
	pFile->AddData( 1, &fpMn );
	pFile->AddData( 2, &fpFreq );
	pFile->AddData( 3, &fpAdd );
	pFile->AddObject( 4, &pTime );
}
/////////////////////////////////////////////////////////////////////////////////////
} // namespace
/////////////////////////////////////////////////////////////////////////////////////
using namespace NGScene;
void RegisterSceneGraphClasses( int nBase )
{
	REGISTER_SAVELOAD_CLASS( nBase + 0, CCTime );
	REGISTER_SAVELOAD_CLASS( nBase + 1, CCMSR );
	REGISTER_SAVELOAD_CLASS( nBase + 2, CCVec3 );
	REGISTER_SAVELOAD_CLASS( nBase + 3, CCFloat );
	REGISTER_SAVELOAD_CLASS( nBase + 4, CCFBTransform );
	REGISTER_SAVELOAD_CLASS( nBase + 5, CMSRConvert );
	REGISTER_SAVELOAD_CLASS( nBase + 6, CVec3Convert );
	REGISTER_SAVELOAD_CLASS( nBase + 7, CSinus );
}
/////////////////////////////////////////////////////////////////////////////////////
