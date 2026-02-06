#ifndef __AIJOB_H_
#define __AIJOB_H_

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIJob
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIJob: public CObjectBase
{
public:
	virtual void DoJob() = 0; // выполнить итерацию
	virtual bool IsIdleJob() = 0; // работа не завершена, но нелать пока нечего
	virtual bool IsJobFinished() = 0; // работа завершена
	virtual IAIJob *GetParentJob() = 0; // работа запустившая данную
	virtual bool IsHighestPriority() = 0; // вернуть true, если все остальные работы не должны выполняться до окончания этих
	//
	virtual void OnJobFinished() = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CIJob
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIJob: public IAIJob
{
	ZDATA
	CPtr<IAIJob> pParentJob;
	bool bJobFinished;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pParentJob); f.Add(3,&bJobFinished); return 0; }
	//
	CAIJob( IAIJob *_pParentJob = 0 ): pParentJob(_pParentJob), bJobFinished( false ) {}
	//
	virtual void DoJob() = 0;
	virtual bool IsIdleJob() { return false; }
	virtual bool IsJobFinished() { return bJobFinished; }
	virtual IAIJob *GetParentJob() { return pParentJob; }
	virtual bool IsHighestPriority() { return false; }
	//
	virtual void OnJobFinished() {}
	void Finish() { bJobFinished = true; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIJobManager
////////////////////////////////////////////////////////////////////////////////////////////////////
class IAIJobManager: public CObjectBase
{
public:
	virtual void Segment() = 0;
	virtual void Add( IAIJob *pAIJob ) = 0; // добавить работу
	virtual void Remove( IAIJob *pAIJob ) = 0; // удалить работу
	virtual void RemoveDelayed( IAIJob *pAIJob ) = 0; // работа будет удалена не сразу
	virtual void WaitForJob( IAIJob *pAnticipantJob, 
		IAIJob *pExpectedJob ) = 0; //  pAnticipantJob не будет выполняться до завершения pExpectedJob
	virtual void Resume( IAIJob *pAnticipantJob ) = 0; // отменить режим ожидания
};
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIJobManager *CreateAIJobManager();
////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif
