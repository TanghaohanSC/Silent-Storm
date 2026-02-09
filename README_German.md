[English](README_English.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm](https://img.shields.io/badge/Game-Silent_Storm-blue.svg)](https://de.wikipedia.org/wiki/Silent_Storm)
[![License: Custom](https://img.shields.io/badge/License-Nival_Ltd-yellow.svg)](../../LICENSE.md)
[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)

<p align="center">
    <a href="https://www.youtube.com/watch?v=NPkFx1h1HEI">
        <img src="Silent_Storm.png" width="320" alt="Silent Storm">
    </a>
</p>

Das Computerspiel [Silent Storm](https://de.wikipedia.org/wiki/Silent_Storm) ist ein taktisches rundenbasiertes RPG/Strategiespiel, entwickelt von [Nival Interactive](http://nival.com/), veröffentlicht im Jahr 2003.

Das Spiel ist weiterhin auf [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) und [GOG.com](https://www.gog.com/game/silent_storm_gold) verfügbar.

Der Quellcode des Spiels wurde unter einer [speziellen Lizenz](../../LICENSE.md) von Nival International Ltd. veröffentlicht, die vollständig offen für die Community, Bildungs- und Forschungszwecke ist. Bitte lesen Sie die Bedingungen der [Lizenzvereinbarung](../../LICENSE.md) sorgfältig durch, bevor Sie sie verwenden.

## Technologie-Stack

- **Engine**: proprietäre 3D-Engine, hauptsächlich in C++ geschrieben
- **Skriptsprache**: Lua 4.0
- **Animation**: Benutzerdefiniertes Animationssystem
- **Video**: Bink Video Technology ⚠️ *Kommerzielle Lizenz - nicht enthalten*
- **Sound**: FMOD sound system ⚠️ *Kommerzielle Lizenz - nicht enthalten*
- **Grafik**: DirectX 8

## Was ist in diesem Repository enthalten

- `Soft/` — Quellcode und Entwicklungstools
- `Complete/` — Spieldaten und Ressourcen
- `Data/` — Spielkonfigurationsdaten
- `Tools/` — Entwicklungs- und Build-Tools
- `bin/` — kompilierte ausführbare Dateien
- `cfg/` — Konfigurationsdateien
- `Versions/Current` — Entwicklerversion

---

# Das Spiel Starten

## Grundlegender Start

1. Navigieren Sie zum Verzeichnis `bin/`
2. Führen Sie die ausführbare Datei des Spiels `Game.exe` aus

Standardmäßig läuft das Spiel im Fenstermodus mit einer Auflösung von 800x600. Für den Fenstermodus muss die Farbtiefe für den aktuellen Modus in Display Properties/Settings/Colors 32 Bit (Truecolor) betragen. Um im Vollbildmodus zu starten, verwenden Sie den Parameter –fullscreen. Um die Auflösung zu ändern, verwenden Sie die Parameter –640, -1024, -1280 für die Auflösungen 640x480, 1024x768, 1280x1024.

⚠️ Es gibt Probleme beim Ausführen auf modernen Betriebssystemen. Wenn Sie eine Lösung finden, lassen Sie es uns bitte über GitHub Issues wissen.

---

# Karten-Editor und Entwicklungstools

## Karten-Editor

- **Standort**: `bin/MapEdit.exe`

---

# Projekt Kompilieren

## Build-Anforderungen

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

## Lizenzinformationen

Dieses Projekt wird unter einer **speziellen nicht-kommerziellen Lizenz** von NIVAL INTERNATIONAL LTD veröffentlicht.

### ⚠️ Zusätzliche Tools, die nicht im Quellcode enthalten sind:

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 Lizenzen von Drittanbieter-Komponenten:

- **STLport** - BSD-ähnliche Lizenz
- **zlib** - zlib-Lizenz
- **Lua 4.0** - MIT-ähnliche Lizenz
- **Ogg Vorbis** - BSD-ähnliche Lizenz

Bitte lesen Sie die vollständige [Lizenzvereinbarung](../../LICENSE.md) sorgfältig durch, bevor Sie diesen Code verwenden.

---

# Zusätzliche Informationen

## Projektstatus

Dies ist eine **historische Quellcode-Veröffentlichung** aus dem Jahr 2003. Der Code wird wie besehen für Bildungszwecke, Erhaltung und potenzielle Community-Entwicklung bereitgestellt.

| Komponente                                  | Hinweise                             |
|---------------------------------------------|--------------------------------------|
| Original-Build (VS .NET 2003)               | Erfordert Original-Tools             |
| Proprietäre Abhängigkeiten entfernt         | FMOD, Bink erfordern Ersatz          |

## Autoren

**Original-Entwicklung:** Nival Interactive (2001-2003)

## Support

- **Probleme:** Verwenden Sie GitHub Issues
- **Verbesserungen:** Wenn Sie eine Korrektur oder neue Funktion vorschlagen möchten, erstellen Sie bitte einen Pull Request
- **Community:** Wenn Sie ein Eigentümer oder aktives Teilnehmer der Game-Community sind und an der Projektentwicklung interessiert sind, schreiben Sie bitte an karim.kimsanbaev@gmail.com.

---

> **Hinweis:** Dies ist eine historische Quellcode-Veröffentlichung. Das Spiel ist weiterhin kommerziell erhältlich. Bitte unterstützen Sie die ursprünglichen Herausgeber, wenn Ihnen das Spiel gefallen hat.

**Silent Storm&trade;** ist ein Warenzeichen seiner Eigentümer. Dieses Repository dient nur Bildungs- und Erhaltungszwecken.

