#ifndef __INTERFACE_LOGPANEL_H_
#define __INTERFACE_LOGPANEL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\Misc\LogStream.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLogPanel
////////////////////////////////////////////////////////////////////////////////////////////////////
struct STextLine
{
	ZDATA
	bool bUsed;
	STime sTime;
	wstring wsText;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bUsed); f.Add(3,&sTime); f.Add(4,&wsText); return 0; }

	STextLine(): bUsed( false ), sTime( 0 ) {}
};
class CLogPanel: public CWindow
{
	OBJECT_BASIC_METHODS(CLogPanel)
private:
	ZDATA_(CWindow)
	int nLastID;
	EStreamType eType;
	list<STextLine> linesList;
	vector<CObj<CTextDraw> > textDrawSet;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&nLastID); f.Add(3,&eType); f.Add(4,&linesList); f.Add(5,&textDrawSet); return 0; }

protected:
	void DrawLine( const STime &sTime, NGScene::I2DGameView *pView, int nLine, const wstring &wsText, int *pHeight, vector<CObj<CTextDraw> > *pNewTextLines );

public:
	CLogPanel() {}
	CLogPanel( const SWindowInfo &sInfo, EStreamType eType );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
