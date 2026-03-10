# Refactoring
---

## 修正項目

### 1. public関数へのDoxygenコメント追加 🔴 **必須**

#### 現状の問題
- 一部のクラスにのみコメントがあり、統一されていない
- Visual Studioでホバー時にガイドが表示されない関数が多数存在
- 他者（就活時の評価者）が見たときに理解しづらい

#### 対象ファイル
- `src/game/ObjectFactory.h` - `init()`, `getPlayer()`
- `src/game/actor/Player.h` - `getId()`
- `src/game/stage/Ground.h` - `getId()`
- `src/game/system/*.h` - すべてのSystem クラス
- `src/core/ecs/EntityManager.h` - `create()`, `destroy()`
- `src/core/ecs/SystemManager.h` - `registerSystem()`, `update()`
- `src/core/ecs/ComponentManager.h` - 全てのpublicメソッド
- `src/infrastructure/*.h` - すべてのpublicメソッド

#### 実装方針
以下のDoxygenフォーマットに統一：
```cpp
/**
 * @brief 簡潔な1行説明
 * @param paramName パラメータの説明
 * @return 戻り値の説明（void以外）
 */
```

---

### 2. ObjectFactory の肥大化対策（Factory Pattern導入） 🟡 **重要**

#### 現状の問題
```cpp
// 現在：オブジェクトが増えるたびに引数が増殖
void init(int playerModelhandle, const data::PlayerData& playerData);

// 将来的な悪い例
void init(
    int playerModelHandle, const PlayerData& playerData,
    int enemyModelHandle, const EnemyData& enemyData,
    int itemModelHandle, const ItemData& itemData,
    ...  // 際限なく増える
);
```

#### 設計方針
**IFactory + 個別Factory + FactoryManager (Facade)** パターンを導入

```
src/game/factory/
├─ IFactory.h                    // 純粋仮想インターフェース
├─ PlayerFactory.h / .cpp        // Player生成専用
├─ GroundFactory.h / .cpp        // Ground生成専用
├─ EnemyFactory.h / .cpp         // (将来) Enemy生成専用
└─ FactoryManager.h / .cpp       // Facade: 全Factoryを統括
```

#### クラス設計
```cpp
// IFactory.h
class IFactory {
public:
    virtual ~IFactory() = default;
    virtual void create() = 0;
};

// PlayerFactory.h
class PlayerFactory : public IFactory {
public:
    PlayerFactory(EntityManager&, ComponentManager&, IResourceManager&);
    void create() override;
    Player& getPlayer() const;
private:
    std::unique_ptr<Player> m_player;
};

// FactoryManager.h (Facade)
class FactoryManager {
public:
    FactoryManager(...);
    void initialize();  // 全オブジェクトを生成
    
    PlayerFactory& getPlayerFactory();
    GroundFactory& getGroundFactory();
private:
    std::unique_ptr<PlayerFactory> m_playerFactory;
    std::unique_ptr<GroundFactory> m_groundFactory;
};
```

#### InGameScene での使用例
```cpp
// Before
m_objectFactory.init(playerModelHandle, m_playerData);
auto& player = m_objectFactory.getPlayer();

// After
m_factoryManager.initialize();  // 全オブジェクト生成完了
auto& player = m_factoryManager.getPlayerFactory().getPlayer();
```

#### 注意点
- 完全に `initialize()` だけで全てを完結させるのは難しい場合がある
- オブジェクト間の依存関係（例：EnemyがPlayerの位置を参照）を考慮する
- 各Factoryに`create()`メソッドを用意し、明示的に呼び出す方式も検討

#### 実装順序
1. `IFactory.h` インターフェース作成
2. `PlayerFactory` 実装（既存の`ObjectFactory`から移植）
3. `GroundFactory` 実装
4. `FactoryManager` 実装（Facade）
5. `InGameScene` のリファクタリング
6. 旧 `ObjectFactory` 削除

---

### 3. メンバ変数の初期化統一 🟢 **推奨**

#### 現状の問題
- 初期化方法が統一されていない
- 未初期化によるバグのリスク

#### 統一ルール

##### ① プリミティブ型 → `{}`で初期化（ゼロクリア）
```cpp
int m_count{};           // 0で初期化
float m_speed{};         // 0.0fで初期化
bool m_isActive{};       // falseで初期化
```

##### ② クラス型 → デフォルトコンストラクタに任せる
```cpp
core::Vector3 m_position;      // Vector3のデフォルトコンストラクタ
std::string m_name;            // 空文字列で初期化される
std::vector<int> m_data;       // 空配列
```

##### ③ enum型 → 明示的にデフォルト値を指定
```cpp
constant::CollisionTag m_tag = constant::CollisionTag::None;  // ✓ 既に実装済み
```

##### ④ スマートポインタ → `{}`で初期化（nullptr）
```cpp
std::unique_ptr<Player> m_player{};
std::shared_ptr<Data> m_data{};
```

