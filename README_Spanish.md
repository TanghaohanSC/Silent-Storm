[English](README_English.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm](https://img.shields.io/badge/Game-Silent_Storm-blue.svg)](https://es.wikipedia.org/wiki/Silent_Storm)
[![License: Custom](https://img.shields.io/badge/License-Nival_Ltd-yellow.svg)](../../LICENSE.md)
[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)

<p align="center">
    <a href="https://www.youtube.com/watch?v=NPkFx1h1HEI">
        <img src="Silent_Storm.png" width="320" alt="Silent Storm">
    </a>
</p>

El videojuego [Silent Storm](https://es.wikipedia.org/wiki/Silent_Storm) es un juego de estrategia/RPG táctico por turnos desarrollado por [Nival Interactive](http://nival.com/), lanzado en 2003.

El juego todavía está disponible en [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) y [GOG.com](https://www.gog.com/game/silent_storm_gold).

El código fuente del juego fue lanzado bajo una [licencia especial](../../LICENSE.md) de Nival International Ltd., que está completamente abierta para la comunidad, fines educativos e investigación. Por favor, revise cuidadosamente los términos del [acuerdo de licencia](../../LICENSE.md) antes de usar.

## Stack Tecnológico

- **Motor**: motor 3D propietario, principalmente escrito en C++
- **Lenguaje de scripting**: Lua 4.0
- **Animación**: Sistema de animación personalizado
- **Video**: Bink Video Technology ⚠️ *Licencia comercial - no incluida*
- **Sonido**: FMOD sound system ⚠️ *Licencia comercial - no incluida*
- **Gráficos**: DirectX 8

## Qué hay en este repositorio

- `Soft/` — código fuente y herramientas de desarrollo
- `Complete/` — datos y recursos del juego
- `Data/` — datos de configuración del juego
- `Tools/` — herramientas de desarrollo y compilación
- `bin/` — ejecutables compilados
- `cfg/` — archivos de configuración
- `Versions/Current` — versión para desarrolladores

---

# Ejecutar el Juego

## Ejecución Básica

1. Navegue al directorio `bin/`
2. Ejecute el archivo ejecutable del juego `Game.exe`

Por defecto, el juego se ejecuta en modo ventana a resolución 800x600. Para el modo ventana, la profundidad de color para el modo actual en Display Properties/Settings/Colors debe ser de 32 bits (truecolor). Para ejecutar en modo pantalla completa, use el parámetro –fullscreen. Para cambiar la resolución, use los parámetros –640, -1024, -1280 para las resoluciones 640x480, 1024x768, 1280x1024 respectivamente.

⚠️ Hay problemas al ejecutar en sistemas operativos modernos. Si encuentra una solución, por favor háganoslo saber a través de GitHub Issues.

---

# Editor de Mapas y Herramientas de Desarrollo

## Editor de Mapas

- **Ubicación**: `bin/MapEdit.exe`

---

# Compilar el Proyecto

## Requisitos de Compilación

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

## Información de Licencia

Este proyecto se lanza bajo una **licencia especial no comercial** de NIVAL INTERNATIONAL LTD.

### ⚠️ Herramientas adicionales no incluidas en el código fuente:

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 Licencias de componentes de terceros:

- **STLport** - Licencia tipo BSD
- **zlib** - Licencia zlib
- **Lua 4.0** - Licencia tipo MIT
- **Ogg Vorbis** - Licencia tipo BSD

Por favor, revise cuidadosamente el [acuerdo de licencia](../../LICENSE.md) completo antes de usar este código.

---

# Información Adicional

## Estado del Proyecto

Este es un **lanzamiento histórico del código fuente** de 2003. El código se proporciona tal cual para fines educativos, preservación y potencial desarrollo comunitario.

| Componente                              | Notas                                |
|-----------------------------------------|--------------------------------------|
| Compilación original (VS .NET 2003)     | Requiere herramientas originales     |
| Dependencias propietarias eliminadas    | FMOD, Bink requieren reemplazo       |

## Autores

**Desarrollo Original:** Nival Interactive (2001-2003)

## Soporte

- **Problemas:** Use GitHub Issues
- **Mejoras:** Si desea sugerir una corrección o nueva función, por favor cree un pull request
- **Comunidad:** Si eres propietario o participante activo de la comunidad del juego y estás interesado en el desarrollo del proyecto, por favor escribe a karim.kimsanbaev@gmail.com.

---

> **Nota:** Este es un lanzamiento histórico del código fuente. El juego todavía está disponible comercialmente. Por favor, apoye a los editores originales si disfrutó el juego.

**Silent Storm&trade;** es una marca registrada de sus propietarios. Este repositorio es solo para fines educativos y de preservación.

[![Blitzkrieg II Trailer](Blitzkrieg_2.png)](https://www.youtube.com/watch?v=Cw8rA2hvDGg)

El videojuego [Blitzkrieg 2] es la segunda entrega de la legendaria serie de juegos de estrategia en tiempo real, desarrollado por [Nival Interactive] y lanzado en 2005.

El juego sigue disponible en [Steam] y [GOG.com].

En 2025, el código fuente del juego fue publicado bajo una [special license] que prohíbe el uso comercial pero está completamente abierta para la comunidad, la educación y la investigación.  
Por favor, revise detenidamente los términos del [license agreement] antes de usarlo.

## Stack tecnológico

- **Motor de juego**: motor 3D propio, escrito principalmente en C++  
- **Lenguaje de scripts**: Lua  
- **Animación**: Granny Animation (RAD Game Tools) ⚠️ *Licencia comercial - no incluida*
- **Vídeo**: Bink Video Technology ⚠️ *Licencia comercial - no incluida*
- **Audio**: FMOD sound system ⚠️ *Licencia comercial - no incluida*  

## Qué contiene este repositorio

- `Complete` — datos y recursos del juego  
- `Design` — documentos de diseño y recursos artísticos  
- `Soft` — código fuente y herramientas de desarrollo  
- `Sound` — recursos de sonido del juego  
- `Tools` — herramientas de desarrollo y compilación  
- `Localizations` — archivos de localización
- `Versions` — diferentes configuraciones de compilación y entornos de prueba  
- `Versions/Temporary/Engine/Sources` — código fuente completo del motor del juego  

---

# Ejecución del juego

## Lanzamiento básico  
1. Vaya al directorio `Complete/bin/`  
2. Ejecute el archivo ejecutable del juego (si está disponible)  

---

# Editor de mapas e herramientas de desarrollo

## Editor de mapas  
- Ubicación: `Complete/Editor/`  
- Documentación: `Design/Manuals/MapEditorManual/`  
- Manual: `Design/Manuals/MapEditorManual/Final/`  
- FAQ: `Design/Manuals/MapEditorManual/FAQ/`  

## Herramientas de desarrollo  
- Plugins de Maya: `Tools/MayaScripts/`  
- Convertidores de texturas: `Tools/TexConv.exe`, `Tools/DxTex.exe`  
- Generador de fuentes: `Tools/FontGen.exe`  
- Herramientas de Granny: `Tools/Granny/`  

---


# Compilación del proyecto

## Requisitos de compilación  
- Microsoft Visual Studio (2003)  
- DirectX SDK  
- Dependencias adicionales indicadas en la documentación

---

## Información sobre licencias

Este proyecto se publica bajo una **licencia especial no comercial** de NIVAL INTERNATIONAL LTD.

### ✅ Lo que está incluido y es de código abierto:
- **Código fuente del motor del juego** - Licencia personalizada de NIVAL INTERNATIONAL LTD (solo uso no comercial)
- **Biblioteca de compresión zlib** - Licencia zlib (permisiva, uso comercial permitido)
- **Scripts, assets y datos del juego** - Licencia personalizada de NIVAL INTERNATIONAL LTD (solo uso no comercial)

### ⚠️ Herramientas adicionales no incluidas en el código fuente:
- **FMOD Audio System**
- **Bink Video Technology**
- **Granny3D Animation System**
- **Stingray Studio UI Components**
- **MySQL Database**
- **S3TC Texture Compression**

### 📋 Licencias de terceros:
- **zlib** (v1.1.3) - Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler - Licencia zlib

Consulte el [acuerdo de licencia](LICENSE.md) completo antes de usar este código.  

