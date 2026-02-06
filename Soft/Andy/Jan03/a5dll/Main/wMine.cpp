#include "StdAfx.h"
#include "wMine.h"
#include "..\Misc\RandomGen.h"
#include "..\DBFormat\DataRPG.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataGeometry.h"
#include "wMain.h"
#include "Transform.h"
#include "wOSBase.h"
#include "scriptCallLua.h"
//
namespace NWorld
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMine
////////////////////////////////////////////////////////////////////////////////////////////////////
CMine::CMine( CWorld *_pWorld, const CVec3 &_vPlace, NDb::CRPGMine *_pMine, int _nDC, int _nFloor )
: pWorld(_pWorld), vPlace(_vPlace), pMine(_pMine), nDC(_nDC), nFloor(_nFloor)
{
	SRand rnd;
	pModel = pMine->pItem->pModel->CreateModel( &rnd );
	fAngle = random.GetFloat( 0, FP_2PI );
	bindGlobal.Link( pWorld->GetUnits(), this );
	pWorld->AddMine( this );
	pMineTracker = pWorld->GetMineTracker();
	pMineTracker->AddMine( this, vPlace );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CMine::~CMine()
{
	if ( IsValid(pMineTracker) )
		pMineTracker->RemoveMine( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMine::Visit( IRenderVisitor *p )
{
	SFBTransform pos;
	MakeMatrix( &pos, CVec3(1,1,1), vPlace, fAngle );
	p->AddMesh( pModel, pos, 0, nFloor, -1 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMine::Visit( IAIVisitor *p )
{
	SFBTransform pos;
	MakeMatrix( &pos, CVec3(1,1,1), vPlace, fAngle );
	int nMask = ( GetMask( pModel->pGeometry->pAIGeometry, pModel->pRPGArmor ) | TS_PICK ) & ~TS_PASS_BLOCKER;
	p->AddHull( pModel->pGeometry->pAIGeometry, pos, pModel->pRPGArmor, nFloor, nMask );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CMine::ProcessAttack( int nUserID, NRPG::CAttackPortion *pAttack, NDb::CRPGArmor *pArmor )
{
	GoBoom();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 CMine::GetMinePos() 
{ 
	return vPlace;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NDb::CRPGItem* CMine::DisarmMine() 
{ 
	NDb::CRPGMine *pRPGMine = pMine;
	CMObj<CMine> pHold(this);
	pHold = 0;
	return pRPGMine->pItem;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMine::GoBoom( CUnitServer *pWho )
{
	if ( !IsValid(this) )
		return;
	ASSERT( pMine->pExplosion );
	if ( pMine->pExplosion )
		pWorld->AddGrenadeExplosion( GetMinePos(), pMine->pExplosion );
	pWorld->RemoveMine( this );
	NScript::luaCallFunction( "OnMineTriggered", "p", IsValid( pWho ) ? CastToObjectBase( pWho ) : 0 );
	CMObj<CMine> pHold(this);
	pHold = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMineTracker
////////////////////////////////////////////////////////////////////////////////////////////////////
/*static NAI::SPathPlace GetNormalized( const NAI::SPathPlace &_place )
{
	NAI::SPathPlace place( _place.GetX(), _place.GetY(), _place.GetLayer() );
	return place;
}*/
const float F_DISCR_STEP = 0.25f;
static CVec3 GetNormalized( const CVec3 &_v )
{
	return CVec3( 
		Float2Int( _v.x / F_DISCR_STEP ) * F_DISCR_STEP,
		Float2Int( _v.y / F_DISCR_STEP ) * F_DISCR_STEP,
		Float2Int( _v.z / F_DISCR_STEP ) * F_DISCR_STEP );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMineTracker::AddMine( CMine *p, const CVec3 &_vPlace )
{
	CVec3 vPlace( GetNormalized( _vPlace ) );
	for ( int nZ = -1; nZ <= 1; ++nZ )
	{
		for ( int nY = -1; nY <= 1; ++nY )
		{
			for ( int nX = -1; nX <= 1; ++nX )
			{
				CVec3 vPos( vPlace );
				vPos.x += nX * F_DISCR_STEP;
				vPos.y += nY * F_DISCR_STEP;
				vPos.z += nZ * F_DISCR_STEP;
				CMineSet &ms = mines[ vPos ];
				if ( !IsInSet( ms, p ) )
					ms.push_back( p );
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMineTracker::RemoveMine( CMine *p )
{
	for ( CPlaceMinesHash::iterator i = mines.begin(); i != mines.end(); )
	{
		CMineSet &ms = i->second;
		CMineSet::iterator k = find( ms.begin(), ms.end(), p );
		if ( k != ms.end() )
			ms.erase( k );
		if ( ms.empty() )
			mines.erase( i++ );
		else
			++i;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CMineTracker::GetMines( const vector<CVec3> &places, vector<CPtr<CMine> > *pRes ) const
{
	pRes->resize(0);
	for ( int k = 0; k < places.size(); ++k )
	{
		CVec3 v( GetNormalized( places[k] ) );
		CPlaceMinesHash::const_iterator i = mines.find( v );
		if ( i != mines.end() )
		{
			const CMineSet &m = i->second;
			for ( int k = 0; k < m.size(); ++k )
			{
				CMine *p = m[k];
				if ( !IsInSet( *pRes, p ) )
					pRes->push_back( p );
			}
		}
	}
	return !pRes->empty();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NWorld;
REGISTER_SAVELOAD_CLASS( 0x019c2120, CMine )
REGISTER_SAVELOAD_CLASS( 0x019c2170, CMineTracker )