# アーキテクチャ設計

## 基本方針
依存方向を一方向に制御するレイヤードアーキテクチャをベースに、
Game層の内部にECSを採用する。
また全レイヤーから使われる共通基盤としてCore層を設ける。

---

## 全体構造

```
┌──────────────────────────────────────────┐
│  Platform層                               │
│  Windows API実装                          │
│  ┌────────────────────────────────────┐  │
│  │  Infrastructure層                   │  │
│  │  DxLib・外部ライブラリへの依存を閉じ込める │  │
│  │  ┌──────────────────────────────┐  │  │
│  │  │  Game層                       │  │  │
│  │  │  ゲーム固有のロジック・ECS     │  │  │
│  │  └──────────────────────────────┘  │  │
│  └────────────────────────────────────┘  │
│  ┌────────────────────────────────────┐  │
│  │  Core層                             │  │
│  │  どのレイヤーからも使える共通基盤    │  │
│  └────────────────────────────────────┘  │
└──────────────────────────────────────────┘
```

依存方向（矢印の逆方向のインクルードは禁止）：
```
platform → infrastructure → game → core
```

- 外側の層は内側の層をインクルードしてよい
- 内側の層は外側の層をインクルードしてはいけない
- Core層はすべての層からインクルードされるが、自身はどの層もインクルードしない

---

## Web UI 層

### 概要

WebView2 上で動作する HTML / CSS / JS による UI 層。
C++ の層とは**別の実行環境（ブラウザランタイム）**であり、Platform 層が生成・制御する。
C++ の層を直接参照・インクルードすることはできない。

### 全体における位置づけ

```
┌─────────────────────────────────────────────────┐
│  Web UI 層（HTML / CSS / JS）                     │
│  ※ C++ の層に直接アクセスできない                 │
│  ※ Platform 層との JSON メッセージのみが唯一の接点 │
└─────────────────────────────────────────────────┘
               ↕  WebView2 postMessage（JSON）
┌─────────────────────────────────────────────────┐
│  Platform 層                                     │
│  ↓                                              │
│  Infrastructure 層 → Game 層 → Core 層           │
└─────────────────────────────────────────────────┘
```

Platform 層が WebView2 ウィンドウを生成・管理し、メッセージ送受信を担う。

### メッセージプロトコル

C++ ↔ Web UI の唯一の通信経路。JSON スキーマがこの境界における事実上のインターフェース定義になる。

```
C++ → JS : webView->PostMessage(json)        受信側: onMessageFromGame(data)
JS → C++ : sendToGame({ type, ... })         受信側: handleMessage(json)   ← messaging.js 経由
```

### フォルダ構成

```
web/
├── common/              ← 全ウィンドウから参照可能な共通基盤
│   ├── messaging.js     ← sendToGame() / onMessageFromGame() ブリッジ
│   └── common.css       ← 共通 CSS 変数・スタイル
├── desktop/             ← タスクバー・デスクトップ UI
├── job/                 ← 職業選択ウィンドウ
├── file/                ← ファイル選択ウィンドウ
├── param/               ← パラメータ表示ウィンドウ
└── difficulty/          ← 難易度選択ウィンドウ
```

各ウィンドウフォルダは `[name].html` / `[name].css` / `[name].js` の3ファイル構成。

### Web UI 層の規則

1. **C++ との通信は `messaging.js` 経由の JSON メッセージのみ**  
   `assets/` フォルダや外部ファイルへの直接アクセス禁止。データは C++ が送信した JSON から受け取る

2. **ウィンドウ間の直接参照禁止**  
   `job/` と `param/` はお互いを知らない。ウィンドウ間の連携は必ず C++ 経由

3. **`common/` は全ウィンドウから参照可**（逆方向は禁止）

4. **ゲームデータの数値を JS にハードコードしない**  
   `jobData.json` などの値は C++ から `init` / `refresh` メッセージで受け取る

