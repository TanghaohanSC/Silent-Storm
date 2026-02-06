#include "StdAfx.h"
#include "Sound.h"
#include "SoundFormat.h"
#include "Camera.h"
#include "..\DBFormat\DataSound.h"
#include "..\FModSound\FMSound.h"
#include "..\Misc\BasicShare.h"
#include "..\Misc\HPTimer.h"
#include "..\Misc\Commands.h"
#include "..\Misc\LogStream.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NSound
{
////////////////////////////////////////////////////////////////////////////////////////////////////
static HWND hWnd;
static CBasicShare<int, CFileSample2D> share2DSamples(127);
static CBasicShare<int, CFileSample3D> share3DSamples(128);
const int N_MAX_SILENCE = 120;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSound: public CObjectBase
{
	OBJECT_BASIC_METHODS(CSound);
public:
	ZDATA
	CObj<NFMSound::CSound3D> pSound;
	CDGPtr< CFuncBase<CVec3> > pPos;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pSound); f.Add(3,&pPos); return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSound2D: public ISound2D
{
	OBJECT_BASIC_METHODS(CSound2D);
public:
	ZDATA
	CObj<NFMSound::CSound2D> pSound;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pSound); return 0; }
	virtual bool IsPlaying()
	{
		return NFMSound::IsPlaying( pSound );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMusic: public CObjectBase
{
	OBJECT_BASIC_METHODS(CMusic);
public:
	ZDATA
	CPtr<NFMSound::CStream> pStream;
	CDBPtr<NDb::CMusic> pMusic;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pStream); f.Add(3,&pMusic); return 0; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSoundScene: public ISoundScene
{
	OBJECT_BASIC_METHODS(CSoundScene);
private:
	ZDATA
	bool bSilence;
	CObj<CMusic> pMusic;
	NHPTimer::STime tStartSilence;
	list< CPtr<CSound> > sounds;
	list< CPtr<CSound2D> > sounds2D;
	CDBPtr<NDb::CMusic> pAmbient;
public:
	ZEND

public:
	CSoundScene( NDb::CMusic *_pAmbient = 0 );

	virtual CSound* Add3DSound( NDb::CSound *pSample, CFuncBase<CVec3> *pPos, STime tStart );
	virtual CSound2D* Add2DSound( NDb::CSound *pSample );

	virtual void SetMusic( NDb::CMusic *pMusic );
	virtual void FadeOutMusic();

	virtual void Draw( const ICamera *pCamera );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CSoundScene::CSoundScene( NDb::CMusic *_pAmbient ): 
	bSilence(true), pAmbient(_pAmbient)
{
	NHPTimer::GetTime(&tStartSilence);
	tStartSilence -= N_MAX_SILENCE * NHPTimer::GetClockRate();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CSound* CSoundScene::Add3DSound( NDb::CSound *pSample, CFuncBase<CVec3> *pPos, STime tStart )
{
	if ( !pSample || !pPos )
	{
		ASSERT( pSample && pPos );
		return 0;
	}
	CDGPtr< CPtrFuncBase<NFMSound::CSample3D> > pSam = share3DSamples.Get( pSample->GetRecordID() );
	pSam.Refresh();
	NFMSound::CSample3D *pData = pSam->GetValue();
	if ( !pData )
		return 0;
	CSound *pSound = new CSound;
	pSound->pPos = pPos;
	pSound->pPos.Refresh();
	pSound->pSound = NFMSound::Play3DSound( pData, pSound->pPos->GetValue() );
	sounds.push_back( pSound );
	
	return pSound;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CSound2D* CSoundScene::Add2DSound( NDb::CSound *pSample )
{
	if ( !pSample )
	{
		ASSERT( pSample );
		return 0;
	}
	CDGPtr< CPtrFuncBase<NFMSound::CSample2D> > pSam = share2DSamples.Get( pSample->GetRecordID() );
	pSam.Refresh();
	CSound2D *p = new CSound2D;
	p->pSound = NFMSound::PlaySound( pSam->GetValue() );
	sounds2D.push_back( p );
	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSoundScene::SetMusic( NDb::CMusic *pDBMusic )
{
	if ( !IsValid( pDBMusic ) )
		return;
	if ( IsValid( pMusic ) && IsValid( pMusic->pMusic ) && IsValid( pMusic->pStream ) 
		//&& (pMusic->pMusic->GetRecordID() == pDBMusic->GetRecordID() || pMusic->pMusic->eType == NDb::MT_COMBAT ) )
		&& (pMusic->pMusic->eType == pDBMusic->eType || pMusic->pMusic->eType == NDb::MT_COMBAT ) )
	{
		NFMSound::CancelFadeOut( pMusic->pStream );
		return;
	}
	CMusic *pM = new CMusic;
	pM->pMusic  = pDBMusic;
	bool bLoop = !(pDBMusic->eType == NDb::MT_AMBIENT);
	pM->pStream = NFMSound::SwitchStream( IsValid( pMusic ) ? pMusic->pStream : 0, pDBMusic->szFileName.c_str(), bLoop );
	pMusic = pM;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSoundScene::FadeOutMusic()
{
	if ( IsValid( pMusic ) && pMusic->pMusic->eType != NDb::MT_AMBIENT )
		NFMSound::FadeOut( pMusic->pStream, 6 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSoundScene::Draw( const ICamera *pCamera )
{
	for ( list< CPtr<CSound> >::iterator it = sounds.begin(); it != sounds.end(); )
	{
		CSound *pSound = *it;
		if ( !pSound || !IsValid( pSound->pSound ) )
		{
			it = sounds.erase( it );
			continue;
		}
		pSound->pPos.Refresh();
		CVec3 ptPos = pSound->pPos->GetValue();
		CDynamicCast<NFMSound::ISound3D> p3D( pSound->pSound );
		p3D->SetPosition( ptPos );
		//
		++it;
	}
	for ( list< CPtr<CSound2D> >::iterator it = sounds2D.begin(); it != sounds2D.end(); )
	{
		CSound2D *pSound = *it;
		if ( !pSound || !IsValid( pSound->pSound ) )
		{
			it = sounds2D.erase( it );
			continue;
		}
		++it;
	}
	//
	NFMSound::SListener listener;
	ICamera::SCameraPos camPos;
	pCamera->GetPlacement( &camPos );
	CVec3 fwd( pCamera->GetForwardDir() );
	fwd.z = 0;
	Normalize( &fwd );
	listener.forward = fwd;
	listener.position = /*pCamera->GetCP();//*/camPos.ptAnchor - listener.forward * 5; //pCamera->GetCP();
	listener.top = CVec3(0,0,1);//pCamera->GetStrafeDir() ^ listener.forward;
	
	NFMSound::Update( listener );
	//
	if ( !IsValid( pMusic ) || !NFMSound::IsPlaying( pMusic->pStream ) )
	{
		if ( !bSilence )
		{
			bSilence = true;
			NHPTimer::GetTime( &tStartSilence );
		}
		else
		{
			NHPTimer::STime t = tStartSilence;
			double dt = NHPTimer::GetTimePassed( &t );
			if ( IsValid( pAmbient ) && dt > N_MAX_SILENCE )
			{
				CMusic *pM = new CMusic;
				pM->pMusic = pAmbient;
				bool bLoop = !(pAmbient->eType == NDb::MT_AMBIENT);
				pM->pStream = NFMSound::PlayStream( pAmbient->szFileName.c_str(), bLoop );
				pMusic = pM;
				bSilence = false;
			}
		}
	}
	else
		bSilence = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool InitSound( HWND _hWnd )
{
	hWnd = _hWnd;
	return NFMSound::SearchDevices();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SetMode( bool bInitSound )
{
	if ( !bInitSound )
		return true;

	NFMSound::SStartInfo info;
	info.hWnd = hWnd;
	return NFMSound::Init( info );	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SetModeFromConfig()
{
	NGlobal::CValue sValue;

	bool bInitSound = false;
	sValue = NGlobal::GetVar( "sound_mode", 1.0f );
	if ( sValue.GetFloat() != 0 )
		bInitSound = true;

	if ( !SetMode( bInitSound ) )
		return false;

	NFMSound::SetSFXMasterVolume( NGlobal::GetVar( "sound_sfxvolume" ).GetFloat() * 0xFF );
	NFMSound::SetMusicMasterVolume( NGlobal::GetVar( "sound_musicvolume" ).GetFloat() * 0xFF );

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ISoundScene* CreateSoundScene( NDb::CMusic *pAmbient )
{
	return new CSoundScene( pAmbient );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DoneSound()
{
	NFMSound::Done();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands/Vars
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CommandSoundUpdate( const string &szID, const vector<wstring> &paramsSet, void *pContext )
{
	DoneSound();
	SetModeFromConfig();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSetSfxVolume( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	NFMSound::SetSFXMasterVolume( sValue.GetFloat() * 0xFF );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSetMusicVolume( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	NFMSound::SetMusicMasterVolume( sValue.GetFloat() * 0xFF );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void VarSetOutputType( const string &szID, const NGlobal::CValue &sValue, void *pContext )
{
	NFMSound::ESpeakerType eType = NFMSound::SOUND_SM_MONO;

	if ( sValue.GetString() == L"mono" )
		eType = NFMSound::SOUND_SM_MONO;
	else if ( sValue.GetString() == L"stereo" )
		eType = NFMSound::SOUND_SM_STEREO;
	else if ( sValue.GetString() == L"headphone" )
		eType = NFMSound::SOUND_SM_HEADPHONE;
	else if ( sValue.GetString() == L"surround" )
		eType = NFMSound::SOUND_SM_SURROUND;
	else if ( sValue.GetString() == L"quad" )
		eType = NFMSound::SOUND_SM_QUAD;
	else if ( sValue.GetString() == L"5dot1" )
		eType = NFMSound::SOUND_SM_5DOT1;

	NFMSound::SetSpeakerType( eType );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(Sound)
	REGISTER_CMD( "sound_update", CommandSoundUpdate )
	////
	REGISTER_VAR( "sound_mode", 0, 1.0f, true )
	REGISTER_VAR( "sound_sfxvolume", VarSetSfxVolume, 1.0f, true )
	REGISTER_VAR( "sound_musicvolume", VarSetMusicVolume, 1.0f, true )
	REGISTER_VAR( "sound_outputmode", VarSetOutputType, NGlobal::CValue( L"mono" ), true )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NSound;
BASIC_REGISTER_CLASS( ISoundScene );
REGISTER_SAVELOAD_CLASS( 0x03081147, CSound );
REGISTER_SAVELOAD_CLASS( 0x02881171, CSoundScene );
