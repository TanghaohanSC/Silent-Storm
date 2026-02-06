#ifndef __A5_MISSIONMOVIE_UI_H__
#define __A5_MISSIONMOVIE_UI_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iDesktopWindow.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMissionMovieUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMissionMovieUI: public CDesktopWindow
{
	OBJECT_NOCOPY_METHODS(CMissionMovieUI);
private:
	NInput::CBind bindCancel;

	enum EStage
	{
		START,
		FADEIN,
		SHOWSCRIPT,
		////
		FINISH,
		FADEOUT
	};
	ZDATA_(CDesktopWindow)
	CPtr<CDesktopWindow> pTransition;
	CPtr<NGame::IMission> pMission;
	////
	int nPanelsStateSave;
	////
	STime sStageTime;
	EStage eStage;
	////
	CPtr<CImage> pTopBackground;
	CPtr<CImage> pBottomBackground;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CDesktopWindow*)this); f.Add(2,&pTransition); f.Add(3,&pMission); f.Add(4,&nPanelsStateSave); f.Add(5,&sStageTime); f.Add(6,&eStage); f.Add(7,&pTopBackground); f.Add(8,&pBottomBackground); return 0; }

protected:
	CAckEvent* PlayAckEvent( const STime &sTime, NWorld::CAckEvent *pEvent );

public:
	CMissionMovieUI();
	CMissionMovieUI( const SWindowInfo &sInfo, NGame::IMission *pMission, CDesktopWindow *pTransition );

	void ShowDesktop();
	void HideDesktop();
	void UpdateDesktop( const STime &sTime );
	NGame::CUICmdExec* CreateExecutor( NWorld::CUICmd *pCmd );

	bool ProcessEvent( const NInput::SEvent &sEvent );
	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
