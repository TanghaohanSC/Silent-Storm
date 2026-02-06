#include "StdAfx.h"
#include "Particles.h"
#include <math.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
void FindNextKey( const vector<float> &values, const SKey &prevKey, float fEpsilon, SKey *pNextKey )
{
	int nTotal = values.size();
	//fprintf( stderr, "NValues %d\n", nTotal );
	int nFrom = prevKey.nT + 1;
	if ( nFrom == nTotal - 1 )
	{
		pNextKey->nT = nTotal - 1;
		pNextKey->fValue = values.back();
		return;
	}
	float fMin = -1e10f, fMax = 1e10f;
	for ( int n = nFrom; n < nTotal; ++n )
	{
		int nX = n - prevKey.nT;
		float fMinNext = (values[n] - fEpsilon - prevKey.fValue) / nX;
		float fMaxNext = (values[n] + fEpsilon - prevKey.fValue) / nX;
		if ( fMaxNext < fMin )
		{
			pNextKey->nT = n - 1;
			pNextKey->fValue = prevKey.fValue + fMin * (pNextKey->nT - prevKey.nT);
			return;
		}
		if ( fMinNext > fMax )
		{
			pNextKey->nT = n - 1;
			pNextKey->fValue = prevKey.fValue + fMax * (pNextKey->nT - prevKey.nT);
			return;
		}
		if ( fMaxNext < fMax )
			fMax = fMaxNext;
		if ( fMinNext > fMin )
			fMin = fMinNext;
	}
	float fLastY = values.back();
	pNextKey->nT = nTotal - 1;
	float fMinY = prevKey.fValue + fMin * (pNextKey->nT - prevKey.nT);
	float fMaxY = prevKey.fValue + fMax * (pNextKey->nT - prevKey.nT);
	if ( fLastY < fMinY )
		pNextKey->fValue = fMinY;
	else if ( fLastY > fMaxY )
		pNextKey->fValue = fMaxY;
	else
		pNextKey->fValue = fLastY;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DropSame( const vector<short> &values, int nTBegin, TKeyTrack<short> *pRes )
{
	if ( values.size() == 0 )
		return;

	TKey<short> resKey;
	resKey.nT = nTBegin;
	resKey.value = values.front();
	pRes->keys.push_back( resKey );
	if ( values.size() == 1 )
		return;

	short nCurrent = values.front();
	for ( int i = 1; i < values.size() - 1; ++i )
	{
		if ( values[i] != nCurrent )
		{
			resKey.nT = nTBegin + i;
			nCurrent = resKey.value = values[i];
			pRes->keys.push_back( resKey );
		}
	}

	resKey.nT = nTBegin + values.size() - 1;
	resKey.value = values.back();
	pRes->keys.push_back( resKey );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int ConvertToSingleFile( const SParticleObject &object, char **pData, int nTotalParticles, int nKeyBytes )
{
	int nOutBytes = sizeof(SParticleObjectFile) + nTotalParticles * sizeof(SParticleFile) + nKeyBytes;
	char *p = *pData = new char[nOutBytes];
	
	SParticleObjectFile *pObject = (SParticleObjectFile*)p;
	SParticleFile *pParticles = (SParticleFile*)(pObject + 1);
	char *pKeys = (char*)(pParticles + nTotalParticles);

	pObject->fTEnd = object.fTEnd;
	pObject->fFrameRate = object.fFrameRate;
	pObject->nParticles = object.particles.size();
	for ( int nP = 0; nP < pObject->nParticles; ++nP )
	{
		const SParticle &particle = object.particles[nP];
		pParticles->nTStart = particle.nTStart;
		pParticles->nTEnd = particle.nTEnd;

		TKey< TVector<3> >* pPos = (TKey< TVector<3> >*)pKeys;
		pParticles->pos.nKeys = particle.pos.keys.size();
		pParticles->pos.keys = int( pKeys - p );
		for ( int nK = 0; nK < pParticles->pos.nKeys; ++nK )
			*(pPos++) = particle.pos.keys[nK];
		pKeys = (char*)pPos;

		TKey< TVector<1> >* pRot = (TKey< TVector<1> >*)pKeys;
		pParticles->rot.nKeys = particle.rot.keys.size();
		pParticles->rot.keys = int( pKeys - p );
		for ( nK = 0; nK < pParticles->rot.nKeys; ++nK )
			*(pRot++) = particle.rot.keys[nK];
		pKeys = (char*)pRot;

		TKey< TVector<2> >* pScale = (TKey< TVector<2> >*)pKeys;
		pParticles->scale.nKeys = particle.scale.keys.size();
		pParticles->scale.keys = int( pKeys - p );
		for ( nK = 0; nK < pParticles->scale.nKeys; ++nK )
			*(pScale++) = particle.scale.keys[nK];
		pKeys = (char*)pScale;

		TKey<DWORD>* pColor = (TKey<DWORD>*)pKeys;
		pParticles->color.nKeys = particle.color.keys.size();
		pParticles->color.keys = int( pKeys - p );
		for ( nK = 0; nK < pParticles->color.nKeys; ++nK )
			*(pColor++) = particle.color.keys[nK];
		pKeys = (char*)pColor;

		TKey<short>* pSprite = (TKey<short>*)pKeys;
		pParticles->sprite.nKeys = particle.sprite.keys.size();
		pParticles->sprite.keys = int( pKeys - p );
		for ( nK = 0; nK < pParticles->sprite.nKeys; ++nK )
			*(pSprite++) = particle.sprite.keys[nK];
		pKeys = (char*)pSprite;

		++pParticles;
	}
	//fprintf( stderr, "Before: %d, After: %d\n", nOutBytes, pKeys - p );
	return nOutBytes;
}

