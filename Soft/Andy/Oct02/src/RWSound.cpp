#include "StdAfx.h"
#include "wInterface.h"
#include "RWSound.h"
#include "Sound.h"
#include "Camera.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRender
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRenderSound: public IRenderSound, public COrdinarySyncDst<NWorld::IVisObj,CRenderSound>, 
public NWorld::ISoundVisitor
{
	typedef COrdinarySyncDst<NWorld::IVisObj,CRenderSound> TParent;
	OBJECT_BASIC_METHODS(CRenderSound);
	CPtr<NSound::ISoundScene> pScene;
public:
	CRenderSound() {}
	CRenderSound( NWorld::IWorld *pWorld, NSound::ISoundScene *pScene );
	
	virtual void Add3DSound( STime tStart, NDb::CSound *pSound, CFuncBase<CVec3> *pPosition );
	virtual void Update(ICamera *pCamera );
	int operator&( CStructureSaver &f );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IRenderSound* CreateRenderSound( NWorld::IWorld *pWorld, NSound::ISoundScene *pSoundScene )
{
	return new CRenderSound( pWorld, pSoundScene );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CRenderSound
////////////////////////////////////////////////////////////////////////////////////////////////////
CRenderSound::CRenderSound( NWorld::IWorld *_pWorld, NSound::ISoundScene *_pScene )
: COrdinarySyncDst<NWorld::IVisObj,CRenderSound>( 
	new CBoolSyncSrc<NWorld::IVisObj, CUnionFunc>( _pWorld->GetActive(), _pWorld->GetUnits() ) ), 
	pScene(_pScene)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderSound::Add3DSound( STime tStart, NDb::CSound *pSound, CFuncBase<CVec3> *pPosition )
{
	if ( !pSound )
		CPtr< CFuncBase<CVec3> > pHold( pPosition );
	else
		Register( pScene->Add3DSound( pSound, pPosition, tStart ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRenderSound::Update( ICamera *pCamera )
{
	Sync();
	pScene->Draw( pCamera );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CRenderSound::operator&( CStructureSaver &f )
{
	CPtr<CSyncSrc<NWorld::IVisObj> > pSrc = GetSource();
	f.Add( 1, &pSrc );
	f.Add( 2, &pScene );
	if ( f.IsReading() )
		SetNewSource( pSrc );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NRender;
REGISTER_SAVELOAD_CLASS( 0x03081142, CRenderSound );
