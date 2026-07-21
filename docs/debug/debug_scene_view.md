# シーンビュー（時間停止＋フリーカメラ）

Unityのシーンビューのように、**時を止めたままカメラだけを自由に動かして**当たり判定などを
厳密に確認できるデバッグ機能。ポーズ状態は `PauseManager`（`PauseReason::DebugSceneView`）で管理する。

## 操作方法

| キー | 動作 |
|---|---|
| **F2** | シーンビュー（時間停止）のON/OFF切り替え（InGameのみ） |
| **WASD / Space / Shift** | フリーカメラの移動（水平・上昇・下降） |
| **マウス** | 視点回転 |

- シーンビュー中は `InGame::update` がゲームロジック（SystemManager）を丸ごとスキップし、
  `DebugCameraSystem` だけを単独更新する。描画（`draw`）は毎フレーム動くため、
  当たり判定ギズモ（InGameViewの既存デバッグ可視化）は通常設定のまま表示される。
- 左上に `SceneView (Time Stopped)` ラベルが出る。
- F1のフリーカメラ（時間は動く）とは独立して動作する。

## リリース時の削除・復元一覧

> `PauseManager` / `Application` / ポーズメニュー自体は**製品機能なので削除しない**。
> 削除するのはシーンビュー（F2）関連のみ。

### 一部を削除するファイル

#### `src/game/PauseManager.h`
- `PauseReason::DebugSceneView` の列挙値（`// DEBUG:` コメント付き）を削除。

#### `src/core/input/KeyCode.h` / `src/infrastructure/InputManager.cpp`
- `F2`（`// DEBUG:` コメント付き）の行を削除。

#### `src/game/scene/InGame.h`
- `system::DebugCameraSystem` の前方宣言と `m_debugCameraSystem` メンバ（`// DEBUG:` コメント付き）を削除。

#### `src/game/scene/InGame.cpp`
- `update()` 内の F2 トグルブロックと、シーンビュー凍結ブロック
  （`isPausedBy(PauseReason::DebugSceneView)` の早期リターン）を削除。
- `setupSystems()` の `m_debugCameraSystem = ...` 登録（debug_camera.md 参照）を削除。

#### `src/game/system/DebugCameraSystem.*`
- ファイルごと削除（[debug_camera.md](debug_camera.md) 参照。`PauseManager&` 依存もここで消える）。

#### `src/game/scene/InGameView.h` / `.cpp`
- `PauseManager` の前方宣言・include・`m_pauseManager` メンバ・コンストラクタ引数
  （すべて `// DEBUG:` コメント付き）と、`drawDebugCameraLabel()` 内のシーンビュー分岐を削除。

## 削除後の確認

- `git grep -n "DebugSceneView"` が0件であること。
- Escのポーズメニュー（製品機能）が引き続き動作すること。
- Debug / Release 両方でビルドが通ること。
