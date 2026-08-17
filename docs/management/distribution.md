# 配布手順

企業へ渡す配布物の作り方をここに固定する。

---

## Release ビルドすれば揃う

`DxLib-3D.vcxproj` の `SyncRuntimeContent` ターゲットが、Release ビルドのたびに
`assets` / `web` / `ファイルはこちらからでも選べます` を `x64\Release\` へ同期する。
手でコピーする作業は不要。

```
x64\Release\
  WinVsMac.exe
  WebView2Loader.dll
  assets\
  web\
  ファイルはこちらからでも選べます\
```

このフォルダはダブルクリックでそのまま動く。

| 仕組み | 理由 |
|---|---|
| `robocopy /MIR` | 差分だけ写す。中身が同じなら何もしない（2回目以降は約0.3秒） |
| | 元で消したファイルは出力先からも消えるので、古い残骸が配布物に混ざらない |
| `AfterTargets="Build"` | ソースが変わらずアセットだけ差し替えたビルドでも確実に走る |
| Release 限定 | Debug は Visual Studio がプロジェクトフォルダをカレントにして起動するため複製がいらない |

同期するフォルダを足すときは、`vcxproj` 末尾の `RuntimeContentDir` に1行足す。

---

## なぜ実行ファイルの隣に置く必要があるのか

`assets` と `web` は **カレントディレクトリからの相対パス**で開いている
（`ResourceManager.cpp` の `RESOURCE_CONFIG_PATH`、`WebView2Host.cpp` の
`SetVirtualHostNameToFolderMapping`）。実行ファイルからの相対ではないため、
バラバラに置くと起動しない。

`WebView2Loader.dll` が無いと、セレクト画面・ローディング・リザルトが開かない。

---

## ZIPまで作る

`x64\Release\` には `.pdb` と `WinVsMac.exe.WebView2`（実行時に作られる
ユーザーデータ）も混ざる。これらを除いてZIPにするところまでやるなら：

```powershell
powershell -ExecutionPolicy Bypass -File tools\make_dist.ps1
```

| 生成物 | 内容 |
|---|---|
| `dist\WinVsMac\` | 配布物と同じ構成のフォルダ |
| `dist\WinVsMac_日付.zip` | 配布用ZIP（約42MB） |

| 指定 | 効果 |
|---|---|
| `-NoBuild` | ビルドを省略し、既存の実行ファイルで詰め直すだけにする |
| `-NoZip` | ZIPを作らず、フォルダの組み立てで止める |

`dist\` は `.gitignore` で除外している。
ZIP圧縮に20秒ほどかかるため、ビルドには含めていない。

除外するもの: `.pdb`、`*.exe.WebView2`、`game_log.txt` / `assert_log.txt`、
`settings.json`（無ければ既定値で起動する）、`desktop.ini` / `Thumbs.db`。

---

## 渡す前の確認

- [ ] `StageRepository.cpp` の `STAGE_DATA_PATH` が本番用になっているか
      （現在は `assets/data/stage-test.json` を指している）
- [ ] `WinVsMac.exe` をダブルクリックして起動するか
- [ ] `ファイルはこちらからでも選べます` の9ファイルが装備できるか
      （拡張子1つにつき1ファイル。9種類の能力と1対1で並べてある）

---

## 渡し方

**`.exe` / `.dll` / `.bat` を含むため、メール添付では届かない。**
ZIPの中に入っていても Gmail / Outlook がブロックする。
Google ドライブや OneDrive の共有リンクで渡すこと。

Release 構成は `/MT`（`RuntimeLibrary` = `MultiThreaded`）で静的リンクしているため、
Visual C++ 再頒布可能パッケージのインストールは不要。

WebView2 ランタイムは Windows 11 に標準で入っている。
Windows 10 の場合は入っていないことがあるため、
セレクト画面が真っ白になったらランタイムの導入を案内する。
