#ifndef __PARTICLES_H__
#define __PARTICLES_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Data.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SKey
{
	short nT;
	float fValue;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 2 )
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TValue>
class TKey
{
public:
	short nT;
	TValue value;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TValue>
class TKeyTrack
{
public:
	ZDATA
	vector< TKey<TValue> > keys;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&keys); return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class TValue>
class TKeyTrackFile
{
public:
	short nKeys;
	int keys; // shift in bytes
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SParticle
{
	short nTStart;
	short nTEnd;
	TKeyTrack< TVector<3> > pos;
	TKeyTrack< TVector<1> > rot;
	TKeyTrack< TVector<2> > scale;
	TKeyTrack< DWORD > color;
	TKeyTrack< short > sprite;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SParticleFile
{
	short nTStart;
	short nTEnd;
	TKeyTrackFile< TVector<3> > pos;
	TKeyTrackFile< TVector<1> > rot;
	TKeyTrackFile< TVector<2> > scale;
	TKeyTrackFile< DWORD > color;
	TKeyTrackFile< short > sprite;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack( pop )
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SParticleObject
{
	float fTEnd;
	float fFrameRate;
	vector< SParticle > particles;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SParticleObjectFile
{
	float fTEnd;
	float fFrameRate;
	int nParticles;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void FindNextKey( const vector<float> &values, const SKey &prevKey, float fEpsilon, SKey *pNextKey );
////////////////////////////////////////////////////////////////////////////////////////////////////
void DropSame( const vector<short> &values, int nTBegin, TKeyTrack<short> *pRes );
////////////////////////////////////////////////////////////////////////////////////////////////////
template <int nCh>
void LinearSpline( const vector< TVector<nCh> > &values, int nTBegin, float fEpsilon, TKeyTrack< TVector<nCh> > *pRes )
{
	if ( values.size() == 0 )
		return;

	TKey< TVector<nCh> > resKey;
	resKey.nT = nTBegin;
	resKey.value = values.front();
	pRes->keys.push_back( resKey );
	if ( values.size() == 1 )
		return;

	vector<float> chan[nCh];
	for ( int n = 0; n < nCh; ++n )
	{
		chan[n].resize( values.size() );
		for ( int i = 0; i < values.size(); ++i )
			chan[n][i] = values[i].val[n];
	}

	TVector<nCh> precision;
	for ( n = 0; n < nCh; ++n )
	{
		float fValMin, fValMax;
		fValMin = fValMax = values.front().val[n];
		for ( int i = 1; i < values.size(); ++i )
		{
			fValMin = (chan[n][i] < fValMin) ? chan[n][i] : fValMin;
			fValMax = (chan[n][i] > fValMax) ? chan[n][i] : fValMax;
		}
		precision.val[n] = fEpsilon * (fValMax - fValMin);
	}

	SKey prev[nCh], next[nCh];
	for ( n = 0; n < nCh; ++n )
	{
		prev[n].nT = 0;
		prev[n].fValue = chan[n].front();
	}

	bool bLeft = true;
	while ( bLeft )
	{
		for ( n = 0; n < nCh; ++n )
			FindNextKey( chan[n], prev[n], precision.val[n], &next[n] );

		int nMinT = next[0].nT;
		for ( n = 1; n < nCh; ++n )
		{
			if ( next[n].nT < nMinT )
				nMinT = next[n].nT;
		}
		for ( n = 0; n < nCh; ++n )
		{
			next[n].fValue = prev[n].fValue +
				(next[n].fValue - prev[n].fValue) * (nMinT - prev[n].nT) / (next[n].nT - prev[n].nT);
			next[n].nT = nMinT;
		}

		resKey.nT = nTBegin + nMinT;
		for ( n = 0; n < nCh; ++n )
		{
			resKey.value.val[n] = next[n].fValue;
			prev[n] = next[n];
		}
		pRes->keys.push_back( resKey );

		if ( nMinT >= values.size() - 1 )
			bLeft = false;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline int FindCloseValue( float fValue, int nMin, int nMax )
{
	int v = (int)fValue;
	if ( fValue - (float)v >= 0.5f )
		++v;
	if ( v < nMin )
		v = nMin;
	if ( v > nMax )
		v = nMax;
	return v;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int ConvertToSingleFile( const SParticleObject &object, char **pData, int nTotalParticles, int nKeyBytes );
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
