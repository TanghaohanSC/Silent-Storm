#ifndef __IINTERMISSION_H_
#define __IINTERMISSION_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iMain.h"
#include "iGlobalMap.h"
#include "iChapterMap.h"
#include "iMission.h" // TacticView
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Type>
class CStateTracker
{
private:
	ZDATA
	bool bUpdated;
	CPtr<Type> pInterface;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bUpdated); f.Add(3,&pInterface); return 0; }

public:
	CStateTracker() {}

	bool IsValid() { return pInterface.IsValid(); }

	void Activate() { NMainLoop::Command( pInterface ); }

	bool IsUpdated() { return bUpdated; }
	void SetUpdate( bool bOn ) { bUpdated = bOn; }

	Type* GetInterface() { return pInterface; }
	void SetInterface( Type *_pInterface ) { pInterface = _pInterface; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CICMissionMap;
class CICGroundArea;
///
class CGlobalGame
{
public:
	CStateTracker<CICGlobalMap> sGlobalMap;
	CStateTracker<CICChapterMap> sChapterMap;
	CStateTracker<CICMissionMap> sMissionMap;
	CStateTracker<CICTacticView> sTacticView;
	CStateTracker<CICGroundArea> sGroundArea;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif