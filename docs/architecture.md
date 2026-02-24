# アーキテクチャ設計

## 基本方針
全体構造にクリーンアーキテクチャを採用し、
Game層の内部にECSを採用する。

---

## 全体構造：クリーンアーキテクチャ
```
┌─────────────────────────────────────┐
│  Platform層                          │
│  Windows API実装                     │
│  ┌───────────────────────────────┐  │
│  │  Engine層                      │  │
│  │  DxLib・シーン管理             │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │  Game層                  │  │  │
│  │  │  ┌──────────────────┐   │  │  │
│  │  │  │  ECS             │   │  │  │
│  │  │  │  Entity          │   │  │  │
│  │  │  │  Component       │   │  │  │
│  │  │  │  System          │   │  │  │
│  │  │  └──────────────────┘   │  │  │
│  │  └─────────────────────────┘  │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
```

依存方向：外側が内側に依存する。内側は外側を知らない。
Game層 ← Engine層 ← Platform層

---

## 各レイヤーの責務

### Game層
ゲームのルールとロジックのみを担当。
DxLibやWindows APIには直接触れない。
内部はECSで設計する。

### Engine層
DxLibを用いた描画・入力・シーン管理を担当。
ゲームロジックは持たない。

### Platform層
Windows APIの処理をすべてここに閉じ込める。
インターフェースの実装クラスを置く。

---

## 依存関係の原則

OS依存コードは直接Game層に書かない。
必ずインターフェース経由で利用する。
```cpp
// NG: Game層でWindows APIを直接呼ぶ
#include <windows.h>
DWORD cpu = GetSystemTimes(...);

// OK: インターフェース経由で取得
ISystemDataProvider* provider = ...;
float cpu = provider->getCpuUsage();
```

---

## Game層内部：ECS設計

### ECSとは
- **Entity**：ただのID番号
- **Component**：データだけを持つ
- **System**：処理だけを行う

### 各要素の役割
```cpp
// Entity: ただのID
using EntityId = uint32_t;

// Component: データだけ
struct HealthComponent
{
    int m_hp;
    int m_maxHp;
};

// System: 処理だけ
class MoveSystem
{
public:
    void update(TransformComponent& transform, float dx, float dy);
};
```

---

## フォルダ構成
```
src/
├── game/
│   ├── GameManager.h / .cpp
│   ├── interfaces/
│   │   ├── ISystemDataProvider.h
│   │   ├── IFileSystemProvider.h
│   │   └── IProcessProvider.h
│   ├── ecs/
│   │   ├── Entity.h
│   │   ├── ComponentManager.h / .cpp
│   │   └── SystemManager.h / .cpp
│   ├── components/
│   │   ├── TransformComponent.h
│   │   ├── HealthComponent.h
│   │   ├── WeaponComponent.h / .cpp
│   │   ├── AIComponent.h
│   │   ├── ProcessComponent.h
│   │   └── RenderComponent.h
│   ├── systems/
│   │   ├── MoveSystem.h / .cpp
│   │   ├── AISystem.h / .cpp
│   │   ├── BattleSystem.h / .cpp
│   │   ├── RenderSystem.h / .cpp
│   │   └── ProcessSystem.h / .cpp
│   └── dungeon/
│       ├── DungeonManager.h / .cpp
│       └── Room.h / .cpp
├── engine/
│   ├── scene/
│   │   ├── IScene.h
│   │   ├── SceneManager.h / .cpp
│   │   ├── TitleScene.h / .cpp
│   │   ├── FileSelectScene.h / .cpp
│   │   ├── LoadingScene.h / .cpp
│   │   ├── GameScene.h / .cpp
│   │   └── ResultScene.h / .cpp
│   ├── Renderer.h / .cpp
│   ├── InputManager.h / .cpp
│   ├── SoundManager.h / .cpp
│   └── Camera.h / .cpp
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

**Step1: ECS基盤を作る**
- Entity
- ComponentManager
- SystemManager

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
- ComponentManagerの具体的な実装方法
- SystemManagerの更新順序
- DungeonManagerの自動生成アルゴリズム
- Cameraのアイソメトリック実装方法