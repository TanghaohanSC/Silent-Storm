#ifndef _aiWaveSearch_H_
#define _aiWaveSearch_H_
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TPosition, class TCost>
struct SMoveInfo
{
	TPosition parentPos;
	TCost cost;
	SMoveInfo( const TPosition &_parentPos, const TCost &_cost): cost(_cost), parentPos(_parentPos) {}
	SMoveInfo() {}   // default constructor needed by CArray2D
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSquareMapCosts
{
public:
	typedef CTPoint<unsigned char> SPosition;
	typedef SMoveInfo<SPosition, WORD> SMove;
private:
	ZDATA
	CArray2D<SMove> moves;
public:	
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&moves); return 0; }
	CSquareMapCosts() {}
	CSquareMapCosts( unsigned char _cSizeX, unsigned char _cSizeY ) { moves.SetSizes( _cSizeX, _cSizeY ); }
	SMove& operator[]( const SPosition &pt ) { return moves[ pt.y ][ pt.x ]; }
	WORD GetCost( const SPosition &pt ) { return moves[ pt.y ][ pt.x ].cost; }
	void MakeInfinity(void)
	{
		// do nothing - we'll make it infinite in other point
	}
	void SetSizes( unsigned char cMaxX, unsigned char cMaxY ) { moves.SetSizes( cMaxX, cMaxY ); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// next is the very simple class providing TCostsTable functionality for sub-rects in square maps...
class CSquareMapSubRectCosts
{
	typedef CTPoint<unsigned char> SPosition;
	typedef SMoveInfo<SPosition, WORD> SMove;
	ZDATA
	unsigned char cX1, cY1, cSizeX, cSizeY;
	CArray2D<SMove> moves;
public:	
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&cX1); f.Add(3,& cY1); f.Add(4,&cY1); f.Add(5,& cSizeX); f.Add(6,&cSizeX); f.Add(7,& cSizeY); f.Add(8,&cSizeY); f.Add(9,&moves); return 0; }
	CSquareMapSubRectCosts( unsigned char _cX1, unsigned char _cY1, unsigned char _cX2, unsigned char _cY2):
			cX1(_cX1), cY1(_cY1), cSizeX(_cX2 - _cX1), cSizeY(_cY2 - _cY1), moves(_cX2 - _cX1, _cY2 - _cY1) {}
	SMove& operator[](const SPosition &pt) { return moves[ pt.y - cY1 ][ pt.x - cX1 ]; }
	WORD GetCost(const SPosition &pt) { return moves[ pt.y - cY1 ][ pt.x - cX1 ].cost; }
	void Move( unsigned char _cX1, unsigned char _cY1 ) { cX1 = _cX1; cY1 = _cY1; } // do not reallocate
	void MakeInfinity(void)
	{
		for ( unsigned char i = 0; i < cSizeX; ++i )
			for ( unsigned char j = 0; j < cSizeY; ++j )
				moves[j][i].cost = 65535;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TMap, class TPosition, class TCost, class TConstraints, class TCostsTable, 
			size_t MAX_BUFFER, size_t MAX_MOVE_COST>
class CWaveCountCosts
{
	TCostsTable* table;
	const TConstraints *constraints;
	const TMap* pMap;
	TPosition position;

	/*static ?*/ TPosition queue[MAX_MOVE_COST+1][MAX_BUFFER];
	int currentPrior;
	int queueMaximums[MAX_MOVE_COST+1];

	/*static ?*/ TPosition current;
	TCost currentCost;

public:
	void SetConstraints( const TConstraints *_constr) { constraints = _constr; }
	void SetMap( const TMap *_pMap) { pMap = _pMap; }
	void SetPosition( const TPosition& _position) { position = _position; }

	CWaveCountCosts
	(TCostsTable *_table, const TConstraints *_constraints, 
	 const TMap *_map, const TPosition& _position): 
	table(_table), constraints(_constraints), pMap(_map), position(_position), currentPrior(0) {}

	void operator()( const TPosition &pos, TCost cost )
	{
		if ( !(*constraints)( pos ) ) 
			return; 
		TCost totalCost = currentCost + cost;
		if ( table->GetCost(pos) <= totalCost ) 
			return; 
		// Добавляем, определяя заодно новую стоимость
		(*table)[pos].parentPos  = current;
		(*table)[pos].cost = totalCost;
		int nQueue = cost + currentPrior; 
		if (nQueue > MAX_MOVE_COST) 
			nQueue -= (MAX_MOVE_COST+1);
		
		// This assertion will fail if buffer size was not enough
		ASSERT(queueMaximums[nQueue] < MAX_BUFFER);

		queue[nQueue][queueMaximums[nQueue]] = pos;
		queueMaximums[nQueue]++;	
	}
////////////////////////////////////////////////////////////////////////////////////////////////////
	void Count( )
	{
		// initializing
		ZeroMemory( queueMaximums, (MAX_MOVE_COST+1)*sizeof(int) );
		
		// somehow make table have infinite values
		table->MakeInfinity();

		// adding first TPosition
		(*table)[position] = SMoveInfo<TPosition, TCost>(position, 0); // parent is the same TPosition and cost is zero
		queue[0][0]= position;
		queueMaximums[0]++;
		currentPrior = 0;

		// Main loop
		while (1)
		{
			// pop next TPosition as current
			//	get next non-empty queue as current prior; if no any, return
			int oldPrior = currentPrior;
			while (!queueMaximums[currentPrior]) 
			{				
				currentPrior++;
				if (currentPrior > MAX_MOVE_COST) currentPrior -= (MAX_MOVE_COST+1);
				if (oldPrior == currentPrior) 
					return; // no non-empty queues
			}
			queueMaximums[currentPrior]--;
			current = queue[currentPrior][queueMaximums[currentPrior]];

		  currentCost = (*table)[current].cost;
			// for each move in TMap beginning with this TPosition, apply operator() ...
			pMap->ForEachMove(current, *this);
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TMap, class TPosition, class TCost, class TConstraints, class TCostsTable, size_t MAX_MOVE_COST>
class CWaveCountCostsMajor
{
	TCostsTable *table;
	const TConstraints *constraints;
	TMap *pMap;
	TPosition position;

	typedef vector<TPosition> TPositionVector;
	TPositionVector queue[MAX_MOVE_COST+1];
	int currentPrior;
	TPosition current;
	TCost currentCost;
	TCost maxCost;  // maxCost == 0 means that maxCost is not defined

public:
	TPosition firstReached;
	void SetConstraints( const TConstraints *_constr) { constraints = _constr; }
	void SetMap( TMap *_pMap) { pMap = _pMap; }
	void SetPosition( const TPosition& _position) { position = _position; }

	CWaveCountCostsMajor
	(TCostsTable* _table, const TConstraints *_constraints, 
	 TMap* _map, const TPosition& _position): 
	table(_table), constraints(_constraints), pMap(_map), position(_position), currentPrior(0) {}

	void operator()( const TPosition &pos, TCost cost )
	{
		ASSERT( cost <= MAX_MOVE_COST );
		TCost totalCost = currentCost + cost;
		if ( table->GetCost( pos ) <= totalCost ) return; 

		if ( !(*constraints)( current, pos ) ) return; 
		// Добавляем, определяя заодно новую стоимость
		if  (( !maxCost ) || ( totalCost < maxCost ))
		{
			(*table)[ pos ].parentPos  = current;
			(*table)[ pos ].cost = totalCost;
			int nQueue = cost + currentPrior; 
			if ( nQueue > MAX_MOVE_COST ) 
				nQueue -= ( MAX_MOVE_COST + 1 );

			queue[ nQueue ].push_back( pos ); 
#ifdef _DEBUG
			if ( ( queue[ nQueue ].size() & 127 ) == 0 )
			{
				char buf[50];			
				sprintf( buf, "WaveSearch buffer: queue size is %d\n", queue[ nQueue ].size() );
				OutputDebugString( buf );
			}
#endif
		}
	}

	bool Count( TCost _maxCost = 0 )
	{
		// initializing
		for ( int i = 0; i < MAX_MOVE_COST+1; ++i )
			queue[i].clear();
		maxCost = _maxCost;
		
		// somehow make table have infinite values
		table->MakeInfinity();

		// adding first TPosition
		(*table)[position] = SMoveInfo<TPosition, TCost>(position, 0); // parent is the same TPosition and cost is zero
		queue[0].push_back( position );
		currentPrior = 0;

		// Main loop
		while (1)
		{
			// pop next TPosition as current
			//	get next non-empty queue as current prior; if no any, return
			int oldPrior = currentPrior;
			while ( queue[currentPrior].empty() ) 
			{				
				currentPrior++;
				if (currentPrior > MAX_MOVE_COST)
					currentPrior -= (MAX_MOVE_COST+1);
				if (oldPrior == currentPrior) 
					return false; // no non-empty queues
			}
			current = queue[currentPrior].back();
			queue[currentPrior].pop_back();
			
			if ( constraints->IsFinal( current ) )
			{
				firstReached = current;
				return true; // final point reached
			}

		  currentCost = (*table)[current].cost;
			// for each move in TMap beginning with this TPosition, apply operator() ...
			pMap->ForEachMove(current, *this);
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif