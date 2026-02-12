[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm Trailer](Silent_Storm.png)](https://www.youtube.com/watch?v=ugRAaC7K_1I)

电脑游戏 [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) 是一款由 [Nival Interactive](http://nival.com/) 开发并于 2003 年发行的回合制战术角色扮演游戏（RPG）。

该游戏目前仍可在 [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) 和 [GOG.com](https://www.gog.com/game/silent_storm_gold) 上购买。

在 2026 年，游戏的源代码以一种[特殊许可证](LICENSE.md)发布，该许可证禁止商业用途，但对游戏社区、教育和科研用途完全开放。使用前请仔细阅读[许可协议](LICENSE.md)中的条款。

## 技术栈

- **引擎**：专有 3D 引擎，主要使用 C++ 编写
- **脚本语言**：Lua 4.0
- **动画**：自定义动画系统
- **视频**：Bink Video Technology ⚠️ *商业授权 — 不包含在内*
- **音频**：FMOD 音频系统 ⚠️ *商业授权 — 不包含在内*
- **图形**：DirectX 8

## 本仓库包含的内容

- `Soft/` — 源代码和开发工具
- `Complete/` — 游戏数据与资源
- `Data/` — 游戏配置数据
- `Tools/` — 开发与构建工具
- `bin/` — 已编译的可执行文件
- `cfg/` — 配置文件
- `Versions/Current` — 开发者版本

---

# 运行游戏

## 基本启动方式

1. 进入 `bin/` 目录。
2. 运行游戏可执行文件 `Game.exe`。

默认情况下，游戏以 800x600 分辨率的窗口模式运行。要使用窗口模式，显示属性（Display Properties/Settings/Colors）中的颜色深度必须设置为 32-bit（真彩色）。要以全屏模式运行，请使用参数 `-fullscreen`。要更改分辨率，可分别使用参数 `-640`、`-1024` 或 `-1280` 来设置 640x480、1024x768 或 1280x1024 分辨率。

⚠️ 在现代操作系统上运行游戏时可能会出现问题。如果你找到了解决方案，请通过 GitHub Issues 告诉我们。

---

# 地图编辑器与开发工具

## 地图编辑器

- **位置**：`bin/MapEdit.exe`

---

# 构建项目

## 构建要求

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

### ⚠️ 源代码中未包含的其他工具：

- **FMOD 音频系统**
- **Bink Video Technology 视频技术**

### 📋 第三方组件许可证：

- **STLport** — 类 BSD 许可证
- **zlib** — zlib 许可证
- **Lua 4.0** — 类 MIT 许可证
- **Ogg Vorbis** — 类 BSD 许可证
