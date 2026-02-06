#include "StdAfx.h"
#include "aiPosition.h"
#include "aiGrid.h"
#include "Grid.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// SPosition
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SPosition::IsValid() const
{
	return p.GetLayer() < pNet->GetNumLayers();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec2 SPosition::GetCPNoHeight() const
{
	return pNet->GetCPNoHeight( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 SPosition::GetCP() const
{
	return pNet->GetCP( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float SPosition::GetDirection() const
{
	return pNet->GetDirection( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int SPosition::GetFloor() const
{
	return pNet->GetFloor( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SPosition::IsLocked() const
{
	return pNet->IsLocked( p );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SPosition::SetNetwork( IPathNetwork *_pNet )
{
	pNet = dynamic_cast<CPathNetwork*>( _pNet );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int SPosition::operator&( CStructureSaver &f )
{
	f.Add( 1, &p );
	f.Add( 2, &pNet );
	return 0;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SObjectPosition
////////////////////////////////////////////////////////////////////////////////////////////////////
int SObjectPosition::operator&( CStructureSaver &f )
{
	f.Add( 1, &pos );
	f.Add( 2, &nFloor );
	return 0;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SUnitPosition
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SUnitPosition::IsValid() const
{
	return pos.IsValid();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float SUnitPosition::GetHeight() const
{
	switch( GetPose() )
	{
		case CRAWL:
			return 0.4f;
		case CROUCH:
			return 1.0f;
		case WALK:
		case RUN:
			return 1.9f;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float SUnitPosition::GetHLHeight( EHitLocation eHL ) const
{
	switch( GetPose() )
	{
		case CRAWL:
			return 0.2f;
		case CROUCH:
			if ( eHL == HL_HEAD )
				return 1.0f;
			else
				return 0.4f;
		case WALK:
		case RUN:
			switch ( eHL )
			{
				case HL_HEAD:
					return 1.6f;
				case HL_BODY:
				case HL_LHAND:
				case HL_RHAND:
					return 1.0f;
				default:
					return 0.4f;
			}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 SUnitPosition::GetCenter() const
{
	CVec3 vAdd = VNULL3;
	switch( GetPose() )
	{
		case CRAWL:
			vAdd.z = 0.25f;
			break;
		case CROUCH:
			vAdd.z = 0.75f;
			break;
		case WALK:
		case RUN:
			vAdd.z = 1.3f;
			break;
	}
	return GetCP() + vAdd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 SUnitPosition::GetEyePosition() const
{
	CVec3 vAdd = VNULL3;
	switch( GetPose() )
	{
		case CRAWL:
			vAdd.z = 0.3f;
			break;
		case CROUCH:
			vAdd.z = 1.1f;
			break;
		case WALK:
		case RUN:
			vAdd.z = 1.56f;
			break;
	}
	return GetCP() + vAdd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SUnitPosition::SetPose( EPose pose ) 
{
	switch( pose )
	{
		case CRAWL:
			pos.p.SetPose( CM_LAY );
			bRun = false;
			break;
		case CROUCH:
			pos.p.SetPose( CM_CROUCH );
			bRun = false;
			break;
		case WALK:
			pos.p.SetPose( CM_STAND );
			bRun = false;
			break;
		case RUN:
			pos.p.SetPose( CM_STAND );
			bRun = true;
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
EPose SUnitPosition::GetPose() const
{
	switch ( pos.p.GetPose() )
	{
		case CM_LAY:
			return CRAWL;
		case CM_CROUCH:
		case CM_INACTIVE:
			return CROUCH;
		default:
			if ( bRun )
				return RUN;
			else
				return WALK;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int SUnitPosition::operator&( CStructureSaver &f )
{
	f.Add( 2, &pos );
	//f.Add( 3, &dir );
	f.Add( 4, &bRun );
	return 0;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const float F_CLOSE_HEIGHT_1 = 0.65f;
const float F_CLOSE_HEIGHT_2 = 1.5f;
EBlowHeight GetBlowHeight( const SUnitPosition &attackerPos, const CVec3 &ptTarget )
{
	float fHeightDiff = ptTarget.z - attackerPos.GetCP().z;
	if ( fHeightDiff < F_CLOSE_HEIGHT_1 )
		return BH_BOTTOM;
	if ( fHeightDiff > F_CLOSE_HEIGHT_2 )
		return attackerPos.GetPose() != NAI::CROUCH ? BH_TOP : BH_MIDDLE;
	return BH_MIDDLE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NAI::SPosition GetNearestPosition( CVec3 ptPos, IPathNetwork *pPathNetwork )
{
	float fMinDistance = 0xFFFF;

	SPosition sNearestPlace;
	bool bFound = false;
	float fRadius = 1.f;
	while ( !bFound )
	{
		SSphere sSphere( ptPos, fRadius );
		vector<SPathPlace> vNearestPlaces;
		pPathNetwork->GetNearPlaces( sSphere, &vNearestPlaces );
		for ( vector<SPathPlace>::iterator p = vNearestPlaces.begin(); p != vNearestPlaces.end(); ++p )
		{
			if ( pPathNetwork->IsPassable( *p ) )
			{
				bFound = true;
				SPosition sPlace;
				sPlace.p = *p;
				sPlace.SetNetwork( pPathNetwork );
				CVec3 ptTmpPosition = sPlace.GetCP();
				if ( fabs( ptTmpPosition - ptPos ) < fMinDistance )
				{
					fMinDistance = fabs( ptTmpPosition - ptPos );
					sNearestPlace = sPlace;
					if ( fMinDistance == 0 )
						break;
				}
			}
		}
		fRadius += 2;
		if ( fRadius > 15 )
		{
			ASSERT(0); // No position found near search place
			SPosition sPlace;
			sPlace.p = vNearestPlaces[0];
			sPlace.SetNetwork( pPathNetwork );
			return sPlace;
		}
	}
	return sNearestPlace;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IPathNetwork* CreateNodesNetwork( IAIMap *pMap )
{
	return new CPathNetwork( pMap );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////





















