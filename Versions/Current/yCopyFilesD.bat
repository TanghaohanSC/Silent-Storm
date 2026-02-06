@echo off
xcopy /D /T /Y .\*.* d:\A5
xcopy /D /S /E /Y .\*.* d:\A5
del d:\A5\zCopyFiles.bat
del d:\A5\yCopyFilesD.bat
del d:\A5\zAlterMilestone.bat
call W:\Complete\MakeCurrent.bat d:\A5
