@echo off
set MSDestDir=W:\Versions\Milestones\#9_May\Progs
xcopy /D /T /Y .\*.* %MSDestDir%
xcopy /D /S /E /Y .\*.* %MSDestDir%
rem copy w:\Data\Data.mdb %MSDestDir%
del %MSDestDir%\zCopyFiles.bat
del %MSDestDir%\yCopyFilesD.bat
del %MSDestDir%\zAlterMilestone.bat
del /Q %MSDestDir%\res\*.*
call W:\Complete\MakeCurrent.bat %MSDestDir%