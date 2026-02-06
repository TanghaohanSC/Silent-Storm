#ifndef __A5_I_MISSIONSTATE_H__
#define __A5_I_MISSIONSTATE_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
/*
class IMissionInterface;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateTurnSwitch: public IState
{
	OBJECT_NOCOPY_METHODS(CStateTurnSwitch);
private:
	NInput::CBind bindOK;
	ZDATA
	bool bPauseState;
	CObj<NUI::IWindow> pContainer;
	CPtr<IMissionInterface> pInterface;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bPauseState); f.Add(3,&pContainer); f.Add(4,&pInterface); return 0; }

public:
	CStateTurnSwitch();
	~CStateTurnSwitch();

	void Initialize( IMissionInterface* pInterface );
	bool Step();
	bool ProcessEvent( const NInput::SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CStateEndGame: public IState
{
	OBJECT_NOCOPY_METHODS(CStateEndGame);
private:
	NInput::CBind bindOK, bindLButtonUp, bindRButtonUp;
	ZDATA
	bool bPauseState;
	wstring wsText;
	CObj<NUI::IWindow> pContainer;
	CPtr<IMissionInterface> pInterface;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bPauseState); f.Add(3,&wsText); f.Add(4,&pContainer); f.Add(5,&pInterface); return 0; }

public:
	CStateEndGame();
	~CStateEndGame();

	void Initialize( IMissionInterface* pInterface, const wstring &wsText );
	bool Step();
	bool ProcessEvent( const NInput::SEvent &sEvent );
};
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
