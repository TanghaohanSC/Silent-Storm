#ifndef __IINPUT_H_
#define __IINPUT_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
namespace NInput
{
	typedef DWORD STime;
	//
	struct SMessage
	{
		int nEventID;
		STime time;
	};
	//
	bool IsExit();
	bool IsActive();
	//
	void PumpMessages();
	bool GetMessage( SMessage *pMsg );
	void RegisterEvent( int nEventID, const char *pszName );
	//
	class CSlider
	{
		string szName;
		int64 prev;
		int nIndex;
	public:
		CSlider( const char *pszName );
		float GetDelta();
		bool IsOn() const;
	};
	//CRAP
	void Bind( const char *pszName, int nKey );
	void BindSlider( const char *pszName, int nKey );
};
/////////////////////////////////////////////////////////////////////////////////////
#endif