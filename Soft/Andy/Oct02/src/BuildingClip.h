#ifndef __BuildingClip_H_
#define __BuildingClip_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\DBFormat\DataConst.h"
#include "DiscretePos.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
	class CModel;
	class CGeometry;
	class CAIGeometry;
}
namespace NBuilding
{
	class CMixedMaterial;
////////////////////////////////////////////////////////////////////////////////////////////////////
const int UNBROKEN_BLOCK32 = 0xffffffff;
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ESide
{
	FRONT = 0,
	BACK,
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
	INTERNAL,
	ESIZE,
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// клипинфо для стен и перекрытий; ObjectInfo шарится по этой инфе
// содержится информация о видимых кусочках и какими плоскостями клипать края
enum EClipType
{
	CLIP_WALL,
	CLIP_FLOOR,	
};
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
struct SThinClipInfo
{
	EClipType type;
	CPtr<NDb::CModel> pModel;
	DWORD parts;
	int nPartID; // идекс сабкуска
	short nClip;
	short nSrcWidthLen;
	short nRoom[2];
	
	void PackWallParts( int nSubPartID, const hash_map<int, bool> &parts );
	void PackFloorParts( int nSubPartID, const hash_map<int, bool> &parts );
	SThinClipInfo() {	memset( this, 0, sizeof( SThinClipInfo ) );	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// клипинфо для сплошных объектов, содержится инфрмация о видимых кусках
struct SSolidClipInfo
{
	CPtr<NDb::CModel> pModel;
	DWORD parts; // битовая маска видимых кусочков
	int nPartID; // идекс сабкуска
	short nRoom;
	
	void PackSolidParts( int nPartID, hash_map<int, bool> &parts );
	
	SSolidClipInfo() { memset( this, 0, sizeof( SSolidClipInfo ) );	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void UnpackWallParts( hash_map<int, bool> *pParts, DWORD parts, int nPartID );
void UnpackFloorParts( hash_map<int, bool> *pParts, DWORD parts, int nPartID, int nSrcWidthLen );
void UnpackSolidParts( hash_map<int, bool> *pParts, DWORD parts, int nPartID );
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
inline int GetPieceHashID( int x, int y, int z )
{
	return z << 4 | y << 2 | x;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void GetPieceCoords( int nHashID, int *px, int *py, int *pz )
{
	*px = nHashID & 0x3;
	*py = (nHashID >> 2) & 0x3;
	*pz = (nHashID >> 4) & 0x7;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline int GetPartHashID( int partX, int partY, int partZ )
{
	return partZ << 13 | partY << 10 | partX << 7;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void GetPartCoords( int nHashID, int *pPartX, int *pPartY, int *pPartZ )
{
	*pPartX = (nHashID >> 7) & 0x7;
	*pPartY = (nHashID >> 10) & 0x7;
	*pPartZ = (nHashID >> 13) & 0x7;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsUnbrokenPart( int nHashID )
{
	return (nHashID & 0x7f) == 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SClipInfo
{
	CPtr<NDb::CGeometry> pGeometry;
	int nSubBlockID;
	CVec3 ptPos;
	int   nRotationID;
	CPtr<CMixedMaterial> pMaterials[NDb::N_MODEL_MATERIALS];
	short nRooms[2];
	union
	{
		int nClip;				// для стены: старшие 2 байта - длина, толщина; младшие - клипающая плоскость 
		int nNeighbors;   // для сплошного объекта (перекрытий, окон и т.д.)
	};
	DWORD dwParts;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void PackParts( DWORD *pdwParts, hash_map<int, bool> &partsHash );
void UnpackParts( hash_map<int, bool> *pPartsHash, DWORD dwParts, int nSubPartID );
void SplitOptimized( hash_map<int, bool> *pPartsHash );
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAIClipInfo
{
	CPtr<NDb::CAIGeometry> pAIGeometry;
	int nSubBlock;
	CVec3 ptPos;
	int   nRotation;
	DWORD dwParts;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
