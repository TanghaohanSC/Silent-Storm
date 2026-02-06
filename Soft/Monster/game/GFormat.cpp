#include "StdAfx.h"
#include "GFormat.h"
#include "Gfx.h"
#include <strstream>
/////////////////////////////////////////////////////////////////////////////////////
// CFileGeometry
/////////////////////////////////////////////////////////////////////////////////////
void CFileGeometry::Update()
{
	typedef NGfx::SGeomVecN SVertex;
	if ( !pValue->IsValid() )
	{
		vector< SVertex > points;
		CFileStream f;
		strstream str;
		str << "Models\\" << nModelID << char(0);
		if ( f.OpenRead( str.str() ) )
		{
			CStructureSaver file( f, CStructureSaver::READ );
			file.AddDataContainer( 1, &points );
		}

		pValue = NGfx::MakeGeometry( points.size(), SVertex::ID, NGfx::STATIC );
		
		NGfx::CGeomLock<SVertex> geom( pValue );
		memcpy( &geom[0], &points[0], sizeof(SVertex) * points.size() );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CFileGeometry::Serialize( CStructureSaver *pFile )
{
	pFile->AddData( 1, &nModelID );
}
/////////////////////////////////////////////////////////////////////////////////////
// CFileTriList
/////////////////////////////////////////////////////////////////////////////////////
using NGfx::S3DTriangle;
void CFileTriList::Update()
{
	if ( !pValue->IsValid() )
	{
		vector< S3DTriangle > faces;
		CFileStream f;
		strstream str;
		str << "Models\\" << nModelID << char(0);
		if ( f.OpenRead( str.str() ) )
		{
			CStructureSaver file( f, CStructureSaver::READ );
			file.AddDataContainer( 2, &faces );
		}

		pValue = NGfx::MakeTriList( faces.size(), NGfx::STATIC );
		NGfx::CTriListLock tris( pValue );

		memcpy( &tris[0], &faces[0], sizeof(S3DTriangle) * faces.size() );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CFileTriList::Serialize( CStructureSaver *pFile )
{
	pFile->AddData( 1, &nModelID );
}
/////////////////////////////////////////////////////////////////////////////////////
void RegisterGFormat( int nBase )
{
	REGISTER_SAVELOAD_CLASS( nBase + 0, CFileGeometry );
	REGISTER_SAVELOAD_CLASS( nBase + 1, CFileTriList );
}
/////////////////////////////////////////////////////////////////////////////////////
