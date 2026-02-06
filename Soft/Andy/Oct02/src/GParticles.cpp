#include "StdAfx.h"
#include "GParticles.h"
#include "GParticleFormat.h"
#include "GGrass.h"
#include "GParticleInfo.h"
#include "..\DBFormat\DataTerrain.h"
#include "..\DBFormat\DataFormat.h"
#include "Grid.h"
#include "Interpolate.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CParticleAnimator
////////////////////////////////////////////////////////////////////////////////////////////////////
void CParticleAnimator::Recalc()
{
	if ( !IsValid( pValue ) )
		pValue = new CStandardParticleEffect;

	CDynamicCast<CStandardParticleEffect> pRealValue(pValue);
	CStandardParticleEffect &value = *pRealValue;

	value.bEnd = true;
	value.nGrassSize = 0;
	value.textures = textureIDs;

	value.pInfo = pInfo;
	value.fScale = pInstance->fScale;
	value.fEndCycle = pInstance->fEndCycle;
	value.pivot = pInstance->pivot;
//	value.bAlphaAdd = (pInstance->alpha == NDb::CParticleInstance::A_ADDITIVE);
	value.transform = pPlacement->GetValue().forward;
	value.frames.clear();
	
	STime time = pTime->GetValue();
	if ( time < stBeginTime )
	{
		value.bEnd = false;
		return;
	}
	float fTObject = (time - stBeginTime) * pInstance->fSpeed / 1000.f - pInstance->fOffset * pInstance->fSpeed;
	CParticlesInfo *pEffect = pInfo->GetValue();
	if ( fTObject < 0 )
	{
		value.bEnd = false;
		return;
	}
	vector<float> fTimes;
	if ( pInstance->fEndCycle == 0 )
	{
		if ( pInstance->nCycleCount )
		{
			if ( fTObject < pEffect->fTEnd )
			{
				fTimes.push_back( fTObject );
				value.bEnd = false;
			}
		}
		else
		{
			fTimes.push_back(0);
			value.bEnd = false;
		}
	}
	else
	{
		int nCurCycle = int( fTObject / pInstance->fEndCycle ) + 1;
		if ( pInstance->nCycleCount && nCurCycle > pInstance->nCycleCount )
			nCurCycle = pInstance->nCycleCount;
		float fT = fTObject - (nCurCycle - 1) * pInstance->fEndCycle;
		for ( int i = 0; i < nCurCycle && fT < pEffect->fTEnd; ++i )
		{
			fTimes.push_back( fT );
			fT += pInstance->fEndCycle;
		}
		if ( pInstance->nCycleCount == 0
			|| fTObject < pEffect->fTEnd + (pInstance->nCycleCount - 1) * pInstance->fEndCycle )
			value.bEnd = false;
	}

	for ( int i = 0; i < fTimes.size(); ++i )
	{
		SParticleFrame frame;
		frame.fT = fTimes[i] * pEffect->fFrameRate;
		frame.bLastCycle = (i == 0);
		value.frames.push_back( frame );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGrassAnimator
////////////////////////////////////////////////////////////////////////////////////////////////////
void CGrassAnimator::Recalc()
{
	if ( !IsValid( pValue ) )
	{
		CGrassParticleEffect *pRealValue = new CGrassParticleEffect;
		const SGrassLayer &gl = pGrassTracker->GetGrassLayer( nLayer );
		pValue = pRealValue;
		pRealValue->positions = pGrassPos->GetValue()->positions;
		pRealValue->colors.resize( pRealValue->positions.size() );
		pRealValue->waveAmps.resize( pRealValue->positions.size() );
		pRealValue->scale = pDBGrass->ptScale;
		pRealValue->fXPivot = pDBGrass->ptPivot.x;
		pRealValue->fYPivot = pDBGrass->ptPivot.y;
		pRealValue->fScaleRange = pDBGrass->fScaleRange;
		for ( int i = 0; i < pRealValue->positions.size(); ++i )
		{
			const CVec3 &pos = pRealValue->positions[i];
			float fColorX = pos.x * FP_INV_GRID_STEP * FP_GRASS_COLOR_SCALE;
			float fColorY = pos.y * FP_INV_GRID_STEP * FP_GRASS_COLOR_SCALE;
			pRealValue->colors[i].color = GetBilinear( gl.grassColor, fColorX, fColorY, NGfx::CInterpolateColor() );
		}
		pRealValue->bEnd = false;
		pRealValue->nGrassSize = pDBGrass->nSideSize;
		pRealValue->textures.resize( 1 );
		pRealValue->textures[0] = pGrassTexture;
	}
	CDynamicCast<CGrassParticleEffect> pRealValue(pValue);
	STime time = pTime->GetValue();
	pRealValue->fTEffect = time / 1000.f;
	for ( int i = 0; i < pRealValue->positions.size(); ++i )
	{
		const CVec3 &pos = pRealValue->positions[i];
		pRealValue->waveAmps[i] = pGrassTracker->GetWaveAmp( pos );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CExplosionAnimator
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExplosionAnimator::Recalc()
{
	if ( !IsValid( pValue ) )
		pValue = new CExplosionParticleEffect;

	CDynamicCast<CExplosionParticleEffect> pRealValue(pValue);
	CExplosionParticleEffect &value = *pRealValue;

	value.bEnd = false;
	value.nGrassSize = 0;
	value.textures = textureIDs;

	value.pInfo = pInfo;
	value.pivot = pInstance->pivot;
//	value.bAlphaAdd = (pInstance->alpha == NDb::CParticleInstance::A_ADDITIVE);

	CParticlesInfo *pEffect = pInfo->GetValue();
	const NGScene::SParticle &part = pEffect->particles[0];
/*
	const CExplosionInfo &exp = pExplosion->GetValue();
	STime tLastBornBefore = tLastBorn;
	for ( int i = 0; i < exp.particles.size(); ++i )
	{
		const SParticleBorn &p = exp.particles[i];
		if ( p.tBorn <= tLastBornBefore )
			continue;
		particles.push_back( p );
		tLastBorn = p.tBorn;
	}

	STime time = pTime->GetValue();
	value.positions.clear();
	value.fTimes.clear();
	for ( list<SParticleBorn>::iterator i = particles.begin(); i != particles.end(); )
	{
		if ( time < i->tBorn )
		{
			++i;
			continue;
		}
		float fTime = (time - i->tBorn) * fSpeed * 0.5f * pEffect->fFrameRate / 1000.f;
		if ( fTime < part.nTStart )
		{
			++i;
			continue;
		}
		if ( fTime > part.nTEnd )
			i = particles.erase(i);
		else
		{
			value.positions.push_back( i->pos );
			value.fTimes.push_back( fTime );
			++i;
		}
	}
*/
	
	const CExplosionInfo &exp = pExplosion->GetValue();
	particles.clear();
	for ( int i = 0; i < exp.particles.size(); ++i )
	{
		const SParticleBorn &p = exp.particles[i];
		particles.push_back( p );
	}

	STime time = pTime->GetValue();
	value.positions.clear();
	value.fTimes.clear();
	for ( list<SParticleBorn>::iterator i = particles.begin(); i != particles.end(); )
	{
		if ( time < i->tBorn )
		{
			++i;
			continue;
		}
		float fTime = (time - i->tBorn) * pInstance->fSpeed * pEffect->fFrameRate / 1000.f;
		if ( fTime < part.nTStart )
		{
			++i;
			continue;
		}
		if ( fTime > part.nTEnd )
			i = particles.erase(i);
		else
		{
			value.positions.push_back( i->pos );
			value.fTimes.push_back( fTime );
			++i;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
using namespace NGScene;
REGISTER_SAVELOAD_CLASS( 0x27041142, CParticleAnimator )
REGISTER_SAVELOAD_CLASS( 0x125A1140, CGrassAnimator )
REGISTER_SAVELOAD_CLASS( 0x11932170, CExplosionAnimator )
