[English](README_English.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm](https://img.shields.io/badge/Game-Silent_Storm-blue.svg)](https://pt.wikipedia.org/wiki/Silent_Storm)
[![License: Custom](https://img.shields.io/badge/License-Nival_Ltd-yellow.svg)](../../LICENSE.md)
[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)

<p align="center">
    <a href="https://www.youtube.com/watch?v=NPkFx1h1HEI">
        <img src="Silent_Storm.png" width="320" alt="Silent Storm">
    </a>
</p>

O jogo de computador [Silent Storm](https://pt.wikipedia.org/wiki/Silent_Storm) é um jogo de estratégia/RPG tático por turnos desenvolvido pela [Nival Interactive](http://nival.com/), lançado em 2003.

O jogo ainda está disponível na [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) e [GOG.com](https://www.gog.com/game/silent_storm_gold).

O código-fonte do jogo foi lançado sob uma [licença especial](../../LICENSE.md) da Nival International Ltd., que está totalmente aberta para a comunidade, fins educacionais e de pesquisa. Por favor, revise cuidadosamente os termos do [acordo de licença](../../LICENSE.md) antes de usar.

## Stack Tecnológico

- **Motor**: motor 3D proprietário, principalmente escrito em C++
- **Linguagem de script**: Lua 4.0
- **Animação**: Sistema de animação personalizado
- **Vídeo**: Bink Video Technology ⚠️ *Licença comercial - não incluída*
- **Som**: FMOD sound system ⚠️ *Licença comercial - não incluída*
- **Gráficos**: DirectX 8

## O que está neste repositório

- `Soft/` — código-fonte e ferramentas de desenvolvimento
- `Complete/` — dados e recursos do jogo
- `Data/` — dados de configuração do jogo
- `Tools/` — ferramentas de desenvolvimento e compilação
- `bin/` — executáveis compilados
- `cfg/` — arquivos de configuração
- `Versions/Current` — versão para desenvolvedores

---

# Executar o Jogo

## Execução Básica

1. Navegue até o diretório `bin/`
2. Execute o arquivo executável do jogo `Game.exe`

Por padrão, o jogo é executado em modo janela na resolução 800x600. Para o modo janela, a profundidade de cor para o modo atual em Display Properties/Settings/Colors deve ser de 32 bits (truecolor). Para executar em modo tela cheia, use o parâmetro –fullscreen. Para alterar a resolução, use os parâmetros –640, -1024, -1280 para as resoluções 640x480, 1024x768, 1280x1024 respectivamente.

⚠️ Há problemas ao executar em sistemas operacionais modernos. Se você encontrar uma solução, por favor nos informe através do GitHub Issues.

---

# Editor de Mapas e Ferramentas de Desenvolvimento

## Editor de Mapas

- **Localização**: `bin/MapEdit.exe`

---

# Compilar o Projeto

## Requisitos de Compilação

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

## Informações de Licença

Este projeto é lançado sob uma **licença especial não comercial** da NIVAL INTERNATIONAL LTD.

### ⚠️ Ferramentas adicionais não incluídas no código-fonte:

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 Licenças de componentes de terceiros:

- **STLport** - Licença tipo BSD
- **zlib** - Licença zlib
- **Lua 4.0** - Licença tipo MIT
- **Ogg Vorbis** - Licença tipo BSD

Por favor, revise cuidadosamente o [acordo de licença](../../LICENSE.md) completo antes de usar este código.

---

# Informações Adicionais

## Status do Projeto

Este é um **lançamento histórico do código-fonte** de 2003. O código é fornecido como está para fins educacionais, preservação e potencial desenvolvimento da comunidade.

| Componente                                  | Notas                                |
|---------------------------------------------|--------------------------------------|
| Compilação original (VS .NET 2003)          | Requer ferramentas originais         |
| Dependências proprietárias removidas        | FMOD, Bink requerem substituição     |

## Autores

**Desenvolvimento Original:** Nival Interactive (2001-2003)

## Suporte

- **Problemas:** Use GitHub Issues
- **Melhorias:** Se você deseja sugerir uma correção ou novo recurso, por favor crie um pull request

---

> **Nota:** Este é um lançamento histórico do código-fonte. O jogo ainda está disponível comercialmente. Por favor, apoie os editores originais se você gostou do jogo.

**Silent Storm&trade;** é uma marca registrada de seus proprietários. Este repositório é apenas para fins educacionais e de preservação.

