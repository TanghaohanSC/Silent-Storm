[English](README_English.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm](https://img.shields.io/badge/Game-Silent_Storm-blue.svg)](https://zh.wikipedia.org/wiki/Silent_Storm)
[![License: Custom](https://img.shields.io/badge/License-Nival_Ltd-yellow.svg)](../../LICENSE.md)
[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)

<p align="center">
    <a href="https://www.youtube.com/watch?v=NPkFx1h1HEI">
        <img src="Silent_Storm.png" width="320" alt="Silent Storm">
    </a>
</p>

电脑游戏 [Silent Storm](https://zh.wikipedia.org/wiki/Silent_Storm) 是由 [Nival Interactive](http://nival.com/) 开发的战术回合制 RPG/策略游戏，于2003年发布。

该游戏仍可在 [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) 和 [GOG.com](https://www.gog.com/game/silent_storm_gold) 上获得。

游戏源代码在 Nival International Ltd. 的[特殊许可证](../../LICENSE.md)下发布，完全开放用于社区、教育和研究目的。使用前请仔细阅读[许可协议](../../LICENSE.md)条款。

## 技术栈

- **引擎**：专有3D引擎，主要用C++编写
- **脚本语言**：Lua 4.0
- **动画**：自定义动画系统
- **视频**：Bink Video Technology ⚠️ *商业许可证 - 未包含*
- **音频**：FMOD sound system ⚠️ *商业许可证 - 未包含*
- **图形**：DirectX 8

## 此仓库包含的内容

- `Soft/` — 源代码和开发工具
- `Complete/` — 游戏数据和资源
- `Data/` — 游戏配置数据
- `Tools/` — 开发和构建工具
- `bin/` — 编译的可执行文件
- `cfg/` — 配置文件
- `Versions/Current` — 开发者版本

---

# 运行游戏

## 基本启动

1. 导航到 `bin/` 目录
2. 运行游戏可执行文件 `Game.exe`

默认情况下，游戏以800x600分辨率的窗口模式运行。对于窗口模式，Display Properties/Settings/Colors中当前模式的颜色深度必须为32位（真彩色）。要以全屏模式运行，请使用 –fullscreen 参数。要更改分辨率，请使用 –640、-1024、-1280 参数分别对应640x480、1024x768、1280x1024分辨率。

⚠️ 在现代操作系统上运行存在问题。如果您找到解决方案，请通过 GitHub Issues 告诉我们。

---

# 地图编辑器和开发工具

## 地图编辑器

- **位置**：`bin/MapEdit.exe`

---

# 构建项目

## 构建要求

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

## 许可证信息

本项目在 NIVAL INTERNATIONAL LTD 的**特殊非商业许可证**下发布。

### ⚠️ 源代码中未包含的附加工具：

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 第三方组件许可证：

- **STLport** - 类BSD许可证
- **zlib** - zlib许可证
- **Lua 4.0** - 类MIT许可证
- **Ogg Vorbis** - 类BSD许可证

使用此代码前请仔细阅读完整的[许可协议](../../LICENSE.md)。

---

# 附加信息

## 项目状态

这是2003年的**历史源代码发布**。代码按原样提供，用于教育目的、保存和潜在的社区开发。

| 组件                          | 说明                     |
|-------------------------------|--------------------------|
| 原始构建 (VS .NET 2003)       | 需要原始工具             |
| 已删除专有依赖项              | FMOD、Bink需要替换       |

## 作者

**原始开发**：Nival Interactive (2001-2003)

## 支持

- **问题**：使用 GitHub Issues
- **改进**：如果您想建议修复或新功能，请创建 pull request

---

> **注意**：这是历史源代码发布。游戏仍可商业获得。如果您喜欢这款游戏，请支持原始发行商。

**Silent Storm&trade;** 是其所有者的商标。此仓库仅用于教育和保存目的。

