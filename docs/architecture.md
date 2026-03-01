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

### Game層
ゲームのルールとロジックのみを担当。
DxLibやWindows APIには直接触れない。
内部はECSで設計する。
Platformのデータはインターフェース経由で取得する。

### Infrastructure層
DxLibを用いた描画・入力・音声・シーン管理を担当。
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
// game/scene/GameScene.cpp
#include "game/ecs/system/MoveSystem.h"
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
componentManager.add<TransformComponent>(player, 0.0f, 0.0f, 0.0f);
componentManager.add<HealthComponent>(player, 100, 100);
componentManager.add<WeaponComponent>(player, weaponData);

// Enemyの生成（Componentの組み合わせが違うだけ）
EntityId enemy = entityManager.create();
componentManager.add<TransformComponent>(enemy, 5.0f, 3.0f, 0.0f);
componentManager.add<HealthComponent>(enemy, 120, 120);
componentManager.add<AIComponent>(enemy, AIType::Heavy, 0.5f);
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
// game/events/InGameEvent.h
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

## グローバルアクセス：ServiceLocator

ServiceLocatorはシングルトンの倉庫。
ゲーム全体で1つだけ存在するManagerクラスを預けて
どこからでも取り出せる仕組み。
Core層に置くことでGame層・Infrastructure層の両方から使える。

```cpp
// Main.cppで全部預ける
ServiceLocator::provide(new InputManager());
ServiceLocator::provide(new SoundManager());
ServiceLocator::provide(new Renderer());

// どこからでも取り出せる
ServiceLocator::get<SoundManager>()->play("attack");
ServiceLocator::get<InputManager>()->getKey();
```

---

## クラス一覧

### Core層
| クラス名 | .cpp | 概要 |
|---|---|---|
| `EventBus` | あり | イベントの発行・購読管理 |
| `ServiceLocator` | あり | グローバルなサービスへのアクセス管理 |
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
| `HealthComponent` | なし | HP |
| `WeaponComponent` | あり | 武器情報・攻撃力計算 |
| `AIComponent` | なし | 敵AI情報・AIType |
| `BossComponent` | なし | ボスのフェーズ管理 |
| `ProcessComponent` | なし | プロセス名・種類 |
| `RenderComponent` | なし | 描画情報 |

#### Systems
| クラス名 | 概要 |
|---|---|
| `MoveSystem` | 移動処理 |
| `AISystem` | 敵AI処理・AITypeで分岐 |
| `BattleSystem` | 戦闘計算 |
| `RenderSystem` | 描画処理 |
| `ProcessSystem` | プロセス連携処理 |

#### イベント
| ファイル名 | 概要 |
|---|---|
| `InGameEvent.h` | 全イベント定義（IGameEventを継承） |

定義されるイベント：
- `EnemyDefeatedEvent` 敵撃破時
- `PlayerDamagedEvent` プレイヤーダメージ時
- `RoomClearedEvent` 部屋クリア時
- `DungeonClearedEvent` ダンジョンクリア時

#### その他Game層
| クラス名 | 概要 |
|---|---|
| `GameManager` | ゲーム全体の進行管理 |
| `DungeonManager` | ダンジョン構造管理 |
| `Room` | 部屋の情報 |
| `WeaponFactory` | ファイル情報から武器を生成 |
| `EntityFactory` | EntityにComponentを組み合わせて生成する |
| `SystemInitializer` | Systemの登録・初期化順序を管理 |

---

### Infrastructure層

#### シーン
| クラス名 | 概要 |
|---|---|
| `IScene` | シーンのインターフェース |
| `SceneManager` | シーン遷移管理 |
| `TitleScene` | タイトル画面 |
| `FileSelectScene` | ファイル選択画面 |
| `LoadingScene` | ロード画面 |
| `GameScene` | ゲーム画面 |
| `ResultScene` | リザルト画面 |
| `ClearScene` | クリア画面 |

#### その他Infrastructure層
| クラス名 | 概要 |
|---|---|
| `Renderer` | 3D描画管理 |
| `InputManager` | キー入力管理 |
| `SoundManager` | サウンド管理 |
| `Camera` | アイソメトリックカメラ制御 |
| `ResourceManager` | モデル・画像リソース管理 |

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
│   ├── provider/
│   │   ├── ISystemDataProvider.h
│   │   ├── IFileSystemProvider.h
│   │   └── IProcessProvider.h
│   ├── utility/
│   │   ├── LogUtil.h
│   │   └── LogUtil.cpp
│   ├── EventBus.h
│   ├── ServiceLocator.h / .cpp
│   └── Vector3.h
├── game/
│   ├── actor/
│   │   └── Player.h / .cpp
│   ├── component/
│   │   ├── TransformComponent.h
│   │   ├── RenderComponent.h
│   │   ├── VelocityComponent.h
│   │   ├── InputComponent.h
│   │   ├── HealthComponent.h
│   │   ├── WeaponComponent.h / .cpp
│   │   ├── AIComponent.h
│   │   ├── BossComponent.h
│   │   └── ProcessComponent.h
│   ├── system/
│   │   ├── MoveSystem.h / .cpp
│   │   ├── InputSystem.h / .cpp
│   │   ├── PhysicsSystem.h / .cpp
│   │   ├── AISystem.h / .cpp
│   │   ├── BattleSystem.h / .cpp
│   │   ├── RenderSystem.h / .cpp
│   │   └── ProcessSystem.h / .cpp
│   ├── scene/
│   │   ├── IScene.h
│   │   ├── InGameScene.h / .cpp
│   │   ├── TitleScene.h / .cpp
│   │   ├── FileSelectScene.h / .cpp
│   │   ├── LoadingScene.h / .cpp
│   │   ├── ResultScene.h / .cpp
│   │   └── ClearScene.h / .cpp
│   ├── event/
│   │   └── InGameEvents.h
│   ├── factory/
│   │   └── EntityFactory.h / .cpp
│   ├── dungeon/
│   │   ├── DungeonManager.h / .cpp
│   │   └── Room.h / .cpp
│   ├── ObjectFactory.h / .cpp
│   └── GameManager.h / .cpp
├── infrastructure/
│   ├── Renderer.h / .cpp
│   ├── Camera.h / .cpp
│   ├── ResourceManager.h / .cpp
│   ├── InputManager.h / .cpp
│   └── SoundManager.h / .cpp
├── platform/
│   ├── WindowsSystemProvider.h / .cpp
│   ├── WindowsFileSystemProvider.h / .cpp
│   └── WindowsProcessProvider.h / .cpp
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