#ifndef __IINTERMISSION_H_
#define __IINTERMISSION_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iMain.h"
namespace NRPG
{
	class CGlobalGame;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CICInterMission: public NMainLoop::CInterfaceCommand
{
	OBJECT_BASIC_METHODS(CICInterMission);
private:
	string szConfig;
	wstring wsMessage;

public:
	CICInterMission() {}
	CICInterMission( const string &szConfig, const wstring &wsMessage = L"" );
	virtual void Exec();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif