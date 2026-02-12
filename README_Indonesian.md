[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm Trailer](Silent_Storm.png)](https://www.youtube.com/watch?v=ugRAaC7K_1I)

Permainan komputer [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) adalah RPG taktis berbasis giliran yang dikembangkan oleh [Nival Interactive](http://nival.com/) dan dirilis pada tahun 2003.

Permainan ini masih tersedia di [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) dan di [GOG.com](https://www.gog.com/game/silent_storm_gold).

Pada tahun 2026, kode sumber permainan ini dirilis di bawah [special license](LICENSE.md) yang melarang penggunaan komersial, tetapi sepenuhnya terbuka untuk komunitas gim, pendidikan, dan riset. Harap pelajari dengan saksama ketentuan [license agreement](LICENSE.md) sebelum menggunakannya.

## Teknologi yang digunakan

- **Engine**: Engine 3D proprieter, terutama ditulis dalam C++
- **Bahasa skrip**: Lua 4.0
- **Animasi**: Sistem animasi kustom
- **Video**: Bink Video Technology ⚠️ *Lisensi komersial — tidak disertakan*
- **Suara**: Sistem suara FMOD ⚠️ *Lisensi komersial — tidak disertakan*
- **Grafis**: DirectX 8

## Isi repositori ini

- `Soft/` — kode sumber dan alat pengembangan
- `Complete/` — data dan sumber daya gim
- `Data/` — data konfigurasi gim
- `Tools/` — alat pengembangan dan build
- `bin/` — berkas executable yang telah dikompilasi
- `cfg/` — berkas konfigurasi
- `Versions/Current` — versi pengembang

---

# Menjalankan Gim

## Peluncuran dasar

1. Masuk ke direktori `bin/`.
2. Jalankan berkas executable gim, `Game.exe`.

Secara bawaan, gim berjalan dalam mode jendela dengan resolusi 800x600. Untuk mode jendela, kedalaman warna di Display Properties/Settings/Colors harus disetel ke 32-bit (true color). Untuk menjalankan dalam mode layar penuh, gunakan parameter `-fullscreen`. Untuk mengubah resolusi, gunakan parameter `-640`, `-1024`, atau `-1280` masing-masing untuk 640x480, 1024x768, atau 1280x1024.

⚠️ Ada masalah saat menjalankan gim di sistem operasi modern. Jika Anda menemukan solusinya, harap beri tahu kami melalui GitHub Issues.

---

# Editor Peta dan Alat Pengembangan

## Editor Peta

- **Lokasi**: `bin/MapEdit.exe`

---

# Melakukan Build Proyek

## Persyaratan build

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

### ⚠️ Alat tambahan yang tidak disertakan dalam kode sumber:

- **Sistem Audio FMOD**
- **Bink Video Technology**

### 📋 Lisensi komponen pihak ketiga:

- **STLport** — lisensi mirip BSD
- **zlib** — Lisensi zlib
- **Lua 4.0** — lisensi mirip MIT
- **Ogg Vorbis** — lisensi mirip BSD
