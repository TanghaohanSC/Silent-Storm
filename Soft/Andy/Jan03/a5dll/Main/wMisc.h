#ifndef __wMisc_H_
#define __wMisc_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wDynObject.h"
#include "wInterfaceVisitors.h"
#include "Sync.h"
namespace NDb
{
	class CSound;
	class CParticleEffect;
}
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorld;
class CTimedObject: public IDynamicObject, public IVisObj
{
	ZDATA
	int nSegmentsLeft;
	STime tEvent;
	CSyncSrcBind<IVisObj> bindGlobal;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nSegmentsLeft); f.Add(3,&tEvent); f.Add(4,&bindGlobal); return 0; }
protected:
	STime GetEventTime() const { return tEvent; }
public:
	CTimedObject() {}
	CTimedObject( int _nSegmentsLeft );
	void Attach( CSyncSrc<NWorld::IVisObj> *pSrc, CWorld *pWorld );
	bool Segment() { return --nSegmentsLeft <= 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CTimedObject *CreateDParticles( CFuncBase<SFBTransform> *pPlace, NDb::CEffect *pEffect, int nFloor = -100 );
CTimedObject *CreateDParticles( const CVec3 &_pos, const CQuat &_q, NDb::CEffect *pEffect, int nFloor = -100 );
CTimedObject *Create3DSound( CFuncBase<CVec3> *pPos, NDb::CSound *pSound );
CTimedObject *Create3DSound( const CVec3 &_pos, NDb::CSound *pSound );
CTimedObject *CreateDGrassEvent( const CVec3 &_ptPlace );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
