# 設計パターン

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