##### ⑤ 参照型 → コンストラクタ初期化リストで初期化（`{}`不可）
```cpp
ICamera& m_camera;  // 初期化リストで初期化必須
```

#### 対象ファイル
- `src/core/ecs/EntityManager.h` - `m_nextId`
- `src/core/ecs/ComponentManager.h` - すべてのメンバ変数
- `src/game/system/*.h` - すべてのSystem クラスのメンバ変数
- `src/infrastructure/*.h` - すべてのメンバ変数

#### 既に正しく実装されている例
- `src/game/component/TransformComponent.h` - `m_scale = {1.0f, 1.0f, 1.0f}` ✓
- `src/game/component/ColliderComponent.h` - `m_tag = constant::CollisionTag::None` ✓

---

### 4. InGameScene コンストラクタの責務分離 🔴 **必須**

#### 現状の問題
`InGameScene.cpp` のコンストラクタが約70行で、以下の責務が混在：
1. **モデルのロード** (19-29行)
2. **オブジェクトの生成** (31-45行)
3. **システムの登録** (47-52行)

#### リファクタリング方針
コンストラクタを以下の3つのprivateメソッドに分割：

```cpp
// InGameScene.h (private:)
void loadResources();      // モデル・リソースのロード
void spawnEntities();      // エンティティ生成
void setupSystems();       // システム登録・設定

// InGameScene.cpp
InGameScene::InGameScene(...)
    : m_camera(camera)
    , m_renderer(renderer)
    , m_animator(animator)
    , m_resourceManager(resourceManager)
    , m_inputProvider(inputProvider)
    , m_objectFactory(m_entityManager, m_componentManager, m_resourceManager)
{
    loadResources();
    spawnEntities();
    setupSystems();
}

void InGameScene::loadResources()
{
    // モデルロード処理
    int playerModelHandle = m_resourceManager.loadModelById(constant::ModelId::PLAYER);
    auto playerMeta = m_resourceManager.getMetadata(constant::ModelId::PLAYER);
    assert(playerMeta.has_value() && "Playerのメタデータが見つかりません");
    m_playerData = game::data::PlayerData::fromMetadata(playerMeta.value());
    
    // 他のモデルロード...
}

void InGameScene::spawnEntities()
{
    // エンティティ生成処理
    m_objectFactory.init(playerModelHandle, m_playerData);
    m_ground = std::make_unique<game::stage::Ground>(...);
}

void InGameScene::setupSystems()
{
    // システム登録処理
    m_systemManager.registerSystem<game::system::InputSystem>(...);
    m_systemManager.registerSystem<game::system::MoveSystem>(...);
    // ...
}
```

#### メリット
- 各処理の責務が明確化
- テスト容易性の向上
- エラーハンドリングの分離が可能
- コードの可読性向上

---

### 5. 命名規則の統一 🟢 **推奨**

#### 問題箇所
`src/core/ecs/SystemManager.h` (24行目)
```cpp
// Before
std::vector<std::unique_ptr<ISystem>> m_system;

// After
std::vector<std::unique_ptr<ISystem>> m_systems;  // 複数形に修正
```

#### 理由
- コレクション型（vector, array等）は複数形にするのが一般的
- 可読性の向上

---

### 6. エンティティIDのキャッシング（パフォーマンス改善） 🟡 **重要**

#### 現状の問題
`InGameScene::update()` で、毎フレーム同じエンティティIDを取得：
```cpp
// 毎フレーム実行（60FPS = 1秒間に60回）
auto& transform = m_componentManager.get<TransformComponent>(m_objectFactory.getPlayer().getId());
auto& render = m_componentManager.get<RenderComponent>(m_objectFactory.getPlayer().getId());
auto& anim = m_componentManager.get<AnimationComponent>(m_objectFactory.getPlayer().getId());
// ... 6回以上の重複呼び出し
```

#### 改善方針
コンストラクタまたは初期化時にキャッシュ：
```cpp
// InGameScene.h (private:)
core::ecs::EntityId m_playerId{};
core::ecs::EntityId m_groundId{};

// InGameScene.cpp (コンストラクタまたはspawnEntities()内)
m_playerId = m_objectFactory.getPlayer().getId();
m_groundId = m_ground->getId();

// update()内
auto& transform = m_componentManager.get<TransformComponent>(m_playerId);
auto& render = m_componentManager.get<RenderComponent>(m_playerId);
```

---

### 7. const correctness と noexcept の追加 🟢 **推奨**

#### 対象
例外を投げないgetter関数に `noexcept` を追加：

```cpp
// Before
core::ecs::EntityId getId() const { return m_entity.getId(); }

// After
core::ecs::EntityId getId() const noexcept { return m_entity.getId(); }
```

#### 対象ファイル
- `src/game/actor/Player.h`
- `src/game/stage/Ground.h`
- `src/core/ecs/Entity.h`
- `src/game/data/PlayerData.h` - すべてのgetter
- `src/game/data/GroundData.h` - すべてのgetter

#### メリット
- コンパイラの最適化が可能になる
- 意図が明確になる（この関数は絶対に例外を投げない）

---

### 8. エラーハンドリングの強化 🟡 **重要**

#### 現状の問題
`assert()` はReleaseビルドで無効化される：
```cpp
assert(playerMeta.has_value() && "Playerのメタデータが見つかりません");
```

#### 改善方針
本番環境でも動作するエラーハンドリングを追加：
```cpp
if (!playerMeta.has_value()) {
    LOG("ERROR", "Playerのメタデータが見つかりません");
    throw std::runtime_error("Failed to load player metadata");
}
```

#### 対象箇所
- `src/game/scene/InGameScene.cpp` - メタデータ取得時
- `src/infrastructure/ResourceManager.cpp` - JSON読み込み失敗時
- `src/game/data/PlayerData.h` - `fromMetadata()` 内

#### 注意点
- 開発中は `assert()` も残す（デバッグに有用）
- 本番用エラーハンドリングを追加で実装

---

### 9. ResourceManager の責務分離（将来的な改善） 🔵 **低優先度**

#### 現状
`ResourceManager` が複数の責務を持つ：
- JSONパース
- モデルロード
- キャッシュ管理

#### 将来的な分離案
```
infrastructure/
├─ ResourceManager.h / .cpp      // ファサード
├─ JsonLoader.h / .cpp           // JSON読み込み専用
└─ ModelCache.h / .cpp           // キャッシュ管理専用
```

#### 注意
- **EditorTool実装前には実施しない**
- PR_feature_collider.md の「技術的負債の記録」に既に記載済み
- EditorTool実装後、必要に応じて実施

---

### 10. 依存関係の逆転（Clean Architecture強化） 🔵 **低優先度**

#### 現状の問題
`InGameScene.cpp` が具体的なSystem実装に直接依存：
```cpp
#include "game/system/InputSystem.h"
#include "game/system/MoveSystem.h"
#include "game/system/PhysicsSystem.h"
// ... 増える一方
```

#### 改善案
`InGameSceneInitializer` などに委譲：
```cpp
// infrastructure/InGameSceneInitializer.h
class InGameSceneInitializer {
public:
    static void setupSystems(SystemManager&, ...);
};
```

#### 注意
- **EditorTool実装前には実施しない**
- システムが増えてから検討

---

## 実装順序（推奨）

### フェーズ1：ドキュメント整備（1-2日） 🔴
- [x] 1. public関数へのDoxygenコメント追加

### フェーズ2：InGameScene リファクタリング（1日） 🔴
- [x] 4. コンストラクタの責務分離 (`loadResources()`, `spawnEntities()`, `setupSystems()`)
- [x] 6. エンティティIDのキャッシング

### フェーズ3：Factory Pattern導入（2-3日） 🟡
- [x] 2. IFactory + PlayerFactory + GroundFactory + FactoryManager 実装

### フェーズ4：細かい改善（1日） 🟢
- [x] 3. メンバ変数の初期化統一
- [x] 5. 命名規則の修正 (`m_system` → `m_systems`)
- [x] 7. const/noexcept の追加
- [x] 8. エラーハンドリングの強化

### フェーズ5：EditorTool開始 ✨
→ リファクタリング完了後、`feature/editor-scene` ブランチ作成

### フェーズ6：将来的な改善（EditorTool実装後） 🔵
- [ ] 9. ResourceManager の責務分離
- [ ] 10. 依存関係の逆転

---

## 完了条件

以下をすべて満たしたら EditorTool 実装に移行：
- ✅ すべてのpublic関数にDoxygenコメント
- ✅ InGameScene コンストラクタが50行以内
- ✅ Factory Pattern が導入され、ObjectFactory が削除されている
- ✅ メンバ変数の初期化が統一されている
- ✅ コンパイル・実行が正常に動作
- ✅ 既存機能（Player移動、コライダー可視化、衝突判定）が正常動作

---

## 備考

### Git管理
- ブランチ名: `refactor/clean-architecture`
- コミットメッセージ例:
  - `docs: public関数にDoxygenコメント追加`
  - `refactor: InGameSceneコンストラクタを3メソッドに分割`
  - `feat: Factory Patternを導入（IFactory + PlayerFactory + FactoryManager）`

### テスト
各フェーズ完了後、以下を確認：
- [x] コンパイルエラーがない
- [x] Player移動が正常動作
- [x] コライダー可視化が正常動作
- [x] 衝突判定が正常動作
- [x] カメラ追従が正常動作

### 就活作品としてのアピールポイント
- **ドキュメント**：すべての関数にコメントがあり、他者が見ても理解しやすい
- **設計パターン**：Factory Pattern を適切に導入し、拡張性を確保
- **Clean Architecture**：責務分離、依存関係逆転を意識した設計
- **保守性**：統一された命名規則、初期化方法、エラーハンドリング
