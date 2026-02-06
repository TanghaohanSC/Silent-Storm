#include "StdAfx.h"
#include "WysiwygClipboard.h"
#include "BuildingInfo.h"
#include "..\Misc\BasicShare.h"
#include "..\MapEdit\FinDBCmd.h"
#include "..\MapEdit\RectsDBCmd.h"
#include "..\MapEdit\UnitDB.h"
#include "..\DBFormat\DataMap.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataObject.h"
#include "..\DBFormat\DataRPG.h"
#include "WysiwygUndo.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
	extern CBasicShare<int, NBuilding::CBuildInfoLoader> shareBuildings;
}
namespace NWysiwyg
{
	extern int AddTerrSpotDB( int nVarID, const NBuilding::SProjectedSpot &spot );
}
namespace NGfx
{
	HWND GetHWND();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static const char szCF_NAME[] = "A5_WYSIWYG_DATA";
static int CF_WYSIWYGDATA = 0;
const int FRAGMENT_SIZE = sizeof( NBuilding::SBuildFragment );
////////////////////////////////////////////////////////////////////////////////////////////////////
static struct SClpBuffer
{
	ZDATA
	vector<NBuilding::SBuildFragment> frags;
	vector<int> objects; // nFinalElementID
	vector<NBuilding::SProjectedSpot> terrSpots;
	vector<NBuilding::SProjectedSpot> wallSpots;
	vector<NBuilding::SLadder> ladders;
	vector<int> units; // nDBUnitID
	vector<int> subtemplates; // nDBRectID
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&frags); f.Add(3,&objects); f.Add(4,&terrSpots); f.Add(5,&wallSpots); f.Add(6,&ladders); f.Add(7,&units); f.Add(8,&subtemplates); return 0; }

	void Clear()
	{
		frags.clear();
		objects.clear();
		terrSpots.clear();
		wallSpots.clear();
		ladders.clear();
		units.clear();
		subtemplates.clear();
	}
} clpBuffer;
////////////////////////////////////////////////////////////////////////////////////////////////////
void SetClipboardData()
{
	if ( !::OpenClipboard( NGfx::GetHWND() ) )
    return;
  // Remove the current Clipboard contents
	if( !::EmptyClipboard() )
	{
		DWORD dwError = GetLastError();
		::CloseClipboard();
    return;
	}
	if ( 0 == CF_WYSIWYGDATA )	
	{
		CF_WYSIWYGDATA = ::RegisterClipboardFormat( szCF_NAME );
		if ( 0 == CF_WYSIWYGDATA )
		{
			::CloseClipboard();
			return;
		}
	}
	//
	CMemoryStream memstream;
	{
		CStructureSaver saver( memstream, CStructureSaver::WRITE );
		saver.Add( 1, &clpBuffer );
	}
	//
	HGLOBAL hGlobal = ::GlobalAlloc( GMEM_SHARE|GMEM_FIXED, memstream.GetSize() );
	void *pClp = (void*)::GlobalLock( hGlobal );
	if ( !pClp )
	{
		::CloseClipboard();
		return;
	}
	//
	memcpy( pClp, memstream.GetBuffer(), memstream.GetSize() );
	//Put reference to global memory into clipboard
	::GlobalUnlock( hGlobal );

  // For the appropriate data formats...
	if ( ::SetClipboardData( CF_WYSIWYGDATA, hGlobal ) == NULL )
  {
		::CloseClipboard();
    return;
  }
	clpBuffer.Clear();
  //
	::CloseClipboard();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
extern void FixRoomMapSize( NBuilding::CBuildInfo *pInfo );
extern void FixCellarSize( NBuilding::CBuildInfo *pInfo, int nX, int nY );
////////////////////////////////////////////////////////////////////////////////////////////////////
static void InsertFragments( SForceSelection *pSel, NBuilding::CBuildInfo *pInfo, const vector<NBuilding::SBuildFragment> &frags, int nWorldID )
{
	ASSERT( pInfo && pSel );
	NDb::CTemplVariant *pVar = NDb::GetTemplVariant( nWorldID );
	if ( !pVar || !IsValid( pVar->pTemplate ) )
		return;
	if ( !frags.empty() )
	{
		pInfo->nMaxX = pVar->pTemplate->nWidth;
		pInfo->nMaxY = pVar->pTemplate->nHeight;
	}
	FixRoomMapSize( pInfo );
	FixCellarSize( pInfo, pVar->pTemplate->nWidth, pVar->pTemplate->nHeight );
	//
	SRand rnd;
	for ( int i = 0; i < frags.size(); ++i )
	{
		NBuilding::SBuildFragment fr = frags[i];
		NDb::CTConstructionPart *pTCP = NDb::GetTConstructionPart( fr.nConstructionPartID );
		if ( !pTCP )
			continue;
		CPtr<NDb::CConstructionPart> pCP = pTCP->CreateConstructionPart( &rnd );
		if ( !IsValid( pCP ) )
			continue;
		fr.nID = pInfo->CreateNextFragmentID();
		if ( pCP->nSizeY == 0 )
		{
			pSel->fragsUserIDs.push_back( pInfo->wallFragments.size() | 0x40000000 );
			pInfo->wallFragments.push_back( fr );
		}
		else
		{
			pSel->fragsUserIDs.push_back( pInfo->solidFragments.size() );
			pInfo->solidFragments.push_back( fr );
		}
		NMapEditor::PushUndoCmd( CreateFragmentSelUndo( CWysiwygUndo::UA_INSERT, nWorldID, &fr, 0 ) );
		int nf = fr.ptPos.z + 0.5f;
		pInfo->nMaxFloor = Max( pInfo->nMaxFloor, nf );
		pInfo->nMinFloor = Min( pInfo->nMinFloor, nf );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void InsertObjects( SForceSelection *pSel, const vector<int> &objects, int nWorldID )
{
	ASSERT( pSel );
	static CFinPosDB db;
	NDb::CTemplVariant *pVar = NDb::GetTemplVariant( nWorldID );
	if ( !pVar || !IsValid( pVar->pTemplate ) )
		return;
	float nWidth = pVar->pTemplate->nWidth;
	float nHeight = pVar->pTemplate->nHeight;
	//
	for ( int i = 0; i < objects.size(); ++i )
	{
		NDb::CFinalElement *pF = NDb::GetFinalElement( objects[i] );
		if ( !pF )
			continue;
		int nID = db.Insert( nWorldID, pF->pObject->GetRecordID() );
		if ( nID > 0 )
		{
//			pF->ptPos.x = Clamp( pF->ptPos.x, 0.0f, nWidth ); // выравнивается вместе со всем селекшеном 
//			pF->ptPos.y = Clamp( pF->ptPos.y, 0.0f, nHeight );
			db.SetPos( nID, pF->ptPos, pF->fDZ, pF->nFloor, pF->fRotation );
			db.SetScale( nID, pF->ptScale.x, pF->ptScale.y, pF->ptScale.z );
			db.SetOpen( nID, pF->bOpen );
			db.SetLightmap( nID, pF->bLightmap );
			pSel->objectIDs.push_back( nID );
			NMapEditor::PushUndoCmd( CreateObjSelectionUndo( CWysiwygUndo::UA_INSERT, pF, 0, nID ) );
		}
	}
	if ( !objects.empty() )
	{
		Sleep( 10 );
		NDatabase::Refresh<NDb::CFinalElement>();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void InsertTerrSpots( SForceSelection *pSel, const vector<NBuilding::SProjectedSpot> &spots, int nWorldID )
{
	for ( int i = 0; i < spots.size(); ++i )
	{
		int nID = NWysiwyg::AddTerrSpotDB( nWorldID, spots[i] );
		if ( nID > 0 )
			pSel->terrSpotsIDs.push_back( nID );
	}
	if ( !spots.empty() )
	{
		Sleep( 30 );
		NDatabase::Refresh<NDb::CRndTerrainSpot>();
		for ( int i = 0; i < pSel->terrSpotsIDs.size(); ++i )
			NMapEditor::PushUndoCmd( CreateTerrSpotUndo( CWysiwygUndo::UA_INSERT, NDb::GetRndTerrainSpot( pSel->terrSpotsIDs[i] ), 0 ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void InsertWallSpots( SForceSelection *pSel, const vector<NBuilding::SProjectedSpot> &spots, int nWorldID )
{
	for ( int i = 0; i < spots.size(); ++i )
	{
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void InsertSubTemplates( SForceSelection *pSel, const vector<int> &subtemplates, int nWorldID )
{
	static CRectPosDB db;
	for ( int i = 0; i < subtemplates.size(); ++i )
	{
		NDb::CRectangle *p = NDb::GetRectangle( subtemplates[i] );
		if ( !IsValid( p ) || !IsValid( p->pTemplate ) )
			continue;
		int nID = db.Insert( nWorldID, p->pTemplate->GetRecordID() );
		if ( nID > 0 )
		{
			pSel->subtemplateIDs.push_back( nID );
			db.SetPos( nID, p->ptCenter, p->fDZ, p->nFloor, p->fRotation );
			NMapEditor::PushUndoCmd( CreateSubTemplateUndo( CWysiwygUndo::UA_INSERT, p, 0, nID ) );
		}
	}
	if ( !subtemplates.empty() )
	{
		Sleep( 50 );
		NDatabase::Refresh<NDb::CRectangle>();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void InsertUnits( SForceSelection *pSel, const vector<int> &units, int nWorldID )
{
	static CUnitPosDB db;
	for ( int i = 0; i < units.size(); ++i )
	{
		NDb::CUnit *p = NDb::GetUnit( units[i] );
		if ( !IsValid( p ) || !IsValid( p->pMonster ) )
			continue;
		int nID = db.Insert( nWorldID, p->pMonster->GetRecordID() );
		if ( nID > 0 )
		{
			pSel->unitIDs.push_back( nID );
			db.SetPos( nID, p->ptPos.x, p->ptPos.y, p->nFloor, p->fRotation );
			NMapEditor::PushUndoCmd( CreateUnitUndo( CWysiwygUndo::UA_INSERT, p, 0, nID ) );
		}
	}
	if ( !units.empty() )
	{
		Sleep( 10 );
		NDatabase::Refresh<NDb::CUnit>();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PasteClipboard( SForceSelection *pSelectionBuf, int nWorldID )
{
	CDGPtr< CPtrFuncBase<NBuilding::CBuildInfo> > pLoader = NGScene::shareBuildings.Get( nWorldID );
	pLoader.Refresh();
	NBuilding::CBuildInfo *pInfo = pLoader->GetValue();
	if ( !pInfo )
		return;
	//
	if ( !::IsClipboardFormatAvailable(CF_WYSIWYGDATA) )
      return; 
	if ( !::OpenClipboard( NGfx::GetHWND() ) )
      return; 

	HANDLE hglb = ::GetClipboardData(CF_WYSIWYGDATA);
  if (hglb != NULL) 
  { 
		int nSize = ::GlobalSize( hglb );
		void *pClp = (void*)::GlobalLock( hglb );

		CFixedMemStream memstream( pClp, nSize );
		CStructureSaver loader( memstream, CStructureSaver::READ );
		SClpBuffer buf;

		loader.Add( 1, &buf );

		NMapEditor::BeginUndoList();
			InsertFragments( pSelectionBuf, pInfo, buf.frags, nWorldID );
			InsertObjects( pSelectionBuf, buf.objects, nWorldID );
			InsertTerrSpots( pSelectionBuf, buf.terrSpots, nWorldID );
			InsertWallSpots( pSelectionBuf, buf.wallSpots, nWorldID );
			InsertSubTemplates( pSelectionBuf, buf.subtemplates, nWorldID );
			InsertUnits( pSelectionBuf, buf.units, nWorldID );
		NMapEditor::EndUndoList();

		pSelectionBuf->nWorldID = nWorldID;
		::GlobalUnlock( hglb );
	}
	::CloseClipboard();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddFragmentToClipboardBuffer( const NBuilding::SBuildFragment &fr )
{
	clpBuffer.frags.push_back( fr );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddObjectToClipboardBuffer( int nObjectID )
{
	clpBuffer.objects.push_back( nObjectID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddTerrSpotToClipboardBuffer( const NBuilding::SProjectedSpot &spot )
{
	clpBuffer.terrSpots.push_back( spot );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddLadderToClipboardBuffer( const NBuilding::SLadder &ladder )
{
	clpBuffer.ladders.push_back( ladder );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddUnitToClipboardBuffer( int nDBUnitID )
{
	clpBuffer.units.push_back( nDBUnitID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AddSubTemplateToClipboardBuffer( int nDBRectID )
{
	clpBuffer.subtemplates.push_back( nDBRectID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////

