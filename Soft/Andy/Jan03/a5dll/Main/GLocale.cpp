#include "StdAfx.h"
#include "DG.h"
#include "FontFormat.h"
#include "GView.h"
#include "GLocale.h"
#include "GFont.h"
#include "GTexture.h"
#include "..\Misc\BasicShare.h"
#include "..\DBFormat\DataFormat.h"
#include "..\DBFormat\DataMap.h"
#include "..\Misc\StrProc.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
externA5 CBasicShare<int, CFileFont> shareFonts;
externA5 CBasicShare<STextureKey, CFileTexture, STextureKeyHash> shareTextures;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFontInfo
{
	int nSize;
	int nSizeIndex;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTextLocaleInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
CTextLocaleInfo::CTextLocaleInfo()
{
	CDBTable<NDb::CTypeface> *pFontTable = NDatabase::GetTable<NDb::CTypeface>();
	CDBIterator<NDb::CTypeface> iTempFont( *pFontTable );
	while( iTempFont.MoveNext() )
	{
		CPtr<NDb::CTypeface> pType = iTempFont.Get();

		CDGPtr< CPtrFuncBase<CFontFormatInfo> > pFormatInfo ( shareFonts.Get( pType->GetRecordID() ) );
		pFormatInfo.Refresh();
		const CFontFormatInfo *pInfo = pFormatInfo->GetValue();
		if ( !pInfo )
			continue;
		// silent-storm-port r26: pType->pTexture is null in records with unfilled
		// fields (CPtr default). Skip this font entry rather than deref null.
		if ( !IsValid( pType->pTexture ) )
			continue;

		fonts.push_back( new CFontInfo( SFont( pInfo->GetHeight(), pType->szName ), shareTextures.Get( pType->pTexture->GetRecordID() ), pFormatInfo ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTextLocaleInfo::Setup( const CVec2 &_vScreenRect )
{
	if ( vScreenRect == _vScreenRect )
		return;

//	fontCache.clear();
	vScreenRect = _vScreenRect;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CFontInfo* CTextLocaleInfo::SearchFont( const SFont &sFont )
{
	int nSize;
	CPtr<CFontInfo> pResFontInfo;

	for ( vector< CObj<CFontInfo> >::iterator iTemp = fonts.begin(); iTemp != fonts.end(); iTemp++ )
	{
		const SFont &sDBFont = (*iTemp)->GetType();
		if ( sDBFont.szName != sFont.szName )
			continue;

		if ( IsValid( pResFontInfo ) )
		{
			if ( abs( sDBFont.nSize - sFont.nSize ) < abs( nSize - sFont.nSize ) )
			{
				nSize = sDBFont.nSize;
				pResFontInfo = (*iTemp);
			}
		}
		else
		{
			nSize = sDBFont.nSize;
			pResFontInfo = (*iTemp);
		}
	}

	return pResFontInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CFontInfo* CTextLocaleInfo::GetFont( const SFont &sFont )
{
	CPtr<CFontInfo> pResFontInfo = 0;

	pResFontInfo = SearchFont( sFont );
	if ( !pResFontInfo )
		pResFontInfo = SearchFont( SFont( 16, "System" ) );

	return pResFontInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x02931162, CTextLocaleInfo )
REGISTER_SAVELOAD_CLASS( 0x020c1140, CFontInfo )
