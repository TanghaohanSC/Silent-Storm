[English](README_English.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

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
- **Community:** If you are an owner or active participant of a game community and you are interested in the project development, please write to karim.kimsanbaev@gmail.com.

---

> **Note:** This is a historical source code release. The game is still commercially available. Please support the original publishers if you enjoyed the game.

**Silent Storm&trade;** is a trademark of its owners. This repository is for educational purposes and preservation only.