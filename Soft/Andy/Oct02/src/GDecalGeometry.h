#ifndef __GDECALGEOMETRY_H_
#define __GDECALGEOMETRY_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DG.h"
#include "DiscretePos.h"
namespace NGScene
{
class CObjectInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDecalGeometry: public CPtrFuncBase<CObjectInfo>
{
	OBJECT_BASIC_METHODS(CDecalGeometry);
	ZDATA
	CDGPtr<CPtrFuncBase<CObjectInfo> > pSource;
	CVec3 vOrigin, vNormal;
	CVec2 vSize;
	float fRotation;
	SDiscretePos srcPos;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pSource); f.Add(3,&vOrigin); f.Add(4,&vNormal); f.Add(5,&vSize); f.Add(6,&fRotation); f.Add(7,&srcPos); return 0; }
protected:
	virtual bool NeedUpdate() { return pSource.Refresh(); }
	virtual void Recalc();
public:
	CDecalGeometry() {}
	// srcPos transforms source geometry to space where _vOrigin, _vNormal are specified
	// rotation is in radians
	CDecalGeometry( CPtrFuncBase<CObjectInfo> *p, const SDiscretePos &_srcPos, 
		const CVec3 &_vOrigin, const CVec3 &_vNormal, const CVec2 &_vSize, float _fRotation );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif