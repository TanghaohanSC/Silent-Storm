#ifndef __A5_IMISSION_EXEC_H__
#define __A5_IMISSION_EXEC_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUICmdExec
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdExec: public CObjectBase
{
	ZDATA
	CPtr<NWorld::CUICmd> pCmd;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pCmd); return 0; }

	CUICmdExec() {}
	CUICmdExec( NWorld::CUICmd *_pCmd ): pCmd( _pCmd ) {}

	virtual bool Update( const STime &sTime ) = 0;
	virtual void Cancel() {}
	virtual void Finished() {}

	NWorld::CUICmd* GetCmd() const { return pCmd; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUICmdExecContainer
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdExecContainer: public CUICmdExec
{
	OBJECT_BASIC_METHODS( CUICmdExecContainer );
	ZDATA
	ZPARENT( CUICmdExec );
	list< CPtr<CUICmdExec> > commands;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CUICmdExec *)this); f.Add(3,&commands); return 0; }
	//
public:
	CUICmdExecContainer() {}
	CUICmdExecContainer( NWorld::CUICmd *_pCmd ): CUICmdExec( _pCmd ) {}
	//
	void Add( CUICmdExec *pCmd );
	virtual bool Update( const STime &sTime );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUICmdCameraExec
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdMoveCameraExec: public CUICmdExec
{
	OBJECT_BASIC_METHODS(CUICmdMoveCameraExec)
private:
	ZDATA_(CUICmdExec)
	CPtr<IMission> pMission;
	ICamera::SCameraPos sTargetPos;
	STime transitionTime;
	////
	STime sMorphTime;
	ICamera::SCameraPos sCameraPos;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmdExec*)this); f.Add(2,&pMission); f.Add(3,&sTargetPos); f.Add(4,&transitionTime); f.Add(5,&sMorphTime); f.Add(6,&sCameraPos); return 0; }

private:
	void NormalizeAngle( float *pfAngle );
	void NormalizePos( ICamera::SCameraPos *pPos );

public:
	CUICmdMoveCameraExec() {}
	CUICmdMoveCameraExec( NWorld::CUICmd *pCmd, IMission *_pMission, 
		const ICamera::SCameraPos &_sTargetPos, STime _transitionTime );

	void SetTarget( const ICamera::SCameraPos &sTargetPos );

	bool Update( const STime &sTime );
	void Cancel();
	void Finished();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUICmdFollowCameraExec
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdFollowCameraExec: public CUICmdExec
{
	OBJECT_BASIC_METHODS(CUICmdFollowCameraExec)
private:
	ZDATA_(CUICmdExec)
	CPtr<IMission> pMission;
	CPtr<NWorld::CUnit> pUnit;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmdExec*)this); f.Add(2,&pMission); f.Add(3,&pUnit); return 0; }

public:
	CUICmdFollowCameraExec() {}
	CUICmdFollowCameraExec( NWorld::CUICmd *pCmd, IMission *pMission, NWorld::CUnit *pUnit );

	bool Update( const STime &sTime );
	void Cancel();
	void Finished();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUICmdRestoreCameraExec
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUICmdRestoreCameraExec: public CUICmdMoveCameraExec
{
	OBJECT_BASIC_METHODS(CUICmdRestoreCameraExec)
private:
	ZDATA_(CUICmdMoveCameraExec)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CUICmdMoveCameraExec*)this); return 0; }

public:
	CUICmdRestoreCameraExec() {}
	CUICmdRestoreCameraExec( NWorld::CUICmd *pCmd, IMission *pMission );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CUICmdExec* CreateExecutor( NWorld::CUICmd *pCmd, IMission *pMission );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif