#include "StdAfx.h"
#include "RPGGame.h"
#include "aiMap.h"
#include "aiRender.h"
#include "..\Misc\RandomGen.h"
#include "wTSFlags.h"
#include "wInterface.h"
#include "RPGUnitMission.h"
#include "..\DBFormat\DataRPG.h"
#include "..\Misc\LogStream.h"
#include "aiGrid.h"
#include "RPGVision.h"

//#define OUTPUT_GRID_TO_FILE

const float F_TILE_VIEW_BOUND = .625f;			// Ширина кубика, для которого мы считаем кавер

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int
	N_WEAPONTRAIL_MAXDISTANCE = 30;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CCoverInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
class CCoverInfo: public CObjectBase
{
	OBJECT_BASIC_METHODS( CCoverInfo );
public:
	struct SRay
	{
		CVec3 ptDir;		// направление откланения
		bool  isPenetrate;
		float fDeviation;	// величина отклонение от идеального попадания
	};
	ZDATA
	vector<SRay> hitRays;	// набор попавших лучей
	CVec3 src;				// точка откуда стреляют
	vector<SRay> looseRays;	// набор промахнувшихся лучей
	vector<SRay> obstRays; // набор лучей, которые уперлись в препятствие на определенном расстоянии от исходной точки
	float fSummAPA;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&hitRays); f.Add(3,&src); f.Add(4,&looseRays); f.Add(5,&obstRays); f.Add(6,&fSummAPA); return 0; }
};
static bool operator < ( const CCoverInfo::SRay &right, const CCoverInfo::SRay &left ) { return right.fDeviation < left.fDeviation; }
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CanShoot( CCoverInfo *pCover )
{
	return !pCover->hitRays.empty() || !pCover->looseRays.empty(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGame
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGame: public IGame
{
	OBJECT_BASIC_METHODS( CGame );
	ZDATA
	CPtr<NAI::IAIMap> pAIMap;
	CVisionTracker vision;
	CPtr<NAI::IPathNetwork> pNet;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pAIMap); f.Add(3,&vision); f.Add(4,&pNet); return 0; }
public:
	CGame() {}
	CGame( NAI::IAIMap *_pAIMap, NAI::IPathNetwork *_pNet ): pAIMap(_pAIMap), pNet(_pNet), vision( _pAIMap ) {}
	virtual CCoverInfo* CalcCovers( const CVec3 &src, const CAttackPortion &attack, 
		NWorld::CUnit *pIgnore, NWorld::CUnit *pDest, int nTargetUserID, float fMinClearDistance, bool bAIMode = false );
	virtual CCoverInfo* CalcCoversForTile( const CVec3 &src, const CAttackPortion &attack, NWorld::CUnit *pIgnore,
		const CVec3 &ptTarget, float fMinClearDistance );
	virtual void ProcessMeleeAttackPortion( const CAttackPortion &a, const CRay &ray, const vector<IAttackable*> &ignores );
	virtual void ProcessRangedAttackPortion( const CAttackPortion &a, const CRay &ray, const vector<IAttackable*> &ignores, vector<STrailPoint> *pTrail );
	virtual EAttackResult ProcessThrowingAttackPortion( CAttackPortion *pA, IAttackable *pTarget, NDb::CRPGArmor *pArmor, int nUserID );
	virtual int GetCompositeToHit( NWorld::CUnit *pAttacker, NWorld::CUnit *pTarget, NAI::EHitLocation eHL, bool bFirstTurn );
	virtual int GetGrenadeCompositeToHit( NWorld::CUnit *pAttacker, 
		CVec3 ptTarget, bool bFirstTurn, NDb::CRPGGrenade *pGrenade );
	virtual int GetTileCompositeToHit(  NWorld::CUnit *pAttacker, CVec3 ptTilePos, 
		NAI::ETileHitLocation eHitLocation, bool bFirstTurn );
	virtual int GetBazookaToHit(  NWorld::CUnit *pAttacker, CVec3 ptTilePos, 
		NAI::ETileHitLocation eHitLocation, bool bFirstTurn );
	virtual bool CheckVisibility( const NWorld::CUnit *pObserver, const NWorld::CUnit *pDest );
	virtual bool CheckPositionVisibility( const NAI::SUnitPosition observerPos, const NAI::SPosition targetPos );
	virtual bool CheckAIPositionVisibility( const NAI::SUnitPosition observerPos, const NAI::SPosition targetPos );
	virtual void GetVisibilityArea( vector<SVisibilitySpot> *pRes, const NWorld::CUnit *pObserver );
	virtual int GetCoverForAIUnit( CVec3 ptFrom, NWorld::CUnit *pIgnore, 
		NWorld::CUnit *pTarget, const NRPG::CAttackPortion &AttackPortion, NAI::EHitLocation HitLocation );

	virtual CVec3 GetIllumination( const vector<CVec3> &unit ) { return CVec3(1,1,1); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static CVec3 GetRandomSpherePoint() // CRAP - move somewhere to util package
{
	float fpX, fpY, fpZ, fpLeng;
	for ( int nSafe = 10; nSafe > 0; nSafe-- )
	{
		fpX = random.GetFloat( -1, 1 );
		fpY = random.GetFloat( -1, 1 );
		fpZ = random.GetFloat( -1, 1 );
		fpLeng = sqr( fpX ) + sqr( fpY ) + sqr( fpZ );
		if ( fpLeng < 1 )
			break;
	}
	return CVec3( fpX, fpY, fpZ ) / sqrt( fpLeng );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AddGridToCovers( CCoverInfo *pRes, const NAI::CFastRenderer &res, const CVec3 &vTargetDir,
	const CObjectBase *pIgnore, const CObjectBase *pTarget, int nTargetUserID, 
	float fDistance, const CAttackPortion &att, float fMinClearDistance )
{
	float _fArmorPiercingAbility = att.nK;
#ifdef OUTPUT_GRID_TO_FILE
	FILE *f;
	if ( res.resGrid.GetXSize() > 10 )
		f = fopen( "cover_l.txt", "wt" );
	else
		f = fopen( "cover_s.txt", "wt" );
	fprintf( f, "Legend:\n# - hit\n. - too close to shoot\n? - not penetrate\n" );
	for ( int i = 0; i < res.resGrid.GetXSize(); ++i )
		fputc( '-', f );
	fputc( '\n', f );
	for ( int y = res.resGrid.GetYSize() - 1; y >= 0; --y )
#else
	for ( int y = 0; y < res.resGrid.GetYSize(); ++y )
#endif
	{
		for ( int x = 0; x < res.resGrid.GetXSize(); ++x )
		{
			CVec3 ptRayDir;
			res.GetDir( &ptRayDir, x, y );
			CCoverInfo::SRay ray;
			ray.isPenetrate = false;
			ray.ptDir = ptRayDir;
			float fRayProjection = vTargetDir * ptRayDir;
			ray.fDeviation = sqr( fRayProjection );
			//const vector<NAI::SSimpleInterval> &intersect = res.grid[y][x];
			bool bHitTarget = false;
			bool bObstacle = false;
			for ( NAI::CFastRenderer::SResult *p = res.resGrid[y][x]; p; p = p->pNext )
			{
				if ( (p->GetInfo().nTSFlags & NWorld::TS_WEAPON_BLOCKER) && p->GetInfo().pUserData != pIgnore && p->fEnter < fMinClearDistance )
				{
					bObstacle = true;
					break;
				}
			}
			if ( bObstacle )
			{
				pRes->obstRays.push_back(ray);
#ifdef OUTPUT_GRID_TO_FILE
				fputc( '.', f );
#endif
				continue;
			}
			for ( NAI::CFastRenderer::SResult *p = res.resGrid[y][x]; p; p = p->pNext )
			{
				if ( pTarget ) // shoot unit
				{
					if ( nTargetUserID != -1 )
					{
						if ( p->GetInfo().pUserData == pTarget && p->pSrc->nUserID == nTargetUserID )
							bHitTarget = true;
					}
					else
					{
						if ( p->GetInfo().pUserData == pTarget )
							bHitTarget = true;
					}
				}
				else
					bHitTarget = true;
			}
			if ( !bHitTarget )
			{
				pRes->looseRays.push_back(ray);
#ifdef OUTPUT_GRID_TO_FILE
				fputc( ' ', f );
#endif
				continue;
			}
			float fTestDistance = fDistance / fRayProjection;
			float fTempPiercing = _fArmorPiercingAbility;

			vector<const CObjectBase*> ignore;
			ignore.push_back( pIgnore );
			for ( const NAI::CFastRenderer::SResult *p = res.resGrid[y][x]; p; p = p->pNext )
			{
				if ( p->fExit < 0 )
					continue;
				if ( find( ignore.begin(), ignore.end(), p->GetInfo().pUserData ) != ignore.end() )
					continue;
				CDynamicCast<NWorld::CUnit> pTargetUnit( p->GetInfo().pUserData );
				if ( pTargetUnit )
					ignore.push_back( p->GetInfo().pUserData );
				if ( p->fEnter < fTestDistance )
				{
					float fExit = p->fExit;
					float fEnter = Max( p->fEnter, 0.0f );
					NDb::CRPGArmor *pArmor = p->GetInfo().pArmor;
					if ( !pArmor )
						pArmor = NDb::GetArmor( NDb::CRPGArmor::WOOD ); // CRAP, should be DEFAULT
					if ( !att.IsArmorIgnored(pArmor) )
					{
						if ( !att.CanDealDmg(pArmor) )
							break;
						if ( p->GetInfo().pUserData == pTarget )
						{
							pRes->fSummAPA += fTempPiercing;
							ray.isPenetrate = true;
						}
					}
					fTempPiercing -= (fExit - fEnter) * pArmor->fDensity;
					// Пуля застряла и дальше вредить не нужно
					if ( fTempPiercing <= 0 )
						break;
				}
			}
			if ( !pTarget && fTempPiercing > 0 ) // shoot tile
			{
				ray.isPenetrate = true;
				pRes->fSummAPA += fTempPiercing;
			}
			pRes->hitRays.push_back( ray );
#ifdef OUTPUT_GRID_TO_FILE
			fputc( ray.isPenetrate ? '#' : '?', f );
#endif
		}
#ifdef OUTPUT_GRID_TO_FILE
		fputc( '\n', f );
#endif
	}
#ifdef OUTPUT_GRID_TO_FILE
	for ( int i = 0; i < res.resGrid.GetXSize(); ++i )
		fputc( '-', f );
	fputc( '\n', f );
	fclose(f);
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CGame::GetCoverForAIUnit( CVec3 ptFrom, NWorld::CUnit *pIgnore, 
	NWorld::CUnit *pTarget, const NRPG::CAttackPortion &AttackPortion, NAI::EHitLocation HitLocation )
{
	int nHitCover = 0;
	CObj<NRPG::CCoverInfo> pCover = CalcCovers( ptFrom, 
		AttackPortion, pIgnore, pTarget, HitLocation, 1.f, true );
	//
	int nPenetrateCount = 0;
	for ( vector<NRPG::CCoverInfo::SRay>::const_iterator i = pCover->hitRays.begin(); i != pCover->hitRays.end(); ++i )
		if ( i->isPenetrate )
			++nPenetrateCount;
	float fAverageAPA = ( pCover->fSummAPA / float(nPenetrateCount) ) / AttackPortion.nK;
	if ( !pCover->hitRays.empty() )
	{
		nHitCover = (100 * nPenetrateCount) / pCover->hitRays.size();
		nHitCover = int( nHitCover * fAverageAPA );
	}
	return nHitCover;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// nTargetUserID == -1 means without hit location targeting
CCoverInfo* CGame::CalcCovers( const CVec3 &src, const CAttackPortion &attack, NWorld::CUnit *pIgnore,
	NWorld::CUnit *pDest, int nTargetUserID, float fMinClearDistance, bool bAIMode )
{
	const float F_VIEW_BOUND = 2.f;			// Диаметр сетки
	const int N_LOW_HALF_GRID = 5;
	int N_HALF_GRID;
	if ( !bAIMode )
		N_HALF_GRID = 20;
	else
		N_HALF_GRID = 8;
	// trace some rays and calc covers
	CCoverInfo *pRes = new CCoverInfo;
	pRes->fSummAPA = 0;
	CVec3 ptTarget;
	pAIMap->GetUnitHLPos( &ptTarget, pDest, nTargetUserID );//pDest->GetPosition().GetCenter();
	CVec3 ptFrom = src;
	CVec3 vTargetDir = ptTarget - ptFrom;
	float fXYDistance = fabs( vTargetDir.x, vTargetDir.y );
	float fDistance = fabs( vTargetDir ) + 1; // +1 to make sure whole target is considered
	Normalize( &vTargetDir );

	float fSquareLimit = fXYDistance * (float)tan(FP_PI8);
	CVec2 viewSquare;
	viewSquare.x = Min( F_VIEW_BOUND * 0.5f, fSquareLimit );
	viewSquare.y = F_VIEW_BOUND * 0.5f;
	NAI::CFastRenderer res, resLow;
	res.InitProjective( src, ptTarget, viewSquare, N_HALF_GRID );
	pAIMap->TraceGrid( &res, NWorld::TS_COVER, NAI::IAIMap::STH_SORT_INTERVALS );
	AddGridToCovers( pRes, res, vTargetDir, pIgnore->GetAttackIgnore(), CastToObjectBase(pDest), nTargetUserID, fDistance, attack, fMinClearDistance );
	// add low res grid
	if ( !bAIMode )
	{
		viewSquare.x = fSquareLimit;
		viewSquare.y = Max( F_VIEW_BOUND * 0.5f, fSquareLimit );
		resLow.InitProjective( src, ptTarget, viewSquare, N_LOW_HALF_GRID );
		pAIMap->TraceGrid( &resLow, NWorld::TS_COVER, NAI::IAIMap::STH_SORT_INTERVALS );
		AddGridToCovers( pRes, resLow, vTargetDir, pIgnore->GetAttackIgnore(), CastToObjectBase(pDest), nTargetUserID, fDistance, attack, fMinClearDistance );
	}
	pRes->src = src;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCoverInfo* CGame::CalcCoversForTile( const CVec3 &src, const CAttackPortion &attack, NWorld::CUnit *pIgnore,
	const CVec3 &ptTarget, float fMinClearDistance )
{
	const int N_LOW_HALF_GRID = 5;
	// trace some rays and calc covers
	CCoverInfo *pRes = new CCoverInfo;
	pRes->fSummAPA = 0;
	CVec3 ptFrom = src;
	CVec3 vTargetDir = ptTarget - ptFrom;
	float fXYDistance = fabs( vTargetDir.x, vTargetDir.y );
	float fDistance = fabs( vTargetDir ) + 1; // +1 to make sure whole target is considered
	Normalize( &vTargetDir );

	float fSquareLimit = fXYDistance * (float)tan(FP_PI8);
	CVec2 viewSquare;
	NAI::CFastRenderer resLow;
	viewSquare.x = F_TILE_VIEW_BOUND * 0.5f;
	viewSquare.y = F_TILE_VIEW_BOUND * 0.5f;
	resLow.InitProjective( src, ptTarget, viewSquare, N_LOW_HALF_GRID );
	pAIMap->TraceGrid( &resLow, NWorld::TS_COVER, NAI::IAIMap::STH_SORT_INTERVALS );
	AddGridToCovers( pRes, resLow, vTargetDir, pIgnore->GetAttackIgnore(), 0, -1, fDistance, attack, fMinClearDistance );
	pRes->src = src;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGame::ProcessMeleeAttackPortion( const CAttackPortion &a, const CRay &ray, const vector<IAttackable*> &ignores )
{
	// every unit/object on path receives damage
	vector<NAI::SInterval> intersect;
	vector<IAttackable*> ignore;
	for ( vector<IAttackable*>::const_iterator it = ignores.begin(); it != ignores.end(); ++it )
		ignore.push_back( *it );
	CAttackPortion tmpAttackPortion( a );
	pAIMap->Trace( ray, &intersect, NWorld::TS_FRAGMENTED );

	for ( vector<NAI::SInterval>::iterator i = intersect.begin(); i != intersect.end(); ++i )
	{
		if ( i->enter.fT > 0 )	// Так как на самом деле это не луч, а прямая
		{
			CDynamicCast<IAttackable> pAttackCatcher( i->pSrc->pUserData );
			if ( pAttackCatcher )
			{
				if ( find( ignore.begin(), ignore.end(), pAttackCatcher ) != ignore.end() )
					continue;
				CDynamicCast<NWorld::CUnit> pTargetUnit( i->pSrc->pUserData );
				if ( IsValid( pTargetUnit ) )
				{
					// Попробуем поиграть в суперменов. "Читеры , они и в африке читеры ;)"
					if ( pTargetUnit->GetPlayer()->IsCheatEnabled( NWorld::CHEAT_GODMODE ) )
						continue;
					ignore.push_back(pAttackCatcher);
				}
			}
			NDb::CRPGArmor *pArmor = i->pSrc->pArmor;
			if ( !pArmor )
				pArmor = NDb::GetArmor( NDb::CRPGArmor::WOOD ); // CRAP, should be DEFAULT
			if ( !tmpAttackPortion.IsArmorIgnored(pArmor) && !tmpAttackPortion.CanDealDmg(pArmor) )
				return;
			// подсчитываем наносимые повреждения
			if ( pAttackCatcher )
				pAttackCatcher->ProcessAttack( i->nUserID, &tmpAttackPortion, pArmor );
			tmpAttackPortion.nK -= (i->exit.fT - i->enter.fT) * pArmor->fDensity;
			//sTrail.explosions.push_back( SWound( i->pUserData, ray.Get( i->enter.fT ), -ray.ptDir, pArmor ) );
			if ( tmpAttackPortion.nK <= 0 )
				return;	// Пуля застряла и дальше вредить не нужно
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGame::ProcessRangedAttackPortion( const CAttackPortion &a, const CRay &ray, const vector<IAttackable*> &ignores, vector<STrailPoint> *pTrail )
{
	// every unit/object on path receives damage
	vector<NAI::SInterval> intersect;
	vector<IAttackable*> ignore;
	for ( vector<IAttackable*>::const_iterator it = ignores.begin(); it != ignores.end(); ++it )
		ignore.push_back( *it );
	CAttackPortion tmpAttackPortion( a );
	pAIMap->Trace( ray, &intersect, NWorld::TS_FRAGMENTED );

	pTrail->clear();
	pTrail->push_back( STrailPoint( 0, ray.ptDir, ray.ptOrigin, tmpAttackPortion, 0, 0 ) );

	for ( vector<NAI::SInterval>::iterator i = intersect.begin(); i != intersect.end(); ++i )
	{
		if ( i->enter.fT > 0 )	// Так как на самом деле это не луч, а прямая
		{
			CDynamicCast<IAttackable> pAttackCatcher( i->pSrc->pUserData );
			if ( pAttackCatcher )
			{
				if ( find( ignore.begin(), ignore.end(), pAttackCatcher ) != ignore.end() )
					continue;
				CDynamicCast<NWorld::CUnit> pTargetUnit( i->pSrc->pUserData );
				if ( IsValid( pTargetUnit ) )
				{
					// Попробуем поиграть в суперменов. "Читеры , они и в африке читеры ;)"
					if ( pTargetUnit->GetPlayer()->IsCheatEnabled( NWorld::CHEAT_GODMODE ) )
						continue;
					ignore.push_back(pAttackCatcher);
				}
			}
			NDb::CRPGArmor *pArmor = i->pSrc->pArmor;
			if ( !pArmor )
				pArmor = NDb::GetArmor( NDb::CRPGArmor::WOOD ); // CRAP, should be DEFAULT
			if ( !tmpAttackPortion.IsArmorIgnored(pArmor) )
			{
				pTrail->push_back( STrailPoint( i->nUserID, ray.ptDir, ray.Get( i->enter.fT ), tmpAttackPortion, i->pSrc->pUserData, pArmor ) );
				if ( !tmpAttackPortion.CanDealDmg(pArmor) )
					return;
			}
			tmpAttackPortion.nK -= (i->exit.fT - i->enter.fT) * pArmor->fDensity;
			if ( tmpAttackPortion.nK <= 0 )
				return;	// Пуля застряла и дальше вредить не нужно
		}
	}
	pTrail->push_back( STrailPoint( 0, ray.ptDir, ray.Get( N_WEAPONTRAIL_MAXDISTANCE ), tmpAttackPortion, 0, 0 ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EAttackResult CGame::ProcessThrowingAttackPortion( CAttackPortion *pA, IAttackable *pTarget, NDb::CRPGArmor *pArmor, int nUserID )
{
	if ( pArmor == NDb::GetArmor( NDb::CRPGArmor::HUMAN_BODY ) )
	{
		pTarget->ProcessAttack( nUserID, pA, pArmor );
		return AR_BOUNCE_BODY;
	}
	// Листва
	if ( pArmor->nDR == 10 )
		return AR_IGNORE;
	// Стекло
	if ( pArmor->nDR == 0 )
	{
		pTarget->ProcessAttack( nUserID, pA, pArmor );
		return AR_IGNORE;
	}
	// Дерево
	if ( pArmor->nDR == 1 || pArmor->nDR == 2 )
		return AR_STUCK;
	return AR_BOUNCE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static SRand rand;
////////////////////////////////////////////////////////////////////////////////////////////////////
bool PeekRayForRocket( CCoverInfo *pCover, CRay *pRes, bool bHit )
{
	float fHitFlag;
	if ( ( bHit && !pCover->hitRays.empty() ) || pCover->looseRays.empty() )
		fHitFlag = 2.f;
	else
		fHitFlag = 0.f;
	bool bIsMiss;
	return PeekRay( pCover, pRes, fHitFlag, &bIsMiss );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool PeekRay( CCoverInfo *pCover, CRay *pRes, float fHit, bool *bIsMiss, bool bStickTo_pRes )
{
	*bIsMiss = true;
	pRes->ptOrigin = pCover->src;
	float fMaxAngle = cos(ToRadian(8.f));
	if ( fHit > 1 )
	{
		ASSERT( pCover->hitRays.size() > 0 );	// Это супер странно, так как cover предаётся сюда только когда набрано минимум 20 лучиков
		if ( pCover->hitRays.empty() )
			return false;

		CRoulette roulette;
		for ( vector<CCoverInfo::SRay>::const_iterator i = pCover->hitRays.begin(); i != pCover->hitRays.end(); ++i )
		{
			float fK = i->fDeviation;
			if ( bStickTo_pRes )
			{
				fK = pRes->ptDir * i->ptDir;
				fK *= fK * fK;
				if ( fK < fMaxAngle )
					fK /= 1000;
			}
			if ( i->isPenetrate )
			{
				fK *= fHit;
				*bIsMiss = false;
			}
			roulette.AddSector(fK);
		}
		if ( *bIsMiss )
			csRPG << CC_RED << "\tCan't hit target!\n";

		pRes->ptDir = pCover->hitRays[roulette.GetRandomSector( &rand )].ptDir;
	}
	else
	{
		ASSERT( pCover->looseRays.size() > 0 );	// Тоже что и выше
		if ( pCover->looseRays.empty() )
			return false;

		CRoulette roulette;
		sort( pCover->looseRays.begin(), pCover->looseRays.end() ); // сортируем от худший к лучшим
		float fStep = 1.f / float( pCover->looseRays.size() );
		int n = 0;
		for ( vector<CCoverInfo::SRay>::const_iterator i = pCover->looseRays.begin(); i != pCover->looseRays.end(); ++i, ++n )
		{
			float fK = sqr( fHit - fabs( float(n) * fStep - fHit ) * i->fDeviation );
			if ( bStickTo_pRes )
			{
				fK = pRes->ptDir * i->ptDir;
				fK *= fK * fK;
				if ( fK < fMaxAngle )
					fK /= 1000;
			}
			roulette.AddSector(fK);
		}
		int nPick = roulette.GetRandomSector( &rand );
//		csRPG << "\tMiss index " << nPick << ", average = " << int( float(n) * fHit ) << " from " << n << "\n";
		pRes->ptDir = pCover->looseRays[nPick].ptDir;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetHitCover( const NWorld::CUnit *pAttacker, CCoverInfo *pCover )
{
	ASSERT(pCover);
	//
	int nPenetrateCount = 0;
	for ( vector<CCoverInfo::SRay>::const_iterator i = pCover->hitRays.begin(); i != pCover->hitRays.end(); ++i )
		if ( i->isPenetrate )
			++nPenetrateCount;
	//
	CDynamicCast<NRPG::IUnitMission> pRealAttacker( pAttacker->GetRPG() );
	vector<NRPG::CAttackPortion> attack;
	pRealAttacker->CreateAttack( &attack, false );
	float fAverageAPA = ( pCover->fSummAPA / float(nPenetrateCount) ) / attack.front().nK;
	// процент попавших лучей
	int nHitCover = 0;
	if ( !pCover->hitRays.empty() )
	{
		nHitCover = (100 * nPenetrateCount) / pCover->hitRays.size();
		nHitCover = int( float(nHitCover) * fAverageAPA );
	}
	return nHitCover;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetAttackerTileToHit( const NWorld::CUnit *pAttacker, const CVec3 ptTarget, int nExtraAP,
	NAI::ETileHitLocation eHitLocation, CCoverInfo *pCover, bool bFirstRound )
{
	int nHitCover = GetHitCover( pAttacker, pCover );
	int nDistance = fabs(pAttacker->GetPosition().GetCP() - ptTarget ) / FP_GRID_STEP;
	CVec3 ptAttacker = pAttacker->GetPosition().GetCenter();
	//
	return pAttacker->GetRPG()->GetTileToHit( pAttacker->GetPose(), nDistance, ptAttacker, 
		ptTarget, eHitLocation, nExtraAP, nHitCover, bFirstRound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int GetAttackerToHit( const NWorld::CUnit *pAttacker, const NWorld::CUnit *pTarget, int nExtraAP,
	NAI::EHitLocation eHL, const vector<int> &accessibleHLs, CCoverInfo *pCover, bool bFirstRound )
{
	ASSERT(pCover);
	//
	int nHitCover = GetHitCover( pAttacker, pCover );
	int nDistance = fabs(pAttacker->GetPosition().GetCP() - pTarget->GetPosition().GetCP()) / FP_GRID_STEP;
	// CRAP: Этот параметр уже нигде не использется
	CVec3 ptAttacker = pAttacker->GetPosition().GetCenter();
	//
	return pAttacker->GetRPG()->GetToHit( pAttacker->GetPose(), nDistance, ptAttacker, 
		pTarget->GetPosition().pos, eHL, nExtraAP,
		pTarget->GetRPG(), accessibleHLs, nHitCover, bFirstRound );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Using only for inteface tasks. Returns not exactly correct value.
int CGame::GetCompositeToHit( NWorld::CUnit *pAttacker, 
	NWorld::CUnit *pTarget, NAI::EHitLocation eHL, bool bFirstTurn )
{
	vector<NRPG::CAttackPortion> attack;
	CDynamicCast<NRPG::IUnitMission> pRealAttacker( pAttacker->GetRPG() );
	ASSERT( pRealAttacker );
	pRealAttacker->PrintLog( false );
	pRealAttacker->CreateAttack( &attack, false );
	if ( attack.empty() )
	{
		pRealAttacker->PrintLog( true );
		//ASSERT(0);
		return 0;
	}
	CVec3 ptAttackPos;
	if ( NDb::IsMeleeWeapon( pRealAttacker->GetWeaponType() ) )
	{
		CVec3 pTargetHL;
		pAIMap->GetUnitHLPos( &pTargetHL, pTarget, eHL );
		ptAttackPos = GetMeleeAttackPos( pAttacker, pTargetHL );
	}
	else
	{
		NAI::SUnitPosition pos = pAttacker->GetPosition();
		NAI::EDirection dir = GetShootDirection( pos.pos.pNet, pos.pos.p, pTarget->GetPosition().GetCP() );
		pos.pos.p.SetDirection( dir );
		ptAttackPos = pAttacker->GetAttackOrigin( pos );
	}
	CObj<NRPG::CCoverInfo> pCover = CalcCovers( ptAttackPos, attack.front(), pAttacker, pTarget, eHL, pAttacker->GetMinClearDistance() );
	vector<int> accessibleHLs; // необходимо только для вычисления Melee ToHit
	if ( NDb::IsMeleeWeapon( pRealAttacker->GetWeaponType() ) )
	{
		pAIMap->GetAccessibleUnitHL( &accessibleHLs, pAttacker->GetPosition().GetCenter(), pAttacker, F_MELEE_DISTANCE );
		if ( NAI::HL_ANY != eHL && find( accessibleHLs.begin(), accessibleHLs.end(), eHL ) != accessibleHLs.end() )
		{
			accessibleHLs.clear();
			accessibleHLs.push_back( eHL );
		}
		if ( accessibleHLs.empty() )
			return 0;
	}
	int nToHit = 0;
	int nRof = pRealAttacker->GetBulletsQuantityInShot();
	pRealAttacker->StartAttack();
	for ( int i = 0; i < nRof; ++i )
	{
		nToHit += GetAttackerToHit( pAttacker, pTarget, pAttacker->GetCarefulShotExtraAP(), 
			eHL, accessibleHLs, pCover, bFirstTurn );
		pRealAttacker->NextBullet();
	}
	nToHit /= nRof;
	pRealAttacker->PrintLog( true );
	return nToHit;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CGame::GetGrenadeCompositeToHit( NWorld::CUnit *pAttacker, 
	CVec3 ptTarget, bool bFirstTurn, NDb::CRPGGrenade *pGrenade )
{
	int nDistance = fabs( pAttacker->GetPosition().GetCP() - ptTarget );

	return pAttacker->GetRPG()->GetGrenadeToHit( pAttacker->GetPose(), 
		nDistance, pAttacker->GetPosition().GetCP(), bFirstTurn, ptTarget, pGrenade );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CGame::GetTileCompositeToHit(  NWorld::CUnit *pAttacker, CVec3 ptTilePos, 
		NAI::ETileHitLocation eHitLocation, bool bFirstTurn )
{
	int nDistance = fabs( pAttacker->GetPosition().GetCP() - ptTilePos );
	//
	CDynamicCast<NRPG::IUnitMission> pRPG( pAttacker->GetRPG() );
	vector<NRPG::CAttackPortion> Attack;
	pRPG->CreateAttack( &Attack, false );
	//
	CVec3 ptFrom = pAttacker->GetAttackOrigin( pAttacker->GetPosition() );
	//
	CObj<NRPG::CCoverInfo> pCover = CalcCoversForTile( ptFrom, Attack[0], pAttacker,
		ptTilePos, pAttacker->GetMinClearDistance() );
	int nHitCover = GetHitCover( pAttacker, pCover );
	//
	return pAttacker->GetRPG()->GetTileToHit( pAttacker->GetPose(), nDistance, pAttacker->GetPosition().GetCP(), 
		ptTilePos, eHitLocation, pAttacker->GetCarefulShotExtraAP(), nHitCover, bFirstTurn );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CGame::GetBazookaToHit(  NWorld::CUnit *pAttacker, CVec3 ptTilePos, 
		NAI::ETileHitLocation eHitLocation, bool bFirstTurn )
{
	int nDistance = fabs( pAttacker->GetPosition().GetCP() - ptTilePos );
	//
	CDynamicCast<NRPG::IUnitMission> pRPG( pAttacker->GetRPG() );
	vector<NRPG::CAttackPortion> Attack;
	pRPG->CreateAttack( &Attack, false );
	CVec3 ptFrom = pAttacker->GetAttackOrigin( pAttacker->GetPosition() );
	//
	return pAttacker->GetRPG()->GetRLauncherToHit( pAttacker->GetPose(), nDistance, pAttacker->GetPosition().GetCP(), 
		ptTilePos, eHitLocation, pAttacker->GetCarefulShotExtraAP(), bFirstTurn );
}
int GetRandomForToHit()
{
	static int nLastRndForToHit = 200;
	int nRnd;
	while ( abs( (nRnd = random.Get(0,100)) - nLastRndForToHit ) < 6 );
	return nLastRndForToHit = nRnd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CheckToHit( const NWorld::CUnit *pAttacker, const NWorld::CUnit *pTarget, int nExtraAP, NAI::EHitLocation eHL, 
	const vector<int> &accessibleHLs, CCoverInfo *pCover, bool bFirstRound, int *nToHit )
{
	*nToHit = GetAttackerToHit( pAttacker, pTarget, nExtraAP, eHL, accessibleHLs, pCover, bFirstRound );
	csRPG << "\tToHit = " << *nToHit;
	int nCheck = GetRandomForToHit();
	csRPG << "\tCheck = " << nCheck << " Hit: " << bool(nCheck < *nToHit) << "\n";
	return float(*nToHit) / float(nCheck);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float CheckTileToHit( const NWorld::CUnit *pAttacker, const CVec3 ptTarget, int nExtraAP,
	NAI::ETileHitLocation eHitLocation, CCoverInfo *pCover, bool bFirstRound, int *nToHit )
{
	*nToHit = GetAttackerTileToHit( pAttacker, ptTarget, nExtraAP, eHitLocation, pCover, bFirstRound );
	csRPG << "\tTileToHit = " << *nToHit;
	int nCheck = GetRandomForToHit();
	csRPG << "\tCheck = " << nCheck << " Hit: " << bool(nCheck < *nToHit) << "\n";
	return float(*nToHit) / float(nCheck);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NAI::EDirection GetShootDirection( NAI::IPathNetwork *pNet, const NAI::SPathPlace &from, const CVec3 &ptTarget )
{
	NAI::SPosition fromPos;
	fromPos.p = from;
	fromPos.SetNetwork( pNet );
	CVec3 ptDir( ptTarget - fromPos.GetCP() );
	float fAngle = atan2( ptDir.y, ptDir.x );
	return pNet->GetClosestDir( from.GetLayer(), fAngle );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 GetMeleeAttackPos( const NWorld::CUnit *pAttacker, const CVec3 &ptTarget )
{
	const NAI::SUnitPosition &pos = pAttacker->GetPosition();
	CVec3 ptAttackPos = pos.GetEyePosition();
	NAI::EBlowHeight eBH = NAI::GetBlowHeight( pos, ptTarget );
	switch ( eBH )
	{
		case NAI::BH_TOP:
			break;
		case NAI::BH_MIDDLE:
			ptAttackPos = pos.GetCenter();
			break;
		case NAI::BH_BOTTOM:
			ptAttackPos = pos.GetCP();
			break;
	}
	return ptAttackPos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// VISION
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
static CVec3 GetForwardDir( const NAI::SUnitPosition &observerPos )
{
	float fAngle = observerPos.GetDirection();
	CVec3 ptDir( cos(fAngle), sin(fAngle), 0 );
	return ptDir;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_LOWER_CUBE_DH = 0.5f;
const float F_NEXT_CUBE_DH = 0.5f;
void CGame::GetVisibilityArea( vector<SVisibilitySpot> *pRes, const NWorld::CUnit *pObserver )
{
	CVec3 ptFwdDir = GetForwardDir( pObserver->GetPosition() );
	const CVec3 ptFrom = pObserver->GetPosition().GetEyePosition();
	//CObj<NAI::CAIVisionCube> pCube = pAIMap->GetVisionCube( ptFrom );
	vector<NAI::SPathPlace> testPlaces;
	pNet->GetNearPlaces( SSphere( ptFrom, N_SIGHTDISTANCE ), &testPlaces );
	pRes->clear();
	for ( int k = 0; k < testPlaces.size(); ++k )
	{
		NAI::SPosition pos;
		pos.p = testPlaces[k];
		pos.SetNetwork( pNet );
		CVec3 ptTest = pos.GetCP();
		ptTest += CVec3( 0,0,F_LOWER_CUBE_DH );
		if ( fabs2( ptTest - ptFrom ) > sqr( N_SIGHTDISTANCE ) )
			continue;
		int nRes = 0;
		for ( int k = 0; k < 3; ++k )
		{
			CVec3 ptTestPoint = ptTest + CVec3(0,0,F_NEXT_CUBE_DH * k );
			if ( vision.IsCubeVisible( ptFrom, ptTestPoint, ptFwdDir ) )
				nRes = k + 1;
			else
				pRes->push_back( SVisibilitySpot( ptTestPoint, 0 ) ); // designers asked for it
		}
		//pRes->push_back( SVisibilitySpot( ptTest, nRes ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetOccupiedCubes( vector<CVec3> *pRes, const NAI::SPosition &pos )
{
	CVec3 ptBase = pos.GetCP() + CVec3( 0,0,F_LOWER_CUBE_DH );
	switch ( pos.p.GetPose() )
	{
	case NAI::CRAWL:
		{
			float fAngle = pos.GetDirection();
			CVec3 ptShift( cos( fAngle ) * FP_GRID_STEP, sin( fAngle ) * FP_GRID_STEP, 0 );
			pRes->push_back( ptBase );
			pRes->push_back( ptBase + ptShift );
			pRes->push_back( ptBase - ptShift );
		}
		break;
	case NAI::CROUCH:
		pRes->push_back( ptBase );
		pRes->push_back( ptBase + CVec3(0,0,F_NEXT_CUBE_DH * 1 ) );
		break;
	case NAI::WALK:
	case NAI::RUN:
		pRes->push_back( ptBase );
		pRes->push_back( ptBase + CVec3(0,0,F_NEXT_CUBE_DH * 1 ) );
		pRes->push_back( ptBase + CVec3(0,0,F_NEXT_CUBE_DH * 2 ) );
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GetTileOccupiedCubes( vector<CVec3> *pRes, const CVec3 &ptPos, NAI::ETileHitLocation eHitLocation )
{
	CVec3 ptBase = ptPos;
	switch ( eHitLocation )
	{
	case NAI::THL_LOWER:
		pRes->push_back( ptBase );
		break;
	case NAI::THL_MIDDLE:
		pRes->push_back( ptBase + CVec3(0,0,F_NEXT_CUBE_DH * 1 ) );
		break;
	case NAI::THL_UPPER:
		pRes->push_back( ptBase + CVec3(0,0,F_NEXT_CUBE_DH * 2 ) );
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGame::CheckVisibility( const NWorld::CUnit *pObserver, const NWorld::CUnit *pDest )
{
	return CheckPositionVisibility( pObserver->GetPosition(), pDest->GetPosition().pos );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGame::CheckPositionVisibility( const NAI::SUnitPosition observerPos, const NAI::SPosition targetPos )
{
	CVec3 ptFwdDir = GetForwardDir( observerPos );
	CVec3 ptFrom = observerPos.GetEyePosition();
	vector<CVec3> testPoints;
	GetOccupiedCubes( &testPoints, targetPos );
	for ( int k = 0; k < testPoints.size(); ++k )
	{
		if ( vision.IsCubeVisible( ptFrom, testPoints[k], ptFwdDir ) )
			return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGame::CheckAIPositionVisibility( const NAI::SUnitPosition observerPos, const NAI::SPosition targetPos )
{
	CVec3 ptFwdDir = GetForwardDir( observerPos );
	CVec3 ptFrom = observerPos.GetEyePosition();
	vector<CVec3> testPoints;
	GetOccupiedCubes( &testPoints, targetPos );
	return vision.IsCubeVisible( ptFrom, testPoints[1], ptFwdDir );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IGame* CreateGame( NAI::IAIMap *pAIMap, NAI::IPathNetwork *pNet )
{
	return new CGame( pAIMap, pNet );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NRPG;
REGISTER_SAVELOAD_CLASS( 0x02841121, CGame );
REGISTER_SAVELOAD_CLASS( 0x02841161, CCoverInfo );
BASIC_REGISTER_CLASS( IGame )
//BASIC_REGISTER_CLASS( IAttackable )