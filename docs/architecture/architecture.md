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

### ECSとは
- **Entity**：ただのID番号
- **Component**：データだけを持つ
- **System**：処理だけを行う

PlayerやEnemyという独立したクラスは存在しない。
EntityにComponentを組み合わせることで実体を表現する。

```cpp
// Playerの生成
EntityId player = entityManager.create();
componentManager.add<TransformComponent>(player, {});
componentManager.add<HealthComponent>(player, {});
componentManager.add<RenderComponent>(player, { handle });

// Enemyの生成（Componentの組み合わせが違うだけ）
EntityId enemy = entityManager.create();
componentManager.add<TransformComponent>(enemy, {});
componentManager.add<HealthComponent>(enemy, {});
componentManager.add<AIComponent>(enemy, {});
```

---

## イベント駆動：EventBus

SystemがSystemを直接呼ばないようにEventBusを使う。
EventBusはCore層に置くことでGame層・Infrastructure層の両方から使える。

```cpp
// BattleSystemがイベントを発行する（Game層）
eventBus.emit<EnemyDefeatedEvent>({ enemyId, "chrome.exe" });

// SoundManagerがイベントを受信する（Infrastructure層）
eventBus.subscribe<EnemyDefeatedEvent>([](const EnemyDefeatedEvent& e) {
    // 効果音を鳴らす
});
```

### イベントの定義
`IGameEvent`をマーカーとして継承し1ファイルにまとめて定義する。

```cpp
// game/event/InGameEvents.h
class IGameEvent {};

struct EnemyDefeatedEvent : public IGameEvent
{
    EntityId m_enemyId;
    std::string m_processName;
};

struct PlayerDamagedEvent : public IGameEvent
{
    int m_damage;
};

struct RoomClearedEvent : public IGameEvent
{
    int m_roomId;
};

struct DungeonClearedEvent : public IGameEvent {};
```

---

## グローバルアクセス

### Singleton基底クラス

ゲーム全体で**絶対に1つだけ存在すべき**クラスに使用する。
インスタンスの唯一性がコンパイル時に保証される。

**用途：**
- `SceneManager` — シーン管理（複製されてはいけない）
- `GameManager` — ゲーム全体の進行管理（複製されてはいけない）
- `InGameManager` — ゲーム進行中の管理（複製されてはいけない）
- `AudioManager` — 音声管理（複製されてはいけない）

**使用方法：**
```cpp
// 定義
class SceneManager : public core::base::Singleton<SceneManager> {
    friend core::base::Singleton<SceneManager>;
private:
    SceneManager() = default;
};

// 使用
auto& sceneManager = SceneManager::getInstance();
```

**重要ルール：**
- Singleton基底クラスを継承したクラスは、ServiceLocatorに登録してはいけない
- 理由：インスタンスの唯一性の保証とServiceLocatorの登録（複数インスタンス可能）が矛盾するため

---

### ServiceLocator

複数の場所から使われ、かつ**実装を差し替えたい**サービスを登録する。
テスト時に異なる実装に置き換え可能。

**用途：**
- `IInputProvider` — 入力処理（キーボード/ゲームパッド/別入力方式に差し替え可能）
- `IResourceManager` — リソース管理（複数実装が存在する可能性）
- `IJobProvider` — 職業情報提供（複数実装が存在する可能性）
- その他インターフェースベースのサービス

**使用方法：**
```cpp
// ServiceLocatorInitializerで登録する
ServiceLocator::provide<IInputProvider>(
    std::make_unique<KeyboardInputProvider>()
);

// どこからでも取り出せる
auto* inputProvider = ServiceLocator::get<IInputProvider>();
inputProvider->isKeyPressed(KeyCode::Space);

// テスト時は実装を差し替える
ServiceLocator::provide<IInputProvider>(
    std::make_unique<MockInputProvider>()
);
```

**重要ルール：**
- ServiceLocatorに登録するクラスはSingleton基底クラスを継承してはいけない
- 理由：複数の実装を持つ可能性があるため

---

### 使い分けの判断基準

| 観点 | Singleton基底クラス | ServiceLocator |
|---|---|---|
| **インスタンス数** | 絶対に1つ | 複数実装が存在する可能性 |
| **アクセス方法** | `ClassName::getInstance()` | `ServiceLocator::get<IInterface>()` |
| **目的** | インスタンス唯一性の保証 | 実装の差し替え可能性・テスト対応 |
| **例** | SceneManager, GameManager | IInputProvider, IResourceManager |
| **ServiceLocator登録** | ❌ 禁止 | ✅ 推奨 |



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