#ifndef __GFORMAT_H_
#define __GFORMAT_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NGfx
{
	class CGeometry;
	class CTriList;
	class CTexture;
	class CRenderTarget;
};
/////////////////////////////////////////////////////////////////////////////////////
// geometry loader from disk
class CFileGeometry: public NDG::CPtrFuncBase<NGfx::CGeometry>
{
	OBJECT_BASIC_METHODS(CFileGeometry);
	int nModelID;
protected:
	virtual void Update();
public:
	CFileGeometry( int _nModelID = 0 ) : nModelID( _nModelID ) {}
	void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
class CFileTriList: public NDG::CPtrFuncBase<NGfx::CTriList>
{
	OBJECT_BASIC_METHODS(CFileTriList);
	int nModelID;
protected:
	virtual void Update();
public:
	CFileTriList( int _nModelID = 0 ) : nModelID( _nModelID ) {}
	void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
void RegisterGFormat( int nBase );
/////////////////////////////////////////////////////////////////////////////////////
#endif