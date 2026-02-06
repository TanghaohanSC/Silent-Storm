#include "stdafx.h"
#include "wAckBase.h"
#include "..\Misc\RandomGen.h"
#include "..\DBFormat\DataAck.h"
#include "wInterface.h"
#include "wUnitServer.h"
#include "wAck.h"
#include "RPGUnitMission.h"
#include "wMain.h"

namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalAck
////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFINE_UNITSERVER_EVENT_HANDLER( HandlerName )                               \
void CGlobalAck::HandlerName( CUnitServer *pUnit )                                   \
{                                                                                    \
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )      \
		(*i)->HandlerName( pUnit );                                                      \
}                                                                                    \
////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFINE_EVENT_HANDLER( HandlerName )                                          \
void CGlobalAck::HandlerName()                                                       \
{                                                                                    \
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )      \
		(*i)->HandlerName();                                                             \
}                                                                                    \
////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_EVENT_HANDLER( OnSegment );
DEFINE_EVENT_HANDLER( OnRealTimeStarted );
DEFINE_UNITSERVER_EVENT_HANDLER( OnLastPieceOfAmmo );
DEFINE_UNITSERVER_EVENT_HANDLER( OnWeaponJammed );
DEFINE_UNITSERVER_EVENT_HANDLER( OnOrderConfirmation );
DEFINE_UNITSERVER_EVENT_HANDLER( OnImpossibleToPerformAction );
DEFINE_UNITSERVER_EVENT_HANDLER( OnTargetHit );
DEFINE_UNITSERVER_EVENT_HANDLER( OnHardTargetHit );
DEFINE_UNITSERVER_EVENT_HANDLER( OnTargetMissed );
DEFINE_UNITSERVER_EVENT_HANDLER( OnSuffersLightDamage );
DEFINE_UNITSERVER_EVENT_HANDLER( OnSuffersHardDamage );
DEFINE_UNITSERVER_EVENT_HANDLER( OnInterrupt );
DEFINE_UNITSERVER_EVENT_HANDLER( OnSkillIncreased );
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnNewTurnStarted( IPlayer *pPlayer )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnNewTurnStarted( pPlayer );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnCannotFinishHeal( CUnitServer *pHealer, CUnitServer *pTarget )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnCannotFinishHeal( pHealer, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnHealFinished( CUnitServer *pHealer, CUnitServer *pTarget )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnHealFinished( pHealer, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnDoCriticalDamage( CUnitServer *pAttacker, CUnitServer *pTarget )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnDoCriticalDamage( pAttacker, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnUnitWasKilled( CUnitServer *pAttacker, CUnitServer *pTarget )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnUnitWasKilled( pAttacker, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnDoDamage( CUnitServer *pAttacker, CUnitServer *pTarget )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnDoDamage( pAttacker, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnDoAccidentalDamage( CUnitServer *pAttacker, CUnitServer *pTarget )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnDoAccidentalDamage( pAttacker, pTarget );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnEnemyBecomesVisible( CUnitServer *pWatcher, 
	CUnitServer *pTarget, bool bRealTime )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnEnemyBecomesVisible( pWatcher, pTarget, bRealTime );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnGrenadeExplosion( CUnitServer *pUnit,
																		int nUnitsDestroyed, int nObjectsDestroyed )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnGrenadeExplosion( pUnit, nUnitsDestroyed, nObjectsDestroyed );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::OnUnitDied( CUnitServer *pUnit )
{
	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); ++i )
		(*i)->OnUnitDied( pUnit );

	for ( vector< CObj<IAck> >::iterator i = vAck.begin(); i != vAck.end(); )
	{
		CDynamicCast<CAckBase> pAckBase( *i );
		CUnitServer *xpUnit = pAckBase->GetUnit();
		if ( pAckBase->GetUnit() == pUnit )
			i = vAck.erase( i );
		else
			++i;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::AddAckSequence( NDb::CDBAck *pAck )
{
	ASSERT( IsValid( pAck ) );
	lSequence.push_back( pAck );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::AddAck( CUnitServer *pUnit )
{
	CDBTable<NDb::CDBAck> *pDBTable = NDatabase::GetTable<NDb::CDBAck>();
	CDBIterator<NDb::CDBAck> i(*pDBTable);
	while ( pDBTable && i.MoveNext() )
	{
		NDb::CDBAck *pDBAck = i.Get();
		if ( pDBAck && pDBAck->nRPGPersID == pUnit->GetUnitRPG()->GetRPGPersID() )
		{
			CAckBase *pAck = CreateAck( pUnit, pDBAck );
			if ( pAck )
				vAck.push_back( pAck );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGlobalAck::IsContainUnit( const list< CPtr<CUnit> > &visibleUnits, int nRPGPersID )
{
	if ( !visibleUnits.empty() )
	{
		for ( list< CPtr<CUnit> >::const_iterator i = visibleUnits.begin(); i != visibleUnits.end(); ++i)
		{
			CDynamicCast<CUnitServer> pUnit(*i);
			if ( pUnit && pUnit->GetUnitRPG()->GetRPGPersID() == nRPGPersID )
				return true;
		}
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGlobalAck::IsSequenceVisible( const list< CPtr<CUnit> > &visibleUnits, NDb::CDBAckSequence *pSequence )
{
	for ( int i = 0; i < NDb::N_ACKINFO_MAX_COUNT; ++i )
		if ( pSequence->pDBAckInfo[i] )
		{			
			int &n = pSequence->pDBAckInfo[i]->nRPGPersID;
			if ( !IsContainUnit( visibleUnits, n ) )
				return false;
		}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::RemoveInvisibleSequences( IPlayer *pPlayer )
{
	// убираем все аски невидимые игроку pPlayer
	if ( !lSequence.empty() )
	{
		list< CPtr<CUnit> > visible;
		pPlayer->GetVisible( &visible );
	
		for ( list< CDBPtr<NDb::CDBAck> >::iterator i = lSequence.begin(); i != lSequence.end(); )	
		{
			if ( !IsSequenceVisible( visible, (*i)->pAckSequence ) )
				i = lSequence.erase( i );
			else 
				++i;
		}		
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::FetchHighestAcks()
{
	// ищем максимальный приоритет
	int nMaxPriority = 0;
	list< CDBPtr<NDb::CDBAck> >::iterator i;
	for ( i = lSequence.begin(); i != lSequence.end(); ++i )
		if ( (*i)->pAckSequence->nPriority > nMaxPriority )
			nMaxPriority = (*i)->pAckSequence->nPriority;
	// удаляем все аски с меньшим приоритетом
	for ( i = lSequence.begin(); i != lSequence.end(); )
		if ( (*i)->pAckSequence->nPriority < nMaxPriority )
			i = lSequence.erase( i );
		else
			++i;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CDBAckSequence *CGlobalAck::GetSequence( IPlayer *pPlayer )
{
	NDb::CDBAckSequence *pRes = 0;
	RemoveInvisibleSequences( pPlayer );
	FetchHighestAcks();

	if ( !lSequence.empty() )
	{
		// Выбираем один из Ack-ов
		int nNum = 0;
		if ( lSequence.size() > 1 )
			nNum = random.Get( 0, lSequence.size() );
		list< CDBPtr<NDb::CDBAck> >::iterator i = lSequence.begin();
		for ( ; nNum != 0; --nNum )
			++i;
		// учитываем вероятность Ack-а
		if ( random.Get( 1, 100 ) <= (*i)->fProbability )
			pRes = (*i)->pAckSequence;
		// очищаем
		lSequence.clear();
	}
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGlobalAck::SayAck( CUnitServer *pWho, int nConditionID )
{
	CDBTable<NDb::CDBAck> *pDBTable = NDatabase::GetTable<NDb::CDBAck>();
	CDBIterator<NDb::CDBAck> i(*pDBTable);
	while ( pDBTable && i.MoveNext() )
	{
		NDb::CDBAck *pDBAck = i.Get();
		if ( pDBAck && pDBAck->nRPGPersID == pWho->GetUnitRPG()->GetRPGPersID() && 
				 pDBAck->nConditionID == nConditionID )
		{
			AddAckSequence( pDBAck );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAckBase
////////////////////////////////////////////////////////////////////////////////////////////////////
CAckBase::CAckBase( CUnitServer *_pUnit, NDb::CDBAck *_pDBAck ): 
	pUnit(_pUnit), pDBAck(_pDBAck) 
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUnitServer *CAckBase::GetUnit()
{ 
	return	pUnit; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CDBAck *CAckBase::GetDBAck() 
{
	return pDBAck; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CWorld *CAckBase::GetWorld() 
{ 
	return GetUnit()->GetWorld(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAckBase::PlayAck() 
{ 
	GetWorld()->GetGlobalAck()->AddAckSequence( GetDBAck() ); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
using namespace NWorld;
//
REGISTER_SAVELOAD_CLASS( 0x52412170, CGlobalAck );