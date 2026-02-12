[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm Trailer](Silent_Storm.png)](https://www.youtube.com/watch?v=ugRAaC7K_1I)

El videojuego para ordenador [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) es un RPG táctico por turnos desarrollado por [Nival Interactive](http://nival.com/) y lanzado en 2003.

El juego sigue estando disponible en [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) y en [GOG.com](https://www.gog.com/game/silent_storm_gold).

En 2026, el código fuente del juego se publicó bajo una [special license](LICENSE.md) que prohíbe el uso comercial, pero está completamente abierta para la comunidad del juego, la educación y la investigación. Por favor, revise detenidamente los términos del [license agreement](LICENSE.md) antes de usarlo.

## Pila tecnológica

- **Motor**: motor 3D propietario, escrito principalmente en C++
- **Lenguaje de scripts**: Lua 4.0
- **Animación**: sistema de animación propio
- **Vídeo**: Bink Video Technology ⚠️ *Licencia comercial — no incluida*
- **Sonido**: sistema de sonido FMOD ⚠️ *Licencia comercial — no incluida*
- **Gráficos**: DirectX 8

## Qué contiene este repositorio

- `Soft/` — código fuente y herramientas de desarrollo
- `Complete/` — datos y recursos del juego
- `Data/` — datos de configuración del juego
- `Tools/` — herramientas de desarrollo y de compilación
- `bin/` — ejecutables compilados
- `cfg/` — archivos de configuración
- `Versions/Current` — versión para desarrolladores

---

# Ejecución del juego

## Ejecución básica

1. Vaya al directorio `bin/`.
2. Ejecute el archivo ejecutable del juego, `Game.exe`.

De forma predeterminada, el juego se ejecuta en modo ventana con una resolución de 800x600. Para el modo ventana, la profundidad de color en Propiedades de pantalla/Configuración/Colores debe estar establecida en 32 bits (color verdadero). Para ejecutar el juego en modo de pantalla completa, use el parámetro `-fullscreen`. Para cambiar la resolución, use los parámetros `-640`, `-1024` o `-1280` para 640x480, 1024x768 o 1280x1024, respectivamente.

⚠️ Existen problemas al ejecutar el juego en sistemas operativos modernos. Si encuentra una solución, por favor infórmenos a través de GitHub Issues.

---

# Editor de mapas y herramientas de desarrollo

## Editor de mapas

- **Ubicación**: `bin/MapEdit.exe`

---

# Compilación del proyecto

## Requisitos de compilación

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

### ⚠️ Herramientas adicionales no incluidas en el código fuente:

- **Sistema de audio FMOD**
- **Bink Video Technology**

### 📋 Licencias de componentes de terceros:

- **STLport** — licencia tipo BSD
- **zlib** — licencia zlib
- **Lua 4.0** — licencia tipo MIT
- **Ogg Vorbis** — licencia tipo BSD
