#include "StdAfx.h"
#include "Win32Helper.h"
#include <D3D8.h>
#include <D3Dx8.h>
#include "GfxInternal.h"
#include "GfxEffects.h"
/////////////////////////////////////////////////////////////////////////////////////
using namespace NGfx;
namespace NGfx
{
/////////////////////////////////////////////////////////////////////////////////////
enum ERenderMode
{
	PS_DIFFUSE = 0,
	PS_TEXTURE,
	PS_SHADOW_TEST,
	PS_LAST
};
enum EVertexShader
{
	VS_DIFFUSE = 0,
	VS_TEXTURE,
	VS_DEPTH,
	VS_SHADOWED_LIGHT,
	VS_CONST_LIGHT,
	VS_LAST
};
/////////////////////////////////////////////////////////////////////////////////////
class CVertexShader
{
	DWORD dwVS;
public:
	CVertexShader() { dwVS = 0; }
	~CVertexShader() { if ( dwVS != 0 ) pDevice->DeleteVertexShader( dwVS ); }
	DWORD GetHandle() const { return dwVS; }
	void Create( const char *pszShader, const DWORD *pdwDataLayout );
};
/////////////////////////////////////////////////////////////////////////////////////
static NWin32Helper::com_ptr<IDirect3DVertexBuffer8> pCurrentVB;
static ERenderMode lastMode;
static DWORD dwStateBlocks[PS_LAST];
static EVertexShader currentVShader;
static int nLastUsedVShader;
static hash_map<int, CVertexShader> vxShaders;
/////////////////////////////////////////////////////////////////////////////////////
// Vertex formats description
/////////////////////////////////////////////////////////////////////////////////////
static DWORD dwVec[] = {
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_END()
};
static DWORD dwVecT[] = {
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(3, D3DVSDT_FLOAT2),
	D3DVSD_END()
};
static DWORD dwVecTC[] = {
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR),
	D3DVSD_REG(3, D3DVSDT_FLOAT2),
	D3DVSD_END()
};
static DWORD dwVecN[] = {
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(1, D3DVSDT_FLOAT3),
	D3DVSD_REG(3, D3DVSDT_FLOAT2),
	D3DVSD_END()
};
SGeomFormatInfo geometryFormatInfo[4] =
{
	{ SGeomVec::ID, sizeof(SGeomVec), dwVec },
	{ SGeomVecT1::ID, sizeof(SGeomVecT1), dwVecT },
	{ SGeomVecT1C1::ID, sizeof(SGeomVecT1C1), dwVecTC },
	{ SGeomVecN::ID, sizeof(SGeomVecN), dwVecN }
};
/////////////////////////////////////////////////////////////////////////////////////
// CVertexShader
/////////////////////////////////////////////////////////////////////////////////////
void CVertexShader::Create( const char *pszShader, const DWORD *pdwDataLayout )
{
	HRESULT hr;
  LPD3DXBUFFER pCode, pError;
	hr = D3DXAssembleShader( pszShader, strlen( pszShader ), 0, NULL, &pCode, &pError );
	const char *pszError = (const char*)pError->GetBufferPointer();
	ASSERT( D3D_OK == hr );
	//
	hr = pDevice->CreateVertexShader( 
		pdwDataLayout, 
		(DWORD*)pCode->GetBufferPointer(),
		&dwVS, 
		D3DUSAGE_SOFTWAREPROCESSING );
	ASSERT( D3D_OK == hr );
	pCode->Release();
	pError->Release();
}
/////////////////////////////////////////////////////////////////////////////////////
// v0 = pos; v1 = normal; v2 = color; v3 = tex1
const char* GetShaderProgram( EVertexShader shader )
{
	switch ( shader )
	{
		case VS_DIFFUSE: return
			"vs.1.0\n"
			"dp4 oPos.x, v0, c0\n"
			"dp4 oPos.y, v0, c1\n"
			"dp4 oPos.z, v0, c2\n"
			"dp4 oPos.w, v0, c3\n"
			"mov oD0, v2\n"
			"\n";
		case VS_TEXTURE: return
			"vs.1.0\n"
			"dp4 oPos.x, v0, c0\n"
			"dp4 oPos.y, v0, c1\n"
			"dp4 oPos.z, v0, c2\n"
			"dp4 oPos.w, v0, c3\n"
			"mov oT0.xy, v3\n"
			"\n";
		case VS_DEPTH: return
			"vs.1.0\n"
			"dp4 r0.x, v0, c0\n"
			"dp4 r0.y, v0, c1\n"
			"dp4 r0.z, v0, c2\n"
			"dp4 r0.w, v0, c3\n"
			"dp4 r1.x, r0, c4\n"
			"mov oPos, r0\n"
			"mov oD0, r1.x\n"
			"\n";
		case VS_SHADOWED_LIGHT: return
			"vs.1.0\n"
			"dp4 r0.x, v0, c0\n"
			"dp4 r0.y, v0, c1\n"
			"dp4 r0.z, v0, c2\n"
			"dp4 r0.w, v0, c3\n"
			"dp4 r1.w, r0, c4\n" // depth
			"dp3 r1.x, v1, v1\n" // normal length
			"rsq r1.x, r1.x\n"
			"mul r2, v1, r1.x\n"
			"dp3 r1.y, r2, c7\n" // r1.y - cos fi
			"mov oPos, r0\n"
			"mul oD0.xyz, r1.y, c8.xyz\n"
			"mov oD0.w, r1.w\n"
			"dp4 oT0.x, r0, c5\n"
			"dp4 oT0.y, r0, c6\n"
			"\n";
		case VS_CONST_LIGHT: return
			"vs.1.0\n"
			"dp4 oPos.x, v0, c0\n"
			"dp4 oPos.y, v0, c1\n"
			"dp4 oPos.z, v0, c2\n"
			"dp4 oPos.w, v0, c3\n"
			"mov oD0, c4\n"
			"\n";
	}
	ASSERT(0);
	return "";
}
static void SelectVertexShader( int nHash, int nFormatID, EVertexShader shader )
{
	DWORD dwShader;
	hash_map<int, CVertexShader>::iterator i = vxShaders.find( nHash );
	if ( i == vxShaders.end() )
	{
		CVertexShader &s = vxShaders[nHash];
		s.Create( GetShaderProgram( shader ), GetVertexLayout( nFormatID ) );
		dwShader = s.GetHandle();
	}
	else
		dwShader = i->second.GetHandle();
	HRESULT hRes = pDevice->SetVertexShader( dwShader );
	ASSERT( hRes == D3D_OK );
}
/////////////////////////////////////////////////////////////////////////////////////
struct SStateBlockCreate
{
	DWORD &dwRes;
	SStateBlockCreate( DWORD &_dwRes ): dwRes(_dwRes ) 
	{
		HRESULT hr = pDevice->BeginStateBlock();
		ASSERT( D3D_OK == hr );
	}
	~SStateBlockCreate()
	{
		HRESULT hr = pDevice->EndStateBlock( &dwRes );
		ASSERT( D3D_OK == hr );
	}
};
static void InitStateBlocks()
{
	{
		SStateBlockCreate s( dwStateBlocks[PS_DIFFUSE] );
		pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	}
	{
		SStateBlockCreate s( dwStateBlocks[PS_TEXTURE] );
		pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	}
	{
		SStateBlockCreate s( dwStateBlocks[PS_SHADOW_TEST] );
		pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		pDevice->SetRenderState( D3DRS_ALPHAREF, 2 );
		pDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL );
		
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SUBTRACT );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		// uncomment texture to see shadow map projection
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );//D3DTA_TEXTURE ); 
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// Some functions for effects realisation
/////////////////////////////////////////////////////////////////////////////////////
static void SetRenderMode( ERenderMode mode )
{
	if ( mode == lastMode )
		return;
	lastMode = mode;
	ASSERT( mode < PS_LAST );
	pDevice->ApplyStateBlock( dwStateBlocks[ mode ] );
}
/////////////////////////////////////////////////////////////////////////////////////
static void SetVertexShader( EVertexShader shader )
{
	currentVShader = shader;
	ASSERT( shader < VS_LAST );
}
/////////////////////////////////////////////////////////////////////////////////////
static void SetVSConst( int nReg, const void *pData, int nSize )
{
	pDevice->SetVertexShaderConstant( nReg, pData, nSize );
}
/////////////////////////////////////////////////////////////////////////////////////
static void SetTexture( int nStage, CTexture *pTex )
{
	ASSERT( pTex->IsValid() );
	if ( pTex->IsValid() )
	{
		HRESULT hRes = pDevice->SetTexture( nStage, pTex->obj );
		ASSERT( hRes == D3D_OK );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// EFFECTS
/////////////////////////////////////////////////////////////////////////////////////
// SEffect
/////////////////////////////////////////////////////////////////////////////////////
void SEffect::ApplyVSConst( const void *pData, int nCount ) const
{
	SetVSConst( 0, pData, nCount );
}
/////////////////////////////////////////////////////////////////////////////////////
void SEffect::ApplyGeometry() const
{
	ASSERT( nVB == 1 ); // multiple vertex streams are not supported
	// multiple streams should have same start offset & same size
	//
	CGeometry *pGeom = vbs[0];
	int nFormatID;
	if ( !pGeom->IsValid() )
	{
		pGeom = GetTempGeometry();
		nFormatID = pGeom->GetFormatID();
	}
	else
	{
		nFormatID = nVBFormat[0];
	}
	ASSERT( pGeom->IsValid() );
	ASSERT( pGeom->GetStride() <= GetGeomFormatSize( nFormatID ) ); // check if correct formatID was specified
	CVB *pVB = pGeom->pDX;
	int nVxShader = ( nFormatID << 16 ) + currentVShader;
	if ( nLastUsedVShader != nVxShader )
	{
		SelectVertexShader( nVxShader, nFormatID, currentVShader );
		nLastUsedVShader = nVxShader;
	}
	if ( pCurrentVB != pVB->obj )
	{
		pCurrentVB = pVB->obj;
		pDevice->SetStreamSource( 0, pVB->obj, pGeom->GetStride() );
	}
	nVBGeomStart = pGeom->GetStart();
	nVBGeomSize = pGeom->GetSize();
}
/////////////////////////////////////////////////////////////////////////////////////
void SEffect::ApplyTexture( int nStage, CTexture *pTex ) const
{
	SetTexture( nStage, pTex );
}
/////////////////////////////////////////////////////////////////////////////////////
void SEffect::SetTransform( const SFBTransform &_transform )
{
	transform = _transform.forward;
}
/////////////////////////////////////////////////////////////////////////////////////
// Effects implementation
/////////////////////////////////////////////////////////////////////////////////////
void SEffDiffuse::Use() const
{
	SetRenderMode( PS_DIFFUSE );
	SetVertexShader( VS_DIFFUSE );
	ApplyGeometry();
	ApplyVSConst( &transform, 4 );
}
/////////////////////////////////////////////////////////////////////////////////////
void SEffTexture::Use() const
{
	SetRenderMode( PS_TEXTURE );
	SetVertexShader( VS_TEXTURE );
	ApplyGeometry();
	ApplyVSConst( &transform, 4 );
	ApplyTexture( 0, pTex );
}
/////////////////////////////////////////////////////////////////////////////////////
void SEffDepth::Use() const
{
	SetRenderMode( PS_DIFFUSE );
	SetVertexShader( VS_DEPTH );
	ApplyGeometry();
	ApplyVSConst( &transform, 5 );
}
/////////////////////////////////////////////////////////////////////////////////////
void SEffShadowedLight::Use() const
{
	SetRenderMode( PS_SHADOW_TEST );
	SetVertexShader( VS_SHADOWED_LIGHT );
	ApplyGeometry();
	ApplyVSConst( &transform, 9 );
	ApplyTexture( 0, pDepth );
}
/////////////////////////////////////////////////////////////////////////////////////
void SEffShadowedLight::SetTransform( const SFBTransform &_transform )
{
	SEffect::SetTransform( _transform );
	_transform.backward.RotateHVector( &vecLightDir, vecLightDirP );
	Normalize( &vecLightDir );
}
/////////////////////////////////////////////////////////////////////////////////////
void SEffConstLight::Use() const
{
	SetRenderMode( PS_DIFFUSE );
	SetVertexShader( VS_CONST_LIGHT );
	ApplyGeometry();
	ApplyVSConst( &transform, 5 );
}
/////////////////////////////////////////////////////////////////////////////////////
// Some callbacks
/////////////////////////////////////////////////////////////////////////////////////
HRESULT NGfx::EffectsInit()
{
	InitStateBlocks();
	lastMode = ERenderMode(-1);
	nLastUsedVShader = -1;
	currentVShader = VS_DIFFUSE;
	return D3D_OK;
}
/////////////////////////////////////////////////////////////////////////////////////
void NGfx::EffectsShutdown()
{
	pCurrentVB = 0;
	for ( int i = 0; i < PS_LAST; i++ )
		pDevice->DeleteStateBlock( dwStateBlocks[i] );
	vxShaders.clear();
}
/////////////////////////////////////////////////////////////////////////////////////
void NGfx::TempVertexStreamChanged()
{
	pCurrentVB = 0;
}
/////////////////////////////////////////////////////////////////////////////////////
} // namespace
/////////////////////////////////////////////////////////////////////////////////////
