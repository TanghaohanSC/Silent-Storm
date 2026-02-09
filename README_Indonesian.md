[English](README_English.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm](https://img.shields.io/badge/Game-Silent_Storm-blue.svg)](https://en.wikipedia.org/wiki/Silent_Storm)
[![License: Custom](https://img.shields.io/badge/License-Nival_Ltd-yellow.svg)](../../LICENSE.md)
[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)

<p align="center">
    <a href="https://www.youtube.com/watch?v=NPkFx1h1HEI">
        <img src="Silent_Storm.png" width="320" alt="Silent Storm">
    </a>
</p>

Permainan komputer [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) adalah game strategi/RPG taktis berbasis giliran yang dikembangkan oleh [Nival Interactive](http://nival.com/), dirilis pada tahun 2003.

Game ini masih tersedia di [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) dan [GOG.com](https://www.gog.com/game/silent_storm_gold).

Kode sumber game telah dirilis di bawah [lisensi khusus](../../LICENSE.md) dari Nival International Ltd., yang sepenuhnya terbuka untuk komunitas, tujuan pendidikan dan penelitian. Harap tinjau dengan cermat ketentuan [perjanjian lisensi](../../LICENSE.md) sebelum menggunakan.

## Stack Teknologi

- **Engine**: engine 3D proprietary, terutama ditulis dalam C++
- **Bahasa scripting**: Lua 4.0
- **Animasi**: Sistem animasi kustom
- **Video**: Bink Video Technology ⚠️ *Lisensi komersial - tidak termasuk*
- **Suara**: FMOD sound system ⚠️ *Lisensi komersial - tidak termasuk*
- **Grafis**: DirectX 8

## Apa yang ada di repositori ini

- `Soft/` — kode sumber dan alat pengembangan
- `Complete/` — data dan sumber daya game
- `Data/` — data konfigurasi game
- `Tools/` — alat pengembangan dan build
- `bin/` — file executable yang dikompilasi
- `cfg/` — file konfigurasi
- `Versions/Current` — versi pengembang

---

# Menjalankan Game

## Peluncuran Dasar

1. Navigasi ke direktori `bin/`
2. Jalankan file executable game `Game.exe`

Secara default, game berjalan dalam mode windowed pada resolusi 800x600. Untuk mode windowed, kedalaman warna untuk mode saat ini di Display Properties/Settings/Colors harus 32 bit (truecolor). Untuk menjalankan dalam mode layar penuh, gunakan parameter –fullscreen. Untuk mengubah resolusi, gunakan parameter –640, -1024, -1280 untuk resolusi 640x480, 1024x768, 1280x1024 masing-masing.

⚠️ Ada masalah saat menjalankan pada sistem operasi modern. Jika Anda menemukan solusi, harap beri tahu kami melalui GitHub Issues.

---

# Editor Peta dan Alat Pengembangan

## Editor Peta

- **Lokasi**: `bin/MapEdit.exe`

---

# Membangun Proyek

## Persyaratan Build

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

## Informasi Lisensi

Proyek ini dirilis di bawah **lisensi khusus non-komersial** dari NIVAL INTERNATIONAL LTD.

### ⚠️ Alat tambahan yang tidak termasuk dalam kode sumber:

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 Lisensi komponen pihak ketiga:

- **STLport** - Lisensi mirip BSD
- **zlib** - Lisensi zlib
- **Lua 4.0** - Lisensi mirip MIT
- **Ogg Vorbis** - Lisensi mirip BSD

Harap tinjau dengan cermat [perjanjian lisensi](../../LICENSE.md) lengkap sebelum menggunakan kode ini.

---

# Informasi Tambahan

## Status Proyek

Ini adalah **rilis kode sumber historis** dari tahun 2003. Kode disediakan apa adanya untuk tujuan pendidikan, pelestarian, dan pengembangan komunitas potensial.

| Komponen                          | Catatan                          |
|-----------------------------------|----------------------------------|
| Build asli (VS .NET 2003)         | Memerlukan alat asli             |
| Dependensi proprietary dihapus    | FMOD, Bink memerlukan pengganti  |

## Penulis

**Pengembangan Asli:** Nival Interactive (2001-2003)

## Dukungan

- **Masalah:** Gunakan GitHub Issues
- **Perbaikan:** Jika Anda ingin menyarankan perbaikan atau fitur baru, silakan buat pull request

---

> **Catatan:** Ini adalah rilis kode sumber historis. Game masih tersedia secara komersial. Harap dukung penerbit asli jika Anda menikmati game ini.

**Silent Storm&trade;** adalah merek dagang dari pemiliknya. Repositori ini hanya untuk tujuan pendidikan dan pelestarian.

