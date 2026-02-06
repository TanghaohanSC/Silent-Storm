#include "StdAfx.h"
#include "wMain.h"
#include "Transform.h"
#include "RPGMission.h"
/////////////////////////////////////////////////////////////////////////////////////
// class for convinient handling of framework`s contexts
class CFWContext
{
	void **pFrameworkPtr;
public:
	template<class T>
		CFWContext( T **pF, T *pData ): pFrameworkPtr((void**)pF) { *pF = pData; }
	~CFWContext() { *pFrameworkPtr = 0; }
};
/////////////////////////////////////////////////////////////////////////////////////
namespace NWorld
{
	using namespace NWorld;
/////////////////////////////////////////////////////////////////////////////////////
static CWorld *pCurrentWorld = 0;
/////////////////////////////////////////////////////////////////////////////////////
// CUnitPath
/////////////////////////////////////////////////////////////////////////////////////
void CUnitPath::Update()
{
	if ( !pTime.Refresh() )
		return;
	switch ( mode )
	{
		case E_MOVE:
		{
			float fDist = fabs( ptDest - ptPos );
			float fTime = ( pTime->GetValue() - tMoveStart ) * (1/1000.0f);
			if ( fTime > fDist )
			{
				ptPos = ptDest;
				MakeMatrix( &value, 0, fpAngle, ptPos );
				mode = E_STAND;
			}
			else
			{
				float fAlpha = fTime / fDist;
				MakeMatrix( &value, 0, fpAngle, ptDest * fAlpha + ptPos * ( 1 - fAlpha ) );
			}
		}
		break;
	}
	Updated();
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitPath::Init( const CVec3 &pos )
{
	mode = E_STAND;
	ptPos = pos;
	fpAngle = 0;
	MakeMatrix( &value, 0, fpAngle, ptPos );
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitPath::Move( const CVec3 &_ptDest )
{
	mode = E_MOVE;
	tMoveStart = pTime->GetValue();
	ptDest = _ptDest;
	fpAngle = atan2( ptDest.y - ptPos.y, ptDest.x - ptPos.x );
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitPath::Serialize( CStructureSaver *pFile )
{
	pFile->AddData( 1, &tMoveStart );
	pFile->AddData( 2, &mode );
	pFile->AddData( 3, &ptPos );
	pFile->AddData( 4, &ptDest );
	pFile->AddData( 5, &fpAngle );
	pFile->AddObject( 6, &pTime );
	//
	if ( pFile->IsReading() && mode == E_STAND )
		Init( ptPos );
}
/////////////////////////////////////////////////////////////////////////////////////
// CUnitServer
/////////////////////////////////////////////////////////////////////////////////////
CUnitServer::CUnitServer()
{
	bIsDynamic = false;
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::Init( CWorld *pWorld, NRPG::CUnitMissionBase *_pRPG )
{
	pRPG = _pRPG;
	pPath = new CUnitPath;
	pUnit = new CUnit;
	pUnit->pPosition = pPath;
	pUnit->nModelID = 1;
	pUnit->pRPG = pRPG;
	pPath->pTime = pWorld->GetTimeNode();
	bIsDynamic = false;
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::Move( const CVec3 &ptDest )
{
	if ( !bIsDynamic )
		pCurrentWorld->StartDynamic();
	pPath->Move( ptDest );
	bIsDynamic = true;
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::Stand( const CVec3 &ptDest )
{
	pPath->Init( ptDest );
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::Segment()
{
	// CRAP - adjust time for UnitPath
	if ( bIsDynamic && pPath->mode == CUnitPath::E_STAND )
	{
		pCurrentWorld->DoneDynamic();
		bIsDynamic = false;
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::ShowUnit( CWorld *pWorld )
{
	pWorld->ShowUnit( pUnit );
}
/////////////////////////////////////////////////////////////////////////////////////
void CUnitServer::Serialize( CStructureSaver *pFile )
{
	pFile->AddObject( 1, &pUnit );
	pFile->AddObject( 2, &pPath );
	pFile->AddData( 3, &bIsDynamic );
}
/////////////////////////////////////////////////////////////////////////////////////
// CWorld
/////////////////////////////////////////////////////////////////////////////////////
static NDG::CFuncBase<SHMatrix>* MakePlacement( const CVec3 &size, const CVec3 &move )
{
	SHMatrix s;
	CMatrixStack43<4> m;
	Identity( &s );
	m.Init( s );
	m.Push( move );
	m.PushScale( size.x, size.y, size.z );
	NGScene::CCMSR *pPlace = new NGScene::CCMSR;
	pPlace->Set( m.Get() );
	return pPlace;
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::AddObject( const CVec3 &pos, const CVec3 &size, int nModelID )
{
	showObjects.push_back( new CObject );
	CObject &o = *showObjects.back();
	o.nModelID = nModelID;
	o.pPosition = MakePlacement( size, pos );
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::AddUnit( const CVec3 &pos, NRPG::CUnitMissionBase *_pRPG, int nModelID )
{
	CUnitServer *pUS = new CUnitServer;
	pUS->Init( this, _pRPG );
	units.push_back( pUS );
	pUS->Stand( pos );
	pUS->ShowUnit( this );
}
/////////////////////////////////////////////////////////////////////////////////////
CUnitServer* CWorld::GetUnitServer( CUnit *pUnit )
{
	for ( list< CObj<CUnitServer> >::const_iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( (*i)->HasUnit( pUnit ) )
			return *i;
	}
	ASSERT(0);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::CreateRandom( vector<CPtr<NRPG::CUnitMissionBase> > &mercs )
{
	CFWContext world( &pCurrentWorld, this );
	//
	AddObject( CVec3(0,0,0), CVec3( 10, 10, 1 ), 1 );
	AddObject( CVec3(4,2,3), CVec3( 2, 2, 2 ), 1 );
	AddObject( CVec3(5,1,3), CVec3( 1, 1, 1 ), 1 );
	AddObject( CVec3(5,5,5), CVec3( 1, 1, 1 ), 2 );
	AddObject( CVec3(5,5,5), CVec3( 1, 1, 1 ), 3 );
	// deploy units
	for ( int i = 0; i < mercs.size(); i++ )
		AddUnit( CVec3(1,1 + i * 2,1), mercs[i], 1 );
	//
	showObjects.sort();
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::ExecMove( CUnit *pUnit, const CVec3 &ptDest )
{
	CFWContext world( &pCurrentWorld, this );
	ASSERT( pUnit->IsValid() );
	if ( !pUnit->IsValid() )
		return;
	ASSERT( find_if( showUnits.begin(), showUnits.end(), SPtrTest( pUnit ) ) != showUnits.end() );
	CUnitServer *pUS = GetUnitServer( pUnit );
	if ( pUS->IsValid() )
		pUS->Move( ptDest );
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::StepWorld()
{
	CFWContext world( &pCurrentWorld, this );
	ASSERT( pTime->IsValid() );
	for ( list< CObj<CUnitServer> >::iterator i = units.begin(); i != units.end(); ++i )
	{
		(*i)->Segment();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CWorld::Serialize( CStructureSaver *pFile )
{
	pFile->AddContainer( 1, &showObjects );
	pFile->AddContainer( 2, &showUnits );
	pFile->AddData( 3, &nExecCounter );
	pFile->AddObject( 4, &pTime );
	pFile->AddContainer( 5, &units );
	//
	showObjects.sort();
	showUnits.sort();
}
/////////////////////////////////////////////////////////////////////////////////////
void RegisterWorldMainClasses( int nBase )
{
	REGISTER_SAVELOAD_CLASS( nBase + 0, CWorld );
	REGISTER_SAVELOAD_CLASS( nBase + 1, CUnitServer );
	REGISTER_SAVELOAD_CLASS( nBase + 2, CUnitPath );
}
/////////////////////////////////////////////////////////////////////////////////////
} // namespace NWorld
/////////////////////////////////////////////////////////////////////////////////////
