#include "StdAfx.h"
#include "SoundFormat.h"
#include "..\FModSound\FMSound.h"
#include "..\DBFormat\DataSound.h"

namespace NSound
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileSample3D::Recalc()
{
	try
	{
		NDb::CSound *pSound = NDb::GetSound( GetKey() );
		if ( !pSound )
			return;
		NGScene::CResourceFileOpener file( "Sounds", GetKey() );

		const int nSize = file->GetSize();
		vector<char> buff( nSize );
		file->Read( &buff[0], nSize );
		pValue = NFMSound::LoadSample3D( &buff[0], nSize, pSound->fMinDistance, pSound->fMaxDistance, pSound->bLoop );
	}
	catch(...)
	{
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileSample2D::Recalc()
{
	try
	{
		NGScene::CResourceFileOpener file( "Sounds", GetKey() );

		const int nSize = file->GetSize();
		vector<char> buff( nSize );
		file->Read( &buff[0], nSize );
		pValue = NFMSound::LoadSample2D( &buff[0], nSize );
	}
	catch(...)
	{
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NSound;
REGISTER_SAVELOAD_CLASS( 0x03081140, CFileSample3D );
REGISTER_SAVELOAD_CLASS( 0x03081141, CFileSample2D );
