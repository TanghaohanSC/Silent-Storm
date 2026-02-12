[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm Trailer](Silent_Storm.png)](https://www.youtube.com/watch?v=ugRAaC7K_1I)

O jogo de computador [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) é um RPG tático por turnos desenvolvido pela [Nival Interactive](http://nival.com/) e lançado em 2003.

O jogo ainda está disponível na [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) e na [GOG.com](https://www.gog.com/game/silent_storm_gold).

Em 2026, o código-fonte do jogo foi disponibilizado sob uma [special license](LICENSE.md) que proíbe o uso comercial, mas é completamente aberta para a comunidade do jogo, educação e pesquisa. Leia atentamente os termos do [license agreement](LICENSE.md) antes de utilizá-lo.

## Stack tecnológico

- **Engine**: engine 3D proprietária, escrita principalmente em C++
- **Linguagem de scripts**: Lua 4.0
- **Animação**: sistema de animação próprio
- **Vídeo**: Bink Video Technology ⚠️ *Licença comercial — não incluída*
- **Som**: sistema de som FMOD ⚠️ *Licença comercial — não incluída*
- **Gráficos**: DirectX 8

## O que há neste repositório

- `Soft/` — código-fonte e ferramentas de desenvolvimento
- `Complete/` — dados e recursos do jogo
- `Data/` — dados de configuração do jogo
- `Tools/` — ferramentas de desenvolvimento e de build
- `bin/` — executáveis compilados
- `cfg/` — arquivos de configuração
- `Versions/Current` — versão de desenvolvimento

---

# Execução do jogo

## Execução básica

1. Acesse o diretório `bin/`.
2. Execute o arquivo do jogo: `Game.exe`.

Por padrão, o jogo é executado em modo janela com resolução de 800x600. Para o modo janela, a profundidade de cor nas Propriedades de Vídeo (Display Properties/Settings/Colors) deve estar configurada para 32-bit (true color). Para executar em modo tela cheia, use o parâmetro `-fullscreen`. Para alterar a resolução, use os parâmetros `-640`, `-1024` ou `-1280` para 640x480, 1024x768 ou 1280x1024, respectivamente.

⚠️ Existem problemas ao executar o jogo em sistemas operacionais modernos. Se você encontrar uma solução, informe-nos por meio do GitHub Issues.

---

# Editor de mapas e ferramentas de desenvolvimento

## Editor de mapas

- **Localização**: `bin/MapEdit.exe`

---

# Build do projeto

## Requisitos para build

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

### ⚠️ Ferramentas adicionais não incluídas no código-fonte:

- **Sistema de áudio FMOD**
- **Tecnologia de vídeo Bink Video Technology**

### 📋 Licenças de componentes de terceiros:

- **STLport** — licença do tipo BSD
- **zlib** — licença zlib
- **Lua 4.0** — licença do tipo MIT
- **Ogg Vorbis** — licença do tipo BSD
