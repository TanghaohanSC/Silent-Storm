W:\Tools\pskill.exe tasktracker.exe
if not exist C:\A5 md c:\A5
xcopy W:\Versions\Current\TaskTracker.exe C:\A5\ /D /I /Y
xcopy W:\Versions\Current\taskTracker.resources C:\A5\ /D /I /Y
c:
cd c:\
cd a5
start TaskTracker.exe
