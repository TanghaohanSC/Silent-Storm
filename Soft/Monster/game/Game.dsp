# Microsoft Developer Studio Project File - Name="Game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Game - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Game.mak" CFG="Game - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Game - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Game - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/A5/Game", JSKAAAAA"
# PROP Scc_LocalPath "."
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Game - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\home\A5\Game\Release"
# PROP Intermediate_Dir "c:\home\A5\Game\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /Gr /MT /W3 /GX /Zi /Ox /Ot /Oa /Ow /Og /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib d3d8.lib d3dx8.lib /nologo /subsystem:windows /debug /machine:I386

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\home\A5\Game\Debug"
# PROP Intermediate_Dir "c:\home\A5\Game\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"StdAfx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib d3d8.lib d3dx8.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Game - Win32 Release"
# Name "Game - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"StdAfx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\stl_user_config.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Gfx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Gfx.cpp
# End Source File
# Begin Source File

SOURCE=.\Gfx.h
# End Source File
# Begin Source File

SOURCE=.\GfxEffects.cpp
# End Source File
# Begin Source File

SOURCE=.\GfxEffects.h
# End Source File
# Begin Source File

SOURCE=.\GfxInternal.h
# End Source File
# Begin Source File

SOURCE=.\GfxUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\GfxUtils.h
# End Source File
# End Group
# Begin Group "DG"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DG.CPP
# End Source File
# Begin Source File

SOURCE=.\DG.H
# End Source File
# Begin Source File

SOURCE=.\GFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\GFormat.h
# End Source File
# Begin Source File

SOURCE=.\GScene.cpp
# End Source File
# Begin Source File

SOURCE=.\GScene.h
# End Source File
# Begin Source File

SOURCE=.\GSceneGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\GSceneGraph.h
# End Source File
# Begin Source File

SOURCE=.\GSceneInternal.h
# End Source File
# End Group
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera.h
# End Source File
# Begin Source File

SOURCE=.\iMain.cpp
# End Source File
# Begin Source File

SOURCE=.\iMain.h
# End Source File
# Begin Source File

SOURCE=.\iMission.cpp
# End Source File
# Begin Source File

SOURCE=.\iMission.h
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\iInput.cpp
# End Source File
# Begin Source File

SOURCE=.\iInput.h
# End Source File
# End Group
# Begin Group "World"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\wInterface.h
# End Source File
# Begin Source File

SOURCE=.\wMain.cpp
# End Source File
# Begin Source File

SOURCE=.\wMain.h
# End Source File
# End Group
# Begin Group "FileIO"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BasicChunk1.cpp
# End Source File
# Begin Source File

SOURCE=.\BasicChunk1.h
# End Source File
# Begin Source File

SOURCE=.\BasicDB.h
# End Source File
# Begin Source File

SOURCE=.\BasicDBfake.cpp
# End Source File
# Begin Source File

SOURCE=.\FilesPackage.cpp
# End Source File
# Begin Source File

SOURCE=.\FilesPackage.h
# End Source File
# Begin Source File

SOURCE=.\Streams.cpp
# End Source File
# Begin Source File

SOURCE=.\Streams.h
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\2Darray.h
# End Source File
# Begin Source File

SOURCE=.\Basic1.h
# End Source File
# Begin Source File

SOURCE=.\BasicFactory.h
# End Source File
# Begin Source File

SOURCE=.\Geom.h
# End Source File
# Begin Source File

SOURCE=.\HPTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\HPTimer.h
# End Source File
# Begin Source File

SOURCE=.\StrProc.cpp
# End Source File
# Begin Source File

SOURCE=.\StrProc.h
# End Source File
# Begin Source File

SOURCE=.\Tools.h
# End Source File
# Begin Source File

SOURCE=.\Transform.cpp
# End Source File
# Begin Source File

SOURCE=.\Transform.h
# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Win32Helper.h
# End Source File
# Begin Source File

SOURCE=.\WinFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\WinFrame.h
# End Source File
# End Group
# Begin Group "DBFormat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DataFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\DataFormat.h
# End Source File
# End Group
# Begin Group "RPGsystem"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RPGGlobal.cpp
# End Source File
# Begin Source File

SOURCE=.\RPGGlobal.h
# End Source File
# Begin Source File

SOURCE=.\RPGMerc.cpp
# End Source File
# Begin Source File

SOURCE=.\RPGMerc.h
# End Source File
# Begin Source File

SOURCE=.\RPGMission.cpp
# End Source File
# Begin Source File

SOURCE=.\RPGMission.h
# End Source File
# Begin Source File

SOURCE=.\RPGUnitInfo.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
