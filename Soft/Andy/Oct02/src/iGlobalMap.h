#ifndef __IGLOBALMAP_H_
#define __IGLOBALMAP_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iMain.h"
#include "GlobalInfo.h"
namespace NUI
{
	class ICursor;
	class CInterface;
}
namespace NRPG
{
	class CGlobalGame;
}
namespace NSound
{
	class ISoundScene;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// IGlobalMap
////////////////////////////////////////////////////////////////////////////////////////////////////
class IGlobalMap: public NMainLoop::IInterfaceBase
{
public:
	virtual float GetGlobalVar( const string &szID, float fDefault = 0 ) const = 0;
	virtual void SetGlobalVar( const string &szID, float fValue ) = 0;

	virtual NUI::ICursor* GetCursor() const = 0;
	virtual NUI::CInterface* GetInterface() const = 0;

	virtual NRPG::CGlobalGame* GetGlobalGame() const = 0;
	virtual NSound::ISoundScene* GetSoundScene() const = 0;
	virtual CPtrFuncBase<CGlobalInfo>* GetGlobalInfo() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CICBeginGlobal: public NMainLoop::CInterfaceCommand
{
	OBJECT_BASIC_METHODS(CICBeginGlobal);
private:
	int nTemplateID;
	vector<CObj<NRPG::CGlobalPlayer> > playersSet;

public:
	CICBeginGlobal() {}
	CICBeginGlobal( int nTemplateID, const vector<CObj<NRPG::CGlobalPlayer> > &playersSet );
	void Exec();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CICContinueGlobal: public NMainLoop::CInterfaceCommand
{
	OBJECT_BASIC_METHODS(CICContinueGlobal);
private:
	CPtr<NRPG::CGlobalGame> pGame;

public:
	CICContinueGlobal() {}
	CICContinueGlobal( NRPG::CGlobalGame *pGame );
	void Exec();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif