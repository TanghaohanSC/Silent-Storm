#ifndef __A5_SAVEMANAGER_H_
#define __A5_SAVEMANAGER_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GPixelFormat.h"
#include "..\Misc\2DArray.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFileStream;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NMainLoop
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_SAVE_MAGIC_NUMBER = 0x818B9F21;
const int N_SAVE_SCREENSHOT_X = 320;
const int N_SAVE_SCREENSHOT_Y = 200;
const char S_SLOT_ACTIVE[] = "temp";
const char S_SAVE_FILENAME[] = "game.sav";
const char S_SLOT_QUICKSAVE[] = "quicksave";
const char S_INVALID_SAVE_CHARS[] = ".<>\\/|\"*^:?";
////////////////////////////////////////////////////////////////////////////////////////////////////
// SSaveFileHeader
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SSaveFileHeader
{
	int nMagic;
	int nChecksum;

	NGfx::SPixel8888 sScreenShot[N_SAVE_SCREENSHOT_Y][N_SAVE_SCREENSHOT_X];
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSaveManager
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSaveManager
{
private:
	int nActiveSlotID;
	string szActiveProfile;

public:
	CSaveManager();

//// Profiles
	void CreateProfile( const string &szProfile ) const;
	void DeleteProfile( const string &szProfile ) const;
	void GetProfilesList( list<string> *pList ) const;

	const string& GetActiveProfile() const;
	void SetActiveProfile( const string &szProfile );

//// Slots
	void SaveSlot( const string &szName );
	void LoadSlot( const string &szName );
	void ClearSlot( const string &szName );
	void DeleteSlot( const string &szName );
	void PrepareSlot( const string &szName );
	void GetSlotsList( list<string> *pList ) const;

	void GetSlotTime( const string &szName, wstring *pTime );
	void GetSlotScreenShot( const string &szName, CArray2D<NGfx::SPixel8888> *pScreenShot );

	string GetSlotFilePath( const string &szName, const string &szFileName ) const;
};
////
CSaveManager* GetSaveManager();
////
void CreateDir( const string &szDir );
void RemoveDir( const string &szDir );
void CopyFiles( const string &szSource, const string &szTarget, const string &szMask );
////////////////////////////////////////////////////////////////////////////////////////////////////
}; // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif