[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm Trailer](Silent_Storm.png)](https://www.youtube.com/watch?v=ugRAaC7K_1I)

Le jeu vidéo [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) est un RPG tactique au tour par tour développé par [Nival Interactive](http://nival.com/) et sorti en 2003.

Le jeu est toujours disponible sur [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) et sur [GOG.com](https://www.gog.com/game/silent_storm_gold).

En 2026, le code source du jeu a été publié sous une [special license](LICENSE.md) qui interdit l’utilisation commerciale mais est entièrement ouverte à la communauté du jeu, à l’enseignement et à la recherche. Veuillez lire attentivement les conditions du [license agreement](LICENSE.md) avant toute utilisation.

## Pile technologique

- **Moteur** : moteur 3D propriétaire, principalement écrit en C++
- **Langage de script** : Lua 4.0
- **Animation** : système d’animation personnalisé
- **Vidéo** : Bink Video Technology ⚠️ *Licence commerciale — non incluse*
- **Son** : système audio FMOD ⚠️ *Licence commerciale — non incluse*
- **Graphismes** : DirectX 8

## Contenu de ce dépôt

- `Soft/` — code source et outils de développement
- `Complete/` — données et ressources du jeu
- `Data/` — données de configuration du jeu
- `Tools/` — outils de développement et de compilation
- `bin/` — exécutables compilés
- `cfg/` — fichiers de configuration
- `Versions/Current` — version développeur

---

# Lancement du jeu

## Lancement de base

1. Accédez au répertoire `bin/`.
2. Lancez l’exécutable du jeu, `Game.exe`.

Par défaut, le jeu se lance en mode fenêtré avec une résolution de 800x600. Pour le mode fenêtré, la profondeur de couleur dans Propriétés d’affichage/Paramètres/Couleurs doit être réglée sur 32 bits (true color). Pour exécuter le jeu en plein écran, utilisez le paramètre `-fullscreen`. Pour changer la résolution, utilisez les paramètres `-640`, `-1024` ou `-1280` pour 640x480, 1024x768 ou 1280x1024, respectivement.

⚠️ Il existe des problèmes d’exécution du jeu sur les systèmes d’exploitation modernes. Si vous trouvez une solution, merci de nous en informer via GitHub Issues.

---

# Éditeur de cartes et outils de développement

## Éditeur de cartes

- **Emplacement** : `bin/MapEdit.exe`

---

# Compilation du projet

## Prérequis de compilation

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

### ⚠️ Outils supplémentaires non inclus dans le code source :

- **Système audio FMOD**
- **Bink Video Technology**

### 📋 Licences des composants tiers :

- **STLport** — licence de type BSD
- **zlib** — licence zlib
- **Lua 4.0** — licence de type MIT
- **Ogg Vorbis** — licence de type BSD
