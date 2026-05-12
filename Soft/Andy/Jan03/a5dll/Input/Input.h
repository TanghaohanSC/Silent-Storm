#ifndef __INPUT_H__
#define __INPUT_H__
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef DWORD STime;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NInput
{
////////////////////////////////////////////////////////////////////////////////////////////////////
	enum EPOVAxis
	{
		PA_UNKNOWN,
		PA_X,
		PA_Y
	};
	enum EControlType
	{
		CT_KEY,
		CT_POV,
		CT_AXIS,
		CT_TIME,
		CT_LIMAXIS,
		CT_UNKNOWN
	};
////////////////////////////////////////////////////////////////////////////////////////////////////
	struct SMessage
	{
		int nAction;
		EPOVAxis ePOVAxis;
		EControlType cType;

		int nParam;
		bool bState;
		STime tTime;
	};
////////////////////////////////////////////////////////////////////////////////////////////////////
	bool InitInput( HWND hWnd, bool bNonExclusiveMode = false, int nSampleBufferSize = -1 );
	bool DoneInput();

	void PumpMessages( bool bFocus );
	bool GetMessage( SMessage *pMsg );
	bool GetCharForKey( int nVirtualKey, WCHAR *pwcChar );
	bool GetKeyForMessage( const SMessage &mMsg, int *pnVirtualKey );
		
	int GetControlID( const string &sCommand );
	void GetControlInfo( int nAction, EControlType *pcType, float *pfGranularity );

	void StartSaveInput( CDataStream *pStream );
	void StopSaveInput();
	void StartEmulateInput( CDataStream *pStream );
	void StopEmulateInput();

	// SDL3 input bridge entry point. Pushes a pre-built SMessage directly
	// into the message queue, bypassing DirectInput8. Called from
	// port/src/platform/sdl_input_bridge.cpp.
	void PushMessageSDL( const SMessage &msg );
////////////////////////////////////////////////////////////////////////////////////////////////////
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
