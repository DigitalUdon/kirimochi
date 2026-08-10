# Kirimochi

**切り抜き動画の制作を効率化するアプリ**

> [!NOTE]
> 現在開発中であるため環境依存の箇所が多く、動作に制約があります。

このアプリは切り抜き動画の制作を補助するためのものであり、自動作成ツールや動画編集ソフトではありません。

## できること

### 1. ブロック感覚でたたき台を作成

> 動画編集ソフトでとりあえず作り始めるが、構成や編集を何回もやり直してしまい時間がかかる...

小さなクリップをブロックのように並べ替えたり、SEを貼り付けるだけで簡単にたたき台を作成できます。

XMLファイルで出力して **Premiere Pro** や **DaVinci Resolve** にそのままインポート可能です。

### 2. 配信をまたぐクリップ管理

> 複数の配信をまたぐ切り抜きを作りたいが、クリップの管理が複雑で大変...

アプリ内で以下のワークフローが完結します。

```
配信のクリップをメモ → 複数の配信メモからタグ検索 → クリップを並べてたたき台の作成
```

## 開発状況

| バージョン | 状況 |
| --- | --- |
| **v0.1**(現在) | 最低限の機能の実装が完了し、実際に使用しながら改善点を洗い出し中 |
| ... | ... |

## Build

### 必要環境

- C++17
- Qt >= 6.10.3
- CMake

### 使用Qtライブラリ

- Qt Graphs
- Qt Multimedia
- Qt Positioning
- Qt Quick 3D
- Qt Quick Timeline
- Qt WebChannel
- Qt WebEngine

### Windows

```bash
cmake -S . -B build-debug -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.10.3\msvc2022_64" -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
"C:\Qt\6.10.3\msvc2022_64\bin\windeployqt6.exe" --qmldir build-debug\Kirimochi build-debug\Debug\appKirimochi.exe
```

### Linux

```bash
cmake -S . -B build-debug -DCMAKE_PREFIX_PATH="$HOME/Qt/6.11.1/gcc_64" -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```
