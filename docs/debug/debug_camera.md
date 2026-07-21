# デバッグカメラ（フリーカメラ）

マイクラのクリエイティブのように、カメラを自由に飛ばして多方向から確認できるデバッグ機能。
状態は `GameManager` が `m_debugMode` として保持し、それに応じて通常カメラと切り替える。

## 追加コミット

- `485cd9132035cfaff2846205bf476660bcd83ea2`（`feat:デバックの用のカメラモードを追加`）

## 操作方法

| キー | 動作 |
|---|---|
| **F1** | デバッグモードのON/OFF切り替え |
| **WASD** | フリーカメラの水平移動（カメラ相対） |
| **Space / Shift** | フリーカメラの上昇 / 下降 |
| **マウス** | 視点回転（yaw/pitch） |
| **矢印キー ↑↓←→** | プレイヤー移動（カメラの向きを基準に移動） |

- デバッグ中は WASD／Space／Shift／マウスをカメラが使うため、プレイヤーは矢印キーのみで操作する。
- デバッグモードに入った瞬間は、通常カメラの位置・向きを引き継いでシームレスに開始する。

## 仕組みの概要

- `DebugCameraSystem` がデバッグ中のみカメラを制御する（自己完結。フリーカメラの状態を自身で保持）。
- `CameraSystem`（通常追従）はデバッグ中は早期リターンして制御を譲る。
- `InputSystem` はデバッグ中は WASD をプレイヤー移動から除外し、矢印キーのみ有効にする。

---

## リリース時の削除・復元一覧

> コード中のデバッグ箇所はすべて `DEBUG:` コメント付き。`git grep -n "DEBUG:"` でも確認できる。
> 行番号はコミット `485cd91` 時点の目安（編集で前後する）。

### 1. ファイルごと削除

| ファイル | 内容 |
|---|---|
| `src/game/system/DebugCameraSystem.h` | フリーカメラSystem（ヘッダ）全体 |
| `src/game/system/DebugCameraSystem.cpp` | フリーカメラSystem（実装）全体 |

### 2. プロジェクトファイルから登録を削除

| ファイル | 削除するエントリ |
|---|---|
| `DxLib-3D.vcxproj` | `DebugCameraSystem.cpp`（`<ClCompile>`）と `DebugCameraSystem.h`（`<ClInclude>`）の2行 |
| `DxLib-3D.vcxproj.filters` | 同上 `DebugCameraSystem.cpp` / `.h` の2エントリ |

### 3. 一部を削除・元に戻すファイル

> 注：脱シングルトンリファクタ以降、デバッグモード状態は `GameManager&` の**コンストラクタ注入**で
> 参照している。削除時は「DEBUGコメント付きの引数・メンバ」も一緒に剥がすこと。

#### `src/game/GameManager.h`
- `// DEBUG: ここからデバッグモード関連` 〜 `// DEBUG: ここまでデバッグモード関連` のブロック（`toggleDebugMode()` / `isDebugMode()`）を削除。
- private の `// DEBUG: デバッグモードの状態` と `bool m_debugMode{ false };` を削除。

#### `src/core/input/KeyCode.h`
- `F1, // DEBUG: ...` の行を削除（F2は [debug_scene_view.md](debug_scene_view.md) 参照）。
- ※ `R`（`// DEBUG: デバッグ用（現在未使用）`）は本機能で追加したものではないため任意。

#### `src/infrastructure/InputManager.cpp`
- `KEY_MAP` 内の `{ core::input::KeyCode::F1, KEY_INPUT_F1 }, // DEBUG: ...` の行を削除。

#### `src/game/system/CameraSystem.h` / `.cpp`
- `GameManager` の前方宣言・コンストラクタ引数・`m_gameManager` メンバ（すべて `// DEBUG:` コメント付き）を削除。
- `update()` 先頭の `// DEBUG: ...` 早期リターンブロック（`if (m_gameManager.isDebugMode()) return;`）と
  `#include "game/GameManager.h"` を削除。
- 呼び出し側（InGame.cpp の registerSystem）の `m_gameManager` 引数も削除。

#### `src/game/system/InputSystem.h` / `.cpp`
- `GameManager` の前方宣言・コンストラクタ引数・`m_gameManager` メンバ（すべて `// DEBUG:` コメント付き）を削除。
- `// DEBUG: デバッグモード中は...` の `const bool debugMode` / `const bool allowWasd` ブロックを削除。
- 移動キーの条件を元に戻す（`allowWasd &&` を外す）。
- 呼び出し側（InGame.cpp の registerSystem）の `m_gameManager` 引数も削除。

#### `src/game/scene/InGame.cpp`
- `#include "game/system/DebugCameraSystem.h" // DEBUG: ...` を削除。
- `setupSystems()` 内の `m_debugCameraSystem = m_systemManager.registerSystem<DebugCameraSystem>(...)`（`// DEBUG:` コメント付き）を削除。
- `update()` 先頭の `// DEBUG: F1キーで...` トグルブロック（`isKeyPressed(F1)` の if 全体）を削除。

#### `src/game/scene/InGameView.h` / `.cpp`
- `GameManager` の前方宣言・コンストラクタ引数・`m_gameManager` メンバ（すべて `// DEBUG:` コメント付き）を削除。
- `draw()` 内の `drawDebugCameraLabel();` 呼び出しと、`drawDebugCameraLabel()` の宣言・定義を削除。
- `#include "game/GameManager.h"` を削除。
- 呼び出し側（InGame.cpp の `m_view{...}` 初期化）の `gameManager` 引数も削除。

---

## 削除後の確認

- `git grep -n "DEBUG:"` にデバッグカメラ関連が残っていないこと。
- `DebugCamera` / `isDebugMode` / `m_debugMode` / `KeyCode::F1` の参照が残っていないこと。
- Debug / Release 両方でビルドが通ること。
