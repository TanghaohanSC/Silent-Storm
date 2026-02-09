[English](README_English.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm](https://img.shields.io/badge/Game-Silent_Storm-blue.svg)](https://fr.wikipedia.org/wiki/Silent_Storm)
[![License: Custom](https://img.shields.io/badge/License-Nival_Ltd-yellow.svg)](../../LICENSE.md)
[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)

<p align="center">
    <a href="https://www.youtube.com/watch?v=NPkFx1h1HEI">
        <img src="Silent_Storm.png" width="320" alt="Silent Storm">
    </a>
</p>

Le jeu vidéo [Silent Storm](https://fr.wikipedia.org/wiki/Silent_Storm) est un jeu de stratégie/RPG tactique au tour par tour développé par [Nival Interactive](http://nival.com/), sorti en 2003.

Le jeu est toujours disponible sur [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) et [GOG.com](https://www.gog.com/game/silent_storm_gold).

Le code source du jeu a été publié sous une [licence spéciale](../../LICENSE.md) de Nival International Ltd., qui est entièrement ouverte à la communauté, à des fins éducatives et de recherche. Veuillez examiner attentivement les termes de l'[accord de licence](../../LICENSE.md) avant utilisation.

## Stack Technologique

- **Moteur**: moteur 3D propriétaire, principalement écrit en C++
- **Langage de script**: Lua 4.0
- **Animation**: Système d'animation personnalisé
- **Vidéo**: Bink Video Technology ⚠️ *Licence commerciale - non incluse*
- **Son**: FMOD sound system ⚠️ *Licence commerciale - non incluse*
- **Graphiques**: DirectX 8

## Contenu de ce dépôt

- `Soft/` — code source et outils de développement
- `Complete/` — données et ressources du jeu
- `Data/` — données de configuration du jeu
- `Tools/` — outils de développement et de compilation
- `bin/` — exécutables compilés
- `cfg/` — fichiers de configuration
- `Versions/Current` — version pour développeurs

---

# Lancer le Jeu

## Lancement de Base

1. Naviguez vers le répertoire `bin/`
2. Exécutez le fichier exécutable du jeu `Game.exe`

Par défaut, le jeu s'exécute en mode fenêtré à une résolution de 800x600. Pour le mode fenêtré, la profondeur de couleur pour le mode actuel dans Display Properties/Settings/Colors doit être de 32 bits (truecolor). Pour exécuter en mode plein écran, utilisez le paramètre –fullscreen. Pour changer la résolution, utilisez les paramètres –640, -1024, -1280 pour les résolutions 640x480, 1024x768, 1280x1024 respectivement.

⚠️ Il y a des problèmes d'exécution sur les systèmes d'exploitation modernes. Si vous trouvez une solution, veuillez nous en informer via GitHub Issues.

---

# Éditeur de Cartes et Outils de Développement

## Éditeur de Cartes

- **Emplacement**: `bin/MapEdit.exe`

---

# Compiler le Projet

## Exigences de Compilation

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

## Informations sur les Licences

Ce projet est publié sous une **licence spéciale non commerciale** de NIVAL INTERNATIONAL LTD.

### ⚠️ Outils supplémentaires non inclus dans le code source:

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 Licences des composants tiers:

- **STLport** - Licence de type BSD
- **zlib** - Licence zlib
- **Lua 4.0** - Licence de type MIT
- **Ogg Vorbis** - Licence de type BSD

Veuillez examiner attentivement l'[accord de licence](../../LICENSE.md) complet avant d'utiliser ce code.

---

# Informations Supplémentaires

## Statut du Projet

Il s'agit d'une **publication historique du code source** de 2003. Le code est fourni tel quel à des fins éducatives, de préservation et de développement communautaire potentiel.

| Composant                                | Notes                                |
|------------------------------------------|--------------------------------------|
| Compilation originale (VS .NET 2003)     | Nécessite les outils d'origine       |
| Dépendances propriétaires supprimées    | FMOD, Bink nécessitent un remplacement |

## Auteurs

**Développement Original:** Nival Interactive (2001-2003)

## Support

- **Problèmes:** Utilisez GitHub Issues
- **Améliorations:** Si vous souhaitez suggérer une correction ou une nouvelle fonctionnalité, veuillez créer une pull request
- **Communauté :** Si vous êtes propriétaire ou participant actif de la communauté du jeu et que vous êtes intéressé par le développement du projet, veuillez écrire à karim.kimsanbaev@gmail.com.

---

> **Note:** Il s'agit d'une publication historique du code source. Le jeu est toujours disponible commercialement. Veuillez soutenir les éditeurs originaux si vous avez apprécié le jeu.

**Silent Storm&trade;** est une marque déposée de ses propriétaires. Ce dépôt est uniquement à des fins éducatives et de préservation.

