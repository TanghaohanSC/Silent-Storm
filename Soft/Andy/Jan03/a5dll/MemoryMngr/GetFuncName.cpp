#include "StdAfx.h"
#include "internal.h"
#include <imagehlp.h>
#include <tchar.h>
#include <TLHELP32.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Include these in case the user forgets to link against them.
#pragma comment( lib, "imagehlp.lib" )
#pragma comment( lib, "version.lib" )

// The flag that says if I have already done the GetProcAddress on
//  g_pfnSymGetLineFromAddr.
static BOOL g_bLookedForSymFuncs = FALSE;

// The static source and line structure.
static IMAGEHLP_LINE g_stLine;

// The flag that indicates that the symbol engine as been initialized.
static BOOL g_bSymEngInit = FALSE;

typedef BOOL (__stdcall *PFNSYMGETLINEFROMADDR)( IN  HANDLE hProcess,
																								IN  DWORD dwAddr,
																								OUT PDWORD pdwDisplacement,
																								OUT PIMAGEHLP_LINE Line );
// The pointer to the SymGetLineFromAddr function I GetProcAddress out
//  of IMAGEHLP.DLL in case the user has an older version that does not
//  support the new extensions.
static PFNSYMGETLINEFROMADDR g_pfnSymGetLineFromAddr = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
// The documentation for this function is in BugSlayer.h.
static BOOL __stdcall GetLoadedModules( DWORD dwPID, UINT uiCount, HMODULE *paModArray, LPUINT puiRealCount )
{
  // Do the debug checking.
  ASSERT( NULL != puiRealCount );
  ASSERT( FALSE == IsBadWritePtr(puiRealCount , sizeof(UINT)) );
#ifdef _DEBUG
  if ( 0 != uiCount )
  {
    ASSERT( NULL != paModArray );
    ASSERT( FALSE == IsBadWritePtr(paModArray, uiCount*sizeof(HMODULE)) );
  }
#endif
	
  // Do the parameter checking for real.  Note that I only check the
  //  memory in paModArray if uiCount is > 0.  The user can pass zero
  //  in uiCount if they are just interested in the total to be
  //  returned so they could dynamically allocate a buffer.
  if ( ( TRUE == IsBadWritePtr(puiRealCount, sizeof(UINT)) ) ||
		( (uiCount > 0) && ( TRUE == IsBadWritePtr(paModArray, uiCount*sizeof(HMODULE)) ) ) )
  {
    SetLastErrorEx( ERROR_INVALID_PARAMETER, SLE_ERROR );
    return 0;
  }
	
  // Figure out which OS we are on.
  OSVERSIONINFO stOSVI;
	
  memset( &stOSVI , NULL , sizeof(OSVERSIONINFO) );
  stOSVI.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	
  BOOL bRet = GetVersionEx( &stOSVI );
  ASSERT( TRUE == bRet );
  if ( FALSE == bRet )
  {
    TRACE0( "GetVersionEx failed!\n" );
    return FALSE;
  }
	
  // Check the version and call the appropriate thing.
  if ( ( VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId ) && ( 4 == stOSVI.dwMajorVersion ) )
		// This is NT 4 so call it's specific version.
    return NT4GetLoadedModules( dwPID, uiCount, paModArray, puiRealCount );
  else
    // Everyone other OS goes through TOOLHELP32.
    return TLHELPGetLoadedModules( dwPID, uiCount, paModArray, puiRealCount );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Initializes the symbol engine if needed.
static void InitSymEng()
{
  if ( FALSE == g_bSymEngInit )
  {
    // Set up the symbol engine.
    DWORD dwOpts = SymGetOptions();
		
    // Turn on load lines.
    SymSetOptions( dwOpts | SYMOPT_LOAD_LINES );
		
    // Initialize the symbol engine.
    VERIFY( SymInitialize((HANDLE)GetCurrentProcessId(), 0, FALSE) );
    UINT uiCount;
    // Find out how many modules there are.
    VERIFY( GetLoadedModules(GetCurrentProcessId(), 0, 0, &uiCount) );
    // Allocate something big enough to hold the list.
    HMODULE *paMods = new HMODULE[uiCount];
		
    // Get the list for real.
    if ( FALSE == GetLoadedModules(GetCurrentProcessId(), uiCount, paMods, &uiCount) )
    {
      ASSERT( FALSE );
      // Free the memory that I allocated earlier.
      delete []paMods;
      // There's not much I can do here...
      g_bSymEngInit = FALSE;
      return;
    }
    // The module filename.
    TCHAR szModName[MAX_PATH];
    for ( UINT uiCurr = 0; uiCurr < uiCount; uiCurr++ )
    {
      // Get the module's filename.
      VERIFY( GetModuleFileName( paMods[uiCurr], szModName, sizeof(szModName)) );
			
      // In order to get the symbol engine to work outside a
      //  debugger, it needs a handle to the image.  Yes, this
      //  will leak but the OS will close it down when the process
      //  ends.
      HANDLE hFile = CreateFile( szModName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
			
      VERIFY( SymLoadModule((HANDLE)GetCurrentProcessId(), hFile, szModName, 0, (DWORD)paMods[uiCurr], 0) );
    }
    delete []paMods;
  }
  g_bSymEngInit = TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Cleans up the symbol engine if needed.
static void CleanupSymEng()
{
  if ( TRUE == g_bSymEngInit )
  {
    VERIFY( SymCleanup((HANDLE)GetCurrentProcessId()) );
  }
  g_bSymEngInit = FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static BOOL InternalSymGetLineFromAddr( 
	IN  HANDLE          hProcess,
	IN  DWORD           dwAddr,
	OUT PDWORD          pdwDisplacement,
	OUT PIMAGEHLP_LINE  Line )
{
  // Have I already done the GetProcAddress?
  if ( FALSE == g_bLookedForSymFuncs )
  {
    g_bLookedForSymFuncs = TRUE;
    g_pfnSymGetLineFromAddr = (PFNSYMGETLINEFROMADDR) GetProcAddress( GetModuleHandle(_T("IMAGEHLP.DLL")), "SymGetLineFromAddr" );
  }
  if ( 0 != g_pfnSymGetLineFromAddr )
  {
#ifdef WORK_AROUND_SRCLINE_BUG
		
    // The problem is that the symbol engine only finds those source
    //  line addresses (after the first lookup) that fall exactly on
    //  a zero displacement.  I will walk backwards 100 bytes to
    //  find the line and return the proper displacement.
    DWORD dwTempDis = 0 ;
    while ( FALSE == (*g_pfnSymGetLineFromAddr)(hProcess, dwAddr - dwTempDis, pdwDisplacement, Line) )
    {
      dwTempDis += 1;
      if ( 100 == dwTempDis )
				return FALSE;
    }
		
    // It was found and the source line information is correct so
    //  change the displacement if it was looked up multiple times.
    if ( 0 != dwTempDis )
      *pdwDisplacement = dwTempDis;
    return TRUE;
		
#else  // WORK_AROUND_SRCLINE_BUG
    return (*g_pfnSymGetLineFromAddr)( hProcess, dwAddr, pdwDisplacement, Line );
#endif
  }
  return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetSourceLine( DWORD pointer, const char **ppszFileName, int *_pnLineNumber )
{
	const char *&pszFileName = *ppszFileName;
	int &nLineNumber = *_pnLineNumber;
  ASSERT ( FALSE == IsBadCodePtr(FARPROC(pointer)) );
	
  // A temporary for all to use.  This saves stack space.
  __try
  {
    // Initialize the symbol engine in case it is not initialized.
    InitSymEng();
		
    ZeroMemory ( &g_stLine , sizeof(IMAGEHLP_LINE) );
    g_stLine.SizeOfStruct = sizeof( IMAGEHLP_LINE );
		
    DWORD dwDisp = 0;
    if ( TRUE == InternalSymGetLineFromAddr((HANDLE)GetCurrentProcessId(), pointer, &dwDisp, &g_stLine) )
    {
			pszFileName = g_stLine.FileName;
			nLineNumber = g_stLine.LineNumber;
		}
	}
  __except ( EXCEPTION_EXECUTE_HANDLER )
  {
    ASSERT( FALSE );
		return false;
  }
	
  return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