5. **表示専用の静的データは例外的に許容**  
   アイコン文字列・ラベル文字列など、表示にのみ使用するものはこの限りではない

---

## 各レイヤーの責務

### Core層
どの層にも依存しない共通基盤。
「誰からでも参照されるが、自分は誰も参照しない層」。
EventBus・ServiceLocator・数学ユーティリティなど全レイヤーから使われるものを置く。
またgame層がinfrastructure層に直接依存しないためのインターフェースもここに置く。

### Game層
ゲームのルールとロジックのみを担当。
DxLibやWindows APIには直接触れない。
内部はECSで設計する。
Platformのデータはインターフェース経由で取得する。

### Infrastructure層
DxLibを用いた描画・入力・音声を担当。
「DxLibが変わったら影響を受けるもの」をすべてここに閉じ込める。
ゲームロジックは持たない。

### Platform層
Windows APIの処理をすべてここに閉じ込める。
インターフェースの実装クラスを置く。
「WindowsAPIが変わったら影響を受けるもの」がここに入る。

---

## 層の判断基準

迷ったときは「何が変わったら影響を受けるか」で判断する。

```
Core層           → どれが変わっても影響を受けない
Game層           → ゲームのルールが変わったら影響を受ける
Infrastructure層 → DxLib（外部ライブラリ）が変わったら影響を受ける
Platform層       → WindowsAPIが変わったら影響を受ける
```

---

## 依存関係の原則

### インクルードの禁止方向

```cpp
// ❌ coreがgameをインクルード
// ❌ gameがinfrastructureをインクルード
// ❌ infrastructureがplatformをインクルード
```

同じ層内での参照はOK：

```cpp
// ✅ game層内でgame層をインクルード
// game/scene/InGameScene.cpp
#include "game/system/MoveSystem.h"
```

### OS依存コードはインターフェース経由で利用する

```cpp
// NG: Game層でWindows APIを直接呼ぶ
#include <windows.h>
DWORD cpu = GetSystemTimes(...);

// OK: インターフェース経由で取得
ISystemDataProvider* provider = ServiceLocator::get<ISystemDataProvider>();
float cpu = provider->getCpuUsage();
```

インターフェースはCore層に置くことで、Game層もPlatform層も両方から参照できる：

```
platform/WindowsSystemProvider  → implements → core/ISystemDataProvider
game/GameScene                  → uses       → core/ISystemDataProvider
```

### infrastructure層への依存をインターフェースで切る

game層がinfrastructure層に直接依存しないようにインターフェースをcore層に置く。

```
infrastructure/ResourceManager  → implements → core/IResourceManager
game/Player                     → uses       → core/IResourceManager
```

---

## Game層内部：ECS設計

詳細は [patterns.md](patterns.md) を参照。

---

## イベント駆動：EventBus

詳細は [patterns.md](patterns.md) を参照。

---

## グローバルアクセス

Singleton基底クラスと ServiceLocator の設計・使い分けについては [patterns.md](patterns.md) を参照。

---

## クラス一覧

### Core層
| クラス名 | .cpp | 概要 |
|---|---|---|
| `EventBus` | あり | イベントの発行・購読管理 |
| `ServiceLocator` | あり | グローバルなサービスへのアクセス管理 |
| `IResourceManager` | なし | リソース管理インターフェース |
| `ISystemDataProvider` | なし | CPU・メモリ取得インターフェース |
| `IFileSystemProvider` | なし | ファイルシステム操作インターフェース |
| `IProcessProvider` | なし | プロセス情報取得インターフェース |

---

### Game層

#### ECS基盤
| クラス名 | .cpp | 概要 |
|---|---|---|
| `Entity` | なし | ただのID番号 |
| `EntityManager` | なし | EntityIdの発行・回収 |
| `IComponent` | なし | 全Componentの基底クラス |
| `ComponentArray` | なし | 1種類のComponentをEntityIdで管理 |
| `ComponentManager` | なし | 全ComponentArrayを型ごとに管理 |
| `ISystem` | なし | 全Systemの純粋仮想クラス |
| `SystemManager` | なし | Systemの管理・更新順序 |

#### Components
| クラス名 | .cpp | 概要 |
|---|---|---|
| `TransformComponent` | なし | 座標・回転 |
| `VelocityComponent` | なし | 速度 |
| `InputComponent` | なし | 入力状態 |
| `RenderComponent` | なし | 描画情報（モデルハンドル） |
| `HealthComponent` | なし | HP |
| `WeaponComponent` | あり | 武器情報・攻撃力計算 |
| `AIComponent` | なし | 敵AI情報・AIType |
| `BossComponent` | なし | ボスのフェーズ管理 |
| `ProcessComponent` | なし | プロセス名・種類 |

#### Systems
| クラス名 | 概要 |
|---|---|
| `InputSystem` | 入力処理 |
| `MoveSystem` | 移動処理 |
| `PhysicsSystem` | 物理処理 |
| `AISystem` | 敵AI処理・AITypeで分岐 |
| `BattleSystem` | 戦闘計算 |
| `RenderSystem` | 描画処理 |
| `ProcessSystem` | プロセス連携処理 |

#### シーン
| クラス名 | 概要 |
|---|---|
| `IScene` | シーンのインターフェース |
| `InGameScene` | インゲーム画面 |
| `TitleScene` | タイトル画面 |
| `FileSelectScene` | ファイル選択画面 |
| `LoadingScene` | ロード画面 |
| `ResultScene` | リザルト画面 |
| `ClearScene` | クリア画面 |

#### イベント
| ファイル名 | 概要 |
|---|---|
| `InGameEvents.h` | 全イベント定義（IGameEventを継承） |

定義されるイベント：
- `EnemyDefeatedEvent` 敵撃破時
- `PlayerDamagedEvent` プレイヤーダメージ時
- `RoomClearedEvent` 部屋クリア時
- `DungeonClearedEvent` ダンジョンクリア時

#### その他Game層
| クラス名 | 概要 |
|---|---|
| `Player` | PlayerのEntityセットアップ |
| `ObjectFactory` | ゲームオブジェクトの生成管理 |
| `GameManager` | ゲーム全体の進行管理 |
| `DungeonManager` | ダンジョン構造管理 |
| `Room` | 部屋の情報 |
| `WeaponFactory` | ファイル情報から武器を生成 |
| `EntityFactory` | EntityにComponentを組み合わせて生成する |
| `SystemInitializer` | Systemの登録・初期化順序を管理 |

---

### Infrastructure層
| クラス名 | 概要 |
|---|---|
| `Renderer` | 3D描画管理 |
| `Camera` | アイソメトリックカメラ制御 |
| `ResourceManager` | モデル・画像リソース管理 |
| `InputManager` | キー入力管理 |
| `SoundManager` | サウンド管理 |

---

### Platform層
| クラス名 | 概要 |
|---|---|
| `WindowsSystemProvider` | CPU・メモリ使用率取得 |
| `WindowsFileSystemProvider` | ファイル・フォルダ走査 |
| `WindowsProcessProvider` | プロセス列挙・レジストリ参照 |

---

## フォルダ構成

