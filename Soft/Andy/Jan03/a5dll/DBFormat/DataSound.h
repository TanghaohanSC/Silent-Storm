#ifndef __DATASOUND_H_
#define __DATASOUND_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\ADOImport\BasicDB.h"
#include "DataConst.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NDb
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ESoundType
{
	ST_PERMANENT,
	ST_RANDOM,
	ST_REALTIME,
	ST_WIND
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSound: public CDBRecord
{
	OBJECT_BASIC_METHODS( CSound );
public:
	ZDATA_(CDBRecord)
	bool bLoop;
	float fMinDistance;
	float fMaxDistance;
	int nPriority;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CDBRecord*)this); f.Add(2,&bLoop); f.Add(3,&fMinDistance); f.Add(4,&fMaxDistance); f.Add(5,&nPriority); return 0; }
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTSound;
class CSoundVariant: public CDBRecord
{
	OBJECT_BASIC_METHODS( CSoundVariant );
public:
	ZDATA_(CDBRecord)
	CPtr<CSound>  pSound;
	CPtr<CTSound> pTemplate;
	vector<SVariantFlags> flags;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CDBRecord*)this); f.Add(2,&pSound); f.Add(3,&pTemplate); f.Add(5,&flags); return 0; }
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTSound: public CRndPtr<CSoundVariant>
{
	OBJECT_BASIC_METHODS(CTSound);
public:
	CSoundVariant* GetSound( SRand *pRand ) const;
	CSoundVariant* GetSound( SRand *pRand, const vector<int> &params ) const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EMusicType
{
	MT_AMBIENT,
	MT_PRECOMBAT,
	MT_COMBAT
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMusic: public CDBRecord
{
  OBJECT_BASIC_METHODS(CMusic);
public:
	ZDATA_(CDBRecord)
	string szFileName;
	EMusicType eType;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CDBRecord*)this); f.Add(2,&szFileName); f.Add(3,&eType); return 0; }

  virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSoundEffect;
class CSoundInstance: public CDBRecord
{
	OBJECT_BASIC_METHODS(CSoundInstance);
public:
	ZDATA_(CDBRecord)
	CPtr<CTSound> pSound;
	int nStartTime;
	int nCycleCount;
	bool bFadeIn;
	bool bFadeOut;
	int  nFadeSamples;
	ESoundType eSoundType;
	float fSoundAvgInterval;
	CPtr<CSoundEffect> pEffect;
	int nVolume;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CDBRecord*)this); f.Add(2,&pSound); f.Add(3,&nStartTime); f.Add(4,&nCycleCount); f.Add(5,&bFadeIn); f.Add(6,&bFadeOut); f.Add(7,&nFadeSamples); f.Add(8,&eSoundType); f.Add(9,&fSoundAvgInterval); f.Add(10,&pEffect); f.Add(11,&nVolume); return 0; }

	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSoundEffect: public CDBRecord
{
	OBJECT_BASIC_METHODS(CSoundEffect);
public:
	ZDATA_(CDBRecord)
	vector<CPtr<CSoundInstance> > instances;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CDBRecord*)this); f.Add(2,&instances); return 0; }

	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CTSound* GetTSound( int nTSoundID );
CSound* GetSound( int nID );
CMusic* GetMusic( int nID );
CSoundEffect* GetSoundEffect( int nID );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __DATASOUND_H_