#include "StdAfx.h"
#include "DG.h"
#include "GfxBuffers.h"
#include "GScene.h"
#include "GCombiner.h"
#include "Bound.h"

typedef NGfx::SGeomVecFull SGfxVertex;
typedef NGfx::SGeomVecNT1 STnLVertex;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// IPart
////////////////////////////////////////////////////////////////////////////////////////////////////
IPart::IPart( CPtrFuncBase<CObjectInfo> *pData, CPerMaterialCombiner *_pCombiner )
	: pObjInfo(pData), pCombiner( _pCombiner ) 
{
	if ( IsValid( pCombiner ) )
		pCombiner->AddPart( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
IPart::~IPart()
{
	if ( IsValid( pCombiner ) ) 
		pCombiner->RemovePart( this ); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void IPart::SetCombiner( CPerMaterialCombiner *_pCombiner, bool bForceUpdate, bool bAnimated )
{
	if ( pCombiner == _pCombiner )
	{
		if ( IsValid( pCombiner ) )
		{
			if ( bForceUpdate )
				pCombiner->MarkWasted();
			if ( bAnimated )
				pCombiner->Animated();
		}
		return;
	}
	if ( IsValid( pCombiner ) )
		pCombiner->RemovePart( this );
	pCombiner = _pCombiner;
	if ( IsValid( pCombiner ) )
		pCombiner->AddPart( this );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CPerMaterialCombiner
////////////////////////////////////////////////////////////////////////////////////////////////////
CPerMaterialCombiner::CPerMaterialCombiner( CVersioningBase *_pTracker ) : pTracker(_pTracker)
{
	pAnimation = new CVersioningBase;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CPerMaterialCombiner::~CPerMaterialCombiner()
{
	if ( IsValid( pTracker ) )
		pTracker->Updated();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPerMaterialCombiner::AddPart( IPart *pPart )
{
	ASSERT( value.size() < PF_MAX_PARTS_PER_COMBINER );
	value.push_back( pPart );
	Updated();
	if ( IsValid( pTracker ) )
		pTracker->Updated();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPerMaterialCombiner::RemovePart( IPart *pPart )
{
	vector< CPtr<IPart> >::iterator i = find( value.begin(), value.end(), pPart );
	if ( i == value.end() )
		return;
	value.erase( i );
	Updated();
	if ( IsValid( pTracker ) )
		pTracker->Updated();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPerMaterialCombiner::MarkWasted()
{
	Updated();
	if ( IsValid( pTracker ) )
		pTracker->Updated();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CPerMaterialCombiner::operator&( CStructureSaver &f )
{
	f.Add( 1, &value );
	f.Add( 2, &pTracker );
	f.Add( 3, &pAnimation );
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAutomaticCombiner
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAutomaticCombiner::NeedUpdate()
{
	bool bRes = false;
	for ( vector< CPtr<IPart> >::iterator i = value.begin(); i != value.end(); )
	{
		if ( IsValid( *i ) )
			++i;
		else
		{
			i = value.erase( i );
			bRes = true;
		}
	}
	return bRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// SPartTransformInfo
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SMMXWord
{
	short nZ, nY, nX, nW;
};
struct SCompactTransformer
{
	SMMXWord a, b, c;
};
struct SMMXFixups
{
	SMMXWord normalFixup, shiftedFixup;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CreateFixups( SMMXFixups *pRes )
{
	short nShift = (short)0x8000;
	SMMXWord &a = pRes->normalFixup;
	a.nZ = nShift; a.nY = nShift; a.nX = nShift; a.nW = 0;
	SMMXWord &b = pRes->shiftedFixup;
	short nFixShift = (short)0x8080;
	b.nX = nFixShift; b.nY = nFixShift; b.nZ = nFixShift; b.nW = 0; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void AssignTransposed( SCompactTransformer *pRes, const SHMatrix &m )
{
	pRes->a.nZ = Float2Int( m._33 * 0x800 );  pRes->a.nY = Float2Int( m._22 * 0x800 );  pRes->a.nX = Float2Int( m._11 * 0x800 );  pRes->a.nW = 0;
	pRes->b.nZ = Float2Int( m._13 * 0x800 );  pRes->b.nY = Float2Int( m._32 * 0x800 );  pRes->b.nX = Float2Int( m._21 * 0x800 );  pRes->b.nW = 0;
	pRes->c.nZ = Float2Int( m._23 * 0x800 );  pRes->c.nY = Float2Int( m._12 * 0x800 );  pRes->c.nX = Float2Int( m._31 * 0x800 );  pRes->c.nW = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void Assign( SCompactTransformer *pRes, const SHMatrix &m )
{
	pRes->a.nZ = Float2Int( m._33 * 0x800 );  pRes->a.nY = Float2Int( m._22 * 0x800 );  pRes->a.nX = Float2Int( m._11 * 0x800 );  pRes->a.nW = 0;
	pRes->b.nZ = Float2Int( m._31 * 0x800 );  pRes->b.nY = Float2Int( m._23 * 0x800 );  pRes->b.nX = Float2Int( m._12 * 0x800 );  pRes->b.nW = 0;
	pRes->c.nZ = Float2Int( m._32 * 0x800 );  pRes->c.nY = Float2Int( m._21 * 0x800 );  pRes->c.nX = Float2Int( m._13 * 0x800 );  pRes->c.nW = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static short nNormalizeTable[16384];
static SMMXWord mmxWeights[512];
static struct SNormalizeInit
{
	SNormalizeInit() 
	{
		nNormalizeTable[0] = 0x7fff;
		for ( int k = 1; k < ARRAY_SIZE(nNormalizeTable); ++k )
			nNormalizeTable[k] = Min( 0x7fff, Float2Int( (64 * (127 * 16)) / sqrt( k ) ) );
		for ( int k = 0; k < ARRAY_SIZE(mmxWeights); ++k )
		{
			SMMXWord &a = mmxWeights[k];
			a.nZ = a.nY = a.nX = a.nW = k << 6;
		}
	}
} normalizeInit;
////////////////////////////////////////////////////////////////////////////////////////////////////
// disable no emms warning, emms is placed after all mmx calcs
#pragma warning( disable : 4799 )
static void MMXTransformVector( NGfx::SCompactVector *pRes, const NGfx::SCompactVector *pSrc, const SMMXFixups *pFixups,
	const SCompactTransformer *pTrans )
{
	ASSERT( pSrc->w == 0 );
	_asm
	{
		mov esi, pSrc
		movd mm7, [esi]
		mov esi, pTrans
		mov edi, pFixups
		pxor mm0, mm0
		punpcklbw mm0, mm7 // unpacked vector
		psubw mm0, [edi]
		
		movq mm1, mm0    // z y x
		pmulhw mm1, [esi] 
		movq mm2, mm0
		movq mm3, mm0
		psllq mm2, 16
		psrlq mm3, 32
		paddw mm2, mm3   // x z y
		pmulhw mm2, [esi+8]
		movq mm3, mm0
		movq mm4, mm0
		paddsw mm1, mm2
		psllq mm3, 32
		psrlq mm4, 16
		paddw mm3, mm4   // y x z
		pmulhw mm3, [esi+16]
		paddsw mm1, mm3 // packed result
		// normalize
		psllw mm1, 5
		movq mm2, mm1
		pmaddwd mm2, mm2
		movq mm3, mm2
		psrlq mm3, 32
		paddd mm2, mm3
		movd ebx, mm2
		shr ebx, 18
		xor eax, eax
		mov ax, [nNormalizeTable + ebx*2]
		movd mm2, eax
		movq mm3, mm2
		psllq mm3, 16
		paddw mm2, mm3
		movq mm3, mm2
		psllq mm3, 32
		paddw mm2, mm3
		pmulhw mm1, mm2
		psllw mm1, 5
		// pack and output result
		paddw mm1, [edi+8]
		psrlw mm1, 8
		packuswb mm1, mm1
		mov esi, pRes
		movd [esi], mm1
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void MMXTransformVector2( NGfx::SCompactVector *pRes, const NGfx::SCompactVector *pSrc, const SMMXFixups *pFixups,
	const SCompactTransformer *pTrans, char w1,
	const SCompactTransformer *pTrans2, char w2 )
{
	ASSERT( pSrc->w == 0 );
	_asm
	{
		mov esi, pSrc
		movd mm7, [esi]
		mov esi, pTrans
		mov ebx, pTrans2
		mov edi, pFixups
		pxor mm0, mm0
		punpcklbw mm0, mm7 // unpacked vector
		psubw mm0, [edi]

		movq mm1, mm0    // z y x
		movq mm5, mm1
		pmulhw mm1, [esi] 
		pmulhw mm5, [ebx]
		movq mm2, mm0
		movq mm3, mm0
		psllq mm2, 16
		psrlq mm3, 32
		paddw mm2, mm3   // x z y
		movq mm6, mm2
		pmulhw mm2, [esi+8]
		pmulhw mm6, [ebx+8]
		movq mm3, mm0
		movq mm4, mm0
		paddsw mm1, mm2
		paddsw mm5, mm6
		psllq mm3, 32
		psrlq mm4, 16
		paddw mm3, mm4   // y x z
		movq mm6, mm3
		pmulhw mm3, [esi+16]
		pmulhw mm6, [ebx+16]
		paddsw mm1, mm3 // packed result
		paddsw mm5, mm6
		movzx esi, w1
		movzx ebx, w2
		psllw mm1, 4
		psllw mm5, 4
		pmulhw mm1, qword ptr[esi*8 + mmxWeights]
		pmulhw mm5, qword ptr[ebx*8 + mmxWeights]
		paddsw mm1, mm5
		// normalize
		psllw mm1, 3
		movq mm2, mm1
		pmaddwd mm2, mm2
		movq mm3, mm2
		psrlq mm3, 32
		paddd mm2, mm3
		movd ebx, mm2
		shr ebx, 18
		xor eax, eax
		mov ax, [nNormalizeTable + ebx*2]
		movd mm2, eax
		movq mm3, mm2
		psllq mm3, 16
		paddw mm2, mm3
		movq mm3, mm2
		psllq mm3, 32
		paddw mm2, mm3
		pmulhw mm1, mm2
		psllw mm1, 5
		// pack and output result
		paddw mm1, [edi+8]
		psrlw mm1, 8
		packuswb mm1, mm1
		mov esi, pRes
		movd [esi], mm1
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void MMXTransformVector3( NGfx::SCompactVector *pRes, const NGfx::SCompactVector *pSrc, const SMMXFixups *pFixups,
	const SCompactTransformer *pTrans, char w1,
	const SCompactTransformer *pTrans2, char w2,
	const SCompactTransformer *pTrans3, char w3 )
{
	ASSERT( pSrc->w == 0 );
	_asm
	{
		mov esi, pSrc
		movd mm7, [esi]
		mov esi, pTrans
		mov ebx, pTrans2
		mov edx, pTrans3
		mov edi, pFixups
		pxor mm0, mm0
		punpcklbw mm0, mm7 // unpacked vector
		psubw mm0, [edi]

		movq mm1, mm0    // z y x
		movq mm5, mm1
		movq mm7, mm1
		pmulhw mm1, [esi] 
		pmulhw mm5, [ebx]
		pmulhw mm7, [edx]
		movq mm2, mm0
		movq mm3, mm0
		psllq mm2, 16
		psrlq mm3, 32
		paddw mm2, mm3   // x z y
		movq mm6, mm2
		movq mm3, mm2
		pmulhw mm2, [esi+8]
		pmulhw mm6, [ebx+8]
		pmulhw mm3, [edx+8]
		paddsw mm7, mm3
		movq mm3, mm0
		movq mm4, mm0
		paddsw mm1, mm2
		paddsw mm5, mm6
		psllq mm3, 32
		psrlq mm4, 16
		paddw mm3, mm4   // y x z
		movq mm6, mm3
		movq mm4, mm3
		pmulhw mm3, [esi+16]
		pmulhw mm6, [ebx+16]
		pmulhw mm4, [edx+16]
		paddsw mm1, mm3 // packed result
		paddsw mm5, mm6
		paddsw mm7, mm4
		movzx esi, w1
		movzx ebx, w2
		movzx edx, w3
		psllw mm1, 4
		psllw mm5, 4
		psllw mm7, 4
		pmulhw mm1, qword ptr[esi*8 + mmxWeights]
		pmulhw mm5, qword ptr[ebx*8 + mmxWeights]
		pmulhw mm7, qword ptr[edx*8 + mmxWeights]
		paddsw mm1, mm5
		paddsw mm1, mm7
		// normalize
		psllw mm1, 3
		movq mm2, mm1
		pmaddwd mm2, mm2
		movq mm3, mm2
		psrlq mm3, 32
		paddd mm2, mm3
		movd ebx, mm2
		shr ebx, 18
		xor eax, eax
		mov ax, [nNormalizeTable + ebx*2]
		movd mm2, eax
		movq mm3, mm2
		psllq mm3, 16
		paddw mm2, mm3
		movq mm3, mm2
		psllq mm3, 32
		paddw mm2, mm3
		pmulhw mm1, mm2
		psllw mm1, 5
		// pack and output result
		paddw mm1, [edi+8]
		psrlw mm1, 8
		packuswb mm1, mm1
		mov esi, pRes
		movd [esi], mm1
	}
}
#pragma warning( default : 4799 )
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TransformVertexT( CVec3 *pRes, const SHMatrix &m, const NGfx::SCompactVector &src )
{
	//CVec3 vRes;
	m.RotateVectorTransposed( pRes, NGfx::GetVector( src ) );
	Normalize( pRes );
/*  //NGfx::SCompactVector *pRes	
	NGfx::SCompactVector test; 
	NGfx::CalcCompactVector( &test, vRes );
	SCompactTransformer transformer;
	AssignTransposed( &transformer, m );
	MMXTransformVector( pRes, &transformer, &src );
	ASSERT( fabs( NGfx::GetVector(test) - NGfx::GetVector(*pRes) ) < 0.02f )*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TransformPosition( const vector<CVec3> &srcPos, CVec3 *pRes, const SHMatrix &m )
{
	for ( int k = 0; k < srcPos.size(); ++k )
		m.RotateHVector( pRes++, srcPos[k] );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TransformPosition( const vector<CVec3> &srcPos, CVec3 *pRes, const SHMatrix &m, const SDiscretePos &dpos )
{
	for ( int k = 0; k < srcPos.size(); ++k )
	{
		CVec3 vPos( srcPos[k] );
		dpos.MoveAndRotate( &vPos );
		m.RotateHVector( pRes++, vPos );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TransformPosition( const vector<CVec3> &srcPos, CVec3 *pRes, const SRealVertexWeight *pWeight, const vector<SHMatrix> &blends )
{
	for ( int k = 0; k < srcPos.size(); ++k, ++pWeight )
	{
		if ( pWeight->nWeights[1] == 0 )
		{
			const SHMatrix &blend = blends[ pWeight->cBoneIndices[0] ];
			blend.RotateHVector( pRes++, srcPos[k] );
		}
		else
		{
			CVec3 vPos( VNULL3 ), p;
			for ( int j = 0; pWeight->nWeights[j] && j < 4; ++j )
			{
				const SHMatrix &blend = blends[ pWeight->cBoneIndices[j] ];
				float fW = pWeight->fWeights[j];
				blend.RotateHVector( &p, srcPos[k] );
				vPos.x += fW * p.x;
				vPos.y += fW * p.y;
				vPos.z += fW * p.z;
			}
			*pRes++ = vPos;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGenericTransformer
{
	typedef NGfx::SGeomVecFull TRes;

	void CopyTransform( const vector<CVec3> &srcPos, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		TRes *pRes )
	{
		int k = 0;
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++k )
		{
			TRes &res = *pRes;
			const CVec3 &pos = srcPos[ posIndices[k] ];
			res.pos = pos;
			res.normal = pSrc->normal;
			res.tex = pSrc->tex;
			res.texLM.dw = 0;
			res.texU = pSrc->texU;
			res.texV = pSrc->texV;
		}
	}
	void SimpleVectorTransform( const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SHMatrix &vertexTransform, TRes *pRes )
	{
		int k = 0;
		SMMXFixups fixups;
		SCompactTransformer transformer;
		CreateFixups( &fixups );
		AssignTransposed( &transformer, vertexTransform );
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++k )
		{
			TRes &res = *pRes;
			const CVec3 &vPos = transformed[ posIndices[k] ];
			res.pos = vPos;
			MMXTransformVector( &res.normal, &pSrc->normal, &fixups, &transformer );
			//TransformVertexT( &res.normal, vertexTransform, pSrc->normal );
			res.tex = pSrc->tex;
			res.texLM.dw = 0;
			MMXTransformVector( &res.texU, &pSrc->texU, &fixups, &transformer );
			MMXTransformVector( &res.texV, &pSrc->texV, &fixups, &transformer );
			_asm emms;
			//TransformVertexT( &res.texU, vertexTransform, pSrc->texU );
			//TransformVertexT( &res.texV, vertexTransform, pSrc->texV );
		}
	}
	void SimpleTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SFBTransform &trans, TRes *pRes )
	{
		SimpleVectorTransform( transformed, pSrc, posIndices, trans.backward, pRes );
	}
	void SimpleDiscreteTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SHMatrix &transform, const SHMatrix &vertexTransform, const SDiscretePos &dpos, TRes *pRes )
	{
		SimpleVectorTransform( transformed, pSrc,posIndices, vertexTransform, pRes );
	}
	void SingleSkinTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SRealVertexWeight *pWeight, const vector<SHMatrix> &blends, TRes *pRes )
	{
		if ( blends.empty() )
		{
			ASSERT(0);
			return CopyTransform( srcPos, pSrc, posIndices, pRes );
		}

		SMMXFixups fixups;
		CreateFixups( &fixups );
		vector<SCompactTransformer> matrices( blends.size() );
		for ( int k = 0; k < matrices.size(); ++k )
			Assign( &matrices[k], blends[k] );
		int k = 0;
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++pWeight, ++k )
		{
			TRes &res = *pRes;
			NGfx::SCompactVector normal, texU, texV;
			if ( pWeight->nWeights[1] == 0 )
			{
				const SCompactTransformer &blend = matrices[ pWeight->cBoneIndices[0] ];
				MMXTransformVector( &normal, &pSrc->normal, &fixups, &blend );
				MMXTransformVector( &texU, &pSrc->texU, &fixups, &blend );
				MMXTransformVector( &texV, &pSrc->texV, &fixups, &blend );
			}
			else if ( pWeight->nWeights[2] == 0 )
			{
				const SCompactTransformer &blend1 = matrices[ pWeight->cBoneIndices[0] ];
				const SCompactTransformer &blend2 = matrices[ pWeight->cBoneIndices[1] ];
				BYTE nW1 = pWeight->nWeights[0], nW2 = pWeight->nWeights[1];
				MMXTransformVector2( &normal, &pSrc->normal, &fixups, &blend1, nW1, &blend2, nW2 );
				MMXTransformVector2( &texU, &pSrc->texU, &fixups, &blend1, nW1, &blend2, nW2 );
				MMXTransformVector2( &texV, &pSrc->texV, &fixups, &blend1, nW1, &blend2, nW2 );
			}
			else
			{
				const SCompactTransformer &blend1 = matrices[ pWeight->cBoneIndices[0] ];
				const SCompactTransformer &blend2 = matrices[ pWeight->cBoneIndices[1] ];
				const SCompactTransformer &blend3 = matrices[ pWeight->cBoneIndices[2] ];
				BYTE nW1 = pWeight->nWeights[0], nW2 = pWeight->nWeights[1], nW3 = pWeight->nWeights[2];
				MMXTransformVector3( &normal, &pSrc->normal, &fixups, &blend1, nW1, &blend2, nW2, &blend3, nW3 );
				MMXTransformVector3( &texU, &pSrc->texU, &fixups, &blend1, nW1, &blend2, nW2, &blend3, nW3 );
				MMXTransformVector3( &texV, &pSrc->texV, &fixups, &blend1, nW1, &blend2, nW2, &blend3, nW3 );
			}	
			_asm emms;

/*			CVec3 tnormal, ttexU, ttexV;
			if ( pWeight->fWeights[1] == 0 )
			{
				const SHMatrix &blend = blends[ pWeight->cBoneIndices[0] ];
				blend.RotateVector( &tnormal, NGfx::GetVector( pSrc->normal ) );
				blend.RotateVector( &ttexU, NGfx::GetVector( pSrc->texU ) );
				blend.RotateVector( &ttexV, NGfx::GetVector( pSrc->texV ) );
			}
			else
			{
				CVec3 p;
				tnormal = VNULL3;
				ttexU = VNULL3;
				ttexV = VNULL3;
				for ( int j = 0; pWeight->fWeights[j] && j < 4; ++j )
				{
					const SHMatrix &blend = blends[ pWeight->cBoneIndices[j] ];
					float fW = pWeight->fWeights[j];
					blend.RotateVector( &p, NGfx::GetVector( pSrc->normal ) );
					tnormal += fW * p;
					blend.RotateVector( &p, NGfx::GetVector( pSrc->texU ) );
					ttexU += fW * p;
					blend.RotateVector( &p, NGfx::GetVector( pSrc->texV ) );
					ttexV += fW * p;
				}
			}
			Normalize( &tnormal );
			Normalize( &ttexU );
			Normalize( &ttexV );
			ASSERT( fabs( NGfx::GetVector(normal) - tnormal ) < 0.02f );
			ASSERT( fabs( NGfx::GetVector(texU) - ttexU ) < 0.02f );
			ASSERT( fabs( NGfx::GetVector(texV) - ttexV ) < 0.02f );*/

			const CVec3 &vPos = transformed[ posIndices[k] ];
			res.pos = vPos;
			res.normal = normal;
			res.tex = pSrc->tex;
			res.texLM.dw = 0;
			res.texU = texU;
			res.texV = texV;
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
__forceinline void CalcSkinnedNormal( CVec3 *pRes, const CVec3 &srcNormal,
	const SRealVertexWeight *pWeight, const vector<SHMatrix> &blends )
{
	CVec3 &tnormal = *pRes;
	if ( pWeight->fWeights[1] == 0 )
	{
		const SHMatrix &blend = blends[ pWeight->cBoneIndices[0] ];
		blend.RotateVector( &tnormal, srcNormal );
	}
	else
	{
		CVec3 p;
		tnormal = VNULL3;
		for ( int j = 0; pWeight->fWeights[j] && j < 4; ++j )
		{
			const SHMatrix &blend = blends[ pWeight->cBoneIndices[j] ];
			float fW = pWeight->fWeights[j];
			blend.RotateVector( &p, srcNormal );
			tnormal += fW * p;
		}
	}
	Normalize( &tnormal );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct STnLTransformer
{
	typedef STnLVertex TRes;

	void CopyTransform( const vector<CVec3> &srcPos, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		TRes *pRes )
	{
		int k = 0;
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++k )
		{
			TRes &res = *pRes;
			const CVec3 &pos = srcPos[ posIndices[k] ];
			res.pos = pos;
			res.normal = NGfx::GetVector( pSrc->normal );
			res.tex = NGfx::GetTexCoords( pSrc->tex );
		}
	}
	void SimpleVectorTransform( const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SHMatrix &vertexTransform, TRes *pRes )
	{
		int k = 0;
		SMMXFixups fixups;
		SCompactTransformer transformer;
		CreateFixups( &fixups );
		AssignTransposed( &transformer, vertexTransform );
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++k )
		{
			TRes &res = *pRes;
			const CVec3 &vPos = transformed[ posIndices[k] ];
			res.pos = vPos;
			TransformVertexT( &res.normal, vertexTransform, pSrc->normal );
			res.tex = NGfx::GetTexCoords( pSrc->tex );
		}
	}
	void SimpleTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SFBTransform &trans, TRes *pRes )
	{
		SimpleVectorTransform( transformed, pSrc,posIndices, trans.backward, pRes );
	}
	void SimpleDiscreteTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SHMatrix &transform, const SHMatrix &vertexTransform, const SDiscretePos &dpos, TRes *pRes )
	{
		SimpleVectorTransform( transformed, pSrc,posIndices, vertexTransform, pRes );
	}
	void SingleSkinTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SRealVertexWeight *pWeight, const vector<SHMatrix> &blends, TRes *pRes )
	{
		if ( blends.empty() )
		{
			ASSERT(0);
			return CopyTransform( srcPos, pSrc, posIndices, pRes );
		}

		int k = 0;
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++pWeight, ++k )
		{
			TRes &res = *pRes;
			CVec3 tnormal;
			CalcSkinnedNormal( &tnormal, NGfx::GetVector( pSrc->normal ), pWeight, blends );

			const CVec3 &vPos = transformed[ posIndices[k] ];
			res.pos = vPos;
			res.normal = tnormal;
			res.tex = NGfx::GetTexCoords( pSrc->tex );
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct STnLSelectionTransformer
{
	typedef STnLVertex TRes;
	SBoundCalcer bc;
	float fOffset;

	void CopyTransform( const vector<CVec3> &srcPos, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		TRes *pRes )
	{
		int k = 0;
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++k )
		{
			TRes &res = *pRes;
			const CVec3 &pos = srcPos[ posIndices[k] ];
			CVec3 vNormal( NGfx::GetVector( pSrc->normal ) );
			res.pos = pos + vNormal * fOffset;
			res.normal = vNormal;
			res.tex = NGfx::GetTexCoords( pSrc->tex );
		}
	}
	void SimpleVectorTransform( const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SHMatrix &vertexTransform, TRes *pRes )
	{
		int k = 0;
		SMMXFixups fixups;
		SCompactTransformer transformer;
		CreateFixups( &fixups );
		AssignTransposed( &transformer, vertexTransform );
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++k )
		{
			TRes &res = *pRes;
			const CVec3 &vPos = transformed[ posIndices[k] ];
			CVec3 vNormal;
			TransformVertexT( &vNormal, vertexTransform, pSrc->normal );
			res.pos = vPos + vNormal * fOffset;
			res.normal = vNormal;
			res.tex = NGfx::GetTexCoords( pSrc->tex );
		}
	}
	void SimpleTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SFBTransform &trans, TRes *pRes )
	{
		SimpleVectorTransform( transformed, pSrc,posIndices, trans.backward, pRes );
	}
	void SimpleDiscreteTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SHMatrix &transform, const SHMatrix &vertexTransform, const SDiscretePos &dpos, TRes *pRes )
	{
		SimpleVectorTransform( transformed, pSrc,posIndices, vertexTransform, pRes );
	}
	void SingleSkinTransform( const vector<CVec3> &srcPos, const vector<CVec3> &transformed, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SRealVertexWeight *pWeight, const vector<SHMatrix> &blends, TRes *pRes )
	{
		if ( blends.empty() )
		{
			ASSERT(0);
			return CopyTransform( srcPos, pSrc, posIndices, pRes );
		}

		int k = 0;
		for ( const SUVInfo *pEnd = pSrc + posIndices.size(); pSrc < pEnd; ++pSrc, ++pRes, ++pWeight, ++k )
		{
			TRes &res = *pRes;
			CVec3 tnormal;
			CalcSkinnedNormal( &tnormal, NGfx::GetVector( pSrc->normal ), pWeight, blends );

			const CVec3 &vPos = transformed[ posIndices[k] ];
			res.pos = vPos + tnormal * fOffset;
			res.normal = tnormal;
			res.tex = NGfx::GetTexCoords( pSrc->tex );
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGenericPosTransformer
{
	typedef CVec3 TRes;
	void CopyTransform( const vector<CVec3> &srcPos, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		TRes *pRes )
	{
		for ( int k = 0; k < srcPos.size(); ++k )
			pRes[k] = srcPos[k];
	}
	void SimpleTransform( const vector<CVec3> &srcPos, const vector<CVec3> &_fake, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SFBTransform &trans, TRes *pRes )
	{
		TransformPosition( srcPos, pRes, trans.forward );
	}
	void SimpleDiscreteTransform( const vector<CVec3> &srcPos, const vector<CVec3> &_fake, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SHMatrix &transform, const SHMatrix &vertexTransform, const SDiscretePos &dpos, TRes *pRes )
	{
		TransformPosition( srcPos, pRes, transform, dpos );
	}
	void SingleSkinTransform( const vector<CVec3> &srcPos, const vector<CVec3> &_fake, const SUVInfo *pSrc, const vector<WORD> &posIndices, 
		const SRealVertexWeight *pWeight, const vector<SHMatrix> &blends, TRes *pRes )
	{
		if ( blends.empty() )
		{
			ASSERT(0);
			return CopyTransform( srcPos, pSrc, posIndices, pRes );
		}
		TransformPosition( srcPos, pRes, pWeight, blends );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
struct SPartTransformer : public T
{
	int DoTransform( IPart *p, T::TRes *pRes, const vector<CVec3> &transformed )
	{
		CObjectInfo *pObjInfo = p->GetObjectInfo();
		if ( !pObjInfo )
			return 0;
		const vector<SUVInfo> &srcVerts = pObjInfo->GetVertices();
		switch ( p->GetTransformType() )
		{
		case TT_NONE:
			CopyTransform( pObjInfo->GetPositions(), &srcVerts[0], pObjInfo->GetPositionIndices(), 
				pRes );
			break;
		case TT_SIMPLE:
			SimpleTransform( pObjInfo->GetPositions(), transformed, &srcVerts[0], pObjInfo->GetPositionIndices(), 
				p->GetSimplePos(), pRes );
			break;
		case TT_SIMPLE_DISCRETE:
			{
				const SDiscretePos& dPos = p->GetDiscretePos();
				SFBTransform transform;
				dPos.MakeMatrix( &transform );
				SimpleDiscreteTransform( pObjInfo->GetPositions(), transformed, &srcVerts[0], pObjInfo->GetPositionIndices(), 
					dPos.GetTransform()->pos.forward, transform.backward, dPos, pRes );
			}
			break;
		case TT_SINGLE_SKIN:
			SingleSkinTransform( pObjInfo->GetPositions(), transformed, &srcVerts[0], pObjInfo->GetPositionIndices(), 
				&(pObjInfo->GetWeights()[0]), p->GetAnimation(), pRes );
			break;
		default:
			ASSERT(0);
			break;
		}
		return srcVerts.size();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SGfxOutputTransformer : public SPartTransformer<SGenericTransformer>
{
	NGfx::CBufferLock<SGfxVertex> geom;
	int nVert;

	SGfxOutputTransformer( CObj<NGfx::CGeometry> *pGeom, int nTotal, NGfx::EBufferUsage usage ) 
		: geom( pGeom, nTotal, usage ), nVert(0) 
	{
	}
	void Transform( IPart *p, const vector<CVec3> &transformed )
	{
		nVert += DoTransform( p, &geom[nVert], transformed );
		ASSERT( nVert <= geom.GetSize() );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TTrans>
struct SGfxTnLTransformer : public SPartTransformer<TTrans>
{
	NGfx::CBufferLock<STnLVertex> geom;
	int nVert;

	SGfxTnLTransformer( CObj<NGfx::CGeometry> *pGeom, int nTotal, NGfx::EBufferUsage usage ) 
		: geom( pGeom, nTotal, usage ), nVert(0) 
	{
	}
	void Transform( IPart *p, const vector<CVec3> &transformed )
	{
		nVert += DoTransform( p, &geom[nVert], transformed );
		ASSERT( nVert <= geom.GetSize() );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void TransformPart( IPart *p, vector<CVec3> *pRes, vector<STriangle> *pTris )
{
	p->RefreshObjectInfo();
	CObjectInfo *pObjInfo = p->GetObjectInfo();
	pObjInfo->GetPosTriangles( pTris );
	pRes->resize( pObjInfo->GetPositions().size() );
	SPartTransformer<SGenericPosTransformer> trans;
	trans.DoTransform( p, &((*pRes)[0]), *pRes );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CVBCombiner
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVBCombiner::XFormPosition()
{
	const vector< CPtr<IPart> > &parts = pCombiner->GetValue();
	partBVs.resize( parts.size() );
	xformedPositions.resize( parts.size() );

	SBoundCalcer bcAll;
	for ( int i = 0; i < parts.size(); ++i )
	{
		IPart *pPart = parts[i];
		pPart->RefreshObjectInfo();
		CObjectInfo *pObjInfo = pPart->GetObjectInfo();
		if ( !pObjInfo )
			continue;
		// xform
		vector<CVec3> *pRes = &xformedPositions[ i ];
		int nSize = pObjInfo->GetPositions().size();
		pRes->resize( nSize );
		SPartTransformer<SGenericPosTransformer> trans;
		trans.DoTransform( pPart, &((*pRes)[0]), *pRes );
		// calc bv
		SBoundCalcer bc;
		StartMMXBound( &bc.ptMin, &bc.ptMax );
		for ( int k = 0; k < nSize; ++k )
			AddMMXBoundPoint( &(*pRes)[k] );
		StoreMMXBoundResult( &bc.ptMin, &bc.ptMax );
		bc.Make( &partBVs[i] );
		bcAll.Add( bc );
	}
	bcAll.Make( &bound );
	bNeedXForm = false;
	bNeedRecalc = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CopyVertex( SGfxVertex *pDst, const NGfx::SGeomVecFull &src, const CVec2 &tex )
{
	pDst->pos = src.pos;
	pDst->normal = src.normal;
	pDst->tex = src.tex;
	NGfx::CalcLMCoords( &pDst->texLM, tex.u, tex.v );
	pDst->texU = src.texU;
	pDst->texV = src.texV;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CalcMirroredVector( NGfx::SCompactVector *pRes, 
	const NGfx::SCompactVector &a, const NGfx::SCompactVector &b, const NGfx::SCompactVector &c )
{
	CVec3 v = NGfx::GetVector(a) + NGfx::GetVector(b) - NGfx::GetVector(c);
	Normalize( &v );
	NGfx::CalcCompactVector( pRes, v );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TTrans>
void CVBCombiner::SimpleTransform( TTrans *p )
{
	const vector< CPtr<IPart> > &parts = pCombiner->GetValue();
	TTrans &trans = *p;
	for ( int i = 0; i < parts.size(); ++i )
	{
		IPart *p = parts[i];
		trans.Transform( p, xformedPositions[ i ] );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// possible optimization: amount of animated geometry could be reduced by separating maximal octree node size
// for static & dynamic geometry
void CVBCombiner::Recalc()
{
	if ( bNeedXForm )
		XFormPosition();
	bNeedRecalc = false;
	ASSERT( NGfx::N_VEC_FULL_TEX_SIZE == N_VERTEX_TEX_SIZE );
	const vector< CPtr<IPart> > &parts = pCombiner->GetValue();
	partBVs.resize( parts.size() );
	vector< CPtr<IPart> >::const_iterator i = parts.begin();
	int nVerts = 0, nLMs = 0, nPositions = 0;
	
	for ( ; i != parts.end(); ++i )
	{
		(*i)->RefreshObjectInfo();
		CObjectInfo *pObjInfo = (*i)->GetObjectInfo();
		if ( !pObjInfo )
			continue;
		nPositions += pObjInfo->GetPositions().size();
		nVerts += pObjInfo->GetVertices().size();
		nLMs += pObjInfo->GetLMInfo().size();
	}

	if ( !nVerts )
	{
		pValue = 0;
		bound.BoxInit( CVec3(0,0,0), CVec3(0,0,0) );
		for ( int k = 0; k < partBVs.size(); ++k )
			partBVs[k] = bound.s;
		return;
	}
	NGfx::EBufferUsage bufUsage = ct == CT_STATIC ? NGfx::STATIC : NGfx::DYNAMIC;
	if ( lt == LT_TNL )
	{
		SGfxTnLTransformer<STnLTransformer> trans( &pValue, nVerts, bufUsage );
		SimpleTransform( &trans );
		return;
	}
	if ( lt == LT_TNL_SELECTION )
	{
		SGfxTnLTransformer<STnLSelectionTransformer> trans( &pValue, nVerts, bufUsage );
		trans.fOffset = fOffset;
		SimpleTransform( &trans );
		return;
	}
	if ( lt == LT_NONE )
	{
		SGfxOutputTransformer trans( &pValue, nVerts, bufUsage );
		SimpleTransform( &trans );
		return;
	}
	SBoundCalcer bc;
	if ( lt == LT_POSITION )
	{
		NGfx::CBufferLock<SGfxVertex> geom( &pValue, nPositions, bufUsage );
		int nOffset = 0;
		for ( i = parts.begin(); i != parts.end(); ++i )
		{
			CObjectInfo *pObjInfo = (*i)->GetObjectInfo();
			if ( !pObjInfo )
				continue;
			int nSize = pObjInfo->GetPositions().size();
			const vector<CVec3> &positions = xformedPositions[ i - parts.begin() ];
			ASSERT( nSize == positions.size() );
			SGfxVertex output;
			for ( int k = 0; k < nSize; ++k )
			{
				output.pos = positions[k];
				geom[ nOffset + k ] = output;
			}
			nOffset += nSize;
		}
		return;
	}
	// hard times - lm vertex buffer is required

	float fLMSize1 = 1.0f / N_LM_SIZE;
	NGfx::CBufferLock<SGfxVertex> geom( &pValue, nLMs * 4, bufUsage );
	// Создаем общий вертекс буфер, в котором слиты все кусочки
	int iVert = 0;
	SPartTransformer<SGenericTransformer> trans;
	for ( i = parts.begin(); i != parts.end(); ++i )
	{
		IPart *p = *i;
		CObjectInfo *pObjInfo = p->GetObjectInfo();
		if ( !pObjInfo )
			continue;
		vector<NGfx::SGeomVecFull> transformed;
		transformed.resize( pObjInfo->GetVertices().size() );
		trans.DoTransform( p, &(transformed[0]), xformedPositions[ i - parts.begin() ] );

		const CTRect<int> &lmRegion = p->GetLMRegion();
		const int nLMLOD = p->GetLMLOD();
		int nShiftX = lmRegion.x1;
		int nShiftY = lmRegion.y1;
		const vector<CObjectInfo::SLightmapInfo> &lmaps = pObjInfo->GetLMInfo();
		for ( int k = 0; k < lmaps.size(); ++k )
		{
			const CObjectInfo::SLightmapInfo &lm = lmaps[k];
			const NGfx::SGeomVecFull &v1 = transformed[ lm.n1 ];
			const NGfx::SGeomVecFull &v2 = transformed[ lm.n2 ];
			const NGfx::SGeomVecFull &v3 = transformed[ lm.n3 ];
			NGfx::SGeomVecFull v4;
			if ( lm.nOrder2 == 0 )
			{
				//float fAlpha = ( ( v2.pos - v3.pos ) * ( v1.pos - v3.pos ) ) / fabs2( v2.pos - v3.pos );
				////ASSERT( fAlpha >= 0 && fAlpha <= 1 );
				//fAlpha = Max( Min( fAlpha, 1.0f ), 0.0f );
				//v4.pos = v3.pos * fAlpha + v2.pos * ( 1 - fAlpha );
				//v4.normal = v3.normal * fAlpha + v2.normal * ( 1 - fAlpha );
				//Normalize( &v4.normal );
				//v4.texU = v3.texU * fAlpha + v2.texU * ( 1 - fAlpha );
				//Normalize( &v4.texU );
				//v4.texV = v3.texV * fAlpha + v2.texV * ( 1 - fAlpha );
				//Normalize( &v4.texV );
				//v4.color = v1.color;
				v4.pos = v2.pos + v3.pos - v1.pos;
				CalcMirroredVector( &v4.normal, v2.normal, v3.normal, v1.normal );
				CalcMirroredVector( &v4.texU, v2.texU, v3.texU, v1.texU );
				CalcMirroredVector( &v4.texV, v2.texV, v3.texV, v1.texV );
			}
			else
				v4 = transformed[ lm.n4 ];
			float fU = ( ( lm.nShiftX << nLMLOD ) + nShiftX ) * fLMSize1;
			float fV = ( ( lm.nShiftY << nLMLOD ) + nShiftY ) * fLMSize1;
			float fDU = ( 1 << ( lm.nXSize + nLMLOD ) ) * fLMSize1;
			float fDV = ( 1 << ( lm.nYSize + nLMLOD ) ) * fLMSize1;
			float fDelta = 0.45f * fLMSize1;
			CopyVertex( &geom[iVert+0], v1, CVec2( fU + fDelta, fV + fDelta ) );
			CopyVertex( &geom[iVert+1], v2, CVec2( fU + fDU - fDelta, fV + fDelta ) );
			CopyVertex( &geom[iVert+2], v3, CVec2( fU + fDelta, fV + fDV - fDelta ) );
			CopyVertex( &geom[iVert+3], v4, CVec2( fU + fDU - fDelta, fV + fDV - fDelta ) );
			iVert += 4;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CIBCombiner
////////////////////////////////////////////////////////////////////////////////////////////////////
void CIBCombiner::Recalc()
{
	const vector< CPtr<IPart> > &parts = pCombiner->GetValue();
	//vector< CPtr<IPart> >::const_iterator i;// = parts.begin();
	value.resize( parts.size() );
	int nTotalTris = 0, nTotalLMs = 0;
	for ( int i = 0; i < parts.size(); ++i )//i = parts.begin(); i != parts.end(); ++i )
	{
		parts[i]->RefreshObjectInfo();
		CObjectInfo *pObjInfo = parts[i]->GetObjectInfo();
		if ( !pObjInfo )
			continue;
		nTotalTris += pObjInfo->GetTrisCount();
		nTotalLMs += pObjInfo->GetLMInfo().size();
	}

	int nTriFill = 0;
	if ( lt == LT_NONE || lt == LT_POSITION )
	{
		triBuffer.resize( nTotalTris );
		//
		int nOffset = 0;
		for ( int i = 0; i < parts.size(); ++i )//i = parts.begin(); i != parts.end(); ++i )
		{
			parts[i]->RefreshObjectInfo();
			CObjectInfo *pObjInfo = parts[i]->GetObjectInfo();
			value[i].pTri = &triBuffer[nTriFill];
			if ( !pObjInfo )
			{
				value[i].nTris = 0;
				continue;
			}
			vector<STriangle> tris;
			if ( lt == LT_POSITION )
			{
				if ( ibt == IBTT_POSITIONS )
					pObjInfo->GetPosTriangles( &tris );
				else
					ASSERT( 0 );
			}
			else
			{
				if ( ibt == IBTT_POSITIONS )
					pObjInfo->GetVxPositionTriangles( &tris );
				else if ( ibt == IBTT_VERTICES )
					pObjInfo->GetVxVerticesTriangles( &tris );
				else
					ASSERT( 0 );
			}
			value[i].nTris = tris.size();
			value[i].nBaseIndex = nOffset;
			for ( int k = 0; k < tris.size(); ++k )
				triBuffer[nTriFill++] = tris[k];
			if ( lt == LT_POSITION )
				nOffset += pObjInfo->GetPositions().size();
			else
				nOffset += pObjInfo->GetVertices().size();
		}
		return;
	}
	//
	triBuffer.resize( Max( 1, nTotalLMs ) * 2 );
	int nTri = 0, nOffset = 0;
	for ( int i = 0; i < parts.size(); ++i )
	{
		parts[i]->RefreshObjectInfo();
		CObjectInfo *pObjInfo = parts[i]->GetObjectInfo();
		value[i].pTri = &triBuffer[nTriFill];
		if ( !pObjInfo )
		{
			value[i].nTris = 0;
			continue;
		}
		int nLMs = pObjInfo->GetLMInfo().size();
		if ( ibt == IBTT_LMCALC )
		{
			value[i].nTris = nLMs * 2;
			value[i].nBaseIndex = nOffset;
			for ( int k = 0; k < nLMs; ++k )
			{
				int nStart = k * 4;
				STriangle &tri1 = triBuffer[nTriFill++];
				tri1.i1 = nStart + 0;
				tri1.i2 = nStart + 1;
				tri1.i3 = nStart + 2;
				STriangle &tri2 = triBuffer[nTriFill++];
				tri2.i1 = nStart + 3;
				tri2.i2 = nStart + 2;
				tri2.i3 = nStart + 1;
			}
		}
		else
		{
			vector<STriangle> tris;
			switch ( ibt )
			{
				case IBTT_POSITIONS: pObjInfo->GetLMPositionTriangles( &tris ); break;
				case IBTT_VERTICES: pObjInfo->GetLMVerticesTriangles( &tris ); break;
				case IBTT_LM: pObjInfo->GetLMTriangles( &tris ); break;
				default: ASSERT(0); break;
			}
			value[i].nTris = tris.size();
			value[i].nBaseIndex = nOffset;
			for ( int k = 0; k < tris.size(); ++k )
			{
				STriangle &tri = triBuffer[nTriFill++];
				tri.i1 = tris[k].i1;
				tri.i2 = tris[k].i2;
				tri.i3 = tris[k].i3;
			}
		}
		nOffset += nLMs * 4;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x02741133, CPerMaterialCombiner )
REGISTER_SAVELOAD_CLASS( 0x02741134, CVBCombiner )
REGISTER_SAVELOAD_CLASS( 0x02741135, CIBCombiner )
REGISTER_SAVELOAD_CLASS( 0x01091206, CAutomaticCombiner )
