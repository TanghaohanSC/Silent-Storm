#ifndef __wMisc_H_
#define __wMisc_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wDynObject.h"
namespace NDb
{
	class CSound;
	class CParticleEffect;
}
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorld;
class CTimedObject: public IDynamicObject
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
class CDParticles: public CTimedObject
{
	OBJECT_NOCOPY_METHODS(CDParticles);
	ZDATA_(CTimedObject)
	CDBPtr<NDb::CEffect> pEffect;
	CObj<CFuncBase<SFBTransform> > pPosition;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CTimedObject*)this); f.Add(2,&pEffect); f.Add(3,&pPosition); return 0; }
public:
	CDParticles() {}
	CDParticles( CFuncBase<SFBTransform> *pPlace, NDb::CEffect *pEffect );
	CDParticles( const CVec3 &_pos, const CQuat &_q, NDb::CEffect *pEffect );
	
	virtual void Visit( IRenderVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class C3DSound: public CTimedObject
{
	OBJECT_NOCOPY_METHODS(C3DSound);
	ZDATA_(CTimedObject)
	CObj<CFuncBase<CVec3> > pPosition;
	CDBPtr<NDb::CSound> pSound;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CTimedObject*)this); f.Add(2,&pPosition); f.Add(3,&pSound); return 0; }
public:
	C3DSound() {}
	C3DSound( CFuncBase<CVec3> *pPos, NDb::CSound *pSound );
	C3DSound( const CVec3 &_pos, NDb::CSound *pSound );

	virtual void Visit( ISoundVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDGrassEvent: public CTimedObject
{
	OBJECT_NOCOPY_METHODS(CDGrassEvent);
	ZDATA_(CTimedObject)
	CVec3 vPlace;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CTimedObject*)this); f.Add(2,&vPlace); return 0; }
public:
	CDGrassEvent() {}
	CDGrassEvent( const CVec3 &_ptPlace );

	virtual void Visit( IRenderVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDFlash: public CTimedObject
{
	OBJECT_NOCOPY_METHODS(CDFlash);
	ZDATA_(CTimedObject)
	CVec3 vPlace;
	CVec3 vColor;
	float fRadius;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CTimedObject*)this); f.Add(2,&vPlace); f.Add(3,&vColor); f.Add(4,&fRadius); return 0; }
public:
	CDFlash() {}
	CDFlash( const CVec3 &_vPlace, CVec3 &_vColor, float _fRadius, int nTime );

	virtual void Visit( IRenderVisitor *p );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
