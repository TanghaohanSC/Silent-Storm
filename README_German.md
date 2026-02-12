[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm Trailer](Silent_Storm.png)](https://www.youtube.com/watch?v=ugRAaC7K_1I)

Das Computerspiel [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) ist ein rundenbasiertes taktisches RPG, das von [Nival Interactive](http://nival.com/) entwickelt und 2003 veröffentlicht wurde.

Das Spiel ist weiterhin auf [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) und bei [GOG.com](https://www.gog.com/game/silent_storm_gold) verfügbar.

Im Jahr 2026 wurde der Quellcode des Spiels unter einer [special license](LICENSE.md) veröffentlicht, die kommerzielle Nutzung untersagt, aber für die Community des Spiels, Bildungszwecke und Forschung vollständig offen ist. Bitte lesen Sie die Bedingungen der [license agreement](LICENSE.md) vor der Nutzung sorgfältig durch.

## Tech-Stack

- **Engine**: proprietäre 3D-Engine, hauptsächlich in C++ geschrieben
- **Skriptsprache**: Lua 4.0
- **Animation**: eigenes Animationssystem
- **Video**: Bink Video Technology ⚠️ *Kommerzielle Lizenz — nicht enthalten*
- **Sound**: FMOD-Soundsystem ⚠️ *Kommerzielle Lizenz — nicht enthalten*
- **Grafik**: DirectX 8

## Inhalt dieses Repositories

- `Soft/` — Quellcode und Entwicklungstools
- `Complete/` — Spieldaten und -ressourcen
- `Data/` — Konfigurationsdaten des Spiels
- `Tools/` — Entwicklungs- und Build-Tools
- `bin/` — kompilierte ausführbare Dateien
- `cfg/` — Konfigurationsdateien
- `Versions/Current` — Entwicklerversion

---

# Ausführen des Spiels

## Basisstart

1. Navigieren Sie in das Verzeichnis `bin/`.
2. Starten Sie die ausführbare Datei des Spiels: `Game.exe`.

Standardmäßig läuft das Spiel im Fenstermodus mit einer Auflösung von 800x600. Für den Fenstermodus muss die Farbtiefe in den Anzeigeeinstellungen (Display Properties/Settings/Colors) auf 32 Bit (True Color) gesetzt sein. Um das Spiel im Vollbildmodus zu starten, verwenden Sie den Parameter `-fullscreen`. Um die Auflösung zu ändern, verwenden Sie die Parameter `-640`, `-1024` oder `-1280` für 640x480, 1024x768 bzw. 1280x1024.

⚠️ Es gibt Probleme beim Ausführen des Spiels auf modernen Betriebssystemen. Wenn Sie eine Lösung finden, teilen Sie sie uns bitte über GitHub Issues mit.

---

# Karteneditor und Entwicklungstools

## Karteneditor

- **Pfad**: `bin/MapEdit.exe`

---

# Build des Projekts

## Build-Anforderungen

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

### ⚠️ Zusätzliche Tools, die nicht im Quellcode enthalten sind:

- **FMOD-Audiosystem**
- **Bink Video Technology**

### 📋 Lizenzen von Drittkomponenten:

- **STLport** — BSD-ähnliche Lizenz
- **zlib** — zlib-Lizenz
- **Lua 4.0** — MIT-ähnliche Lizenz
- **Ogg Vorbis** — BSD-ähnliche Lizenz
