#ifndef __MAPBUILD_H__
#define __MAPBUILD_H__

#include "PolyUtils.h"
#include "aiPosition.h"
#include "aiWaypoint.h"
#include "TerrainInfo.h"
#include "MapBuildingInfo.h"
#include "..\Misc\RandomGen.h"

struct SRandomSeed;
namespace NBuilding
{
	class CBuildingGrid;
}
namespace NDb
{
	class CObject;
	class CTemplVariant;
	class CRPGPers;
	class CAmbientLight;
	class CWaypointName;
	class CRPGItem;
	class CScript;
	enum EScenarioClueType;
}
namespace NAI
{
	class IPathNetwork;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMapElement
{
	CPtr<NDb::CObject> pObject;
	SMapPosition pos;
	int nRelFloor;
	bool bLightmap;								// тип источника света в контейнере
	bool bTerrAlignment;
	bool bOpen;
	int nPassageZoneID;
	int nPassageObjectID;
	int nAPRadius;
	SMapElement(): bTerrAlignment(true), bOpen(false) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMapRPGElement
{
	CPtr<NDb::CRPGItem> pItem;
	SMapPosition pos;
	int nRelFloor;
	bool bTerrAlignment;
	bool bOpen;
	SMapRPGElement(): bTerrAlignment(true), bOpen(false) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMapWaypoint: public CObjectBase
{
	OBJECT_BASIC_METHODS(CMapWaypoint);
public:
	CDBPtr<NDb::CWaypointName> pName;
	vector<NAI::SCommand> commands;
	bool bTerrAlign;
	SMapPosition pos;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMapUnit
{
	int nUnitID; // for route generation;
	CPtr<NDb::CRPGPers> pPers;
	SMapPosition pos;
	vector<CPtr<CMapWaypoint> > route;
	bool bSlot;
	int nDiplomacy;
	int nScenarioPlayer;
	SMapUnit() : bSlot(false), nDiplomacy(0), nScenarioPlayer(0) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMapWall
{
	ZDATA
	int nHeightMin;
	int nHeightMax;
	CVec2 vBeg;
	CVec2 vEnd;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nHeightMin); f.Add(3,&nHeightMax); f.Add(4,&vBeg); f.Add(5,&vEnd); return 0; }
};
struct SMapHole
{
	ZDATA
	int nFloor;
	int nHeight;
	bool bVisible;
	TPolygonsList polygonsList;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nFloor); f.Add(3,&nHeight); f.Add(4,&bVisible); f.Add(5,&polygonsList); return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SDeploySpot
{
	SMapPosition pos;
	//CVec3 ptPos;
	//float fAngle;
	bool bTerrAlignment;

	SDeploySpot(): bTerrAlignment(true) {}
	SDeploySpot( const SMapPosition &_pos ): pos(_pos), bTerrAlignment(true) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SClueSlot
{
	SMapPosition pos;
	bool bTerrAlignment;
	//
	int nUnitID;
	CPtr<NDb::CRPGPers> pPers;
	bool bPersSlot;
	bool bInventorySlot;
	SClueSlot(): bTerrAlignment(true), bInventorySlot(false), bPersSlot(false) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMapInfo
{
	int nBaseTerrainFloor;
	list<SMapWall> wallsList;
	list<SMapHole> holesList;
	list<SMapElement> items;
	list<SMapRPGElement> rpgitems;
	list<SMapUnit> units;
	list< CObj<CMapWaypoint> > waypoints;
	list< CDBPtr<NDb::CScript> > scripts;
	vector<SMapBuilding> buildings;
	STerrainInfo terrain;
	CTRect<float> sMapSafeZone;
	CDBPtr<NDb::CAmbientLight> pDefaultLight;
	vector<SDeploySpot> deploySpots;
	vector<SClueSlot> slots;

	SMapInfo() {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
bool BuildMap( int nPlacementID, const vector<string> &strParams, 
	NAI::IPathNetwork *pNet, SMapInfo *pInfo, int nDepth = -1, SRandomSeed sSeed = SRandomSeed() );
////////////////////////////////////////////////////////////////////////////////////////////////////
bool BuildTerrain( int nMapID, SMapInfo *pInfo, int nDepth ); // MapEdit
bool BuildMapEditMap( int nMapID, NAI::IPathNetwork *pNet, SMapInfo *pInfo, int nDepth, const SMapPosition &pos, const CVec3 &ptBuilding, bool bTerrAlign );
////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __MAPBUILD_H__