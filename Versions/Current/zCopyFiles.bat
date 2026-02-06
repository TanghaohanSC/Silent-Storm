@echo off
cd w:\Versions\Current
xcopy /D /T /Y .\*.* c:\_A5
xcopy /D /S /E /Y .\*.* c:\_A5

del c:\_A5\zCopyFiles.bat
del c:\_A5\zCopyFilesGF2.bat
del c:\_A5\zCopyFilesGF4.bat
del c:\_A5\zCopyFilesGF4_drv4041.bat
del c:\_A5\zAlterMilestone.bat
del c:\_A5\yCopyFilesD.bat
call W:\Complete\MakeCurrent.bat c:\_A5
