#include "StdAfx.h"
#include "BuildingClip.h"

namespace NBuilding
{
/*
////////////////////////////////////////////////////////////////////////////////////////////////////
// SThinClipInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
void SThinClipInfo::PackWallParts( int nSubPartID, const hash_map<int, bool> &partsHash )
{
	nPartID = nSubPartID;
	parts = 0;
	if ( partsHash.empty() )
	{
		parts = UNBROKEN_BLOCK32;
		return;
	}

	for ( hash_map<int, bool>::const_iterator it = partsHash.begin(); it != partsHash.end(); ++it )
	{
		if ( !it->second )
			continue;
		int x, y, z;
		GetPieceCoords( it->first, &x, &y, &z );
		ASSERT( 1 == y ); // должна быть тонкая стенка
		ASSERT( x < 24 );

		int ibit = x & 0x1 ? 5 * (x >> 1) + (z - 2)/2 : 5 * ((x >> 1) - 1) + 2 + (z - 1)/2;
		ASSERT( ibit < 32 );
		parts |= 1 << ibit;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UnpackWallParts( hash_map<int, bool> *pParts, DWORD parts, int nPartID )
{
	if ( UNBROKEN_BLOCK32 == parts )
	{
		(*pParts)[nPartID] = true;
		return;
	}
	int i, x;

	for ( x = 1, i = 0; x <= 3; ++x )
	{
		if ( parts & (1 << i) )
			(*pParts)[nPartID | GetPieceHashID( x, 1, 2 )] = true;
		++i;
		if ( parts & (1 << i) )
			(*pParts)[nPartID | GetPieceHashID( x, 1, 4 )] = true;
		++x;
		++i;
		if ( parts & (1 << i) )
			(*pParts)[nPartID | GetPieceHashID( x, 1, 1 )] = true;
		++i;
		if ( parts & (1 << i) )
			(*pParts)[nPartID | GetPieceHashID( x, 1, 3 )] = true;
		++i;
		if ( parts & (1 << i) )
			(*pParts)[nPartID | GetPieceHashID( x, 1, 5 )] = true;
		++i;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SThinClipInfo::PackFloorParts( int nSubPartID, const hash_map<int, bool> &partsHash )
{
	nPartID = nSubPartID;
	parts = 0;
	if ( partsHash.empty() )
	{
		parts = UNBROKEN_BLOCK32;
		return;
	}

	int nLength = nSrcWidthLen >> 8;
	int nOdd = nLength;	     // число узлов на нечетных линиях Y
	int nEven = nLength + 1; // число узлов на четных линиях Y
	
	for ( hash_map<int, bool>::const_iterator it = partsHash.begin(); it != partsHash.end(); ++it )
	{
		if ( !it->second )
			continue;
		int x, y, z;
		GetPieceCoords( it->first, &x, &y, &z );
		ASSERT( 1 == z ); // должна быть тонкая стенка

		int iBit = ((y - 1) / 2) * (nOdd + nEven);
		if ( y & 0x1 )
			iBit += (x - 2) / 2;
		else
			iBit += nOdd + (x - 1) / 2;
		ASSERT( iBit < 32 );
		parts |= 1 << iBit;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UnpackFloorParts( hash_map<int, bool> *pParts, DWORD parts, int nPartID, int nSrcWidthLen )
{
	if ( UNBROKEN_BLOCK32 == parts )
	{
		(*pParts)[nPartID] = true;
		return;
	}
	int nLength = nSrcWidthLen >> 8;
	int nOdd = nLength;
	int nEven = nLength + 1;

	const int maxi = 32 / (nOdd + nEven);
	int i, x, y, ibit = 0;
	
	for ( y = 1, i = 0; i < maxi; ++y, ++i )
	{
		for ( x = 2; x <= nLength * 2; x += 2 )
		{
			if ( parts & (1 << ibit) )
				(*pParts)[nPartID | GetPieceHashID( x, y, 1 )] = true;
			++ibit;
		}
		++y;
		for ( x = 1; x <= nLength * 2 + 1; x += 2 )
		{
			if ( parts & (1 << ibit) )
				(*pParts)[nPartID | GetPieceHashID( x, y, 1 )] = true;
			++ibit;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SSolidClipInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
// пакуется аналогично перекрытиям, только z определяет один из 5 возможных слоев
// так же учитывается разное расположение узлов при четных\нечетных z
void SSolidClipInfo::PackSolidParts( int nPart, hash_map<int, bool> &partsHash )
{
	nPartID = nPart;
	memset( &parts, 0, sizeof( parts ) );
	if ( partsHash.empty() )
	{
		parts = UNBROKEN_BLOCK32;
		return;
	}
	
	for ( int z = 1; z <= 5; ++z )
	{
		int offset = (z - 1) * 5;
		parts |= int( partsHash[GetPieceHashID( 2, 1, z )] ) << offset;
		parts |= int( partsHash[GetPieceHashID( 1, 2, z )] ) << (offset + 1);
		parts |= int( partsHash[GetPieceHashID( 3, 2, z )] ) << (offset + 2);
		parts |= int( partsHash[GetPieceHashID( 2, 3, z )] ) << (offset + 3);
		//
		offset += 5;
		if ( ++z > 5 )
			break;
		//
		parts |= int( partsHash[GetPieceHashID( 1, 1, z )] ) << offset;
		parts |= int( partsHash[GetPieceHashID( 3, 1, z )] ) << (offset + 1);
		parts |= int( partsHash[GetPieceHashID( 2, 2, z )] ) << (offset + 2);
		parts |= int( partsHash[GetPieceHashID( 1, 3, z )] ) << (offset + 3);
		parts |= int( partsHash[GetPieceHashID( 3, 3, z )] ) << (offset + 4);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UnpackSolidParts( hash_map<int, bool> *pParts, DWORD parts, int nPartID )
{
	if ( UNBROKEN_BLOCK32 == parts )
	{
		(*pParts)[nPartID] = true;
		return;
	}
	//
	for ( int z = 1; z <= 5; ++z )
	{
		int offset = (z - 1) * 5;
		(*pParts)[nPartID | GetPieceHashID( 2, 1, z )] = parts & (1 << offset);
		(*pParts)[nPartID | GetPieceHashID( 1, 2, z )] = parts & (1 << (offset + 1));
		(*pParts)[nPartID | GetPieceHashID( 3, 2, z )] = parts & (1 << (offset + 2));
		(*pParts)[nPartID | GetPieceHashID( 2, 3, z )] = parts & (1 << (offset + 3));
		//
		offset += 5;
		if ( ++z > 5 )
			break;
		//
		(*pParts)[nPartID | GetPieceHashID( 1, 1, z )] = parts & (1 << offset);
		(*pParts)[nPartID | GetPieceHashID( 3, 1, z )] = parts & (1 << (offset + 1));
		(*pParts)[nPartID | GetPieceHashID( 2, 2, z )] = parts & (1 << (offset + 2));
		(*pParts)[nPartID | GetPieceHashID( 1, 3, z )] = parts & (1 << (offset + 3));
		(*pParts)[nPartID | GetPieceHashID( 3, 3, z )] = parts & (1 << (offset + 4));
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// SClipInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
// z определяет один из 5 возможных слоев
// так же учитывается разное расположение узлов при четных\нечетных z
void PackParts( DWORD *pdwParts, hash_map<int, bool> &partsHash )
{
	DWORD &parts = *pdwParts;
	memset( &parts, 0, sizeof( parts ) );
	if ( partsHash.empty() )
	{
		parts = UNBROKEN_BLOCK32;
		return;
	}
	
	for ( int z = 1; z <= 5; ++z )
	{
		int offset = (z - 1) * 5;
		parts |= int( partsHash[GetPieceHashID( 2, 1, z )] ) << offset;
		parts |= int( partsHash[GetPieceHashID( 1, 2, z )] ) << (offset + 1);
		parts |= int( partsHash[GetPieceHashID( 3, 2, z )] ) << (offset + 2);
		parts |= int( partsHash[GetPieceHashID( 2, 3, z )] ) << (offset + 3);
		//
		offset += 5;
		if ( ++z > 5 )
			break;
		//
		parts |= int( partsHash[GetPieceHashID( 1, 1, z )] ) << offset;
		parts |= int( partsHash[GetPieceHashID( 3, 1, z )] ) << (offset + 1);
		parts |= int( partsHash[GetPieceHashID( 2, 2, z )] ) << (offset + 2);
		parts |= int( partsHash[GetPieceHashID( 1, 3, z )] ) << (offset + 3);
		parts |= int( partsHash[GetPieceHashID( 3, 3, z )] ) << (offset + 4);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UnpackParts( hash_map<int, bool> *pParts, DWORD parts, int nPartID )
{
	if ( UNBROKEN_BLOCK32 == parts )
	{
		(*pParts)[nPartID] = true;
		return;
	}
	//
	for ( int z = 1; z <= 5; ++z )
	{
		int offset = (z - 1) * 5;
		(*pParts)[nPartID | GetPieceHashID( 2, 1, z )] = parts & (1 << offset);
		(*pParts)[nPartID | GetPieceHashID( 1, 2, z )] = parts & (1 << (offset + 1));
		(*pParts)[nPartID | GetPieceHashID( 3, 2, z )] = parts & (1 << (offset + 2));
		(*pParts)[nPartID | GetPieceHashID( 2, 3, z )] = parts & (1 << (offset + 3));
		//
		offset += 5;
		if ( ++z > 5 )
			break;
		//
		(*pParts)[nPartID | GetPieceHashID( 1, 1, z )] = parts & (1 << offset);
		(*pParts)[nPartID | GetPieceHashID( 3, 1, z )] = parts & (1 << (offset + 1));
		(*pParts)[nPartID | GetPieceHashID( 2, 2, z )] = parts & (1 << (offset + 2));
		(*pParts)[nPartID | GetPieceHashID( 1, 3, z )] = parts & (1 << (offset + 3));
		(*pParts)[nPartID | GetPieceHashID( 3, 3, z )] = parts & (1 << (offset + 4));
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SplitOptimized( hash_map<int, bool> *pPartsHash )
{
	if ( pPartsHash->empty() )
		return;
	int x, y, z;
	GetPartCoords( pPartsHash->begin()->first, &x, &y, &z );
	const nPart = GetPartHashID( x, y, z );
	hash_map<int, bool>::iterator it = pPartsHash->find( nPart );
	if ( it == pPartsHash->end() )
		return;
	pPartsHash->erase( it );
	for ( int z = 1; z < 6; ++z )
	{
		(*pPartsHash)[nPart | GetPieceHashID( 2, 1, z )] = true;
		(*pPartsHash)[nPart | GetPieceHashID( 1, 2, z )] = true;
		(*pPartsHash)[nPart | GetPieceHashID( 3, 2, z )] = true;
		(*pPartsHash)[nPart | GetPieceHashID( 2, 3, z )] = true;
		if ( ++z > 5 )
			break;
		(*pPartsHash)[nPart | GetPieceHashID( 1, 1, z )] = true;
		(*pPartsHash)[nPart | GetPieceHashID( 3, 1, z )] = true;
		(*pPartsHash)[nPart | GetPieceHashID( 2, 2, z )] = true;
		(*pPartsHash)[nPart | GetPieceHashID( 1, 3, z )] = true;
		(*pPartsHash)[nPart | GetPieceHashID( 3, 3, z )] = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
