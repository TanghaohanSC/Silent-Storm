#include "stdafx.h"

#include "..\Misc\HPTimer.h"

#include "aiJob.h"

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_AI_TIME_TO_THINK = 0.030f;
////////////////////////////////////////////////////////////////////////////////////////////////////
// IAIJobManager
////////////////////////////////////////////////////////////////////////////////////////////////////
class CAIJobManager: public IAIJobManager
{
	typedef hash_map< CPtr<IAIJob>, list< CPtr<IAIJob> >, SPtrHash > DependencesHash;
	//
	OBJECT_BASIC_METHODS(CAIJobManager);
	ZDATA
	float fTime;
	float fOverrunTime;
	int nCurrentJob;
	vector< CPtr<IAIJob> > Jobs;
	list< CPtr<IAIJob> > JobsToRemove;
	DependencesHash Dependences;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&fTime); f.Add(3,&fOverrunTime); f.Add(4,&nCurrentJob); f.Add(5,&Jobs); f.Add(6,&JobsToRemove); f.Add(7,&Dependences); return 0; }
	//
	bool CanSkip();
	void Refresh();
	bool IsWaiting( IAIJob *pAIJob );
	void RemoveDependences( IAIJob *pAIJob );
public:
	//	
	CAIJobManager() {}
	CAIJobManager( float _fTime );
	// IAIJobManager
	virtual void Segment();
	virtual void Add( IAIJob *pAIJob );
	virtual void Remove( IAIJob *pAIJob );
	virtual void RemoveDelayed( IAIJob *pAIJob );
	virtual void WaitForJob( IAIJob *pAnticipantJob, 
		IAIJob *pExpectedJob );
	virtual void Resume( IAIJob *pAnticipantJob );
	virtual void DebugOutput();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CAIJobManager::CAIJobManager( float _fTime ) :
	fTime(_fTime), fOverrunTime(0), nCurrentJob(0)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::DebugOutput()
{
	char szStr[128];
	OutputDebugString( "[AI JOB MANAGER] {\n" );
	sprintf( szStr, "Job manager has %d jobs\n", Jobs.size() );
	OutputDebugString( szStr );
	OutputDebugString( "[AI JOB MANAGER] }\n" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::Add( IAIJob *pAIJob )
{
	Jobs.push_back( pAIJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::RemoveDependences( IAIJob *pAIJob )
{
	Dependences.erase( pAIJob );
	//
	for ( DependencesHash::iterator i = Dependences.begin(); i != Dependences.end(); ++i )
		for ( list< CPtr<IAIJob> >::iterator j = i->second.begin(); j != i->second.end(); )
			if ( (*j) == pAIJob )
				j = i->second.erase( j );
			else
				++j;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::Refresh()
{
	// удаляем старые задачи
	for ( list< CPtr<IAIJob> >::iterator i = JobsToRemove.begin(); i != JobsToRemove.end(); ++i )
	{
		vector< CPtr<IAIJob> >::iterator t = find( Jobs.begin(), Jobs.end(), *i );
		if ( t != Jobs.end() )
		{
			RemoveDependences( *t );
			Jobs.erase( t );
		}
	}
	// 
	JobsToRemove.clear();
	// обновляем
	nCurrentJob = 0;
	//DebugOutput();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::RemoveDelayed( IAIJob *pAIJob )
{
	if ( find( Jobs.begin(), Jobs.end(), pAIJob ) != Jobs.end() )
		JobsToRemove.push_back( pAIJob );
	//
	for ( vector< CPtr<IAIJob> >::iterator i = Jobs.begin(); i != Jobs.end(); ++i )
		if ( (*i)->GetParentJob() == pAIJob )
			RemoveDelayed( *i );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::Remove( IAIJob *pAIJob )
{
	RemoveDelayed( pAIJob );
	Refresh();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIJobManager::CanSkip()
{
	// если нет задач
	if ( Jobs.empty() )
		return true;
	// если никто не хочет думать :)
	for ( int k = 0; k < Jobs.size(); ++k )
	{
		CPtr<IAIJob> &pAIJob = Jobs[k];
		if ( !pAIJob->IsIdleJob() && !IsWaiting( pAIJob ) )
			return false;
	}
	//
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAIJobManager::IsWaiting( IAIJob *pAIJob )
{
	list< CPtr<IAIJob> > &ExpectedJobs = Dependences[pAIJob];
	for ( list< CPtr<IAIJob> >::iterator i = ExpectedJobs.begin(); i != ExpectedJobs.end(); ++i )
	{
		if ( !(*i)->IsJobFinished() )
			return true;
	}
	//
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::WaitForJob( IAIJob *pAnticipantJob, IAIJob *pExpectedJob )
{
	ASSERT( find( Jobs.begin(), Jobs.end(), pAnticipantJob ) != Jobs.end() );
	ASSERT( find( Jobs.begin(), Jobs.end(), pExpectedJob ) != Jobs.end() );
	//
	Dependences[pAnticipantJob].push_back( pExpectedJob );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::Resume( IAIJob *pAnticipantJob )
{
	Dependences[pAnticipantJob].clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAIJobManager::Segment()
{
	if ( CanSkip() )
		return;
	//
	NHPTimer::STime tTime, tTmpTime;
	NHPTimer::GetTime( &tTime );
	tTmpTime = tTime;
	//
	while ( !Jobs.empty() && NHPTimer::GetTimePassed( &tTmpTime ) < (fTime - fOverrunTime) ) 
	{	
		tTmpTime = tTime;
		//
		CPtr<IAIJob> pAIJob = Jobs[nCurrentJob];
		if ( !pAIJob->IsJobFinished() && !pAIJob->IsIdleJob() && !IsWaiting( pAIJob ) )
		{
			pAIJob->DoJob();
			if ( pAIJob->IsJobFinished() )
			{
				pAIJob->OnJobFinished();
				RemoveDelayed( pAIJob );
			}
		}
		//
		++nCurrentJob;
		if ( nCurrentJob >= Jobs.size() )
			Refresh();
	}
	//
	tTmpTime = tTime;
	fOverrunTime += NHPTimer::GetTimePassed( &tTmpTime ) - fTime;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IAIJobManager *CreateAIJobManager()
{
	return new CAIJobManager( F_AI_TIME_TO_THINK );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}

using namespace NAI;
REGISTER_SAVELOAD_CLASS( 0x52442120, CAIJobManager );