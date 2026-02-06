#ifndef __IGLOBALMAP_H_
#define __IGLOBALMAP_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iMain.h"
#include "GlobalInfo.h"
namespace NDb
{
	class CGlobalMap;
}
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
	enum EMode
	{
		MODE_SHOW,
		MODE_NORMAL
	};

public:
	virtual EMode GetMode() const = 0;

	virtual NUI::ICursor* GetCursor() const = 0;
	virtual NUI::CInterface* GetInterface() const = 0;

	virtual NRPG::CGlobalGame* GetGlobalGame() const = 0;
	virtual NRPG::CGlobalPlayer* GetGlobalPlayer() const = 0;
	virtual NSound::ISoundScene* GetSoundScene() const = 0;
	virtual NDb::CGlobalMap* GetGlobalMap() const = 0;
	virtual CPtrFuncBase<CGlobalInfo>* GetGlobalInfo() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CICBeginGame: public NMainLoop::CInterfaceCommand
{
	OBJECT_BASIC_METHODS(CICBeginGame);
private:
	int nTemplateID;
	vector<CObj<NRPG::CGlobalPlayer> > playersSet;

public:
	CICBeginGame() {}
	CICBeginGame( int nTemplateID, const vector<CObj<NRPG::CGlobalPlayer> > &playersSet );
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
class CICShowGlobal: public NMainLoop::CInterfaceCommand
{
	OBJECT_BASIC_METHODS(CICShowGlobal);
private:
	CPtr<NRPG::CGlobalGame> pGame;

public:
	CICShowGlobal() {}
	CICShowGlobal( NRPG::CGlobalGame *pGame );
	void Exec();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif