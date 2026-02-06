#include "StdAfx.h"
#include "GMatShare.h"
#include "GMaterial.h"
#include "..\DBFormat\DataFormat.h"
#include "..\Misc\BasicShare.h"
#include "GTexture.h"
#include "GSceneUtils.h"

#include "..\Misc\RandomGen.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSkyAdapter
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSkyAdapter::NeedUpdate() 
{ 
	if ( !pTex )
		return false;
	return pTex.Refresh(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSkyAdapter::Recalc()
{
	pValue = 0;
	if ( pTex )
		pValue = pTex->GetValue();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSkyAdapter::SetSource( CPtrFuncBase<NGfx::CCubeTexture> *_pTex ) 
{ 
	pTex = _pTex;
	pValue = 0;
	Updated();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
extern CBasicShare<STextureKey, CFileTexture, STextureKeyHash> shareTextures;
static IMaterial* CreateMaterialFromDBRecord( NDb::CMaterial *_pMaterial, 
	CPtrFuncBase<NGfx::CCubeTexture> *pSky )
{
	CVec3 vColor( 1, 0.3f, 0.3f );
	bool bWrap = _pMaterial->addrMode == NDb::CMaterial::AM_WRAP;
	CObj<CPtrFuncBase<NGfx::CTexture> > pTexture, pBump, pGloss, pMirror;
	if ( IsValid( _pMaterial->pTexture ) )
	{
		pTexture = shareTextures.Get( STextureKey( _pMaterial->pTexture->GetRecordID(), bWrap ) );
		CVec4 v = NGfx::GetCVec4Color( _pMaterial->pTexture->dwAverageColor );
		vColor = CVec3( v.r, v.g, v.b );
	}
	//pBump = shareTextures.Get( 13 );
	if ( IsValid( _pMaterial->pBump ) ) 
		pBump = shareTextures.Get( STextureKey( _pMaterial->pBump->GetRecordID(), bWrap ) );
	if ( IsValid( _pMaterial->pGloss ) ) 
		pGloss = shareTextures.Get( STextureKey( _pMaterial->pGloss->GetRecordID(), bWrap ) );
	if ( IsValid( _pMaterial->pMirror ) ) 
		pMirror = shareTextures.Get( STextureKey( _pMaterial->pMirror->GetRecordID(), bWrap ) );
	//_pMaterial->vSpecColor = CVec3(1,1,1);
	//_pMaterial->fSpecFactor = 1;//random.GetFloat( 0.2f, 1 );//0.8f;//0.5f;
	EMaterialAlpha alphaMode;
	switch ( _pMaterial->alpha )
	{
		case NDb::CMaterial::A_OPAQUE: 	        alphaMode = MA_OPAQUE; break;
		case NDb::CMaterial::A_ALPHA_TEST:      alphaMode = MA_ALPHA_TEST; break;
		case NDb::CMaterial::A_TRANSPARENT:     alphaMode = MA_TRANSPARENT; break;
		case NDb::CMaterial::A_ADD_TRANSPARENT: alphaMode = MA_ADD_TRANSPARENT; break;
		case NDb::CMaterial::A_OVERLAY:         alphaMode = MA_OVERLAY; break;
	}
	return CreateMaterial( vColor, pTexture, pBump, _pMaterial->fSpecFactor, 
		_pMaterial->vSpecColor, pGloss, pMirror, pSky, 
		_pMaterial->fMetalMirror, _pMaterial->fDielMirror, alphaMode, _pMaterial->bCastShadow );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CMaterialShare
////////////////////////////////////////////////////////////////////////////////////////////////////
IMaterial* CMaterialShare::CreateMaterial( NDb::CMaterial *pMaterial )
{
	return CreateMaterial( pMaterial, pMaterial->GetRecordID() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IMaterial* CMaterialShare::CreateOccluderMaterial()
{
	if ( pOccluder )
		return pOccluder;
	pOccluder = NGScene::CreateOccluderMaterial();
	return pOccluder;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IMaterial* CMaterialShare::CreateMaterial( NDb::CMaterial *pMaterial, int key )
{
	CMatHashmap::iterator i = materials.find( key );
	if ( i != materials.end() )
		return i->second;
	IMaterial *pRes = CreateMaterialFromDBRecord( pMaterial, pSky );
	materials[key] = pRes;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CColorMaterialShare
////////////////////////////////////////////////////////////////////////////////////////////////////
IMaterial* CColorMaterialShare::CreateMaterial( const CVec3 &color )
{
	CMatHashmap::iterator i = materials.find( color );
	if ( i != materials.end() )
		return i->second;
	IMaterial *pRes = NGScene::CreateMaterial( color );

	materials[color] = pRes;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CTransparentMaterialShare
////////////////////////////////////////////////////////////////////////////////////////////////////
IMaterial* CTransparentMaterialShare::CreateMaterial( const CVec4 &color )
{
	CMatHashmap::iterator i = materials.find( color );
	if ( i != materials.end() )
		return i->second;
	CObj<CPtrFuncBase<NGfx::CTexture> > pTexture = new CColorTexture( color );//shareTextures.Get( 1724 );
	IMaterial *pRes = NGScene::CreateMaterial( 
		CVec3( 1, 1, 1 ), pTexture, 0, 0, CVec3(0,0,0), 0, 0, 0, 0, 0, MA_TRANSPARENT );
	materials[color] = pRes;
	return pRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x01671182, CMaterialShare )
REGISTER_SAVELOAD_CLASS( 0x01812140, CSkyAdapter )