```
src/
├── core/
│   ├── ecs/
│   │   ├── ComponentArray.h
│   │   ├── ComponentManager.h  
│   │   ├── Entity.h
│   │   ├── EntityManager.h
│   │   ├── IComponent.h
│   │   ├── ISystem.h
│   │   └── SystemManager.h
│   ├── utility/
│   │   ├── LogUtil.h
│   │   └── LogUtil.cpp
│   ├── EventBus.h
│   ├── IResourceManager.h
│   ├── ISystemDataProvider.h
│   ├── IFileSystemProvider.h
│   ├── IProcessProvider.h
│   ├── ServiceLocator.h
│   ├── ServiceLocator.cpp
│   └── Vector3.h
├── game/
│   ├── actor/
│   │   ├── Player.h
│   │   └── Player.cpp
│   ├── component/
│   │   ├── TransformComponent.h
│   │   ├── RenderComponent.h
│   │   ├── VelocityComponent.h
│   │   ├── InputComponent.h
│   │   ├── HealthComponent.h
│   │   ├── WeaponComponent.h
│   │   ├── WeaponComponent.cpp
│   │   ├── AIComponent.h
│   │   ├── BossComponent.h
│   │   └── ProcessComponent.h
│   ├── system/
│   │   ├── InputSystem.h
│   │   ├── InputSystem.cpp
│   │   ├── MoveSystem.h
│   │   ├── MoveSystem.cpp
│   │   ├── PhysicsSystem.h
│   │   ├── PhysicsSystem.cpp
│   │   ├── AISystem.h
│   │   ├── AISystem.cpp
│   │   ├── BattleSystem.h
│   │   ├── BattleSystem.cpp
│   │   ├── RenderSystem.h
│   │   ├── RenderSystem.cpp
│   │   ├── ProcessSystem.h
│   │   └── ProcessSystem.cpp
│   ├── scene/
│   │   ├── IScene.h
│   │   ├── InGameScene.h
│   │   ├── InGameScene.cpp
│   │   ├── TitleScene.h
│   │   ├── TitleScene.cpp
│   │   ├── FileSelectScene.h
│   │   ├── FileSelectScene.cpp
│   │   ├── LoadingScene.h
│   │   ├── LoadingScene.cpp
│   │   ├── ResultScene.h
│   │   ├── ResultScene.cpp
│   │   ├── ClearScene.h
│   │   └── ClearScene.cpp
│   ├── event/
│   │   └── InGameEvents.h
│   ├── factory/
│   │   ├── EntityFactory.h
│   │   └── EntityFactory.cpp
│   ├── dungeon/
│   │   ├── DungeonManager.h
│   │   ├── DungeonManager.cpp
│   │   ├── Room.h
│   │   └── Room.cpp
│   ├── ObjectFactory.h
│   ├── ObjectFactory.cpp
│   ├── GameManager.h
│   └── GameManager.cpp
├── infrastructure/
│   ├── Renderer.h
│   ├── Renderer.cpp
│   ├── Camera.h
│   ├── Camera.cpp
│   ├── ResourceManager.h
│   ├── ResourceManager.cpp
│   ├── InputManager.h
│   ├── InputManager.cpp
│   ├── SoundManager.h
│   └── SoundManager.cpp
├── platform/
│   ├── WindowsSystemProvider.h
│   ├── WindowsSystemProvider.cpp
│   ├── WindowsFileSystemProvider.h
│   ├── WindowsFileSystemProvider.cpp
│   ├── WindowsProcessProvider.h
│   └── WindowsProcessProvider.cpp
├── ServiceLocatorInitializer.h
├── ServiceLocatorInitializer.cpp
└── Main.cpp
```

---

## 開発方針
最初からECSで作る。移行作業によるリスクを避けるため
オブジェクト指向からECSへの移行は行わない。

### 開発順序

**Step1: ECS基盤・Core層を作る**
- Entity
- EntityManager
- IComponent / ComponentArray / ComponentManager
- ISystem / SystemManager
- EventBus
- ServiceLocator

**Step2: 最小限のゲームを動かす**
- TransformComponent + MoveSystem でPlayerが動く
- HealthComponent + BattleSystem で攻撃が当たる

**Step3: 徐々に肉付けする**
- 敵AI
- ダンジョン生成
- Windows API連携
- 全シーン実装
- ボス実装

---

## 未決定事項
- SystemManagerの更新順序
- DungeonManagerの自動生成アルゴリズム
- Cameraのアイソメトリック実装方法
- SystemInitializerの具体的な設計
- 全ファイルのコメントをDoxygenコメントに統一する