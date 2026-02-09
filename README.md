[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm](https://img.shields.io/badge/Game-Silent_Storm-blue.svg)](https://en.wikipedia.org/wiki/Silent_Storm)
[![License: Custom](https://img.shields.io/badge/License-Nival_Ltd-yellow.svg)](../../LICENSE.md)
[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)

<p align="center">
    <a href="https://www.youtube.com/watch?v=NPkFx1h1HEI">
        <img src="Silent_Storm.png" width="320" alt="Silent Storm">
    </a>
</p>

The computer game [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) is a tactical turn-based RPG/strategy game developed by [Nival Interactive](http://nival.com/), released in 2003.

The game is still available on [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) and [GOG.com](https://www.gog.com/game/silent_storm_gold).

The game's source code was released under a [special license](../../LICENSE.md) from Nival International Ltd., which is fully open for community, educational, and research purposes. Please carefully review the terms of the [license agreement](../../LICENSE.md) before use.

## Technology Stack

- **Engine**: proprietary 3D engine, primarily written in C++
- **Scripting Language**: Lua 4.0
- **Animation**: Custom animation system
- **Video**: Bink Video Technology ⚠️ *Commercial license - not included*
- **Sound**: FMOD sound system ⚠️ *Commercial license - not included*
- **Graphics**: DirectX 8

## What's in this repository

- `Soft/` — source code and development tools
- `Complete/` — game data and resources
- `Data/` — game configuration data
- `Tools/` — development and build tools
- `bin/` — compiled executables
- `cfg/` — configuration files
- `Versions/Current` — developer version

---

# Running the Game

## Basic Launch

1. Navigate to the `bin/` directory
2. Run the game executable `Game.exe`

By default, the game runs in windowed mode at 800x600 resolution. For windowed mode, the color depth for the current mode in Display Properties/Settings/Colors must be 32-bit (truecolor). To run in fullscreen mode, use the –fullscreen parameter. To change resolution, use the –640, -1024, -1280 parameters for resolutions 640x480, 1024x768, 1280x1024 respectively.

⚠️ There are issues running on modern operating systems. If you find a solution, please let us know through GitHub Issues.

---

# Map Editor and Development Tools

## Map Editor

- **Location**: `bin/MapEdit.exe`

---

# Building the Project

## Build Requirements

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

## License Information

This project is released under a **special non-commercial license** from NIVAL INTERNATIONAL LTD.

### ⚠️ Additional tools not included in the source code:

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 Third-party component licenses:

- **STLport** - BSD-like license
- **zlib** - zlib License
- **Lua 4.0** - MIT-like license
- **Ogg Vorbis** - BSD-like license

Please carefully review the full [license agreement](../../LICENSE.md) before using this code.

---

# Additional Information

## Project Status

This is a **historical source code release** from 2003. The code is provided as-is for educational purposes, preservation, and potential community development.

| Component                          | Notes                            |
|------------------------------------|----------------------------------|
| Original build (VS .NET 2003)      | Requires original tools          |
| Proprietary dependencies removed   | FMOD, Bink require replacement   |

## Authors

**Original Development:** Nival Interactive (2001-2003)

## Support

- **Issues:** Use GitHub Issues
- **Improvements:** If you want to suggest a fix or new feature, please create a pull request

---

> **Note:** This is a historical source code release. The game is still commercially available. Please support the original publishers if you enjoyed the game.

**Silent Storm&trade;** is a trademark of its owners. This repository is for educational purposes and preservation only.

[![Blitzkrieg Trailer](Blitzkrieg.png)](https://www.youtube.com/watch?v=zNxMvTcsJbk)

The computer game [Blitzkrieg](https://wikipedia.org/wiki/Blitzkrieg_(video_game)) is the first installment of the legendary series of real-time strategy war games, developed by [Nival Interactive](http://nival.com/) and released by 1C Company on March 28, 2003.

The game is still available on [Steam](https://store.steampowered.com/app/313480/Blitzkrieg_Anthology/) and [GOG.com](https://www.gog.com/en/game/blitzkrieg_anthology).

In 2025, the game's singleplayer source code was released under a [special license](LICENSE.md) that prohibits commercial use but is completely open for the game's community, education and research.
Please review the terms of the [license agreement](LICENSE.md) carefully before using it.

# What is in this repository
- `Data` - game data
- `Soft` and `Tools` - development tools
- `Versions` - compiled versions of the game, including map editors
- `Sources` - source code and tools
- `Versions/Temporary/Engine/Sources` - complete game engine source code

# Preparation

All libraries from the SDK directory are needed for compilation. The paths to them must be entered in **Tools => Options => Directories** in the following order:

## Include
```
C:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\VC98\STLPORT
C:\SDK\BINK (not included in the repository)
C:\SDK\FMOD\API\INC (not included in the repository)
C:\SDK\S3TC
C:\SDK\STINGRAY STUDIO 2002\INCLUDE\TOOLKIT (not included in the repository)
C:\SDK\STINGRAY STUDIO 2002\INCLUDE (not included in the repository)
C:\SDK\STINGRAY STUDIO 2002\REGEX\INCLUDE (not included in the repository)
C:\SDK\Maya4.0\include
```

## Lib
```
C:\SDK\BINK (not included in the repository)
C:\SDK\FMOD\API\LIB (not included in the repository)
C:\SDK\S3TC
C:\SDK\STINGRAY STUDIO 2002\LIB (not included in the repository)
C:\SDK\STINGRAY STUDIO 2002\REGEX\LIB (not included in the repository)
C:\SDK\Maya4.0\lib
```

In addition, **DirectX 8.1** or higher is required (it will automatically be added to the paths).

### Important Notes

### ⚠️ Additional tools not included in source code:
- **FMOD Audio System**
- **Bink Video Technology**
- **Granny3D Animation System**
- **Stingray Studio UI Components**
- **MySQL Database**
- **S3TC Texture Compression**

### 📋 Third-party licenses:
- **zlib** (v1.1.3) - Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler - zlib License

Please review the complete [license agreement](LICENSE.md) before using this code.

- **Bink, FMOD, Stingray** libraries are not included in this repository as they require separate licensing.
- **stlport** *must* be located in the Visual C directory, alongside `include`.
- The path `C:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\VC98\STLPORT` must be **first**, otherwise, the build will fail.

---

# Additional Tools

- The **tools** directory contains utilities used during the build process.
- Resources are stored in **zip (deflate)** format and are packed/unpacked using **zip/unzip**.
- **Do not use pkzip** — it truncates file names and does not use the deflate algorithm.
- Some data is edited manually using an **XML-editor**, as frequent editing was not necessary and writing a separate editor was impractical.

---

# Files in `data`

In the game's directory, under **data**, there are files that are manually edited or simply placed:

- `sin.arr` — binary file with a sine table (just place it, do not touch).
- `objects.xml` — registry of game objects (edited manually).
- `consts.xml` — game constants for designers (edited manually).
- `MusicSettings.xml` — music settings (edited manually).
- `partys.xml` — country data (which squad to use for gun crew, parachutist model, etc.).

## Files in `medals`

In the **medals** subdirectory, files `ranks.xml` contain ranks and **experience** needed to obtain them, organized by country.
