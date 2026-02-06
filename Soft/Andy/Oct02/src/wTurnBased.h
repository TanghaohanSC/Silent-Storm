#ifndef __wTurnBased_H_
#define __wTurnBased_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
namespace NRPG
{
	class CGlobalGame;
}
namespace NWorld
{
template <class T, class TElem>
inline bool IsInSet( const T &c, const TElem &e ) { return find( c.begin(), c.end(), e ) != c.end(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
enum ETBSEvent
{
	TBS_START_NEW_TURN,
	TBS_FINISH_OWN_TURN,
	TBS_START_REAL_TIME,
	TBS_ACTION_FINISH,
	TBS_CANCEL_ACTION,
	TBS_RECALC_COMMAND
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCommand;
template <class TUnit, class TPlayer>
class CTBSUnit
{
protected:
	ZDATA
	CPtr<TPlayer> pPlayer;
	list<CPtr<TUnit> > visible;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pPlayer); f.Add(3,&visible); return 0; }
	TPlayer* GetTBSPlayer() const { return pPlayer; }
	virtual void Do( CCommand* ) = 0;
	virtual bool IsDead() const = 0;
	virtual bool IsUnconscious() const = 0;
	virtual void OnTBSEvent( ETBSEvent event ) {}
	virtual bool IsPerformingAction() const = 0;
	const list<CPtr<TUnit> >& GetTBSVisible() const { return visible; }
	bool CanSeePlayer( TPlayer *pThisPlayer ) const
	{
		for ( list<CPtr<TUnit> >::const_iterator k = visible.begin(); k != visible.end(); ++k )
			if ( (*k)->GetTBSPlayer() != pThisPlayer ) 
				return true;
		return false;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TUnit>
class CPlayerBase
{
protected:
	typedef list< CPtr<TUnit> > TUnitSet;
	ZDATA
	TUnitSet addToVisible;
	vector< CPtr<TUnit> > units;
	TUnitSet visible;
	bool bSeeOtherPlayers;
	int nPlayerID;
	bool bTurnDone;
	CObj<CCommander> pCommander;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&addToVisible); f.Add(3,&units); f.Add(4,&visible); f.Add(5,&bSeeOtherPlayers); f.Add(6,&nPlayerID); f.Add(7,&bTurnDone); f.Add(8,&pCommander); return 0; }

	CPlayerBase(): nPlayerID( 0 ), bTurnDone( false ) {}

	int GetPlayerID() const { return nPlayerID; }
	void SetPlayerID( int nID ) { nPlayerID = nID; }
	CCommander *GetCommander() const { ASSERT( pCommander ); return pCommander; }	
	void SetCommander( CCommander *_pCommander ) { pCommander = _pCommander; }
	const vector<CPtr<TUnit> >& GetPlayerUnits() const { return units; }
	bool CanSeeOtherPlayers() const { return bSeeOtherPlayers; }
	bool CanSee( TUnit *p ) { return find( visible.begin(), visible.end(), p ) != visible.end(); }
	const list<CPtr<TUnit> >& GetTBSVisible() const { return visible; }
	void AddUnit( TUnit *pUnit ) { units.push_back( pUnit ); }
	bool HasLostUnits()
	{ 
		if ( !HasAlivePeople() )
			return false;
		//
		for ( vector< CPtr<TUnit> >::iterator i = units.begin(); i != units.end(); ++i )
			if ( (*i)->HasLostUnits() )
				return true;
		//
		return false;
	}
	bool IsTurnDone()
	{
		return bTurnDone || !HasAlivePeople();
	}
	void UpdateVisible()
	{
		for ( TUnitSet::iterator i = visible.begin(); i != visible.end(); ++i )
		{
			if ( (*i)->IsDead() || (*i)->IsUnconscious() )
			{
				if ( !IsInSet( addToVisible, *i ) )
				{
					addToVisible.push_back( *i );
				}
			}
		}
		visible = addToVisible;
		bSeeOtherPlayers = false;
		for ( int k = 0; k < units.size(); ++k )
		{
			TUnit *pWatcher = units[k];
			if ( !IsInSet( visible, pWatcher ) )
				visible.push_back( pWatcher );
			if ( !pWatcher->IsDead() && !pWatcher->IsUnconscious() )
			{
				const list<CPtr<TUnit> > &vis = pWatcher->GetTBSVisible();
				for ( list<CPtr<TUnit> >::const_iterator i = vis.begin(); i != vis.end(); ++i )
				{
					TUnit *pUnit = *i;
					if ( !IsInSet( visible, pUnit ) )
					{
						if ( !pUnit->IsDead() && !pUnit->IsUnconscious() )
							bSeeOtherPlayers |= ( pUnit->GetTBSPlayer() != this );
						visible.push_back( *i );
					}
				}
			}
		}
	}
	void OnTBSEvent( ETBSEvent event )
	{
		if ( event == TBS_FINISH_OWN_TURN )
			bTurnDone = true;
		else if ( event == TBS_START_REAL_TIME )
			bTurnDone = false;
		//
		GetCommander()->OnTBSEvent( event );
		//
		for ( int k = 0; k < units.size(); ++k )
		{
			if ( !units[k]->IsDead() )
				units[k]->OnTBSEvent( event );
		}
	}
	bool HasAlivePeople() const
	{
		for ( int k = 0; k < units.size(); ++k )
		{
			if ( !units[k]->IsDead() && !units[k]->IsUnconscious() )
				return true;
		}
		return false;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TUnit, class TPlayer>
class CTBSWorld
{
	struct SInterrupt
	{
		ZDATA
		CPtr<TPlayer> pPlayer;
		list<CPtr<TUnit> > units;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pPlayer); f.Add(3,&units); return 0; }

		SInterrupt() {}
		SInterrupt( const list<TUnit*> &_units ): pPlayer( _units.front()->GetTBSPlayer() )
		{ 
			for ( list<TUnit*>::const_iterator i = _units.begin(); i != _units.end(); ++i )
			{
				ASSERT( (*i)->GetTBSPlayer() == pPlayer );
				units.push_back( *i ); 
			}
		}
		SInterrupt( TPlayer *_pPlayer ): pPlayer(_pPlayer) 
		{
			const vector<CPtr<TUnit> > &_units = pPlayer->GetPlayerUnits();
			for ( int i = 0; i < _units.size(); ++i )
				units.push_back( _units[i] );
		}
		void RemoveUnit( TUnit *_pUnit ) { units.remove( _pUnit ); }
		bool HasUnits() const { return !units.empty(); }
	};
		//typedef CInterrupt<TUnit, TPlayer> TInterrupt;
	typedef list< CObj<TPlayer> > TPlayerList;
	//typedef list< CObj<TInterrupt> > TInterruptList;
	typedef list<SInterrupt> TInterruptList;
	ZDATA
	CPtr<CActionCounter> pSkippableCount, pActiveCount;
	TInterruptList interrupts, addInterrupts;
	TPlayerList players;
	int nLastPlayerID, nTurnPlayerID;
	bool bWasAction;
	bool bFirstTurn;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pSkippableCount); f.Add(3,&pActiveCount); f.Add(4,&interrupts); f.Add(5,&addInterrupts); f.Add(6,&players); f.Add(7,&nLastPlayerID); f.Add(8,&nTurnPlayerID); f.Add(9,&bWasAction); f.Add(10,&bFirstTurn); return 0; }
private:
	TPlayer* GetNextPlayer( int nMinimal )
	{
		TPlayer *pBest = 0;
		int nBest = 0x7fffffff;
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
		{
			TPlayer *p = *i;
			if ( p->GetPlayerID() >= nMinimal && p->GetPlayerID() < nBest )
			{
				nBest = p->GetPlayerID();
				pBest = p;
			}
		}
		return pBest;
	}
	bool IsAIRealTimeModePossible() const
	{
		for ( TPlayerList::const_iterator i = players.begin(); i != players.end(); ++i )
			if ( (*i)->HasLostUnits() || !(*i)->IsTurnDone() )
				return false;
		//
		return true;
	}
	bool IsRealTimePossible() const
	{
		if ( !IsTBSRealTimeModePossible() )
			return false;
		if ( !IsAIRealTimeModePossible() )
			return false;
		bool bSeeEachOther = false;
		for ( TPlayerList::const_iterator k = players.begin(); k != players.end(); ++k )
			bSeeEachOther |= (*k)->CanSeeOtherPlayers();
		return !bSeeEachOther;
	}
	void StartPlayerTurn( TPlayer *pPlayer )
	{
		if ( !IsValid( pPlayer ) )
			return;
		pPlayer->OnTBSEvent( TBS_START_NEW_TURN );
		OnNewPlayerTurn( pPlayer );
		interrupts.push_back( SInterrupt( pPlayer ) );
		OnPassControl();
		nTurnPlayerID = pPlayer->GetPlayerID();
 	}
	void StartNextPlayerTurn()
	{
		if ( !IsRealTimePossible() )
		{
			TPlayer *pNext;
			pNext = GetNextPlayer( nTurnPlayerID + 1 );
			if ( !pNext )
				pNext = GetNextPlayer( 0 );
			StartPlayerTurn( pNext );
		}
	}
	void RecalcCurrentPlayerCommands()
	{
		if ( !interrupts.empty() )
			interrupts.back().pPlayer->OnTBSEvent( TBS_RECALC_COMMAND );//RecalcCommands();
	}
	void OnPassControl()
	{
		TPlayer *pCurrentPlayer = 0;
		if ( !interrupts.empty() )
			pCurrentPlayer = interrupts.back().pPlayer;

		RecalcCurrentPlayerCommands();
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			(*i)->GetCommander()->OnPassControl( pCurrentPlayer );
	}
	void EndOfTurn()
	{
		//ASSERT( !interrupts.empty() );
		if ( interrupts.empty() )
			return;
		CPtr<TPlayer> pPrevPlayer = interrupts.back().pPlayer;
		interrupts.pop_back();
		if ( !interrupts.empty() )
		{
			OnPassControl();
			return;
		}
		bFirstTurn = false;
		if ( IsValid( pPrevPlayer ) )
			pPrevPlayer->OnTBSEvent( TBS_FINISH_OWN_TURN );//FinishOwnTurn();
		StartNextPlayerTurn();
		if ( interrupts.empty() )
		{
			OnRealTimeStarted();
			for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
				(*i)->OnTBSEvent( TBS_START_REAL_TIME );
		}
	}
	void FetchPlayerCommands( TPlayer *pPlayer, bool bAllowNotSkippable )
	{
		for(;;)
		{
			CObj<CCommand> pCmd = pPlayer->GetCommander()->GetCommand();
			if ( !IsValid( pCmd ) )
				break;
			ASSERT( pCmd->IsSkippable() || bAllowNotSkippable );
			if ( CDynamicCast<CCmdEndOfTurn>(pCmd) )
			{
				EndOfTurn();
				break;
			}
			else if ( CDynamicCast<CCmdQuitGame>(pCmd) )
			{
				ASSERT( 0 );// not implemented yet
				EndOfTurn();
				break;
			}
			else if ( CDynamicCast<CCmdCheat> pCheatCmd(pCmd) )
				pPlayer->SetCheat( pCheatCmd->nCheatMask, pCheatCmd->bState );
			else
				ExecuteCommand( pCmd );
		}
	}
	CActionCounter* RenewActionCounter( CPtr<CActionCounter> *pDst )
	{
		if ( !IsValid( *pDst ) )
			(*pDst) = new CActionCounter;
		return *pDst;
	}
protected:
	TPlayer* GetNextPlayer( TPlayer* pPrev )
	{
		int nPlayerID = pPrev? pPrev->GetPlayerID() : -1;
		TPlayer *pBest = GetNextPlayer( nPlayerID + 1 );
		return pBest;
	}
	void CancelAllAction()
	{
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			(*i)->OnTBSEvent( TBS_CANCEL_ACTION );//CancelAction();
	}
	void GlobalSituationChanged()
	{
		// clear all queued cmds since situation changed
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
		{
			(*i)->GetCommander()->ClearList();
		}
		CancelAllAction();
	}
	virtual bool IsTBSRealTimeModePossible() const = 0;
	virtual void ExecuteCommand( CCommand *_pCmd ) = 0;
	virtual void OnAction( bool bStartAction ) = 0;
	void RegisterPlayer( TPlayer *pPlayer )
	{
		pPlayer->SetPlayerID( nLastPlayerID-- );
		players.push_back( pPlayer );
		return;
	}
	void StartTBSGame()
	{
		interrupts.clear();
		addInterrupts.clear();
		nTurnPlayerID = 0;
		StartNextPlayerTurn();
		bFirstTurn = !interrupts.empty();
	}
public:
	CTBSWorld()
	{
		nLastPlayerID = 10000;
		nTurnPlayerID = 0;
		bWasAction = false;
		bFirstTurn = false;
	}
	virtual void UpdateVisible() = 0;
	CActionCounter* GetSkippableCounter() { bWasAction = true; return RenewActionCounter( &pSkippableCount ); }
	CActionCounter* GetActiveCounter() { bWasAction = true; return RenewActionCounter( &pActiveCount ); }
	bool IsAction() const { return IsValid( pSkippableCount ) || IsValid( pActiveCount ); }
	bool IsFirstTurn() const { return bFirstTurn || IsRealTime(); }
	bool IsInterrupt() const { return interrupts.size() > 1; }
	int GetEnemyPlayerWatchers( const vector<CPtr<TUnit> > &units, TPlayer *pl ) const // "internal heap limit reached" workaround
	{
		int nWatchers = 0;
		const vector<CPtr<TUnit> > &enemeies = pl->GetPlayerUnits();
		for ( vector<CPtr<TUnit> >::const_iterator iu = enemeies.begin(); iu != enemeies.end(); ++iu )
		{
			if ( (*iu)->IsDead() || (*iu)->IsUnconscious() )
				continue;
			vector<CPtr<CUnit> > visible;
			(*iu)->GetVisible( &visible );
			for ( vector<CPtr<TUnit> >::const_iterator ku = units.begin(); ku != units.end(); ++ku )
			{
				const CUnit *pU = *ku;
				if ( !pU->IsDead() && !pU->IsUnconscious() && IsInSet( visible, pU ) )
						++nWatchers;
			}
		}
		return nWatchers;
	}
	int  GetEnemyWatchers( TPlayer *pPlayer ) const
	{
		if ( !IsValid( pPlayer ) )
			return 0;
		const vector<CPtr<TUnit> > &units = pPlayer->GetPlayerUnits();
		int nWatchers = 0;
		for ( TPlayerList::const_iterator i = players.begin(); i != players.end(); ++i )
			if ( *i != pPlayer )
				nWatchers += GetEnemyPlayerWatchers( units, *i );

		return nWatchers;
	}
	void AddInterrupt( const list<TUnit*> &units )
	{
		ASSERT( !units.empty() );
		if ( units.empty() )
			return;
		GlobalSituationChanged();
		addInterrupts.clear();
		addInterrupts.push_back( SInterrupt( units ) );
	}
	void UnitWasKilled( TUnit *p )
	{
		// сообщаем Commander-ам, что был убит Unit
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			(*i)->GetCommander()->OnUnitWasKilled( p );

		for ( TInterruptList::iterator i = interrupts.begin(); i != interrupts.end(); )
		{
			i->RemoveUnit( p );
			if ( !i->HasUnits() )
				i = interrupts.erase( i );
			else
				++i;
		}
		for ( TInterruptList::iterator i = addInterrupts.begin(); i != addInterrupts.end(); )
		{
			i->RemoveUnit( p );
			if ( !i->HasUnits() )
				i = addInterrupts.erase( i );
			else
				++i;
		}
		if ( interrupts.empty() && addInterrupts.empty() )
			StartNextPlayerTurn();
	}
	void GetInterrupts( vector< CPtr<IPlayer> > *pInterrups ) const
	{
		pInterrups->resize( interrupts.size() );
		int nTemp = 0;
		for ( TInterruptList::const_iterator iTemp = interrupts.begin(); iTemp != interrupts.end(); iTemp++ )
		{
			(*pInterrups)[nTemp] = iTemp->pPlayer;
			nTemp++;
		}
	}
	void Segment()
	{
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			(*i)->GetCommander()->Segment();

		if ( bWasAction && !IsAction() )
		{
			UpdateVisible();
			RecalcCurrentPlayerCommands();
			OnAction( false );
			for ( TPlayerList::const_iterator k = players.begin(); k != players.end(); ++k )
				(*k)->OnTBSEvent( TBS_ACTION_FINISH );//OnActionFinish();
		}
		if ( !addInterrupts.empty() )
		{
			ASSERT( addInterrupts.size() == 1 );
			if ( interrupts.empty() )
			{
				bFirstTurn = true;
				StartPlayerTurn( addInterrupts.front().pPlayer );
			}
			else
			{
				// no more then one interrupt in stack is allowed
				while ( interrupts.size() > 1 )
					interrupts.pop_back();
				// ignore same player interrupt
				if ( addInterrupts.front().pPlayer == interrupts.back().pPlayer )
				{
//					ASSERT( 0 );
					addInterrupts.clear();
				}
				else
					interrupts.splice( interrupts.end(), addInterrupts );
				OnPassControl();
			}
		}
		// check if someone want interrupt
		if ( interrupts.empty() )
		{
			for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			{
				TPlayer *pPlayer = *i;
				CCommander *pC = pPlayer->GetCommander();
				if ( pC->IsRequestInterrupt() )
				{
					CancelAllAction();
					bFirstTurn = true;
					StartPlayerTurn( pPlayer );
					break;
				}
				else if ( pC->IsRequestCancel() )
					pPlayer->OnTBSEvent( TBS_CANCEL_ACTION );//CancelAction();
			}
		}
		else
		{
			CCommander *pCommander = interrupts.back().pPlayer->GetCommander();
			if ( pCommander->IsRequestCancel() )//|| pCommander->IsRequestInterrupt() )
				CancelAllAction();
		}
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			(*i)->GetCommander()->ClearRequests();
		// decide on commands for next segment
		if ( interrupts.empty() )
		{
			// real time mode
			// pick commands from every player
			for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
				FetchPlayerCommands( *i, false );
		}
		else
		{
			// turn based mode
			if ( !IsAction() )
			{
				// pick commands from current player regarding current units
				FetchPlayerCommands( interrupts.back().pPlayer, true );
			}
			else
				OnAction( true );
		}
		bWasAction = IsAction();
	}
	void UpdatePlayersVisibleSets()
	{
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			(*i)->UpdateVisible();
	}
	TPlayer* GetTBSCurrentPlayer() const { if ( interrupts.empty() ) return 0; return interrupts.back().pPlayer; }
	bool IsRealTime() const { return interrupts.empty(); }
	bool IsTBSUnitActive( TUnit *pUnit ) const { if ( interrupts.empty() ) return true; return IsInSet( interrupts.back().units, pUnit ); }
	bool CanPlayerSeeAction( TPlayer *_pPlayer ) const
	{
		// some skippable action is taking place
		// check if _pPlayer see any units performing skippable action
		const list<CPtr<TUnit> > &v = _pPlayer->GetTBSVisible();
		for ( list<CPtr<TUnit> >::const_iterator k = v.begin(); k != v.end(); ++k )
		{
			TUnit *pTest = (*k);
			if ( pTest->IsDead() || pTest->IsUnconscious() )
				continue;
			if ( pTest->IsPerformingAction() )
				return true;
		}
		return false;
	}
	bool CanSkip( TPlayer *_pPlayer ) const
	{
		if ( !interrupts.empty() && interrupts.back().pPlayer != _pPlayer )
		{
			// is real time & not current player is active
			if ( !IsValid( pActiveCount ) && IsValid( pSkippableCount ) )
			{
				// some skippable action is taking place
				// check if _pPlayer see any units performing skippable action
				return !CanPlayerSeeAction( _pPlayer );
			}
		}
		return false;
	}
	bool HasEnemies( TPlayer *pPlayer )
	{
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
		{
			TPlayer *p = *i;
			if ( pPlayer != p && p->HasAlivePeople() )
				return true;
		}
		return false;
	}
	void WantTurnBased( TPlayer *pPlayer )
	{
		if ( interrupts.empty() && addInterrupts.empty() )
			addInterrupts.push_back( SInterrupt( pPlayer ) );
	}
	void ProcessAISignals()
	{
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			(*i)->GetCommander()->ProcessAISignals();
	}
	virtual void OnUnitAdded( TUnit *pUnit ) 
	{
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			(*i)->GetCommander()->OnUnitAdded( pUnit );	
	}
	virtual void OnNewPlayerTurn( TPlayer *pPlayer ) {}
	virtual void OnRealTimeStarted() {}
	void GetPlayersList( list< CPtr<TPlayer> > *pPlayers )
	{
		pPlayers->clear();
		for ( TPlayerList::iterator i = players.begin(); i != players.end(); ++i )
			pPlayers->push_back( (*i).GetPtr() );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
