@echo off
if "%1" == "" goto updateCA5
echo updating %1
echo Importing database...
cd w:\
w:
cd data
w:\tools\dataImport.exe %1\game.db -dbserver a5server -database A5GAME
cd w:\complete
w:\Tools\PkgBuilder %1\res\AIBinds.res w:\Complete\AIBinds
w:\Tools\PkgBuilder %1\res\AIBSPTrees.res w:\Complete\AIBSPTrees
w:\Tools\PkgBuilder %1\res\AIGeometries.res w:\Complete\AIGeometries
w:\Tools\PkgBuilder %1\res\Animations.res w:\Complete\Animations
w:\Tools\PkgBuilder %1\res\Binds.res w:\Complete\Binds
w:\Tools\PkgBuilder %1\res\Effects.res w:\Complete\Effects
w:\Tools\PkgBuilder %1\res\Fonts.res w:\Complete\Fonts
w:\Tools\PkgBuilder %1\res\Geometries.res w:\Complete\Geometries
w:\Tools\PkgBuilder %1\res\Skeletons.res w:\Complete\Skeletons
w:\Tools\PkgBuilder %1\res\Textures.res w:\Complete\Textures
w:\Tools\PkgBuilder %1\res\Terrain.res w:\Complete\Terrain
w:\Tools\PkgBuilder %1\res\Locators.res w:\Complete\Locators
w:\Tools\PkgBuilder %1\res\Sounds.res w:\Complete\Sounds
w:\Tools\PkgBuilder %1\res\Buildings.res w:\Complete\Buildings
w:\Tools\PkgBuilder %1\res\Waypoints.res w:\Complete\Waypoints
w:\Tools\PkgBuilder %1\res\Units.res w:\Complete\Units
w:\Tools\PkgBuilder %1\res\Heads.res w:\Complete\Heads
w:\Tools\PkgBuilder %1\res\Sequences.res w:\Complete\Sequences
w:\Tools\PkgBuilder %1\res\Lights.res w:\Complete\Lights
w:\Tools\PkgBuilder %1\res\Chapters.res w:\Complete\Chapters
w:\Tools\PkgBuilder %1\res\Globals.res w:\Complete\Globals
w:\Tools\PkgBuilder %1\res\Groups.res w:\Complete\Groups
w:\Tools\PkgBuilder %1\res\LRTextures.res w:\Complete\LRTextures
rem copy w:\Complete\game.db %1\game.db
rem w:\Tools\PkgBuilder w:\Versions\Current\res\Scripts.res w:\Complete\Scripts
rem xcopy /D /T /Y .\*.* w:\Versions\Current
rem xcopy /D /S /E /Y .\*.* w:\Versions\Current
rem xcopy /D /Y .\*.* w:\Versions\Current
rem del w:\Versions\Current\makeCurrent.bat
rem rmdir /S /Q w:\Versions\Current\Logs
goto end
:updateCA5
call w:\complete\makeCurrent.bat c:\a5
:end