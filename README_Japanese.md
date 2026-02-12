[English](README.md)        [Русский](README_Russian.md)        [中文](README_Chinese.md)        [हिन्दी](README_Hindi.md)        [Español](README_Spanish.md)        [Français](README_French.md)        [Deutsch](README_German.md)        [Português](README_Portuguese.md)        [日本語](README_Japanese.md)        [Bahasa Indonesia](README_Indonesian.md)

[![Silent Storm Trailer](Silent_Storm.png)](https://www.youtube.com/watch?v=ugRAaC7K_1I)

コンピューターゲーム [Silent Storm](https://en.wikipedia.org/wiki/Silent_Storm) は、[Nival Interactive](http://nival.com/) によって開発され、2003年に発売されたターン制タクティカルRPGです。

このゲームは現在も [Steam](https://store.steampowered.com/app/254960/Silent_Storm_Gold_Edition/) および [GOG.com](https://www.gog.com/game/silent_storm_gold) で入手可能です。

2026年に、本作のソースコードが、商用利用を禁止しつつもコミュニティ、教育、研究目的には完全に開かれた [special license](LICENSE.md) の下で公開されました。利用する前に、必ず [license agreement](LICENSE.md) の条件をよく確認してください。

## 技術スタック

- **エンジン**: 主に C++ で記述された独自の 3D エンジン
- **スクリプト言語**: Lua 4.0
- **アニメーション**: 独自アニメーションシステム
- **ビデオ**: Bink Video Technology ⚠️ *商用ライセンス — 同梱されていません*
- **サウンド**: FMOD サウンドシステム ⚠️ *商用ライセンス — 同梱されていません*
- **グラフィックス**: DirectX 8

## このリポジトリに含まれるもの

- `Soft/` — ソースコードおよび開発ツール
- `Complete/` — ゲームデータおよびリソース
- `Data/` — ゲームの設定データ
- `Tools/` — 開発およびビルドツール
- `bin/` — コンパイル済み実行ファイル
- `cfg/` — 設定ファイル
- `Versions/Current` — 開発者向けバージョン

---

# ゲームの起動

## 基本的な起動方法

1. `bin/` ディレクトリに移動します。
2. ゲームの実行ファイル `Game.exe` を起動します。

デフォルトでは、ゲームは 800x600 の解像度でウィンドウモードとして起動します。ウィンドウモードで動作させるには、画面のプロパティ（Display Properties/Settings/Colors）で色数を 32-bit（トゥルーカラー）に設定しておく必要があります。フルスクリーンモードで実行するには、`-fullscreen` パラメータを使用します。解像度を変更するには、640x480、1024x768、1280x1024 に対してそれぞれ `-640`、`-1024`、`-1280` のパラメータを使用します。

⚠️ 現在のオペレーティングシステム上では、ゲームの実行に問題が発生する場合があります。解決方法を見つけた場合は、GitHub Issues を通じてお知らせください。

---

# マップエディタおよび開発ツール

## マップエディタ

- **場所**: `bin/MapEdit.exe`

---

# プロジェクトのビルド

## ビルドに必要なもの

- Microsoft Visual Studio .NET 2003
- DirectX 8.1 SDK

---

### ⚠️ ソースコードには含まれていない追加ツール:

- **FMOD Audio System**
- **Bink Video Technology**

### 📋 サードパーティコンポーネントのライセンス:

- **STLport** — BSD 互換ライセンス
- **zlib** — zlib ライセンス
- **Lua 4.0** — MIT 互換ライセンス
- **Ogg Vorbis** — BSD 互換ライセンス
