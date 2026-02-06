#include "StdAfx.h"
#include "GScene.h"
#include "GfxEffects.h"
#include "Transform.h"
#include "GSceneInternal.h"
#include "GFormat.h"
/////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
static NGScene::CGScene* pCurrentScene = 0;
/////////////////////////////////////////////////////////////////////////////////////
template<class TSet, class TFunc>
inline void DrawSet( TSet *pSet, TFunc f )
{
	TSet::iterator i;
	for ( i = pSet->begin(); i != pSet->end(); )
	{
		if ( (*i)->IsValid() )
		{
			f( *i );
			++i;
		}
		else
			i = pSet->erase( i );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
struct SDrawGeometry
{
	NGfx::SEffect *pEffect;
	//
	SDrawGeometry( NGfx::SEffect *_pEffect ): pEffect(_pEffect) {}
	operator()( CRenderNode *pNode ) { pNode->DrawGeometry( pEffect ); }
};
/////////////////////////////////////////////////////////////////////////////////////
struct SDrawDiffuse
{
	operator()( CRenderNode *pNode ) { pNode->DrawDiffuse(); }
};
/////////////////////////////////////////////////////////////////////////////////////
// CMSRNode
/////////////////////////////////////////////////////////////////////////////////////
void CMSRNode::Update()
{
	if ( pAncestor.Refresh() | pPos.Refresh() )
	{
		// CRAP - slow implementation (special cases are not taken into account)
		MultiplyF43( &value.forward, pPos->GetValue(), pAncestor->GetValue().forward );
		InvertMatrix( value.forward, &value.backward );
		Updated();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CMSRNode::Serialize( CStructureSaver *pFile )
{
	pFile->AddObject( 1, &pAncestor ); 
	pFile->AddObject( 2, &pPos );
}
/////////////////////////////////////////////////////////////////////////////////////
// CCubeGeometry
/////////////////////////////////////////////////////////////////////////////////////
void CCubeGeometry::Update()
{
	typedef NGfx::SGeomVecT1C1 SVertex;
	CVec3 base(0,0,0), size(1,1,1);
	if ( !pValue->IsValid() )
	{
		pValue = NGfx::MakeGeometry( 8, SVertex::ID, NGfx::STATIC );
		
		NGfx::CGeomLock<SVertex> geom( pValue );
		CVec3 gpos[8];
		gpos[0] = CVec3( base.x,          base.y,          base.z );
		gpos[1] = CVec3( base.x,          base.y + size.y, base.z );
		gpos[2] = CVec3( base.x + size.x, base.y + size.y, base.z );
		gpos[3] = CVec3( base.x + size.x, base.y         , base.z );
		gpos[4] = CVec3( base.x,          base.y,          base.z + size.z);
		gpos[5] = CVec3( base.x,          base.y + size.y, base.z + size.z );
		gpos[6] = CVec3( base.x + size.x, base.y + size.y, base.z + size.z );
		gpos[7] = CVec3( base.x + size.x, base.y         , base.z + size.z );
		for ( int i = 0; i < 8; i++ )
		{
			geom[i].pos = gpos[i];
			//unsigned char c = (unsigned char) gpos[i].z * 10;
			geom[i].color.dwColor = rand();//SColor(c,c,c,c);
			//geom[i].tex.u = 0.5f + ( gpos[i] - lightCenter ) * lightX / 256.0f / FP_LT_SCALE;
			//geom[i].tex.v = 0.5f - ( gpos[i] - lightCenter ) * lightZ / 256.0f / FP_LT_SCALE;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// CSimpleTriList
/////////////////////////////////////////////////////////////////////////////////////
using NGfx::S3DTriangle;
void CCubeTriList::Update()
{
	if ( !pValue->IsValid() )
	{
		pValue = NGfx::MakeTriList( 12, NGfx::STATIC );
		NGfx::CTriListLock tris( pValue );
		tris[0] = S3DTriangle( 0, 1, 2 );
		tris[1] = S3DTriangle( 0, 2, 3 );
		tris[2] = S3DTriangle( 3, 6, 7 );
		tris[3] = S3DTriangle( 3, 2, 6 );
		tris[4] = S3DTriangle( 0, 7, 4 );
		tris[5] = S3DTriangle( 0, 3, 7 );
		tris[6] = S3DTriangle( 5, 0, 4 );
		tris[7] = S3DTriangle( 5, 1, 0 );
		tris[8] = S3DTriangle( 6, 1, 5 );
		tris[9] = S3DTriangle( 6, 2, 1 );
		tris[10] = S3DTriangle( 4, 6, 5 );
		tris[11] = S3DTriangle( 4, 7, 6 );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// CRenderNode
/////////////////////////////////////////////////////////////////////////////////////
void CRenderNode::Update()
{
	pPlacement.Refresh();
	pGeometry.Refresh();
	pTriList.Refresh();
}
/////////////////////////////////////////////////////////////////////////////////////
void CRenderNode::DrawGeometry(NGfx::SEffect *pEffect )
{
	Update();
	pEffect->SetTransform( pPlacement->GetValue() );
	pEffect->ResetGeometry();
	pEffect->AddGeometry( pGeometry->GetValue() );
	pEffect->Use();
	NGfx::DrawPrimitive( *pTriList->GetValue() );
}
/////////////////////////////////////////////////////////////////////////////////////
void CRenderNode::DrawDiffuse()
{
	Update();
	NGfx::SEffDiffuse d;
	d.SetTransform( pPlacement->GetValue() );
	d.AddGeometry( pGeometry->GetValue() );
	d.Use();
	NGfx::DrawPrimitive( *pTriList->GetValue() );
}
/////////////////////////////////////////////////////////////////////////////////////
void CRenderNode::Serialize( CStructureSaver *pFile )
{
	pFile->AddObject( 1, &pPlacement );
	pFile->AddObject( 2, &pGeometry );
	pFile->AddObject( 3, &pTriList );
}
/////////////////////////////////////////////////////////////////////////////////////
// CShadowedLight
/////////////////////////////////////////////////////////////////////////////////////
void CShadowedLight::Update()
{
	if ( !pTexture->IsValid() )
	{
		pTarget = NGfx::MakeRenderTarget( 256, 256, 0 );
		pTexture = pTarget->GetTexture();
	}
	//
	NGfx::SetRenderTarget( pTarget );
	NGfx::ClearBuffers( 0 );
	//
	NGfx::SEffDepth d;
	d.vecDepth = vecDepthP;
	//
	CGScene &scene = *pCurrentScene;
	scene.SetRootTransform( ts.Get() );
	DrawSet( &scene.visible, SDrawGeometry( &d ) );
}
/////////////////////////////////////////////////////////////////////////////////////
static CVec4 CalcProjectVector( const CVec3 &ptCameraSpace, const SHMatrix &cameraPos, 
															 float fWidth, float fBias )
{
	CVec3 ptVecU;
	cameraPos.RotateVector( &ptVecU, ptCameraSpace );
	ptVecU *= 0.5f / fWidth;
	return CVec4( ptVecU.x, ptVecU.y, ptVecU.z, fBias + 0.5f - ptVecU * cameraPos.GetTranslation() );
}
/////////////////////////////////////////////////////////////////////////////////////
void CShadowedLight::SetupLight( const CVec3 &ptCenter, const CVec3 &ptLight, float fHalfSize, float fpMaxHeight )
{
	CVec3 ptLightDir( ptLight );
	CVec3 ptCameraPos( ptCenter );
	//
	Normalize( &ptLightDir );
	ASSERT( ptLightDir.z < 0 );
	ptCameraPos += ptLightDir * ( ( fpMaxHeight + fHalfSize - ptCameraPos.z ) / ptLightDir.z );
	float fWidth = fHalfSize * 2;
	ts.MakeParallel( fWidth, fWidth );
	//
	SHMatrix cameraPos;
	float fpTang = atan2( ptLightDir.z, sqrt( sqr( ptLightDir.x ) + sqr( ptLightDir.y ) ) );
	float fpRisk = atan2( ptLightDir.y, ptLightDir.x ) - FP_PI2;
	MakeMatrix( &cameraPos, fpTang, fpRisk, ptCameraPos );
	ts.SetCamera( cameraPos );
	vecDepth = CVec4( 0, 0, 1 / fpMaxHeight, 0 );
	vecU = CalcProjectVector( CVec3( 1, 0, 0 ), cameraPos, fWidth * 0.5f, 0.5f/256 );
	vecV = CalcProjectVector( CVec3( 0, 0, -1 ), cameraPos, fWidth * 0.5f, 0 );
	vecLightDir = ptLightDir; 
	vecLightDir.w = 0;
	//
	const SHMatrix &invTrans = ts.Get().backward;
	invTrans.RotateHVectorTransposed( &vecDepthP, vecDepth );
}
/////////////////////////////////////////////////////////////////////////////////////
void CShadowedLight::Draw()
{
	CGScene &scene = *pCurrentScene;
	const SFBTransform &rootTrans = scene.GetRootTransform();
	//ShowTexture( pValue );
	//
	const SHMatrix &srcTransform = rootTrans.forward;
	NGfx::SEffShadowedLight light;
	//
	srcTransform.RotateHVector( &light.vecLightDirP, vecLightDir );
	//
	const SHMatrix &invTrans = rootTrans.backward;
	invTrans.RotateHVectorTransposed( &light.vecDepth, vecDepth );
	invTrans.RotateHVectorTransposed( &light.vecU, vecU );
	invTrans.RotateHVectorTransposed( &light.vecV, vecV );
	//
	light.color = CVec4( 0.3f, 0.3f, 0.3f, 0 );
	light.pDepth = pTexture;
	DrawSet( &scene.visible, SDrawGeometry( &light ) );
}
/////////////////////////////////////////////////////////////////////////////////////
void CShadowedLight::Serialize( CStructureSaver *pFile )
{
	pFile->AddData( 1, &vecU );
	pFile->AddData( 2, &vecV );
	pFile->AddData( 3, &vecDepth );
	pFile->AddData( 4, &vecDepthP );
	pFile->AddData( 5, &ts );
	pFile->AddData( 6, &vecLightDir );
}
/////////////////////////////////////////////////////////////////////////////////////
// CHilight
/////////////////////////////////////////////////////////////////////////////////////
void CHilight::SetupLight( CRenderNode *_pDest, const CVec3 &ptColor )
{
	pDest = _pDest;
	color = ptColor;
}
/////////////////////////////////////////////////////////////////////////////////////
void CHilight::Draw()
{
	NGfx::SEffConstLight light;
	light.color = color;
	if ( pDest->IsValid() )
		pDest->DrawGeometry( &light );
	else
		ASSERT(0);
}
/////////////////////////////////////////////////////////////////////////////////////
void CHilight::Serialize( CStructureSaver *pFile )
{
	pFile->AddObject( 1, &pDest );
	pFile->AddData( 2, &color );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// CGScene
/////////////////////////////////////////////////////////////////////////////////////
CRenderNode* CGScene::CreateModel( int nModelID, CFuncBase<SHMatrix> *pPlacement )
{
	CMSRNode *pMSR = new CMSRNode;
	pMSR->pAncestor = pTransformRoot;
	pMSR->pPos = pPlacement;
	//
	CRenderNode *pR = new CRenderNode;
	pR->pPlacement = pMSR;
	pR->pGeometry = new CFileGeometry( nModelID );
	pR->pTriList = new CFileTriList( nModelID );
	visible.push_back( pR );
	return pR;
}
/////////////////////////////////////////////////////////////////////////////////////
CBaseNode* CGScene::AddHilight( CRenderNode *pDst, const CVec3 &ptColor )
{
	CHilight *pLight = new CHilight;
	pLight->SetupLight( pDst, ptColor );
	lights.push_back( pLight );
	return pLight;
}
/////////////////////////////////////////////////////////////////////////////////////
void CGScene::AddLight( const CVec3 &ptCenter, const CVec3 &ptLight, float fpHalfSize, float fpMaxHeight )
{
	CShadowedLight *pLight = new CShadowedLight;
	pLight->SetupLight( ptCenter, ptLight, fpHalfSize, fpMaxHeight );
	lights.push_back( pLight );
}
/////////////////////////////////////////////////////////////////////////////////////
void CGScene::Draw( CTransformStack *pTS )
{
	pCurrentScene = this;
	NGfx::ClearBuffers();
	UpdateSet( &lights );
	NGfx::SetRenderTarget();
	//
	SetRootTransform( pTS->Get() );
	// draw diffuse light
	for ( list< CPtr<CLightBase> >::iterator k = lights.begin(); k != lights.end(); ++k )
	{
		(*k)->Draw();
	}
	// draw scene
/*	
	for ( list< CPtr<CRenderNode> >::iterator i = visible.begin(); i != visible.end(); ++i )
	{
		(*i)->Draw( pTS );
	}*/
	// draw specular light
	//
	pCurrentScene = 0;
	NGfx::Flip();
}
/////////////////////////////////////////////////////////////////////////////////////
void CGScene::Serialize( CStructureSaver *pFile )
{
	CGSceneBase::Serialize( pFile );
	pFile->AddContainer( 10, &visible );
	pFile->AddContainer( 11, &lights );
	pFile->AddObject( 12, &pTransformRoot );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
CGSceneBase* CreateNewScene()
{
	return new CGScene;
}
/////////////////////////////////////////////////////////////////////////////////////
bool Is3DActive()
{
	return NGfx::Is3DActive();
}
/////////////////////////////////////////////////////////////////////////////////////
void SetWireframe( bool bWire )
{
	NGfx::SetWireframe( bWire );
}
/////////////////////////////////////////////////////////////////////////////////////
} // namespace
/////////////////////////////////////////////////////////////////////////////////////
using namespace NGScene;
void RegisterGSceneClasses( int nBase )
{
	REGISTER_SAVELOAD_CLASS( nBase + 0, CMSRNode );
	REGISTER_SAVELOAD_CLASS( nBase + 1, CCubeGeometry );
	REGISTER_SAVELOAD_CLASS( nBase + 2, CCubeTriList );
	REGISTER_SAVELOAD_CLASS( nBase + 3, CRenderNode );
	REGISTER_SAVELOAD_CLASS( nBase + 4, CShadowedLight );
	REGISTER_SAVELOAD_CLASS( nBase + 5, CHilight );
	REGISTER_SAVELOAD_CLASS( nBase + 6, CGScene );
	RegisterSceneGraphClasses( nBase + 0x100000 ); 
	RegisterGFormat( nBase + 0x200000 );
}
/////////////////////////////////////////////////////////////////////////////////////
