#include "StdAfx.h"
#include "iInput.h"
#include "WinFrame.h"
/////////////////////////////////////////////////////////////////////////////////////
using namespace NInput;
/////////////////////////////////////////////////////////////////////////////////////
bool NInput::IsExit()
{
	return NWinFrame::IsExit();
}
/////////////////////////////////////////////////////////////////////////////////////
bool NInput::IsActive()
{
	return NWinFrame::IsAppActive();
}
/////////////////////////////////////////////////////////////////////////////////////
void NInput::PumpMessages()
{
	NWinFrame::PumpMessages();
}
/////////////////////////////////////////////////////////////////////////////////////
// Message processing
/////////////////////////////////////////////////////////////////////////////////////
static NInput::STime GetInputTime( const NHPTimer::STime &time )
{
	return NHPTimer::GetSeconds( time ) * 1000;
}
/////////////////////////////////////////////////////////////////////////////////////
struct SSliderInfo
{
	int64 nData;
	int nIsOn;
	string szName;
	NHPTimer::STime timeLastOn;
	//
	SSliderInfo() { nData = 0; nIsOn = 0; }
	void On( const NHPTimer::STime &time );
	void Off( const NHPTimer::STime &time ); 
	void Update( const NHPTimer::STime &time );
	void Skip( const NHPTimer::STime &time );
};
static vector< SSliderInfo > sliders;
/////////////////////////////////////////////////////////////////////////////////////
struct SEventInfo
{
	int nEventID;
	string szName;
	//
	SEventInfo() { nEventID = -1; }
};
static vector< SEventInfo > events;
/////////////////////////////////////////////////////////////////////////////////////
struct SBind
{
	int nKey;
	int nIndex;
	//
	SBind() {}
	SBind( int _nKey, int _nIndex ): nKey(_nKey), nIndex(_nIndex) {}
};
static vector<SBind> eventBinds;
static vector<SBind> sliderBinds;
/////////////////////////////////////////////////////////////////////////////////////
// SSliderInfo
/////////////////////////////////////////////////////////////////////////////////////
void SSliderInfo::On( const NHPTimer::STime &time ) 
{
	if ( nIsOn == 0 ) 
		timeLastOn = time;
	++nIsOn;
}
/////////////////////////////////////////////////////////////////////////////////////
void SSliderInfo::Off( const NHPTimer::STime &time ) 
{
	if ( nIsOn != 0 ) 
		nData += NHPTimer::GetSeconds( time - timeLastOn ) * 0x10000;
	nIsOn = 0;
}
/////////////////////////////////////////////////////////////////////////////////////
void SSliderInfo::Update( const NHPTimer::STime &time ) 
{
	if ( nIsOn != 0 ) 
	{
		nData += NHPTimer::GetSeconds( time - timeLastOn ) * 0x10000;
		timeLastOn = time; 
	} 
}
/////////////////////////////////////////////////////////////////////////////////////
void SSliderInfo::Skip( const NHPTimer::STime &time )
{
	timeLastOn = time; 
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
static bool GetIndex( const vector<SBind> &data, int nKey, int *pIndex )
{
	for ( int i = 0; i < data.size(); i++ )
	{
		if ( data[i].nKey == nKey )
		{
			*pIndex = data[i].nIndex;
			return true;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline int GetIndex( vector<T> *data, const char *pszName )
{
	for ( int i = 0; i < data->size(); i++ )
	{
		if ( (*data)[i].szName == pszName )
			return i;
	}
	data->push_back();
	T &info = data->back();
	info.szName = pszName;
	return data->size() - 1;
}
/////////////////////////////////////////////////////////////////////////////////////
bool NInput::GetMessage( SMessage *pMsg )
{
	NWinFrame::SWindowsMsg msg;
	for(;;)
	{
		if ( !NWinFrame::GetMessage( &msg ) )
		{
			// no more messages in queue
			if ( NWinFrame::IsAppActive() )
			{
				for ( int i = 0; i < sliders.size(); i++ )
					sliders[i].Update( msg.time );
			}
			else
			{
				for ( int i = 0; i < sliders.size(); i++ )
					sliders[i].Skip( msg.time );
			}
			//
			pMsg->time = GetInputTime( msg.time );
			return false;
		}
		if ( msg.msg == NWinFrame::SWindowsMsg::KEY_DOWN )
		{
			int i;
			if ( GetIndex( eventBinds, msg.nKey, &i ) && events[i].nEventID >= 0 )
			{
				pMsg->time = GetInputTime( msg.time );
				pMsg->nEventID = events[i].nEventID;
				return true;
			}
			if ( GetIndex( sliderBinds, msg.nKey, &i ) )
				sliders[i].On( msg.time );
		}
		if ( msg.msg == NWinFrame::SWindowsMsg::KEY_UP )
		{
			int i;
			if ( GetIndex( sliderBinds, msg.nKey, &i ) )
				sliders[i].Off( msg.time );
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void NInput::RegisterEvent( int nEventID, const char *pszName )
{
	int nIndex = GetIndex( &events, pszName );
	events[nIndex].nEventID = nEventID;
}
/////////////////////////////////////////////////////////////////////////////////////
// CSlider
/////////////////////////////////////////////////////////////////////////////////////
CSlider::CSlider( const char *pszName ): szName(pszName), prev(0)
{
	nIndex = GetIndex( &sliders, pszName );
	prev = sliders[nIndex].nData;
}
/////////////////////////////////////////////////////////////////////////////////////
float CSlider::GetDelta()
{
	float fpRes = ((float)( sliders[nIndex].nData - prev )) * ( 1.0f / 0x10000 );
	prev = sliders[nIndex].nData;
	return fpRes;
}
/////////////////////////////////////////////////////////////////////////////////////
bool CSlider::IsOn() const
{
	return sliders[nIndex].nIsOn != 0;
}
/////////////////////////////////////////////////////////////////////////////////////
// CRAP fast hack binding (no .cfgs)
void NInput::Bind( const char *pszName, int nKey )
{
	int nIndex = GetIndex( &events, pszName );
	eventBinds.push_back( SBind( nKey, nIndex ) );
}
/////////////////////////////////////////////////////////////////////////////////////
void NInput::BindSlider( const char *pszName, int nKey )
{
	int nIndex = GetIndex( &sliders, pszName );
	sliderBinds.push_back( SBind( nKey, nIndex ) );
}
/////////////////////////////////////////////////////////////////////////////////////
