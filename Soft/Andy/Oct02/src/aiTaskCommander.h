#ifndef __AITASKCOMMANDER_H_
#define __AITASKCOMMANDER_H_

namespace NWorld
{
	class CCommand;
	class CUnitServer;
}

struct SMapUnit;

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EPose;
class IAIUnit;
class CAICommander;
class CTaskCommand;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAITaskCommander
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTask;
class CAITaskCommander: public CObjectBase
{
	OBJECT_BASIC_METHODS(CAITaskCommander);
	ZDATA
	list< CObj<CTask> > Tasks;
	CPtr<CAICommander> pAICommander;
	list< CObj<CTask> > TasksToAddOrRemove;
	list<bool> TasksAddFlag;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&Tasks); f.Add(3,&pAICommander); f.Add(4,&TasksToAddOrRemove); f.Add(5,&TasksAddFlag); return 0; }
	//
	bool CanGetCommand() const;
	NWorld::CWorld *GetWorld() const;
public:
	//
	CAITaskCommander() {}
	CAITaskCommander( CAICommander *_pAICommander ): pAICommander( _pAICommander ) {}
	//
	virtual void AddTask( CTask *pTask );
	virtual void RemoveTask( CTask *pTask );
	virtual NWorld::CCommand* GetCommand();
	virtual bool IsEndOfTurn() const;
	virtual bool IsUnderControl( IAIUnit *pAIUnit ) const;
	virtual bool IsUnderControl( NWorld::CUnitServer *pUnitServer ) const;
	virtual void CreateRoute( NWorld::CUnitServer *pUnitServer, SMapUnit sMapUnit, IPathNetwork *pPathNetwork );
	virtual void Segment();
	virtual void OnTurnStarted();
	virtual void OnTurnFinished() {}
	virtual void OnUnitWasKilled( NWorld::CUnitServer *pUnitServer ) {}
	bool HasActiveTasks() const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommandList
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTask: public CObjectBase
{
	OBJECT_BASIC_METHODS(CTask);
	ZDATA
	bool bCircled; // если true, то комманды выполняются по кругу	
	CPtr<NWorld::CUnitServer> pUnitServer; // кто выполняет комманды
	vector< CObj<CTaskCommand> > Commands; // комманды
	int nCurrentCommand; // комманда переданная на исполнение последней
	STime tTime; // время с которого начнется исполнение task-а
	bool bActive;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&bCircled); f.Add(3,&pUnitServer); f.Add(4,&Commands); f.Add(5,&nCurrentCommand); f.Add(6,&tTime); f.Add(7,&bActive); return 0; }
	//
public:
	CTask() {}
	CTask( NWorld::CUnitServer *_pUnitServer, bool _bCircled );
	//
	void AddCommand( CTaskCommand *pCmd );
	void AddLookAround();
	virtual NWorld::CCmd *GetCommand();
	virtual CTaskCommand *GetCurrentTaskCommand() { return Commands[nCurrentCommand]; }
	void SetToBeginning() { nCurrentCommand = -1; }
	void ClearCommands() { Commands.clear(); }
	virtual void OnNewTurn();
	virtual NWorld::CUnitServer *GetUnitServer() { return pUnitServer; }
	virtual bool IsEndOfTurn() const;
	virtual bool IsEmpty()  const { return Commands.empty(); }
	virtual bool IsEndOfTask()  const { return !bCircled && nCurrentCommand == (int)Commands.size(); }
	virtual void DelayExecution( int nTime );
	virtual void DebugOutput();
	void Activate() { bActive = true; }
	void DeActivate() { bActive = false; }
	bool IsActive() { return bActive; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommand
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTaskCommand: public CObjectBase
{
	ZDATA
public:
	CPtr<NWorld::CUnitServer> pUnitServer; // unit server исполняющий комманды
	list< CPtr<NWorld::CCmd> > Commands; // буфер для комманд
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pUnitServer); f.Add(3,&Commands); return 0; }
	
	CTaskCommand( NWorld::CUnitServer *_pUnitServer = 0 ): pUnitServer(_pUnitServer) {}

	virtual NWorld::CCmd *GetCommand();
	virtual void Do() {}

	virtual bool IsEndOfCommand() { return Commands.empty(); }
	virtual bool IsEndOfUnitTurn() { return false; }

	virtual void SetUnitServer( NWorld::CUnitServer *_pUnitServer ) { pUnitServer = _pUnitServer; }
	virtual void DoCommand( NWorld::CCmd *pCmd ) { Commands.push_back(pCmd); }
	virtual void OnNewTurn() {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommandGoto
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTaskCommandGoto: public CTaskCommand
{
	OBJECT_BASIC_METHODS(CTaskCommandGoto);
	ZDATA_(CTaskCommand)
	SPosition ptPosition;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CTaskCommand*)this); f.Add(2,&ptPosition); return 0; }
public:
	CTaskCommandGoto(): CTaskCommand(0) {}
	CTaskCommandGoto( SPosition _ptPosition ): CTaskCommand(0),	ptPosition(_ptPosition) {}
	//
	virtual void Do();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// СTaskCommandChangePose
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTaskCommandChangePose: public CTaskCommand
{
	OBJECT_BASIC_METHODS(CTaskCommandChangePose);
	ZDATA_(CTaskCommand)
	EPose nPose; 
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CTaskCommand*)this); f.Add(2,&nPose); return 0; }
public:
	CTaskCommandChangePose() {}
	CTaskCommandChangePose( EPose _nPose ): CTaskCommand(0), nPose(_nPose) {}
	//
	virtual void Do();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// СTaskCommandWait
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTaskCommandWait: public CTaskCommand
{
	OBJECT_BASIC_METHODS(CTaskCommandWait);
	ZDATA_(CTaskCommand)
	STime tTime; // до какого момента ждать
	STime tLength; // сколько ждать
	bool bIsWaiting; // ждем или нет
	bool bNewTurnStarted;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CTaskCommand*)this); f.Add(2,&tTime); f.Add(3,&tLength); f.Add(4,&bIsWaiting); f.Add(5,&bNewTurnStarted); return 0; }
public:
	CTaskCommandWait() {}
	CTaskCommandWait( STime _tLength ): CTaskCommand(), tLength(_tLength), bIsWaiting(false), bNewTurnStarted(false) {}
	//
	virtual void Do();
	virtual bool IsEndOfCommand();
	virtual bool IsEndOfUnitTurn();
	virtual void OnNewTurn();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTaskCommandChangeDirection
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTaskCommandChangeDirection: public CTaskCommand
{
	OBJECT_BASIC_METHODS(CTaskCommandChangeDirection);
	ZDATA_(CTaskCommand)
	int nDirection;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CTaskCommand*)this); f.Add(2,&nDirection); return 0; }
public:
	CTaskCommandChangeDirection() {}
	CTaskCommandChangeDirection( int _nDirection ) : CTaskCommand(), nDirection(_nDirection) {}
	//
	virtual void Do();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif 